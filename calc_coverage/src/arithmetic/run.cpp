#include <iostream>
#include "arithmetic.h"
#include "../logic/logic.h"

int run() {
	int a = 1;
	int b = 2;
	std::cout << "a and b:" << bitwise_and(a, b) << std::endl;
	std::cout << "a or b:" << bitwise_or(a, b) << std::endl;
	std::cout << "a xor b:" << bitwise_xor(a, b) << std::endl;
	std::cout << "not a:" << bitwise_not(a) << std::endl;
	
	a = 10;
	b = 20;
	std::cout << "a + b:" << sum(a, b) << std::endl;
	std::cout << "a - b:" << sub(a, b) << std::endl;
	std::cout << "a * b:" << mult(a, b) << std::endl;
	std::cout << "a / b:" << divide(a, b) << std::endl;
	return 0;
 }

