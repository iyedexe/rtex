cmake_minimum_required(VERSION 3.10)
project(RTEX)

set(CMAKE_CXX_STANDARD 17)
set(JSON_ImplicitConversions OFF)

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

set(COMMON_SOURCES
    src/common/WebSocketListener.cpp
    src/bnb/marketConnection/BNBMarketConnectionConfig.cpp
    src/bnb/marketConnection/BNBFeeder.cpp
    src/bnb/marketConnection/BNBBroker.cpp
    src/common/Scheduler.cpp
    src/bnb/utils/BNBRequests.cpp
)

set(RECORDER_SOURCES
    ${COMMON_SOURCES}
    src/BNBRecorder.cpp
    src/common/RecorderMonitor.cpp
    src/recorder_main.cpp
)
set(TRADER_SOURCES
    ${COMMON_SOURCES}
    src/strategies/CircularArb.cpp
    src/strategies/IStrategy.cpp
    src/bnb/utils/SymbolFilter.cpp
    src/bnb/utils/ExchangeInfo.cpp
    src/trader_main.cpp
)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Sodium REQUIRED)

set(COMMON_LIBS
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    quill::quill
    fmt::fmt
    sodium
    prometheus-cpp::core
    prometheus-cpp::pull
)


add_executable(recorder ${RECORDER_SOURCES})
target_link_libraries(recorder PRIVATE ${COMMON_LIBS})

add_executable(trader ${TRADER_SOURCES})
target_link_libraries(trader PRIVATE ${COMMON_LIBS})
