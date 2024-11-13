cmake_minimum_required(VERSION 3.5)
project( DisplayImage )
set (CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

SET( MAIN_EXEC
     main
     ColorDistribution
     )

add_executable( main main.cpp ColorDistribution.h ColorDistribution.cpp )
target_link_libraries( main ${OpenCV_LIBS} )
