cmake_minimum_required(VERSION 3.0.2)
project(sensor_data)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  sensor_msgs
  message_generation
  serial
)

add_message_files(
  FILES
  SensorData.msg
)

generate_messages(
  DEPENDENCIES
  std_msgs
  sensor_msgs
)

catkin_package(
  CATKIN_DEPENDS 
  roscpp 
  rospy 
  std_msgs 
  sensor_msgs
  message_runtime
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${serial_INCLUDE_DIRS}
)

add_executable(sensor_node src/sensor_node.cpp)
add_dependencies(sensor_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(sensor_node
  ${catkin_LIBRARIES}
  ${serial_LIBRARIES}
)

add_executable(serial_raw_viewer src/serial_raw_viewer.cpp)
target_link_libraries(serial_raw_viewer
  ${catkin_LIBRARIES}
)

