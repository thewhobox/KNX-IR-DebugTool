#include <Arduino.h>
#include <SendOnlySoftwareSerial.h>

#include "IRremote.h"
#include "Segment.h"
#include "pins.h"

SendOnlySoftwareSerial *serial = new SendOnlySoftwareSerial(PIN_D6);
Segment *seg;
IRrecv *rec;

#define btnUp PIN_D13
#define btnDown PIN_D14
#define btnSelect PIN_D11
int counter;
int state;
bool pressedUp;
bool pressedDown;
bool pressedSelect;
unsigned long pressed;
unsigned long led1;
unsigned long led2;
uint8_t pinsIn[] = { PIN_D1, PIN_D4, PIN_D3, PIN_D2 };
uint8_t pinsDig[] = { PIN_DI3, PIN_DI1, PIN_DI2, PIN_DI4 };
IRData data;

void setup()
{
    seg = new Segment(4);
    rec = new IRrecv();

    seg->setInputPins(pinsIn);
    seg->setDigitPins(pinsDig);
    seg->setSerial(serial);

    pinMode(btnUp, INPUT);
    pinMode(btnDown, INPUT);
    pinMode(PIN_D7, OUTPUT);
    pinMode(PIN_D8, OUTPUT);
    pinMode(PIN_D9, INPUT);

    seg->setNumber(-1);


    Serial.begin(115200);
    serial->begin(115200);

    delay(5000);
    serial->println("Starte");
    seg->setDigits(-1);

    rec->begin(PIN_D9);
    rec->enableIRIn();
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
    digitalWrite(PIN_D7, HIGH);
}

void ShowLED2(int time)
{
    led2 = millis() + time;
    digitalWrite(PIN_D8, HIGH);
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
        (data.extra >> 8) & 0xFF,
        data.extra & 0xFF,
        data.flags,
        0xAC
    };
    Serial.write(pckg, 13);
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
        digitalWrite(PIN_D7, LOW);
        led1 = 0;
    }

    if(led2 != 0 && led2 < millis())
    {
        digitalWrite(PIN_D8, LOW);
        led2 = 0;
    }

    switch(state)
    {
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
                ShowLED1(200);
            }
            break;
        }

        case 1:
        {
            if(digitalRead(btnSelect) && !pressedUp && !pressedDown && !pressedSelect)
            {
                pressedSelect = true;
                pressed = millis();
            }
            if(digitalRead(btnUp) && !pressedDown && !pressedUp && !pressedSelect)
            {
                pressedUp = true;
            }
            if(digitalRead(btnDown) && !pressedDown && !pressedUp && !pressedSelect)
            {
                pressedDown = true;
            }
            if(pressedSelect && pressed + 4000 < millis())
            {
                pressedSelect = false;
                state = 2;
                ShowLED1(200);
                serial->println("Go to State 2");
            }
            if(pressedUp && !digitalRead(btnUp))
            {
                pressedUp = false;
                counter++;
                seg->setNumber(counter, false);
                serial->print("Count up: ");
                serial->println(counter);
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
                    serial->print("Count down: ");
                    serial->println(counter);
                }
            }
            break;
        }

        case 2:
        {
            if(!pressedUp)
            {
                pressedUp = true;
                seg->setDigit(-2, 3);
                pressed = millis();
            }
            if(rec->decode())
            {
                rec->resume();
                if(millis() - pressed < 100)
                    return;
                pressed = millis();
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

        case 3:
        {
            if(rec->decode())
            {
                rec->resume();
                if(pressed + 500 > millis())
                    return;
                    
                if(!pressedUp)
                {
                    pressedUp = true;
                    return;
                }
                IRData _data = rec->decodedIRData;
                printIRData(_data);
                if(_data.address != data.address)
                {
                    serial->println("Ungleiche Adresse");
                    serial->print(data.address);
                    serial->print(" ");
                    serial->println(_data.address);
                    state = 2;
                    ShowLED2(1000);
                    return;
                }
                if(_data.command != data.command)
                {
                    serial->println("Ungleiches Kommando");
                    state = 2;
                    ShowLED2(1000);
                    return;
                }
                serial->println("Got same command");
                ShowLED1(1000);
                sendIRData();
                state = 0;
                counter = 0;
                seg->setDigits(-1);
                pressedUp = false;
            }
        }
    }
}