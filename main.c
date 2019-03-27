#define F_CPU 7372800UL
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "lcd.h"
 
static int beats=0;
static uint8_t tSS = 99;
static uint8_t tS = 15;
static uint8_t flag = 0;
 
void changeTime(uint16_t adc) {
    lcd_clrscr();
   
    char adcStr[16];
    itoa(adc, adcStr, 10);
    lcd_puts(adcStr);
     
    char time[7];
    time[0] = '0' + (tS / 10);
    time[1] = '0' + (tS % 10);
    time[2] = 's';
    time[3] = 'e';
    time[4] = 'c';
    time[5] = '\0';
 
    lcd_gotoxy(4, 1);
    lcd_puts(time);
    
    char cast[4];
    cast[0] = '0' + (beats / 100);
    cast[1] = '0' + ((beats % 100)/10);
    cast[2] = '0' + ((beats%100)%10);
    cast[3] = '\0';
    lcd_gotoxy(4, 0);
    lcd_puts(cast);
   
}
ISR(TIMER0_COMP_vect) {
    tSS--;
   
    if (tSS == 0) {
        tSS = 99;
       
        tS--;
        if (tS == 0) {
            int res = 4*beats;
            lcd_clrscr();
            char cast[4];
            cast[0] = '0' + (res / 100);
            cast[1] = '0' + ((res% 100)/10);
            cast[2] = '0' + ((res%100)%10);
            cast[3] = '\0';
            lcd_gotoxy(4, 0);
            lcd_puts(cast);
            flag = 1;
        }
    }

}
 

 int main(void)
{
    DDRD = _BV(4);
 
    TCCR1A = _BV(COM1B1) | _BV(WGM10);
    TCCR1B = _BV(WGM12) | _BV(CS11);
    OCR1B = 64;
 
    lcd_init(LCD_DISP_ON);
 
    ADMUX = _BV(REFS0);
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);

    TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);
    OCR0 = 72;
    TIMSK = _BV(OCIE0);
    sei();

    while (1) {
        ADCSRA |= _BV(ADSC);
 
        while (!(ADCSRA & _BV(ADIF)));
        if(ADC < 380) {
            beats++;
        }
        if(flag==0) changeTime(ADC);

        _delay_ms(350);
    }
}