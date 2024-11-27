#include "RrConfig.h"
#include <fstream>
#include <stdlib.h>

namespace rr
{

	//Check if it is a space or TAB character
	bool RrConfig::IsSpace(char c)
	{
		if (' ' == c || '\t' == c)
			return true;
		return false;
	}

	// Check if it is a "#"" or public character
	bool RrConfig::IsCommentChar(char c)
	{
		switch (c) {
		case '#':
			return true;
		default:
			return false;
		}
	}
	//Extract the contents of that line and store them in "str"
	void RrConfig::Trim(std::string& str)
	{
		if (str.empty())
		{
			return;
		}
		long unsigned int i;
		int start_pos, end_pos;
		for (i = 0; i < str.size(); ++i) {
			if (!IsSpace(str[i])) {
				break;
			}
		}
		if (i == str.size())
		{
			str = "";
			return;
		}
		start_pos = i;
		for (i = str.size() - 1; i >= 0; --i) {
			if (!IsSpace(str[i])) {
				break;
			}
		}
		end_pos = i;
		str = str.substr(start_pos, end_pos - start_pos + 1);
	}

	//Put the values in [] in "section", and get the values to the left and right of the equal sign in "key" and "value".
	bool RrConfig::AnalyseLine(const std::string& line, std::string& section, std::string& key, std::string& value)
	{
		if (line.empty())
			return false;
		int start_pos = 0, end_pos = line.size() - 1, pos, s_startpos, s_endpos;
		if ((pos = line.find("#")) != -1)   //Read only the string before "#"".
		{
			if (0 == pos)
			{
				return false;
			}
			end_pos = pos - 1;
		}
		
		// Find the value in [] and concatenate it into a string in "substr".
		if (((s_startpos = line.find("[")) != -1) && ((s_endpos = line.find("]"))) != -1)
		{
			section = line.substr(s_startpos + 1, s_endpos - 1);
			return true;
		}
		std::string new_line = line.substr(start_pos, start_pos + 1 - end_pos); //获得整行字符串。
		if ((pos = new_line.find('=')) == -1)
			return false;
		
		// Get the key and value
		key = new_line.substr(0, pos);  //Get the value to the left of the equal sign
		value = new_line.substr(pos + 1, end_pos + 1 - (pos + 1));   //Get the value to the right of the equal sign.
		Trim(key);
		if (key.empty()) {
			return false;
		}
		Trim(value);
		if ((pos = value.find("\r")) > 0)   //Delete "\r"
		{
			value.replace(pos, 1, "");
		}
		if ((pos = value.find("\n")) > 0)  //Delete "\n"
		{
			value.replace(pos, 1, "");
		}
		return true;
	}

	//Open and read the configuration file, and store the data structure information in the "map" container "settings"
	bool RrConfig::ReadConfig(const std::string& filename)
	{
		settings_.clear();
		std::ifstream infile(filename.c_str());
		if (!infile) {
			return false;
		}
		std::string line, key, value, section;
		std::map<std::string, std::string> k_v;
		std::map<std::string, std::map<std::string, std::string> >::iterator it;
		while (getline(infile, line))
		{
			if (AnalyseLine(line, section, key, value))  //If we find an equal sign on that line, we enter the if statement and store the data structure in the "map" container "settings"
			{
				it = settings_.find(section);
				if (it != settings_.end())
				{
					k_v[key] = value;
					it->second = k_v;
				}
				else
				{
					k_v.clear();
					settings_.insert(std::make_pair(section, k_v));
				}
			}
			key.clear();
			value.clear();
		}
		infile.close();
		return true;
	}

	//The key is "itme" and the value is the value of the string.
	std::string RrConfig::ReadString(const char* section, const char* item, const char* default_value)
	{
		std::string tmp_s(section);
		std::string tmp_i(item);
		std::string def(default_value);
		std::map<std::string, std::string> k_v;
		std::map<std::string, std::string>::iterator it_item;
		std::map<std::string, std::map<std::string, std::string> >::iterator it;
		it = settings_.find(tmp_s);
		if (it == settings_.end())
		{
			//printf("111");
			return def;
		}
		k_v = it->second;
		it_item = k_v.find(tmp_i);
		if (it_item == k_v.end())
		{
			//printf("222");
			return def;
		}
		return it_item->second;
	}

	int RrConfig::ReadInt(const char* section, const char* item, const int& default_value)
	{
		std::string tmp_s(section);
		std::string tmp_i(item);
		std::map<std::string, std::string> k_v;
		std::map<std::string, std::string>::iterator it_item;
		std::map<std::string, std::map<std::string, std::string> >::iterator it;
		it = settings_.find(tmp_s);
		if (it == settings_.end())
		{
			return default_value;
		}
		k_v = it->second;
		it_item = k_v.find(tmp_i);
		if (it_item == k_v.end())
		{
			return default_value;
		}
		return atoi(it_item->second.c_str());
	}

	//Read floating-point values from "item"
	float RrConfig::ReadFloat(const char* section, const char* item, const float& default_value)
	{
		std::string tmp_s(section);
		std::string tmp_i(item);
		std::map<std::string, std::string> k_v;
		std::map<std::string, std::string>::iterator it_item;
		std::map<std::string, std::map<std::string, std::string> >::iterator it;
		it = settings_.find(tmp_s);
		if (it == settings_.end())
		{
			return default_value;
		}
		k_v = it->second;
		it_item = k_v.find(tmp_i);
		if (it_item == k_v.end())
		{
			return default_value;
		}
		return atof(it_item->second.c_str());
	}
}
