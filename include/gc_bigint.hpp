#pragma once
#include "gc.hpp"
#include "bignum.hpp"

struct SuBigInt : GcObj {
    BigInt value;  

    SuBigInt() : value(0LL) {
        type = ObjType::BIGINT;
    }
    
    explicit SuBigInt(const BigInt& v) : value(v) {
        type = ObjType::BIGINT;
    }
    
    explicit SuBigInt(BigInt&& v) : value(std::move(v)) {
        type = ObjType::BIGINT;
    }
    
    ~SuBigInt() = default;

    static SuBigInt* create(const BigInt& val) {
        void* mem = gcAlloc(sizeof(SuBigInt));
        if(!mem) return nullptr;
        return new(mem) SuBigInt(val);
    }

    static SuBigInt* create(BigInt&& val) {
        void* mem = gcAlloc(sizeof(SuBigInt));
        if(!mem) return nullptr;
        return new(mem) SuBigInt(std::move(val));
    }

    static SuBigInt* create(long long v = 0) {
        void* mem = gcAlloc(sizeof(SuBigInt));
        if(!mem) return nullptr;
        return new(mem) SuBigInt(BigInt(v));
    }
};



inline SuValue makeBigIntVal(const BigInt& v) {
    return SuValue::make_obj(SuBigInt::create(v));
}

inline SuValue makeBigIntVal(BigInt&& v) {
    return SuValue::make_obj(SuBigInt::create(std::move(v)));
}