CXX ?= c++
BUILD_DIR ?= /tmp/devlab-ddp-host-tests
ARDUINO_SRC := libraries/arduino/DevLabDDP/src

.PHONY: check protocol-check host-tests

check: protocol-check host-tests

protocol-check:
	cmp -s protocol/include/devlab_protocol.h $(ARDUINO_SRC)/DevLabDDPProtocol.h
	@echo "DDP firmware contract and Arduino library are synchronized"

host-tests: $(BUILD_DIR)/read_buttons_test
	$(BUILD_DIR)/read_buttons_test

$(BUILD_DIR)/read_buttons_test: tests/read_buttons_test.cpp tests/stubs/Arduino.h tests/stubs/Wire.h $(ARDUINO_SRC)/DevLabDDP.h $(ARDUINO_SRC)/DevLabDDPProtocol.h
	mkdir -p $(BUILD_DIR)
	$(CXX) -std=c++11 -O2 -Wall -Wextra -Werror -Itests/stubs -I$(ARDUINO_SRC) $< -o $@
