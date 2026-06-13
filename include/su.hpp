#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "scanner.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

class Su{
	private:
	static void run(const std::string&);
	public:
	static void runFile(const std::string&);
	static void runPrompt(); 
};
