all:
	mkdir -p build
	cd build && cmake .. && make

clean: build
	rm -rf build/
