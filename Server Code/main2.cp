/*************************************************************************
 * servo.c
 *
 * PCA9685 servo example
 * Connect a servo to any pin. It will rotate to random angles.
 *
 *
 * This software is a devLib extension to wiringPi <http://wiringpi.com/>
 * and enables it to control the Adafruit PCA9685 16-Channel 12-bit
 * PWM/Servo Driver <http://www.adafruit.com/products/815> via I2C interface.
 *
 * Copyright (c) 2014 Reinhard Sprung
 *
 * If you have questions or improvements email me at
 * reinhard.sprung[at]gmail.com
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The given code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You can view the contents of the licence at <http://www.gnu.org/licenses/>.
 **************************************************************************
 */

#include "pca9685.h"

#include <wiringPi.h>



#define PIN_BASE 300
#define MAX_PWM 4096
#define HERTZ 50

#if 0
//#define GPIO_1 12
//#define GPIO_2 16
//#define GPIO_3 20
//#define GPIO_4 21

#define GPIO_1 13
#define GPIO_2 19
#define GPIO_3 26
#define GPIO_4 6


#else
#define GPIO_1 17
#define GPIO_2 27
#define GPIO_3 23
#define GPIO_4 22

#endif

void gpio_setup()
{
	wiringPiSetupGpio();
	pinMode (GPIO_1, OUTPUT);
	pinMode (GPIO_2, OUTPUT);
	pinMode (GPIO_3, OUTPUT);
	pinMode (GPIO_4, OUTPUT);
	
}


/**
 * Calculate the number of ticks the signal should be high for the required amount of time
 */
int calcTicks(float impulseMs, int hertz)
{
	float cycleMs = 1000.0f / hertz;
	return (int)(MAX_PWM * impulseMs / cycleMs + 0.5f);
}

/**
 * input is [0..1]
 * output is [min..max]
 */
float map(float input, float min, float max)
{
	return (input * max) + (1 - input) * min;
}

int servo_init()
{
	int fd = pca9685Setup(PIN_BASE, 0x40, HERTZ);

	printf("fd=%d\n",fd);
	if (fd < 0)
	{
		printf("Error in setup\n");
		return fd;
	}

	// Reset all output
	pca9685PWMReset(fd);

	return fd;
}

void gpio_high(int pin)
{
 	digitalWrite (pin, HIGH); 

}

void gpio_low(int pin)
{
 	digitalWrite (pin, LOW); 
}

int servo_speed(int pin, int speed) // speed limit 0-100
{
	// Set servo to neutral position at 1.5 milliseconds
	// (View http://en.wikipedia.org/wiki/Servo_control#Pulse_duration)
	if( speed > 100)
	{
		printf("ERROR speed should be (0-100), but it is set to %d\n",speed);
		return -1;
	}
	
	float millis = 1 + ((float)speed/100); // 1->2
	int tick = calcTicks(millis, HERTZ);

//	int pin = 0;

	// Set servo to neutral position at 1.5 milliseconds
	// (View http://en.wikipedia.org/wiki/Servo_control#Pulse_duration)

	if( speed == 100)
		tick = 4096;
	else
		tick = (4096/100)*speed;

	printf("tick = %d\n", tick);
	
	pwmWrite(PIN_BASE + pin, tick);
//	delay(2000);

	return 0;
}

void servo_stop_all()
{
	servo_speed(0,0);
	servo_speed(1,0);
	servo_speed(2,0);
	servo_speed(3,0);

	servo_speed(4,0);
	servo_speed(5,0);
	servo_speed(6,0);
	servo_speed(7,0);

}

int main(void)
{
	printf("PCA9685 servo example\n");
	printf("Connect a servo to any pin. It will rotate to random angles\n");

	// Setup with pinbase 300 and i2c location 0x40
	int fd;
	
	gpio_setup();

#if 1
	gpio_high(GPIO_1);
	gpio_high(GPIO_2);
	gpio_high(GPIO_3);
	gpio_high(GPIO_4);
#else	
	gpio_low(GPIO_1);
	gpio_low(GPIO_2);
	gpio_low(GPIO_3);
	gpio_low(GPIO_4);
	
#endif	
		
	
	fd = servo_init();
	if( fd < 0 )
	{
		printf("servo init error\n");
		return 0;
	}

	servo_speed(0, 0);

	servo_speed(4,0);
	servo_speed(5,0);
	servo_speed(6,0);
	servo_speed(7,0);
#if 0

	int active = 1;
	while (active)
	{
		// That's a hack. We need a random number < 1
		float r = rand();
		while (r > 1)
			r /= 10;

		millis = map(r, 1, 2);
		tick = calcTicks(millis, HERTZ);
		
		printf("millis=%f tick:%d\n", millis, tick);
		tick = 600;

		pwmWrite(PIN_BASE + pin, tick);
		delay(1000);
		break;
	}
#endif
	servo_stop_all();
	return 0;
}

