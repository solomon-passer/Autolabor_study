cmake_minimum_required(VERSION 3.0.2)
project(gas_heatmap)

## 开启 C++11 编译（适配 ROS Melodic/Noetic，避免语法兼容问题）
add_compile_options(-std=c++11)

## 查找依赖包（保持原依赖不变，确保覆盖所有需求）
find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  message_filters
  message_generation
  message_runtime
  nav_msgs
  roscpp
  sensor_msgs
  std_msgs
)

## 声明自定义消息文件（关键：启用 GasData.msg 生成）
add_message_files(
  FILES
  GasData.msg  # 对应你创建的自定义气体传感器消息
)

## 生成消息（依赖 std_msgs，因为 GasData 字段基于 std_msgs 类型）
generate_messages(
  DEPENDENCIES
  std_msgs  # 必须添加，否则消息生成会失败
)

## 声明 catkin 包配置（传递依赖给其他包，确保消息运行时可用）
catkin_package(
  # INCLUDE_DIRS include  # 若后续添加头文件目录，可取消注释并补充路径
  # LIBRARIES gas_heatmap  # 若后续添加库文件，可取消注释并补充
  CATKIN_DEPENDS 
    geometry_msgs 
    message_filters 
    message_runtime 
    nav_msgs 
    roscpp 
    sensor_msgs
  # DEPENDS system_lib  # 若依赖系统库（如 Boost），可补充
)

## 头文件目录（关联 catkin 依赖的头文件）
include_directories(
  # include  # 若后续添加自定义头文件，可取消注释并补充路径
  ${catkin_INCLUDE_DIRS}
)

## 编译 C++ 节点（关键：生成 gas_data_logger 可执行文件）
add_executable(gas_data_logger 
  src/gas_data_logger.cpp  # 你的数据同步+CSV保存节点源码
)

## 节点依赖（确保消息生成在节点编译之前，避免链接错误）
add_dependencies(gas_data_logger 
  ${${PROJECT_NAME}_EXPORTED_TARGETS}  # 依赖自定义消息生成目标
  ${catkin_EXPORTED_TARGETS}          # 依赖 catkin 其他包的生成目标
)

## 链接库（关联 catkin 依赖的库文件）
target_link_libraries(gas_data_logger
  ${catkin_LIBRARIES}
)

## （可选）安装节点到系统路径（方便 rosrun 直接调用，建议添加）
install(TARGETS gas_data_logger
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}  # 安装到 ROS 二进制目录
)

## （可选）安装自定义消息（确保其他包可引用，建议添加）
install(DIRECTORY msg/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/msg
)

