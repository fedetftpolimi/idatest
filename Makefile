
all:
	g++ -O3 -mfpmath=sse -I/usr/include/suitesparse -o Thermal1DIDADense Thermal1DIDADense.cpp -lsundials_ida -lsundials_nvecserial

clean:
	rm Thermal1DIDADense
