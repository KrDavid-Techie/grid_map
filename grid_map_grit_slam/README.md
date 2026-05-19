# grid_map_grit_slam

ROS 2 Humble package that converts PointCloud2 messages into `grid_map_msgs/msg/GridMap` and Foxglove-friendly visualization topics.

## Run

```bash
ros2 launch grid_map_grit_slam global_map_launch.py

ros2 launch grid_map_grit_slam local_map_launch.py
```

## Topics

- Global input default: `/grit_slam/global_map` (`sensor_msgs/msg/PointCloud2`)
- Local input default: `/lidar/cloud_combined` (`sensor_msgs/msg/PointCloud2`)
- Grid map output: `/grid_map` (`grid_map_msgs/msg/GridMap`)
- Visualization output: `/elevation_points` (`sensor_msgs/msg/PointCloud2`)
- Visualization output: `/elevation_grid` (`nav_msgs/msg/OccupancyGrid`)

## Local Map Parameters

- `input_topic`: defaults to `/lidar/cloud_combined`
- `grid_map_topic`: defaults to `/grid_map`
- `layer_name`: defaults to `elevation`
- `map_frame`: defaults to the incoming `header.frame_id` when empty
- `qos_depth`: defaults to `1`
- `input_qos_reliability`: `best_effort` or `reliable`
- `input_qos_durability`: `volatile` or `transient_local`
- `processing_config`: defaults to `config/local_map_pcl_parameters.yaml`

## Notes

- The bridge node reuses `grid_map_pcl::GridMapPclLoader` directly instead of calling the existing PCD loader executable.
- `local_map_launch.py` starts only the bridge and `grid_map_visualization`; it does not start RViz.
- Foxglove should primarily subscribe to `/elevation_points` and `/elevation_grid`.