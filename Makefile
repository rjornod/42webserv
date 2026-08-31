SERV = webserv
CXX = c++ -Wall -Wextra -Werror
CXXFLAGS += -g -std=c++17
INCL = -Iinclude -Iinclude/config -Iinclude/server -Iinclude/utils -Iinclude/http -Iinterfaces
SRC_DIR = ./src
OBJ_DIR = ./obj
SRC_CORE = Server.cpp  ConfigParser.cpp TokenType.cpp LocationConfig.cpp ServerConfig.cpp \
	HttpParser.cpp HttpRequest.cpp HttpResponse.cpp Router.cpp RequestProcessor.cpp \
	FileResolver.cpp 
OBJ_CORE = $(SRC_CORE:%.cpp=$(OBJ_DIR)/%.o)

GTEST_REPO = https://github.com/google/googletest.git
GTEST_DIR = ./googletest
GTEST_OBJ_DIR = $(OBJ_DIR)/gtest_obj
CPPFLAGS += -isystem $(GTEST_DIR)/include

ALL_TEST = gtests
HTTP_P_TEST = http_gtests
CONFIG_P_TEST = config_gtests
ROUTER_TEST = router_gtests
SRC_TEST_DIR = ./tests
SRC_CONFIG_P_TEST = ConfigParserTest.cpp
SRC_HTTP_P_TEST = HttpParserTest.cpp
SRC_ROUTER_TEST = RouterTest.cpp
OBJ_CONFIG_P_TEST = $(SRC_CONFIG_P_TEST:%.cpp=$(GTEST_OBJ_DIR)/%.o)
OBJ_HTTP_P_TEST = $(SRC_HTTP_P_TEST:%.cpp=$(GTEST_OBJ_DIR)/%.o)
OBJ_ROUTER_TEST = $(SRC_ROUTER_TEST:%.cpp=$(GTEST_OBJ_DIR)/%.o)

P_TEST = httpParserTest


GREEN = \033[32m
RESET = \033[0m

all: $(SERV)

$(SERV): $(OBJ_DIR)/main.o $(OBJ_CORE)
	$(CXX) $(CXXFLAGS) $(INCL) $(OBJ_DIR)/main.o $(OBJ_CORE) -o $(SERV)
	@ echo "${GREEN}$(SERV)${RESET} made successfully"

$(OBJ_DIR)/main.o: src/main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCL) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCL) -c $< -o $@

mini_parse_tester: $(P_TEST)

$(P_TEST): $(OBJ_DIR)/testHttpParser.o $(OBJ_CORE)
	$(CXX) $(CXXFLAGS) $(INCL) $(OBJ_DIR)/testHttpParser.o $(OBJ_CORE) -o $(P_TEST)
	@ echo "${GREEN}$(P_TEST)${RESET} made successfully"

$(OBJ_DIR)/testHttpParser.o: src/testHttpParser.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCL) -c $< -o $@

tests: $(GTEST_DIR) $(ALL_TEST)

tests_config_parser: $(GTEST_DIR) $(CONFIG_P_TEST)

tests_http_parser: $(GTEST_DIR) $(HTTP_P_TEST)

tests_router: $(GTEST_DIR) $(ROUTER_TEST)

$(GTEST_DIR):
	@ echo "Clonning GTest repo ..."
	@git clone --depth 1 $(GTEST_REPO) $(GTEST_DIR)

$(ALL_TEST): $(CONFIG_P_TEST) $(HTTP_P_TEST) $(ROUTER_TEST)
	@ echo "Tests made successfully"

$(CONFIG_P_TEST): $(OBJ_CONFIG_P_TEST) $(OBJ_CORE) $(GTEST_OBJ_DIR)/gtest-all.o $(GTEST_OBJ_DIR)/gtest-main.o
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	$(OBJ_CONFIG_P_TEST) $(OBJ_CORE) \
	$(GTEST_OBJ_DIR)/gtest-all.o \
	$(GTEST_OBJ_DIR)/gtest-main.o \
	-pthread \
	-o $(CONFIG_P_TEST) 
	@ echo "${GREEN}$(CONFIG_P_TEST)${RESET} made successfully"

$(HTTP_P_TEST): $(OBJ_HTTP_P_TEST) $(OBJ_CORE) $(GTEST_OBJ_DIR)/gtest-all.o $(GTEST_OBJ_DIR)/gtest-main.o
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	$(OBJ_HTTP_P_TEST) $(OBJ_CORE) \
	$(GTEST_OBJ_DIR)/gtest-all.o \
	$(GTEST_OBJ_DIR)/gtest-main.o \
	-pthread \
	-o $(HTTP_P_TEST) 
	@ echo "${GREEN}$(HTTP_P_TEST)${RESET} made successfully"

$(ROUTER_TEST): $(OBJ_ROUTER_TEST) $(OBJ_CORE) $(GTEST_OBJ_DIR)/gtest-all.o $(GTEST_OBJ_DIR)/gtest-main.o
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	$(OBJ_ROUTER_TEST) $(OBJ_CORE) \
	$(GTEST_OBJ_DIR)/gtest-all.o \
	$(GTEST_OBJ_DIR)/gtest-main.o \
	-pthread \
	-o $(ROUTER_TEST) 
	@ echo "${GREEN}$(ROUTER_TEST)${RESET} made successfully"

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
	$(INCL) \
	-I$(GTEST_DIR)/googletest/include \
	-I$(GTEST_DIR)/googletest \
	-c $< -o $@

clean:
	rm -f $(OBJ_CORE)
	rm -rf $(OBJ_DIR)
	rm -rf $(GTEST_DIR)

fclean: clean
	rm -f $(SERV)
	rm -f $(P_TEST)
	rm -f $(CONFIG_P_TEST)
	rm -f $(HTTP_P_TEST)
	rm -f $(ROUTER_TEST)

re: fclean all

.PHONY: all clean fclean re test