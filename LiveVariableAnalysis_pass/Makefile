CPP := g++
CPPFLAGS = -Wall -g
RTTIFLAG := -fno-rtti
LLVMCONFIG := llvm-config
LLVMCOMPONENTS := #--libs bitwriter
LLVMCPPFLAGS := $(shell $(LLVMCONFIG) --cxxflags)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags $(LLVMCOMPONENTS))
LLVMVER := LLVM_33

MYUTIL = 

MYPASS = LVA.cpp

UTIL_OBJS=$(MYUTIL:.cpp=.o)
PASS_OBJS=$(MYPASS:.cpp=.o)
SOS=$(PASS_OBJS:.o=.so)

all:$(SOS)
%.so:$(UTIL_OBJS) %.o
	$(CPP) -shared -o $@ $^ $(LLVMLDFLAGS) -Wall
%.o:%.cpp %.h
	$(CPP) -fPIC -g -c $^ $(LLVMCPPFLAGS) -Wall -D$(LLVMVER)
clean:
	rm -f *.so *.o *.bc *.gch

