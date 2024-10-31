
all:
	clang++ -O3 -I/usr/include/suitesparse -mfpmath=sse -o Thermal1DIDADense Thermal1DIDADense.cpp -lsundials_ida -lsundials_nvecserial -lsundials_sunlinsolklu -lklu

clean:
	rm Thermal1DIDADense
