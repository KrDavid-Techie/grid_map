#include "grid_map_grit_slam/GlobalMapBridge.hpp"

#include <memory>

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<grid_map_grit_slam::GlobalMapBridge>());
  rclcpp::shutdown();
  return 0;
}