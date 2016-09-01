#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

//#include "main.h"
#include "simple_moving_average.h"

#include <stdint.h>
#include <stdbool.h>

#define MOTOR_STATE_CYCLES_PER_FULL_ROTATION 4

volatile bool motor_off; //had extern
volatile bool motor_forwards; //had extern
volatile uint8_t expected_motor_state; //had extern
volatile bool motor_emergency_stop_flag;

volatile bool back_emf_zero_crossing_flag;
volatile uint8_t back_emf_PORTC_state;

Simple_moving_average *sum_of_time_between_states;

void init_motor_driver();

void cycle_f(unsigned us100);

void cycle_b(unsigned us100);

void simple1();

bool startup_f1();

void change_motor_state();

uint16_t get_rotations_per_second();

#endif
