#include "ResourceManager.h"

#include <iostream>

struct Example {
	std::string test;
};

Example load(std::string s, char c) {

	Example ex;
	ex.test = s + c;

	return ex;
}

void free(Example* ex) {

	std::cout << "Example " << ex->test << " gets freed" << std::endl;
}


void main() {

	ResourceManager<Example, std::string, std::string, char> mng(&load, &free);

	mng.loadDataAsync("abc", "ab", 'c'); // zero references to abc

	auto abc = mng.getData("abc"); // one reference to abc

	std::cout << abc->test << std::endl;

	auto gfh = mng.getData("gfh", "gf", 'h'); // one reference to gfh since it got loaded from params

	std::cout << gfh->test << std::endl;

	mng.freeData("abc"); // zero references to abc -> free func gets called

	abc = mng.getData("abc"); // returns null since it has no reference and cant load since no params were passed

	if (abc)
		std::cout << abc->test << std::endl;
	else
		std::cout << "abc was nullptr" << std::endl;
}