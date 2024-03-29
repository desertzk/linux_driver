/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//#define DEBUG

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/notifier.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#include "../iio.h"
#include "../sysfs.h"
#include "../machine.h"

#define _USING_PROTOTYPE_

/*
 * ADC definitions
 */
#define	ADC_MAX_SAMPLE_RATE		1*1000*1000	// with 6bit
#define	ADC_MAX_SAMPLE_BITS		6
#define	ADC_MAX_PRESCALE		256			// 8bit
#define	ADC_MIN_PRESCALE		20			// 8bit
#define	ADC_WAIT_DELAY			1000000		// usec

/*
 * ADC register
 */
struct adc_register {
	volatile U32 ADCCON;
	volatile U32 ADCDAT;
	volatile U32 ADCINTENB;
	volatile U32 ADCINTCLR;
};

#define	APEN_BITP	(14)
#define	APSV_BITP	(6)
#define	ASEL_BITP	(3)
#define	ADCON_STBY	(2)
#define	ADEN_BITP	(0)
#define	AIEN_BITP	(0)
#define	AICL_BITP	(0)

#define	ADC_BASE	((struct adc_register *)IO_ADDRESS(PHY_BASEADDR_ADC))

/*
 * ADC data
 */
struct nxp_adc_info {
	void __iomem *adc_base;
	ulong clock_rate;
	ulong sample_rate;
	ulong max_sampele_rate;
	ulong min_sampele_rate;
	int		value;
	int		prescale;
	spinlock_t	lock;
	struct completion completion;
	int support_interrupt;
	int irq;
	struct iio_map *map;
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	struct iio_dev *iio;
	struct workqueue_struct *monitoring_wqueue;
	struct delayed_work monitoring_work;

	int board_temperature;
	int tmp_voltage;
	int prev_board_temperature;

	int isValid;
	int bFirst;
	int isCheckedCount;
	struct notifier_block pm_notifier;
	unsigned long resume_state;
#endif
};

#define	STATE_RESUME_DONE	0

static const char *str_adc_ch[] = {
	"adc.0", "adc.1", "adc.2", "adc.3",
#ifdef CONFIG_ARCH_S5P4418
	"adc.4", "adc.5", "adc.6", "adc.7",
#endif
};

static const char *str_adc_label[] = {
	"ADC0", "ADC1", "ADC2", "ADC3",
#ifdef CONFIG_ARCH_S5P4418
	"ADC4", "ADC5", "ADC6", "ADC7",
#endif
};

#define ADC_CHANNEL_SPEC(_id) {	\
	.type = IIO_VOLTAGE,		\
	.indexed = 1,				\
	.channel = _id,				\
	.scan_index = _id,			\
}

static struct iio_chan_spec nxp_adc_iio_channels [] = {
	ADC_CHANNEL_SPEC(0),
	ADC_CHANNEL_SPEC(1),
	ADC_CHANNEL_SPEC(2),
	ADC_CHANNEL_SPEC(3),
#ifdef CONFIG_ARCH_S5P4418
	ADC_CHANNEL_SPEC(4),
	ADC_CHANNEL_SPEC(5),
	ADC_CHANNEL_SPEC(6),
	ADC_CHANNEL_SPEC(7),
#endif
};

/*
 * for get channel
 * 		consumer_dev_name 	= iio_st_channel_get_all 	: name = nxp-adc
 * 		consumer_channel 	= iio_st_channel_get 		: channel_name = adc.0, 1, ...
 *
 * for find channel spec (iio_chan_spec)
 * 		adc_channel_label	= must be equal to iio_chan_spec->datasheet_name = ADC0, 1, ...
 */
#define ADC_CHANNEL_MAP(_id) {	\
    .consumer_dev_name =  DEV_NAME_ADC,	\
}

static struct iio_map nxp_adc_iio_maps [] = {
	ADC_CHANNEL_MAP(0),
	ADC_CHANNEL_MAP(1),
	ADC_CHANNEL_MAP(2),
	ADC_CHANNEL_MAP(3),
	ADC_CHANNEL_MAP(4),
	ADC_CHANNEL_MAP(5),
	ADC_CHANNEL_MAP(6),
	ADC_CHANNEL_MAP(7),
	{ },
};

extern int iio_map_array_register(struct iio_dev *indio_dev, struct iio_map *maps);
extern int iio_map_array_unregister(struct iio_dev *indio_dev, struct iio_map *maps);

/*
 * ADC functions
 */
#ifdef CONFIG_NXP_ADC_INTERRUPT
static irqreturn_t nxp_adc_isr(int irq, void *dev_id)
{
	struct nxp_adc_info *adc = (struct nxp_adc_info *)dev_id;
	struct adc_register *reg = ADC_BASE;

	__raw_writel(1, &reg->ADCINTCLR);

	adc->value = __raw_readl(&reg->ADCDAT);	/* get */
	complete(&adc->completion);
	return IRQ_HANDLED;
}
#endif

#define	ADC_HW_RESET()		do { nxp_soc_peri_reset_set(RESET_ID_ADC); } while (0)

static int nxp_adc_setup(struct nxp_adc_info *adc, struct platform_device *pdev)
{
	struct clk *clk = NULL;
	ulong sample_rate, clk_rate, min_rate;
#ifdef _USING_PROTOTYPE_
#else
	struct adc_register *reg = ADC_BASE;
	unsigned int adcon = 0;
#endif
	int irq = 0, interrupt = 0, prescale = 0;
	int ret = 0;

	clk	= clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		pr_err("Fail: getting clock ADC !!!\n");
		return -EINVAL;
	}
 	clk_rate = clk_get_rate(clk);
	clk_put(clk);

	sample_rate = *(ulong *)(pdev->dev.platform_data);
	prescale = clk_rate/(sample_rate * ADC_MAX_SAMPLE_BITS);
	min_rate = clk_rate/(ADC_MAX_PRESCALE * ADC_MAX_SAMPLE_BITS);

	if (sample_rate > ADC_MAX_SAMPLE_RATE ||
		min_rate > sample_rate) {
		pr_err("ADC: not suport %lu(%d ~ %lu) sample rate\n",
			sample_rate, ADC_MAX_SAMPLE_RATE, min_rate);
		return -EINVAL;
	}

#ifdef CONFIG_NXP_ADC_INTERRUPT
	irq = platform_get_irq(pdev, 0);
	if ((irq >= 0) || (NR_IRQS > irq)) {
		ret = request_irq(irq, nxp_adc_isr, 0, DEV_NAME_ADC, adc);
		if (0 == ret)
			interrupt = 1;
		ret = 0;
	}
#endif

	adc->clock_rate = clk_rate;
	adc->support_interrupt = interrupt;
	adc->irq = irq;
	adc->sample_rate = sample_rate;
	adc->max_sampele_rate = ADC_MAX_SAMPLE_RATE;
	adc->min_sampele_rate = min_rate;
	adc->prescale = prescale;
	spin_lock_init(&adc->lock);

#ifdef _USING_PROTOTYPE_
	NX_ADC_SetBaseAddress(0, (U32)IO_ADDRESS(NX_ADC_GetPhysicalAddress(0)));
 	NX_ADC_OpenModule(0);

	ADC_HW_RESET();
	NX_ADC_SetInterruptEnableAll(0, CTRUE);
	//	NX_ADC_SetInputChannel(0, ch);
	NX_ADC_SetPrescalerValue(0, adc->prescale) ;
	NX_ADC_SetPrescalerEnable(0, CTRUE);
	NX_ADC_SetStandbyMode(0, CFALSE);

#else
	ADC_HW_RESET();
	adcon = ((prescale & 0xFF) << APSV_BITP) |
			(1 << APEN_BITP) |
			(0 << ADCON_STBY) ;
	__raw_writel(adcon, &reg->ADCCON);

	if (interrupt) {
		__raw_writel(1, &reg->ADCINTCLR);
		__raw_writel(1, &reg->ADCINTENB);
		init_completion(&adc->completion);
	}
#endif

	pr_info("ADC: CHs %d, %ld(%ld ~ %ld) sample rate, %s mode, scale=%d(bit %d)\n",
		ARRAY_SIZE(nxp_adc_iio_channels), adc->sample_rate,
		adc->max_sampele_rate, adc->min_sampele_rate,
		interrupt?"irq":"polling", prescale, ADC_MAX_SAMPLE_BITS);

	return ret;
}

static void nxp_adc_release(struct nxp_adc_info *adc)
{
	if (adc->support_interrupt)
		free_irq(adc->irq, adc);
}

static int nxp_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan,
				int *val,
				int *val2,
				long mask)
{
	struct nxp_adc_info *adc = iio_priv(indio_dev);
	struct adc_register *reg = ADC_BASE;
	int ch = chan->channel;
	volatile int wait = ADC_WAIT_DELAY;
	unsigned int adcon = 0;
	unsigned long flags;

	if (adc->support_interrupt) {
		mutex_lock(&indio_dev->mlock);

		adcon  = __raw_readl(&reg->ADCCON) & ~(0x07 << ASEL_BITP);
		adcon |= ch << ASEL_BITP;	// channel
		adcon |=  1 << ADEN_BITP;	// start
		__raw_writel(adcon, &reg->ADCCON);

		wait_for_completion(&adc->completion);
		*val = adc->value;

		mutex_unlock(&indio_dev->mlock);
	} else {
#ifdef _USING_PROTOTYPE_
		int value = 0;

		spin_lock_irqsave(&adc->lock, flags);

		wait = ADC_WAIT_DELAY;

		NX_ADC_SetInputChannel(0, ch);
		NX_ADC_ClearInterruptPendingAll(0);
		NX_ADC_Start(0);

		do {
			if (NX_ADC_GetInterruptPendingAll(0)) {
				value = NX_ADC_GetConvertedData(0);
				NX_ADC_ClearInterruptPendingAll(0);

				break;
			}
		} while (wait-- > 0);
		if (0 > wait) {
			spin_unlock_irqrestore(&adc->lock, flags);
			return -EINVAL;
		}

		*val = value;
		spin_unlock_irqrestore(&adc->lock, flags);
#else
		spin_lock_irqsave(&adc->lock, flags);

		adcon  = __raw_readl(&reg->ADCCON) & ~(0x07 << ASEL_BITP);
		adcon |= ch << ASEL_BITP;	// channel
		adcon |=  1 << ADEN_BITP;	// start
		__raw_writel(adcon, &reg->ADCCON);

		while (wait-- > 0) {
			if (! (__raw_readl(&reg->ADCCON) & (1<<ADEN_BITP))) {
				*val = __raw_readl(&reg->ADCDAT);	/* get */
				break;
			}
		}
		if (0 > wait) {
			spin_unlock_irqrestore(&adc->lock, flags);
			return -EINVAL;
		}
		spin_unlock_irqrestore(&adc->lock, flags);
#endif
	}

	usleep_range (1, 10);
	pr_debug("%s, ch=%d, val=0x%x\n", __func__, ch, *val);

	return IIO_VAL_INT;
}

static const struct iio_info nxp_adc_iio_info = {
	.read_raw = &nxp_read_raw,
	.driver_module = THIS_MODULE,
};

static int nxp_adc_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int nxp_adc_resume(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct nxp_adc_info *adc = iio_priv(indio_dev);
#ifdef _USING_PROTOTYPE_
#else
	struct adc_register *reg = ADC_BASE;
	unsigned int adcon = 0;
#endif

#ifdef _USING_PROTOTYPE_
	NX_ADC_SetBaseAddress(0, (U32)IO_ADDRESS(NX_ADC_GetPhysicalAddress(0)));
 	NX_ADC_OpenModule(0);

	ADC_HW_RESET();
	NX_ADC_SetInterruptEnableAll(0, CTRUE);
	//	NX_ADC_SetInputChannel(0, ch);
	NX_ADC_SetPrescalerValue(0, adc->prescale) ;
	NX_ADC_SetPrescalerEnable(0, CTRUE);
	NX_ADC_SetStandbyMode(0, CFALSE);
#else
	ADC_HW_RESET();
	adcon = ((adc->prescale & 0xFF) << APSV_BITP) |
			(1 << APEN_BITP) |
			(0 << ADCON_STBY) ;
	__raw_writel(adcon, &reg->ADCCON);

	if (adc->support_interrupt) {
		__raw_writel(1, &reg->ADCINTCLR);
		__raw_writel(1, &reg->ADCINTENB);
		init_completion(&adc->completion);
	}
#endif
	return 0;
}


#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
int eBoard_temperature = 0;
int NXP_Get_BoardTemperature(void)
{
	return eBoard_temperature;
}
EXPORT_SYMBOL_GPL(NXP_Get_BoardTemperature);

static int nxp_cpufreq_pm_notify(struct notifier_block *this,
        unsigned long mode, void *unused)
{
	struct nxp_adc_info *adc = container_of(this,
					struct nxp_adc_info, pm_notifier);

    switch(mode) {
    case PM_SUSPEND_PREPARE:
    	clear_bit(STATE_RESUME_DONE, &adc->resume_state);
    	break;
    case PM_POST_SUSPEND:
    	set_bit(STATE_RESUME_DONE, &adc->resume_state);
    	break;
    }
	return 0;
}

// initialize table for register value matching with temperature
static int drone_temperature_table[10][2] =
{
	{9900, 40}, // 0
	{9100, 45},
	{8400, 50},
	{7700, 55},
	{7000, 60}, // 4
	{6300, 65}, // 5
	{5700, 70},
	{5200, 75},
	{4700, 80},
	{4200, 85}  // 9
};

static void nxl_monitor_work_func(struct work_struct *work)
{
	struct nxp_adc_info *adc = container_of(work, struct nxp_adc_info, monitoring_work.work);
	struct iio_chan_spec const *chan;
	int val = 0;
	int val2 = 0;
	// calculate temperature
	int voltage_table_interval;
	int interval;
	int num_grade;


	if (!test_bit(STATE_RESUME_DONE, &adc->resume_state))
		goto exit_mon;

	chan = &nxp_adc_iio_channels[2];
	nxp_read_raw(adc->iio, chan, &val, &val2, 0);

	adc->tmp_voltage = (18*val*1000)/4096;

	//  according to Register Voltage table, calculate board temperature.
	for(num_grade=0, interval=0; num_grade<10; num_grade++)
	{
		if(adc->tmp_voltage > drone_temperature_table[num_grade][0])
		{
			if(num_grade != 0)
			{
				interval = (drone_temperature_table[num_grade-1][0] - drone_temperature_table[num_grade][0])/5;
				break;
			}
		}
	}

	if(num_grade == 10)
	{
		adc->board_temperature = 90;
	}
	else if(interval == 0)
	{
		adc->board_temperature = 40;
	}
	else
	{
		adc->board_temperature = drone_temperature_table[num_grade-1][1];
		voltage_table_interval = drone_temperature_table[num_grade-1][0] - interval;
		for(; voltage_table_interval>drone_temperature_table[num_grade][0]; voltage_table_interval-=interval)
		{
			if(adc->tmp_voltage > voltage_table_interval)
				break;
			adc->board_temperature++;
		}
		if(adc->board_temperature > drone_temperature_table[num_grade][1])
			adc->board_temperature = drone_temperature_table[num_grade][1];
	}

	// ignore the temperature value when booting.
	if(adc->isValid == 0)
	{
		if(adc->bFirst == 0)
		{
			adc->bFirst = 1;
			adc->prev_board_temperature = adc->board_temperature;
		}
		else
		{
			if(adc->prev_board_temperature == adc->board_temperature)
				adc->isCheckedCount++;
			else
				adc->isCheckedCount = 0;
			adc->prev_board_temperature = adc->board_temperature;

			if(adc->isCheckedCount == 3)
				adc->isValid = 1;
		}
		if(adc->isValid == 0)
		{
			queue_delayed_work(adc->monitoring_wqueue, &adc->monitoring_work, HZ);
			return;
		}
	}


	// adjust the temperature value .
	if(adc->prev_board_temperature <= adc->board_temperature)
	{
		int gap = adc->board_temperature - adc->prev_board_temperature;
		if(gap >= 5) // ignore.
		{
			adc->board_temperature = adc->prev_board_temperature;
		}
		else
		{
			adc->prev_board_temperature = adc->board_temperature;
		}
	}
	else
	{
		int gap = adc->prev_board_temperature  - adc->board_temperature;
		if(gap >= 5) // ignore.
		{
			adc->board_temperature = adc->prev_board_temperature;
		}
		else
		{
			adc->prev_board_temperature = adc->board_temperature;
		}
	}

	eBoard_temperature = adc->board_temperature;

exit_mon:
	queue_delayed_work(adc->monitoring_wqueue, &adc->monitoring_work, HZ);

}
#endif

static int __devinit nxp_adc_probe(struct platform_device *pdev)
{
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	static struct notifier_block *pm_notifier;
#endif
	struct iio_dev *iio = NULL;
	struct nxp_adc_info *adc = NULL;
	struct iio_chan_spec *spec;
	struct iio_map *map;
	int i = 0, ret = -ENODEV;

	iio = iio_allocate_device(sizeof(struct nxp_adc_info));
	if (!iio) {
		pr_err("Fail: allocating iio ADC device\n");
		return -ENOMEM;
	}

	adc = iio_priv(iio);
	ret = nxp_adc_setup(adc, pdev);
	if (0 > ret) {
		pr_err("Fail: setup iio ADC device\n");
		goto err_iio_free;
	}

	iio->name = DEV_NAME_ADC;
	iio->dev.parent = &pdev->dev;
	iio->info = &nxp_adc_iio_info;
	iio->modes = INDIO_DIRECT_MODE;
	iio->channels = nxp_adc_iio_channels;
	iio->num_channels = ARRAY_SIZE(nxp_adc_iio_channels);

	platform_set_drvdata(pdev, iio);

	/*
	* sys interface : user interface
	*/
	spec = nxp_adc_iio_channels;
	for (i = 0; iio->num_channels > i; i++)
		spec[i].datasheet_name = str_adc_label[i];

	ret = iio_device_register(iio);
	if (ret)
		goto err_iio_release;

	/*
	 * inkern interface : kernel interface
	 */
	map = nxp_adc_iio_maps;
	for (i = 0; ARRAY_SIZE(nxp_adc_iio_maps) - 1 > i; i++) {
		map[i].consumer_channel = str_adc_ch[i];
		map[i].adc_channel_label = str_adc_label[i];
	}

    ret = iio_map_array_register(iio, map);
    if (ret)
        goto err_iio_register;

	adc->map = nxp_adc_iio_maps;

#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	adc->isCheckedCount = 0;
	adc->isValid = 0;
	adc->bFirst = 0;

	adc->iio = iio;
	adc->monitoring_wqueue = create_singlethread_workqueue("monitoring_wqueue");
	INIT_DELAYED_WORK_DEFERRABLE(&adc->monitoring_work, nxl_monitor_work_func);
	queue_delayed_work(adc->monitoring_wqueue, &adc->monitoring_work, 15*HZ);

	pm_notifier = &adc->pm_notifier;
	pm_notifier->notifier_call = nxp_cpufreq_pm_notify;
	if (register_pm_notifier(pm_notifier)) {
		dev_err(&pdev->dev, "%s: Cannot pm notifier \n", __func__);
		return -1;
	}
	set_bit(STATE_RESUME_DONE, &adc->resume_state);
#endif

	pr_debug("ADC init success\n");

	return 0;

err_iio_register:
	iio_device_unregister(iio);
err_iio_release:
	nxp_adc_release(adc);
err_iio_free:
	iio_free_device(iio);
	pr_err("Fail: load ADC driver ...\n");
	return ret;
}

static int __devexit nxp_adc_remove(struct platform_device *pdev)
{
	struct iio_dev *iio = platform_get_drvdata(pdev);
	struct nxp_adc_info *adc = iio_priv(iio);

	iio_device_unregister(iio);
	if (adc->map)
		iio_map_array_unregister(iio, adc->map);

	nxp_adc_release(adc);
	platform_set_drvdata(pdev, NULL);

	iio_free_device(iio);
	return 0;
}

static struct platform_driver nxp_adc_driver = {
	.probe		= nxp_adc_probe,
	.remove		= __devexit_p(nxp_adc_remove),
	.suspend	= nxp_adc_suspend,
	.resume		= nxp_adc_resume,
	.driver		= {
		.name	= DEV_NAME_ADC,
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(nxp_adc_driver);

MODULE_AUTHOR("jhkim <jhkim@nexell.co.kr>");
MODULE_DESCRIPTION("ADC driver for the Nexell");
MODULE_LICENSE("GPL");
