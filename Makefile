CXX=clang++
CXX_FLAGS=-g -Wall -pedantic -O2
LD_FLAGS=
TARGET=sire
SOURCES=\
  main.cpp \
  Error.cpp \
  Table.cpp \
  Tree.cpp \
  Lex.cpp \
  Syn.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(TARGET)

%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(LD_FLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

count:
	wc -l *.cpp *.h

