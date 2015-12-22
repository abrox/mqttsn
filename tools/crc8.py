#!/usr/bin/env python


def crc8(buff, crc = 0, poly = 0x8C):
    l = len(buff)
    i = 0
    while i < l:
        ch = ord(buff[i])
        uc = 0
        while uc < 8:
            if (crc & 1) ^ (ch & 1):
                crc = (crc >> 1) ^ poly
            else:
                crc >>= 1
            ch >>= 1
            uc += 1
        i += 1
    return crc

if __name__=="__main__":
  print "%x"%crc8("annuli") 
  print "%x"%crc16("annuli") 
