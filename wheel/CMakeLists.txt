cmake_minimum_required(VERSION 2.8.3)
project(wheel)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  sensor_msgs
  geometry_msgs
  nav_msgs
)

#find_package(Boost REQUIRED signals)

catkin_package()

include_directories(
 # ${Boost_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)

add_executable(wheel src/wheel.cpp src/wheel_control.cpp src/comm.cpp)
target_link_libraries(wheel ${catkin_LIBRARIES})

add_executable(keyboard_control src/keyborad_control.cpp)
target_link_libraries(keyboard_control ${catkin_LIBRARIES})





