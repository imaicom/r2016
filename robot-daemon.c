// cc -o robot-daemon robot-daemon.c -lwiringPi -lm
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include <wiringPi.h>

unsigned int timWheel;
long int cntWheel = 0;
unsigned int timBar;
long int Bar = 0;

int check_port(char fnp[256],int d) {
	
	FILE *fp;
	char fn[256]="/tmp/";

	strcat(fn,fnp);
	strcat(fn,".txt");
	
	if((fp=fopen(fn,"r+"))==NULL) {
		fp=fopen(fn,"w+");
	};
	
	fprintf(fp,"%8d",d);
	fclose(fp);
}

void main() {
	
	char s[256];
	int tmp;
	
	wiringPiSetup();
	pinMode( 3,INPUT);pullUpDnControl( 3,PUD_UP);	// program-sw
	pinMode(12,INPUT);pullUpDnControl(12,PUD_UP); // cntWheel
	pinMode(13,INPUT);pullUpDnControl(13,PUD_UP); // gnd-sensor1
	pinMode(14,INPUT);pullUpDnControl(14,PUD_UP); // gnd-sensor2
	pinMode(15,INPUT);pullUpDnControl(15,PUD_UP); // ball
	pinMode( 5,INPUT);pullUpDnControl( 5,PUD_UP); // kill sw(red)
	pinMode( 6,INPUT);pullUpDnControl( 6,PUD_UP); // start sw(white)
	system("/home/pi/Pi-PWM/pwm-controller-xy-4 &");
	
	while(1) {
		
		if (timWheel != digitalRead(12)) {
			timWheel = digitalRead(12);
			cntWheel++;
		};
		
		if(digitalRead(13)+digitalRead(14)>0) tmp = 1; else tmp = 0;
		if(timBar != tmp) {
			if(digitalRead(13)+digitalRead(14)>0) timBar = 1; else timBar = 0;
			Bar++;
		};
		
		check_port("program-sw"	,digitalRead( 3) );
		check_port("cntWheel"		,cntWheel );
		check_port("bar"			,Bar );
		check_port("ball"			,digitalRead(15) );
		check_port("kill-sw"		,digitalRead(5) );
		check_port("start-sw"		,digitalRead(6) );
		
//		if(!(digitalRead( 5))) {system("sudo shutdown -h now &");};
		

	};
}

