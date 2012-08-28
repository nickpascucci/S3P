# Basic makefile for building and testing libS3P.

all: lib

lib: s3p.c s3p.h
	@echo "Building lib with $(CC)."
	$(CC) -I. -c s3p.c --std=c99

test: s3ptest.c lib
	@echo "Building test program."
	$(CC) -I. -o s3ptest s3ptest.c s3p.o
	@echo "Executing tests."
	./s3ptest

clean:
	$(RM) s3p.o s3ptest
