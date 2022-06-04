FILE := $(shell ./get-path.sh)
SOURCE := $(shell find . -name "*.h") $(shell find . -name "*.c") $(shell find . -name "*.kt")
FULLPATH := ./build/bin/native/debugExecutable/$(FILE).kexe

.PHONY: all
all: $(SOURCE)
	gradle nativeBinaries

run: all
	kitty --hold $(FULLPATH)

clean:
	gradle clean
