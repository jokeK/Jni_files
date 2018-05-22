# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#设置so库路径  CMAKE_SOURCE_DIR就是cmake.txt的绝对路径
set(my_lib_path ${CMAKE_SOURCE_DIR}/libs)
#将第三方库作为动态库引用
#add_library(third-party-lib
#            SHARED
#            IMPORTED
#            )
 #指明第三方库的路径 ANDROID_ABI 动态的平台名称(arm64/armeabi-v7a等)
#set_target_properties(third-party-lib
#                      PROPERTIES IMPORTED_LOCATION
#                      ${my_lib_path}/${ANDROID_ABI}/libnative-lib.so)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#本地main/cpp下的c和cpp文件生成的库名和文件路径
#就是自己编写的JNI文件
add_library( # Sets the name of the library.
             my_jni

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-jni.c )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
#系统环境的库
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       my_jni
                       #third-party-lib
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )