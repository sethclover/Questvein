CC = g++ -g # -g for debugging
CFLAGS = -Wall -Werror -Iinclude -std=c++17 -MMD
LDFLAGS = -lm -lncurses

SRC_DIR = src
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
DEPENDS = $(OBJECTS:.o=.d)
EXEC = $(BUILD_DIR)/dungeon

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(BUILD_DIR)/* $(SRC_DIR)/*~

.PHONY: all clean

-include $(DEPENDS)
