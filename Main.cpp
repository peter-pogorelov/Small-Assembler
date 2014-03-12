#include <iostream>

#include "Compiler.h"

void main()
{
	auto text = SmallAssembler::Helpers::LoadFromFile("local.txt");
	SmallAssembler::Compiler a(text.c_str());
	if (!a.Compile())
	{
		std::cout << a.GetLastError();
	}
	else
	{
		std::cout << "Successful compilation (" << a.BinarySize() << " bytes).";
		SmallAssembler::Helpers::WriteBinaryFoFile("out.compiled", a.GetBytes());
	}

	getchar();
}