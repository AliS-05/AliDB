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
	M_PING,
	M_SET,
	M_GET,
	M_DEL,
	M_CHECK,
	M_INCR,
	M_DECR,
	M_UNKNOWN
};

enum class ValueType{
	STRING, //encompass integers floats and booleans (basically just straight bits)
	LIST,
	SET
};

class ValueObject{ //class or struct ? 
	//i think we probably are going to want to have innate functions
	public:
		//SET mykey "Hello, Redis!"
		// command can be handled by server, object needs key and Value
		
		//this might be useless actually
		ValueType type;
		//keys
		std::vector<std::string> list;
		std::unordered_set<std::string> set;
		//values
		std::string valueStr;
				//syntax for enum classes
		ValueObject() : type(ValueType::STRING) {}
		ValueObject(ValueType t) : type(t) {}
};
