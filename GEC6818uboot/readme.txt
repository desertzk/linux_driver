=================================================================================================
镜像文件相关说明：
Bootloader:
|                  \   
├── 2ndboot ------ |
├── nsih.txt ----- | => GECuboot.bin
└── u-boot.bin --- /

Kernel:
|                           \
├── battery.bmp ----------- |
├── debug-ramdisk.img ----- |
├── logo.bmp -------------- |
├── ramdisk-recovery.img -- | => boot.img
├── root.img.gz ----------- |
├── uImage ---------------- |
└── upate.bmp ------------- /
                                    
Filesystem:
├── system.img
├── userdata.img
├── cache.img
└── recovery.img

Tools:
└── s5p6818-sdmmc.sh -- For burnning GECuboot.bin to external booting SDCARD

=================================================================================================
fastboot手动更新方式:
1、系统上电，在串口终端中按空格键进入uboot命令行
2、如果系统环境变量有改变，特别是分区相关的变量，可以执行如下指令复位：env default -f -a; save;
3、确认USB线已正确连接至PC端，在uboot的串口终端中键入命令：fastboot 以启动传输服务
4、在PC端的命令行下，按需键入如下命令:
BOOTLOADER:
	fastboot flash GECuboot GECuboot.bin

BOOT IMAGE:
	fastboot flash boot boot.img
	
FILESYSTEM:
	fastboot flash system system.img
	可选烧写其他分区：
	fastboot flash userdata userdata.img
	fastboot flash cache cache.img
	fastboot flash recovery recovery.img

备注：如果需要手动更新QT，bootloader及内核更新方法同上，仅仅需要将system分区对应的烧写文件改为qt-rootfs.img即可。
在更换文件系统后，请确认环境变量的设置是否正确，具体设置见下文说明。

=================================================================================================
SD卡更新方式:

Android系统：
1、准备一张SD卡，在其根目录下建立GEC6818-android文件夹
2、拷贝镜像文件：GECuboot.bin boot.img system.img至GEC6818-android目录下,如果拷贝userdata.img，cache.img，recovery.img则一便更新。
3、并在GEC6818-android目录下，创建环境变量默认配置文件env.txt,不存在，则不更新系统环境变量
4、env.txt文件内容配置示例:(需正确设置环境变量,在文件末尾需要保留一空行)
	bootcmd=ext4load mmc 2:1 0x48000000 uImage;ext4load mmc 2:1 0x49000000 root.img.gz;bootm 0x48000000
	bootargs=lcd=vs070cxn tp=ft5x06
5、系统上电，系统自动检测是否需要升级，等待即可

QT系统：
1、准备一张SD卡，在其根目录下建立GEC6818-qt文件夹
2、拷贝镜像文件：GECuboot.bin boot.img qt-rootfs.img至GEC6818-qt目录下
3、并在GEC6818-qt目录下，创建环境变量默认配置文件env.txt,不存在，则不更新系统环境变量
4、env.txt文件内容配置示例:(需正确设置环境变量,在文件末尾需要保留一空行)
	bootcmd=ext4load mmc 2:1 0x48000000 uImage;bootm 0x48000000
	bootargs=root=/dev/mmcblk0p2 rw rootfstype=ext4 lcd=vs070cxn tp=ft5x06-linux
5、系统上电，系统自动检测是否需要升级，等待即可

UBUNTU系统:
1、准备一张SD卡，在其根目录下建立GEC6818-ubuntu文件夹
2、拷贝镜像文件：GECuboot.bin boot.img ubuntu-rootfs.tar.bz2至GEC6818-ubuntu目录下
3、并在GEC6818-ubuntu目录下，创建环境变量默认配置文件env.txt,不存在，则不更新系统环境变量
4、env.txt文件内容配置示例:(需正确设置环境变量,在文件末尾需要保留一空行)
	bootcmd=ext4load mmc 2:1 0x48000000 uImage;bootm 0x48000000
	bootargs=root=/dev/mmcblk0p7 rw rootfstype=ext4 lcd=vs070cxn tp=ft5x06-linux v4l=remove
5、系统上电，系统自动检测是否需要升级，等待即可

备注：如需强制更新，按着Left键启动或者将环境变量恢复为默认设置，则会触发强制升级
env default -f -a
env save
注意，自动检测是否升级是依据文件的crc来实现的，为避免较长的计算时间，最大读取单个升级文件的前5MB并计算CRC。比如system.img这类镜像文件，如果前5MB没变化，且bootloader和内核都没改变，则不会触发自动更新。为解决此类情况，可执行强制更新模式。

=================================================================================================
制作量产启动SDCARD
1、准备一张SD卡，通过gparted工具将前面保留100多MB空间，后面格式化为FAT32分区
2、运行脚本s5p6818-sdmmc.sh制作启动卡
	烧写GECuboot.bin烧写命令：
	sudo ./GEC6818-sdmmc.sh /dev/sdc GECuboot.bin
3、拷贝相应的升级文件至SD卡，方法同上。

====================================================================================
UBOOT环境变量参数说明：
1)选择LCD液晶屏
lcd变量可选择参数列表如下：
	vga-1024x768	(1024 X 768)(VGA)
	vga-1440x900	(1440 X 900)(VGA)
	hdmi-720p		(1280 X 720)(HDMI)(DON'T USING IN ANDROID)
	hdmi-1080p		(1920 X 1080)(HDMI)(DON'T USING IN ANDROID)
	vs070cxn		(1024 X 600)(RGB)
	at070tn92		(800  X 480)(RGB) 
	b116xtn04		(1366 X 768)(LVDS)
	wy070ml			(1024 X 600)(MIPI)
	p101kda			(1200 X 1920)(MIPI)
	ts8055pn		(720 X 1280)(MIPI)
示例，选择vs070cxn高清屏(1024 X 600)
env set bootargs "lcd=vs070cxn"
env save

2)选择触摸屏
tp变量可选择参数列表如下：
	ft5x06			(ANDROID)
	ft5x06-linux	(LINUX / UBUNTU)
	gt9517			(ANDROID)
示例，选择ft5x06
env set bootargs "tp=ft5x06"
env save

3)v4l=remove
	此变量用于主动屏蔽nxp_v4l驱动，解决ubuntu下uvc camera不能使用的问题，
	ubuntu下启动camera指令: guvciew -d /dev/video1

====================================================================================
环境变量示例参数：
vga-1024x768:
env set bootargs "lcd=vga-1024x768"
env save

vga-1440x900:
env set bootargs "lcd=vga-1440x900"
env save

hdmi-720p:
env set bootargs "lcd=hdmi-720p"
env save

hdmi-1080p:
env set bootargs "lcd=hdmi-1080p"
env save

RGB高清屏:
env set bootargs "lcd=vs070cxn"
env save

RGB屏:
env set bootargs "lcd=at070tn92"
env save

LVDS11.6寸屏:
env set bootargs "lcd=b116xtn04"
env save

MIPI 7寸高清屏:
env set bootargs "lcd=wy070ml"
env save

MIPI 10.1寸高清屏:
env set bootargs "lcd=p101kda"
env save

MIPI 5.5寸高清屏:
env set bootargs "lcd=ts8055pn"
env save

====================================================================================
备注：
1) 启动debug ramdisk
ext4load mmc 2:1 0x48000000 uImage; ext4load mmc 2:1 0x49000000 debug-ramdisk.img; bootm 0x48000000

2) ANDROID系统环境变量手动设置
env set bootcmd "ext4load mmc 2:1 0x48000000 uImage;ext4load mmc 2:1 0x49000000 root.img.gz;bootm 0x48000000"
env set bootargs "lcd=at070tn92 tp=ft5x06"
env save

3) QT系统环境变量手动设置
env set bootcmd "ext4load mmc 2:1 0x48000000 uImage;bootm 0x48000000"
env set bootargs "root=/dev/mmcblk0p2 rootfstype=ext4 lcd=at070tn92 tp=ft5x06-linux v4l=remove"
env save

3) UBUNTU系统环境变量手动设置
env set bootcmd "ext4load mmc 2:1 0x48000000 uImage;bootm 0x48000000"
env set bootargs "root=/dev/mmcblk0p7 rootfstype=ext4 lcd=at070tn92 tp=ft5x06-linux v4l=remove"
env save

