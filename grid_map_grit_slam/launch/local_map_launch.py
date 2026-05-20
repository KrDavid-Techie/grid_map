import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    package_dir = get_package_share_directory('grid_map_grit_slam')

    processing_config = LaunchConfiguration('processing_config')
    visualization_config = LaunchConfiguration('visualization_config')
    input_topic = LaunchConfiguration('input_topic')
    grid_map_topic = LaunchConfiguration('grid_map_topic')
    layer_name = LaunchConfiguration('layer_name')
    map_frame = LaunchConfiguration('map_frame')
    qos_depth = LaunchConfiguration('qos_depth')
    input_qos_reliability = LaunchConfiguration('input_qos_reliability')
    input_qos_durability = LaunchConfiguration('input_qos_durability')
    output_transient_local = LaunchConfiguration('output_transient_local')
    occupancy_data_min = LaunchConfiguration('occupancy_data_min')
    occupancy_data_max = LaunchConfiguration('occupancy_data_max')

    bridge_node = Node(
        package='grid_map_grit_slam',
        executable='local_map_bridge_node',
        name='local_map_bridge',
        output='screen',
        parameters=[{
            'input_topic': input_topic,
            'grid_map_topic': grid_map_topic,
            'layer_name': layer_name,
            'map_frame': map_frame,
            'processing_config_file': processing_config,
            'qos_depth': ParameterValue(qos_depth, value_type=int),
            'input_qos_reliability': input_qos_reliability,
            'input_qos_durability': input_qos_durability,
            'output_transient_local': ParameterValue(output_transient_local, value_type=bool),
        }]
    )

    visualization_node = Node(
        package='grid_map_visualization',
        executable='grid_map_visualization',
        name='grid_map_visualization',
        output='screen',
        parameters=[
            visualization_config,
            {
                'grid_map_topic': grid_map_topic,
                'transient_local': ParameterValue(output_transient_local, value_type=bool),
                'elevation_points.params.layer': layer_name,
                'elevation_grid.params.layer': layer_name,
                'elevation_grid.params.data_min': ParameterValue(occupancy_data_min, value_type=float),
                'elevation_grid.params.data_max': ParameterValue(occupancy_data_max, value_type=float),
            },
        ]
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'processing_config',
            default_value=os.path.join(package_dir, 'config', 'local_map_pcl_parameters.yaml'),
            description='Path to the grid_map_pcl processing configuration file.'),
        DeclareLaunchArgument(
            'visualization_config',
            default_value=os.path.join(package_dir, 'config', 'local_map_visualization.yaml'),
            description='Path to the grid_map_visualization configuration file.'),
        DeclareLaunchArgument(
            'input_topic',
            #default_value='/utlidar/cloud_base',
            default_value='/lidar/cloud_combined',
            description='Input PointCloud2 topic.'),
        DeclareLaunchArgument(
            'grid_map_topic',
            default_value='/grid_map',
            description='Output GridMap topic.'),
        DeclareLaunchArgument(
            'layer_name',
            default_value='elevation',
            description='GridMap layer used for elevation.'),
        DeclareLaunchArgument(
            'map_frame',
            default_value='',
            description='Optional frame override for the generated GridMap.'),
        DeclareLaunchArgument(
            'qos_depth',
            default_value='1',
            description='QoS queue depth for input subscription and GridMap publisher.'),
        DeclareLaunchArgument(
            'input_qos_reliability',
            default_value='best_effort',
            description='Input PointCloud2 QoS reliability: best_effort or reliable.'),
        DeclareLaunchArgument(
            'input_qos_durability',
            default_value='volatile',
            description='Input PointCloud2 QoS durability: volatile or transient_local.'),
        DeclareLaunchArgument(
            'output_transient_local',
            default_value='true',
            description='Whether GridMap and visualization subscriptions use transient local durability.'),
        DeclareLaunchArgument(
            'occupancy_data_min',
            default_value='-1.0',
            description='Minimum elevation value used when converting to OccupancyGrid.'),
        DeclareLaunchArgument(
            'occupancy_data_max',
            default_value='2.0',
            description='Maximum elevation value used when converting to OccupancyGrid.'),
        bridge_node,
        visualization_node,
    ])