CXX = g++
FLAGS = -std=c++17 -g -Wall -Wshadow
FLAGS_DEBUG = -DLOCAL -fsanitize=undefined -fsanitize=address -D_GLIBCXX_DEBUG
FLAGS_FAST = -O3

WZO = wzo.cpp
CODE_DIR = .code
BIN = .bin

SUFFIX_SOURCES =           \
		includes.h             \
		tuple_unpacker.h       \
		serializer.h           \
		utils.h                \
		systm.h                \
		input_output.h         \
		signals.h              \
		state.h                \
		output_controller.h    \
		status_bar.h           \
		children.h             \
		term_codes.h           \
		child_process.h        \
		main_process.h         \
		command_line.h         \
		responder.h            \
		                       \
		serializer.cpp         \
		child_process.cpp      \
		children.cpp           \
		command_line.cpp       \
		output_controller.cpp  \
		input_output.cpp       \
		main_process.cpp       \
		signals.cpp            \
		state.cpp              \
		term_codes.cpp         \
		status_bar.cpp         \
		systm.cpp              \
		utils.cpp              \
		responder.cpp          \
		solution.cpp           \
		main.cpp               \

FULL_SUFFIX_SOURCES = $(addprefix $(CODE_DIR)/, $(SUFFIX_SOURCES))

.cpp:
	@make --no-print-directory $(BIN)/all/debug/$@.e
	@make --no-print-directory .$@.cpp
	@cp $(BIN)/all/debug/$@.e $@.e

precompute: $(BIN)/lib_debug.o $(BIN)/lib_fast.o

$(BIN)/all/debug/%.e: $(BIN)/all/debug/%.o $(BIN)/lib_debug.o | $(BIN)/all/debug
	@echo "Linking"
	@$(CXX) $(FLAGS) $(FLAGS_DEBUG) $^ -o $@

$(BIN)/all/fast/%.e: $(BIN)/all/fast/%.o $(BIN)/lib_fast.o | $(BIN)/all/fast
	@echo "Linking"
	@$(CXX) $(FLAGS) $(FLAGS_FAST) $^ -o $@

$(BIN)/all/debug/%.o: $(BIN)/all/%.cpp | $(BIN)/all/debug
	@echo "Compiling with flags: $(FLAGS) $(FLAGS_DEBUG)"
	@$(CXX) $(FLAGS) $(FLAGS_DEBUG) -c $^ -o $@

$(BIN)/all/fast/%.o: $(BIN)/all/%.cpp | $(BIN)/all/fast
	@echo "Compiling with flags: $(FLAGS) $(FLAGS_DEBUG)"
	@$(CXX) $(FLAGS) $(FLAGS_FAST) -c $^ -o $@

.%.cpp: $(BIN)/all/%.cpp
	@cp $< $@

$(BIN)/all/%.cpp: %.cpp $(CODE_DIR)/prefix.cpp $(CODE_DIR)/wzo_suffix.cpp | $(BIN)/all
	@cp $(CODE_DIR)/prefix.cpp $@
	@echo "#line 1 \"$<\"" >> $@
	@cat $< >> $@
	@cat $(CODE_DIR)/wzo_suffix.cpp >> $@

$(BIN)/lib_debug.o: $(BIN)/lib.cpp | $(BIN)
	@echo "\033[35mCompiling debug library: $(FLAGS) $(FLAGS_DEBUG)\033[0m"
	@$(CXX) $(FLAGS) $(FLAGS_DEBUG) -c $^ -o $@

$(BIN)/lib_fast.o: $(BIN)/lib.cpp | $(BIN)
	@echo "\033[35mCompiling fast library: $(FLAGS) $(FLAGS_FAST)\033[0m"
	@$(CXX) $(FLAGS) $(FLAGS_FAST) -c $^ -o $@

$(BIN)/lib.cpp: $(CODE_DIR)/declaration_prefix.cpp $(CODE_DIR)/wzo_as_declarations.cpp $(BIN)/suffix.cpp | $(BIN)
	@cat $^ > $@

$(BIN)/suffix.cpp: $(FULL_SUFFIX_SOURCES) | $(BIN)
	@rm -f $@
	@for file in $^; do \
		echo "#line 1 \"$$file\"" >> $@; \
		cat $$file >> $@; \
	done

$(BIN)/all/debug: | $(BIN)/all
	@mkdir -p $(BIN)/all/debug

$(BIN)/all/fast: | $(BIN)/all
	@mkdir -p $(BIN)/all/fast

$(BIN)/all: | $(BIN)
	@mkdir -p $(BIN)/all

$(BIN):
	@mkdir -p $(BIN)

.PHONY: clean
clean:
	rm -rf .*.cpp *.e $(BIN)

.PRECIOUS: $(BIN)/all/%.cpp $(BIN)/all/debug/%.o $(BIN)/all/fast/%.o
