MPICC=mpicc
CFLAGS=-g -O3 -Wall

TARGETS=libmpisection.so

all : $(TARGETS)


libmpisection.so : ./MPI_section.c
	$(MPICC) $(CFLAGS) -shared -fpic -o $@ $^


clean:
	rm -fr $(TARGETS)
