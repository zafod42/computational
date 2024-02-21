
all: lab1
.PHONY: all

main.o: main.c
	gcc -o main.o -c main.c

quanc8.o: quanc8.c
	gcc -o quanc8.o -c quanc8.c

spline.o: spline.c
	gcc -o spline.o -c spline.c

lab1: main.o quanc8.o spline.o
	gcc -o lab1 main.o quanc8.o spline.o -lm
.PHONY: lab1

clean:
	rm lab1 *.o
.PHONY: clean
