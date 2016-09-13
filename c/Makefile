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
INCLUDES= $(OPTINC) -rdynamic

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

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so:
LIBS =-lhts -lpthread -lz -lm -ldl
LIBBWLIBS=-lBigWig -lz -lcurl -lm -lgnutls -ltasn1 -lhogweed -lnettle -lgmp -lp11-kit

# define the C source files
#SRCS = ./bam_access.c ./bam_stats_output.c ./bam_stats_calcs.c
SRCS= ./bam_access.c ./utils.c
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
BAM2BASES_TARGET=../bin/bam2bwbases
CAT_TARGET=../bin/bwcat
make_BW=../bin/makebw


#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean test make_htslib_tmp remove_htslib_tmp pre

.NOTPARALLEL: test

all: clean pre make_htslib_tmp $(BAM2BG_TARGET) $(BAM2BW_TARGET) $(BAM2BASES_TARGET) remove_htslib_tmp $(JOIN_TARGET) $(CAT_TARGET) test
	@echo  bwcat, bwjoin, bam2bedgraph, bam2bw and bam2bwbases compiled.

$(CAT_TARGET): $(OBJS)
	$(CC) $(JOIN_INCLUDES) $(INCLUDES) $(CFLAGS) ./catbw.c  $(OBJS)  $(LFLAGS) $(CAT_LFLAGS) $(LIBS) $(LIBBWLIBS) -o $(CAT_TARGET)

$(JOIN_TARGET): $(OBJS)
	$(CC) $(JOIN_INCLUDES) $(INCLUDES) $(CFLAGS) ./bwjoin.c $(OBJS)  $(LFLAGS) $(CAT_LFLAGS) $(LIBS) $(LIBBWLIBS) -o $(JOIN_TARGET)

$(BAM2BW_TARGET): $(OBJS)
	$(CC) $(JOIN_INCLUDES) $(INCLUDES) $(CFLAGS) ./bam2bw.c $(OBJS) $(LFLAGS) $(CAT_LFLAGS) $(LIBBWLIBS) $(LIBS) -o $(BAM2BW_TARGET)

$(BAM2BASES_TARGET): $(OBJS)
	$(CC) $(JOIN_INCLUDES) $(INCLUDES) $(CFLAGS) ./bam2bwbases.c $(OBJS) $(LFLAGS) $(CAT_LFLAGS) $(LIBBWLIBS) $(LIBS) -o $(BAM2BASES_TARGET)

$(BAM2BG_TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(JOIN_INCLUDES) $(INCLUDES) ./bam2bedgraph.c $(OBJS) $(LFLAGS) $(LIBS) -o $(BAM2BG_TARGET)

pre:
	mkdir ../bin


#Unit Tests
test: $(BAM2BG_TARGET) $(BAM2BW_TARGET) $(BAM2BASES_TARGET) $(JOIN_TARGET) $(CAT_TARGET)
test: CFLAGS += $(JOIN_INCLUDES) $(INCLUDES) -I./ $(OBJS) $(LFLAGS) $(LIBS) $(CAT_LFLAGS)
test: $(TESTS)
	sh ./c_tests/runtests.sh

#Unit tests with coverage
coverage: CFLAGS += --coverage
coverage: test

make_bam2bw: clean pre make_htslib_tmp $(BAM2BW_TARGET) remove_htslib_tmp
	@echo $(BAM2BW_TARGET) done

make_bam2bwbases: clean pre make_htslib_tmp $(BAM2BASES_TARGET) remove_htslib_tmp
	@echo $(BAM2BASES_TARGET) done

make_bam2bedgraph: clean pre make_htslib_tmp $(BAM2BG_TARGET) remove_htslib_tmp
	@echo $(BAM2BG_TARGET) done

make_bwcat: $(CAT_TARGET)
	@echo $(CAT_TARGET) done

make_bwjoin: $(JOIN_TARGET)
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
	chmod a+x $(BAM2BG_TARGET) $(BAM2BW_TARGET) $(BAM2BASES_TARGET) $(JOIN_TARGET) $(CAT_TARGET)

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)


# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(JOIN_INCLUDES) $(INCLUDES) -c $<  -o $@

clean:
	@echo clean
	$(RM) ./*.o *~ $(BAM2BG_TARGET) $(BAM2BW_TARGET) $(BAM2BASES_TARGET) $(JOIN_TARGET) $(CAT_TARGET) ./tests/tests_log $(TESTS) ./*.gcda ./*.gcov ./*.gcno *.gcda *.gcov *.gcno ./c_tests/*.gcda ./c_tests/*.gcov ./c_tests/*.gcno
	-rm -rf $(HTSTMP) ../bin

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
