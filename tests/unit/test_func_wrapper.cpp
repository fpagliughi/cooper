// func_wrapper_test.cpp
// 

#include "cooper/func_wrapper.h"
#include "catch2_version.h"

using namespace cooper;

#if 0
#include <iostream>
#include <functional>

void hello(const std::string& name)
{
	std::cout << "Hello, " << name << std::endl;
}

int main(int argc, char* argv[])
{
	std::string name(argc > 1 ? argv[1] : "Bubba");
	cooper::func_wrapper fw1(std::bind(hello, name));
	cooper::func_wrapper fw(std::move(fw1));
	fw();
	return 0;
}
#endif


TEST_CASE("func_wrapper constructors", "[func_wrapper]") {
	REQUIRE(true);
}

