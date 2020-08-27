/*
#include <iostream>
#include "version.hpp"

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout << argv[0] << " Version " << version() << std::endl;
		std::cout << "Usage: " << argv[0] << " number" << std::endl;
		return 1;
	}

	double const inputValue = std::stod(argv[1]);
	std::cout << inputValue * inputValue << std::endl;
	return 0;
}
*/
#include "ffi.h"
#include "k4_types.hpp"

K K4_DECL version(K)
{
	return K_NIL;
}
