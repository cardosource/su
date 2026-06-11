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

// Instância global do interpretador
Interpreter interpreter{};

void Su::runFile(const std::string& path) {
    // Verifica se o arquivo existe
    if (!fs::exists(path)) {
        std::cerr << "Erro: Arquivo não encontrado: " << path << "\n";
        std::exit(66);
    }
    
    // Abre o arquivo
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Erro: Não foi possível abrir o arquivo: " << path << "\n";
        std::exit(66);
    }
    
    // Lê o conteúdo do arquivo
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);

    if (!file.read(buffer.data(), size)) {
        std::cerr << "Erro: Falha ao ler o arquivo: " << path << "\n";
        std::exit(77);	
    }
    
    // Converte para string e executa
    std::string content(buffer.begin(), buffer.end());
    run(content);
    
    // Verifica erros e sai com código apropriado
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
        // Lê uma linha de entrada
        if (!std::getline(std::cin, line) || line == "sair") {
            break;
        }
        
        // Reseta as flags de erro para cada linha
        Debug::hardError = false;
        Debug::hardRuntimeError = false;
        
        // Executa a linha
        run(line);
        
        // Continua para próxima linha mesmo se houver erro
        std::cout << "su > ";
    }
    
    std::cout << "Saindo...\n";
}

void Su::run(const std::string& source) {
    // Fase 1: Scanning (análise léxica)
    Scanner scanner(source);
    std::vector<Token> tokens = scanner.scanTokens();
    
    // Verifica se houve erro no scanning
    if (Debug::hardError) {
        return;
    }
    
    // Debug: mostra os tokens
    /*
    std::cout << "Tokens:\n";
    for (const auto& token : tokens) {
        std::cout << "  " << token.toString() << "\n";
    }
    */
    
    // Fase 2: Parsing (análise sintática)
    Parser parser{tokens};
    std::vector<std::shared_ptr<Statement::Stmt>> statements = parser.parser();
    
    // Verifica se houve erro no parsing
    if (Debug::hardError) {
        return;
    }
    
    // Fase 3: Interpretação
    interpreter.interpret(statements);
    if(Debug::hardError){return;}
} 
