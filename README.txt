// README: Created on February 1, 2014
// Author: John H. (jxheee) WH6EDN
// Purpose: RIT ARC Blimp software development


General Description of Feature of MSP430G2553

This particular model has these particular features that are in use:

2 Timer A's (Timer A0 and Timer A1)
3 Capture-Control registers per timer
Comparator
Full control of the clock
32 kHz external watch crystal


General Description of the code function:

The pulse-width modulated signal from the Spektrum receiver outputs 1 to 2 ms long pulses every 20 ms (?), so the purpose of the microcontroller is to take this stream and convert it to 0% to 100% duty cycle PWM for the motor controller.
The frequency of the output is approximately 100 Hz; the motor controller has a maximum input frequency of 10 kHz; which shouldn't be a problem.

The input to the microcontroller is taken on P1.1 and 1.2, 1.1 is the aileron input, responsible for the turning of the blimp. P1.2 is the throttle input and controls the speed of the motors. These inputs have the characteristics as described above. They are fed into the capture-control registers of Timer A1.

Math is done to determine the length of the pulse, which is then used to determine duty cycle of the output. (See program for explicit details)

The calculated values are then applied to the registers of Timer A1 which have ports P2.0 to P2.5 used (6 outputs to the motor controller are used).
The outputs are:

2 PWM for each motor
4 control signals (two for each motor) to determine the direction of the motors (forward, backward, brake to GND, brake to Vdd,)
D1/D2 references in the schematic refer to disable control signals which are not controlled by the microcontroller. They are hardwired to pull the motors on at all times.

