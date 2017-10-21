#include <iostream>
#include <std_msgs/Int32.h>
#include <pthread.h>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "sensor_msgs/LaserScan.h"
#include "uart_driver.h"
using namespace std;

int startOrStop = 1;	// 1是start，0是stop
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void publish_scan(ros::Publisher *pub, double *dist, int count, ros::Time start, double scan_time)
{
	static int scan_count = 0;
	sensor_msgs::LaserScan scan_msg;
	scan_msg.header.stamp = start;
	scan_msg.header.frame_id = "base_laser";
	scan_count++;

	scan_msg.angle_min = 3.1415926;
	scan_msg.angle_max = -3.1415926;
	scan_msg.angle_increment = (scan_msg.angle_max - scan_msg.angle_min) / (double)(count - 1);
	scan_msg.scan_time = scan_time;
	scan_msg.time_increment = scan_time / (double)(count - 1);
	
	scan_msg.range_min = 0.15;
	scan_msg.range_max = 6.0;

	scan_msg.intensities.resize(count);
	scan_msg.ranges.resize(count);


	for (int i = 0; i < count; i++)
	{
		if (dist[i] == 0.0)
			scan_msg.ranges[i] = std::numeric_limits<float>::infinity();
		else
			scan_msg.ranges[i] = dist[i] / 1000.0;
		scan_msg.intensities[i] = 0;
	}
	pub->publish(scan_msg);
}

void startStopCB(const std_msgs::Int32 msg)
{
	pthread_mutex_lock(&mutex);
	startOrStop = msg.data;
	printf("startorstop = %d\n",startOrStop);
	pthread_mutex_unlock(&mutex);
}

int main(int argv, char **argc)
{
	ros::init(argv, argc, "laser");
	ros::NodeHandle n;

	ros::Publisher  scan_pub = n.advertise<sensor_msgs::LaserScan>("scan", 1000);
	ros::Subscriber stop_sub = n.subscribe<std_msgs::Int32>("laser_startOrStop", 10, startStopCB);

	io_driver driver;
  	int ret = driver.OpenSerial(B230400);
	if(ret < 0 )
	{
		ROS_INFO_STREAM("anbot_laser openSerial error");
		return -1;
	}
        driver.StartScan();
	bool isStarted = true;
	
	double angle[360 * 2];
	double distance[360 * 2];
	double data[360 * 2];
	double speed;
	int count = 0;
		
	ros::Time starts = ros::Time::now();
	ros::Time ends = ros::Time::now();
	ROS_INFO("anbot_laser....");
	while(ros::ok())
	{
		ros::spinOnce();

		pthread_mutex_lock(&mutex);
		if(isStarted && 0 == startOrStop) // 当前正在扫描且要求停止
		{
			//ROS_INFO("stop");
			driver.StopScan();
			isStarted = false;
		}
		else if(!isStarted && 1 == startOrStop)	// 当前未扫描且要求开始扫描
		{
			//ROS_INFO("start");
			driver.StartScan();
			isStarted = true;
		}
		pthread_mutex_unlock(&mutex);

		//ROS_INFO("%s", isStarted ? "Started":"Stopped");
		if(!isStarted)
			continue;


		memset(data, 0, sizeof(data));
		int ret = driver.GetScanData(angle, distance, PACKLEN, &speed);
		for (int i = 0; i < ret; i++)
		{
			data[(int)(angle[i]+0.5)] = distance[i];
			//printf("i = %d ,double = %f\n",(int)(angle[i]+0.5),distance[i]);
		}
		ends = ros::Time::now();
		float scan_duration = (ends - starts).toSec() * 1e-3;
		publish_scan(&scan_pub, data, 360, starts, scan_duration);
		starts = ends;
	}
	return 0;
}


