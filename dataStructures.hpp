#include <string>
#include <vector>
#include <unordered_set>
#pragma once

enum class ValueType{
	STRING, //encompass integers floats and booleans (basically just straight bits)
	LIST,
	SET
};

class ValueObject{ //class or struct ? 
	//i think we probably are going to want to have innate functions
	public:
		ValueType type;
		std::string str;
		std::vector<std::string> list;
		std::unordered_set<std::string> set;
				//syntax for enum classes
		ValueObject() : type(ValueType::STRING) {}
		ValueObject(ValueType t) : type(t) {}
};
