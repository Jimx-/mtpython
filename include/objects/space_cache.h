#ifndef _SPACE_CACHE_H_
#define _SPACE_CACHE_H_

#include "interpreter/typedef.h"
#include "objects/base_object.h"
#include "gc/garbage_collector.h"
#include <unordered_map>

namespace mtpython {

namespace objects {

class ObjSpace;

template <typename K, typename V> class ObjSpaceCache {
private:
    std::unordered_map<K, V> map;

protected:
    ObjSpace* space;

public:
    ObjSpaceCache(ObjSpace* space) { this->space = space; }

    V get(K key)
    {
        auto got = map.find(key);
        if (got == map.end()) {
            V value = build(key);
            map[key] = value;
            return value;
        }
        return got->second;
    }

    virtual V build(K key) { return V(); }

    virtual void mark_objects(gc::GarbageCollector* gc)
    {
        for (const auto& [k, v] : map)
            gc->mark_object(v);
    }
};

/* Typedef -> TypeObject mapping */
class TypedefCache
    : public ObjSpaceCache<interpreter::Typedef*, M_BaseObject*> {
public:
    TypedefCache(ObjSpace* space) : ObjSpaceCache(space) {}
};

/* InterpFunctionWrapper -> Function mapping */
class GatewayCache : public ObjSpaceCache<M_BaseObject*, M_BaseObject*> {
public:
    GatewayCache(ObjSpace* space) : ObjSpaceCache(space) {}

    virtual M_BaseObject* build(M_BaseObject* key);
};

} // namespace objects
} // namespace mtpython

#endif
