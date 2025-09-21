#!/usr/bin/env python3
import rospy
import yaml
from visualization_msgs.msg import Marker, MarkerArray
from geometry_msgs.msg import Point, Pose, Quaternion

def load_waypoints():
    rospy.init_node('waypoints_visualizer')
    pub = rospy.Publisher('/waypoints_markers', MarkerArray, queue_size=10)

    # 读取 YAML 文件
    yaml_path = rospy.get_param('~yaml_path', 'default/path/to/waypoints.yaml')
    with open(yaml_path, 'r') as file:
        waypoints_data = yaml.safe_load(file)

    marker_array = MarkerArray()
    for idx, wp in enumerate(waypoints_data['waypoints']):
        # 创建 Marker（点）
        marker = Marker()
        marker.header.frame_id = wp['frame_id']
        marker.header.stamp = rospy.Time.now()
        marker.ns = "waypoints"
        marker.id = idx
        marker.type = Marker.SPHERE
        marker.action = Marker.ADD
        
        # 设置位置和方向（将字典转换为 ROS 消息）
        marker.pose = Pose()
        marker.pose.position = Point(**wp['pose']['position'])
        marker.pose.orientation = Quaternion(**wp['pose']['orientation'])
        
        marker.scale.x = 1.0
        marker.scale.y = 1.0
        marker.scale.z = 1.0
        marker.color.a = 1.0
        marker.color.r = 0.0
        marker.color.g = 1.0
        marker.color.b = 0.0
        marker_array.markers.append(marker)

        # 创建 Marker（文字标签）
        text_marker = Marker()
        text_marker.header.frame_id = wp['frame_id']
        text_marker.header.stamp = rospy.Time.now()
        text_marker.ns = "waypoints_text"
        text_marker.id = idx + 1000
        text_marker.type = Marker.TEXT_VIEW_FACING
        text_marker.action = Marker.ADD
        
        # 设置文字位置（基于点的位置，但抬高 Z 轴）
        text_marker.pose = Pose()
        text_marker.pose.position = Point(**wp['pose']['position'])
        text_marker.pose.position.z += 0.5  # 抬高文字避免重叠
        text_marker.pose.orientation.w = 1.0  # 默认朝向
        
        text_marker.scale.z = 0.3
        text_marker.color.a = 1.0
        text_marker.color.r = 1.0
        text_marker.color.g = 1.0
        text_marker.color.b = 1.0
        text_marker.text = wp['name']
        marker_array.markers.append(text_marker)

    rospy.loginfo("Publishing waypoints to RViz...")
    rate = rospy.Rate(1)
    while not rospy.is_shutdown():
        pub.publish(marker_array)
        rate.sleep()

if __name__ == '__main__':
    try:
        load_waypoints()
    except rospy.ROSInterruptException:
        pass
