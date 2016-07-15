#define F_CPU 1000000L
#include <avr/io.h>
#include <util/delay.h>


//pb1 - sync and interruption
//pb0 - clock
//pb2 - data

#define DATA 2
#define CLOCK 0
#define SYNC 1
#define DATA_PORT PORTB
#define DATA_DDR DDRB
#define CLOCK_PIN PINB
#define CLOCK_DDR DDRB
#define SYNC_PIN PINB
#define SYNC_DDR DDRB

char prev;
int i;

void sendToRFModule (unsigned char recieved)
{
    prev = 0xFF;
    DATA_PORT &= ~(1<<DATA); //установка 0
    for(i = 0; i < 11; i++)
    {
        while (prev == (CLOCK_PIN & (1<<CLOCK))){} //detects change in clock 
        prev = CLOCK_PIN & (1<<CLOCK); // should be after downward edge of clock, so send bit of data now 
        if (i < 2)
		{
            DATA_PORT &= ~(1<<DATA); //установка 0
		}
		else
		{
            if (recieved >> 7)
			{
			    DATA_PORT |= 1<<DATA; //установка 1
			}
			else
			{
                DATA_PORT &= ~(1<<DATA); //установка 0
			}
	    	recieved = recieved << 1;
		}

        while (prev == (CLOCK_PIN & (1<<CLOCK))){} //detects upward edge of clock 
        prev = CLOCK_PIN & (1<<CLOCK);
    }
    DATA_PORT |= 1<<DATA; //установка 1
}

int main(void)
{

    DATA_DDR |= 1<<DATA;   //gpio init
    CLOCK_DDR &= ~(1<<CLOCK); //gpio init
	SYNC_DDR &= ~(1<<SYNC); //gpio init


    DATA_PORT |= 1<<DATA; //установка 1

    _delay_ms(3500);

	sendToRFModule(0x84);

	int timer;
	int flag;

	while(1)
	{
        if ((SYNC_PIN & (1<<SYNC)) == 0)
		{
		    flag = 1;

            for (timer = 0; (timer < 400) && (flag); timer++)
			{
			   if (SYNC_PIN & (1<<SYNC))
			   {
                   sendToRFModule(0x04);
				   flag = 0;
			   }
               _delay_ms(2);
			}
			if (flag)
			{
                sendToRFModule(0xA9);
				_delay_ms(4);
				sendToRFModule(0xB6);
				_delay_ms(4);
				sendToRFModule(0x09);
				_delay_ms(500);
                sendToRFModule(0xA0);
				_delay_ms(4);
				sendToRFModule(0xB0);
			}
			_delay_ms(2000);
		}
	}
}
