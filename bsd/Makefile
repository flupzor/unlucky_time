../unlucky_time.so: ../unlucky_time.c
	cc -DOVERRIDE_CLOCK_GETTIME -DOVERRIDE_GETTIMEOFDAY -shared ../unlucky_time.c -o ../unlucky_time.so

../test: ../test.c ../unlucky_time.c
	cc -DOVERRIDE_CLOCK_GETTIME -DOVERRIDE_GETTIMEOFDAY ../unlucky_time.c ../test.c -o ../test
clean:
	rm ../unlucky_time.so
