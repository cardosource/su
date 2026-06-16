#pragma once
#include "su_config.hpp"
#include <cstring>

enum class ObjType : uint8_t {
    STRING,
    BIGINT,
    BIGFLOAT,
    LIST,
    CLOSURE,
};

struct GcObj {
    GcObj*  next    = nullptr;
    ObjType type    = ObjType::STRING;
    bool    marked  = false;
    uint8_t age     = 0;
};

struct SuString;
struct SuBigFloat;
struct SuBigInt;


struct SuValue {
    enum class Tag : uint8_t { NIL, BOOL, INT, OBJ } tag = Tag::NIL;

    union {
        bool    boolean;
        int64_t integer;
        GcObj*  obj;
    };

    static SuValue nil()              { SuValue v; v.tag = Tag::NIL; v.obj = nullptr; return v; }
    static SuValue make_bool(bool b)  { SuValue v; v.tag = Tag::BOOL; v.boolean = b; return v; }
    static SuValue make_int(int64_t i){ SuValue v; v.tag = Tag::INT;  v.integer = i; return v; }
    static SuValue make_obj(GcObj* o) { SuValue v; v.tag = Tag::OBJ;  v.obj = o; return v; }

    bool isNil()    const { return tag == Tag::NIL; }
    bool isBool()   const { return tag == Tag::BOOL; }
    bool isInt()    const { return tag == Tag::INT; }
    bool isObj()    const { return tag == Tag::OBJ && obj != nullptr; }
    bool isString() const { return isObj() && obj->type == ObjType::STRING; }
    bool isFloat()  const { return isObj() && obj->type == ObjType::BIGFLOAT; }
    bool isBigInt() const { return isObj() && obj->type == ObjType::BIGINT; }

    GcObj*    asObj()    const { return obj; }
    int64_t   asInt()    const { return integer; }
    bool      asBool()   const { return boolean; }
    SuString* asString() const { return reinterpret_cast<SuString*>(obj); }

    void gcMark() const;
};
//  GC - Gerenciador de memória

class GC {
public:
    static GC& instance() { static GC gc; return gc; }

    void* alloc(size_t size);
    void collect();
    void mark(GcObj* obj);
    void addRoot(GcObj** root);
    void removeRoot(GcObj** root);
    void step(int maxSteps = 1);
    void shutdown();

    size_t bytesAllocated() const { return bytesAllocated_; }
    size_t objectCount() const { return objectCount_; }

private:
    GC();
    ~GC();

    enum class State { IDLE, MARK, SWEEP };
    State state_ = State::IDLE;

    GcObj* objects_ = nullptr;
    size_t bytesAllocated_ = 0;
    size_t objectCount_ = 0;
    size_t threshold_ = SU_GC_THRESHOLD;

    GcObj** grayStack_ = nullptr;
    int grayCount_ = 0;
    int grayCapacity_ = 0;
    GcObj** sweepPtr_ = nullptr;

    static constexpr int MAX_ROOTS = 256;
    GcObj** roots_[MAX_ROOTS];
    int rootCount_ = 0;

    void markRoots();
    bool markStep();
    bool sweepStep();
    void pushGray(GcObj* obj);
    GcObj* popGray();
    void growGrayStack();
    void freeObj(GcObj* obj);

#ifdef SU_USE_ARENA
    uint8_t* arena_;
    size_t arenaTop_;
    size_t arenaSize_;
    void* arenaAlloc(size_t size);
#else
    void* heapAlloc(size_t size);
#endif
};

inline void* gcAlloc(size_t size) {
    return GC::instance().alloc(size);
}


struct SuString : GcObj {
    static constexpr size_t MAX_LEN = SU_MAX_STRING_LEN;
    char data[MAX_LEN];
    size_t len = 0;

    static SuString* create(const char* s, size_t len);
    static SuString* create(const char* s);
    const char* c_str() const { return data; }
};

struct SuBigFloat : GcObj {
    double value = 0.0;
    static SuBigFloat* create(double v);
};

inline void SuValue::gcMark() const {
    if (isObj() && obj) GC::instance().mark(obj);
}