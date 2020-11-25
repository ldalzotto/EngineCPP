#pragma once

#include "Scene/scene.hpp"
#include "AssetServer/asset_server.hpp"
#include <string>

typedef void* EngineHandle;

struct ExternalHooks
{
	void* closure = nullptr;
	void(*ext_update)(void* p_closure, float p_delta) = nullptr;
	ExternalHooks() {};
};

EngineHandle engine_create(const std::string& p_executeable_path, const ExternalHooks& p_hooks);
void engine_mainloop(const EngineHandle& p_engine);
bool engine_should_close(const EngineHandle& p_engine);
void engine_poll_events(const EngineHandle& p_engine);
void engine_singleframe(const EngineHandle& p_engine);
void engine_singleframe(const EngineHandle& p_engine, float p_delta);
void engine_destroy(const EngineHandle& p_engine);
void engine_exit(const EngineHandle& p_engine);
SceneHandle engine_scene(const EngineHandle& p_engine);
AssetServerHandle engine_assetserver(const EngineHandle& p_engine);