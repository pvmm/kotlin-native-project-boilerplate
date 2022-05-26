
.PHONY: all
all:
	gradle clean nativeBinaries

run: build/bin/native/debugExecutable/native-sample.kexe
	./build/bin/native/debugExecutable/native-sample.kexe

clean:
	gradle clean
