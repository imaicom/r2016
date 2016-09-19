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

int check_port(char fnp[256],long int d) {
	
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
	
	char t[256];
	char s[256];
	int tmp;
	
	wiringPiSetup();
	pinMode( 3,INPUT);pullUpDnControl( 3,PUD_UP);	// program-sw
	pinMode(12,INPUT);pullUpDnControl(12,PUD_UP); // cntWheel
	pinMode(21,INPUT);pullUpDnControl(21,PUD_UP); // gnd-sensor1
	pinMode(22,INPUT);pullUpDnControl(22,PUD_UP); // gnd-sensor2
	pinMode(30,INPUT);pullUpDnControl(30,PUD_UP); // gnd-sensor center
	pinMode(23,INPUT);pullUpDnControl(23,PUD_UP); // gnd-sensor3
	pinMode(24,INPUT);pullUpDnControl(24,PUD_UP); // gnd-sensor4
	pinMode(15,INPUT);pullUpDnControl(15,PUD_UP); // ball
	pinMode( 5,INPUT);pullUpDnControl( 5,PUD_UP); // kill sw(red)
	pinMode( 6,INPUT);pullUpDnControl( 6,PUD_UP); // start sw(white)
//	system("sudo /home/pi/robot/self &");
	
	while(1) {
		
		if (timWheel != digitalRead(12)) {
			timWheel = digitalRead(12);
			cntWheel++;
		};
		
//		if(digitalRead(13)+digitalRead(14)>0) tmp = 1; else tmp = 0;
//		if(timBar != tmp) {
//			if(digitalRead(13)+digitalRead(14)>0) timBar = 1; else timBar = 0;
//			Bar++;
//		};
		
//		if(digitalRead(21)+digitalRead(22)+digitalRead(23)+digitalRead(24)==4) tmp = 1; else tmp = 0;
//		if(timBar != tmp) {
//			if(digitalRead(21)+digitalRead(22)+digitalRead(23)+digitalRead(24)==4) timBar = 1; else timBar = 0;
//			Bar++;
//		};

//		if(digitalRead(22)+digitalRead(23)==2) tmp = 1; else tmp = 0;
//		if(timBar != tmp) {
//			if(digitalRead(22)+digitalRead(23)==2) timBar = 1; else timBar = 0;
//			Bar++;
//		};

		if(digitalRead(30)) tmp = 0; else tmp = 1;
		if(timBar != tmp) {
			if(digitalRead(30)) timBar = 0; else timBar = 1;
			Bar++;
			sprintf(t,"%02d",Bar);
			printf("%s",t);
			strcpy(s,"mpg123 /home/pi/Music/");
			strcat(s,t);
			strcat(s,".mp3 &");
			printf("%s\n",s);
			system(s);
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

