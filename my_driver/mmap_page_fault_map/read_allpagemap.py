#!/usr/bin/env python3

import os
import sys
import binascii

def read_pagemap(pid):
    with open("/proc/{}/pagemap".format(pid), "rb") as f:
        while True:
            data = f.read(8)
            if not data:
                break
            entry = int(binascii.hexlify(data), 16)
            print("Page Entry: 0x{:016x}".format(entry))

if __name__ == "__main__":
    pid = int(sys.argv[1])
    read_pagemap(pid)