SERV = webserver
CLI = client
CXX = c++
CXXFLAGS += -std=c++17
SRC_DIR = ./src
OBJ_DIR = ./obj
SRC_SERV = main.cpp Server.cpp ConfigParser.cpp ConfigTokenizer.cpp
# SRC_CLI = client.cpp
OBJ_SERV = $(SRC_SERV:%.cpp=$(OBJ_DIR)/%.o)
OBJ_CLI = $(SRC_CLI:%.cpp=$(OBJ_DIR)/%.o)


GREEN = \033[32m
RESET = \033[0m

all: $(SERV)


$(SERV): $(OBJ_SERV)
	$(CXX) $(CXXFLAGS) -Iinclude $(OBJ_SERV) -o $(SERV)
	@ echo "${GREEN}$(SERV)${RESET} made successfully"

# $(CLI): $(OBJ_CLI)
# 	$(CXX) $(CXXFLAGS) -Iinclude $(OBJ_CLI) -o $(CLI)
# 	@ echo "${GREEN}$(CLI)${RESET} made successfully"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(OBJ_SERV)
	rm -f $(OBJ_CLI)
	rm -rf $(OBJ_DIR)
	rm -rf $(GTEST_DIR)

fclean: clean
	rm -f $(SERV)
	rm -f $(CLI)
	rm -f $(TEST)

re: fclean all

.PHONY: all clean fclean re test