build:
	g++ -Wall -o ./serial ./rsa_serial.cpp
	g++ -Wall -fopenmp -o ./openmp ./rsa_openmp.cpp
	g++ -Wall -o ./pthread ./rsa_pthread.cpp -lpthread
	mpic++ -Wall -o ./mpi ./rsa_mpi.cpp -lm
	nvcc -O2 -g -std=c++11 ./rsa_cuda.cu -o ./cuda

run_serial:
	./serial

run_openmp:
	./openmp

run_mpi:
	mpirun -np 8 ./mpi

run_pthread:
	./pthread 8

run_cuda:
	./cuda

clean:
	rm -f ./serial
	rm -f ./openmp
	rm -f ./mpi
	rm -f ./pthread
	