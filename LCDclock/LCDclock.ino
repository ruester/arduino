/*
    LCDclock - LCDclock.ino

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

#include <Time.h>

#define TIME_MSG_LEN 11   /* time sync to PC is HEADER followed by Unix */
                          /* time_t as ten ASCII digits */
#define TIME_HEADER  'T'  /* header tag for serial time sync message */

/* example: T1262347200 --> noon Jan 1 2010 */

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
     * taken from http://www.sprut.de/electronic/lcd/index.htm */

    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW);
    lcd_send(HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
    lcd_send(LOW, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
    lcd_send(LOW, LOW, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, LOW);
}

void lcd_print(char *s)
{
    int i;

    for (i = 0; s[i]; i++)
        lcd_send(s[i] & 1,  s[i] & 2,  s[i] & 4,  s[i] & 8,
                 s[i] & 16, s[i] & 32, s[i] & 64, s[i] & 128,
                 HIGH, LOW);
}

void lcd_print_at(int x, int y, char *s)
{
    gotoxy(x, y);
    lcd_print(s);
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

/*
 * Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C":
 * see http://www.jb.man.ac.uk/~slowe/cpp/itoa.html for more details
 */
void strreverse(char* begin, char* end)
{
    char aux;

    while (end > begin) {
        aux      = *end;
        *end--   = *begin;
        *begin++ = aux;
    }
}

/* to avoid ambigious function names */
void my_itoa(long value, char *str, int base)
{
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr = str;
    unsigned long sign;

    /* Validate base */
    if (base < 2 || base > 35) {
        *wstr = '\0';
        return;
    }

    /* Take care of sign */
    if ((sign = value) < 0)
        value = -value;

    /* Conversion. Number is reversed. */
    do {
      *wstr++ = num[value%base];
    } while (value /= base);

    if (sign < 0)
        *wstr++ = '-';

    *wstr='\0';

    /* Reverse string */
    strreverse(str, wstr - 1);
}

void second_handler()
{
    print_time();
}

void check_second()
{
    static unsigned long prev = millis();
    unsigned long cur = millis();

    if (cur - prev < 1000)
        return;

    prev = cur;
    second_handler();
}

void setup()
{
    Serial.begin(9600);
    init_pin_modes();
    lcd_init();
    set_display(ON, OFF, OFF);
}

void loop()
{
    if (Serial.available() >= TIME_MSG_LEN)
        read_sync_message();

    if (timeStatus() == timeNotSet) {
        Serial.println("waiting for sync message");
        delay(1000);
    }

    check_second();
}

#define LEADING_ZERO(a) do { if (!(a)[1]) { \
                                 (a)[2] = '\0'; \
                                 (a)[1] = (a)[0]; \
                                 (a)[0] = '0'; \
                             } \
                        } while (0)

void print_time()
{
    static int h = -1, m  = -1, s = -1,
               d = -1, mo = -1, y = -1;
    int h_n, m_n,  s_n,
        d_n, mo_n, y_n;
    char str[5];

    h_n = hour();
    m_n = minute();
    s_n = second();

    if (h_n != h) {
        h = h_n;
        my_itoa(h, (char *) str, 10);
        LEADING_ZERO(str);
        lcd_print_at(0, 0, str);
        lcd_print_at(2, 0, ":");
    }

    if (m_n != m) {
        m = m_n;
        my_itoa(m, (char *) str, 10);
        LEADING_ZERO(str);
        lcd_print_at(3, 0, str);
        lcd_print_at(5, 0, ":");
    }

    if (s_n != s) {
        s = s_n;
        my_itoa(s, str, 10);
        LEADING_ZERO(str);
        lcd_print_at(6, 0, str);
    }

    d_n  = day();
    mo_n = month();
    y_n  = year();

    if (d_n != d) {
        d = d_n;
        my_itoa(d, str, 10);
        LEADING_ZERO(str);
        lcd_print_at(0, 1, str);
        lcd_print_at(2, 1, ".");
    }

    if (mo_n != mo) {
        mo = mo_n;
        my_itoa(mo, str, 10);
        LEADING_ZERO(str);
        lcd_print_at(3, 1, str);
        lcd_print_at(5, 1, ".");
    }

    if (y_n != y) {
        y = y_n;
        my_itoa(y, str, 10);
        LEADING_ZERO(str);
        lcd_print_at(6, 1, str);
    }
}

void read_sync_message()
{
    int i;
    char c;

    c = Serial.read();

    if (c != TIME_HEADER)
        return;

    time_t pctime = 0;

    for (i = 0; i < TIME_MSG_LEN - 1; i++) {
        c = Serial.read();

        if (c < '0' || c > '9')
            continue;

        pctime = (10 * pctime) + (c - '0');
    }

    setTime(pctime);
    Serial.println("time set");
}
