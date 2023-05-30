all: compile run

compile:
	cmake -S . -B cmake-build
	cd cmake-build && make
final:
	cmake -S . -B result
	cd result && make
run: compile
	./cmake-build/download-file

clean:
	rm -rf cmake-build
