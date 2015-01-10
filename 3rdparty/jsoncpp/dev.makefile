all: build test-amalgamate

build:
	mkdir -p build/debug
	cd build/debug; cmake -DCMAKE_BUILD_TYPE=debug -DJSONCPP_LIB_BUILD_SHARED=ON -G "Unix Makefiles" ../..
	make -C build/debug

# Currently, this depends on include/json/version.h generated
# by cmake.
test-amalgamate: build
	python2.7 amalgamate.py
	python3.4 amalgamate.py

.PHONY: build
