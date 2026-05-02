# -------- Compiler & flags --------
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic

# -------- Target --------
TARGET := beamsearch

# -------- Sources --------
SRCS := \
	main.cpp \
	utils.cpp \
	globals.cpp \
	Instance.cpp

# (If you later add BeamSearch.cpp, Utils.cpp, etc., just list them here)

OBJS := $(SRCS:.cpp=.o)

# -------- Default rule --------
all: $(TARGET)

# -------- Link --------
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# -------- Compile --------
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -------- Clean --------
clean:
	rm -f $(OBJS) $(TARGET)

# -------- Phony --------
.PHONY: all clean

