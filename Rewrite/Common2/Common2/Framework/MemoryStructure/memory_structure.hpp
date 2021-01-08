#pragma once

#define declare_indexed_pool(ElementType, PoolName) \
Pool<##ElementType##> PoolName;\
Vector<Token(ElementType)> PoolName##_index;

#define declare_pool(ElementType, PoolName) Pool<##ElementType##> PoolName
