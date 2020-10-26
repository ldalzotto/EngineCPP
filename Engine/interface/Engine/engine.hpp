#pragma once

typedef void* EngineHandle;

EngineHandle engine_create();
void engine_mainloop(const EngineHandle& p_engine);
void engine_destroy(const EngineHandle& p_engine);