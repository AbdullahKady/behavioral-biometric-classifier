#include <REG52.H>
#include <stdio.h>
#include <string.h>


char pass[11] = {'.', 't', 'i', 'e', '5', 'R', 'o', 'n', 'a', 'l', '\0'};

/*
	Up to 255 decimal count of "overflows", ranging from 0 to ~15 seconds.
*/
unsigned int timingsA[9];
unsigned int timingsB[9];
unsigned char timer = 0; // In overflows :D (~0.06sec)
unsigned char* testTimings; // Calculated in the main method, but used by calculateEculidean


void init() {
	/*------------------------------------------------
	Setup the serial port for 2400 baud at 12MHz.
	------------------------------------------------*/
	SCON = 0x50; // SCON: mode 1, 8-bit UART, enable reciever
	TMOD = 0x21; // TMOD: timer 1, mode 2, 8-bit reload <> timer 0, mode 1
	TH1 = 0xF3; // TH1: reload value for 2400 baud @ 12MHz
	TR1 = 1; // TR1: timer 1 run
	TI = 1; // TI: set TI to send first char of UART
	ET0 = 1; // Enable interrupts for T0
	EA = 1; // Enable global interrupts
}

// Up to ~15 seconds
void handleOverflow(void) interrupt 1 {
	timer ++;
}

// unsigned int readTimer () {
// 	return (TH0 << 8) | TL0 ;
// }

void stopTimer() {
	TR0 = 0;
}

void resetTimer() {
	TL0 = 0x00;
	TH0 = 0x00;
	timer = 0;
	TR0 = 1;
}

unsigned char* calculateTimings() {
	unsigned char charIndex = 0;
	unsigned char localTimings[9];

	while (1) {
		char c = _getkey();
		if (c == pass[charIndex]) {
			stopTimer();
			if (charIndex != 0)
				localTimings[charIndex - 1] = timer;
			charIndex++;
		} else {
			printf("FAILED\n");
			charIndex = 0;
		}

		resetTimer();

		// One run of correct input password
		if (charIndex == 10) {
			return localTimings;
		}
	}
}

void trainUser(unsigned int timings[]) {
	unsigned char count = 0;
	unsigned char i;
	unsigned char *currentTimings;

	resetTimer();
	while (count < 5) {
		currentTimings = calculateTimings();
		for (i = 0 ; i < 9 ; i++) {
			timings[i] += currentTimings[i];
		}
		count ++;
		printf("Trained -> %bu\n", count);
	}

	// Done successful <counts> of input runs, just average them.
	for (i = 0; i < 9; i++) {
		timings[i] = (timings[i] / count);
	}
}

unsigned int calculateEculidean(unsigned int timings[]) {
	unsigned char i;
	unsigned long summation = 0;

	for (i = 0 ; i < 9 ; i++) {
		// Much cheaper than actually importing the POW
		summation += ((testTimings[i] - timings[i]) * (testTimings[i] - timings[i]));
	}

	/*
		* Worst possible case is, test timing = 255, original timing = 0 ?
		* This will lead to 255*255*9, so still safe for an unsigned long

		* Hypothesis: if the summation is greater than X, then SQRT is
	  * greater than X as well: https://i.imgflip.com/2lspgm.jpg
	*/

	return summation;

}

void main (void) {

	init();
	trainUser(timingsA);
	trainUser(timingsB);

	/*
		* Train once, then keep looping forever, taking input and testing
		* it against the values obtained from the training
	*/
	while (1) {
		testTimings = calculateTimings();

		if (calculateEculidean(timingsA) < calculateEculidean(timingsB)) {
			printf("A\n");
		} else {
			printf("B\n");
		}
	}
}
