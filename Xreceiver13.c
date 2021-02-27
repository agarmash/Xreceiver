#define F_CPU 1000000L
#include <avr/io.h>
#include <util/delay.h>

//pb1 - sync button
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


void sendToRFModule (unsigned char received)
{
    char prev = 0xFF;
    DATA_PORT &= ~(1<<DATA); //set data line 0 to begin data transmission
    for(int i = 0; i < 11; i++) //first 2 bytes of sending data are always 0, the rest 8 are the actual command
    {
        while (prev == (CLOCK_PIN & (1<<CLOCK))){} //detects change in clock 
        prev = CLOCK_PIN & (1<<CLOCK); // should be after downward edge of clock, so send bit of data now 
        if (i < 2)
		{
            DATA_PORT &= ~(1<<DATA); //set 0
		}
		else
		{
            if (received >> 7) //get highest bit of data
			{
			    DATA_PORT |= 1<<DATA; //set 1
			}
			else
			{
                DATA_PORT &= ~(1<<DATA); //set 0
			}
	    	received = received << 1; //shift data 1 bit left so we can get next bit during next iteration
		}

        while (prev == (CLOCK_PIN & (1<<CLOCK))){} //detects upward edge of clock 
        prev = CLOCK_PIN & (1<<CLOCK);
    }
    DATA_PORT |= 1<<DATA; //set 1
}

int main(void)
{

    DATA_DDR |= 1<<DATA; //configure as output
    CLOCK_DDR &= ~(1<<CLOCK); //configure as input
	SYNC_DDR &= ~(1<<SYNC); //configure as input


    DATA_PORT |= 1<<DATA; //set 1

    _delay_ms(3500); //RF module does some internal initialization after power up, it's good to give it some time for this

	sendToRFModule(0x84); //inits module and turns on central led

	int timer;
	int flag;

	while(1)
	{
        if ((SYNC_PIN & (1<<SYNC)) == 0) //check if button is pressed
		{
		    flag = 1;

            for (timer = 0; (timer < 400) && (flag); timer++)
			{
			   if (SYNC_PIN & (1<<SYNC)) //if button has been held less than ~800ms
			   {
                   sendToRFModule(0x04); //send command to start syncronization
				   flag = 0;
			   }
               _delay_ms(2);
			}
			if (flag) //else if button has been held more than ~800ms
			{
                sendToRFModule(0xA9);
				_delay_ms(4);
				sendToRFModule(0xB6); //turn on green-red ring of light
				_delay_ms(4);
				sendToRFModule(0x09); //send command to disable all controllers
				_delay_ms(500);
                sendToRFModule(0xA0);
				_delay_ms(4);
				sendToRFModule(0xB0); //turn off ring of light
			}
			_delay_ms(2000); //and wait ~2s to exclude false actions
		}
	}
}
