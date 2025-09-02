#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

#define true 1
#define false 0

volatile uint16_t delayBeforeLow = 40; //ticks at 8mhz prescaler 256, assume 5ms base pulse and 3.5ms ish for first pulse
volatile int16_t tempDelayCalc = 0;
volatile uint16_t startTime14 = 0;
volatile uint16_t startTime23 = 0;
volatile uint16_t timeToGoLow23 = 0;
volatile uint16_t timeToGoLow14 = 0;
volatile uint8_t pulses = 0;
volatile uint8_t lastStatePB0 = false;
volatile uint8_t lastStatePB1 = false;
volatile uint8_t set23Low = false;
volatile uint8_t set14Low = false; 

ISR(PCINT1_vect){ // interrupt for all port b pins

    if (!(PINB & (1 << PB1)) && lastStatePB1){    //IGN 1 & 4
        // PB1 is LOW, turn off PA2 (1 & 4)
        PORTA |= (1 << PA2); // set high for off

        tempDelayCalc = (uint16_t)(TCNT1 - startTime14) - 70; //2.2ms
        if (pulses <= 20){
            tempDelayCalc = (uint16_t)(TCNT1 - startTime14) - 109; // 3.5ms
            pulses = pulses + 1;
        }

        if (tempDelayCalc >= 0){
            delayBeforeLow = tempDelayCalc;
        } else {
            delayBeforeLow = 0;
        }
        lastStatePB1 = false; // false
        


    } else if ((PINB & (1 << PB1)) && !lastStatePB1) {   //IGN 1 & 4
        // PB1 is HIGH
        startTime14 = (uint16_t)TCNT1; // set timer timestamp

        timeToGoLow14 = (uint16_t)(startTime14 + delayBeforeLow);

        set14Low = true; // mark pin as has to go low eventually 

        lastStatePB1 = true; // true

    }



    if (!(PINB & (1 << PB0)) && lastStatePB0){    //IGN 2 & 3
        // PB0 is LOW, turn off PA1 (2 & 3)
        PORTA |= (1 << PA1); // set high for off

        tempDelayCalc = (uint16_t)(TCNT1 - startTime23) - 70; //2.2ms
        if (pulses <= 20){
            tempDelayCalc = (uint16_t)(TCNT1 - startTime14) - 109; // 3.5ms
            pulses = pulses + 1;
        }        

        if (tempDelayCalc >= 0){
            delayBeforeLow = tempDelayCalc;
        } else {
            delayBeforeLow = 0;
        }
        lastStatePB0 = false; // false

    } else if ((PINB & (1 << PB0)) && !lastStatePB0) {   //IGN 2 & 3
        // PB0 is HIGH
        startTime23 = (uint16_t)TCNT1; // set timer timestamp

        timeToGoLow23 = (uint16_t)(startTime23 + delayBeforeLow);

        set23Low = true; // mark pin as has to go low eventually 

        lastStatePB0 = true; // true

    }

     
}


int main(void) {

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
            cli(); //16 bit math and time sensitive stuff
            if ((uint16_t)TCNT1 >= timeToGoLow23){
                PORTA &= ~(1 << PA1); // set low to start trigger
                set23Low = false;
            }
            sei(); //enable interrupts
        }
        if (set14Low){
            cli(); //16 bit math and time sensitive stuff 
            if ((uint16_t)TCNT1 >= timeToGoLow14){
                PORTA &= ~(1 << PA2); // set low to start trigger
                set14Low = false;
            }
            sei(); //enable interrupts
        }
    }

}