#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "XOne.h"

#define LOG(a) std::cout << a << std::endl

int main(void)
{

	aveng::XOne app{};

	try {
		app.run();
	}
	catch (const std::exception& e)
	{
		LOG(e.what());
		return -1;
	}

	return EXIT_SUCCESS;

}