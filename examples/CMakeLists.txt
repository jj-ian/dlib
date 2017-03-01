#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#


cmake_minimum_required(VERSION 2.8.4)

PROJECT(examples)
set(CMAKE_CXX_STANDARD 11) 

include(../dlib/cmake)

# cpr stuff
add_subdirectory(cpr)


   include_directories(${CPR_INCLUDE_DIRS})
# end cpr stuff

# Tell CMake to compile a program.  We do this with the ADD_EXECUTABLE()
# statement which takes the name of the output executable and then a list of
# .cpp files to compile.  Here each example consists of only one .cpp file but
# in general you will make programs that const of many .cpp files.
ADD_EXECUTABLE(assignment_learning_ex assignment_learning_ex.cpp)
# Then we tell it to link with dlib.
TARGET_LINK_LIBRARIES(assignment_learning_ex dlib)




# Since there are a lot of examples I'm going to use a macro to simply this
# CMakeLists.txt file.  However, usually you will create only one executable in
# your cmake projects and use the syntax shown above.
MACRO(add_example name)
   ADD_EXECUTABLE(${name} ${name}.cpp)
   TARGET_LINK_LIBRARIES(${name} dlib )
ENDMACRO()


#here we apply our macros 
find_package(OpenCV)
if (OpenCV_FOUND)
   include_directories(${OpenCV_INCLUDE_DIRS})

   ADD_EXECUTABLE(webcam_face_pose_ex webcam_face_pose_ex.cpp)
   TARGET_LINK_LIBRARIES(webcam_face_pose_ex dlib ${OpenCV_LIBS} )

   ADD_EXECUTABLE(webcam_head_pose webcam_head_pose.cpp)
   TARGET_LINK_LIBRARIES(webcam_head_pose dlib ${OpenCV_LIBS} ${CPR_LIBRARIES} )
   #target_link_libraries(webcam_face_pose dlib ${CPR_LIBRARIES})

	 ADD_EXECUTABLE(webcam_face_pose_fast webcam_face_pose_fast)
   TARGET_LINK_LIBRARIES(webcam_face_pose_fast dlib ${OpenCV_LIBS} )


   ADD_EXECUTABLE(face_landmark_detection_to_file face_landmark_detection_to_file.cpp)
   TARGET_LINK_LIBRARIES(face_landmark_detection_to_file dlib ${OpenCV_LIBS} )
endif()


if (DLIB_LINK_WITH_SQLITE3)
   add_example(sqlite_ex)
endif()


