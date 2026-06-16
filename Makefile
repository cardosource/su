# ═══════════════════════════════════════════════════════════════════════════
#  Makefile para Linguagem Marx
#  Executável: tests/marx
#  Objetos: build/*.o
# ═══════════════════════════════════════════════════════════════════════════

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Iinclude -g -O2 -Wno-reorder
LDFLAGS = -ldl

TARGET = tests/marx
BUILD_DIR = build

# Arquivos fonte
SOURCES = src/astPrinter.cpp \
          src/debug.cpp \
          src/environment.cpp \
          src/expr.cpp \
          src/gc.cpp \
          src/interpreter.cpp \
          src/main.cpp \
          src/parser.cpp \
          src/runtimeError.cpp \
          src/scanner.cpp \
          src/stmt.cpp \
          src/su.cpp \
          src/token.cpp

# Objetos (na pasta build)
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

# ═══════════════════════════════════════════════════════════════════════════
#  Regras principais
# ═══════════════════════════════════════════════════════════════════════════

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p tests

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo ""
	@echo "Build sucess: ./$(TARGET)"
	@echo ""

$(BUILD_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -DSU_PLATFORM_LINUX -c $< -o $@

# ═══════════════════════════════════════════════════════════════════════════
#  Limpeza
# ═══════════════════════════════════════════════════════════════════════════

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

clean-objs:
	rm -rf $(BUILD_DIR)

# ═══════════════════════════════════════════════════════════════════════════
#  Testes
# ═══════════════════════════════════════════════════════════════════════════

test: $(TARGET)
	./$(TARGET) tests/a.su

run: $(TARGET)
	./$(TARGET) tests/exemplo.su

shell: $(TARGET)
	./$(TARGET)

