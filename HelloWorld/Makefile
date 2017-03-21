CPP := g++
CPPFLAGS = -Wall -O3
RTTIFLAG := -fno-rtti
COVRFLAG := #-fprofile-arcs -ftest-coverage -lgcov
LLVMCONFIG := llvm-config
LLVMCOMPONENTS := #--libs Analysis --system-libs
LLVMCPPFLAGS := $(shell $(LLVMCONFIG) --cxxflags)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags $(LLVMCOMPONENTS))
LLVMVER := LLVM_34

MYUTIL = 

MYPASS = hw.cpp

UTIL_OBJS=$(MYUTIL:.cpp=.o)
PASS_OBJS=$(MYPASS:.cpp=.o)
SOS=$(PASS_OBJS:.o=.so)

all:$(SOS)
%.so:$(UTIL_OBJS) %.o
	$(CPP) -shared -o $@ $^ $(LLVMLDFLAGS) -Wall $(COVRFLAG)
%.o:%.cpp 
	$(CPP) -fPIC -c $^ $(LLVMCPPFLAGS) $(COVRFLAG) -Wall -D$(LLVMVER)
run:
	opt -S -load ./hw.so -hellogai < fib.ll > /dev/null
clean:
	rm -f *.so *.o *.bc *.gch *.dot .*.sw* *.gcov *.gcda *.gcno
