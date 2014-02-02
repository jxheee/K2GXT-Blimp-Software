/*
 * K2GXT blimp software
 * Copyright 2013 RIT Amateur Radio Club, K2GXT
 *
 * Author: Matt Smicinski (mjs7769@rit.edu)
 *
 * Output:
 *  Left: 2.1 (PWM) 2.0 (IN1), 2.2 (IN2) (H bridge control, forward, reverse, etc)
 *  Right: 2.4 (PWM) 2.3 (IN1), 2.5 (IN2) (H bridge control, forward, reverse, etc)
 * 
 * Input:
 *  Throttle: 1.2 (PWM)
 *  Aileron: 1.1 (PWM)
 *
 *
 *  TODO:
 *      Handle H bridge control
 *
 */

#include <msp430g2553.h>

#define DEBUG 0

#define throttleInput BIT2
#define aileronInput BIT1

#define leftPWM BIT1
#define rightPWM BIT4

#define leftIN1 BIT0
#define leftIN2 BIT2

#define rightIN1 BIT3
#define rightIN2 BIT5

#define LEFTOUTPUT TA1CCR1
#define RIGHTOUTPUT TA1CCR2



unsigned int throttleRising = 0;
unsigned int throttleFalling = 0;
unsigned int throttleDuration = 0;
unsigned int zeroThrottle = 0;
unsigned int aileronRising = 0;
unsigned int aileronFalling = 0;
unsigned int aileronDuration = 0;
unsigned int turnLength = 0;
unsigned int turnValue = 0;
unsigned int throt;
unsigned int centerAileron = 0;

__interrupt void Timer0_A0();
__interrupt void Timer0_A1();
void forward();
void reverseLeft();
void reverseRight();
void outputPwm();

void main(void){

    
    WDTCTL = WDTPW + WDTHOLD;  //Stop watchdog timer
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
    CCTL0 = CCIE;
    CCTL1 = CCIE;

    TA1CTL |= TASSEL_1 + MC_1; //Timer A0 (output) setup
    
    TA0CTL |= TASSEL_1  + MC_2; //Timer A1 (input) setup

    TA1CCR0 = 90;  //PWM output period

    //Left output setup
    P2DIR |= leftIN1 | leftPWM | leftIN2;
    P2SEL |= leftPWM;
//    P2OUT &= leftIN1;
 //   P2OUT |= ~leftIN2;
    //Right output setup
    P2DIR |= rightIN1 | rightPWM | rightIN2 ;
    P2SEL |= rightPWM;
 //   P2OUT |= rightIN1;
//    P2OUT |= ~rightIN2;
    forward();
    //Left output setup
    TA1CCR2 = 0; //0% duty cycle (TA0CCR2/TA0CCR0 = duty cycle)

    TA1CCTL2 = OUTMOD_7;

    //Right output setup
    TA1CCR1 = 0; //0% duty cycle

    TA1CCTL1 = OUTMOD_7 ;

    if (DEBUG) {
    	P1DIR |= BIT0;
    }
    //Aileron setup

    //Use Pin 1.1 for PWM input
    P1DIR &= ~aileronInput;
    P1SEL |= aileronInput;

    //Setup timer
    TA0CCTL0 = CM_3 | SCS | CCIS_0 | CAP | CCIE;


    //Throttle setup

    //Use Pin 1.2 for PWM input
    P1DIR &= ~throttleInput;
    P1SEL |= throttleInput;

    //Setup timer
    TA0CCTL1 = CM_3 | SCS | CCIS_0 | CAP | CCIE;

//    __enable_interrupt();
    __bis_SR_register(LPM0_bits + GIE);        // enter LPM0 with interrrupt enable


}

void outputPwm() {

	if(DEBUG) {
		P1OUT ^= BIT0; //Sanity check
	}
    forward();

	throt = throttleDuration - zeroThrottle;

    if(throt > 60000) { //overflow check
        throt = 0;
    } else if (throt > 30) { //max check
        throt = 30;
    }

    throt = throt * 2;

    if (aileronDuration > 60) {
    	aileronDuration = 60;
    }

    if (aileronDuration > 53) { //left
    	LEFTOUTPUT = throt;
    	RIGHTOUTPUT = throt + ((aileronDuration - 53) * 2);
    } else if (aileronDuration < 52) { //right
    	LEFTOUTPUT = throt + ((52 - aileronDuration)*2);
    	RIGHTOUTPUT = throt;
    } else {
    	LEFTOUTPUT = throt;
    	RIGHTOUTPUT = throt;

    }
}

//H bridge IN
void forward() {
	 P2OUT ^= 0x0009;
}


void reverseLeft() {
	P2OUT &= ~leftIN1;
	P2OUT |= leftIN2;
}
void reverseRight() {
	P2OUT &= ~rightIN1;
	P2OUT |= rightIN2;
}


//Aileron
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0() {
    if (aileronRising != 0 && aileronFalling == 0) {
        aileronFalling = TA0R;
    } else {
        int t = aileronFalling - aileronRising;
        if(t < 67) { //64 = left, added 1 for small buffer
            if(centerAileron == 0) {
                centerAileron = 52;
            }
            aileronDuration = t;
        }
        aileronFalling = 0;
        aileronRising = TA0R;
    }
    __bic_SR_register_on_exit(LPM0_bits);


}

//Throttle
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1() {
    if (TA0IV == TA0IV_TACCR1) {  //Check if TA0CCR1 triggered this interrupt (that's Pin 2.4 in this case)
            if (throttleRising != 0 && throttleFalling == 0) {
                throttleFalling = TA0R;
            } else {
                int d = throttleFalling - throttleRising;
                if (d < 67) { //64 = 100% max value
                    if(zeroThrottle == 0) {
                        zeroThrottle = 30;
                    }
                    throttleDuration = d;
                }
                throttleFalling = 0;
                throttleRising = TA0R;
                outputPwm();

            }
    }
    __bic_SR_register_on_exit(LPM0_bits);


}
