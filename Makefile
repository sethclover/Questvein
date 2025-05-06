CC = g++ -g
CFLAGS = -Wall -Werror -Iinclude -std=c++17 -MMD
LDFLAGS = -lm -lncurses

SRC_DIR = src
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
DEPENDS = $(OBJECTS:.o=.d)

LEX_SRC = $(SRC_DIR)/lexer.l
YACC_SRC = $(SRC_DIR)/parser.y
LEX_OBJ = $(BUILD_DIR)/lex.yy.o
YACC_OBJ = $(BUILD_DIR)/y.tab.o
EXEC = $(BUILD_DIR)/dungeon

all: $(EXEC)
	@if [ -z "$(MAKE_RESTARTS)" ]; then echo "$(shell shuf -n 1 quips/nothing.txt)"; fi

$(EXEC): $(OBJECTS) $(YACC_OBJ) $(LEX_OBJ)
	@echo "$(shell shuf -n 1 quips/start.txt)"
	@$(CC) $(OBJECTS) $(YACC_OBJ) $(LEX_OBJ) -o $@ $(LDFLAGS)

$(LEX_OBJ): $(BUILD_DIR)/lex.yy.c $(BUILD_DIR)/parser.tab.h
	@echo "$(shell shuf -n 1 quips/start.txt)"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(YACC_OBJ): $(BUILD_DIR)/parser.tab.c
	@echo "$(shell shuf -n 1 quips/start.txt)"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/parser.tab.h: $(YACC_SRC)
	@echo "$(shell shuf -n 1 quips/start.txt)"
	@mkdir -p $(BUILD_DIR)
	@bison -d -o $(BUILD_DIR)/parser.tab.c $<

$(BUILD_DIR)/lex.yy.c: $(LEX_SRC) $(BUILD_DIR)/parser.tab.h
	@echo "$(shell shuf -n 1 quips/start.txt)"
	@mkdir -p $(BUILD_DIR)
	@flex -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "$(shell shuf -n 1 quips/start.txt)"
	@mkdir -p $(BUILD_DIR)
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@echo "$(shell shuf -n 1 quips/clean.txt)"
	@rm -f $(BUILD_DIR)/* $(SRC_DIR)/*~

.PHONY: all clean

-include $(DEPENDS)