#pragma once

//https://github.com/MCUdude/MiniCore/blob/master/avr/variants/standard/pins_arduino.h#L154
/*
#define PIN_PD0 0
#define PIN_PD1 1
#define PIN_PD2 2
#define PIN_PD3 3
#define PIN_PD4 4
#define PIN_PD5 5
#define PIN_PD6 6
#define PIN_PD7 7
#define PIN_PB0 8
#define PIN_PB1 9
#define PIN_PB2 10
#define PIN_PB3 11
#define PIN_PB4 12
#define PIN_PB5 13
#define PIN_PC0 14 // A0
#define PIN_PC1 15 // A1
#define PIN_PC2 16 // A2
#define PIN_PC3 17 // A3
#define PIN_PC4 18 // A4
#define PIN_PC5 19 // A5
#define PIN_PB6 20 // XTAL1
#define PIN_PB7 21 // XTAL2
#define PIN_PC6 22 // RESET
#define PIN_A6 (6)
#define PIN_A7 (7)
*/

#define PIN_D1 PIN_PC1
#define PIN_D2 PIN_PC2
#define PIN_D3 PIN_PB2
#define PIN_D4 PIN_PB1
#define PIN_D5 PIN_PB0
#define PIN_D6 PIN_PD7
#define PIN_D7 PIN_PD6
#define PIN_D8 PIN_PD5
#define PIN_D9 PIN_PB7
#define PIN_D10 PIN_PB6
#define PIN_D11 PIN_PD2
#define PIN_D13 PIN_PD4
#define PIN_D14 PIN_PD3
#define PIN_MISO PIN_PB4
#define PIN_MOSI PIN_PB3
#define PIN_SCK PIN_PB5
#define PIN_DI1 PIN_PC5
#define PIN_DI2 PIN_PC2
#define PIN_DI3 PIN_PC4
#define PIN_DI4 PIN_PC3
#define PIN_RX PIN_PD0
#define PIN_TX PIN_PD1

#define PIN_SDA PIN_PC4
#define PIN_SCL PIN_PC5