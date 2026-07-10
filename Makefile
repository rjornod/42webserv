SERV = webserver
CXX = c++
CXXFLAGS += -std=c++17
SRC_DIR = ./src
OBJ_DIR = ./obj
SRC_CORE = Server.cpp HttpParser.cpp HttpRequest.cpp ConfigTokenizer.cpp
OBJ_CORE = $(SRC_CORE:%.cpp=$(OBJ_DIR)/%.o)

GTEST_REPO = https://github.com/google/googletest.git
GTEST_DIR = ./googletest
GTEST_OBJ_DIR = $(OBJ_DIR)/gtest_obj
CPPFLAGS += -isystem $(GTEST_DIR)/include

TEST = gtests
SRC_TEST_DIR = ./tests
SRC_TEST = HttpParserTest.cpp 
OBJ_TEST = $(SRC_TEST:%.cpp=$(GTEST_OBJ_DIR)/%.o)

P_TEST = parserTest


GREEN = \033[32m
RESET = \033[0m

all: $(SERV)

$(SERV): $(OBJ_DIR)/main.o $(OBJ_CORE)
	$(CXX) $(CXXFLAGS) -Iinclude -Iinterfaces $(OBJ_DIR)/main.o $(OBJ_CORE) -o $(SERV)
	@ echo "${GREEN}$(SERV)${RESET} made successfully"

$(OBJ_DIR)/main.o: src/main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

ptest: $(P_TEST)

$(P_TEST): $(OBJ_DIR)/testParser.o $(OBJ_CORE)
	$(CXX) $(CXXFLAGS) -Iinclude -Iinterfaces $(OBJ_DIR)/testParser.o $(OBJ_CORE) -o $(P_TEST)
	@ echo "${GREEN}$(P_TEST)${RESET} made successfully"

$(OBJ_DIR)/testParser.o: src/testParser.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

tests: $(GTEST_DIR) $(TEST)
# 	./$(TEST) #Uncomment if you want to run the tests immediately from the make

$(GTEST_DIR):
	@ echo "Clonning GTest repo ..."
	@git clone --depth 1 $(GTEST_REPO) $(GTEST_DIR)

$(TEST): $(OBJ_TEST) $(OBJ_CORE) $(GTEST_OBJ_DIR)/gtest-all.o $(GTEST_OBJ_DIR)/gtest-main.o
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	$(OBJ_TEST) $(OBJ_CORE) \
	$(GTEST_OBJ_DIR)/gtest-all.o \
	$(GTEST_OBJ_DIR)/gtest-main.o \
	-pthread \
	-o $(TEST) 
	@ echo "${GREEN}$(TEST)${RESET} made successfully"

$(GTEST_OBJ_DIR)/gtest-main.o: $(GTEST_DIR)
	@mkdir -p $(GTEST_OBJ_DIR)
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	-I$(GTEST_DIR)/googletest/include \
	-I$(GTEST_DIR)/googletest \
	-c $(GTEST_DIR)/googletest/src/gtest_main.cc \
	-o $@

$(GTEST_OBJ_DIR)/gtest-all.o: $(GTEST_DIR)
	@mkdir -p $(GTEST_OBJ_DIR)
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	-I$(GTEST_DIR)/googletest/include \
	-I$(GTEST_DIR)/googletest \
	-c $(GTEST_DIR)/googletest/src/gtest-all.cc \
	-o $@

$(GTEST_OBJ_DIR)/%.o: $(SRC_TEST_DIR)/%.cpp
	@mkdir -p $(GTEST_OBJ_DIR)
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	-Iinclude \
	-I$(GTEST_DIR)/googletest/include \
	-I$(GTEST_DIR)/googletest \
	-c $< -o $@

clean:
	rm -f $(OBJ_CORE)
	rm -rf $(OBJ_DIR)
	rm -rf $(GTEST_DIR)

fclean: clean
	rm -f $(SERV)
	rm -f $(TEST)
	rm -f $(P_TEST)

re: fclean all

.PHONY: all clean fclean re test