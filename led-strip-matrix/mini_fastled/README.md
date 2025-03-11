# mini_fastled

## Overview

This folder contains a number of sample applications that have been adapted from INO examples from the FastLED web site.  Check the folder for more details.

## Content

### color-temperature

This is a port of the FastLED sample app:

[ColorTemperature.ino](https://fastled.io/docs/_color_temperature_8ino-example.html)

It demonstrates how to adjust the color and brightness of LED strips and matrices via the library's
brightness and temperature profiles.

There are a couple of notable differences with the original:

- a CRGBArray is used for the array of LEDs for easier porting
- the data pin was changed to the appropriate value for Raspberry Pi 4
- the strip type was changed to match what would work for my LED matrix
- the number of LEDs was changed to 256 to match the size LED matrix I was testing with, and all LEDs are filled with the rainbow colours
- instead of just two temperature profiles used in the original sample, this version iterates through all of the temperature profiles (for 2 seconds each, instead of 5)

[color-temperature](color-temperature)

### rgb-set-demo

This is a port of the FastLED sample app:

[RGBSetDemo.ino](https://fastled.io/docs/_r_g_b_set_demo_8ino-example.html)

It demonstrates a simple way to set LED colors in a rainbow using HSV colors combined with simple fade effects.

There are a couple of minor differences with the original:

- the number of LEDs was changed to 256 to match the size LED matrix I was testing with
- the data pin was changed to the appropriate value for Raspberry Pi 4
- the hue is incremented more slowly in this example than the original for stretching the colors

[rgb-set-demo](rgb-set-demo)

### demo-reel-100

This is a port of the FastLED sample app:

[DemoReel100.ino](https://fastled.io/docs/_demo_reel100_8ino-example.html)

This is the "100-lines-of-code" demo reel, showing just a few of the kinds of animation patterns you can quickly and easily compose using the library.  

There are a couple of minor differences with the original:

- the number of LEDs was changed to 256 to match the size LED matrix I was testing with
- the data pin was changed to the appropriate value for RaspBerry PI 4
- a CRGBArray is used for the array of LEDs for easier porting
- some of the ported code is reordered with respect to the original script due to differences with C dependency ordering

[demo-reel-100](demo-reel-100)
