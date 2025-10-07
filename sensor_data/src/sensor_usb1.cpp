#include <ros/ros.h>
#include <serial/serial.h>
#include <vector>
#include "sensor_data/SensorData.h"  // Include custom message

// 帧定义
const uint8_t FRAME_HEAD1 = 0x3C;
const uint8_t FRAME_HEAD2 = 0x02;
const uint16_t FRAME_LEN = 17;

// 校验和计算
bool checkChecksum(const std::vector<uint8_t>& frame) {
    uint8_t sum = 0;
    for (size_t i = 0; i < FRAME_LEN - 1; ++i) {
        sum += frame[i];
    }
    return sum == frame[FRAME_LEN - 1];
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "sensor_data_node");
    ros::NodeHandle nh;
    ros::NodeHandle private_nh("~");

    // 串口参数
    std::string port;
    int baudrate;
    private_nh.param<std::string>("port", port, "/dev/ttyUSB1");  // 默认新串口
    private_nh.param<int>("baudrate", baudrate, 9600);

    // 创建发布者
    ros::Publisher pub = nh.advertise<sensor_data::SensorData>("/env_sensor_data", 10);

    // 配置串口
    serial::Serial ser;
    try {
        ser.setPort(port);
        ser.setBaudrate(baudrate);
        serial::Timeout to = serial::Timeout::simpleTimeout(100);
        ser.setTimeout(to);
        ser.open();
    } catch (serial::IOException& e) {
        ROS_ERROR_STREAM("Failed to open serial port " << port << ": " << e.what());
        return -1;
    }

    if (ser.isOpen()) {
        ROS_INFO_STREAM("Serial port " << port << " opened successfully, baudrate: " << baudrate);
    } else {
        ROS_ERROR_STREAM("Failed to open serial port " << port);
        return -1;
    }

    std::vector<uint8_t> frameBuffer;
    frameBuffer.reserve(FRAME_LEN);

    while (ros::ok()) {
        if (ser.available() > 0) {
            uint8_t byte;
            ser.read(&byte, 1);
            
            // 寻找帧头
            if (frameBuffer.empty() && byte != FRAME_HEAD1) {
                continue;
            }
            
            frameBuffer.push_back(byte);
            
            // 检查帧头第二字节
            if (frameBuffer.size() == 2) {
                if (frameBuffer[0] != FRAME_HEAD1 || frameBuffer[1] != FRAME_HEAD2) {
                    frameBuffer.clear();
                    continue;
                }
            }
            
            // 完整帧
            if (frameBuffer.size() == FRAME_LEN) {
                if (checkChecksum(frameBuffer)) {
                    sensor_data::SensorData msg;
                    msg.header.stamp = ros::Time::now();
                    static uint32_t seq = 0;
                    msg.header.seq = seq++;
                    msg.header.frame_id = "sensor_frame";

                    // 解析数据
                    msg.eco2 = static_cast<float>((frameBuffer[2] << 8) | frameBuffer[3]);
                    msg.ech2o = static_cast<float>((frameBuffer[4] << 8) | frameBuffer[5]);
                    msg.tvoc = static_cast<float>((frameBuffer[6] << 8) | frameBuffer[7]);
                    msg.pm2_5 = static_cast<float>((frameBuffer[8] << 8) | frameBuffer[9]);
                    msg.pm10 = static_cast<float>((frameBuffer[10] << 8) | frameBuffer[11]);

                    // 温度
                    int temp_int = frameBuffer[12];
                    float temperature = temp_int + (frameBuffer[13] / 100.0f);
                    if (temp_int & 0x80) {
                        temperature = -((temp_int & 0x7F) + (frameBuffer[13] / 100.0f));
                    }
                    msg.temperature = temperature;

                    // 湿度
                    msg.humidity = frameBuffer[14] + (frameBuffer[15] / 100.0f);

                    pub.publish(msg);
                    ROS_INFO_STREAM_THROTTLE(5, "Published sensor data");
                } else {
                    ROS_WARN_STREAM("Checksum error, discarding frame");
                }
                frameBuffer.clear();
            }
        }
        ros::spinOnce();
    }

    if (ser.isOpen()) {
        ser.close();
        ROS_INFO_STREAM("Serial port closed");
    }

    return 0;
}

