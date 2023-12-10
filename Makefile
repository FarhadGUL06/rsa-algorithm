build:
	g++ -Wall -o ./serial ./rsa_serial.cpp
	g++ -Wall -fopenmp -o ./openmp ./rsa_openmp.cpp
	g++ -Wall -o ./pthread ./rsa_pthread.cpp -lpthread
	mpic++ -Wall -o ./mpi ./rsa_mpi.cpp -lm
	mpic++ -Wall -o ./mpi_openmp ./rsa_mpi_openmp.cpp -lm -fopenmp
	nvcc -O2 -g -std=c++11 ./rsa_cuda.cu -o ./cuda

run_serial:
	./serial $(ARGS)

run_openmp:
	./openmp $(ARGS)

run_mpi:
	mpirun -np 8 ./mpi $(ARGS)

run_mpi_openmp:
	mpirun -np 8 ./mpi_openmp $(ARGS)

run_pthread:
	./pthread 8 $(ARGS)

run_cuda:
	./cuda $(ARGS)

clean:
	rm -f ./serial
	rm -f ./openmp
	rm -f ./mpi
	rm -f ./pthread
	