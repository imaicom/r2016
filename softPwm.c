#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

int main(int argc,char *argv[]) {
	int fnum = 0;
	wiringPiSetup();
	printf("Raspberry SOFT PWM OUTPUT\n");

	softPWmCreate(20,0,100); // GP20 start-0 10ms
//	softPWmCreate(21,0,100); // GP21 start-0 10ms

	softPwmWrite(20,30); // GP20 3ms
//	softPwmWrite(21,30); // GP21 3ms

	while(1);

	return 0;
}	
