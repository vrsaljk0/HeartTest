#define F_CPU 7372800UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcd.h"	 //User defined UART library which contains the UART routines
#include <avr/eeprom.h>  //User defined library which contains eeprom routines

static int beats = 0;
static uint8_t tSS = 99;
static uint8_t tS = 5;
static uint8_t flag = 0;
static uint8_t start = 0;
static int last_time = 0;

void print(int num, int position1, int position2){ //funkcija za castanje integera i ispis na LCDU
	char cast[4];
	cast[0] = '0' + (num / 100);
	cast[1] = '0' + ((num % 100)/10);
	cast[2] = '0' + ((num%100)%10);
	cast[3] = '\0';
	lcd_gotoxy(position1, position2);
	lcd_puts(cast);
}

void changeTime() { //prikaz stoperice i trenutnog stanja beatsa
	char time[6];

	time[0] = '0' + (tS / 10);
	time[1] = '0' + (tS % 10);
	time[2] = 's';
	time[3] = 'e';
	time[4] = 'c';
	time[5] = '\0';

	lcd_clrscr();
	lcd_gotoxy(3, 1);
	lcd_puts("Vrijeme:");
	lcd_gotoxy(11, 1);
	lcd_puts(time);
	
	lcd_gotoxy(9, 0);
	lcd_puts("BPM:");
	print(beats, 13, 0);
}

ISR(TIMER0_COMP_vect) { //timer0 za štopericu m/s/ms
	if(start == 1){
		tSS--;
	
		if (tSS == 0) {
			tSS = 99;
		
			tS--;
		
			if (tS == 0) {
				last_time = eeprom_read_word(0);
				int res = 4*beats;	
				lcd_clrscr();
				lcd_gotoxy(0, 1);
				lcd_puts("Trenutni BPM:");
				lcd_gotoxy(0, 0);
				lcd_puts("Prethodni:");
				print(last_time, 13, 0);
				print(res, 13, 1);
				flag = 1;
				eeprom_write_word(0, res);
				cli();
				start = 0;
			}
		}
		if(flag == 0) changeTime();
	}
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
	if(start == 1){
		beats++;
		debounce();	
	}
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
	
	PORTB = _BV(3) | _BV(2);
	DDRB = 0b11001111;
	
	lcd_init(LCD_DISP_ON);
	lcd_gotoxy(0, 0);
	lcd_puts("Pritisnite tipku za start");
	lcd_gotoxy(0, 1);
	lcd_puts("za start");
	sei();
	while (1){
		if(bit_is_clear(PINB, 3)) { //reset mjerenja
			start = 1;
			beats = 0;
			tSS = 99;
			tS = 15;
			flag = 0;
			sei();
			lcd_clrscr();
			_delay_ms(100);
		}
		if(bit_is_clear(PINB, 2)){ //detaljnije
			lcd_clrscr();
			
			if((beats*4) < 65){
				lcd_puts("Usporen puls");
			}
			else if((beats*4) >= 65 && (beats*4) <= 90){
				lcd_puts("Normalan puls");
			}
			else{
				lcd_puts("Ubrzan puls");
			}
			
			float prosjek =(((float)(beats*4)/(float)last_time)-1)*100;
			
			if (prosjek < 0) {
				lcd_gotoxy(0, 1);
				lcd_puts("Sporiji za: ");
				prosjek *= -1;
				int rjs = round(prosjek);
				print(rjs, 12, 1);
				lcd_gotoxy(15, 1);
				lcd_puts("%");
			} else if (prosjek > 0){
				lcd_gotoxy(0, 1);
				lcd_puts("Brzi za: ");
				int rjs = round(prosjek);
				lcd_gotoxy(12, 1);
				lcd_puts("%");
				print(rjs, 9, 1);
			} else if (prosjek == 0) {
				lcd_gotoxy(0, 1);
				lcd_puts("Puls je jednak");
			}
		
		}
	}
}