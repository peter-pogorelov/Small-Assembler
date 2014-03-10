#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <sstream>

#define EMPTY_COMMAND = -1

#define IN_3A			0x03 //mm mr mv
#define IN_2A			0x02 //m r v
#define IN_1A			0x01 //no arguments

#define TP_RR			(TP_R << 8) | TP_R
#define TP_RM			(TP_R << 8) | TP_M
#define TP_RV			(TP_R << 8) | TP_V

#define TP_MR			(TP_M << 8) | TP_R
#define TP_MM			(TP_M << 8) | TP_M
#define TP_MV			(TP_M << 8) | TP_V

#define TP_M			0x00
#define TP_R			0x01
#define TP_V			0x02

#define IN_RESERVED		0x00
#define IN_MOV			(IN_RESERVED+1		<< 16 | IN_3A)
#define IN_DEC			((IN_MOV>>16)+1		<< 16 |	IN_2A)
#define IN_INC			((IN_DEC>>16)+1		<< 16 |	IN_2A)
#define IN_ADD			((IN_INC>>16)+1		<< 16 |	IN_3A)
#define IN_SUB			((IN_ADD>>16)+1		<< 16 |	IN_3A)
#define IN_MUL			((IN_SUB>>16)+1		<< 16 |	IN_3A)
#define IN_DIV			((IN_MUL>>16)+1		<< 16 |	IN_3A)
#define IN_JMP			((IN_INC>>16)+1		<< 16 |	IN_2A)
#define IN_JE			((IN_JMP>>16)+1		<< 16 |	IN_2A)
#define IN_JGE			((IN_JE>>16)+1		<< 16 |	IN_2A)
#define IN_JLE			((IN_JGE>>16)+1		<< 16 |	IN_2A)
#define IN_JNE			((IN_JLE>>16)+1		<< 16 |	IN_2A)
#define IN_CMP			((IN_JNE>>16)+1		<< 16 |	IN_3A)
#define IN_OUT			((IN_CMP>>16)+1		<< 16 |	IN_3A)
#define IN_PUSH			((IN_OUT>>16)+1		<< 16 |	IN_2A)
#define IN_POP			((IN_PUSH>>16)+1	<< 16 |	IN_2A)
#define IN_POPAD		((IN_POP>>16)+1		<< 16 |	IN_1A)
#define IN_PUSHAD		((IN_POPAD>>16)+1	<< 16 | IN_1A)
#define IN_CALL			((IN_PUSHAD>>16)+1	<< 16 | IN_2A)
#define IN_RET			((IN_CALL>>16)+1	<< 16 |	IN_1A)
#define IN_IN			((IN_RET>>16)+1		<< 16 |	IN_2A)

//registers
#define IR_RESERVED		0x00
//comparing registers
#define IR_REQ			0x01
#define IR_RNEQ			0x02
#define IR_REG			0x03
#define IR_REGEQ		0x04
#define IR_REL			0x05
#define IR_RELEQ		0x06

#define ALLOC_CMD		"alloc"

#define RESERVE_DWORD(bytes)\
	bytes.push_back(0);\
	bytes.push_back(0);\
	bytes.push_back(0);\
	bytes.push_back(0);\

#define PUSH_DWORD(bytes, str)\
	{\
		int n = atoi(str);\
		char b[4];\
		memcpy(b, &n, sizeof n);\
		bytes.push_back(b[0]);\
		bytes.push_back(b[1]);\
		bytes.push_back(b[2]);\
		bytes.push_back(b[3]);\
	}

namespace Helpers
{
	bool is_str_name(const std::string& str)
	{
		if (str.empty())
			return false;

		for (int i = 0; i < str.length(); ++i)
		{
			if (i == 0 && !isalpha(str[0]) && str[0] != '_')
				return false;

			if (!isalpha(str[i]) && !isdigit(str[i]) && str[i] != '_')
				return false;
		}

		return true;
	}

	bool is_str_number(const std::string& str)
	{
		if (str.empty())
			return false;
		int dots = 0;
		for (auto& i : str){
			if (i == '.')
				++dots;

			if (!isdigit(i) && i != '.' && dots > 1)
				return false;
		}

		return true;
	}

}

namespace SmallAssembler
{
	enum class TokenType
	{
		TOK_Alloc,
		TOK_Name,
		TOK_Instruction,
		TOK_Invalid
	};


	class OpcodeCollection
	{
	public:
		static OpcodeCollection* Inst()
		{
			if (self == nullptr)
				self = new OpcodeCollection();

			return self;
		}

		static void Release()
		{
			if (self)
				delete self;
		}

		int FromStr(std::string& oc)
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

		int Args(std::string& oc)
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

		const char* ToStr(int oc)
		{
			const char* result;

			for (auto&i : opcodes)
			{
				if (i.first == oc)
				{
					result = i.second;
					break;
				}
			}

			return result;
		}

	private:
		OpcodeCollection()
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

		static OpcodeCollection* self;
		std::vector<std::pair<int, const char*>> opcodes;
	};

	OpcodeCollection* OpcodeCollection::self = nullptr;


	class RegisterCollection
	{
	public:
		static RegisterCollection* Inst()
		{
			if (self == nullptr)
				self = new RegisterCollection();

			return self;
		}

		static void Release()
		{
			if (self)
				delete self;
		}

		int FromStr(std::string& oc)
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

		const char* ToStr(int oc)
		{
			const char* result;

			for (auto& i : regs)
			{
				if (i.first == oc)
				{
					result = i.second;
					break;
				}
			}

			return result;
		}

	private:
		RegisterCollection()
		{
			regs.push_back(std::make_pair(IR_REQ, "req"));
			regs.push_back(std::make_pair(IR_RNEQ, "rneq"));
			regs.push_back(std::make_pair(IR_REG, "reg"));
			regs.push_back(std::make_pair(IR_REGEQ, "regeq"));
			regs.push_back(std::make_pair(IR_REL, "rel"));
			regs.push_back(std::make_pair(IR_RELEQ, "releq"));
		}

		std::vector<std::pair<int, const char*>> regs;
		static RegisterCollection* self;
	};

	RegisterCollection* RegisterCollection::self = nullptr;

	class Compiler
	{
	public:
		Compiler(const char* file_name) :
			file_name(file_name), line(0)
		{
		}

		std::string GetLastError()
		{
			return error_stream.str();
		}

		bool Compile()
		{
			if (file_name.empty())
			{
				error_stream << "Empty input file.";
				return false;
			}

			std::ifstream file(file_name, std::ifstream::in);

			while (!file.eof())
			{
				static char buff[128];
				file.getline(buff, sizeof buff);
				line = buff;
				if (!TryParse())
					return false;
			}
			file.close();
			InsertMarkers();

			std::ofstream comp(file_name + ".comp", std::ofstream::binary);

			for (auto&i : bytes)
			{
				comp << i;
			}
			comp.close();
			return true;
		}
	private:

		bool InsertMarkers()
		{
			for (auto&i : replace_places)
			{
				if (markers.find(i.first) != markers.end())
				{
					char b[4];
					int tmp = markers[i.first];
					memcpy(b, &tmp, sizeof b);

					for (int k = 0; k < 4; ++k)
						bytes[i.second + k] = b[k];
				}
				else
				{
					error_stream << "Invalid using of marker or variable (" << i.first << ").";
					return false;
				}
			}

			return true;
		}

		bool TryParse()
		{
			lineCount++;
		RepeatCommand:
			auto& tok = GetNextToken();

			switch (GetTokenType(tok))
			{
			case TokenType::TOK_Alloc:
			{
				tok = GetNextToken();
				if (!Helpers::is_str_name(tok))
				{
					error_stream << "Line: " << lineCount << " invalid name used in alloc command.";
					return false;
				}

				markers[tok] = bytes.size();
				do
				{
					tok = GetNextToken();
					if (!Helpers::is_str_number(tok))
					{
						error_stream << "Line: " << lineCount << " unknown type in alloc command. Integer exprexted.";
						return false;
					}

					PUSH_DWORD(bytes, tok.c_str());

					tok = GetNextToken();
				} while (tok == " ");

				return true;
			}
			case TokenType::TOK_Instruction:
			{
				switch (OpcodeCollection::Inst()->Args(tok))
				{
				case IN_1A:
					return BuildInst1A(tok);
				case IN_2A:
					return BuildInst2A(tok);
				case IN_3A:
					return BuildInst3A(tok);
				}
			}
				return true;
			case TokenType::TOK_Name:
			{
				std::string name = tok;
				tok = GetNextToken();
				if (tok != ":")
				{
					error_stream << "Line: " << lineCount << " invalid using of marker. ':' is missed.";
					return false;
				}
				markers[name] = bytes.size();
				goto RepeatCommand;
			}
			case TokenType::TOK_Invalid:
				return true;
			}
		}

		bool BuildInst1A(std::string& inst)
		{
			if (!GetNextToken().empty())
			{
				error_stream << "Line: " << lineCount << " instruction " << inst << " takes no arguments.";
				return false;
			}

			bytes.push_back(OpcodeCollection::Inst()->FromStr(inst));
			return true;
		}

		bool BuildInst2A(std::string& inst) // TODO
		{
			std::string& oper = GetNextToken();

			if (oper.empty() || !GetNextToken().empty())
			{
				error_stream << "Line: " << lineCount << " instruction " << inst << " takes 1 argument.";
				return false;
			}

			bytes.push_back(OpcodeCollection::Inst()->FromStr(inst));
			int reg = RegisterCollection::Inst()->FromStr(oper);

			if (reg != -1)
			{
				bytes.push_back(TP_R);
				bytes.push_back(reg);
			}
			else if (Helpers::is_str_name(oper))
			{
				bytes.push_back(TP_M);
				replace_places.push_back(std::make_pair(oper, bytes.size()));
				RESERVE_DWORD(bytes);
			}
			else if (Helpers::is_str_number(oper))
			{
				bytes.push_back(TP_V);
				PUSH_DWORD(bytes, oper.c_str());
			}
			else
			{
				error_stream << "Line: " << lineCount << " unknown type of 1-st operand of " << inst << ".";
				return false;
			}

			return true;
		}

		bool BuildInst3A(std::string& inst)
		{
			std::string& op1 = GetNextToken();
			std::string& op2 = GetNextToken();

			if (op1.empty() || op2.empty() || !GetNextToken().empty())
			{
				error_stream << "Line: " << lineCount << " instruction " << inst << " takes 2 arguments.";
				return false;
			}

			short type = 0;
			int reg;

			bytes.push_back(OpcodeCollection::Inst()->FromStr(inst));
			bytes.push_back(0); //reserve place for type of operands
			bytes.push_back(0);
			int place = bytes.size() - 2;

			if ((reg = RegisterCollection::Inst()->FromStr(op1)) != -1)
			{
				bytes.push_back(reg);
				type |= TP_R << 8;
			}
			else if (Helpers::is_str_name(op1))
			{
				replace_places.push_back(std::make_pair(op1, bytes.size()));
				RESERVE_DWORD(bytes); //for future editing
				type |= TP_M << 8;
			}
			else if (Helpers::is_str_number(op1)) //you can pass a variable to a given address
			{
				bytes.push_back(atoi(op1.c_str()));
				type |= TP_M << 8;
			}
			else
			{
				error_stream << "Line: " << lineCount << " unknown type of 1-st operand of " << inst << ".";
				return false;
			}

			if ((reg = RegisterCollection::Inst()->FromStr(op2)) != -1)
			{
				bytes.push_back(reg);
				type |= TP_R;
			}
			else if (Helpers::is_str_name(op2))
			{
				replace_places.push_back(std::make_pair(op2, bytes.size()));
				RESERVE_DWORD(bytes); //for future editing
				type |= TP_M;
			}
			else if (Helpers::is_str_number(op2))
			{
				PUSH_DWORD(bytes, op2.c_str());
				type |= TP_V;
			}
			else
			{
				error_stream << "Line: " << lineCount << " unknown type of 2-nd operand of " << inst << ".";
				return false;
			}

			bytes[place] = type >> 8;
			bytes[place + 1] = type & 0xFF;

			return true;
		}

		std::string GetNextToken()
		{
			std::string token = std::string();
			char* l = line;
			while (isspace(*l) || *l == '\n') ++l;
			if (isalpha(*l) || isdigit(*l) || *l == '_'){
				while (isalpha(*l) || isdigit(*l) || *l == '_') token += tolower(*l++);
			}
			else {
				while (*l != '\t' && *l != ' ' && *l != '\n' && *l != '\0') token += tolower(*l++);
			}
			line = l;
			return token;
		}

		TokenType GetTokenType(std::string& token)
		{
			if (OpcodeCollection::Inst()->FromStr(token) != -1)
			{
				return TokenType::TOK_Instruction;
			}

			if (token == ALLOC_CMD)
			{
				return TokenType::TOK_Alloc;
			}

			if (Helpers::is_str_name(token))
			{
				return TokenType::TOK_Name;
			}

			return TokenType::TOK_Invalid;
		}
	private:
		std::stringstream error_stream;
		int lineCount;

		std::map<std::string, int> markers;
		std::vector<std::pair<std::string, int>> replace_places;
		std::vector<unsigned char> bytes;
		std::string file_name;
		char* line;
		char* text;
	};
}

void main()
{
	SmallAssembler::Compiler a("local.txt");
	if (!a.Compile())
	{
		std::cout << a.GetLastError();
	}

	getchar();
}