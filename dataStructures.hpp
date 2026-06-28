#pragma once
#include <string>
#include <deque>
#include <vector>
#include <unordered_set>
#include <variant>


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
	LPUSH,
	RPUSH,
	LPOP,
	RPOP,
	UNKNOWN
};

enum class ValueType{
	STRING, //encompass integers floats and booleans (basically just straight bits)
	LIST,
	SET
};

struct Return {
	Code code;
	std::string value;
};

class ValueObject{
	public:
		std::variant<std::string,
			std::deque<std::string>, 
			std::unordered_set<std::string>
		> value;

		ValueObject(){}
		template <typename T>
		ValueObject(T v) : value(v) {}
};
