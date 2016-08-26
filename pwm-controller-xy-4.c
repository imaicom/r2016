// cc -o pwm-controller-xy-4 pwm-controller-xy-4.c -lwiringPi -lm
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include <wiringPi.h>
#include <softPwm.h>

#include "controller.h"
#include <math.h>


struct ps3ctls {
	
	int fd;
	unsigned char nr_buttons;	// Max number of Buttons
	unsigned char nr_sticks;	// Max number of Sticks
	short *button;			// button[nr_buttons]
	short *stick;			// stick[nr_sticks]
};

// global
int fds;
int s0 = 0;
int s1 = 0;
unsigned int timWheel;
int cntWheel = 0;

int resetPCA9685(int fd) {
	wiringPiI2CWriteReg8(fd,0,0);
}

int setPCA9685Freq(int fd , float freq) {
	float prescaleval;
	int prescale , oldmode , newmode;
	freq = 0.9 * freq;
	prescaleval = 25000000.0;
	prescaleval /= 4096.0;
	prescaleval /= freq;
	prescaleval -= 1.0;
	prescale = prescaleval + 0.5;
	oldmode = wiringPiI2CReadReg8(fd,0x00);
	newmode = (oldmode & 0x7F)|0x10;
	wiringPiI2CWriteReg8(fd , 0x00 , newmode);
	wiringPiI2CWriteReg8(fd , 0xFE , prescale);
	wiringPiI2CWriteReg8(fd , 0x00 , oldmode);
	sleep(0.005);
	wiringPiI2CWriteReg8(fd , 0x00 , oldmode | 0xA1);
}


int setPCA9685Duty(int fd , int channel , int on , int off) {
	int channelpos;
	channelpos = 0x6 + 4 * channel;
	wiringPiI2CWriteReg16(fd , channelpos   , on  & 0x0FFF);
	wiringPiI2CWriteReg16(fd , channelpos+2 , off & 0x0FFF);
}

int ps3c_test(struct ps3ctls *ps3dat) {

	unsigned char i;
	unsigned char nr_btn = ps3dat->nr_buttons;
	unsigned char nr_stk = ps3dat->nr_sticks;
	int m1,m2;

	setPCA9685Duty(fds , 0 , 0 , 276 + s0 / 2);
	setPCA9685Duty(fds , 1 , 0 , 276 + s1 / 2);

//	printf("%d %d\n",nr_btn,nr_stk);

  	printf(" 1=%2d ",ps3dat->button[PAD_KEY_LEFT]);
	printf(" 2=%2d ",ps3dat->button[PAD_KEY_RIGHT]);
	printf(" 3=%2d ",ps3dat->button[PAD_KEY_UP]);
	printf(" 4=%2d ",ps3dat->button[PAD_KEY_DOWN]);
	printf(" 5=%4d ",ps3dat->stick [PAD_LEFT_X]);
	printf(" 6=%4d ",ps3dat->stick [PAD_LEFT_Y]);
	printf(" 7=%4d ",ps3dat->stick [PAD_RIGHT_X]);
	printf(" 8=%4d ",ps3dat->stick [PAD_RIGHT_Y]);
	printf(" 9=%4d ",digitalRead(12));
	printf(" a=%4d ",digitalRead(13));
	printf(" b=%4d ",digitalRead(14));
	printf(" c=%4d ",digitalRead(15));
	printf(" d=%4d ",digitalRead( 3));
	printf(" e=%4d ",cntWheel);

	if (ps3dat->button[PAD_KEY_TRIANGLE]) {
		digitalWrite(7,1);
	} else {
		digitalWrite(7,0);
	};

	if (ps3dat->button[PAD_KEY_CIRCLE]) {
		softPwmWrite(25,50);
	} else {
		softPwmWrite(25,0);
	};
	
	if(digitalRead(13)+digitalRead(14)>0) {
		softPwmWrite(25,50);
	} else {
		softPwmWrite(25,0);
	};

//	if (ps3dat->button[PAD_KEY_LEFT])	{ s0++; if(s0 > +200) s0 = +200; };
//	if (ps3dat->button[PAD_KEY_RIGHT])	{ s0--; if(s0 < -200) s0 = -200; };
//	if (ps3dat->button[PAD_KEY_UP]) 	{ s1++; if(s1 > +200) s1 = +200; };
//	if (ps3dat->button[PAD_KEY_DOWN])	{ s1--; if(s1 < -200) s1 = -200; };

//	printf(" s0=%4d ",s0);
//	printf(" s1=%4d ",s1);
	printf("\n"); 

	m1 = ps3dat->stick [PAD_LEFT_Y ];
	m2 = ps3dat->stick [PAD_RIGHT_Y];

	if(abs(m1) < 5) {
		softPwmWrite(16,0);
		softPwmWrite( 1,0);
	} else if(m1 > 0) {
		softPwmWrite(16,0);
		softPwmWrite( 1,abs(m1));
	} else {
		softPwmWrite(16,abs(m1));
		softPwmWrite( 1,0);
	};

	if(abs(m2) < 5) {
		softPwmWrite(28,0);
		softPwmWrite(29,0);
	} else if(m2 > 0) {
		softPwmWrite(28,0);
		softPwmWrite(29,abs(m2));
	} else {
		softPwmWrite(28,abs(m2));
		softPwmWrite(29,0);
	};
	
	if (timWheel != digitalRead(12)) {
		timWheel = digitalRead(12);
		cntWheel++;
	};

	if(ps3dat->button[PAD_KEY_CROSS]==1) return -1; // end of program
	return 0;
}


int ps3c_input(struct ps3ctls *ps3dat) {

	int rp;
	struct js_event ev;

	do {
		rp = read(ps3dat->fd, &ev, sizeof(struct js_event));
		if (rp != sizeof(struct js_event)) {
			return -1;
		}

		if (timWheel != digitalRead(12)) {
			timWheel = digitalRead(12);
			cntWheel++;
		};

	} while (ev.type & JS_EVENT_INIT);

	switch (ev.type) {
		case JS_EVENT_BUTTON:
			if (ev.number < ps3dat->nr_buttons) {
				ps3dat->button[ev.number] = ev.value;
			}
			break;
		case JS_EVENT_AXIS:
			if (ev.number < ps3dat->nr_sticks) {
				ps3dat->stick[ev.number] = ev.value / 327; // range -32767 ~ +32768 -> -100 ~ +100
			}
			break;
		default:
			break;
	}

	return 0;
}


int ps3c_getinfo(struct ps3ctls *ps3dat) {

	if(ioctl(ps3dat->fd , JSIOCGBUTTONS , &ps3dat->nr_buttons) < 0) return -1;
	if(ioctl(ps3dat->fd , JSIOCGAXES    , &ps3dat->nr_sticks ) < 0) return -2;

	return 0;
}


int ps3c_init(struct ps3ctls *ps3dat, const char *df) {

	unsigned char nr_btn;
	unsigned char nr_stk;
	unsigned char *p;
	int i;

	ps3dat->fd = open(df, O_RDONLY);
	if (ps3dat->fd < 0) return -1;

	if (ps3c_getinfo(ps3dat) < 0) {
		close(ps3dat->fd);
		return -2;
	}

	nr_btn = ps3dat->nr_buttons;
	nr_stk = ps3dat->nr_sticks;
//	printf("%d %d\n",nr_btn,nr_stk);

	p = calloc(nr_btn + nr_stk , sizeof(short));
	if (p == NULL) {
		close(ps3dat->fd);
		return -3;
	}
	ps3dat->button = (short *)p;
	ps3dat->stick  = (short *)&p[nr_btn * sizeof(short)];
	
//	for (i = 0; i<nr_btn; i++) ps3dat->button[i] = 0;
//	for (i = 0; i<nr_stk; i++) ps3dat->stick [i] = 0;
//	ps3dat->button[PAD_KEY_LEFT]=0;
//	ps3dat->button[PAD_KEY_RIGHT]=0;
//	ps3dat->button[PAD_KEY_UP]=0;;
//	ps3dat->button[PAD_KEY_DOWN]=0;
//	ps3dat->stick [PAD_LEFT_X]=0;
//	ps3dat->stick [PAD_LEFT_Y]=0;
//	ps3dat->stick [PAD_RIGHT_X]=0;
//	ps3dat->stick [PAD_RIGHT_Y]=0;
	timWheel = digitalRead(12);

	return 0;
}

void ps3c_exit   (struct ps3ctls *ps3dat) {

	free (ps3dat->button);
	close(ps3dat->fd);
}


void main() {

	char *df = "/dev/input/js0";
	struct ps3ctls ps3dat;

	wiringPiSetup();
	softPwmCreate(16,0,20); // start-0 10ms
	softPwmCreate( 1,0,20); // start-0 10ms
	softPwmCreate(28,0,20); // start-0 10ms
	softPwmCreate(29,0,20); // start-0 10ms
	softPwmCreate(25,0,20); // start-0 10ms

	pinMode( 3,INPUT);pullUpDnControl( 3,PUD_UP);
	pinMode(12,INPUT);pullUpDnControl(12,PUD_UP);
	pinMode(13,INPUT);pullUpDnControl(13,PUD_UP);
	pinMode(14,INPUT);pullUpDnControl(14,PUD_UP);
	pinMode(15,INPUT);pullUpDnControl(15,PUD_UP);
	
	pinMode(4,OUTPUT);digitalWrite(4,0);
	pinMode(7,OUTPUT);digitalWrite(7,0);
	
	fds = wiringPiI2CSetup(0x40);	// PCA9685
	resetPCA9685(fds);
	setPCA9685Freq(fds,50);

//	while(1) {
		if(!(ps3c_init(&ps3dat, df))) {

			do {
				if (ps3c_test(&ps3dat) < 0) break;
//				if(!(digitalRead( 3))) {system("sudo shutdown -h now &");};
				if(digitalRead( 3)) {
					digitalWrite(4,1);
					break;
				};
			} while (!(ps3c_input(&ps3dat)));
		
			ps3c_exit(&ps3dat);		
		};
//	};
}

