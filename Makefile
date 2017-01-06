SRC = unlucky_time.c override.c utils.c

OBJ_DIR = obj/shared
OBJ= $(SRC:%.c=$(OBJ_DIR)/%.o)

TEST_OBJ_DIR = obj/test
TEST_OBJ = $(SRC:%.c=$(TEST_OBJ_DIR)/%.o)

CFLAGS = -g -DOVERRIDE_CLOCK_GETTIME -DOVERRIDE_GETTIMEOFDAY -D OVERRIDE_TIME -DDEBUG
LDADD = -ldl -lbsd

$(OBJ_DIR)/%.o: %.c
	gcc -c -shared -fPIC ${CFLAGS} $< -o $@

$(TEST_OBJ_DIR)/%.o: %.c
	gcc -c ${CFLAGS} $< -o $@

unlucky_time.so: ${OBJ}
	gcc -o $@ -shared -fPIC ${CFLAGS}  $^  ${LDADD}

test_override: test.o test_override.o ${TEST_OBJ}
	gcc ${CFLAGS} $^ ${LDADD} -o $@

test_unlucky_time: test.o test_unlucky_time.o ${TEST_OBJ}
	gcc  ${CFLAGS} $^ ${LDADD} -o $@

test: test_unlucky_time test_override
	@./test_unlucky_time
	@./test_override
	@echo "All tests ran successfully"

clean:
	rm -f unlucky_time.so test ${OBJ} ${TEST_OBJ} test.o test_override.o test_unlucky_time.o
