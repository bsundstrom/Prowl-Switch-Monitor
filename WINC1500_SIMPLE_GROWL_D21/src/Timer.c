/*
 * Timer.c
 *
 * Created: 7/24/2016 10:48:48 PM
 *  Author: Brad
 */ 
#include "Timer.h"
#include "main.h"

struct tc_module tc_instance;

void tc_callback(struct tc_module *const module_inst)
{
	TimerCallback();
}

void configure_tc(void)
{
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);
	config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc.clock_source = GCLK_GENERATOR_1;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV256;
	config_tc.counter_8_bit.period = 32; //This sets interrupt time - 128cnts/sec - 32 =.25sec
	config_tc.counter_8_bit.compare_capture_channel[0] = 50;
	config_tc.counter_8_bit.compare_capture_channel[1] = 99;

	tc_init(&tc_instance, CONF_TC_MODULE, &config_tc);
	tc_enable(&tc_instance);
}

void configure_tc_callbacks(void)
{
	tc_register_callback(&tc_instance, tc_callback, TC_CALLBACK_OVERFLOW);
	//tc_register_callback(&tc_instance, tc_callback, TC_CALLBACK_CC_CHANNEL0);
	//tc_register_callback(&tc_instance, tc_callback, TC_CALLBACK_CC_CHANNEL1);
	tc_enable_callback(&tc_instance, TC_CALLBACK_OVERFLOW);
	//tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
	//tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
}