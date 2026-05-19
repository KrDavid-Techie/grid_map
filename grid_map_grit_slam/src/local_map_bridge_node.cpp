#include "grid_map_grit_slam/GlobalMapBridge.hpp"

#include <memory>

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto bridgeDefaults = grid_map_grit_slam::BridgeDefaults{};
  bridgeDefaults.nodeName = "local_map_bridge";
  bridgeDefaults.inputTopic = "/lidar/cloud_combined";
  bridgeDefaults.processingConfigFile = "local_map_pcl_parameters.yaml";

  rclcpp::spin(std::make_shared<grid_map_grit_slam::GlobalMapBridge>(bridgeDefaults));
  rclcpp::shutdown();
  return 0;
}