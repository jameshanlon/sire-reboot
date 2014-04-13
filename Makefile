CXX=clang++
CXX_FLAGS=-g -Wall -pedantic -O2
LD_FLAGS=
TARGET=sire
SOURCES=\
  main.cpp \
  Error.cpp \
  SymTable.cpp \
  Lexer.cpp \
  Tree.cpp \
  Parser.cpp
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

