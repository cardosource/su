#pragma once
#include "gc.hpp"
#include <dlfcn.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <functional>

//  Sistema de Bindings Genéricos

class GenericBindings {
private:
    struct Library {
        void* handle;
        std::unordered_map<std::string, void*> functions;
        std::string path;
    };
    
    std::unordered_map<std::string, Library> libraries_;
    
    // Obter valor float de SuValue
    double getFloatValue(const SuValue& v) const {
        if (v.isInt()) return (double)v.asInt();
        if (v.isObj() && v.obj->type == ObjType::BIGFLOAT) {
            return reinterpret_cast<SuBigFloat*>(v.obj)->value;
        }
        return 0.0;
    }
    
    // Converter SuValue para tipos C
    template<typename T>
    T convertValue(const SuValue& v) {
        if constexpr (std::is_same_v<T, int>) {
            return v.isInt() ? (int)v.asInt() : 0;
        }
        else if constexpr (std::is_same_v<T, long>) {
            return v.isInt() ? (long)v.asInt() : 0;
        }
        else if constexpr (std::is_same_v<T, long long>) {
            return v.isInt() ? (long long)v.asInt() : 0;
        }
        else if constexpr (std::is_same_v<T, float>) {
            return (float)getFloatValue(v);
        }
        else if constexpr (std::is_same_v<T, double>) {
            return getFloatValue(v);
        }
        else if constexpr (std::is_same_v<T, const char*>) {
            if (v.isString()) return v.asString()->c_str();
            return "";
        }
        else if constexpr (std::is_same_v<T, char*>) {
            if (v.isString()) return const_cast<char*>(v.asString()->c_str());
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            if (v.isString()) return std::string(v.asString()->c_str());
            return "";
        }
        else if constexpr (std::is_pointer_v<T>) {
            if (v.isObj()) return (T)(v.asObj());
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if (v.isBool()) return v.asBool();
            return false;
        }
        else if constexpr (std::is_same_v<T, void*>) {
            if (v.isObj()) return (void*)(v.asObj());
            return nullptr;
        }
        else {
            return T{};
        }
    }
    
public:
    GenericBindings() = default;
    ~GenericBindings() {
        for (auto& [name, lib] : libraries_) {
            if (lib.handle) dlclose(lib.handle);
        }
    }
    
    // Carregar biblioteca
    bool load(const std::string& name, const std::string& path) {
        if (libraries_.count(name)) return true;
        
        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "Erro ao carregar %s: %s\n", path.c_str(), dlerror());
            return false;
        }
        
        libraries_[name] = {handle, {}, path};
        return true;
    }
    
    // Obter função
    void* getFunction(const std::string& lib, const std::string& func) {
        auto it = libraries_.find(lib);
        if (it == libraries_.end()) return nullptr;
        
        auto& cache = it->second.functions;
        auto cached = cache.find(func);
        if (cached != cache.end()) return cached->second;
        
        dlerror();
        void* fn = dlsym(it->second.handle, func.c_str());
        const char* error = dlerror();
        if (error) {
            fprintf(stderr, "Erro ao buscar %s em %s: %s\n", 
                    func.c_str(), it->second.path.c_str(), error);
            return nullptr;
        }
        
        cache[func] = fn;
        return fn;
    }
    
    // Chamar função genérica
    SuValue call(const std::string& lib, const std::string& func, 
                 const std::vector<SuValue>& args) {
        void* fn = getFunction(lib, func);
        if (!fn) return SuValue::nil();
        
        if (args.size() <= 4) {
            if (args.size() == 0) {
                int (*f)() = (int(*)())fn;
                return SuValue::make_int(f());
            }
            else if (args.size() == 1) {
                // Tentar diferentes assinaturas
                if (args[0].isInt()) {
                    int (*f)(int) = (int(*)(int))fn;
                    return SuValue::make_int(f(convertValue<int>(args[0])));
                }
                else if (args[0].isFloat()) {
                    double (*f)(double) = (double(*)(double))fn;
                    double result = f(getFloatValue(args[0]));
                    return SuValue::make_obj(SuBigFloat::create(result));
                }
                else if (args[0].isString()) {
                    const char* (*f)(const char*) = (const char*(*)(const char*))fn;
                    const char* result = f(convertValue<const char*>(args[0]));
                    if (result) return SuValue::make_obj(SuString::create(result));
                    return SuValue::nil();
                }
                else {
                    void (*f)(void*) = (void(*)(void*))fn;
                    f(convertValue<void*>(args[0]));
                    return SuValue::nil();
                }
            }
            else if (args.size() == 2) {
                if (args[0].isInt() && args[1].isInt()) {
                    int (*f)(int, int) = (int(*)(int, int))fn;
                    return SuValue::make_int(f(convertValue<int>(args[0]), 
                                               convertValue<int>(args[1])));
                }
                else {
                    double (*f)(double, double) = (double(*)(double, double))fn;
                    double result = f(getFloatValue(args[0]), getFloatValue(args[1]));
                    return SuValue::make_obj(SuBigFloat::create(result));
                }
            }
            else if (args.size() == 3) {
                int (*f)(int, int, int) = (int(*)(int, int, int))fn;
                return SuValue::make_int(f(convertValue<int>(args[0]),
                                           convertValue<int>(args[1]),
                                           convertValue<int>(args[2])));
            }
            else if (args.size() == 4) {
                int (*f)(int, int, int, int) = (int(*)(int, int, int, int))fn;
                return SuValue::make_int(f(convertValue<int>(args[0]),
                                           convertValue<int>(args[1]),
                                           convertValue<int>(args[2]),
                                           convertValue<int>(args[3])));
            }
        }
        
        return SuValue::nil();
    }
    
    // Descarregar biblioteca
    void unload(const std::string& name) {
        auto it = libraries_.find(name);
        if (it != libraries_.end()) {
            dlclose(it->second.handle);
            libraries_.erase(it);
        }
    }
    
    // Verificar se biblioteca está carregada
    bool isLoaded(const std::string& name) const {
        return libraries_.count(name) > 0;
    }
    
    // Listar bibliotecas carregadas
    std::vector<std::string> listLibraries() const {
        std::vector<std::string> result;
        for (const auto& [name, lib] : libraries_) {
            result.push_back(name);
        }
        return result;
    }
};