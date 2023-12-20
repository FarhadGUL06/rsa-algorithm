CC=g++
MPICC=mpic++
CUDA=nvcc
CFLAGS=-g -Wall -ggdb3 -fno-omit-frame-pointer

build:
	$(CC) $(CFLAGS) -o ./serial ./rsa_serial.cpp
	$(CC) $(CFLAGS) -o ./serial_opt ./rsa_serial_opt.cpp

	$(CC) $(CFLAGS) -fopenmp -o ./openmp ./rsa_openmp.cpp

	$(CC) $(CFLAGS) -o ./pthread ./rsa_pthread.cpp -lpthread

	$(MPICC) $(CFLAGS) -o ./mpi ./rsa_mpi.cpp -lm

	$(MPICC) $(CFLAGS) -o ./mpi_openmp ./rsa_mpi_openmp.cpp -lm -fopenmp
	
	$(CUDA) -O2 -g -std=c++11 ./rsa_cuda_opt.cu -o ./cuda_opt

run_serial:
	./serial $(ARGS)

run_serial_opt:
	./serial_opt $(ARGS)

run_openmp:
	./openmp $(ARGS)

run_mpi:
	mpirun --mca btl_tcp_if_include eth0 -np 8 ./mpi $(ARGS)

run_mpi_openmp:
	mpirun -np 8 ./mpi_openmp $(ARGS)

run_pthread:
	./pthread 8 $(ARGS)
	
run_cuda_opt:
	./cuda_opt $(ARGS)

clean:
	rm -f ./serial*
	rm -f ./openmp
	rm -f ./mpi*
	rm -f ./pthread
	rm -f ./cuda*
	rm -f ./tests/output/*.txt
	