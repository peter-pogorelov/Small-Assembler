#include "Collections.h"

namespace SmallAssembler
{
	OpcodeCollection* OpcodeCollection::self = nullptr;
	RegisterCollection* RegisterCollection::self = nullptr;

	OpcodeCollection* OpcodeCollection::Inst()
	{
		if (self == nullptr)
			self = new OpcodeCollection();

		return self;
	}

	void OpcodeCollection::Release()
	{
		if (self)
			delete self;
	}

	int OpcodeCollection::FromStr(std::string& oc)
	{
		int result = -1;

		for (auto& i : opcodes)
		{
			if (oc == i.second)
			{
				result = i.first >> 16;
				break;
			}
		}
		return result;
	}

	int OpcodeCollection::Args(std::string& oc)
	{
		int result = -1;

		for (auto& i : opcodes)
		{
			if (oc == i.second)
			{
				result = i.first & 0xFFFF;
				break;
			}
		}
		return result;
	}

	OpcodeCollection::OpcodeCollection()
	{

		opcodes.push_back(std::make_pair(IN_MOV, (const char*)"mov"));
		opcodes.push_back(std::make_pair(IN_DEC, (const char*)"dec"));
		opcodes.push_back(std::make_pair(IN_INC, (const char*)"inc"));
		opcodes.push_back(std::make_pair(IN_ADD, (const char*)"add"));
		opcodes.push_back(std::make_pair(IN_SUB, (const char*)"sub"));
		opcodes.push_back(std::make_pair(IN_MUL, (const char*)"mul"));
		opcodes.push_back(std::make_pair(IN_DIV, (const char*)"div"));
		opcodes.push_back(std::make_pair(IN_JMP, (const char*)"jmp"));
		opcodes.push_back(std::make_pair(IN_JE, (const char*)"je"));
		opcodes.push_back(std::make_pair(IN_JGE, (const char*)"jge"));
		opcodes.push_back(std::make_pair(IN_JNE, (const char*)"jne"));
		opcodes.push_back(std::make_pair(IN_JLE, (const char*)"jle"));
		opcodes.push_back(std::make_pair(IN_JNE, (const char*)"jne"));
		opcodes.push_back(std::make_pair(IN_CMP, (const char*)"cmp"));
		opcodes.push_back(std::make_pair(IN_OUT, (const char*)"out"));
		opcodes.push_back(std::make_pair(IN_PUSH, (const char*)"push"));
		opcodes.push_back(std::make_pair(IN_POP, (const char*)"pop"));
		opcodes.push_back(std::make_pair(IN_POPAD, (const char*)"popad"));
		opcodes.push_back(std::make_pair(IN_PUSHAD, (const char*)"pushad"));
		opcodes.push_back(std::make_pair(IN_CALL, (const char*)"call"));
		opcodes.push_back(std::make_pair(IN_RET, (const char*)"ret"));
		opcodes.push_back(std::make_pair(IN_IN, (const char*)"in"));
	}

	RegisterCollection* RegisterCollection::Inst()
	{
		if (self == nullptr)
			self = new RegisterCollection();

		return self;
	}

	void RegisterCollection::Release()
	{
		if (self)
			delete self;
	}

	int RegisterCollection::FromStr(std::string& oc)
	{
		int result = -1;

		for (auto& i : regs)
		{
			if (oc == i.second)
			{
				result = i.first;
				break;
			}
		}
		return result;
	}

	RegisterCollection::RegisterCollection()
	{
		regs.push_back(std::make_pair(IR_REQ, "req"));
		regs.push_back(std::make_pair(IR_RNEQ, "rneq"));
		regs.push_back(std::make_pair(IR_REG, "reg"));
		regs.push_back(std::make_pair(IR_REGEQ, "regeq"));
		regs.push_back(std::make_pair(IR_REL, "rel"));
		regs.push_back(std::make_pair(IR_RELEQ, "releq"));
	}
}