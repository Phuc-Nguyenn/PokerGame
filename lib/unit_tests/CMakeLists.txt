find_package(GTest REQUIRED)


add_executable(unit_tests TestDeck.cpp)

target_compile_definitions(unit_tests PRIVATE
	UNIT_TEST
)

target_link_libraries(unit_tests PRIVATE
	PokerSimLib
	GTest::gtest
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)