#include <string>
#include <vector>
#include <unordered_set>
#include <variant>
#pragma once

enum class Code{
	SUCCESS,
	ERROR,
	UNKNOWN
};

enum class Method{
	PING,
	//String methods
	SET,
	GET,
	DEL,
	CHECK,
	INCR,
	DECR,
	//List Methods
	
	UNKNOWN
};

enum class ValueType{
	STRING, //encompass integers floats and booleans (basically just straight bits)
	LIST,
	SET
};

class ValueObject{
	public:
		std::variant<std::string,
			std::vector<std::string>, 
			std::unordered_set<std::string>
		> value;

		ValueObject(){}
		template <typename T>
		ValueObject(T v) : value(v) {}
};
