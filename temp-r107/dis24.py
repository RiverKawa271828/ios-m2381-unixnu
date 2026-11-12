#!/usr/bin/env python3
"""Disassemble 24A446 donor kernelcache at donor VAs (base 0xfffffe0007004000)."""
import sys, struct
from capstone import Cs, CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN
KC='/home/ri/work/unixnu-meizu/ios-m2381-unixnu/out/ipsw/24A446/kernelcache.decompressed'
BASE=0xfffffe0007004000
data=open(KC,'rb').read()
md=Cs(CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN); md.detail=False
def lo(va,n,back=0):
    s=va-back
    b=data[s-BASE:s-BASE+n*4]
    for i in md.disasm(b,s):
        m=" <<<" if i.address==va else ""
        print(f"0x{i.address:x}: {i.mnemonic}\t{i.op_str}{m}")
if __name__=='__main__':
    va=int(sys.argv[1],16); n=int(sys.argv[2]) if len(sys.argv)>2 else 30
    back=int(sys.argv[3]) if len(sys.argv)>3 else 0
    lo(va,n,back)
