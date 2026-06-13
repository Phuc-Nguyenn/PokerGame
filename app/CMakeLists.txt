file(GLOB SOURCE_FILES *.cpp)

add_executable(PokerSim ${SOURCE_FILES})

target_link_libraries(PokerSim PRIVATE PokerSimLib)