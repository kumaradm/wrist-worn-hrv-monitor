cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS components/drivers components/network components/services)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wrist_worn_hrv_monitor)
