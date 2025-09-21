#!/usr/bin/env python3
import rospy
import yaml
import os
from visualization_msgs.msg import Marker, MarkerArray
from geometry_msgs.msg import Point, Pose, Quaternion

def load_waypoints():
    rospy.init_node('waypoints_visualizer')
    pub = rospy.Publisher('/waypoints_markers', MarkerArray, queue_size=10)

    # 构造 YAML 路径（与脚本同目录）
    script_dir = os.path.dirname(os.path.abspath(__file__))
    yaml_path = os.path.join(script_dir, '..', 'config', 'singapore_points.yaml')

    if not os.path.exists(yaml_path):
        rospy.logerr(f"YAML file not found: {yaml_path}")
        return

    with open(yaml_path, 'r') as file:
        waypoints_data = yaml.safe_load(file)

    marker_array = MarkerArray()
    for idx, wp in enumerate(waypoints_data['waypoints']):
        # 创建 Marker（箭头）
        marker = Marker()
        marker.header.frame_id = wp['frame_id']
        marker.header.stamp = rospy.Time.now()
        marker.ns = "waypoints_arrow"
        marker.id = idx
        marker.type = Marker.ARROW
        marker.action = Marker.ADD

        marker.pose = Pose()
        marker.pose.position = Point(**wp['pose']['position'])
        marker.pose.orientation = Quaternion(**wp['pose']['orientation'])

        marker.scale.x = 1.6  # 箭头长度
        marker.scale.y = 0.3  # 箭头宽度
        marker.scale.z = 0.15 # 箭头高度
        marker.color.a = 1.0
        marker.color.r = 0.0
        marker.color.g = 0.8
        marker.color.b = 1.0
        marker_array.markers.append(marker)

        # 文字标签
        text_marker = Marker()
        text_marker.header.frame_id = wp['frame_id']
        text_marker.header.stamp = rospy.Time.now()
        text_marker.ns = "waypoints_text"
        text_marker.id = idx + 1000
        text_marker.type = Marker.TEXT_VIEW_FACING
        text_marker.action = Marker.ADD

        text_marker.pose = Pose()
        text_marker.pose.position = Point(**wp['pose']['position'])
        text_marker.pose.position.z += 0.5
        text_marker.pose.orientation.w = 1.0

        text_marker.scale.z = 0.3
        text_marker.color.a = 1.0
        text_marker.color.r = 1.0
        text_marker.color.g = 1.0
        text_marker.color.b = 0.0
        text_marker.text = wp['name']
        marker_array.markers.append(text_marker)

    rospy.loginfo("Publishing arrow markers from singapore_points.yaml...")
    rate = rospy.Rate(1)
    while not rospy.is_shutdown():
        pub.publish(marker_array)
        rate.sleep()

if __name__ == '__main__':
    try:
        load_waypoints()
    except rospy.ROSInterruptException:
        pass

