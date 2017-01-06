#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void
assert_int_equals(char *var1, int int1, char *var2, int int2,
		   const char *file, int line, const char *func)
{
	if (int1 != int2) {
		printf("assertion \"%s == %s\" failed: "
		       "file \"%s\", line %d, function \"%s\"\n", var1, var2,
		       file, line, func);
		printf("value of %s is: %d\n", var1, int1);
		printf("value of %s is: %d\n", var2, int2);
		exit(EXIT_FAILURE);
	}
}

void
assert_time_equals(char *var1, time_t time1, char *var2, time_t time2,
		   const char *file, int line, const char *func)
{
	if (time1 != time2) {
		printf("assertion \"%s == %s\" failed: "
		       "file \"%s\", line %d, function \"%s\"\n", var1, var2,
		       file, line, func);
		printf("value of %s is: %d\n", var1, time1);
		printf("value of %s is: %d\n", var2, time2);
		exit(EXIT_FAILURE);
	}
}

void
assert_time_diff(char *name1, time_t time1, char *name2, time_t time2, char *diff_name, time_t diff,
		 const char *file, int line, const char *func)
{
	time_t diff_found;

	diff_found = time1 - time2;

	if (diff_found != diff) {
		printf("assertion \"(%s - %s) == %s\" "
		       "failed: file \"%s\", line %d, function \"%s\"\n",
		       name1, name2, diff_name, file, line, func);
		printf("value of %s is: %d\n", name1, time1);
		printf("value of %s is: %d\n", name2, time2);
		printf("value of %s is: %d\n", diff_name, diff);
		exit(EXIT_FAILURE);
	}
}
