#pragma once

#define COMA ,

#define cast(Type, Value) ((Type)(Value))
#define castv(Type, Value) *(Type*)(&Value)

#define loop(Iteratorname, BeginNumber, EndNumber) uimax Iteratorname = BeginNumber; Iteratorname < EndNumber; Iteratorname++
#define loop_int16(Iteratorname, BeginNumber, EndNumber) int16 Iteratorname = BeginNumber; Iteratorname < EndNumber; Iteratorname++

#define loop_reverse(Iteratorname, BeginNumber, EndNumber) uimax Iteratorname = BeginNumber; Iteratorname != EndNumber; --Iteratorname

#define vector_loop(VectorVariable, Iteratorname) uimax Iteratorname = 0; Iteratorname < (VectorVariable)->Size; Iteratorname++
#define vector_loop_reverse(VectorVariable, Iteratorname) uimax Iteratorname = (VectorVariable)->Size - 1; Iteratorname != -1; --Iteratorname

#define pool_loop(PoolVariable, Iteratorname) uimax Iteratorname = 0; Iteratorname < (PoolVariable)->get_size(); Iteratorname++

#define varyingvector_loop(VaryingVectorVariable, Iteratorname) uimax Iteratorname = 0; Iteratorname < (VaryingVectorVariable)->get_size(); Iteratorname++