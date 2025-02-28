#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;
int run();

TEST(RunMain, RunResult) {
  stringstream buffer;
  auto prevcoutbuf = cout.rdbuf(buffer.rdbuf());
  
  run();
  
  auto output = buffer.str();
  cout.rdbuf(prevcoutbuf);
  
  EXPECT_NE(std::string::npos, output.find("a and b:0")) << "Incorrect AND result";
  EXPECT_NE(std::string::npos, output.find("a or b:3")) << "Incorrect OR result";  
  EXPECT_NE(std::string::npos, output.find("a xor b:3")) << "Incorrect XOR result";  
  EXPECT_NE(std::string::npos, output.find("not a:-2")) << "Incorrect NOT result";  
  EXPECT_NE(std::string::npos, output.find("a + b:30")) << "Incorrect + result";  
  EXPECT_NE(std::string::npos, output.find("a - b:-10")) << "Incorrect - result";  
  EXPECT_NE(std::string::npos, output.find("a * b:200")) << "Incorrect * result";  
  EXPECT_NE(std::string::npos, output.find("a / b:0.5")) << "Incorrect / result";
}
