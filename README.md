# stm32-gpio

Control STM32F10x GPIO over USB serial.

Commands are terminated by `\n`.

Command strings (examples affect pin PA9):
* `sa9`: no-op (just return pin status)
* `ha9`: set output high
* `la9`: set output low
* `ua9`: set input pull-up
* `da9`: set input pull-down
* `fa9`: set input floating

All commands return a single byte representing pin status: `0` or `1`.

Unrecognised input is ignored (no response).

Send `\n` to sync. For example: `\nha9\n`.

### Redistribution

Source code, and all binary releases, are freely redistributable
in any form. Please see the [license](COPYING).
