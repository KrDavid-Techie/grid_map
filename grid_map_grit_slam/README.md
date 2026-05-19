# grid_map_grit_slam

ROS 2 Humble package that converts `/grit_slam/global_map` PointCloud2 messages into `grid_map_msgs/msg/GridMap` and Foxglove-friendly visualization topics.

## Run

```bash
ros2 launch grid_map_grit_slam global_map_launch.py

ros2 launch grid_map_grit_slam local_map_launch.py
```

## Topics

- Input: `/grit_slam/global_map` (`sensor_msgs/msg/PointCloud2`)
- Output: `/grid_map` (`grid_map_msgs/msg/GridMap`)
- Visualization: `/elevation_points` (`sensor_msgs/msg/PointCloud2`)
- Visualization: `/elevation_grid` (`nav_msgs/msg/OccupancyGrid`)

## Notes

- The bridge node reuses `grid_map_pcl::GridMapPclLoader` directly instead of calling the existing PCD loader executable.
- Foxglove should primarily subscribe to `/elevation_points` and `/elevation_grid`.