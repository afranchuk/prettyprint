CFLAGS=-std=c99 -MMD
BUILD=build

CLIB=$(addprefix $(BUILD)/,libprettyprint.a prettyprint.h)

.PHONY: all
all: $(CLIB)

.PHONY: example
example: example/c-api

$(BUILD)/libprettyprint.a: src/prettyprint.o | $(BUILD)
	$(AR) rcs $@ $^

$(BUILD)/prettyprint.h: src/prettyprint.h | $(BUILD)
	cp $< $@

example/c-api: CFLAGS+=-I$(BUILD)
example/c-api: example/c-api.o $(BUILD)/libprettyprint.a
	$(CC) $(LDFLAGS) -o $@ $^

example/c-api.o: $(BUILD)/prettyprint.h

$(BUILD):
	mkdir -p $@

clean:
	rm -rf src/*.o src/*.d example/*.o example/*.d example/c-api $(BUILD)

-include src/prettyprint.d example/c-api.d
