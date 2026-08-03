.SUFFIXES:
MAKEFLAGS+=-r

config=debug
defines=standard
cxxstd=any

BUILD=build/make-$(firstword $(CXX))-$(config)-$(defines)-$(cxxstd)

SOURCES=src/pugixml.cpp $(filter-out tests/fuzz_%,$(wildcard tests/*.cpp))
EXECUTABLE=$(BUILD)/test

VERSION=$(shell sed -n 's/.*version \(.*\).*/\1/p; /version/q' src/pugiconfig.hpp)
RELEASE=$(filter-out scripts/archive.py docs/%.adoc,$(shell git ls-files docs scripts src CMakeLists.txt LICENSE.md readme.txt))

CXXFLAGS=-g -Wall -Wextra -Werror -pedantic -Wundef -Wshadow -Wcast-align -Wcast-qual -Wold-style-cast -Wdouble-promotion
LDFLAGS=

ifeq ($(config),release)
	CXXFLAGS+=-O3 -DNDEBUG
endif

ifeq ($(config),coverage)
	CXXFLAGS+=-coverage
	LDFLAGS+=-coverage
endif

ifeq ($(config),sanitize)
	CXXFLAGS+=-fsanitize=address,undefined -fno-sanitize-recover=all
	LDFLAGS+=-fsanitize=address,undefined
endif

ifeq ($(config),analyze)
	CXXFLAGS+=--analyze
endif

ifneq ($(defines),standard)
	COMMA=,
	CXXFLAGS+=-D $(subst $(COMMA), -D ,$(defines))
endif

ifneq ($(findstring PUGIXML_NO_EXCEPTIONS,$(defines)),)
	CXXFLAGS+=-fno-exceptions
endif

ifneq ($(cxxstd),any)
	CXXFLAGS+=-std=$(cxxstd)
endif

OBJECTS=$(SOURCES:%=$(BUILD)/%.o)

all: $(EXECUTABLE)

ifeq ($(config),coverage)
test: $(EXECUTABLE)
	-@find $(BUILD) -name '*.gcda' -exec rm {} +
	./$(EXECUTABLE)
	@gcov -b -o $(BUILD)/src/ pugixml.cpp.gcda | sed -e '/./{H;$!d;}' -e 'x;/pugixml.cpp/!d;'
	@find . -name '*.gcov' -and -not -name 'pugixml.cpp.gcov' -exec rm {} +
	@sed -i -e "s/#####\(.*\)\(\/\/ unreachable.*\)/    1\1\2/" pugixml.cpp.gcov
else
test: $(EXECUTABLE)
	./$(EXECUTABLE)
endif

fuzz_%: $(BUILD)/fuzz_%
	@mkdir -p build/$@
	$< build/$@ tests/data_fuzz_$* -max_len=1024 -dict=tests/fuzz_$*.dict -fork=16

clean:
	rm -rf $(BUILD)

release: build/pugixml-$(VERSION).tar.gz build/pugixml-$(VERSION).zip

docs: docs/quickstart.html docs/manual.html

build/pugixml-%: .FORCE | $(RELEASE)
	@mkdir -p $(BUILD)
	TIMESTAMP=`git show v$(VERSION) -s --format=%ct` && python3 scripts/archive.py $@ pugixml-$(VERSION) $$TIMESTAMP $|

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD)/fuzz_%: tests/fuzz_%.cpp src/pugixml.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -fsanitize=address,fuzzer $^ -o $@

$(BUILD)/%.o: %
	@mkdir -p $(dir $@)
	$(CXX) $< $(CXXFLAGS) -c -MMD -MP -o $@

-include $(OBJECTS:.o=.d)

.SECONDEXPANSION:
docs/%.html: docs/%.adoc $$(shell sed -n 's/include\:\:\(.*\)\[.*/docs\/\1/p' docs/%.adoc)
	asciidoctor -b html5 -a version=$(VERSION) $< -o $@

.PHONY: all test clean release .FORCE
