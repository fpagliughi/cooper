// func_wrapper_test.cpp
// 

#include "cooper/func_wrapper.h"
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

