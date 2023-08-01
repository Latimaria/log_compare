CXX = g++
CXXFLAGS = -std=c++11 # -Wall

TARGET = compare

SOURCES = main.cpp Event.cpp Log.cpp Trace.cpp PrefixTree.cpp
OBJDIR = build

OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS := $(addprefix $(OBJDIR)/,$(OBJECTS))

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Clean rule
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
