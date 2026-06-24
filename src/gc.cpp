#include "gc.hpp"
#include "gc_bigint.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>


GC::GC() : objects_(nullptr), 
           bytesAllocated_(0), 
           objectCount_(0),
           threshold_(SU_GC_THRESHOLD),
           state_(State::IDLE),
           grayStack_(nullptr),
           grayCount_(0),
           grayCapacity_(0),
           sweepPtr_(nullptr),
           rootCount_(0) {
    
#ifdef SU_USE_ARENA
    arenaSize_ = SU_ARENA_SIZE;
    arena_ = (uint8_t*)malloc(arenaSize_);
    arenaTop_ = 0;
    if (arena_) {
        memset(arena_, 0, arenaSize_);
    }
#endif
}


GC::~GC() {
    free(grayStack_);
    
    GcObj* obj = objects_;
    while (obj) {
        GcObj* next = obj->next;
        freeObj(obj);
        obj = next;
    }
    
#ifdef SU_USE_ARENA
    free(arena_);
#endif
}


void* GC::alloc(size_t size) {
    size = (size + 7) & ~(size_t)7;

    if (bytesAllocated_ >= threshold_) {
        step(1);
    }

    if (bytesAllocated_ >= threshold_) {
        collect();
        threshold_ = bytesAllocated_ * 2;
        if (threshold_ < SU_GC_THRESHOLD) {
            threshold_ = SU_GC_THRESHOLD;
        }
    }

#ifdef SU_USE_ARENA
    void* mem = arenaAlloc(size);
#else
    void* mem = heapAlloc(size);
#endif
    if (!mem) return nullptr;

    GcObj* obj = static_cast<GcObj*>(mem);
    obj->next = objects_;
    obj->marked = false;
    obj->age = 0;
    obj->type = ObjType::STRING;
    objects_ = obj;

    bytesAllocated_ += size;
    objectCount_++;
    return mem;
}

void GC::mark(GcObj* obj) {
    if (!obj || obj->marked) return;
    obj->marked = true;
    pushGray(obj);
}


void GC::collect() {
    while (state_ != State::IDLE) {
        if (state_ == State::MARK) {
            while (markStep()) {}
            state_ = State::SWEEP;
            sweepPtr_ = &objects_;
        } else if (state_ == State::SWEEP) {
            while (sweepStep()) {}
            state_ = State::IDLE;
        }
    }

    for (GcObj* o = objects_; o; o = o->next) {
        o->marked = false;
    }
    markRoots();

    GcObj** curr = &objects_;
    while (*curr) {
        if (!(*curr)->marked) {
            GcObj* dead = *curr;
            *curr = dead->next;
            freeObj(dead);
            objectCount_--;
        } else {
            (*curr)->marked = false;
            curr = &(*curr)->next;
        }
    }
}

void GC::shutdown() {
    while (state_ != State::IDLE) {
        if (state_ == State::MARK) {
            while (markStep()) {}
            state_ = State::SWEEP;
        } else if (state_ == State::SWEEP) {
            while (sweepStep()) {}
            state_ = State::IDLE;
        }
    }
    collect();
}

void GC::step(int maxSteps) {
    int steps = 0;

    if (state_ == State::IDLE && bytesAllocated_ >= threshold_) {
        state_ = State::MARK;
        markRoots();
    }

    while (state_ != State::IDLE && steps < maxSteps) {
        if (state_ == State::MARK) {
            if (!markStep()) {
                state_ = State::SWEEP;
                sweepPtr_ = &objects_;
                continue;
            }
        } else if (state_ == State::SWEEP) {
            if (!sweepStep()) {
                state_ = State::IDLE;
                threshold_ = bytesAllocated_ * 2;
                if (threshold_ < SU_GC_THRESHOLD) {
                    threshold_ = SU_GC_THRESHOLD;
                }
                break;
            }
        }
        steps++;
    }
}

void GC::markRoots() {
    for (int i = 0; i < rootCount_; i++) {
        if (roots_[i] && *roots_[i]) {
            mark(*roots_[i]);
        }
    }
}

bool GC::markStep() {
    if (grayCount_ == 0) return false;
    GcObj* obj = popGray();
    if (!obj) return false;
    
    if (obj->type == ObjType::LIST) {
        SuList* list = reinterpret_cast<SuList*>(obj);
        for (size_t i = 0; i < list->length; i++) {
            if (list->elements[i].isObj()) {
                mark(list->elements[i].asObj());
            }
        }
    }
    
    return true;
}

bool GC::sweepStep() {
    if (!sweepPtr_) return false;

    while (*sweepPtr_) {
        if (!(*sweepPtr_)->marked) {
            GcObj* dead = *sweepPtr_;
            *sweepPtr_ = dead->next;
            freeObj(dead);
            objectCount_--;
            return true;
        } else {
            (*sweepPtr_)->marked = false;
            sweepPtr_ = &(*sweepPtr_)->next;
        }
    }
    sweepPtr_ = nullptr;
    return false;
}

void GC::pushGray(GcObj* obj) {
    if (!obj) return;
    if (grayCount_ >= grayCapacity_) growGrayStack();
    grayStack_[grayCount_++] = obj;
}

GcObj* GC::popGray() {
    if (grayCount_ == 0) return nullptr;
    return grayStack_[--grayCount_];
}

void GC::growGrayStack() {
    int newCapacity = grayCapacity_ == 0 ? 64 : grayCapacity_ * 2;
    GcObj** newStack = (GcObj**)realloc(grayStack_, newCapacity * sizeof(GcObj*));
    if (newStack) {
        grayStack_ = newStack;
        grayCapacity_ = newCapacity;
    }
}

void GC::addRoot(GcObj** root) {
    if (rootCount_ < MAX_ROOTS) {
        roots_[rootCount_++] = root;
    }
}

void GC::removeRoot(GcObj** root) {
    for (int i = 0; i < rootCount_; i++) {
        if (roots_[i] == root) {
            roots_[i] = roots_[--rootCount_];
            return;
        }
    }
}

void GC::freeObj(GcObj* obj) {
    size_t size = 0;
    switch (obj->type) {
        case ObjType::STRING:
            size = sizeof(SuString);
            break;
        case ObjType::BIGINT:
            reinterpret_cast<SuBigInt*>(obj)->~SuBigInt();
            size = sizeof(SuBigInt);
            break;
        case ObjType::BIGFLOAT:
            size = sizeof(SuBigFloat);
            break;
        case ObjType::LIST:
            size = sizeof(SuList);
            break;
        default:
            size = sizeof(GcObj);
            break;
    }
    size = (size + 7) & ~(size_t)7;
    if (bytesAllocated_ >= size) {
        bytesAllocated_ -= size;
    }

#ifndef SU_USE_ARENA
    free(obj);
#endif
}

#ifdef SU_USE_ARENA
void* GC::arenaAlloc(size_t size) {
    if (!arena_ || arenaTop_ + size > arenaSize_) {
        return nullptr;
    }
    void* ptr = arena_ + arenaTop_;
    arenaTop_ += size;
    return ptr;
}
#endif

#ifndef SU_USE_ARENA
void* GC::heapAlloc(size_t size) {
    return malloc(size);
}
#endif

SuString* SuString::create(const char* s, size_t length) {
    void* mem = gcAlloc(sizeof(SuString));
    if (!mem) return nullptr;
    
    SuString* str = static_cast<SuString*>(mem);
    str->type = ObjType::STRING;
    str->marked = false;
    str->next = nullptr;
    str->len = length < MAX_LEN ? length : MAX_LEN - 1;
    memcpy(str->data, s, str->len);
    str->data[str->len] = '\0';
    return str;
}

SuString* SuString::create(const char* s) {
    return create(s, strlen(s));
}

SuBigFloat* SuBigFloat::create(double v) {
    void* mem = gcAlloc(sizeof(SuBigFloat));
    if (!mem) return nullptr;
    
    SuBigFloat* obj = static_cast<SuBigFloat*>(mem);
    obj->type = ObjType::BIGFLOAT;
    obj->marked = false;
    obj->next = nullptr;
    obj->value = v;
    return obj;
}