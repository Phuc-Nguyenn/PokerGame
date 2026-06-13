file(GLOB SOURCE_FILES *.cpp)
file(GLOB UTIL_FILES utils/*.cpp)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_library(PokerSimLib STATIC 
	${SOURCE_FILES}
	${UTIL_FILES}
)

# Optionally specify include directories if needed
target_include_directories(PokerSimLib PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
)

target_link_libraries(PokerSimLib PUBLIC
    Boost::boost
    spdlog::spdlog
    fmt::fmt
)