
# Makefile
# myfirstc: myfirstprogram.c
# 	gcc myfirstprogram.c -o myfirstc

# Default target: make uses the first non-comment target as the default. In your file that's tempraturec, not run. If you intended run to be default, put it first or add an all target.

# NIce

run: main
	./build/main

main: ./src/main.c ./src/helper.c
	mkdir -p ./build/
	gcc ./src/main.c ./src/helper.c -o ./build/main

clean:
	rm -f ./build/main
