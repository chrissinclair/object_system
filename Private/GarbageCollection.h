#pragma once

struct Object;

void CollectGarbage();
void AddToRootSet(Object* object);
void RemoveFromRootSet(Object* object);
