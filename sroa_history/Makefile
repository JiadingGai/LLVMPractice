CPP := g++
CPPFLAGS = -Wall -g
RTTIFLAG := -fno-rtti
COVRFLAG := -fprofile-arcs -ftest-coverage -lgcov
LLVMCONFIG := llvm-config
LLVMCOMPONENTS := #--libs bitwriter
LLVMCPPFLAGS := $(shell $(LLVMCONFIG) --cxxflags)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags $(LLVMCOMPONENTS))
LLVMVER := LLVM_34

MYUTIL = barrier_inst.cpp \

MYPASS = inline_devicefn.cpp \
         mysroa.cpp

UTIL_OBJS=$(MYUTIL:.cpp=.o)
PASS_OBJS=$(MYPASS:.cpp=.o)
SOS=$(PASS_OBJS:.o=.so)

all:$(SOS)
%.so:$(UTIL_OBJS) %.o
	$(CPP) -shared -o $@ $^ $(LLVMLDFLAGS) -Wall $(COVRFLAG)
%.o:%.cpp 
	$(CPP) -fPIC -g -c $^ $(LLVMCPPFLAGS) $(COVRFLAG) -Wall -D$(LLVMVER)
clean:
	rm -f *.so *.o *.bc *.gch *.dot .*.sw* *.gcov *.gcda *.gcno
