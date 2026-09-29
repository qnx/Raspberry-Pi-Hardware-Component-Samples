# Relay Controlled Electromagnet Hardware Sample

This project demonstrates how to control a 12V electromagnet using a relay module connected to a Raspberry Pi.

It includes relay control through a GPIO output and user input through a push button, allowing the electromagnet to be energized while the button is pressed.

## Relay Module + Push Button + Electromagnet

This project uses a relay module to safely switch a 12V electromagnet from a Raspberry Pi GPIO pin.

Since the Raspberry Pi GPIO operates at 3.3V and cannot directly power high-current devices, a relay acts as an electrically isolated switch between the Raspberry Pi and the electromagnet power supply.

A push button connected to a GPIO input allows the user to control the relay. When the button is pressed, the relay is activated and the electromagnet is energized. When the button is released, the relay is deactivated and the electromagnet turns off.

## Hardware Requirements

- Raspberry Pi 5
- 5V Relay Module
- Push Button
- 12V Electromagnet
- 12V DC Power Supply
- Breadboard
- Jumper Wires
- Resistance

## Pin Configuration

| Component | Raspberry Pi Pin | Description |
|------------|-----------------|-------------|
| Relay IN   | GPIO 17  | Relay control signal |
| Relay VCC  | 5V       | Relay power |
| Relay GND  | GND     | Ground |
| Button     | GPIO 19  | Button input |
| Button     | 3.3V    | Button power |

## Relay Wiring

### Raspberry Pi to Relay

| Relay Pin | Raspberry Pi Pin |
|------------|-----------------|
| VCC        | 5V |
| GND        | GND |
| IN         | GPIO 17 |

### Relay to Electromagnet Power Circuit

| Relay Terminal | Connection |
|---------------|------------|
| COM           | +12V Power Supply |
| NO            | Electromagnet Positive (+) |
| Electromagnet Negative (-) | 12V Power Supply Ground (-) |

The Normally Open (NO) contact is used so the electromagnet remains off until the relay is energized.

## Button Wiring

| Button Connection | Raspberry Pi Pin |
|------------------|------------------|
| One Side         | GPIO 19 |
| Other Side       | 3.3V |

### Pull-Down Resistor

| Resistor Connection | Connection |
|--------------------|------------|
| One End            | GPIO 19 |
| Other End          | GND |
| Value              | 10kΩ |


## Operation

1. Press the push button.
2. GPIO 19 detects the input signal.
3. GPIO 17 activates the relay.
4. The relay closes the 12V circuit.
5. The electromagnet is energized.

When the button is released:

1. GPIO 19 returns low.
2. GPIO 17 deactivates the relay.
3. The relay opens the circuit.
4. The electromagnet is de-energized.

## Schematic Diagram

<img src="./circuit_image.svg" >