#include <ros/ros.h>
#include <serial/serial.h>
#include <iomanip> 

int main(int argc, char** argv) {
    ros::init(argc, argv, "serial_raw_viewer");
    ros::NodeHandle nh;
    ros::NodeHandle private_nh("~");

    std::string port;
    int baudrate;
    private_nh.param<std::string>("port", port, "/dev/ttyUSB0");
    private_nh.param<int>("baudrate", baudrate, 9600);

    serial::Serial ser;
    try {
        ser.setPort(port);
        ser.setBaudrate(baudrate);
        serial::Timeout to = serial::Timeout::simpleTimeout(1000);
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

    ros::Rate rate(100); 
    const size_t FRAME_SIZE = 17; // 一帧数据的字节数

    // 记录开始时间
    ros::Time start_time = ros::Time::now();
    const double duration_sec = 3.0; // 持续时间 5 秒

    while (ros::ok()) {
        // 检查是否超过 5 秒
        if ((ros::Time::now() - start_time).toSec() > duration_sec) {
            ROS_INFO_STREAM("3 seconds elapsed. Stopping...");
            break;
        }

        // 等待串口缓冲区中有至少一帧（17字节）的数据
        if (ser.available() >= FRAME_SIZE) {
            std::vector<uint8_t> buffer(FRAME_SIZE);
            size_t bytes_read = ser.read(buffer.data(), FRAME_SIZE);

            if (bytes_read == FRAME_SIZE) {
                ROS_INFO_STREAM("Read " << bytes_read << " bytes (full frame):");
                for (size_t i = 0; i < bytes_read; ++i) {
                    ROS_INFO_STREAM(std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]) << " ");
                }
                ROS_INFO_STREAM("");
            } else {
                ROS_WARN_STREAM("Expected " << FRAME_SIZE << " bytes, but read " << bytes_read << " bytes");
            }
        }
        ros::spinOnce();
        rate.sleep();
    }

    if (ser.isOpen()) {
        ser.close();
        ROS_INFO_STREAM("Serial port closed");
    }

    return 0;
}

