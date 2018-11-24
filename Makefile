COMMON_FLAGS=-MMD

CFLAGS=-std=c99
CXXFLAGS=-std=c++11 -DPRETTYPRINT_CPP_INTERNAL=1

DEBUG_FLAGS=-g -O0
RELEASE_FLAGS=-O2

ifdef RELEASE
COMMON_FLAGS+=$(RELEASE_FLAGS)
else
COMMON_FLAGS+=$(DEBUG_FLAGS)
endif

CFLAGS+=$(COMMON_FLAGS)
CXXFLAGS+=$(COMMON_FLAGS)

BUILD=build

CLIB=$(addprefix $(BUILD)/,libprettyprint.a prettyprint.h)

.PHONY: all
all: $(CLIB)

.PHONY: example
example: $(addprefix example/,c-api cpp-api)

$(BUILD)/libprettyprint.a: src/prettyprint.o src/prettyprintcpp.o | $(BUILD)
	$(AR) rcs $@ $^

$(BUILD)/prettyprint.h: src/prettyprint.h | $(BUILD)
	cp $< $@

example/c-api: CFLAGS+=-I$(BUILD)
example/c-api: example/c-api.o $(BUILD)/libprettyprint.a
	$(CC) $(LDFLAGS) -o $@ $^

example/c-api.o: $(BUILD)/prettyprint.h

example/cpp-api: CXXFLAGS+=-I$(BUILD)
example/cpp-api: example/cpp-api.o $(BUILD)/libprettyprint.a
	$(CXX) $(LDFLAGS) -o $@ $^

example/cpp-api.o: $(BUILD)/prettyprint.h

$(BUILD):
	mkdir -p $@

clean:
	rm -rf src/*.o src/*.d example/*.o example/*.d example/c-api example/cpp-api $(BUILD)

-include src/prettyprint.d example/c-api.d example/cpp-api.d
