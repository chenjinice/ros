#include <ros/ros.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>
#include "wheel_control.h"
#include <geometry_msgs/Twist.h>

using namespace std;

//#define PI 3.141592654

//左右轮速度
static short g_leftSpeed = 0;
static short g_rightSpeed = 0;

//两个轮子间距离
static double g_wheelR = 480.0;

static pthread_mutex_t g_tMutex  = PTHREAD_MUTEX_INITIALIZER;

void sigfun(int sig)
{
	wheel_control_deInit();
	pthread_mutex_destroy(&g_tMutex);
	exit(0);
}

void twist_message_received(const geometry_msgs::Twist &msg)
{
//收到消息回调函数
	pthread_mutex_lock(&g_tMutex);
	double linear_x = msg.linear.x*1000;
	double angular_z = msg.angular.z;
	g_rightSpeed = (short)(linear_x+g_wheelR*angular_z/2);
	g_leftSpeed  = (short)(linear_x-g_wheelR*angular_z/2);
	pthread_mutex_unlock(&g_tMutex);
	
	//cout << "left = " << g_leftSpeed << ",right = " << g_rightSpeed << endl;
}

int main(int argc,char **argv)
{
	ros::init(argc,argv,"anbot_wheel");
	ros::NodeHandle nh;

	ros::Publisher  pub = nh.advertise<geometry_msgs::Twist>("real_vel", 50);
//订阅消息
	ros::Subscriber sub = nh.subscribe("cmd_vel",10,&twist_message_received);

	signal(SIGINT,sigfun);
	signal(SIGQUIT,sigfun);

	int ret = wheel_control_init();
	if(ret != 0)
	{
		ROS_INFO_STREAM("wheel_control_init failed !");
		return -1;
	}
	Wheel_Control *pwheel = wheel_control_get_handler();

	ros::Rate r(40);
	while(ros::ok())
	{
		ros::spinOnce();

		pwheel->SetSpeedData(g_leftSpeed,g_rightSpeed);

		speeddatatype nowspeed;
		pwheel->ReadSpeedData(&nowspeed);
		double leftSpeed = (double)nowspeed.leftSpeed/4000.0/17.0*2*M_PI*0.205/0.01;
		double rightSpeed =(double)nowspeed.rightSpeed/4000.0/17.0*2*M_PI*0.205/0.01;

		geometry_msgs::Twist msg;
		msg.linear.x = (leftSpeed+rightSpeed)/2;
		msg.angular.z = (rightSpeed - leftSpeed)/g_wheelR*1000;
		pub.publish(msg);
//        cout << "leftSpeed = " << leftSpeed << ",rightSpeed = "<< rightSpeed << ",angular.z = " << msg.angular.z <<  endl;
		r.sleep();
	}

	wheel_control_deInit();
	pthread_mutex_destroy(&g_tMutex);
	return 0;
}


