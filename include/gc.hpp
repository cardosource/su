#pragma once
#include "su_config.hpp"
#include <cstring>
#include <cstddef>

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
struct SuList;

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
    bool isList()   const { return isObj() && obj->type == ObjType::LIST; }
    

    GcObj*    asObj()    const { return obj; }
    int64_t   asInt()    const { return integer; }
    bool      asBool()   const { return boolean; }
    SuString* asString() const { return reinterpret_cast<SuString*>(obj); }
    SuList*   asList()   const { return reinterpret_cast<SuList*>(obj); }

    void gcMark() const;
};

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

struct SuList : GcObj {
    static constexpr size_t MAX_ELEMENTS = 128;
    SuValue elements[MAX_ELEMENTS];
    size_t length = 0;

    static SuList* create() {
        void* mem = gcAlloc(sizeof(SuList));
        if (!mem) return nullptr;
        SuList* list = new(mem) SuList();
        list->type = ObjType::LIST;
        list->marked = false;
        list->next = nullptr;
        list->length = 0;
        return list;
    }

    static SuList* fromArray(const SuValue* arr, size_t len) {
        if (len > MAX_ELEMENTS) return nullptr;
        SuList* list = create();
        if (!list) return nullptr;
        list->length = len;
        for (size_t i = 0; i < len; i++) {
            list->elements[i] = arr[i];
        }
        return list;
    }

    SuValue get(size_t index) const {
        if (index >= length) return SuValue::nil();
        return elements[index];
    }

    SuList* concat(const SuList* other) const {
        size_t newLen = length + other->length;
        if (newLen > MAX_ELEMENTS) return nullptr;
        
        SuList* result = create();
        if (!result) return nullptr;
        
        for (size_t i = 0; i < length; i++) {
            result->elements[i] = elements[i];
        }
        for (size_t i = 0; i < other->length; i++) {
            result->elements[length + i] = other->elements[i];
        }
        result->length = newLen;
        return result;
    }

    SuList* sublist(size_t start, size_t end) const {
        if (start > end || end > length) return nullptr;
        size_t newLen = end - start;
        if (newLen == 0) return create();
        
        SuList* result = create();
        if (!result) return nullptr;
        
        for (size_t i = 0; i < newLen; i++) {
            result->elements[i] = elements[start + i];
        }
        result->length = newLen;
        return result;
    }

    bool contains(const SuValue& v) const {
        for (size_t i = 0; i < length; i++) {
            if (isEqual(elements[i], v)) return true;
        }
        return false;
    }

    size_t size() const { return length; }
    bool empty() const { return length == 0; }

private:
    SuList() = default;

    static bool isEqual(const SuValue& a, const SuValue& b) {
        if (a.isNil() && b.isNil()) return true;
        if (a.isNil() || b.isNil()) return false;
        if (a.isBool() && b.isBool()) return a.asBool() == b.asBool();
        if (a.isInt() && b.isInt()) return a.asInt() == b.asInt();
        if (a.isString() && b.isString()) {
            return strcmp(a.asString()->c_str(), b.asString()->c_str()) == 0;
        }
        if (a.isObj() && b.isObj() && a.obj->type == ObjType::LIST && b.obj->type == ObjType::LIST) {
            const SuList* la = reinterpret_cast<const SuList*>(a.obj);
            const SuList* lb = reinterpret_cast<const SuList*>(b.obj);
            if (la->length != lb->length) return false;
            for (size_t i = 0; i < la->length; i++) {
                if (!isEqual(la->elements[i], lb->elements[i])) return false;
            }
            return true;
        }
        return false;
    }
};

inline void SuValue::gcMark() const {
    if (isObj() && obj) GC::instance().mark(obj);
}