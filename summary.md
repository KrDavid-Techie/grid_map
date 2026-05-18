# grid_map 정리

## 0. 한 줄 요약

grid_map은 ROS 2 Humble에서 사용할 수 있는 2.5D 멀티레이어 지도 라이브러리다. 기본 출력은 경로(path)가 아니라 `grid_map_msgs/msg/GridMap` 형태의 지도 데이터이며, 필요하면 `nav_msgs/msg/OccupancyGrid`, `sensor_msgs/msg/PointCloud2`, `visualization_msgs/msg/Marker`, 혹은 costmap 형태로 변환해서 planner 쪽 입력으로 쓸 수 있다.

## 1. ROS 2 Humble 환경에서 어떻게 실행하나

### 1-1. 공식 지원 전제

- 저장소 README 기준으로 이 프로젝트는 ROS 2 Humble + Ubuntu Jammy 22.04 환경에서 테스트됐다.
- 현재 작업 환경은 macOS이지만, 저장소 문서 기준의 공식 실행 경로는 Ubuntu Jammy 22.04다.
- 따라서 실제 실행은 Ubuntu, Docker, 혹은 Linux VM/WSL2 같은 Humble 환경에서 하는 것이 안전하다.

근거:

- [README.md](README.md)

### 1-2. 빌드 절차

저장소 README에 있는 기준 절차는 아래와 같다.

```bash
source /opt/ros/humble/setup.bash

sudo apt-get install libeigen3-dev

mkdir -p ~/gridmap_dep/src
cd ~/gridmap_dep
wget https://raw.githubusercontent.com/ANYbotics/grid_map/humble/tools/ros2_dependencies.repos
vcs import src < ros2_dependencies.repos
rosdep install -y --ignore-src --from-paths src
colcon build --symlink-install --packages-up-to pcl_ros

source ~/gridmap_dep/install/setup.bash

mkdir -p ~/gridmap_ws/src
cd ~/gridmap_ws/src
git clone https://github.com/anybotics/grid_map.git --branch humble
cd ../
rosdep install -y --ignore-src --from-paths src
colcon build --symlink-install
```

성능을 위해서는 Release 빌드가 권장된다.

```bash
colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release
```

### 1-3. 가장 간단한 실행 예시

빌드 후에는 overlay를 source한 다음 데모 launch를 실행하면 된다.

```bash
source ~/gridmap_ws/install/setup.bash

ros2 launch grid_map_demos simple_demo_launch.py
```

이 launch는 다음 3개를 같이 띄운다.

- `grid_map_demos/simple_demo`: 내부에서 elevation 레이어를 생성해 `/grid_map`으로 발행
- `grid_map_visualization/grid_map_visualization`: `/grid_map`을 읽어서 시각화용 토픽으로 재발행
- `rviz2`: 미리 준비된 RViz 설정으로 실행

근거:

- [README.md](README.md)
- [grid_map_demos/launch/simple_demo_launch.py](grid_map_demos/launch/simple_demo_launch.py)
- [grid_map_demos/src/simple_demo_node.cpp](grid_map_demos/src/simple_demo_node.cpp)
- [grid_map_demos/config/simple_demo.yaml](grid_map_demos/config/simple_demo.yaml)

### 1-4. 실사용에 가까운 데모 실행 예시

#### A. 단순 생성/시각화 확인

```bash
ros2 launch grid_map_demos simple_demo_launch.py
```

용도:

- 설치 확인
- `/grid_map` 발행 확인
- RViz 시각화 확인

#### B. 이미지 -> GridMap -> Traversability 예시

```bash
ros2 launch grid_map_demos filters_demo_launch.py
```

이 launch는 내부적으로 다음을 모두 올린다.

- `image_publisher.py`: `terrain.png`를 `image` 토픽으로 발행
- `image_to_gridmap_demo`: 이미지를 `/grid_map`으로 변환
- `filters_demo`: `/grid_map`을 받아 `/filtered_map`으로 필터링 결과 발행
- `grid_map_visualization`: `/filtered_map`을 occupancy grid, normal vector 등으로 재발행
- `rviz2`

근거:

- [grid_map_demos/launch/filters_demo_launch.py](grid_map_demos/launch/filters_demo_launch.py)
- [grid_map_demos/config/filters_demo.yaml](grid_map_demos/config/filters_demo.yaml)
- [grid_map_demos/config/filters_demo_filter_chain.yaml](grid_map_demos/config/filters_demo_filter_chain.yaml)
- [grid_map_demos/src/FiltersDemo.cpp](grid_map_demos/src/FiltersDemo.cpp)

#### C. 이미지 입력만 보고 싶을 때

```bash
ros2 launch grid_map_demos image_to_gridmap_demo_launch.py
```

이 launch는 `eth_logo.png`를 입력 이미지로 발행하고, 그것을 grid map으로 바꿔 RViz에서 보여준다.

근거:

- [grid_map_demos/launch/image_to_gridmap_demo_launch.py](grid_map_demos/launch/image_to_gridmap_demo_launch.py)
- [grid_map_demos/src/ImageToGridmapDemo.cpp](grid_map_demos/src/ImageToGridmapDemo.cpp)
- [grid_map_demos/config/image_to_gridmap_demo.yaml](grid_map_demos/config/image_to_gridmap_demo.yaml)

#### D. rosbag에 저장된 GridMap 다시 띄우기

```bash
ros2 launch grid_map_demos grid_map_loader_demo_launch.py
```

이 launch는 데모 데이터인 `grid_map_bag`를 읽어 `/grid_map`으로 다시 발행한다.

근거:

- [grid_map_demos/launch/grid_map_loader_demo_launch.py](grid_map_demos/launch/grid_map_loader_demo_launch.py)
- [grid_map_loader/src/GridMapLoader.cpp](grid_map_loader/src/GridMapLoader.cpp)
- [grid_map_demos/config/grid_map_loader_demo.yaml](grid_map_demos/config/grid_map_loader_demo.yaml)

#### E. OctoMap -> GridMap 변환

```bash
ros2 launch grid_map_demos octomap_to_gridmap_demo_launch.py
```

주의:

- README에 이 데모는 현재 정상 동작하지 않는다고 명시되어 있다.
- 이유는 `octomap_server` 포팅 이슈 때문이다.

근거:

- [README.md](README.md)
- [grid_map_demos/src/OctomapToGridmapDemo.cpp](grid_map_demos/src/OctomapToGridmapDemo.cpp)
- [grid_map_demos/config/octomap_to_gridmap_demo.yaml](grid_map_demos/config/octomap_to_gridmap_demo.yaml)

## 2. 실행 시 필요한 input 값은 무엇인가

grid_map 전체를 하나의 애플리케이션으로 보면 “항상 필요한 단일 입력”이 있는 구조는 아니다. 어떤 데모/노드를 쓰느냐에 따라 입력이 달라진다.

### 2-1. 공통적으로 필요한 것

- ROS 2 Humble 환경 source
- `grid_map` 및 의존 패키지 빌드 완료
- 실행 전 overlay source

### 2-2. 외부 입력이 없어도 되는 데모

아래 데모는 노드 내부에서 지도를 직접 생성하므로 외부 센서 입력이 없어도 된다.

- `simple_demo`
- `tutorial_demo`
- `iterators_demo`
- `interpolation_demo`
- `move_demo`
- `resolution_change_demo`
- `opencv_demo`

예를 들어 `simple_demo`는 내부에서 `elevation` 레이어를 계산해서 바로 `/grid_map`을 발행한다.

근거:

- [grid_map_demos/src/simple_demo_node.cpp](grid_map_demos/src/simple_demo_node.cpp)
- [grid_map_demos/src/tutorial_demo_node.cpp](grid_map_demos/src/tutorial_demo_node.cpp)
- [grid_map_demos/src/IteratorsDemo.cpp](grid_map_demos/src/IteratorsDemo.cpp)
- [grid_map_demos/src/InterpolationDemo.cpp](grid_map_demos/src/InterpolationDemo.cpp)

### 2-3. 이미지 기반 입력이 필요한 경우

`image_to_gridmap_demo`는 `sensor_msgs/msg/Image` 입력이 필요하다.

주요 파라미터:

- `image_topic`: 기본 `/image`
- `resolution`
- `min_height`
- `max_height`

launch 파일을 그대로 쓰면 외부 카메라 없이도 내부 `image_publisher.py`가 이미지를 올려주므로 바로 실행 가능하다.

근거:

- [grid_map_demos/src/ImageToGridmapDemo.cpp](grid_map_demos/src/ImageToGridmapDemo.cpp)
- [grid_map_demos/config/image_to_gridmap_demo.yaml](grid_map_demos/config/image_to_gridmap_demo.yaml)
- [grid_map_demos/launch/image_to_gridmap_demo_launch.py](grid_map_demos/launch/image_to_gridmap_demo_launch.py)

### 2-4. 필터 체인을 쓰는 경우

`filters_demo`는 입력으로 `grid_map_msgs/msg/GridMap` 토픽이 필요하다.

주요 파라미터:

- `input_topic`: 필수, 예제에서는 `/grid_map`
- `output_topic`: 기본 `output`, 예제에서는 `/filtered_map`
- `filter_chain_parameter_name`: 기본 `filters`

실제 필터 파라미터 예시:

- inpaint 반경
- normal vector 계산 반경
- slope, roughness, traversability 수식
- threshold 범위

즉, filters_demo는 “원본 GridMap”을 입력으로 받고 “가공된 GridMap”을 출력한다.

근거:

- [grid_map_demos/src/FiltersDemo.cpp](grid_map_demos/src/FiltersDemo.cpp)
- [grid_map_demos/config/filters_demo_filter_chain.yaml](grid_map_demos/config/filters_demo_filter_chain.yaml)

### 2-5. OctoMap 기반 입력이 필요한 경우

`octomap_to_gridmap_demo`는 토픽이 아니라 서비스 입력이 필요하다.

주요 입력:

- `octomap_service_topic`: 기본 `/octomap_binary`
- 선택적 bounding box 파라미터: `min_x`, `max_x`, `min_y`, `max_y`, `min_z`, `max_z`

즉, 이 데모는 외부 OctoMap 서버가 떠 있어야 의미가 있다.

근거:

- [grid_map_demos/src/OctomapToGridmapDemo.cpp](grid_map_demos/src/OctomapToGridmapDemo.cpp)

### 2-6. Bag 파일을 다시 발행하는 경우

`grid_map_loader`는 rosbag 경로가 필요하다.

주요 파라미터:

- `file_path`
- `bag_topic`
- `publish_topic`
- `duration`
- `qos_transient_local`

즉, bag 파일 안에 저장된 GridMap 메시지를 읽어 다시 ROS 2 토픽으로 뿌리는 용도다.

근거:

- [grid_map_loader/src/GridMapLoader.cpp](grid_map_loader/src/GridMapLoader.cpp)
- [grid_map_demos/launch/grid_map_loader_demo_launch.py](grid_map_demos/launch/grid_map_loader_demo_launch.py)

## 3. ROS 2 topic 값으로 나와서 visualization 할 수 있나

가능하다. 이 저장소는 시각화를 두 가지 방식으로 제공한다.

### 3-1. 직접 GridMap 메시지를 RViz 플러그인으로 보기

- `grid_map_rviz_plugin`은 `grid_map_msgs/GridMap` 메시지를 직접 표시하는 RViz 플러그인이다.
- README에도 grid map을 3D surface plot으로 RViz에 렌더링한다고 되어 있다.

근거:

- [README.md](README.md)
- [grid_map_rviz_plugin/package.xml](grid_map_rviz_plugin/package.xml)
- [grid_map_rviz_plugin/plugin_description.xml](grid_map_rviz_plugin/plugin_description.xml)

### 3-2. GridMap을 표준 ROS 메시지로 변환해서 보기

`grid_map_visualization` 노드는 `grid_map_topic`으로 GridMap을 구독한 뒤, 설정에 따라 여러 표준 메시지로 재발행한다.

대표 예시는 아래와 같다.

- `point_cloud` -> `sensor_msgs/msg/PointCloud2`
- `flat_point_cloud` -> `sensor_msgs/msg/PointCloud2`
- `occupancy_grid` -> `nav_msgs/msg/OccupancyGrid`
- `vectors` -> `visualization_msgs/msg/Marker`
- `map_region` -> `visualization_msgs/msg/Marker`

중요한 점은 재발행 토픽 이름이 시각화 이름 자체라는 점이다. 예를 들면:

- `elevation_points` 설정 -> `elevation_points` 토픽 발행
- `elevation_grid` 설정 -> `elevation_grid` 토픽 발행
- `surface_normals` 설정 -> `surface_normals` 토픽 발행
- `traversability_grid` 설정 -> `traversability_grid` 토픽 발행

즉, GridMap 원본 토픽 하나를 받아서 RViz가 잘 이해하는 토픽들로 바꿔 보여줄 수 있다.

근거:

- [grid_map_visualization/src/GridMapVisualization.cpp](grid_map_visualization/src/GridMapVisualization.cpp)
- [grid_map_visualization/src/visualizations/PointCloudVisualization.cpp](grid_map_visualization/src/visualizations/PointCloudVisualization.cpp)
- [grid_map_visualization/src/visualizations/OccupancyGridVisualization.cpp](grid_map_visualization/src/visualizations/OccupancyGridVisualization.cpp)
- [grid_map_visualization/src/visualizations/VectorVisualization.cpp](grid_map_visualization/src/visualizations/VectorVisualization.cpp)
- [grid_map_visualization/src/visualizations/FlatPointCloudVisualization.cpp](grid_map_visualization/src/visualizations/FlatPointCloudVisualization.cpp)

### 3-3. 실제 예시 토픽 흐름

#### simple_demo

- 입력: 없음
- 원본 출력: `/grid_map` (`grid_map_msgs/msg/GridMap`)
- 시각화 출력: `elevation_points` (`PointCloud2`), `elevation_grid` (`OccupancyGrid`)

근거:

- [grid_map_demos/src/simple_demo_node.cpp](grid_map_demos/src/simple_demo_node.cpp)
- [grid_map_demos/config/simple_demo.yaml](grid_map_demos/config/simple_demo.yaml)

#### filters_demo

- 입력: `/grid_map`
- 필터 결과: `/filtered_map` (`grid_map_msgs/msg/GridMap`)
- 시각화 출력: `surface_normals` (`Marker`), `traversability_grid` (`OccupancyGrid`)

근거:

- [grid_map_demos/config/filters_demo.yaml](grid_map_demos/config/filters_demo.yaml)
- [grid_map_demos/src/FiltersDemo.cpp](grid_map_demos/src/FiltersDemo.cpp)

#### image_to_gridmap_demo

- 입력: `/image` (`sensor_msgs/msg/Image`)
- 원본 출력: `grid_map` (`grid_map_msgs/msg/GridMap`)
- 시각화 출력: `elevation_points`, `elevation_grid`, `flat_grid`

근거:

- [grid_map_demos/src/ImageToGridmapDemo.cpp](grid_map_demos/src/ImageToGridmapDemo.cpp)
- [grid_map_demos/config/image_to_gridmap_demo.yaml](grid_map_demos/config/image_to_gridmap_demo.yaml)

## 4. 최종 output은 무엇이고, path planning에서 어떻게 활용하나

### 4-1. 이 저장소의 기본 출력은 path가 아니다

핵심 출력 메시지는 [grid_map_msgs/msg/GridMap.msg](grid_map_msgs/msg/GridMap.msg) 에 정의된 `GridMap`이다.

이 메시지는 아래 성격을 가진다.

- 지도 좌표계와 시간 정보가 있다.
- 여러 레이어 이름을 동시에 담을 수 있다.
- 각 레이어는 2차원 float 배열 형태 데이터다.
- `basic_layers` 개념으로 셀 유효성 기준도 가진다.

즉, 최종 출력은 “경로 계획 결과”가 아니라 “다층 지도 표현”이다.

### 4-2. costmap인가, path plan 그 자체인가

정확히 말하면 기본 출력은 “GridMap”이지 “path plan”도 아니고 “Nav2 costmap 그 자체”도 아니다.

다만 다음처럼 활용할 수 있다.

- elevation map으로 사용
- traversability map으로 사용
- occupancy grid로 변환해 장애물 표현으로 사용
- costmap 계열 데이터로 변환해 planner 입력으로 사용

특히 저장소에는 `grid_map_costmap_2d` 패키지가 따로 있고, 설명도 “grid maps to the costmap_2d format interface”다. 즉, planner가 직접 쓰는 costmap 형식으로 이어붙일 수는 있지만, 이 저장소 자체가 경로를 계산해 주지는 않는다.

근거:

- [README.md](README.md)
- [grid_map_costmap_2d/package.xml](grid_map_costmap_2d/package.xml)

### 4-3. path planning 관점에서 이해하면

실무적으로는 아래처럼 보는 게 맞다.

1. 센서/이미지/OctoMap/bag 등으로부터 GridMap 생성
2. 필요한 레이어 계산
	- 예: elevation, normal, slope, roughness, traversability
3. 필요하면 occupancy grid 또는 costmap 형태로 변환
4. 그 결과를 Nav2 같은 planner의 입력으로 사용

즉, grid_map은 planner 자체가 아니라 planner 앞단의 “지도 표현 및 전처리 계층”에 가깝다.

### 4-4. filters_demo를 planning 관점으로 보면

`filters_demo`는 planning 입력에 가장 가까운 예시다.

- 입력: `/grid_map`
- 내부 계산: `elevation_inpainted`, `elevation_smooth`, `normal_vectors_*`, `slope`, `roughness`, `edges`, `traversability`
- 출력: `/filtered_map`
- 추가 시각화: `traversability_grid` (`OccupancyGrid`)

즉, 여기서 나오는 `traversability` 레이어는 planner가 사용할 수 있는 비용/주행 가능도 정보에 가깝다. 하지만 여전히 “경로” 자체는 아니다.

근거:

- [grid_map_demos/config/filters_demo_filter_chain.yaml](grid_map_demos/config/filters_demo_filter_chain.yaml)
- [grid_map_demos/config/filters_demo.yaml](grid_map_demos/config/filters_demo.yaml)

## 5. 결론

- ROS 2 Humble에서의 공식 기준은 Ubuntu Jammy 22.04 + colcon 빌드다.
- 가장 쉬운 실행은 `ros2 launch grid_map_demos simple_demo_launch.py` 이다.
- 입력은 데모별로 다르며, simple demo는 입력이 없고 image/octomap/bag 기반 데모는 각각 이미지 토픽, OctoMap 서비스, bag 경로가 필요하다.
- 출력은 주로 `grid_map_msgs/msg/GridMap` 이며, RViz 플러그인 또는 `grid_map_visualization`을 통해 PointCloud2, OccupancyGrid, Marker로 시각화할 수 있다.
- grid_map은 path planner가 아니라 다층 지도 표현 라이브러리다. path planning에서는 cost/traversability/elevation 정보를 제공하는 upstream map representation으로 보는 것이 맞다.

## 6. PointCloud2 전체 맵 topic을 입력으로 쓰고, Foxglove로 결과를 보고 싶을 때

### 6-1. 먼저 결론

가능하다. 다만 이 저장소에는 `sensor_msgs/msg/PointCloud2` topic을 직접 subscribe해서 바로 `GridMap`으로 바꿔 주는 완성형 ROS 2 노드는 기본 제공되지 않는다.

현재 저장소에 있는 것은 두 가지다.

- `grid_map_pcl`: point cloud를 grid map으로 바꾸는 알고리즘/라이브러리
- `grid_map_pcl_loader_node`: PCD 파일을 읽어서 grid map으로 만드는 예제 노드

즉, live topic 입력을 쓰려면 얇은 브리지 노드 하나를 추가해서 `PointCloud2 -> PCL -> GridMap` 흐름으로 연결해야 한다.

근거:

- [grid_map_pcl/src/grid_map_pcl_loader_node.cpp](grid_map_pcl/src/grid_map_pcl_loader_node.cpp)
- [grid_map_pcl/launch/grid_map_pcl_loader_launch.py](grid_map_pcl/launch/grid_map_pcl_loader_launch.py)
- [grid_map_pcl/include/grid_map_pcl/GridMapPclLoader.hpp](grid_map_pcl/include/grid_map_pcl/GridMapPclLoader.hpp)

### 6-2. 권장 파이프라인

입력 topic이 예를 들어 `/full_map_points` 라고 하면 권장 흐름은 아래와 같다.

```text
/full_map_points (sensor_msgs/msg/PointCloud2, 0.2 Hz)
	-> custom bridge node
	-> /grid_map (grid_map_msgs/msg/GridMap)
	-> /elevation_points (sensor_msgs/msg/PointCloud2)
	-> /elevation_grid (nav_msgs/msg/OccupancyGrid)
	-> Foxglove
```

실무적으로는 다음 3개를 함께 내보내는 것이 좋다.

- 원본 입력 point cloud: 입력 검증용
- grid_map 결과: 알고리즘 결과 원본
- visualization friendly 토픽: Foxglove 표시용

### 6-3. 왜 바로 Foxglove에서 `GridMap`을 보지 않고 변환 토픽을 같이 내보내는가

Foxglove에서 가장 확실하게 보기 쉬운 것은 표준 메시지다.

- `sensor_msgs/msg/PointCloud2`
- `nav_msgs/msg/OccupancyGrid`
- `visualization_msgs/msg/Marker`

반면 `grid_map_msgs/msg/GridMap`은 저장소 내부에서는 핵심 메시지이지만, Foxglove에서 바로 “지형 맵”으로 렌더링하는 표준 패널 입력은 아니다. 따라서 결과를 안정적으로 확인하려면 `GridMap`을 그대로 하나 publish하고, 동시에 `PointCloud2`나 `OccupancyGrid`로 변환해서 같이 publish하는 편이 맞다.

### 6-4. 구현 방식

이 저장소 기준으로는 아래 순서로 노드를 만들면 된다.

1. `sensor_msgs/msg/PointCloud2` subscribe
2. 메시지를 PCL cloud로 변환
3. `grid_map::GridMapPclLoader`에 입력 cloud 설정
4. `preProcessInputCloud()` 실행
5. `initializeGridMapGeometryFromInputCloud()` 실행
6. `addLayerFromInputCloud("elevation")` 실행
7. 결과를 `grid_map_msgs/msg/GridMap`으로 publish
8. 추가로 `PointCloud2` 또는 `OccupancyGrid`로 변환해서 publish

`GridMapPclLoader`는 이미 live cloud를 코드에서 직접 넣을 수 있게 `setInputCloud()`를 제공한다.

근거:

- [grid_map_pcl/include/grid_map_pcl/GridMapPclLoader.hpp](grid_map_pcl/include/grid_map_pcl/GridMapPclLoader.hpp)
- [grid_map_pcl/include/grid_map_pcl/helpers.hpp](grid_map_pcl/include/grid_map_pcl/helpers.hpp)
- [grid_map_pcl/src/helpers.cpp](grid_map_pcl/src/helpers.cpp)
- [grid_map_ros/include/grid_map_ros/GridMapRosConverter.hpp](grid_map_ros/include/grid_map_ros/GridMapRosConverter.hpp)

### 6-5. 최소 예시 코드 구조

개념적으로는 아래와 같은 노드가 필요하다.

```cpp
class PointCloudToGridMapNode : public rclcpp::Node {
public:
	PointCloudToGridMapNode()
	: Node("pointcloud_to_grid_map"),
		loader_(this->get_logger())
	{
		sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
			"/full_map_points", rclcpp::QoS(1),
			std::bind(&PointCloudToGridMapNode::callback, this, std::placeholders::_1));

		grid_map_pub_ = create_publisher<grid_map_msgs::msg::GridMap>("/grid_map", rclcpp::QoS(1).transient_local());
		elevation_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>("/elevation_points", rclcpp::QoS(1).transient_local());
		occ_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>("/elevation_grid", rclcpp::QoS(1).transient_local());

		loader_.loadParameters("/path/to/parameters.yaml");
	}

private:
	void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg)
	{
		pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>());
		pcl::fromROSMsg(*msg, *cloud);

		loader_.setInputCloud(cloud);
		loader_.preProcessInputCloud();
		loader_.initializeGridMapGeometryFromInputCloud();
		loader_.addLayerFromInputCloud("elevation");

		auto map = loader_.getGridMap();

		auto grid_map_msg = grid_map::GridMapRosConverter::toMessage(map);
		grid_map_pub_->publish(std::move(grid_map_msg));

		sensor_msgs::msg::PointCloud2 elevation_cloud;
		grid_map::GridMapRosConverter::toPointCloud(map, "elevation", elevation_cloud);
		elevation_pub_->publish(elevation_cloud);

		nav_msgs::msg::OccupancyGrid occupancy_grid;
		grid_map::GridMapRosConverter::toOccupancyGrid(map, "elevation", -1.0, 2.0, occupancy_grid);
		occ_pub_->publish(occupancy_grid);
	}
};
```

주의:

- 위 코드는 개념 예시다.
- 실제 빌드에는 `pcl_conversions` 또는 적절한 PCL ROS 변환 헤더가 추가로 필요할 수 있다.
- `parameters.yaml`은 `grid_map_pcl`의 기존 파라미터 파일 형식을 재사용하면 된다.

### 6-6. Foxglove에서는 무엇을 보면 되나

Foxglove에서는 아래처럼 보는 것이 가장 실용적이다.

#### 3D Panel

- `/full_map_points`: 원본 전체 point cloud
- `/elevation_points`: grid_map에서 elevation layer를 다시 point cloud로 변환한 결과
- 필요하면 Marker 기반 normal/vector 토픽도 추가

#### Map 또는 3D Panel

- `/elevation_grid`: elevation을 occupancy grid처럼 정규화한 결과
- traversability 레이어를 만들었다면 `/traversability_grid`가 planner 관점에서 더 유용함

#### Raw Messages Panel

- `/grid_map`: 원본 `grid_map_msgs/msg/GridMap` 확인용

즉, Foxglove에서 사람 눈으로 확인할 때는 `/elevation_points` 또는 `/traversability_grid`를 보는 것이 가장 편하고, `/grid_map`은 디버깅용 원본으로 같이 남겨두는 구성이 좋다.

### 6-7. 0.2 Hz 입력일 때 주의할 점

0.2 Hz는 5초에 한 번 전체 맵이 들어온다는 뜻이다. 이 경우 핵심은 “처리 시간이 5초 안에 끝나느냐”다.

권장 사항:

- subscription QoS depth는 1로 두기
- 최신 맵만 유지하고 오래된 full map은 쌓지 않기
- point cloud가 매우 크면 downsampling 파라미터를 먼저 적용하기
- full map 전체를 매번 다시 계산하는 비용이 크면, 후속 단계는 `OccupancyGrid`만 publish하도록 단순화하기

`grid_map_pcl` 자체도 downsampling, outlier removal, cluster extraction 관련 파라미터를 이미 제공한다.

근거:

- [grid_map_pcl/README.md](grid_map_pcl/README.md)

### 6-8. path planning 용도로 보고 싶다면

Foxglove에서 결과 확인만 하려면 `elevation_points`면 충분하다. 하지만 planner 입력까지 생각하면 elevation만으로는 부족할 수 있다.

추천 흐름:

1. PointCloud2 -> GridMap elevation 생성
2. 필요하면 `grid_map_filters`로 slope, roughness, traversability 계산
3. `traversability` 레이어를 `OccupancyGrid` 또는 costmap 계열 값으로 변환
4. Foxglove에서는 `/traversability_grid`를 보고, planner에는 해당 결과를 입력

이 구성이 가장 planner 친화적이다.

### 6-9. 정리

- PointCloud2 topic을 입력으로 쓰는 것은 가능하다.
- 하지만 이 저장소에는 topic subscriber 완성 노드가 없고, PCD 기반 loader 예제만 있다.
- 따라서 `GridMapPclLoader::setInputCloud()`를 사용하는 custom bridge node를 하나 두는 것이 정석이다.
- Foxglove에서는 `GridMap` 원본보다 `PointCloud2`와 `OccupancyGrid`로 변환한 결과를 보는 것이 가장 쉽다.
- 결과를 planning에 쓰려면 elevation map에서 끝내지 말고 traversability/cost layer까지 계산하는 편이 좋다.
