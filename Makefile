all: compile run

compile:
	cmake -S torrent-client-prototype -B cmake-build
	cd cmake-build && make

install_python_requirements:
	pip3 install -r requirements.txt

run: compile install_python_requirements
	/bin/bash check_script.sh ./cmake-build/torrent-client-prototype

clean:
	rm -rf cmake-build

