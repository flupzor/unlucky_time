../unlucky_time.so: ../unlucky_time.c
	cc -DOVERRIDE_CLOCK_GETTIME -DOVERRIDE_GETTIMEOFDAY -shared -fPIC ../unlucky_time.c -ldl -lbsd -o ../unlucky_time.so
../test: ../test.c ../unlucky_time.c
	cc -DOVERRIDE_CLOCK_GETTIME -DOVERRIDE_GETTIMEOFDAY ../unlucky_time.c ../test.c -ldl -lbsd -o ../test
clean:
	rm unlucky_time.so
