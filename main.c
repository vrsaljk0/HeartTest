#define F_CPU 7372800UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"

static int beats = 0;
static uint8_t tSS = 99;
static uint8_t tS = 5;
static uint8_t flag = 0;

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
	print(beats);
}

ISR(TIMER0_COMP_vect) {
	tSS--;
	
	if (tSS == 0) {
		tSS = 99;
		
		tS--;
		
		if (tS == 0) {
			int res = 4*beats;
			lcd_clrscr();
			print(res);
			flag = 1;
		}
		if(flag == 0) changeTime();
	}
}

void print(int num){
	char cast[4];
	cast[0] = '0' + (num / 100);
	cast[1] = '0' + ((num % 100)/10);
	cast[2] = '0' + ((num%100)%10);
	cast[3] = '\0';
	lcd_gotoxy(4, 0);
	lcd_puts(cast);
}
void debounce(){
	GICR &= ~_BV(INT1);
	sei();
	_delay_ms(500);
	GIFR = _BV(INTF1);
	GICR |= _BV(INT1);
	cli();
}

ISR(INT1_vect) {
	beats++;
	debounce();
}

int main(void) {
	DDRD = _BV(4);
	TCCR1A = _BV(WGM10) | _BV(COM1B1);
	TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10);
	OCR1B = 50;
	
	TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);
	OCR0 = 72;
	TIMSK = _BV(OCIE0);
	
	MCUCR = _BV(ISC10) | _BV(ISC11);
	GICR = _BV(INT1);
	
	lcd_init(LCD_DISP_ON);
	//print();
	sei();

	while (1);
}
