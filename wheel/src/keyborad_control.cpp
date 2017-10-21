#include <ros/ros.h>
#include <termios.h>  
#include <signal.h>  
#include <math.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <sys/poll.h> 
#include <geometry_msgs/Twist.h>

#define KEYCODE_W 0x77  
#define KEYCODE_A 0x61  
#define KEYCODE_S 0x73  
#define KEYCODE_D 0x64 
#define KEYCODE_SPACE 0x20

int main(int argc,char **argv)
{
	ros::init(argc,argv,"key_control_wheel");
	ros::NodeHandle nh;

	ros::Publisher  pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 50);
	
	ros::Rate r(25);

	int kfd = 0;
	char c;
	bool dirty = false;
	struct termios cooked, raw;
	tcsetattr(kfd, TCSANOW, &cooked);
	tcgetattr(kfd, &cooked);  
    memcpy(&raw, &cooked, sizeof(struct termios));  
    raw.c_lflag &=~ (ICANON | ECHO);  
    raw.c_cc[VEOL] = 1;  
    raw.c_cc[VEOF] = 2;  
    tcsetattr(kfd, TCSANOW, &raw);  
	struct pollfd ufd;  
    ufd.fd = kfd;  
    ufd.events = POLLIN;  
 
	while(ros::ok())
	{
		int num;   
		geometry_msgs::Twist msg;
        if ((num = poll(&ufd, 1, 250)) < 0)  
        {  
            perror("poll():");  
            return -1;  
        }  
        else if(num > 0)  
        {  
            if(read(kfd, &c, 1) < 0)  
            {  
                perror("read():");  
                return -1;  
            }  
        }  
        else  
        {  
            if (dirty == true)  
            {  
               // stopRobot();  
                dirty = false;  
            }  
            continue;  
        }
//		printf("%x\n",c);
		switch(c)  
        {  
            case KEYCODE_W:
				msg.linear.x=0.1;
				break;
			case KEYCODE_A:
				msg.angular.z=0.2;
				break;
			case KEYCODE_S:
				msg.linear.x=-0.1;
				break;
			case KEYCODE_D:
				msg.angular.z=-0.2;
				break;
			case KEYCODE_SPACE:
				break;
			default:
				continue;
				break;
		}
		pub.publish(msg);
		r.sleep();
	}
	
	return 0;
}


