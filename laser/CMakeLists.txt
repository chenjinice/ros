cmake_minimum_required(VERSION 2.8.3)
project(laser)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  sensor_msgs
)

#find_package(Boost REQUIRED signals)

catkin_package()

include_directories(
 # ${Boost_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)

add_executable(laser src/laser.cpp src/uart_driver.cpp)
target_link_libraries(laser ${catkin_LIBRARIES})




