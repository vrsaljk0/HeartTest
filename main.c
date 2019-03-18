#define F_CPU 7372800
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include <stdio.h>

static uint8_t tSS;
static uint8_t tS;
static int flag;
static int beats;
static int start;

void init() {
	tSS = 99;
	tS = 15;
	flag = 0;
	beats = 0;
	start = 0;
}

void changeTime() {
	char time[6];
	
	time[0] = '0' + (tS / 10);
	time[1] = '0' + (tS % 10);
	time[2] = 's';
	time[3] = 'e';
	time[4] = 'c';
	time[5] = '\0';
	
	lcd_clrscr();
	lcd_gotoxy(4, 1);
	lcd_puts(time);
	
	char cast[2];
	cast[0] = '0' + (beats / 10);
	cast[1] = '0' + (beats % 10);
	cast[2] = '\0';
	lcd_gotoxy(4, 0);
	lcd_puts(cast);
}

ISR(TIMER0_COMP_vect) {
	if(start == 1) {
		tSS--;
		
		if (tSS == 0) {
			tSS = 99;
			
			tS--;
			if (tS == 0) {
				lcd_clrscr();
				lcd_gotoxy(0, 1);
				lcd_puts("kraj-pritisnite INT0 za poèetak");
				lcd_gotoxy(0, 0);
				int res = 4*beats;
				char puls[4];
				puls[0] = '0' + (res / 100);
				puls[1] = '0' + ((res % 100)/10);
				puls[2] = '0' + ((res%100)%10);
				puls[3] = '\0';
				lcd_puts(puls);
				flag = 1;
			}
			if(flag == 0) changeTime();
		}
	}
}

ISR(INT1_vect) { //RESET
	lcd_clrscr();
	lcd_gotoxy(0, 1);
	lcd_puts("PRITISNITE INT0 ZA POÈETAK");
	init();
}

ISR(INT0_vect) { //START
	init();
	start = 1;
}

int main(void) {
	DDRD = _BV(4);
	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 50;
	
	
	TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);
	OCR0 = 72;
	
	TIMSK = _BV(OCIE0);
	MCUCR = _BV(ISC01) |_BV(ISC00);
	GICR = _BV(INT0) |_BV(INT1);
	sei();
	
	init();
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_gotoxy(0, 1);
	lcd_puts("PRITISNITE INT0 ZA POÈETAK");
	
	while (1) {
		if(bit_is_clear(PINA, 0) && start == 1) {
			beats += 1;
			//PORTA = 0x00;
		}
		_delay_ms(200);
	}
}