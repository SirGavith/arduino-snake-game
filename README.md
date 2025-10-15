# Snake game
Technical Manual v0.0.2

Updated October 14, 2025

## What the device does
The device is a simple snake game. The display is an MAX7219 LED Dot Matrix Module, which is an 8x8 matrix of red leds in which the snake lives. The input is a standard dual-axis joystick module. These are connected to a standard Arduino UNO. Manipulating the joystick controls the snake as it moves on the display. As you eat apples, the snake grows longer, but if it eats its own tail, you lose. How long can you get before you lose?

## Required Components
- 1x Arduino UNO
- 1x Breadboard
- 1x MAX7219 LED Dot Matrix Module
- 1x Joystick Module
- 1x 74HC595 Shift Register IC
- 1x 1 Digit 7-Segment Display
- 8x 220 ohm resistors
- 28x M-M wires

## How the subsystems come together to make it work
To wire together the system, begin by placing each component on the breadboard. Continue by wiring the 74HC595 shift register to the 7-segment display. Refer to the wiring diagram for the exact connections. On the wiring diagram, these wires are denoted with rainbow colors. Note that on the 74HC595, these are pins 0-7 and 15. Also note that each of the 8 connections should include a 220Ω current limiting resistor. Next ground the 7-segment display (pins 3 and 8), and the 74HC595 (pins 8 and 13). Connect pins 10 and 16 of the 74HC595 to high. Finally, make the three connections (denoted in white) from the Arduino’s digital pins 2, 3, and 4 to the 74HC595’s pins 14, 12, 11, respectively.

Next, connect the Dot Matrix Module CLK, CS, and DIN to the Arduino using digital pins 10, 11, and 12, respectively. Connect the Joystick Module Vy and Vx to the Arduino Analog pins 0 and 1 respectively. Finally, connect the ground and Vcc of the Dot Matrix Module and the Joystick Module to the busses, and connect them to the Arduino 5V and GND.


Figure I: A wiring diagram for the completed device.

The 74HC595 Shift Register is used so that the Arduino can use only three output pins to control all eight inputs to the 7-segment display. It does this by shifting out a byte one bit at a time, the timing given by the clock.

In software, the code relies on the LedControl library in order to easily write to the display. It has a custom light XY library, in order to make the game logic easy. It polls inputs every 10 ms to make the game feel good, and the game ticks (making the snake move) starting every second, although as you eat more food, the snake moves faster, with the time between steps decaying exponentially.

The game stores a list of positions that make up the snake. When the snake moves, a new one is pushed to the end in the new head position. If the snake isn’t growing, the tail is also popped off the start. When its head collides with the apple, a new apple spawns somewhere else.


## How to play
In order to play the snake game, simply turn on the device by connecting it to a laptop and uploading the code. A snake will appear on the screen. Control it using the joystick. Eat apples to grow longer, but make sure not to eat your own tail! How many apples can you eat before the snake grows too fast and long to handle?

## Troubleshooting
In the case that nothing lights up:
- Verify that the Arduino is connected to the computer. A light on the Arduino should flash to signify this.
- If it flashes for a second, but then everything shuts off, you may have a short circuit. A detailed wiring diagram can be found in Figure I.
- Verify that all connections are made correctly and solidly. A detailed wiring diagram can be found in Figure I.
- Ensure that the code is successfully being uploaded to the Arduino. There may be unforeseen errors in the console, such as connection compatibility issues, baud issues, or similar. To read diagnostic information via the serial monitor, use 9600 baud.

In the case that the joystick makes the snake turn in the wrong direction:
- Some wires may have been crossed. Ensure that all wires are correctly connected, particularly those between the joystick and Arduino input (VR_x and VR_y).

In the case that the snake goes away or some other software glitch:
- Reboot the system by pressing the Arduino’s reset button or reuploading the software.

## Code
Code can be found at https://github.com/SirGavith/arduino-snake-game/blob/main/snake.ino
