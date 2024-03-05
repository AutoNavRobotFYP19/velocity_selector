/*
 * Author       : Dhanuja Jayasinghe
 * Data         : 29-01-2024
 * Description  : 
 */

// #include "utility.hpp"
#include <ros/ros.h>
#include <std_msgs/Int32.h>
#include <geometry_msgs/Twist.h>

class VelocitySelector {

    private:
    ros::NodeHandle nh;
    geometry_msgs::Twist twist_cmd;
    ros::Publisher vel_final_topic;
    ros::Subscriber cmd_vel_topic;
    ros::Subscriber cam_vel_topic;
    double update_frequency = 20.0; //20Hz
    int status = 0; // 0 -> publish smoothed_cmd_vel | 1 -> publish cam_vel
    int count = 0;
    bool start_count = false;

    void smoothed_cmd_vel_cb (const geometry_msgs::Twist::ConstPtr& msg) 
    {
        if (!status) {
            // ROS_INFO("publish smoothed_cmd_vel");
            twist_cmd.linear.x = msg->linear.x;
            twist_cmd.linear.y = msg->linear.y;
            twist_cmd.linear.z = msg->linear.z;

            twist_cmd.angular.x = msg->angular.x;
            twist_cmd.angular.y = msg->angular.y;
            twist_cmd.angular.z = msg->angular.z;
        }
        
    }

    void cam_vel_cb (const geometry_msgs::Twist::ConstPtr& msg) 
    {
        // ROS_INFO("/cam_vel msg received");
        // if (msg->linear.x != 0) {
        //     status = 1;
        // } else if (msg->linear.x == 0) {
        //     status = 0;
        // }
        ROS_INFO("\u001b[31mNavigation using Camera\u001b[47m");
        status = 1;
        start_count = true;

        // publish /cam_vel when an object is detected
        if (status) {
            twist_cmd.linear.x = msg->linear.x;
            twist_cmd.linear.y = msg->linear.y;
            twist_cmd.linear.z = msg->linear.z;

            twist_cmd.angular.x = msg->angular.x;
            twist_cmd.angular.y = msg->angular.y;
            twist_cmd.angular.z = msg->angular.z;
        }
    }

    void publish_velocity () 
    {
        // ROS_INFO("Linear x = %f \t Angular x = %f", twist_cmd.linear.x, twist_cmd.angular.z);
        vel_final_topic.publish(twist_cmd);
    }

    public:
    //Class Constructor
    VelocitySelector() 
    {
        // ROS publisher topic
        vel_final_topic = nh.advertise<geometry_msgs::Twist>("vel_final", 10);
        cmd_vel_topic = nh.subscribe("smoothed_cmd_vel", 50, &VelocitySelector::smoothed_cmd_vel_cb, this);
        cam_vel_topic = nh.subscribe("cam_vel", 50, &VelocitySelector::cam_vel_cb, this);
    }

    void waiting_loop()
    {
        ros::Rate rate(update_frequency);
        while (ros::ok())
        {
            publish_velocity();
            if (count > 100) {
                ROS_INFO("\u001b[32mSwitch Back to Navigation Stack\u001b[47m");
                status = 0;
                count = 0;
                start_count = false;
            }
            if (start_count) {
                count += 1;
            }
            ros::spinOnce();
            rate.sleep();
        }
    }

};

int main (int argc, char** argv) {

    ros::init(argc, argv, "velocity_selector");

    VelocitySelector velo_sel;

    ROS_INFO("\033[1;32m---->\033[0m Velocity Selector Started.");

    velo_sel.waiting_loop();

    return 0;
}