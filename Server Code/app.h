// bob sang

#ifndef __S_APP_H__
#define __S_APP_H__

#include "object.h"

#include "message.h"

typedef struct s_car_angle_t_
{
	int64 m_ms0;
	int64 m_delta_ms;
	int m_angle;
	int m_speed;
	int m_dir;		// > 0 left, < 0 right
	
} s_car_angle_t ;

#define RUN_STATE_IDLE 0
#define RUN_STATE_CAR_MOVING 1
#define RUN_STATE_CAR_ANGLE 2

class s_app_t : public s_object_t
{
public:
    s_app_t();
    ~s_app_t();

public:    
    static s_app_t* get_app();
    virtual int start();
    virtual int stop();
    virtual int run();
    int is_quit();
	
	int light_on();
	int light_off();

	int car_init();
	int car_speed(int pin, int speed); // speed limit 0-100

	int move_car_left(int speed);
	int move_car_right(int speed);
	int move_car_forward(int speed);
	int move_car_backward(int speed);
	int move_car_stop();

	int move_car_angle(int speed, int angle);

	int servo_loc(int id, int location); // location is from 1000-2000 (1000us - 2000us)
	
protected:
	int do_angle();	
	
private:
	static void * thread_keyboard_entry(void *);
	int run_keyboard();
private:
	int m_run_state;
		
	s_message_t * m_msg;
		

	thread_t m_keyboard_thread;

	int64	m_last_ms;
	int m_exit;


	int m_alpha1;
	int m_alpha2;

	s_car_angle_t m_angle;
	};
	
#endif
	

