find_package(Catch2 3 REQUIRED)

function(rapidfuzz_add_test NAME SOURCE)
	add_executable(test_${NAME} ${SOURCE})
	target_link_libraries(test_${NAME} ${PROJECT_NAME})
	target_link_libraries(test_${NAME} Catch2::Catch2WithMain )
	add_test(NAME ${NAME} COMMAND test_${NAME})
endfunction()

rapidfuzz_add_test(fuzz tests-fuzz.cpp)
rapidfuzz_add_test(string-metric tests-string-metric.cpp)
rapidfuzz_add_test(common tests-common.cpp)
rapidfuzz_add_test(jaro-winkler tests-jaro-winkler.cpp)
