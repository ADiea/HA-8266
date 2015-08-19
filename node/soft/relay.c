#include "relay.h"

volatile unsigned long g_lastCrossTime = 0;

volatile uint16_t g_lastPeriod = 0;

volatile uint16_t g_numCrosses = 0;

volatile uint16_t g_numSwitches = 0;


volatile uint16_t g_delayActivation = 2*8*1000;
#define TIME_KEEP_ON 400
#define TIME_SAMPLE 40

#define RL_STATE_IDLE 0
#define RL_STATE_SAMPLE 1
#define RL_STATE_WAIT 2
#define RL_STATE_ACTIVE 3



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
	TCCR1B |= 1<<CS10;
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

