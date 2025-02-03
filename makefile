
all:
	mpicc -o output ./main.c -lm
exec:
	mpiexec -n 4 ./output c
clean:
	rm -rf ./output
