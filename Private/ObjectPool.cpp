#include "ObjectPool.h"

Array<ObjectPool>& ObjectPool::GetPools() {
    static Array<ObjectPool> pools;
    return pools;
}

ObjectHeader* GetHeaderForObject(const Object* object) {
    ObjectHeader* header = (ObjectHeader*)((u8*) object - sizeof(ObjectHeader));

    if (header->Magic != ObjectHeader::RequiredMagic) {
        return nullptr;
    }

    return header;
}

ObjectPool::ObjectPool(const u32 poolElementSize)
    : PoolElementSize(poolElementSize)
{}

void* ObjectPool::Allocate() {
    if (!FreeListHeader) {
        AllocateBlock();
        if (!FreeListHeader) {
            return nullptr;
        }
    }

    ObjectHeader* header = FreeListHeader;
    FreeListHeader = FreeListHeader->NextFree;
    header->Generation++;
    SetFlag(header->Flags, ObjectFlags::Allocated);
    SetFlag(header->Flags, ObjectFlags::Unreachable);
    header->NextFree = nullptr;

    return header + 1;
}

void ObjectPool::Free(Object* object) {
    ObjectHeader* header = GetHeaderForObject(object);
    if (header) {
        header->Generation++;
        UnsetFlag(header->Flags, ObjectFlags::Allocated);
        UnsetFlag(header->Flags, ObjectFlags::Unreachable);
        header->NextFree = FreeListHeader;
        FreeListHeader = header;
    }
}

bool ObjectPool::ContainsObject(Object* object) const {
    u8* address = (u8*) object;
    for (const Array<u8>& block : Blocks) {
        if (block.data() < address && address < (block.data() + block.size())) {
            return true;
        }
    }
    return false;
}

u32 ObjectPool::GetPoolSizeForObjectSize(const u32 objectSize) {
    return objectSize;
}

ObjectPool* ObjectPool::FindObjectPoolContainingObject(Object* object) {
    Array<ObjectPool>& pools = GetPools();
    auto it = std::find_if(pools.begin(), pools.end(), [object](const ObjectPool& pool) {
        return pool.ContainsObject(object);
    });

    if (it == pools.end()) {
        return nullptr;
    }

    return &*it;
}

void* ObjectPool::AllocateObject(u32 objectSize) {
    if (objectSize == 0) [[unlikely]] {
        objectSize = 1;
    }

    Array<ObjectPool>& pools = GetPools();
    const u32 poolSizeForAllocation = GetPoolSizeForObjectSize(objectSize);
    auto it = std::find_if(pools.begin(), pools.end(), [poolSizeForAllocation](const ObjectPool& pool) {
        return pool.PoolElementSize == poolSizeForAllocation;
    });

    if (it == pools.end()) {
        it = pools.emplace(pools.end(), poolSizeForAllocation);
    }

    if (it == pools.end()) [[unlikely]] {
        return nullptr;
    }

    return it->Allocate();
}

void ObjectPool::DestroyObject(Object* object) {
    object->~Object();
    Free(object);
}

void ObjectPool::AllocateBlock() {
    const i32 numberOfObjectsToAllocate = 128; // Todo - this should be configurable
    const u64 blockElementSize = PoolElementSize + sizeof(ObjectHeader);
    const u64 allocationSize = numberOfObjectsToAllocate * blockElementSize;
    Array<u8> block(allocationSize);
    for (i32 index = 0; index < numberOfObjectsToAllocate; index++) {
        ObjectHeader* header = (ObjectHeader*)(block.data() + index * blockElementSize);
        header->Generation = 0;
        header->Flags = ObjectFlags::None;
        header->Magic = ObjectHeader::RequiredMagic;
        header->NextFree = index < (numberOfObjectsToAllocate - 1) ?
            (ObjectHeader*)(block.data() + (index + 1) * blockElementSize) :
            FreeListHeader;
    }

    FreeListHeader = (ObjectHeader*)block.data();
    Blocks.emplace_back(Move(block));
}
