Compile=gcc
Target=driver.o

all:	driver.o
	$(Compile) driver.c functions.c -o $(Target) -lm
clean:
	rm $(target)
	rm *.o
