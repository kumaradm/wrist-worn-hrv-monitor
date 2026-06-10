SOURCES = max30101.c
TARGET  = ../model/max30101.wasm

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
		rm -rf ../model

model:
		mkdir -p ../model

$(TARGET): model $(SOURCES) ../wokwi-api.h
	  clang --target=wasm32-unknown-wasi --sysroot /opt/wasi-libc -nostartfiles -Wl,--import-memory -Wl,--export-table -Wl,--no-entry -Werror -Wl,--initial-memory=131072 -Wl,--stack-first -o $(TARGET) $(SOURCES)

.PHONY: test
test:
	  cd test && arduino-cli compile -e -b arduino:avr:uno blink