#include <gtest/gtest.h>
#include <vector>
#include <execution>
#include <algorithm>
#include <atomic>
  
   
TEST(AlgoTest, Equal) {

 std::atomic<long long> sum(0);
   int vec_size{1'000'000};
   
   std::vector<int> acc(vec_size);

   for(int i = 0; i < vec_size; ++i)
   acc[i] = i;
   
  std::for_each(std::execution::par, std::begin(acc), std::end(acc), [&sum](int x){
            	sum += x;
	});
   EXPECT_EQ(sum, 499999500000);
}
