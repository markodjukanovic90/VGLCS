PROGRAM   = main
CXX       = g++
CXXFLAGS  = -g -std=c++23 -Ofast -Wall #-pg 

$(PROGRAM): main.o Instance.o Node.o Algorithm.o global_variable.o#vns.o greedy.o utils.o
	$(CXX) $(CXXFLAGS) -o $(PROGRAM) $^

.PHONY: clean

clean:
	-rm -f *.o $(PROGRAM)
