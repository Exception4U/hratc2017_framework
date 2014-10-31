/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2014, ISR University of Coimbra.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the ISR University of Coimbra nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
* Author: Goncalo Cabrita on 21/04/2014
*********************************************************************/


#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <tf/message_filter.h>
#include <message_filters/subscriber.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

using namespace std;

class RobotPose
{
public:
    RobotPose(string targetFrame, string inputTopic)
    {
        target_frame_ = targetFrame;
        input_topic_ = inputTopic;

        sub_.subscribe(n_, input_topic_, 10);
        tf_filter_ = new tf::MessageFilter<geometry_msgs::PoseWithCovarianceStamped>(sub_, tf_, target_frame_, 10);
        tf_filter_->registerCallback( boost::bind(&RobotPose::msgCallback, this, _1) );
    }

    const geometry_msgs::PoseStamped & GetGlobalPose()
    {
        return globalPose_;
    }

    const geometry_msgs::PoseStamped & GetLocalPose()
    {
        return localPose_;
    }

private:
    message_filters::Subscriber<geometry_msgs::PoseWithCovarianceStamped> sub_;
    tf::TransformListener tf_;
    tf::MessageFilter<geometry_msgs::PoseWithCovarianceStamped> * tf_filter_;
    ros::NodeHandle n_;
    string target_frame_;
    string input_topic_;
    geometry_msgs::PoseStamped globalPose_;
    geometry_msgs::PoseStamped localPose_;


    //  Callback to register with tf::MessageFilter to be called when transforms are available
    void msgCallback(const boost::shared_ptr<const geometry_msgs::PoseWithCovarianceStamped>& msg)
    {
        globalPose_.header.frame_id = msg->header.frame_id;
//        cout << globalPose_.header.frame_id << ' ' << target_frame_ << endl;
        globalPose_.header.stamp = msg->header.stamp;
        globalPose_.pose.position.x = msg->pose.pose.position.x;
        globalPose_.pose.position.y = msg->pose.pose.position.y;
        globalPose_.pose.position.z = msg->pose.pose.position.z;
        globalPose_.pose.orientation = msg->pose.pose.orientation;

        try
        {
            tf_.transformPose(target_frame_, globalPose_, localPose_);
//            ROS_INFO("Robot is at x:%lf y:%lf yaw:%lf", pose_in.pose.position.x, pose_in.pose.position.y, tf::getYaw(pose_in.pose.orientation));
//            ROS_INFO("Robot is at x:%lf y:%lf yaw:%lf", pose_out.pose.position.x, pose_out.pose.position.y, tf::getYaw(pose_out.pose.orientation));
        }
        catch (tf::TransformException &ex)
        {
            ROS_WARN("Failure %s\n", ex.what());
        }
    }
};

