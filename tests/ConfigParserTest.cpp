#include <gtest/gtest.h>
#include "ConfigParser.hpp"
#include "GlobalConfig.hpp"
#include <vector>
#include <fstream>
#include <cstdio>

class ConfigParserTest : public ::testing::Test {
protected:
};

TEST_F(ConfigParserTest, FileOK) {
	char path[] = "configs/default-config.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_FALSE(parser.processConfig());
}

TEST_F(ConfigParserTest, EmptyFile) {
    char path[] = "configs/empty.conf";

    GlobalConfig config;
    ConfigParser parser(path, config);

    EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, NoServerBlock) {
	char path[] = "configs/no-server-block.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, UnknownDirective) {
	char path[] = "configs/unknown-directive.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, DuplicateListen) {
	char path[] = "configs/listen/duplicate-listen.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, NoListenDirective) {
	char path[] = "configs/listen/no-listen.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ListenMissingArgument) {
	char path[] = "configs/listen/listen-missing-argument.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ListenTooManyArguments) {
	char path[] = "configs/listen/listen-too-many-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ListenInvalidPort) {
	char path[] = "configs/listen/listen-invalid-port.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ListenMissingSemicolon) {
	char path[] = "configs/listen/listen-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ServerBlockMalformed) {
	char path[] = "configs/server/server-malformed.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ServerBlockEmpty) {
	char path[] = "configs/server/server-empty.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ClientMaxBodySizeArgumentNotInteger) {
		char path[] = "configs/client_max_body_size/body_size-argument-not-int.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ServerNameInvalidNumberOfArguments) {
	char path[] = "configs/server_name/server_name-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ServerNameDuplicateValues) {
	char path[] = "configs/server_name/server_name-duplicate-values.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ServerNameMissingSemicolon) {
	char path[] = "configs/server_name/server_name-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, RootInvalidNumberOfArguments) {
	char path[] = "configs/root/root-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, RootMissingSemicolon) {
	char path[] = "configs/root/root-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, IndexInvalidNumberOfArguments) {
	char path[] = "configs/index/index-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, IndexMissingSemicolon) {
	char path[] = "configs/index/index-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ClientMaxBodySizeInvalidNumberOfArguments) {
	char path[] = "configs/client_max_body_size/client_max_body_size-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ClientMaxBodySizeInvalidNumber) {
	char path[] = "configs/client_max_body_size/client_max_body_size-invalid-number.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ClientMaxBodySizeMissingSemicolon) {
	char path[] = "configs/client_max_body_size/client_max_body_size-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, AutoIndexInvalidNumberOfArguments) {
	char path[] = "configs/autoindex/autoindex-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, AutoIndexInvalidValue) {
	char path[] = "configs/autoindex/autoindex-invalid-value.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, AutoIndexMissingSemicolon) {
	char path[] = "configs/autoindex/autoindex-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ErrorPageInvalidNumberOfArguments) {
	char path[] = "configs/error_page/error_page-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ErrorPageInvalidErrorCode) {
	char path[] = "configs/error_page/error_page-invalid-error-code.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ErrorPageMissingSemicolon) {
	char path[] = "configs/error_page/error_page-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ReturnInvalidNumberOfArguments) {
	char path[] = "configs/return/return-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ReturnInvalidErrorCode) {
	char path[] = "configs/return/return-invalid-error-code.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, ReturnMissingSemicolon) {
	char path[] = "configs/return/return-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, AllowedMethodsInvalidNumberOfArguments) {
	char path[] = "configs/allowed_methods/allowed_methods-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, AllowedMethodsInvalidMethod) {
	char path[] = "configs/allowed_methods/allowed_methods-invalid-method.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, AllowedMethodsDuplicateMethods) {
	char path[] = "configs/allowed_methods/allowed_methods-duplicate-methods.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, AllowedMethodsMissingSemicolon) {
	char path[] = "configs/allowed_methods/allowed_methods-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, UploadStoreInvalidNumberOfArguments) {
	char path[] = "configs/upload_store/upload_store-invalid-number-of-arguments.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}

TEST_F(ConfigParserTest, UploadStoreMissingSemicolon) {
	char path[] = "configs/upload_store/upload_store-missing-semicolon.conf";

	GlobalConfig config;
	ConfigParser parser(path, config);

	EXPECT_TRUE(parser.processConfig());
}