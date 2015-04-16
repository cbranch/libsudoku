UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  OS := Linux
endif
ifeq ($(UNAME_S),Darwin)
  OS := Darwin
endif

CXXFLAGS += -std=c++11 -fPIC -O3
LDFLAGS += -L. 

# definitions for library
LIB_TARGET := libSudoku.a
LIB_OBJECTS := \
  SudokuLib/SudokuGrid.o \
  SudokuLib/SudokuSolver.o

# definitions for CLI interface for library
CLI_TARGET := sudoku
CLI_CXXFLAGS := -I.
CLI_LIBRARY_DEPS := $(LIB_TARGET)
CLI_OBJECTS := \
  SudokuCLI/SudokuCLI.o

# definitions for test suite
TEST_TARGET := sudoku_tests
TEST_CXXFLAGS := -I. -ISudokuTest/gtest-1.7.0 -ISudokuTest/gtest-1.7.0/include
TEST_LDFLAGS :=
TEST_LIBRARY_DEPS := $(LIB_TARGET)
TEST_OBJECTS := \
  SudokuTest/SudokuTest.o
# gtest uses .cc for their c++ files
GTEST_OBJECTS := \
  SudokuTest/gtest-1.7.0/src/gtest-all.o \
  SudokuTest/gtest-1.7.0/src/gtest_main.o
ifeq ($(OS),Linux)
  # gtest uses pthreads on linux
  TEST_CXXFLAGS += -pthread
  TEST_LDFLAGS += -pthread -lpthread
endif

# for deps & clean
ALL_OBJECTS := $(LIB_OBJECTS) $(CLI_OBJECTS) $(TEST_OBJECTS) $(GTEST_OBJECTS)

# for clean
GENERATED_FILES := \
  $(CLI_TARGET) \
  $(LIB_TARGET) \
  $(TEST_TARGET) \
  $(ALL_OBJECTS) \
  $(ALL_OBJECTS:.o=.d) \

# common targets
all: $(CLI_TARGET) $(TEST_TARGET)

test: $(TEST_TARGET)

clean:
	-rm $(GENERATED_FILES)

watch:
ifeq ($(OS),Darwin)
	fswatch -0 -o -r --include \.cpp$$ --include \.h$$ --exclude $$ . | xargs -0 -n1 -I{} make
endif
ifeq ($(OS),Linux)
	while ! inotifywait -r -e modify .; do make; done
endif

.PHONY: all test watch clean

# provide dependency info if exists
-include $(ALL_OBJECTS:.o=.d)

# target build rules
$(TARGET): $(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJECTS)
	$(CXX) $(LDFLAGS) -shared -o $@ $^

$(CLI_TARGET): $(CLI_OBJECTS) $(CLI_LIBRARY_DEPS) 
	$(CXX) $(LDFLAGS) -o $@ \
	  $(CLI_OBJECTS) $(patsubst lib%.a,-l%,$(CLI_LIBRARY_DEPS))

$(TEST_TARGET): $(TEST_OBJECTS) $(GTEST_OBJECTS) $(TEST_LIBRARY_DEPS) 
	$(CXX) $(LDFLAGS) $(TEST_LDFLAGS) -o $@ \
	  $(TEST_OBJECTS) $(GTEST_OBJECTS) $(patsubst lib%.a,-l%,$(TEST_LIBRARY_DEPS))

$(LIB_OBJECTS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -MD -MF $*.d -o $@ -c $<

$(CLI_OBJECTS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CLI_CXXFLAGS) -MD -MF $*.d -o $@ -c $<

$(TEST_OBJECTS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) $(TEST_CXXFLAGS) -MD -MF $*.d -o $@ -c $<

$(GTEST_OBJECTS): %.o: %.cc
	$(CXX) $(CXXFLAGS) $(TEST_CXXFLAGS) -MD -MF $*.d -o $@ -c $<
