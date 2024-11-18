SOURCES := $(wildcard *.cpp)
EXECUTABLES := $(SOURCES:.cpp=)

all: $(EXECUTABLES)

%: %.cpp
	g++ -o $@ $<

clean:
	rm -f $(EXECUTABLES)