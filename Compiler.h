#include <fstream>
#include <vector>
#include <map>

#include <sstream>

#include "Helpers.h"
#include "Defines.h"
#include "Collections.h"

namespace SmallAssembler
{
	enum class TokenType
	{
		TOK_Alloc,
		TOK_Name,
		TOK_Instruction,
		TOK_Invalid
	};

	class Compiler
	{
	public:
		Compiler(const char* code) :
			file_name(file_name), line(code)
		{
		}

		std::string GetLastError()
		{
			return error_stream.str();
		}

		bool Compile()
		{
			while (*line != 0)
			{
				if (!TryParse())
					return false;

				if(*line == '\n')//to the new line;
					++line;
			}

			InsertMarkers();
			
			return true;
		}

		std::vector<unsigned char>& GetBytes()
		{
			return bytes;
		}

		size_t BinarySize()
		{
			return bytes.size();
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
			const char* l = line;
			while (isspace(*l) && *l != '\n') ++l;
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
		const char* line;
	};
}