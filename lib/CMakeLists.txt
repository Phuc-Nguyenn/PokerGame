set(JSON_BuildTests OFF CACHE INTERNAL "")

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json 3.12.0 REQUIRED)

enable_testing()

add_library(PokerSimLib STATIC 
	src/Common.cpp
	src/Connection.cpp
	src/Deck.cpp
	src/Player.cpp
	src/PokerGame.cpp
	errors/Error.cpp
)

target_include_directories(PokerSimLib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/errors
)

target_link_libraries(PokerSimLib PUBLIC
Boost::boost
Boost::filesystem 
Boost::system
spdlog::spdlog
fmt::fmt
nlohmann_json::nlohmann_json
)

target_compile_definitions(PokerSimLib PRIVATE
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
)

add_subdirectory(unit_tests)