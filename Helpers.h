#pragma once

#include <string>

namespace SmallAssembler
{
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

		std::string LoadFromFile(const char* path)
		{
			std::ifstream file(path, std::ifstream::in);
			std::string result;

			if (file.is_open())
			{
				while (!file.eof())
				{
					static char temp[128];
					file.getline(temp, sizeof temp);
					result += temp;
					result += "\n";
				}
			}

			return result;
		}

		void WriteBinaryFoFile(const char* path, std::vector<unsigned char>& bytes)
		{
			std::ofstream file(path, std::ofstream::out | std::ofstream::binary);

			for (auto&i : bytes)
			{
				file << i;
			}

			file.close();
		}
	}
}