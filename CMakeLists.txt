cmake_minimum_required(VERSION 3.10)
project(MarketDataRecorder)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(external/json)
add_subdirectory(external/websocketpp)
add_subdirectory(external/quill)
add_subdirectory(external/fmt)
add_subdirectory(external/prometheus)

include_directories(include)
include_directories(external/websocketpp)
include_directories(external/websocketpp/asio)
include_directories(external/json/single_include)
include_directories(external/quill)
include_directories(external/fmt)
include_directories(external/prometheus)

set(SOURCES
    src/common/WebSocketListener.cpp
    src/common/Scheduler.cpp
    src/common/RecorderMonitor.cpp
    src/bnb/utils/BNBRequests.cpp
    src/bnb/marketConnection/BNBMarketConnectionConfig.cpp
    src/bnb/marketConnection/BNBFeeder.cpp
    src/bnb/marketConnection/BNBBroker.cpp
    src/BNBRecorder.cpp
    src/recorder_main.cpp
)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Sodium REQUIRED)

add_executable(recorder ${SOURCES})

target_link_libraries(recorder
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    quill::quill
    fmt::fmt
    sodium
    prometheus-cpp::core
    prometheus-cpp::pull
)
