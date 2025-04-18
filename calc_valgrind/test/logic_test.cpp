#include <gtest/gtest.h>
#include "logic.h"


TEST(LogicTest, BitAnd) {
  int a = 1;
  int b = 2;
  EXPECT_EQ(bitwise_and(a, b), 0);
}

TEST(LogicTest, BitOr) {
  int a = 1;
  int b = 2;
  EXPECT_EQ(bitwise_or(a, b), 3);
}

TEST(LogicTest, BitXor1) {
  int a = 1;
  int b = 2;
  EXPECT_EQ(bitwise_xor(a, b), 3);
}

