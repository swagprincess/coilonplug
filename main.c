#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

volatile uint16_t delayBeforeLow = 125; //ticks at 8mhz prescaler 256, 4ms base pulse
volatile int16_t tempDelayCalc = 0;
volatile uint16_t startTime14 = 0;
volatile uint16_t startTime23 = 0;
volatile bool lastStatePB0 = false;
volatile bool lastStatePB1 = false;
volatile bool set23Low = false;
volatile bool set14Low = false;


ISR(PCINT1_vect){ // interrupt for all port b pins

    if (!(PINB & (1 << PB1)) && lastStatePB1){
        // PB1 is LOW, turn off PA2 (1 & 4)
        PORTA |= (1 << PA2); // set high for off

        tempDelayCalc = TCNT1 - startTime14 - 75; //2.4ms setting

        if (tempDelayCalc <= 0){
            delayBeforeLow = tempDelayCalc;
        } else {
            delayBeforeLow = 0;
        }

        lastStatePB1 = false;

    } else if ((PINB & (1 << PB1)) && !lastStatePB1) {   //IGN 1 & 4
        // PB1 is HIGH
        startTime14 = TCNT1; // set timer timestamp

        set14Low = true; // mark pin as has to go low eventually 

        lastStatePB1 = true;

    }


    if (!(PINB & (1 << PB0)) && lastStatePB0){
        // PB0 is LOW, turn off PA1 (2 & 3)
        PORTA |= (1 << PA1); // set high for off

        tempDelayCalc = TCNT1 - startTime14 - 75; //2.4ms setting

        if (tempDelayCalc <= 0){
            delayBeforeLow = tempDelayCalc;
        } else {
            delayBeforeLow = 0;
        }

        lastStatePB0 = false;

    } else if ((PINB & (1 << PB0)) && !lastStatePB0) {   //IGN 2 & 3
        // PB0 is HIGH 
        startTime23 = TCNT1; // set timer timestamp

        set23Low = true; // mark pin as has to go low eventually 

        lastStatePB0 = true;

    }
     
}


int main() {

    //set output pins as output and set high
    DDRA |= ((1 << DDA1) | (1<< DDA2));
    PORTA |= ((1 << PA1) | (1 << PA2));

    //set input pins as input
    DDRB &= ~((1 << DDB0) | (1 << DDB1));

    TCCR1B = 0; // stop timer

    TCCR1B |= (1 << CS12); // start timer at prescaler 256

    GIMSK |= (1 << PCIE1); //enable group 1 pin interrupts
    PCMSK1 |= (1 << PCINT8) | (1 << PCINT9); // allow pins to trigger interrupt

    sei(); //enable interrupts


    while (1){
        if (set23Low){
            if ((uint16_t)(TCNT1 - startTime23) >= delayBeforeLow){
                PORTA &= ~(1 << PA1); // set low to start trigger
                set23Low = false;
            }
        }
        if (set14Low){
            if ((uint16_t)(TCNT1 - startTime14) >= delayBeforeLow){
                PORTA &= ~(1 << PA2); // set low to start trigger
                set14Low = false;
            }
        }
    }

}