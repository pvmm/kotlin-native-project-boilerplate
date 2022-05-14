
.PHONY: all
all:
	gradle clean nativeBinaries

run: all
	./build/bin/native/debugExecutable/native-sample.kexe

clean:
	gradle clean
