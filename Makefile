CXX = g++
#FLAGS = -std=c++11 -g -DLOCAL
FLAGS = -std=c++11 -O2

WZO = wzo.cpp
BIN = wzo.e
FULL_CODE = code.cpp

$(BIN): $(FULL_CODE)
	$(CXX) $^ -o $@ $(FLAGS)

$(FULL_CODE): .prefix.cpp $(WZO) .suffix.cpp
	cp .prefix.cpp $@
	echo "#line 1 \"$(WZO)\"" >> $@
	cat $(WZO) .suffix.cpp >> $@

.PHONY: clean
clean:
	rm -f $(BIN) $(FULL_CODE)
