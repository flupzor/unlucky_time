void assert_time_equals(char *, time_t, char *, time_t, const char *, int,
			const char *);
void assert_time_diff(char *, time_t, char *, time_t, char *, time_t, const
		      char *, int, const char *);

#define ASSERT_TIME_EQUALS(time1, time2) \
	 assert_time_equals(#time1, time1, #time2, time2, __FILE__, __LINE__, __func__);

#define ASSERT_INT_EQUALS(int1, int2) \
	 assert_time_equals(#int1, int1, #int2, int2, __FILE__, __LINE__, __func__);

#define ASSERT_TIME_DIFF(time1, time2, diff) \
	 assert_time_diff(#time1, time1, #time2, time2, #diff, diff, \
		 	  __FILE__, __LINE__, __func__);

