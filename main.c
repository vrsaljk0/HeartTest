#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcd.h"

static int beats=0;

void writeLCD(uint16_t adc) {
    lcd_clrscr();

    char adcStr[16];
    itoa(adc, adcStr, 10);
    lcd_puts(adcStr);
	
	char cast[4];
	cast[0] = '0' + (beats / 100);
	cast[1] = '0' + ((beats % 100)/10);
	cast[2] = '0' + ((beats%100)%10);
	cast[3] = '\0';
	lcd_gotoxy(4, 1);
	lcd_puts(cast);
	
	
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

	while (1) {
		ADCSRA |= _BV(ADSC);

		while (!(ADCSRA & _BV(ADIF)));
		if(ADC < 350) {
			beats++;
		}
		writeLCD(ADC);

		_delay_ms(300);
	}
}
