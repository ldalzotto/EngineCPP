#pragma once

#define COMA ,

#define cast(Type, Value) ((Type)(Value))
#define castv(Type, Value) *(Type*)(&Value)

#define loop(Iteratorname, BeginNumber, EndNumber) size_t Iteratorname = BeginNumber; Iteratorname < EndNumber; Iteratorname++
#define loop_si(Iteratorname, BeginNumber, EndNumber) short int Iteratorname = BeginNumber; Iteratorname < EndNumber; Iteratorname++