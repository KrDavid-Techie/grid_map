목표: 이 저장소에 ROS 2 Humble용 새 패키지 `grid_map_grit_slam`을 구현해라.
- 입력: `/grit_slam/global_map` (`sensor_msgs/msg/PointCloud2`, 전체 맵, 약 0.2 Hz)
- 처리: 입력 PointCloud2를 기반으로 `GridMap` 생성
- 실행: `ros2 launch grid_map_grit_slam global_map_launch.py`
- 결과: Foxglove에서 바로 볼 수 있도록 `GridMap` 원본과 함께 `OccupancyGrid` 또는 `PointCloud2` 형태의 시각화 토픽도 함께 publish

필수 요구사항:

1. 새 패키지 `grid_map_grit_slam` 추가
2. C++ bridge node 작성
	- `/grit_slam/global_map` subscribe
	- `PointCloud2 -> pcl::PointCloud<pcl::PointXYZ>` 변환
	- `grid_map::GridMapPclLoader` 재사용
	- `setInputCloud()`
	- `preProcessInputCloud()`
	- `initializeGridMapGeometryFromInputCloud()`
	- `addLayerFromInputCloud("elevation")`
	- 결과를 `/grid_map` (`grid_map_msgs/msg/GridMap`) 으로 publish
3. 단독 launch `global_map_launch.py` 작성
	- bridge node 실행
	- `grid_map_visualization` 노드도 함께 실행
	- RViz는 띄우지 말 것
4. Foxglove용 출력 토픽 포함
	- `/grid_map` : `grid_map_msgs/msg/GridMap`
	- `/elevation_points` : `sensor_msgs/msg/PointCloud2`
	- `/elevation_grid` : `nav_msgs/msg/OccupancyGrid`
5. 구현은 기존 라이브러리 재사용 중심으로 할 것
	- `grid_map_pcl` 라이브러리를 직접 링크
	- `grid_map_visualization` 재사용
	- 임시 PCD 파일 저장 같은 우회 금지

권장 구조:

- bridge node는 `/grid_map` 생성만 담당
- `grid_map_visualization` 은 `/grid_map` 을 받아 `/elevation_points`, `/elevation_grid` 를 publish
- Foxglove에서는 `/elevation_points` 와 `/elevation_grid` 를 주로 보게 구성

파라미터화할 것:

- input topic 기본값: `/grit_slam/global_map`
- grid map output topic 기본값: `/grid_map`
- layer name 기본값: `elevation`
- map frame: 기본은 입력 메시지 `header.frame_id`, 필요 시 override 가능
- QoS depth 기본값: 1
- `grid_map_pcl` 처리 파라미터들(resolution, min points per cell, downsampling, outlier removal, cluster extraction)

QoS 및 동작 조건:

- 전체 맵 입력이므로 오래된 메시지를 쌓지 말 것
- 최신 맵 기준으로만 처리
- 입력 토픽이 없어도 노드는 종료되지 말고 대기
- 필요하면 reliability/durability도 파라미터화

참고해서 재사용할 파일:

- `grid_map_pcl/include/grid_map_pcl/GridMapPclLoader.hpp`
- `grid_map_pcl/src/grid_map_pcl_loader_node.cpp`
- `grid_map_pcl/CMakeLists.txt`
- `grid_map_visualization/src/GridMapVisualization.cpp`
- `grid_map_visualization/src/visualizations/PointCloudVisualization.cpp`
- `grid_map_visualization/src/visualizations/OccupancyGridVisualization.cpp`
- `grid_map_demos/launch/simple_demo_launch.py`
- `grid_map_demos/config/simple_demo.yaml`

산출물:

- `grid_map_grit_slam/package.xml`
- `grid_map_grit_slam/CMakeLists.txt`
- bridge node 소스
- `grid_map_grit_slam/launch/global_map_launch.py`
- visualization config yaml
- pcl 처리 파라미터 yaml
- 간단한 사용법 문서

검증 기준:

1. `colcon build --packages-up-to grid_map_grit_slam` 성공
2. `ros2 launch grid_map_grit_slam global_map_launch.py` 실행 가능
3. `/grit_slam/global_map` 이 들어오면 아래 토픽 확인
	- `/grid_map`
	- `/elevation_points`
	- `/elevation_grid`
4. 각 토픽 타입이 기대와 일치
5. Foxglove에서 `/elevation_points` 또는 `/elevation_grid` 로 결과 확인 가능

최종 보고에는 아래를 포함해라.

1. 무엇을 만들었는지
2. 실행 명령
3. 입력/출력 토픽 목록
4. Foxglove에서 볼 토픽
5. 남은 제약사항

---

구현은 끝났습니다. 새 패키지 package.xml 과 CMakeLists.txt 를 추가했고, PointCloud2를 GridMap으로 바꾸는 브리지 노드는 GlobalMapBridge.cpp 와 global_map_bridge_node.cpp 에 들어 있습니다. 단독 실행 launch는 global_map_launch.py, Foxglove용 시각화 설정은 global_map_visualization.yaml, PCL 처리 파라미터는 pcl_parameters.yaml 에 추가했습니다. 사용법 요약은 README.md 에 적어 두었습니다.

실행 명령은 아래입니다.

ros2 launch grid_map_grit_slam global_map_launch.py

입력과 출력은 다음 구조입니다.

- 입력: /grit_slam/global_map, sensor_msgs/msg/PointCloud2
- 원본 결과: /grid_map, grid_map_msgs/msg/GridMap
- Foxglove 시각화용: /elevation_points, sensor_msgs/msg/PointCloud2
- Foxglove 시각화용: /elevation_grid, nav_msgs/msg/OccupancyGrid

구성은 브리지 노드가 /grid_map 만 만들고, grid_map_visualization 이 이를 받아 /elevation_points 와 /elevation_grid 를 재발행하는 방식입니다. Foxglove에서는 주로 /elevation_points 와 /elevation_grid 를 보면 되고, /grid_map 은 raw message 확인용입니다.