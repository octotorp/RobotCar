cmake_minimum_required(VERSION 3.20)
project(car_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Жорстко вказуємо Python і версію API:
set(Python3_EXECUTABLE "C:/Users/Vlad/AppData/Local/Python/pythoncore-3.15-64/python.exe" CACHE FILEPATH "Python 3 executable" FORCE)
set(GODOTCPP_API_VERSION "4.5" CACHE STRING "Godot API version" FORCE)

# Підключаємо godot-cpp
add_subdirectory(godot-cpp)

# Файли нашого коду
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} "src/neural_net.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE godot::cpp)

# Прямі шляхи до заголовків (включаючи Eigen):
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}/godot-cpp/gen/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/godot-cpp/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/godot-cpp/gdextension"
    "${CMAKE_CURRENT_SOURCE_DIR}/eigen"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
