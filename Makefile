CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
LEX = flex
YACC = bison

# Output files
PARSER = parser.c
LEXER = lexer.c
HEADER = parser.h

# Object files
OBJECTS = main.o lexer.o parser.o pcb_model.o

# Target executable
TARGET = circuitscript.out

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c parser.h pcb_model.h
	$(CC) $(CFLAGS) -c -o $@ $<

lexer.o: $(LEXER)
	$(CC) $(CFLAGS) -c -o $@ $<

parser.o: $(PARSER)
	$(CC) $(CFLAGS) -c -o $@ $<

pcb_model.o: pcb_model.c pcb_model.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(LEXER): circuitscript.l $(HEADER)
	$(LEX) -o $@ $<

$(PARSER) $(HEADER): circuitscript.y
	$(YACC) -d -o $(PARSER) $<

clean:
	rm -rf $(TARGET) $(OBJECTS) $(PARSER) $(LEXER) $(HEADER) output/

.PHONY: all clean