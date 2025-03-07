#pragma once

#include "Object/Types.h"
#include "Object/Object.h"

struct ObjectHeader {
    ObjectHeader* NextFree;
    u16 Generation;
    u16 Magic;
    ObjectFlags Flags;

    static constexpr u16 RequiredMagic = 0xc0fe;
};
static_assert(sizeof(ObjectHeader) == 16, "Object header should be 16 bytes");

struct ObjectPool {
    ObjectPool(const u32 poolElementSize);

    u32 PoolElementSize;

    void* Allocate();
    void Free(Object* object);
    bool ContainsObject(Object* object) const;
    Array<Array<u8>>& GetBlocks() { return Blocks; }

    static u32 GetPoolSizeForObjectSize(u32 objectSize);
    static ObjectPool* FindObjectPoolContainingObject(Object* object);
    static void* AllocateObject(u32 objectSize);
    void DestroyObject(Object* object);

    static Array<ObjectPool>& GetPools();

private:
    Array<Array<u8>> Blocks;
    ObjectHeader* FreeListHeader = nullptr;

    void AllocateBlock();
};

ObjectHeader* GetHeaderForObject(const Object* object);
