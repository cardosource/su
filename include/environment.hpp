#pragma once
#include "su_config.hpp"
#include "gc.hpp"
#include "token.hpp"

#ifdef SU_NO_STL
    // Versão sem STL (Arduino AVR)
    #include <cstring>
    
    struct Binding {
        char name[32];
        SuValue value;
        bool initialized;
        bool immutable;
    };
    
    class Env {
    public:
        Env();
        Env(Env* enclosing);
        ~Env();

        void define(const std::string& name, SuValue value, int line = 0, bool immutable = true);
        void assign(const Token& name, SuValue value);
        SuValue get(const Token& name);
        SuValue getByName(const std::string& name);
        void gcMark();

        Env* enclosing;
        
    private:
        Binding bindings[SU_MAX_VARS];
        int bindingCount;
    };
    
#else
    // Versão com STL (PC, ESP32, etc)
    #include <string>
    #include <unordered_map>
    
    struct Binding {
        SuValue value;
        bool initialized;
        bool immutable;
    };
    
    class Env {
    public:
        Env();
        Env(Env* enclosing);
        ~Env();

        void define(const std::string& name, SuValue value, int line = 0, bool immutable = true);
        void assign(const Token& name, SuValue value);
        SuValue get(const Token& name);
        SuValue getByName(const std::string& name);
        void gcMark();

        Env* enclosing;
        
    private:
        std::unordered_map<std::string, Binding> bindings;
    };
#endif