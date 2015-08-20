#include "relay.h"

volatile unsigned long g_lastCrossTime = 0;

volatile uint16_t g_lastPeriod = 0;

volatile uint16_t g_numCrosses = 0;

volatile uint16_t g_numSwitches = 0;

volatile uint16_t g_ignoredPulses = 0;

#define TIME_SEMILOOP 10000 //10ms

#define TIME_GUARD 100

volatile uint16_t g_delayActivation = TIME_SEMILOOP - TIME_GUARD;
#define TIME_KEEP_ON 50
#define TIME_SAMPLE 10



#define RL_STATE_IDLE 0
#define RL_STATE_SAMPLE 1
#define RL_STATE_WAIT 2
#define RL_STATE_ACTIVE 3
#define RL_STATE_SAMPLE_N 4
#define RL_STATE_WAIT_N 5
#define RL_STATE_ACTIVE_N 6


volatile uint8_t g_relayState = RL_STATE_IDLE;

void relay_init(void)
{
	DDRD &= ~(1<<3);
	PORTD |= 1<<3;
	
	//int1 ena on falling edge
	MCUCR |= 1<<ISC11;
	GICR |= 1<<INT1;
	
	DDRB |= 1<<2;
	PORTB &= ~(1<<2);
}

void startTimer1(uint16_t cmpValue)
{
	TCCR1B |= 1<<CS11;
	TCNT1 = 0;
	OCR1A = cmpValue;
	TIMSK |= 1<<OCIE1A;
}

void stopTimer1(void)
{
	TCCR1B = 0;
}

ISR(TIMER1_COMPA_vect )
{
	stopTimer1();
	switch(g_relayState)
	{
		case RL_STATE_SAMPLE:
			//if(PIND & (1<<3))
			//{
				g_relayState = RL_STATE_WAIT;
				startTimer1(g_delayActivation);
			//}
			//else
			//{
			//	g_relayState = RL_STATE_IDLE;
			//}
		break;
		
		case RL_STATE_WAIT:
			PORTB |= (1<<2);
			startTimer1(TIME_KEEP_ON);
			++g_numSwitches;//more coorect is in the next state
			g_relayState = RL_STATE_ACTIVE;
		break;
		
		case RL_STATE_ACTIVE:
			PORTB &= ~(1<<2);
			g_relayState = RL_STATE_SAMPLE_N;
			startTimer1(TIME_SEMILOOP - TIME_KEEP_ON - g_delayActivation);
		break;
		
		case RL_STATE_SAMPLE_N:
			//if(PIND & (1<<3))
			//{
				g_relayState = RL_STATE_WAIT_N;
				startTimer1(g_delayActivation);
			//}
			//else
			//{
			//	g_relayState = RL_STATE_IDLE;
			//}
		break;
		
		case RL_STATE_WAIT_N:
			PORTB |= (1<<2);
			startTimer1(TIME_KEEP_ON);
			++g_numSwitches;//more correct is in the next state
			g_relayState = RL_STATE_ACTIVE_N;
		break;
		
		case RL_STATE_ACTIVE_N:
			PORTB &= ~(1<<2);
			g_relayState = RL_STATE_IDLE;
		break;		
		
		default:
			g_relayState = RL_STATE_IDLE;
		break;
	
	};
}

ISR(INT1_vect)
{
	if(g_relayState == RL_STATE_IDLE)
	{
		g_lastPeriod = us_x32() - g_lastCrossTime;
		
		g_lastCrossTime = us_x32();
		
		++g_numCrosses;
		
		g_relayState = RL_STATE_SAMPLE;
		startTimer1(TIME_SAMPLE);
	}
	else
	{
		++g_ignoredPulses;
	}
}

void relay_setDim(uint8_t dim)
{
	if(dim > 240)
		dim = 240;
	
/*	
	if(dim > 100 && dim < 156)
	{
		if(dim -100 < 156 - dim)
			dim = 100;
		else
			dim = 156;
	}
*/	

dim = 255 - dim;
	
while(g_relayState != RL_STATE_IDLE);
	
	g_delayActivation =  (((uint32_t)dim * (TIME_SEMILOOP - TIME_GUARD))) >> 8;
	
	//debugf("\ndim=%u -> %u ", dim, g_delayActivation);
}

//return ast period in multiple of 32 us
uint16_t relay_getLastPeriod(void)
{
	return g_lastPeriod;
}

uint16_t relay_getNumCrosses(void)
{
	return g_numCrosses;
}

uint16_t relay_getNumSwitches(void)
{
	return g_numSwitches;
}

uint16_t relay_getIgnoredPulses(void)
{
	return g_ignoredPulses;
}

