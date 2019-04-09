I2C NeoPixel Controller
================================================================================

This is a tiny sketch that lets you control [NeoPixels] over I2C. It was
originally designed for a project where a [Raspberry Pi] needed to control a
bunch of LEDs but was busy doing other things.

It's designed for the [Trinket M0] but will probably run on just about anything.

[NeoPixels]: https://www.adafruit.com/category/168
[Raspberry Pi]: https://www.raspberrypi.org/
[Trinket M0]: https://www.adafruit.com/product/3500

Setup
--------------------------------------------------------------------------------

It's simple!

1. Acquire some sort of board you can program with the Arduino IDE. I suggest a
   [Trinket M0].
2. Install the [Adafruit NeoPixel Library] by opening your Arduino IDE and going
   to Sketch -> Include Libraries -> Manage Libraries...
3. verify that `I2C_ADDRESS` is set to a usable value. It defaults to `0x45` but
   maybe you have a device on that address already?
4. set `NEOPIXEL_PIN`, `NEOPIXEL_COUNT`, and `NEOPIXEL_TYPE` to match your setup
5. (optional) configure default values for `colorCorrection` and `brightness`
6. flash your board!

[Adafruit NeoPixel Library]: https://github.com/adafruit/Adafruit_NeoPixel

Usage
--------------------------------------------------------------------------------

By default the device is on i2c address `0x45`. It's controlled via a simple
protocol which consists of a command byte and (optionally) additional bytes as
arguments. See [Protocol Reference](#protocol-reference) below for details.

**Note**: the command parser is intentionally very simple. It looks at the first
byte of the message and dispatches to a handler that may or may not read
additional bytes. When the handler is done the buffer is flushed. This means
that you must send each command in a single, self-contained i2c message. This
works for me but may be a bad idea; please open an issue if you have objections.

You can enable and disable quick-and-dirty color correction using the `0x03` and
`0x04` commands, respectively. The gamma table is precalculated for your
convenience. See [this page][gamma] for more information. Color correction is
disabled by default.

[gamma]: https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix


Protocol Reference
--------------------------------------------------------------------------------

| Command | Args                  | Description                             |
|---------|-----------------------|-----------------------------------------|
| `0x00`  | None                  | Turns off all of the LEDs               |
| `0x01`  | 1 byte                | Sets the maximum brightness             |
| `0x02`  | 4 bytes: LED, R, G, B | Sets the given LED's R, G, and B values |
| `0x03`  | None                  | Enables color correction                |
| `0x04`  | None                  | Disables color correction               |

License
--------------------------------------------------------------------------------

Copyright (c) 2019 [Ben Bleything](mailto:ben@bleything.net)

Released under the terms of the MIT License; see `LICENSE` for details.
