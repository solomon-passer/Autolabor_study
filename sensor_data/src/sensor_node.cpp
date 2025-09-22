#include <ros/ros.h>
#include <serial/serial.h>
#include <vector>
#include "sensor_data/SensorData.h"  // Include custom message

int main(int argc, char** argv) {
    // Initialize ROS node
    ros::init(argc, argv, "sensor_data_node");
    ros::NodeHandle nh;
    ros::NodeHandle private_nh("~");

    // Get serial port configuration from parameter server
    std::string port;
    int baudrate;
    private_nh.param<std::string>("port", port, "/dev/ttyUSB0");
    private_nh.param<int>("baudrate", baudrate, 9600);

    // Create publisher using custom message type
    ros::Publisher pub = nh.advertise<sensor_data::SensorData>("/env_sensor_data", 10);

    // Configure serial port
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

    // Check if serial port is open
    if (ser.isOpen()) {
        ROS_INFO_STREAM("Serial port " << port << " opened successfully, baudrate: " << baudrate);
    } else {
        ROS_ERROR_STREAM("Failed to open serial port " << port);
        return -1;
    }

    ros::Rate rate(0.5); // 0.5Hz loop frequency

    while (ros::ok()) {
        // Read serial data, assuming each frame is 17 bytes
        if (ser.available() >= 17) {
            std::vector<uint8_t> buffer(17);
            size_t bytes_read = ser.read(buffer.data(), 17);

            // Check if a complete frame is read
            if (bytes_read == 17) {
                // Check frame header
                if (buffer[0] == 0x3C && buffer[1] == 0x02) {
                    sensor_data::SensorData msg;
                    
                    // Set header information
                    msg.header.stamp = ros::Time::now();
                    static uint32_t seq = 0;
                    msg.header.seq = seq++;
                    msg.header.frame_id = "sensor_frame";

                    // Parse sensor data
                    msg.eco2 = static_cast<float>((buffer[2] << 8) | buffer[3]);
                    msg.ech2o = static_cast<float>((buffer[4] << 8) | buffer[5]);
                    msg.tvoc = static_cast<float>((buffer[6] << 8) | buffer[7]);
                    msg.pm2_5 = static_cast<float>((buffer[8] << 8) | buffer[9]);
                    msg.pm10 = static_cast<float>((buffer[10] << 8) | buffer[11]);

                    // Parse temperature data
                    int temperature_int = buffer[12];
                    float temperature = temperature_int + (buffer[13] / 100.0f);
                    if (temperature_int & 0x80) {  // Handle negative temperature
                        temperature = -temperature;
                    }
                    msg.temperature = temperature;

                    // Parse humidity data
                    msg.humidity = buffer[14] + (buffer[15] / 100.0f);

                    // Publish data
                    pub.publish(msg);
                    
                    // Print debug info (throttled)
                    ROS_INFO_STREAM_THROTTLE(5, "...............data start........");
                } else {
                    ROS_WARN_STREAM("Invalid frame header received, discarding data");
                }
            } else {
                ROS_WARN_STREAM("Incomplete data read, expected 17 bytes, received " << bytes_read << " bytes");
            }
        }

        ros::spinOnce();
        rate.sleep();
    }

    // Close serial port
    if (ser.isOpen()) {
        ser.close();
        ROS_INFO_STREAM("Serial port closed");
    }

    return 0;
}

