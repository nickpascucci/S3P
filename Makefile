# Basic makefile for building and testing libS3P.

CC_FLAGS += \
--std=c99 \
--pedantic \
-Wall \
-Werror \
-Wextra \
-ftest-coverage \
-fprofile-arcs \

INCLUDES += \
-I. \

all: lib bindings

lib: s3p.c s3p.h
	@echo "Building lib with $(CC)."
	$(CC) $(INCLUDES) $(CC_FLAGS) -c s3p.c -o s3p.o
	ar -rcs libS3P.a s3p.o

test: s3ptest.c lib
	@echo "Building C test program."
	$(CC) $(INCLUDES) $(CC_FLAGS) -o s3ptest s3ptest.c s3p.o
	@echo "Executing C tests."
	./s3ptest

bindings: s3pmodule.c lib
	@echo "Building Python bindings."
	python setup.py build

clean:
	$(RM) *.o *.gcda *.gcno *.info s3ptest libS3P.a

coverage:
	lcov --capture --directory ./ --output-file s3p.info
	genhtml --output-directory ./coverage s3p.info
