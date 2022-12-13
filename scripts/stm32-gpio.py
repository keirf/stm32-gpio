#!/usr/bin/env python3

import sys, serial

f = serial.Serial('/dev/stm32-gpio')
f.reset_output_buffer()
f.reset_input_buffer()

def cmd(cmd):
    f.write(b'\n' + cmd.encode('utf-8') + b'\n')
    return f.read(1).decode('utf-8')

sts = cmd(sys.argv[1])
print(sts)
