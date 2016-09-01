//#include "main.h"
#include "pins.h"
#include "led.h"
#include "thermal.h"
#include "motor_driver.h"
#include "motor_states.h"
#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//static const uint8_t 	BLINK_DELAY_MS = 15;
//static const uint8_t	MAX_TIME_IN_STATE_MS = 10;

#define REPEAT while(true) 

//TODO: implement task prioritization (i.e. switching motor state is always highest)
//TODO: control messages, actual motor controller logic
void initialize() 
{
	//Set up PORTB
	DDRB = 0x3D; 	//set bits 0, 2,3,4, and 5 on PORTB as output, clear all others
	PORTB |= ~0x3D;	//enable pull-up resisors for pins 1, 6-7
	PORTB &= ~0x3D;	//clears bits 0,2,3,4,5

	//Set up PORTC
	DDRC &= ~0x3F; 	//set bits 0-5 on PORTC as input
	PORTC &= ~0x38; //disable pull-up resistors for bits 3-5
	PORTC |= 0x07;	//enable pull-up resistors for bits 0-2

	//Set up PORTD
	DDRD = 0xFC;	//set bits 2-7 on PORTD as output, clear 0,1 
	PORTD |= 0x0F;	//enable pull-up resistors at pins 0,1,2,3
	PORTD &= ~0xF0;	//clear bits 4-7

	//Set up motor driver
	init_motor_driver();

	//Set up thermal monitoring
	init_thermal();

	//Set up UART
	init_UART(UBRR_UART);

	//Task scheduler timer (Used for LED flashing and thermal monitoring)
	TCCR0A |= (1 << WGM01); // Configure timer 0 for CTC mode
	OCR0A = 255; // for LED; delay of ~114 ms after additional delay from the counter in the ISR
	TIMSK0 |= (1 << OCIE0A);  // for LED; Enable CTC interrupt (TIMER0_COMPA_vect)
	TCCR0B |= ((1 << CS02) | (1 << CS00)); // Start timer 0 at Fcpu/1024
	//The value of this timer is stored in TCNT0

	//TODO: complete motor state timer implementation

	//Reset timers

	sei();	//global enable interrupts
}

int main() {

	initialize();

	motor_off = false;

	set_flash_green();
	//set_flash_red();
	//set_flash_yellow();
	
	startup_f1();

	/*
	UART_enqueue('1');
	UART_enqueue('2');
	UART_enqueue('3');
	UART_enqueue('4');
	UART_enqueue('5');
	UART_enqueue('6');
	UART_enqueue('7');
	UART_enqueue('8');
	UART_enqueue('9');
	UART_enqueue('a');
	UART_enqueue('b');
	UART_enqueue('c');
	UART_enqueue('d');
	UART_enqueue('e');
	UART_enqueue('f');
	*/
	
	REPEAT {
		//TOG_LED_YELLOW();
		//_delay_ms(BLINK_DELAY_MS);

		//UDR0 = 'a';
		//_delay_ms(500);
		
		//ACTUAL REPEATED CODE

		/*
		unsigned char just_read = UART_read();
		if(just_read == 'p'){
			TOG_LED_GREEN();
		}
		if(just_read == ' '){
			TOG_LED_RED();
		}
		_delay_ms(50);
		UART_write('\n');
		UART_write(just_read);
		*/
		//UART_write_flush();
		//UART_enqueue('z');
		//UART_write();
		if(back_emf_zero_crossing_flag){
			change_motor_state();
			back_emf_zero_crossing_flag = false;
		}

		if(flash_leds_flag){
			flash_leds();
			flash_leds_flag = false;
		}

		if(rx_overflow_flag){
			UART_write_urgent('\n');
			UART_write();
			while(rx_queue_length > 0){
				UART_enqueue(UART_read());
			}
			UART_write_flush();
			rx_overflow_flag = false;
		}

		if(sample_gate_temperatures_flag) {
			sample_gate_temperatures();
			sample_gate_temperatures_flag = false;
		}

		if(motor_emergency_stop_flag) {
			//Note that lower value means hotter
			if(hottest_adc_reading >= high_adc_reading){
				clear_flash_red();
				CLR_LED_RED();
				motor_emergency_stop_flag = false;
				EIMSK |= (1 << INT1);
			}
		}
		
	}

	return 0;
}

//Periodically requests for thermal sampling and LED toggling (for flashing)
ISR(TIMER0_COMPA_vect)
{
	static uint8_t cycle_count = 1;
	
	if(cycle_count == 3){
		sample_gate_temperatures_flag = true;
	}

	if(cycle_count >= 6){
		flash_leds_flag = true;
		cycle_count = 0;
	}

	cycle_count++;
}