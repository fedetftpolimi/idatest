
all:
	g++ -O3 -mfpmath=sse -I/usr/include/suitesparse -o Thermal1DIDADense Thermal1DIDADense.cpp -lsundials_ida -lsundials_nvecserial -L /home/mscuttari/projects/MARCO/sundials/lib

clean:
	rm Thermal1DIDADense
