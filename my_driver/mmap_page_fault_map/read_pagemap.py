import os
import sys
import struct

def read_pagemap(pid, virtual_address):
    page_size = os.sysconf(os.sysconf_names['SC_PAGE_SIZE'])
    index = virtual_address/page_size * 8
    print("virtual_address ",virtual_address)
    print("page_size ",page_size)
    print("index ",index)
    with open('/proc/{}/pagemap'.format(pid), 'rb') as f:
        f.seek(index)
        entry = f.read(8)

    pfn = struct.unpack('Q', entry)[0] & 0x7FFFFFFFFFFFFF
    flags = struct.unpack('Q', entry)[0] & 0x8000000000000000

    print('Process ID: {}'.format(pid))
    print('Virtual Address: {}'.format(hex(virtual_address)))
    print('PFN: {}'.format(hex(pfn)))
    print('Flags: {}'.format(hex(flags)))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <PID> <Virtual_Address>")
        sys.exit(1)

    pid = int(sys.argv[1])
    virtual_address = int(sys.argv[2], 16)

    read_pagemap(pid, virtual_address)