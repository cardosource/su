#include "environment.hpp"
#include "runtimeError.hpp"
#include <iostream>

Env::Env() : enclosing(nullptr) {}

Env::Env(Env* enclosing) : enclosing(enclosing) {}

Env::~Env() {}

void Env::define(const std::string& name, SuValue value, int line, bool immutable) {
    if (bindings.count(name)) {
        Token tok{TokenType::IDENTIFIER, name, std::any{}, line};
        throw RuntimeError(tok,
            "The binding '" + name + "' already belongs to the collective. "
            "In .su, all bindings are immutable — the revolution does not allow revisionism.");
    }
    bindings[name] = {value, true, immutable};
}

void Env::assign(const Token& name, SuValue value) {
    auto it = bindings.find(name.lexeme);
    if (it != bindings.end()) {
        if (it->second.immutable) {
            Token tok{TokenType::IDENTIFIER, name.lexeme, std::any{}, name.line};
            throw RuntimeError(tok,
                "Cannot reassign immutable binding: " + name.lexeme + ". "
                "The revolution does not allow revisionism.");
        }
        it->second.value = value;
        it->second.initialized = true;
        return;
    }
    if (enclosing != nullptr) {
        enclosing->assign(name, value);
        return;
    }
    throw RuntimeError(name, "Undefined binding: " + name.lexeme + ".");
}

SuValue Env::get(const Token& name) {
    auto it = bindings.find(name.lexeme);
    if (it != bindings.end()) {
        if (!it->second.initialized) {
            throw RuntimeError(name, "Uninitialized binding: " + name.lexeme);
        }
        return it->second.value;
    }
    if (enclosing != nullptr) {
        return enclosing->get(name);
    }
    throw RuntimeError(name, "Unreferenced binding: " + name.lexeme + ".");
}

SuValue Env::getByName(const std::string& name) {
    auto it = bindings.find(name);
    if (it != bindings.end()) {
        if (!it->second.initialized) {
            return SuValue::nil();
        }
        return it->second.value;
    }
    if (enclosing != nullptr) {
        return enclosing->getByName(name);
    }
    return SuValue::nil();
}

void Env::gcMark() {
    for (auto& pair : bindings) {
        if (pair.second.value.isObj()) {
            GC::instance().mark(pair.second.value.asObj());
        }
    }
    if (enclosing) {
        enclosing->gcMark();
    }
}
