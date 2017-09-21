//
#include "app.h"
#include "message.h"
#include "log.h"
#include "utilities.h"
#include <math.h>
#include <wiringPi.h>
#include "timer.h"

#include "pca9685.h"

using namespace s_log;
using namespace s_utilities;

#define ONE_DEGREE_MS (4595/360)

#define PIN_BASE 300
#define MAX_PWM 4096
#define HERTZ 50 // PWM width is 20ms

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

s_app_t::s_app_t()
{
	m_msg = s_message_t::get_inst();	
		
    CREATE_THREAD(m_keyboard_thread, thread_keyboard_entry, 0x8000, this, 1);         // Reserve 32KB stack
	m_last_ms = 0;
	m_exit = 0;
	
	m_msg->start();

	car_init();
	
	m_run_state = RUN_STATE_IDLE;
	memset(&m_angle, 0, sizeof(m_angle));

}

s_app_t::~s_app_t()
{
	m_msg->stop();

}

s_app_t* s_app_t::get_app()
{
	static s_app_t * g_app = NULL;
	if( !g_app )
		g_app = new s_app_t();

	return g_app;
}

int s_app_t::start()
{
	if( is_started() )
		return 0;
	s_object_t::start();
	
	m_run_state = 0;
	return 0;
}

int s_app_t::stop()
{
	s_log_info("app stop\n");
	if( !is_started() )
		return 0;
	s_object_t::stop();

	move_car_stop();

	return 0;
}

int s_app_t::light_on()
{
	digitalWrite (GPIO_1, HIGH); 
	return 0;
}

int s_app_t::light_off()
{
 	digitalWrite (GPIO_1, LOW); 
	return 0;
}

int s_app_t::car_init()
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

	wiringPiSetupGpio();
	
	pinMode (GPIO_1, OUTPUT);
	pinMode (GPIO_2, OUTPUT);
	pinMode (GPIO_3, OUTPUT);
	pinMode (GPIO_4, OUTPUT);

	return 0;
}

int s_app_t::servo_loc(int pin, int location)
{
	if (pin < 8 )
	{
		printf("ERROR pin is out of range (%d < 8)\n", pin);
		return 0;
	}
	else if( pin > 15 )
	{
		printf("ERROR pin is out of range (%d > 15)\n", pin);
		return 0;
	}
	
//	if( (location != 0) &&  ( location <1000 )
//		location = 1000;
	
//	if( location > 2000 )
//		location = 2000;
	
	pwmWrite(PIN_BASE + pin, location);

	return 0;
}

int s_app_t::car_speed(int pin, int speed) // speed limit 0-100%
{
	// Set servo to neutral position at 1.5 milliseconds
	// (View http://en.wikipedia.org/wiki/Servo_control#Pulse_duration)
	if( speed > 100)
	{
		printf("ERROR speed should be (0-100), but it is set to %d\n",speed);
		return -1;
	}
	
//	float millis = 1 + ((float)speed/100); // 1->2
	int tick;// = calcTicks(millis, HERTZ);

//	int pin = 0;

	// Set servo to neutral position at 1.5 milliseconds
	// (View http://en.wikipedia.org/wiki/Servo_control#Pulse_duration)

	if( speed == 100)
		tick = 4096;
	else
		tick = (4096*speed)/100;

	printf("pin=%d tick = %d\n",pin, tick);
	
	pwmWrite(PIN_BASE + pin, tick);

	return 0;
}

int s_app_t::move_car_left(int speed)
{
	car_speed(0,speed);
	car_speed(1,100-speed);
	car_speed(2,speed);
	car_speed(3,100-speed);

	car_speed(4,100);
	car_speed(5,0);
	car_speed(6,100);
	car_speed(7,0);
	return 0;

}

int s_app_t::move_car_right(int speed)
{
	car_speed(0,100-speed);
	car_speed(1,speed);
	car_speed(2,100-speed);
	car_speed(3,speed);

	car_speed(4,0);
	car_speed(5,100);
	car_speed(6,0);
	car_speed(7,100);
	return 0;
}

int s_app_t::move_car_forward(int speed)
{
	car_speed(4,0);
	car_speed(0,speed);
	
	car_speed(5,0);
	car_speed(1,speed);
	
	car_speed(6,0);
	car_speed(2,speed);
	
	car_speed(7,0);
	car_speed(3,speed);

	return 0;
}

int s_app_t::move_car_backward(int speed)
{
	car_speed(0,100-speed);
	car_speed(1,100-speed);
	car_speed(2,100-speed);
	car_speed(3,100-speed);

	car_speed(4,100);
	car_speed(5,100);
	car_speed(6,100);
	car_speed(7,100);
	return 0;
}

int s_app_t::move_car_stop()
{
	car_speed(0,0);
	car_speed(1,0);
	car_speed(2,0);
	car_speed(3,0);

	car_speed(4,0);
	car_speed(5,0);
	car_speed(6,0);
	car_speed(7,0);
	m_run_state = RUN_STATE_IDLE;
	return 0;
}

int s_app_t::move_car_angle(int speed, int angle)
{
	int delta;
	s_timer_t * p_timer = s_timer_t::get_inst();
	m_angle.m_speed = speed;
	move_car_stop();

	delta = angle - m_angle.m_angle;
	if( delta < 0 )
	{
		m_angle.m_dir = 0;
		delta = -1*delta;
	}
	else
	{
		m_angle.m_dir = 1;
	}
	m_angle.m_delta_ms = delta * ONE_DEGREE_MS;
	
	printf(" delta angle=%d dir:%d ms:%lldms\n", delta, m_angle.m_dir, m_angle.m_delta_ms);
	
	m_angle.m_angle = angle;

	m_angle.m_ms0 = p_timer->get_ms();
	m_run_state = RUN_STATE_CAR_ANGLE;
	
	if( m_angle.m_dir )
	{
		move_car_left(20);
	}
	else
	{
		move_car_right(20);
	}

	return 0;
	
}



int s_app_t::run_keyboard()
{
	char ch;
	int64 ms0;
	s_timer_t * p_timer = s_timer_t::get_inst();
	

	while(1)
	{
		ch = getchar();
		s_log_info("ch=%c\n", ch);
		
		switch(ch)
		{
			case 'a':
				move_car_angle(5, 45);
				break;
			case 'A':
				move_car_angle(5, -45);
				break;					
			case '0':
				move_car_angle(5, 0);
				break;
			case '1':
				move_car_forward(20);
				break;
			case '2':
				move_car_forward(70);
				break;
			case '3':
				move_car_backward(20);
				break;
			case '4':
				move_car_backward(50);
				break;
				
			case '5':
				servo_loc(8,205);
				break;				
			case '6':
				servo_loc(8, 300);
				break;
			case '7':
				servo_loc(8, 405);
				break;
			case '8':
				servo_loc(8, 500);
				break;
			case '9':
				servo_loc(9, 100);
				break;
			case '-':
				servo_loc(9,150);
				break;
			case '=':
				servo_loc(9,355);
				break;
				
			case 'l':
				ms0 = p_timer->get_ms();
				move_car_left(20);
				break;
			case 'r':
				ms0 = p_timer->get_ms();
				move_car_right(40);
				break;
			case 's':			
				int64 ms1;
				move_car_stop();
				m_run_state = RUN_STATE_IDLE;
				ms1 = p_timer->get_ms();
				printf(" time duration: %lldms %lld %lld\n", ms1 - ms0, ms1, ms0);
				break;
			case 'q':				
				servo_loc(8, 0);
				servo_loc(9, 0);

				break;

		}
		if( ch == 'q')
			break;
	}
	move_car_stop();
	m_exit = 1;
	if( m_quit) 
		m_exit = 2;
	return 0;
}

int s_app_t::is_quit()
{
	return (m_exit == 2);
}

void * s_app_t::thread_keyboard_entry(void * pval)
{
   	s_app_t *papp = (s_app_t*)pval;

	papp->run_keyboard();
	return NULL;
}

int s_app_t::do_angle()
{
	s_timer_t * p_timer = s_timer_t::get_inst();
	int64 ms;
	ms = p_timer->get_ms();
	if( (ms - m_angle.m_ms0 ) >= m_angle.m_delta_ms )
	{
		int speed;
		printf(" stop turning...\n");
		m_angle.m_delta_ms = 0;
		
		m_run_state = RUN_STATE_CAR_MOVING;
		
		speed = m_angle.m_speed;
		if( speed > 10 )
			speed = 5;
		if( speed < 0 )
			speed = 5;

		speed *= 10;

		move_car_forward(speed);
	
	}
	return 0;
}

int s_app_t::run()
{
	while(!this->m_quit)
	{
		if( m_exit == 1)
			break;
	 	switch( m_run_state )
		{
			case RUN_STATE_CAR_ANGLE:
				do_angle();
				break;
	 	}
		s_sleep_ms(50);
	}
	m_quit = 1;
	s_log_info("s_app_t::run EXIT quit:%d\n", this->m_quit);
	if( m_exit == 1 )
	{
		stop();
		m_exit = 2;
	}

	return 0;
}


