#include <gtest/gtest.h>
#include "arithmetic.h"

TEST(ArithmeticTest, Sum) {
  int a = 10;
  int b = 20;
  EXPECT_EQ(sum(a, b), 30);
}

TEST(ArithmeticTest, Sub) {
  int a = 10;
  int b = 20;
  EXPECT_EQ(sub(a, b), -10);
}

TEST(ArithmeticTest, Mult) {
  int a = 10;
  int b = 20;
  EXPECT_EQ(mult(a, b), 250);
}
