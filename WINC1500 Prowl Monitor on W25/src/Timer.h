/*
 * Timer.h
 *
 * Created: 7/24/2016 10:49:47 PM
 *  Author: Brad
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include "asf.h"

//enable TC3
#define CONF_TC_MODULE TC3

void configure_tc(void);
void configure_tc_callbacks(void);
void tc_callback(struct tc_module *const module_inst);

#endif /* TIMER_H_ */