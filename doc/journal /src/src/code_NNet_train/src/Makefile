PROGRAM   = main
CXX       = g++
EIGENDIR = ./..
CXXFLAGS  = -g -std=c++20 -Ofast -Wall -I$(EIGENDIR) 

$(PROGRAM): main.o Instance.o Node.o beam_search.o globals.o nnet.o
	$(CXX) $(CXXFLAGS) -o $(PROGRAM) $^

.PHONY: clean

clean:
	-rm -f *.o $(PROGRAM)
