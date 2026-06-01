BUILD_DIR ?= build
PYTHON ?= python3
CMAKE ?= cmake
CTEST ?= ctest
CPPCHECK ?= cppcheck
CLANG_TIDY ?= clang-tidy

.PHONY: all configure build test lint validate-yaml check clean generate generate-docs generate-cpp generate-roundtrip-tests

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

test: build
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

lint: configure
	$(CPPCHECK) --enable=warning,style,performance,portability \
		--check-level=exhaustive \
		--error-exitcode=1 \
		-I include/ include/ tests/
	$(CLANG_TIDY) -p $(BUILD_DIR) \
		tests/*.cpp -- -Iinclude -std=c++20

validate-yaml:
	$(PYTHON) scripts/validate_yaml_source.py

check: validate-yaml test lint

generate-docs:
	$(PYTHON) scripts/generate_docs_from_yaml.py

generate-cpp:
	$(PYTHON) scripts/generate_vita_cpp.py

generate-roundtrip-tests:
	$(PYTHON) scripts/generate_roundtrip_tests.py

generate: generate-docs generate-cpp generate-roundtrip-tests

clean:
	rm -rf $(BUILD_DIR) tests/build
