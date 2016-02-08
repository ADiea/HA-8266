#ifndef _MAIN__H
#define _MAIN__H

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <math.h>

#include "uart.h"
#include "radio.h"
#include "key.h"
#include "ws2812.h"
#include "spi.h"
#include "systimer.h"
#include "led.h"
#include "relay.h"
#include "light.h"
#include "heater.h"
#include "../../protocol/radio_protocol.h"

#define NODE_LIGHT 0
#define NODE_THSENSOR 1
#define NODE_HEATER 2

#define NODETYPE NODE_HEATER
#define HAS_RGBLED 0

#define MY_ID 0x01

extern uint16_t g_LedStateInterval;

#endif /*_MAIN__H*/