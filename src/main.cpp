#include <Arduino.h>

#include "IRremote.h"
#include "Segment.h"
#include "pins.h"

HardwareSerial *serial = &Serial;
Segment *seg;
IRrecv *rec;

#define btnUp D1
#define btnDown D2
#define btnSelect 0
#define LED1 LED_BUILTIN
#define LED2 LED_BUILTIN
int counter;
int state;
bool pressedUp;
bool pressedDown;
unsigned long pressed;
unsigned long led1;
unsigned long led2;
uint8_t pinsIn[] = { D3, D4, D5, D6 };
uint8_t pinsDig[] = { D10, D9, D8, D7 };
IRData data;


void setup()
{
    seg = new Segment(4, false);
    rec = new IRrecv(D0);

    seg->setInputPins(pinsIn);
    seg->setDigitPins(pinsDig);

    pinMode(btnUp, INPUT);
    pinMode(btnDown, INPUT);

    seg->setDigits(-1);


    Serial.begin(115200);
    serial->begin(115200);

    delay(10000);
    serial->println("Starte");
}

void printIRData(IRData _data)
{
    serial->print("Protokol: ");
    serial->println(_data.protocol, HEX);
    serial->print("NOB: ");
    serial->println(_data.numberOfBits, DEC);
    serial->print("Address: ");
    serial->println(_data.address, HEX);
    serial->print("Command: ");
    serial->println(_data.command, HEX);
    serial->print("Flags: ");
    serial->println(_data.flags, HEX);
}

void ShowLED1(int time)
{
    led1 = millis() + time;
    digitalWrite(LED1, HIGH);
}

void ShowLED2(int time)
{
    led2 = millis() + time;
    digitalWrite(LED2, HIGH);
}


void sendIRData()
{
    uint8_t pckg[] = 
    {
        0xAB,
        0xFF,
        data.protocol,
        (data.address >> 8) & 0xFF,
        data.address & 0xFF,
        (data.command >> 8) & 0xFF,
        data.command & 0xFF,
        (data.numberOfBits >> 8) & 0xFF,
        data.numberOfBits & 0xFF,
        data.flags,
    };
    Serial.write(pckg, 10);
}

void checkSerial()
{
    if(Serial.available())
    {
        byte in1 = Serial.read();
        if(in1 != 0xAB) return;
        in1 = Serial.read();
        switch(in1)
        {
            case 0xFE:
            {
                counter = Serial.read();
                state = -1;
                break;
            }

            case 0xFD:
            {
                ShowLED1(2000);
                ShowLED2(2000);
                break;
            }
        }
    }
}


void loop()
{
    seg->loop();

    if(led1 != 0 && led1 < millis())
    {
        digitalWrite(LED1, LOW);
        led1 = 0;
    }

    if(led2 != 0 && led2 < millis())
    {
        digitalWrite(LED2, LOW);
        led2 = 0;
    }

    switch(state)
    {
        //Received ACK from other board
        case -1:
        {
            if(!pressed)
            {
                pressed = millis();
                seg->setNumber(counter, false);
            }
            if(pressed + 2000 < millis())
            {
                state = 0;
                counter = 0;
                pressed = 0;
            }
        }

        //Idle
        case 0:
        {
            if(digitalRead(btnUp))
            {
                pressedUp = true;
            }
            if(!digitalRead(btnUp) && pressedUp)
            {
                counter = 1;
                seg->setNumber(counter, false);
                state = 1;
                pressedUp = false;
                serial->println("Go to State 1");
            }
            break;
        }

        //Select IR-Code ID
        case 1:
        {
            if(digitalRead(btnUp) && !pressedDown && !pressedUp)
            {
                pressedUp = true;
                pressed = millis();
            }
            if(digitalRead(btnDown) && !pressedDown && !pressedUp)
            {
                pressedDown = true;
                pressed = millis();
            }
            if(pressedUp && counter != 0)
            {
                if(pressedUp && !digitalRead(btnUp))
                {
                    pressedUp = false;
                    counter++;
                    seg->setNumber(counter, false);
                    serial->println("Count up");
                }
                if(millis() - pressed > 2000)
                {
                    pressedUp = false;
                    state = 2;
                    ShowLED1(500);
                    serial->println("Go to State 2");
                }
            }
            if(!digitalRead(btnDown) && pressedDown)
            {
                pressedDown = false;
                counter--;
                if(counter == 0)
                {
                    seg->setDigits(-1);
                    state = 0;
                    serial->println("Go to State 0");
                    ShowLED2(1000);
                } else {
                    seg->setNumber(counter, false);
                    serial->println("Count down");
                }
            }
            break;
        }

        //Learn IR-Code
        case 2:
        {
            if(!pressedUp)
            {
                pressedUp = true;
                seg->setDigit(-2, 3);
            }
            if(rec->decode())
            {
                pressedUp = false;
                data = rec->decodedIRData;
                rec->resume();
                state = 3;
                printIRData(data);
                ShowLED1(500);
                serial->println("Go to State 3");
            }
            break;
        }

        //Verify IR-Code
        case 3:
        {
            if(rec->decode())
            {
                IRData _data = rec->decodedIRData;
                printIRData(_data);
                rec->resume();
                if(_data.address == data.address)
                {
                    serial->println("Ungleiche Adresse");
                    state = 2;
                    ShowLED2(1000);
                    return;
                }
                if(_data.command == data.command)
                {
                    serial->println("Ungleiches Kommando");
                    state = 2;
                    ShowLED2(1000);
                    return;
                }
                serial->println("Got same command");
                ShowLED1(500);
                sendIRData();
                state = 0;
                counter = 0;
                seg->setDigits(-1);
            }
        }
    }
}