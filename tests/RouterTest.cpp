#include <gtest/gtest.h>
#include "Router.hpp"
#include "LocationConfig.hpp"

class RouterTest : public ::testing::Test {

  protected:

    Router router;

    LocationConfig root;
    LocationConfig images;
    LocationConfig icons;

    void SetUp() override {
      root.setPath("/");
      images.setPath("/images");
      icons.setPath("/images/icons");
    }

};

TEST_F(RouterTest, PrefixMatchWorks){
  EXPECT_TRUE(router.isPrefixMatch("/images", "/images/logo.png"));
  EXPECT_TRUE(router.isPrefixMatch("/images", "/images"));
  EXPECT_TRUE(router.isPrefixMatch("/images/icons", "/images/icons/logo.png"));

  EXPECT_FALSE(router.isPrefixMatch("/images", "/images-old/logo.png"));
  EXPECT_FALSE(router.isPrefixMatch("/images", "/image"));
}

TEST_F(RouterTest, FindSimpleRoute){
  std::vector<LocationConfig> locations{root, images};
  const LocationConfig* result = router.matchLocation(locations, "/images/logo.png");

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->getPath(), "/images");
}

TEST_F(RouterTest, LongestMatchingPrefixWins) {
  std::vector<LocationConfig> locations{root, images, icons};
  const LocationConfig* result = router.matchLocation(locations, "/images/icons/logo.png");

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->getPath(), "/images/icons");
}

TEST_F(RouterTest, RootIsFallback) {
  std::vector<LocationConfig> locations{root, images, icons};
  const LocationConfig* result = router.matchLocation(locations, "/index.html");

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->getPath(), "/");
}

TEST_F(RouterTest, DoesNotMatchPartialPath) {
  std::vector<LocationConfig> locations{root, images};
  const LocationConfig* result = router.matchLocation(locations, "/images-old/logo.png");

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->getPath(), "/");
}