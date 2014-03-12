#pragma once

#include <string>
#include <vector>

#include "Defines.h"

namespace SmallAssembler
{
	class OpcodeCollection
	{
	public:
		static OpcodeCollection* Inst();
		static void Release();
		int FromStr(std::string& oc);
		int Args(std::string& oc);

	private:
		OpcodeCollection();

		static OpcodeCollection* self;
		std::vector<std::pair<int, const char*>> opcodes;
	};


	class RegisterCollection
	{
	public:
		static RegisterCollection* Inst();
		static void Release();
		int FromStr(std::string& oc);

	private:
		RegisterCollection();

		std::vector<std::pair<int, const char*>> regs;
		static RegisterCollection* self;
	};
}