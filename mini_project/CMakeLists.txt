cmake_minimum_required(VERSION 3.5)

# 把$ENV{IDF_PATH}/tools/cmake/project.cmake中的内容 复制到本文件中
# 这个project.cmake文件其实里面是一大段cmake命令，这个文件直接确定了ESP-IDF工程的结构,project.cmake会去找main文件。
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 当前目录是工程的根目录 且工程名称叫minimal_project
project(minimal_project)
