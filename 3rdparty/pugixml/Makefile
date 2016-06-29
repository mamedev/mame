.SUFFIXES:
MAKEFLAGS+=-r

config=debug
defines=standard
cxxstd=c++11
# set cxxstd=any to disable use of -std=...

BUILD=build/make-$(CXX)-$(config)-$(defines)-$(cxxstd)

SOURCES=src/pugixml.cpp $(filter-out tests/fuzz_%,$(wildcard tests/*.cpp))
EXECUTABLE=$(BUILD)/test

VERSION=$(shell sed -n 's/.*version \(.*\).*/\1/p' src/pugiconfig.hpp)
RELEASE=$(shell git ls-files src docs/*.html docs/*.css docs/samples docs/images scripts contrib CMakeLists.txt readme.txt)

CXXFLAGS=-g -Wall -Wextra -Werror -pedantic -Wundef -Wshadow -Wold-style-cast -Wcast-align
LDFLAGS=

ifeq ($(config),release)
	CXXFLAGS+=-O3 -DNDEBUG
endif

ifeq ($(config),coverage)
	CXXFLAGS+=-coverage
	LDFLAGS+=-coverage
endif

ifeq ($(config),sanitize)
	CXXFLAGS+=-fsanitize=address
	LDFLAGS+=-fsanitize=address

	ifneq ($(shell uname),Darwin)
		CXXFLAGS+=-fsanitize=undefined
		LDFLAGS+=-fsanitize=undefined
	endif
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
else
test: $(EXECUTABLE)
	./$(EXECUTABLE)
endif

fuzz:
	@mkdir -p $(BUILD)
	$(AFL)/afl-clang++ tests/fuzz_parse.cpp tests/allocator.cpp src/pugixml.cpp $(CXXFLAGS) -o $(BUILD)/fuzz_parse
	$(AFL)/afl-fuzz -i tests/data_fuzz_parse -o $(BUILD)/fuzz_parse_out -x $(AFL)/testcases/_extras/xml/ -- $(BUILD)/fuzz_parse @@

clean:
	rm -rf $(BUILD)

release: build/pugixml-$(VERSION).tar.gz build/pugixml-$(VERSION).zip

docs: docs/quickstart.html docs/manual.html

build/pugixml-%: .FORCE | $(RELEASE)
	@mkdir -p $(BUILD)
	perl tests/archive.pl $@ $|

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD)/%.o: %
	@mkdir -p $(dir $@)
	$(CXX) $< $(CXXFLAGS) -c -MMD -MP -o $@

-include $(OBJECTS:.o=.d)

.SECONDEXPANSION:
docs/%.html: docs/%.adoc $$(shell sed -n 's/include\:\:\(.*\)\[.*/docs\/\1/p' docs/%.adoc)
	asciidoctor -b html5 -a version=$(VERSION) $< -o $@

.PHONY: all test clean release .FORCE
