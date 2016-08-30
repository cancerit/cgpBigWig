#VERSION=1.10.0
VERSION=$(shell cat ../VERSION.txt)

#Compiler
CC = gcc -O3 -DVERSION='"$(VERSION)"' -g

#CC = gcc -O3 -DVERSION='"$(VERSION)"' -g

#compiler flags
# -g adds debug info to the executable file
# -Wall turns on most warnings from compiler
CFLAGS = -Wall


HTSLOC?=$(HTSLIB)

#if test -z "$$LIBBIGWIG"
#LIBBIGWIGLOC?=$(LIBBIGWIG)
#echo "*******DEFINED"
#endif

HTSTMP?=./htslib_tmp
prefix=?/usr/local/

#Define locations of header files
OPTINC?= -I$(HTSLOC)/
INCLUDES= -I./ $(OPTINC) -rdynamic

ifdef LIBBIGWIG
JOIN_INCLUDES= -I$(LIBBIGWIG)
CAT_LFLAGS= -L$(LIBBIGWIG)
endif

JOIN_INCLUDES+= -I$(prefix)/include
CAT_LFLAGS+= -L$(prefix)/lib

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS?= -L$(HTSTMP)
LIB_LOCS?=

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so:
LIBS =-lhts -lpthread -lz -lm -ldl
LIBBWLIBS=-lBigWig -lz -lcurl -lm -lgnutls -ltasn1 -lhogweed -lnettle -lgmp -lp11-kit

# define the C source files
#SRCS = ./bam_access.c ./bam_stats_output.c ./bam_stats_calcs.c
SRCS=
#Define test sources
TEST_SRC=$(wildcard ./c_tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

MD := mkdir

#Build target executable
JOIN_TARGET=../bin/bwjoin
BAM2BG_TARGET=../bin/bam2bedgraph
BAM2BW_TARGET=../bin/bam2bw
CAT_TARGET=../bin/bwcat


#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean test make_htslib_tmp remove_htslib_tmp pre

.NOTPARALLEL: test

all: clean pre make_htslib_tmp $(BAM2BG_TARGET) $(BAM2BW_TARGET) remove_htslib_tmp $(JOIN_TARGET) $(CAT_TARGET)
#all: clean pre make_htslib_tmp $(BAM2BG_TARGET) $(BAM2BW_TARGET) test remove_htslib_tmp $(JOIN_TARGET)
	@echo  bwcat, bwjoin, bam2bedgraph and bam2bw compiled.

$(CAT_TARGET):
	$(CC) $(JOIN_INCLUDES) $(CFLAGS) ./catbw.c $(CAT_LFLAGS) $(LIB_LOCS) $(LIBBWLIBS) -o $(CAT_TARGET)

$(JOIN_TARGET):
	$(CC) $(JOIN_INCLUDES) $(CFLAGS) ./bwjoin.c $(CAT_LFLAGS) $(LIB_LOCS) $(LIBBWLIBS) -o $(JOIN_TARGET)

$(BAM2BW_TARGET):
	$(CC) $(JOIN_INCLUDES) $(INCLUDES) $(CFLAGS) ./bam2bw.c $(CAT_LFLAGS) $(LFLAGS) $(LIB_LOCS) $(LIBBWLIBS) $(LIBS) -o $(BAM2BW_TARGET)

$(BAM2BG_TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BAM2BG_TARGET) $(OBJS) $(LFLAGS) $(LIBS) ./bam2bedgraph.c


#Unit Tests
test:
test: CFLAGS += $(INCLUDES) $(OBJS) $(LFLAGS) $(LIBS)
test: $(TESTS)
	sh ./c_tests/runtests.sh

#Unit tests with coverage
coverage: CFLAGS += --coverage
coverage: test

make_bam2bw: clean pre make_htslib_tmp $(BAM2BW_TARGET) remove_htslib_tmp
	@echo $(BAM2BW_TARGET) done

make_bigwigcat: $(JOIN_TARGET)
	@echo $(JOIN_TARGET) done

make_htslib_tmp:
	$(MD) $(HTSTMP)
	#Do some magic to ensure we compile with the static libhts.a rather than libhts.so
	ln -s $(HTSLOC)/libhts.a $(HTSTMP)/libhts.a

remove_htslib_tmp:
	@echo remove tmp hts location
	-rm -rf $(HTSTMP)

copyscript:
	cp ./scripts/* ./bin/
	chmod a+x $(JOIN_TARGET) $(BAM2BW_TARGET) $(BAM2BG_TARGET)

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)


# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	@echo clean
	$(RM) ./*.o *~ $(CAT_TARGET) $(JOIN_TARGET) $(BAM2BW_TARGET) $(BAM2BG_TARGET) ./tests/tests_log $(TESTS) ./*.gcda ./*.gcov ./*.gcno *.gcda *.gcov *.gcno ./tests/*.gcda ./tests/*.gcov ./tests/*.gcno
	-rm -rf $(HTSTMP)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
