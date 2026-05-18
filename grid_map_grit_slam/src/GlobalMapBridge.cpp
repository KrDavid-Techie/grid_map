#include "grid_map_grit_slam/GlobalMapBridge.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <grid_map_ros/GridMapRosConverter.hpp>
#include <pcl/PCLPointCloud2.h>
#include <pcl/conversions.h>
#include <pcl_conversions/pcl_conversions.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <algorithm>

namespace grid_map_grit_slam
{

GlobalMapBridge::GlobalMapBridge(const rclcpp::NodeOptions & options)
: Node("global_map_bridge", options),
  loader_(this->get_logger())
{
  this->declare_parameter("input_topic", std::string("/grit_slam/global_map"));
  this->declare_parameter("grid_map_topic", std::string("/grid_map"));
  this->declare_parameter("layer_name", std::string("elevation"));
  this->declare_parameter("map_frame", std::string(""));
  this->declare_parameter("processing_config_file", getDefaultProcessingConfigPath());
  this->declare_parameter("qos_depth", 1);
  this->declare_parameter("input_qos_reliability", std::string("best_effort"));
  this->declare_parameter("input_qos_durability", std::string("volatile"));
  this->declare_parameter("output_transient_local", true);

  this->get_parameter("input_topic", inputTopic_);
  this->get_parameter("grid_map_topic", gridMapTopic_);
  this->get_parameter("layer_name", layerName_);
  this->get_parameter("map_frame", mapFrameOverride_);
  this->get_parameter("input_qos_reliability", inputQosReliability_);
  this->get_parameter("input_qos_durability", inputQosDurability_);
  this->get_parameter("qos_depth", qosDepth_);
  this->get_parameter("output_transient_local", outputTransientLocal_);

  std::string processingConfigPath;
  this->get_parameter("processing_config_file", processingConfigPath);
  loadProcessingConfig(processingConfigPath);

  auto outputQos = rclcpp::QoS(static_cast<size_t>(std::max(qosDepth_, 1)));
  if (outputTransientLocal_) {
    outputQos.transient_local();
  }

  gridMapPublisher_ = this->create_publisher<grid_map_msgs::msg::GridMap>(
    gridMapTopic_, outputQos);

  pointCloudSubscriber_ = this->create_subscription<sensor_msgs::msg::PointCloud2>(
    inputTopic_, makeInputQos(),
    std::bind(&GlobalMapBridge::pointCloudCallback, this, std::placeholders::_1));

  RCLCPP_INFO(
    this->get_logger(),
    "Listening for global map point clouds on '%s' and publishing GridMap on '%s'.",
    inputTopic_.c_str(), gridMapTopic_.c_str());
}

void GlobalMapBridge::pointCloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr message)
{
  if (message->data.empty()) {
    RCLCPP_WARN_THROTTLE(
      this->get_logger(), *this->get_clock(), 5000,
      "Received an empty PointCloud2 message on '%s'.", inputTopic_.c_str());
    return;
  }

  auto cloud = std::make_shared<grid_map::GridMapPclLoader::Pointcloud>();
  pcl::PCLPointCloud2 pclCloud;
  pcl_conversions::toPCL(*message, pclCloud);
  pcl::fromPCLPointCloud2(pclCloud, *cloud);

  if (cloud->empty()) {
    RCLCPP_WARN_THROTTLE(
      this->get_logger(), *this->get_clock(), 5000,
      "Converted PointCloud2 message on '%s' contains no XYZ points.", inputTopic_.c_str());
    return;
  }

  std::lock_guard<std::mutex> lock(loaderMutex_);

  try {
    loader_.setInputCloud(cloud);
    loader_.preProcessInputCloud();
    loader_.initializeGridMapGeometryFromInputCloud();
    loader_.addLayerFromInputCloud(layerName_);

    grid_map::GridMap map(loader_.getGridMap());
    map.setBasicLayers({layerName_});
    map.setTimestamp(rclcpp::Time(message->header.stamp).nanoseconds());

    if (!mapFrameOverride_.empty()) {
      map.setFrameId(mapFrameOverride_);
    } else if (!message->header.frame_id.empty()) {
      map.setFrameId(message->header.frame_id);
    } else {
      map.setFrameId("map");
    }

    auto gridMapMessage = grid_map::GridMapRosConverter::toMessage(map);
    gridMapPublisher_->publish(std::move(gridMapMessage));

    RCLCPP_INFO_THROTTLE(
      this->get_logger(), *this->get_clock(), 5000,
      "Published GridMap on '%s'.", gridMapTopic_.c_str());
  } catch (const std::exception & exception) {
    RCLCPP_ERROR_THROTTLE(
      this->get_logger(), *this->get_clock(), 5000,
      "Failed to convert PointCloud2 to GridMap: %s", exception.what());
  }
}

void GlobalMapBridge::loadProcessingConfig(const std::string & processingConfigPath)
{
  if (processingConfigPath.empty() || !std::filesystem::exists(processingConfigPath)) {
    throw std::runtime_error(
            "Processing config file not found: " + processingConfigPath);
  }

  loader_.loadParameters(processingConfigPath);
}

rclcpp::QoS GlobalMapBridge::makeInputQos() const
{
  auto qos = rclcpp::QoS(static_cast<size_t>(std::max(qosDepth_, 1)));

  if (inputQosReliability_ == "best_effort") {
    qos.best_effort();
  } else if (inputQosReliability_ == "reliable") {
    qos.reliable();
  }

  if (inputQosDurability_ == "transient_local") {
    qos.transient_local();
  }

  return qos;
}

std::string GlobalMapBridge::getDefaultProcessingConfigPath()
{
  return ament_index_cpp::get_package_share_directory("grid_map_grit_slam") +
         "/config/pcl_parameters.yaml";
}

}  // namespace grid_map_grit_slam