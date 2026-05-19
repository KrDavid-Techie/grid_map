#ifndef GRID_MAP_GRIT_SLAM__GLOBALMAPBRIDGE_HPP_
#define GRID_MAP_GRIT_SLAM__GLOBALMAPBRIDGE_HPP_

#include <grid_map_msgs/msg/grid_map.hpp>
#include <grid_map_pcl/GridMapPclLoader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

#include <mutex>
#include <string>

namespace grid_map_grit_slam
{

struct BridgeDefaults
{
  std::string nodeName{"global_map_bridge"};
  std::string inputTopic{"/grit_slam/global_map"};
  std::string gridMapTopic{"/grid_map"};
  std::string layerName{"elevation"};
  std::string processingConfigFile{"pcl_parameters.yaml"};
  bool outputTransientLocal{true};
};

class GlobalMapBridge : public rclcpp::Node
{
public:
  explicit GlobalMapBridge(
    const BridgeDefaults & defaults = BridgeDefaults(),
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
  void pointCloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr message);
  void loadProcessingConfig(const std::string & processingConfigPath);
  rclcpp::QoS makeInputQos() const;
  static std::string getDefaultProcessingConfigPath(const std::string & fileName);

  BridgeDefaults defaults_;
  std::string inputTopic_;
  std::string gridMapTopic_;
  std::string layerName_;
  std::string mapFrameOverride_;
  std::string inputQosReliability_;
  std::string inputQosDurability_;
  int qosDepth_;
  bool outputTransientLocal_;

  grid_map::GridMapPclLoader loader_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr pointCloudSubscriber_;
  rclcpp::Publisher<grid_map_msgs::msg::GridMap>::SharedPtr gridMapPublisher_;
  std::mutex loaderMutex_;
};

}  // namespace grid_map_grit_slam

#endif  // GRID_MAP_GRIT_SLAM__GLOBALMAPBRIDGE_HPP_