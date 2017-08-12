#CXX = g++
CXX = clang++-4.0
FLAGS = -std=c++11 -g -DLOCAL
#FLAGS = -std=c++11 -O2

WZO = wzo.cpp
BIN = wzo.e
FULL_CODE = code.cpp

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
		main.cpp               \

FULL_SUFFIX_SOURCES = $(addprefix code/, $(SUFFIX_SOURCES))

$(BIN): $(FULL_CODE)
	$(CXX) $^ -o $@ $(FLAGS)

$(FULL_CODE): .prefix.cpp $(WZO) .suffix.cpp
	cp .prefix.cpp $@
	echo "#line 1 \"$(WZO)\"" >> $@
	cat $(WZO) .suffix.cpp >> $@

.prefix.cpp: code/prefix.cpp
	cp $< $@

.suffix.cpp: $(FULL_SUFFIX_SOURCES)
	rm -f $@
	for file in $^; do \
		echo "#line 1 \"$$file\"" >> $@; \
		cat $$file >> $@; \
	done

.PHONY: clean
clean:
	rm -f $(BIN) $(FULL_CODE)
