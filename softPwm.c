// cc -o softPwm softPwm.c -lwiringPi
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

int main(int argc , char *argv[]) {

	wiringPiSetup();
	printf("Raspberry SOFT PWM OUTPUT\n");

	pinMode(23,OUTPUT);
//	pinMode(25,OUTPUT);
	digitalWrite(23,0);
//	digitalWrite(25,1);

//	softPwmCreate(23,0,100); // GP20 start-0 10ms
	softPwmCreate(25,0,100); // GP21 start-0 10ms

//	softPwmWrite(23,30); // GP20 3ms
	softPwmWrite(25,0); // GP21 3ms


	while (getchar() == EOF) ;

	return 0;
}	
