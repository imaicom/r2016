// cc -o pwm-controller-xy-rm pwm-controller-xy-rm.c -lwiringPi -lm
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


int ps3c_test(struct ps3ctls *ps3dat) {

	unsigned char i;
	unsigned char nr_btn = ps3dat->nr_buttons;
	unsigned char nr_stk = ps3dat->nr_sticks;
	int xx,yy,x,y,z;
	
//	printf("%d %d\n",nr_btn,nr_stk);

  	printf(" 1=%2d ",ps3dat->button[PAD_KEY_LEFT]);
	printf(" 2=%2d ",ps3dat->button[PAD_KEY_RIGHT]);
	printf(" 3=%2d ",ps3dat->button[PAD_KEY_UP]);
	printf(" 4=%2d ",ps3dat->button[PAD_KEY_DOWN]);
	printf(" 5=%4d ",ps3dat->stick [PAD_LEFT_X]);
	printf(" 6=%4d ",ps3dat->stick [PAD_LEFT_Y]);
	printf(" 7=%4d ",ps3dat->stick [PAD_RIGHT_X]);
	printf(" 8=%4d ",ps3dat->stick [PAD_RIGHT_Y]);
	printf("\n"); 

	xx = ps3dat->stick [PAD_LEFT_X];
	yy = ps3dat->stick [PAD_LEFT_Y];
	x = -xx * cos(-M_PI/4) - yy * sin(-M_PI/4);
	y = -xx * sin(-M_PI/4) + yy * cos(-M_PI/4);

	if(abs(x) < 5) {
		softPwmWrite(26,0);
		softPwmWrite(27,0);
	} else if(x > 0) {
		softPwmWrite(26,0);
		softPwmWrite(27,abs(x));
	} else {
		softPwmWrite(26,abs(x));
		softPwmWrite(27,0);
	};

	if(abs(y) < 5) {
		softPwmWrite(5,0);
		softPwmWrite(6,0);
	} else if(y > 0) {
		softPwmWrite(5,0);
		softPwmWrite(6,abs(y));
	} else {
		softPwmWrite(5,abs(y));
		softPwmWrite(6,0);
	};
	
	z = ps3dat->stick [PAD_RIGHT_X];

	if(abs(z) < 5) {
		softPwmWrite(28,0);
		softPwmWrite(29,0);
		softPwmWrite(24,0);
		softPwmWrite(25,0);
	} else if(z > 0) {
		softPwmWrite(28,0);
		softPwmWrite(29,abs(z));
		softPwmWrite(24,abs(z));
		softPwmWrite(25,0);
	} else {
		softPwmWrite(28,abs(z));
		softPwmWrite(29,0);
		softPwmWrite(24,0);
		softPwmWrite(25,abs(z));
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
	softPwmCreate(5,0,20); // motor-1 10ms
	softPwmCreate(6,0,20); // motor-1 10ms
	softPwmCreate(26,0,20); // motor-2 10ms
	softPwmCreate(27,0,20); // motor-2 10ms
	softPwmCreate(28,0,20); // motor-3 10ms
	softPwmCreate(29,0,20); // motor-3 10ms
	softPwmCreate(24,0,20); // motor-4 10ms
	softPwmCreate(25,0,20); // motor-4 10ms
	softPwmCreate(14,0,20); // motor-5 10ms // NC
	softPwmCreate(23,0,20); // motor-5 10ms // NC

	if(!(ps3c_init(&ps3dat, df))) {

		do {
			if (ps3c_test(&ps3dat) < 0) break;
		} while (!(ps3c_input(&ps3dat)));
		
		ps3c_exit(&ps3dat);		
	}
}

