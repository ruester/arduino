/*
    HD44780 - HD44780.ino

    Copyright (C) 2012 Matthias Ruester

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define DEBUG

#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

#define RS 12
#define RW 11
#define ENABLE 10

#define ON HIGH
#define OFF LOW

#define INTERFACE_4BIT LOW
#define INTERFACE_8BIT HIGH

#define ONE_LINE LOW
#define TWO_LINES HIGH

#define FIVE_X_EIGHT LOW
#define FIVE_X_ELEVEN HIGH

void reset()
{
    digitalWrite(D0, LOW);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D3, LOW);
    digitalWrite(D4, LOW);
    digitalWrite(D5, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(D7, LOW);
    digitalWrite(RS, LOW);
    digitalWrite(RW, LOW);
    digitalWrite(ENABLE, LOW);
}

void lcd_send(int d0, int d1, int d2, int d3,
              int d4, int d5, int d6, int d7,
              int rs, int rw)
{
    int b;

    b = 0;

    /* read busy flag */
    pinMode(D7, INPUT);

    digitalWrite(D0, LOW);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D3, LOW);
    digitalWrite(D4, LOW);
    digitalWrite(D5, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(RS, LOW);
    digitalWrite(RW, HIGH);

    do {
        digitalWrite(ENABLE, LOW);
        digitalWrite(ENABLE, HIGH);

        b = digitalRead(D7);

#ifdef DEBUG
        if (b == 1)
            Serial.println("I HAVE TO WAIT");
#endif
    } while (b == 1);

    digitalWrite(ENABLE, LOW);

    pinMode(D7, OUTPUT);

    digitalWrite(D0, d0);
    digitalWrite(D1, d1);
    digitalWrite(D2, d2);
    digitalWrite(D3, d3);
    digitalWrite(D4, d4);
    digitalWrite(D5, d5);
    digitalWrite(D6, d6);
    digitalWrite(D7, d7);
    digitalWrite(RS, rs);
    digitalWrite(RW, rw);
    digitalWrite(ENABLE, HIGH);
    digitalWrite(ENABLE, LOW);
}

void clear_display()
{
    reset_cursor();
    lcd_send(HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
}

void set_display(int display, int cursor, int blink)
{
    lcd_send(blink, cursor, display, HIGH, LOW, LOW, LOW, LOW, LOW, LOW);
}

void function_set(int interface, int lines, int font)
{
    lcd_send(LOW, LOW, font, lines, interface, LOW, LOW, LOW, LOW, LOW);
}

void reset_cursor()
{
    lcd_send(LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
}

void lcd_init()
{
    /* initialization routine
       taken from http://www.sprut.de/electronic/lcd/index.htm
    */
    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW);
    lcd_send(HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
    lcd_send(LOW, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, LOW);

#ifdef DEBUG
    set_display(ON, ON, ON);
#endif
}

void lcd_print(String s)
{
    int i;

    for (i = 0; s[i]; i++)
        lcd_send(s[i] & 1,  s[i] & 2,  s[i] & 4,  s[i] & 8,
                 s[i] & 16, s[i] & 32, s[i] & 64, s[i] & 128,
                 HIGH, LOW);
}

void gotoxy(int x, int y)
{
    if (x >= 40)
        return;

    if (y)
        x |= 0b01000000;

    x |= 0b10000000;

    lcd_send(x & 1,  x & 2,  x & 4,  x & 8,
             x & 16, x & 32, x & 64, x & 128,
             LOW, LOW);
}

void init_pin_modes()
{
    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
    pinMode(D4, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    pinMode(RS, OUTPUT);
    pinMode(RW, OUTPUT);
    pinMode(ENABLE, OUTPUT);
}

void setup()
{
     Serial.begin(9600);

     init_pin_modes();

     lcd_init();
}

#define MODE_COUNT 1

void loop()
{
    static int mode = 0;

    if (mode == 0)
        lcd_print("Hello World!");

    mode++;

    /* END */
    while (mode == MODE_COUNT) {
        /* heartbeat */
        pinMode(13, OUTPUT);
        digitalWrite(13, HIGH);
        delay(200);
        digitalWrite(13, LOW);
        delay(200);
        digitalWrite(13, HIGH);
        delay(200);
        digitalWrite(13, LOW);
        delay(1000);
    }
}
