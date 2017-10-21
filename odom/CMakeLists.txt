cmake_minimum_required(VERSION 2.8.3)
project(odom)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  sensor_msgs
  geometry_msgs
  tf
  nav_msgs
)

catkin_package()

include_directories(
  ${catkin_INCLUDE_DIRS}
)


add_executable(odom src/odom.cpp)
target_link_libraries(odom ${catkin_LIBRARIES})





