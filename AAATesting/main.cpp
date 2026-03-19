#include <gtest/gtest.h>
#include <vector>
#include <limits>
#include <cstdint>
#include <iostream>
#include <stdexcept>

int calculateDiscount(int sum) 
{
    if (sum < 0)
        throw std::invalid_argument("sum must be >= 0");
    
    if (sum < 100)
        return 0;
    
    if (sum <= 1000)
        return 5;
    
    return 10;
}

TEST(DiscountTest, NegativeSum) {
    int sum = -1;
    EXPECT_THROW(calculateDiscount(sum), std::invalid_argument);
}

TEST(DiscountTest, BoundaryZeroSum) {
    int sum = 0;
    EXPECT_NO_THROW({calculateDiscount(sum);});
}

TEST(DiscountTest, PositiveSum) {
    int sum = 1;
    EXPECT_NO_THROW({calculateDiscount(sum);});
}

TEST(DiscountTest, Below100) {
    int sum = 99;
    int discount = calculateDiscount(sum);	
    EXPECT_EQ(discount, 0);
}

TEST(DiscountTest, BoundaryEqual100) {
    int sum = 100;
    int discount = calculateDiscount(sum);	
    EXPECT_EQ(discount, 5);
}

TEST(DiscountTest, Above100) {
    int sum = 101;
    int discount = calculateDiscount(sum);
    EXPECT_EQ(discount, 5);
}

TEST(DiscountTest, Below1000) {
   int sum = 999;
   int discount = calculateDiscount(sum);
   EXPECT_EQ(discount, 5);
}

TEST(DiscountServiceTest, Boundary1000) {
   int sum = 1000;
   int discount = calculateDiscount(sum);
   EXPECT_EQ(discount, 5);
}

TEST(DiscountServiceTest, Above1000) {
   int sum = 1001;
   int discount = calculateDiscount(sum);
   EXPECT_EQ(discount, 10);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


