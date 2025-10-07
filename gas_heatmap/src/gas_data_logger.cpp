#include <ros/ros.h>
#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>  // 正确的消息类型
#include "gas_heatmap/GasData.h"
#include <fstream>
#include <iomanip>

using namespace message_filters;
std::ofstream outfile;
bool file_header_written = false;

void syncCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& amcl_msg,
                  const gas_heatmap::GasData::ConstPtr& gas_msg)
{
    if (!file_header_written) {
        // 仅保留数据列名（作为CSV首行，符合标准格式）
        outfile << "timestamp,x,y,eco2,ech2o,tvoc,pm2_5,pm10,temperature,humidity" << std::endl;
        file_header_written = true;
    }

    // 获取机器人坐标
    double x = amcl_msg->pose.pose.position.x;
    double y = amcl_msg->pose.pose.position.y;

    // 写入数据（固定 6 位小数）
    outfile << std::fixed << std::setprecision(6)
            << amcl_msg->header.stamp.toSec() << ","
            << x << "," << y << ","
            << gas_msg->eco2 << ","
            << gas_msg->ech2o << ","
            << gas_msg->tvoc << ","
            << gas_msg->pm2_5 << ","
            << gas_msg->pm10 << ","
            << gas_msg->temperature << ","
            << gas_msg->humidity << std::endl;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "gas_data_logger");
    ros::NodeHandle nh;

    // 设置地图参数（与你的地图路径匹配）—— 保留原逻辑，不修改
    nh.setParam("/map_resolution", 0.02);
    nh.setParam("/map_origin_x", -20.0);
    nh.setParam("/map_origin_y", -20.0);
    nh.setParam("/map_pgm_path", "/home/agilex/agilex_ws/src/limo_ros/limo_bringup/maps/map1.pgm");

    // 打开 CSV 文件（保存在当前运行目录）—— 保留原逻辑
    outfile.open("gas_inspection_data.csv");
    if (!outfile.is_open()) {
        ROS_ERROR("Failed to open CSV file!");
        return -1;
    }

    // 订阅话题（正确的消息类型）—— 保留原逻辑
    Subscriber<geometry_msgs::PoseWithCovarianceStamped> amcl_sub(nh, "/amcl_pose", 10);
    Subscriber<gas_heatmap::GasData> gas_sub(nh, "/env_sensor_data", 10);

    // 时间同步器（匹配订阅的消息类型）—— 保留原逻辑
    using MySyncPolicy = sync_policies::ApproximateTime<
        geometry_msgs::PoseWithCovarianceStamped, gas_heatmap::GasData>;
    Synchronizer<MySyncPolicy> sync(MySyncPolicy(10), amcl_sub, gas_sub);
    sync.registerCallback(boost::bind(&syncCallback, _1, _2));

    ROS_INFO("Gas data logger started. Saving to gas_inspection_data.csv");
    ros::spin();
    outfile.close();
    return 0;
}

