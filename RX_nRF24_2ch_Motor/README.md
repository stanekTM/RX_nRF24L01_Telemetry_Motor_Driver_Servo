# RX 2ch (motor driver, telemetry)
RC receiver nRF24L01 with ATmega328P/16Mhz processor for smaller dimensions.
It is possible to use Arduino Nano or Pro Mini.
Telemetry sends the monitored voltage RX to TX. 
It includes CMT2150A transponder for laps timing and motor driver.
The motor driver IC is based on MX1208, MX1508, MX1515, MX1616, MX1919 and others similar, using 4x PWM input control signals.
The possibility of setting the brake is in the code.
The firmware will be used for micro cars, boats, tanks and robots.

This RC receiver works with RC transmitters [TX_nRF24_2ch_OLED](https://github.com/stanekTM/TX_nRF24_2ch_OLED) or
[TX_nRF24_5ch_LED](https://github.com/stanekTM/TX_nRF24_5ch_LED) from my fork.

Note: I use (Arduino) ATmega328P 5V/16Mhz and supply VCC only with 3.3V voltage. 
If you supply the VCC directly with a LiPo 1S cell, except for the nRF24L01, the analog voltage measurement will not work due to the VREF.

## Function
* MotorA = adjustable pwm/ch1
* MotorB = adjustable pwm/ch2
* Brake = on, off or adjustable effect 
* Normal mode = LED RX is lit
* Battery voltage 1S LiPo (4.2V) < monitored voltage = RX, TX LEDs flash at a interval of 0.5s
* TX transmitter off or signal loss = RX LED flash at a interval of 0.1s 
* Fail-safe = MotorA and MotorB stopped

## Arduino pins
```
D9  - pwm1/MotorA
D10 - pwm2/MotorA
D3  - pwm3/MotorB
D11 - pwm4/MotorB

D2  - LED
A7  - telemetry analog input RX battery

nRF24L01:
A0  - CE
A1  - CSN
A2  - SCK
A3  - MOSI
A4  - MISO
```

## Used libraries
* <RF24.h>      https://github.com/nRF24/RF24
* <DigitalIO.h> https://github.com/greiman/DigitalIO

## Example micro RX
The PCB is created by a custom version of the open source [PCB Elegance](https://github.com/stanekTM/PCB_Elegance) and manufactured by [JLCPCB](https://jlcpcb.com)

<img src="https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_1.jpg" width="415"><img src="https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_2.jpg" width="415">
<img src="https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_3.jpg" width="415"><img src="https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_4.jpg" width="415">
<img src="https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_5.jpg" width="415"><img src="https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_6.jpg" width="415">

## Schema
The schematic is created by a custom version of the open source [PCB Elegance](https://github.com/stanekTM/PCB_Elegance)

![Schema_Micro_RX_2ch_A1_Motor](https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_schema.PNG)

## Layout
The layout is created by a custom version of the open source [PCB Elegance](https://github.com/stanekTM/PCB_Elegance)

![Layout_Micro_RX_2ch_A1_Motor](https://raw.githubusercontent.com/stanekTM/RX_nRF24_Motor_Servo/master/documents/micro_rx_layout.PNG)
