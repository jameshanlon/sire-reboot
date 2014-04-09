CXX=clang++
CXX_FLAGS=-g -Wall -pedantic -O2
LD_FLAGS=
TARGET=sire
SOURCES=\
  main.cpp \
  Lexer.cpp \
  Error.cpp
#  Parser.cpp \
# NameTable.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(TARGET)

%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(LD_FLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

