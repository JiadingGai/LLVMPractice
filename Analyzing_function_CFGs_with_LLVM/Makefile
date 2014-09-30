all:bb_toposort_sccs
%:%.cpp
	g++ -o $@ $^ `llvm-config --cxxflags --ldflags --libs` -ldl -Wall
clean:
	rm -f bb_toposort_sccs
