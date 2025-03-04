# Reading a 10K potentiometer with the PCF8591 ADC / DAC chip

This is a simple demonstration of how to use a PCF8591 chip
with the RaspBerry PI 4B to read the resistance of a 10K
potentiometer, commonly used for adjustments on smaller
boards (usually with trim pots) or larger potentiometers
with knobs are used to adjust settings or volume.

Both the [C](./c) and [Python](./python) versions read the
potentiometer value and print it.

## Circuit

This diagram shows a representation of a circuit that can be wired up before running the sample app.

![circuit diagram connecting RaspBerry PI 4, PCF8591 chip/board and 10K potentiometer](./pcf8591-10kpot.png)

The circuit diagram is leveraging the following part from Adafruit:

[Adafruit PCF8591 Quad 8-bit ADC + 8-bit DAC - STEMMA QT / Qwiic](https://www.adafruit.com/product/4648)

but testing for the code used another board with the same chip from Amazon.ca from Gump's Grocer:

[Gump's grocery AD/DA PCF8591 Converter Module for Arduino Raspberry Pi](https://www.amazon.ca/Gumps-grocery-PCF8591-Converter-Raspberry/dp/B082W8WV97)

This board comes with a trim pot and some additional sensors built-in but removing some jumpers will allow for connecting the chips inputs and outputs to external devices.

### Connections

#### RaspBerry PI 4 to PCF8591 board

Pin 14 (GND) to PCF8591 board GND
Pin  1 (3.3V) to PCF8591 board VCC
Pin  2 (I2C1 SDA) to PCF8591 board SDA
Pin  3 (I2C1 SDL) to PCF8591 board SDL

#### external 10K potentiometer to PCF8591 board

middle pin to PCF8591 board AIN0
left and right pins to GND and Vcc from PCF8591 board or breadboard

#### Gump's Grocer board potentiometer connections

This board has a built-in 10K trim pot
that is connected to AIN0 via a jumper connecting it to the outer pin adjacent to it on the board by default.

If you do want to connect an external pot instead, remove the jumper from AIN0
and then connect the middle pin of the external pot to AIN0.
