# Makefile helper
# Used to prevent misusage of cmake

RELEASE_DIR		=	Release
DEBUG_DIR		=	Debug
BUILD_DIR		=	$(RELEASE_DIR)
BUILD_TYPE		=	Release
COVERAGE_OUTPUT	=	coverage.info

CMAKE_ARGS		=

RM		=	rm -rf

# Default: build release
all: release

# Build rules
build:
	cmake -E make_directory $(BUILD_DIR)
	cmake -B $(BUILD_DIR) -H. -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ${CMAKE_ARGS} -GNinja
	cmake --build $(BUILD_DIR)

release:
	$(MAKE) build BUILD_DIR=$(RELEASE_DIR) BUILD_TYPE=Release

debug:
	$(MAKE) build BUILD_DIR=$(DEBUG_DIR) BUILD_TYPE=Debug

# Unit testing rules
tests:
	$(MAKE) release CMAKE_ARGS+=-DTESTS=ON

tests_debug:
	$(MAKE) debug CMAKE_ARGS+=-DTESTS=ON

run_tests: tests
	ninja -C $(RELEASE_DIR) test

run_tests_debug: tests_debug
	ninja -C $(DEBUG_DIR) test

# Code coverage rules
coverage:
	$(MAKE) tests_debug CMAKE_ARGS+="-DTESTS=ON -DCODE_COVERAGE=ON"

run_coverage: coverage
	ninja -C $(DEBUG_DIR) test
	lcov --directory . --capture --no-external --output-file $(COVERAGE_OUTPUT)
	lcov --remove $(COVERAGE_OUTPUT) "*/Tests/*" "*/Core/*" -o $(COVERAGE_OUTPUT)
	lcov --remove $(COVERAGE_OUTPUT) "*/Core/*" "*/Core/*" -o $(COVERAGE_OUTPUT)
	lcov --list $(COVERAGE_OUTPUT)

# Benchmarks rules
benchmarks:
	$(MAKE) release CMAKE_ARGS+=-DBENCHMARKS=ON

benchmarks_debug:
	$(MAKE) debug CMAKE_ARGS+=-DBENCHMARKS=ON


# Cleaning rules
clean_release:
	$(RM) ${RELEASE_DIR}

clean_debug:
	$(RM) ${DEBUG_DIR}

clean: clean_release clean_debug


# Full cleaning rules
fclean_release: clean_release

fclean_debug: clean_debug

fclean: fclean_release fclean_debug

# Clean everything then rebuild
re: clean all


.PHONY: all \
	release debug \
	tests tests_debug run_tests run_tests_debug \
	benchmarks benchmarks_debug \
	clean clean_release clean_debug \
	fclean fclean_release fclean_debug \
	re