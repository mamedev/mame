# SAM8905 Standalone Test Harness Makefile

CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
LDFLAGS =

TARGET = test_harness
SOURCES = test_harness.cpp sam8905_core.cpp
OBJECTS = $(SOURCES:.cpp=.o)

PYTHON = python3
DATA_SCRIPT = extract_test_data.py
DATA_DIR = test_data

.PHONY: all clean test data play help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp sam8905_core.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Extract test data from ROM
data: $(DATA_SCRIPT)
	$(PYTHON) $(DATA_SCRIPT)

# Run built-in sine test
test: $(TARGET) data
	./$(TARGET) --test -s 2.0 -o builtin_sine.wav
	@echo ""
	@echo "Generated: builtin_sine.wav"

# Test with extracted algorithm
test-algo: $(TARGET) data
	./$(TARGET) -a $(DATA_DIR)/algo_minimal.bin -d $(DATA_DIR)/dram_sine_440hz.bin -o algo_test.wav --dump
	@echo ""
	@echo "Generated: algo_test.wav"

# Test ROM algorithm
test-rom: $(TARGET) data
	./$(TARGET) -a $(DATA_DIR)/algo_e000.bin -d $(DATA_DIR)/dram_sine_440hz.bin -o rom_algo.wav --dump
	@echo ""
	@echo "Generated: rom_algo.wav"

# Test chord
test-chord: $(TARGET) data
	./$(TARGET) -a $(DATA_DIR)/algo_minimal.bin -d $(DATA_DIR)/dram_chord.bin -o chord.wav --dump -s 2.0
	@echo ""
	@echo "Generated: chord.wav"

# Play the output (requires aplay)
play: builtin_sine.wav
	aplay $<

# Clean build artifacts
clean:
	rm -f $(TARGET) $(OBJECTS) *.wav

# Clean everything including test data
distclean: clean
	rm -rf $(DATA_DIR)

help:
	@echo "SAM8905 Test Harness"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build test harness (default)"
	@echo "  data       - Extract test data from ROM"
	@echo "  test       - Run built-in sine wave test"
	@echo "  test-algo  - Test with minimal algorithm"
	@echo "  test-rom   - Test with ROM algorithm (algo_e000)"
	@echo "  test-chord - Test two-note chord"
	@echo "  play       - Play generated WAV file"
	@echo "  clean      - Remove build artifacts"
	@echo "  distclean  - Remove all generated files"
	@echo "  help       - Show this message"
	@echo ""
	@echo "Manual usage:"
	@echo "  ./test_harness --test                    # Built-in sine test"
	@echo "  ./test_harness -a algo.bin -d dram.bin   # Load files"
	@echo "  ./test_harness -h                        # Show options"
