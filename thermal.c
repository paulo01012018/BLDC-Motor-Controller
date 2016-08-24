#include "thermal.h"
#include "led.h"
#include "pins.h"

//TODO: recalculate 8-bit versions of these
const uint8_t operational_adc_reading 	= 128; // at ~60C
const uint8_t high_adc_reading 			= 52; // at ~110C
const uint8_t critical_adc_reading 		= 43; // at ~150C

ISR(TIMER0_COMPB_vect)
{
	static uint8_t thermal_count = 1;
	static uint8_t adc_to_sample = 0;

	if(thermal_count >= 7) {

		uint8_t reading = ADCH;

		switch(adc_to_sample) {
			case 0:
				adc_to_sample = 1;
				ADMUX &= ~0x0F;
				ADMUX |= 0x01;	//switches ADC multiplexer to ADC1
			case 1:
				adc_to_sample = 2;
				ADMUX &= ~0x0F;
				ADMUX |= 0x02; //switches ADC multiplexer to ADC2
			case 2:
				adc_to_sample = 0;
				ADMUX &= ~0x0F; //switches ADC multiplexer to ADC0
		}
		
		if(reading < critical_adc_reading) {
			set_flash_red();
		} else if(reading < high_adc_reading) {
			clear_flash_red();
			SET_LED_RED();
		} else if(reading < operational_adc_reading) {
			clear_flash_red();
			CLR_LED_RED();
			SET_LED_YELLOW();
		} else {
			clear_flash_red();
			CLR_LED_RED();
			CLR_LED_YELLOW();
		}
		
		thermal_count = 0;
	}
	thermal_count++;
}