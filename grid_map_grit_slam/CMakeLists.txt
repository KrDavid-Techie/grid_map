cmake_minimum_required(VERSION 3.8)
project(grid_map_grit_slam)

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(grid_map_msgs REQUIRED)
find_package(grid_map_pcl REQUIRED)
find_package(grid_map_ros REQUIRED)
find_package(PCL REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

include_directories(
  include
  SYSTEM
    ${PCL_INCLUDE_DIRS}
)

link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

set(BRIDGE_COMMON_SOURCES
  src/GlobalMapBridge.cpp
)

add_executable(global_map_bridge_node
  ${BRIDGE_COMMON_SOURCES}
  src/global_map_bridge_node.cpp
)

add_executable(local_map_bridge_node
  ${BRIDGE_COMMON_SOURCES}
  src/local_map_bridge_node.cpp
)

target_compile_features(global_map_bridge_node PUBLIC cxx_std_17)
target_compile_features(local_map_bridge_node PUBLIC cxx_std_17)

target_include_directories(global_map_bridge_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(local_map_bridge_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(global_map_bridge_node
  ament_index_cpp
  grid_map_msgs
  grid_map_pcl
  grid_map_ros
  pcl_conversions
  rclcpp
  sensor_msgs
)

ament_target_dependencies(local_map_bridge_node
  ament_index_cpp
  grid_map_msgs
  grid_map_pcl
  grid_map_ros
  pcl_conversions
  rclcpp
  sensor_msgs
)

target_link_libraries(global_map_bridge_node
  ${PCL_LIBRARIES}
)

target_link_libraries(local_map_bridge_node
  ${PCL_LIBRARIES}
)

install(
  TARGETS global_map_bridge_node local_map_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}
)

install(
  FILES README.md
  DESTINATION share/${PROJECT_NAME}
)

ament_package()