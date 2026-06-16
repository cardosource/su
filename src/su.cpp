#include "su.hpp"
#include "debug.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "scanner.hpp"
#include "visitor.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;


Interpreter interpreter{};


struct GlobalInit {
    GlobalInit() {
    }
} globalInit;

void Su::runFile(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "Erro: Arquivo nao encontrado: " << path << "\n";
        std::exit(66);
    }
    
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo: " << path << "\n";
        std::exit(66);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);

    if (!file.read(buffer.data(), size)) {
        std::cerr << "Erro: Falha ao ler o arquivo: " << path << "\n";
        std::exit(77);	
    }
    
    std::string content(buffer.begin(), buffer.end());
    run(content);
    
    // Limpa memória no final
    GC::instance().shutdown();
    
    if (Debug::hardError) {
        std::exit(65);
    } 
    if (Debug::hardRuntimeError) {
        std::exit(70);
    } 
}

void Su::runPrompt() {
    std::string line;
    std::cout << "su > ";
    
    for (;;) {
        if (!std::getline(std::cin, line) || line == "sair") {
            break;
        }
        
        Debug::hardError = false;
        Debug::hardRuntimeError = false;
        
        run(line);
        
        std::cout << "su > ";
    }
    
    std::cout << "Saindo...\n";
    GC::instance().shutdown();
}

void Su::run(const std::string& source) {
    Scanner scanner(source);
    std::vector<Token> tokens = scanner.scanTokens();
    
    if (Debug::hardError) {
        return;
    }
    
    Parser parser{tokens};
    std::vector<std::shared_ptr<Statement::Stmt>> statements = parser.parser();
    
    if (Debug::hardError) {
        return;
    }
    
    interpreter.interpret(statements);
}