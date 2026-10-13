#include <gtest/gtest.h>
#include "FileResolver.hpp"
#include "RequestContext.hpp"

class FileResolverTest : public ::testing::Test {

  protected:

  FileResolver resolver;
    

};

TEST_F(FileResolverTest, DecodesSpaces) {
  std::string_view spacesUri = "https://example.com/my%20documents/annual%20report.pdf";
  std::string_view specialCharUri = "https://example.com/search?q=cats%20%26%20dogs";
  std::string_view emailUri = "https://example.com/subscribe?email=jane.doe%40example.com";
  std::string_view percentUri = "https://example.com/sale?discount=50%25";


  Result<std::string, UriDecodeError> spaces = resolver.percentDecode(spacesUri);
  Result<std::string, UriDecodeError> specialChar = resolver.percentDecode(specialCharUri);
  Result<std::string, UriDecodeError> email = resolver.percentDecode(emailUri);
  Result<std::string, UriDecodeError> percent = resolver.percentDecode(percentUri);


  EXPECT_TRUE(spaces.isOk());

  EXPECT_EQ(spaces.value(), "https://example.com/my documents/annual report.pdf");
  EXPECT_EQ(specialChar.value(), "https://example.com/search?q=cats & dogs");
  EXPECT_EQ(email.value(), "https://example.com/subscribe?email=jane.doe@example.com");
  EXPECT_EQ(percent.value(), "https://example.com/sale?discount=50%");

}

TEST_F(FileResolverTest, NormalizesSlashesBetweenBaseAndSuffix) {
  EXPECT_EQ(resolver.joinPath("/var/www", "index.html"),   "/var/www/index.html");
  EXPECT_EQ(resolver.joinPath("/var/www/", "index.html"),  "/var/www/index.html");
  EXPECT_EQ(resolver.joinPath("/var/www", "/index.html"),  "/var/www/index.html");
  EXPECT_EQ(resolver.joinPath("/var/www/", "/index.html"), "/var/www/index.html");
}

TEST_F(FileResolverTest, HandlesEmptyInputs) {
  EXPECT_EQ(resolver.joinPath("", "index.html"), "/index.html");
  EXPECT_EQ(resolver.joinPath("/var/www", ""),   "/var/www/");
  EXPECT_EQ(resolver.joinPath("", ""),           "/");
}

TEST_F(FileResolverTest, NormalizeKeepsPlainSegments) {
  
  auto result = resolver.normalizeSegments("/a/b/c");
  EXPECT_TRUE(result.isOk());
  EXPECT_EQ(result.value(), (std::vector<std::string_view>{"a", "b", "c"}));
}

TEST_F(FileResolverTest, NormalizeSkipsEmptyAndDotSegments) {
  auto result = resolver.normalizeSegments("/a//b/./c/");
  ASSERT_TRUE(result.isOk());
  EXPECT_EQ(result.value(), (std::vector<std::string_view>{"a", "b", "c"}));
}

TEST_F(FileResolverTest, NormalizeResolvesDotDotWithinRoot) {
  auto result = resolver.normalizeSegments("/a/b/../c");
  ASSERT_TRUE(result.isOk());
  EXPECT_EQ(result.value(), (std::vector<std::string_view>{"a", "c"}));
}

TEST_F(FileResolverTest, NormalizeRejectsEscapingRoot) {
  EXPECT_FALSE(resolver.normalizeSegments("/..").isOk());
  EXPECT_TRUE(resolver.normalizeSegments("/..").isErr());
  EXPECT_FALSE(resolver.normalizeSegments("/a/../..").isOk());
  EXPECT_TRUE(resolver.normalizeSegments("/a/../..").isErr());
  EXPECT_FALSE(resolver.normalizeSegments("/../../etc/passwd").isOk());
  EXPECT_TRUE(resolver.normalizeSegments("/../../etc/passwd").isErr());
}

TEST_F(FileResolverTest, NormalizeAllowsReturningToRoot) {
  auto result = resolver.normalizeSegments("/a/..");
  ASSERT_TRUE(result.isOk());
  EXPECT_TRUE(result.value().empty());
}
