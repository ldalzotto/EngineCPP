#include <stdio.h>

#include "Common/Container/pool.hpp"
#include "Common/Thread/thread.hpp"
#include "Common/File/file.hpp"
#include "Engine/engine.hpp"
#include "Common/Functional/ToString.hpp"
#include "Scene/scene.hpp"
#include "Scene/kernel/scene.hpp"
#include "SceneSerialization/scene_serialization.hpp"
#include "Middleware/scene_middleware.hpp"
#include "Input/input.hpp"
#include "Common/Clock/clock.hpp"
#include "Math/math.hpp"
#include "Math/contants.hpp"
#include "SceneComponents/components.hpp"
#include "Middleware/render_middleware.hpp"

struct MainToolConstants
{
	inline static const SceneNodeTag EditorNodeTag = SceneNodeTag(Hash<ConstString>::hash("editor"));

	struct SceneNodeEditorFilter
	{
		inline bool evaluate(NTreeResolve<SceneNode>& p_node)
		{
			return !SceneKernel::contains_tag(p_node.element, MainToolConstants::EditorNodeTag);
		};
	};

	inline static SceneNodeEditorFilter sceneNodeEditorFilter = SceneNodeEditorFilter();
};

struct ScenePersister
{
	inline static void persist(EngineHandle& p_engine, Scene* p_scene, const String<>& p_scene_asset_path)
	{
		AssetServerHandle l_asset_server = engine_assetserver(p_engine);

		SceneAsset l_dup_asset = SceneSerializer2::Scene_to_SceneAsset(*p_scene, MainToolConstants::sceneNodeEditorFilter);
		com::Vector<char> l_scene_json = SceneSerializer2::SceneAsset_to_JSON(l_dup_asset, l_asset_server);

		//backup
		{
			{
				File<FilePathMemoryLayout::STRING> l_tmp_scene_folder;
				FilePath<FilePathMemoryLayout::STRING> l_tmp_scene_folder_path;
				l_tmp_scene_folder_path.allocate(0);
				l_tmp_scene_folder_path.path.append(l_asset_server.get_asset_basepath().c_str());
				l_tmp_scene_folder_path.path.append(".tmp/scenes/");
				l_tmp_scene_folder.allocate(FileType::FOLDER, l_tmp_scene_folder_path);
				l_tmp_scene_folder.create();
				l_tmp_scene_folder.free();
			}

			File<FilePathMemoryLayout::STRING> l_scene_file;
			FilePath<FilePathMemoryLayout::STRING> l_scene_file_path;
			l_scene_file_path.allocate(0);
			l_scene_file_path.path.append(l_asset_server.get_asset_basepath().c_str());
			l_scene_file_path.path.append(".tmp/");
			l_scene_file_path.path.append(p_scene_asset_path);
			l_scene_file_path.path.append(".");
			l_scene_file_path.path.append(clock_currenttime_mics());
			l_scene_file.allocate(FileType::CONTENT, l_scene_file_path);
			l_scene_file.create_override();
			l_scene_file.append(StringSlice(l_scene_json.Memory));
			l_scene_file.free();
		}
		{
			File<FilePathMemoryLayout::STRING> l_scene_file;
			FilePath<FilePathMemoryLayout::STRING> l_scene_file_path;
			l_scene_file_path.allocate(0);
			l_scene_file_path.path.append(l_asset_server.get_asset_basepath().c_str());
			l_scene_file_path.path.append(p_scene_asset_path);
			l_scene_file.allocate(FileType::CONTENT, l_scene_file_path);
			l_scene_file.create_override();
			l_scene_file.append(StringSlice(l_scene_json.Memory));
			l_scene_file.free();
		}


		l_scene_json.free();
		l_dup_asset.free();
	}
};

struct EditorSceneEventHeader
{
	size_t Type;
};

struct EditorSceneEventMoveNode
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventMoveNode");

	SceneNodeToken scene_node;
	Math::vec3f old_localposition;
	Math::vec3f new_localposition;

	inline EditorSceneEventMoveNode(SceneNodeToken& p_scene_node, Math::vec3f& p_old_local, Math::vec3f& p_new_local)
	{
		this->scene_node = p_scene_node;
		this->old_localposition = p_old_local;
		this->new_localposition = p_new_local;
	};

	inline void _do(Scene* p_scene)
	{
		SceneKernel::set_localposition(this->scene_node, p_scene, this->new_localposition);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::set_localposition(this->scene_node, p_scene, this->old_localposition);
	};
};

struct EditorSceneEventRotateNode
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventRotateNode");

	SceneNodeToken scene_node;
	Math::quat old_localrotation;
	Math::quat new_localrotation;

	inline EditorSceneEventRotateNode(SceneNodeToken& p_scene_node, Math::quat& p_old_local, Math::quat& p_new_local)
	{
		this->scene_node = p_scene_node;
		this->old_localrotation = p_old_local;
		this->new_localrotation = p_new_local;
	};

	inline void _do(Scene* p_scene)
	{
		SceneKernel::set_localrotation(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->new_localrotation);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::set_localrotation(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->old_localrotation);
	};
};

struct EditorSceneEventCreateNode
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventCreateNode");

	SceneNodeToken created_node;
	SceneNodeToken parent;
	Math::Transform local_transform;

	inline EditorSceneEventCreateNode(const SceneNodeToken& p_parent, const Math::Transform& p_local_transform)
	{
		this->parent = p_parent;
		this->local_transform = p_local_transform;
	};

	inline void _do(Scene* p_scene)
	{
		this->created_node = SceneKernel::add_node(p_scene, this->parent, this->local_transform);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::remove_node(p_scene, this->created_node);
	};

};

struct EditorSceneEventRemoveNode
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventRemoveNode");

	SceneNodeToken erased_node;

	//For undo
	SceneAsset erased_node_as_sceneasset;
	com::Vector<SceneNodeToken> sceneassetnode_to_scenenode;
	SceneNodeToken parent;

	inline EditorSceneEventRemoveNode(const SceneNodeToken& p_erased_node)
	{
		this->erased_node = p_erased_node;
	};

	inline void _do(Scene* p_scene)
	{
		this->parent = SceneNodeToken(SceneKernel::resolve_node(p_scene, this->erased_node).node->parent.val);
		SceneSerializer2::SceneSingleNode_to_SceneAsset(*p_scene, this->erased_node, &this->erased_node_as_sceneasset, &this->sceneassetnode_to_scenenode, MainToolConstants::sceneNodeEditorFilter);
		//We wrap the this->erased_node to bypass the fact that token is resetted when freed
		SceneKernel::remove_node(p_scene, SceneNodeToken(this->erased_node.val));
	};

	inline void _undo(Scene* p_scene)
	{
		struct AddNode
		{
			com::Vector<SceneNodeToken>* sceneassetnode_to_scenenode;
			inline AddNode(com::Vector<SceneNodeToken>* p_sceneassetnode_to_scenenode)
			{
				this->sceneassetnode_to_scenenode = p_sceneassetnode_to_scenenode;
			};

			inline SceneNodeToken add_node(Scene* thiz, const SceneNodeToken& p_parent, const Math::Transform& p_initial_local_transform, const size_t p_nodeaddet_index)
			{
				SceneKernel::add_node_at_freenode(thiz, this->sceneassetnode_to_scenenode->operator[](p_nodeaddet_index), p_parent, p_initial_local_transform);
				return this->sceneassetnode_to_scenenode->operator[](p_nodeaddet_index);
			};
		};

		SceneKernel::feed_with_asset(p_scene, this->erased_node_as_sceneasset, this->parent, AddNode(&this->sceneassetnode_to_scenenode));

		this->erased_node_as_sceneasset.free();
		this->sceneassetnode_to_scenenode.free();
	};
};

struct EditorSceneEventDuplicateNode
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventDuplicateNode");

	SceneNodeToken created_node;
	SceneNodeToken duplicated_node;

	inline EditorSceneEventDuplicateNode(const SceneNodeToken& p_duplicated_node)
	{
		this->duplicated_node = p_duplicated_node;
	};

	inline void _do(Scene* p_scene)
	{
		// this->created_node = SceneKernel::duplicate_single_node(p_scene, this->duplicated_node);
		this->created_node = SceneKernel::duplicate_tree_node(p_scene, this->duplicated_node);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::remove_node(p_scene, this->created_node);
	};

};

struct EditorSceneEventAddComponent
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventAddComponent");

	SceneNodeComponentToken created_component;
	const SceneNodeComponent_TypeInfo* component_type;
	SceneNodeToken node;

	inline EditorSceneEventAddComponent(const SceneNodeComponent_TypeInfo* p_component_type, const SceneNodeToken& p_node)
	{
		this->component_type = p_component_type;
		this->node = p_node;
	};

	inline void _do(Scene* p_scene)
	{
		switch (this->component_type->id)
		{
		case MeshRenderer::Id:
		{
			MeshRenderer l_mesh_renderer = MeshRenderer();
			l_mesh_renderer.initialize_default();
			this->created_component = SceneKernel::add_component<MeshRenderer>(p_scene, this->node, l_mesh_renderer);
		}
		break;
		}

	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::remove_component(p_scene, this->node, *this->component_type);
	};
};

struct EditorSceneEventRemoveComponent
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventRemoveComponent");

	ComponentAsset component_asset;
	const SceneNodeComponent_TypeInfo* component_type;
	SceneNodeToken node;

	void* allocated_component_asset_object;

	inline EditorSceneEventRemoveComponent(const SceneNodeComponent_TypeInfo* p_component_type, const SceneNodeToken& p_node)
	{
		this->component_type = p_component_type;
		this->node = p_node;
	};

	struct LocalComponentAssetAllocator
	{
		void** allocated_component_object;
		inline LocalComponentAssetAllocator(void** p_allocated_component_object)
		{
			this->allocated_component_object = p_allocated_component_object;
		};

		template<class ElementType>
		inline ElementType* allocate()
		{
			(*allocated_component_object) = malloc(sizeof(ElementType));
			return (ElementType*)(*allocated_component_object);
		}
	};

	inline void _do(Scene* p_scene)
	{

		SceneNodeComponentHeader* l_component_header;
		SceneKernel::get_component(p_scene, this->node, *this->component_type, &l_component_header);
		LocalComponentAssetAllocator l_local_allocator = LocalComponentAssetAllocator(&this->allocated_component_asset_object);
		ComponentAssetSerializer::Component_to_ComponentAsset2(l_component_header, l_local_allocator);
		SceneKernel::remove_component(p_scene, this->node, *this->component_type);
	};

	inline void _undo(Scene* p_scene)
	{
		struct OnComponentDeserialized
		{
			Scene* scene;
			SceneNodeToken scene_node;
			inline OnComponentDeserialized(Scene* p_scene, SceneNodeToken& p_node)
			{
				this->scene = p_scene;
				this->scene_node = p_node;
			};

			inline void with_component(void* p_component, const SceneNodeComponent_TypeInfo& p_type)
			{
				SceneKernel::add_component(this->scene, this->scene_node, p_type, p_component);
			};
		};

		ComponentAssetSerializer::ComponentAsset_to_Component(this->component_type->id, this->allocated_component_asset_object, OnComponentDeserialized(p_scene, this->node));
		free(this->allocated_component_asset_object);
	};
};

struct EditorSceneEventSetParent
{
	inline static const size_t Type = Hash<ConstString>::hash("EditorSceneEventSetParent");

	SceneNodeToken node;
	SceneNodeToken parent;

	SceneNodeToken old_parent;

	inline EditorSceneEventSetParent(const SceneNodeToken& p_node, const SceneNodeToken& p_parent)
	{
		this->node = p_node;
		this->parent = p_parent;
	};

	inline void _do(Scene* p_scene)
	{
		this->old_parent = SceneNodeToken(SceneKernel::resolve_node(p_scene, this->node).node->parent.val);
		SceneKernel::add_child(p_scene, this->parent, this->node);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::add_child(p_scene, this->old_parent, this->node);
	};
};

struct EditorSceneEvent
{
	EditorSceneEventHeader header;
	char* object;

	template<class Event>
	inline void allocate(const Event& p_initial_value = Event())
	{
		this->header.Type = Event::Type;
		this->object = (char*)malloc(sizeof(Event));
		if (this->object)
		{
			memcpy(this->object, &p_initial_value, sizeof(Event));
		}
	};

	inline void free()
	{
		::free(this->object);
	};
};


struct EditorScene
{
	Scene* engine_scene;
	com::Vector<EditorSceneEvent> undo_events;
	String<> engine_scene_asset_path;


	// /!\ editor scene nodes are always allocated at the end of the scene node tree memory.
	//     this is to avoid weird behavior when we undo scene node deletion or creationg. To avoid that the newly placed editor node takes the slot of a potential undoable
	//     node.
	struct EditorSceneNodeAllocator
	{
		size_t scenenode_memory_constraint = 0;

		inline SceneNodeToken allocate_node(Scene* p_scene, SceneNodeToken p_parent)
		{
			SceneNodeToken l_node = SceneKernel::add_node_memoryposition_constrained(p_scene, p_parent, Math::Transform(), this->scenenode_memory_constraint);

			if (l_node.val >= this->scenenode_memory_constraint)
			{
				this->scenenode_memory_constraint = l_node.val;
			}

			SceneKernel::add_tag(p_scene, l_node, MainToolConstants::EditorNodeTag);
			return l_node;
		}

	} editor_scenenode_allocator;

	inline void allocate(Scene* p_engine_scene, StringSlice& p_engine_asset_path)
	{
		this->engine_scene = p_engine_scene;
		this->engine_scene_asset_path.allocate(p_engine_asset_path.size());
		this->engine_scene_asset_path.append(p_engine_asset_path);
	};

	inline void free()
	{
		for (size_t i = 0; i < this->undo_events.Size; i++)
		{
			this->undo_events[i].free();
		}
		this->undo_events.free();
		this->engine_scene = nullptr;

		this->engine_scene_asset_path.free();
	};

	inline void persist_current_scene(EngineHandle p_engine)
	{
		ScenePersister::persist(p_engine, this->engine_scene, this->engine_scene_asset_path);
	};

	inline void set_localposition(NTreeResolve<SceneNode>& p_scene_node, Math::vec3f& p_local_position)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventMoveNode(SceneNodeToken(p_scene_node.node->index.val), SceneKernel::get_localposition(p_scene_node), p_local_position));
		this->undo_events.push_back(l_event);

		((EditorSceneEventMoveNode*)l_event.object)->_do(this->engine_scene);
	};

	inline void set_localrotation(NTreeResolve<SceneNode>& p_scene_node, Math::quat& p_local_rotation)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventRotateNode(SceneNodeToken(p_scene_node.node->index.val), SceneKernel::get_localrotation(p_scene_node), p_local_rotation));
		this->undo_events.push_back(l_event);

		((EditorSceneEventRotateNode*)l_event.object)->_do(this->engine_scene);
	};

	inline void set_parent(SceneNodeToken p_node, SceneNodeToken p_parent)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventSetParent(p_node, p_parent));
		this->undo_events.push_back(l_event);

		((EditorSceneEventSetParent*)l_event.object)->_do(this->engine_scene);
	};

	inline SceneNodeToken create_node(const SceneNodeToken& p_parent, const Math::Transform& p_local_transform)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventCreateNode(p_parent, p_local_transform));
		this->undo_events.push_back(l_event);

		((EditorSceneEventCreateNode*)l_event.object)->_do(this->engine_scene);
		return ((EditorSceneEventCreateNode*)l_event.object)->created_node;
	};

	inline void remove_node(const SceneNodeToken& p_removed_node)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventRemoveNode(p_removed_node));
		this->undo_events.push_back(l_event);

		((EditorSceneEventRemoveNode*)l_event.object)->_do(this->engine_scene);
	};

	inline SceneNodeToken duplicate_node(const SceneNodeToken& p_duplicated_node)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventDuplicateNode(p_duplicated_node));
		this->undo_events.push_back(l_event);

		((EditorSceneEventDuplicateNode*)l_event.object)->_do(this->engine_scene);
		return ((EditorSceneEventDuplicateNode*)l_event.object)->created_node;
	};

	inline SceneNodeComponentToken add_component(const SceneNodeComponent_TypeInfo* p_component_type, const SceneNodeToken& p_node)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventAddComponent(p_component_type, p_node));
		this->undo_events.push_back(l_event);

		((EditorSceneEventAddComponent*)l_event.object)->_do(this->engine_scene);
		return ((EditorSceneEventAddComponent*)l_event.object)->created_component;
	};

	inline void remove_component(const SceneNodeComponent_TypeInfo* p_component_type, const SceneNodeToken& p_node)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventRemoveComponent(p_component_type, p_node));
		this->undo_events.push_back(l_event);

		((EditorSceneEventRemoveComponent*)l_event.object)->_do(this->engine_scene);
	};

	inline void _undo()
	{
		if (this->undo_events.Size > 0)
		{
			EditorSceneEvent& l_event = this->undo_events[this->undo_events.Size - 1];
			switch (l_event.header.Type)
			{
			case EditorSceneEventMoveNode::Type:
			{
				((EditorSceneEventMoveNode*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			case EditorSceneEventRotateNode::Type:
			{
				((EditorSceneEventRotateNode*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			case EditorSceneEventCreateNode::Type:
			{
				((EditorSceneEventCreateNode*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			case EditorSceneEventRemoveNode::Type:
			{
				((EditorSceneEventRemoveNode*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			case EditorSceneEventDuplicateNode::Type:
			{
				((EditorSceneEventDuplicateNode*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			case EditorSceneEventAddComponent::Type:
			{
				((EditorSceneEventAddComponent*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			case EditorSceneEventRemoveComponent::Type:
			{
				((EditorSceneEventRemoveComponent*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			case EditorSceneEventSetParent::Type:
			{
				((EditorSceneEventSetParent*)l_event.object)->_undo(this->engine_scene);
			}
			break;
			}

			l_event.free();
			this->undo_events.erase_at(this->undo_events.Size - 1, 1);
		}
	};
};




struct EngineRunningModule
{
	ExternalHooks engine_hooks;
	EngineHandle running_engine = nullptr;
	EditorScene editor_scene;
	bool frame_executed = false;

	inline void start()
	{
		this->engine_hooks.closure = this;
		this->engine_hooks.ext_update = EngineRunningModule::update;
		this->running_engine = engine_create("", this->engine_hooks);
		this->editor_scene.allocate(engine_scene(this->running_engine), StringSlice(""));
	};

	inline void load_scene(StringSlice& p_scene)
	{
		Scene tmp_scene = *engine_scene(this->running_engine);
		SceneKernel::free_scene(engine_scene(this->running_engine));
		SceneKernel::allocate_scene(engine_scene(this->running_engine), tmp_scene.component_added_callback, tmp_scene.component_removed_callback, tmp_scene.component_asset_push_callback);


		com::Vector<char> l_scene = engine_assetserver(this->running_engine).get_resource(Hash<StringSlice>::hash(p_scene));
		SceneAsset l_deserialized_scene = SceneSerializer2::Binary_to_SceneAsset(l_scene);
		{
			SceneKernel::feed_with_asset(engine_scene(this->running_engine), l_deserialized_scene);
		}

		l_scene.free();

		this->editor_scene.free();
		this->editor_scene.allocate(engine_scene(this->running_engine), p_scene);

	};

	inline void stop()
	{
		engine_destroy(this->running_engine);
		this->editor_scene.free();
	};

	inline void editor_update()
	{
		if (this->running_engine)
		{
			engine_poll_events(this->running_engine);
		}
	};

	inline void update()
	{
		this->frame_executed = false;
		if (this->running_engine)
		{
			if (!engine_should_close(this->running_engine))
			{
				Clock* l_clock = engine_clock(this->running_engine);
				size_t l_frame_before = l_clock->framecount;
				engine_singleframe(this->running_engine);
				if (l_clock->framecount != l_frame_before)
				{
					this->frame_executed = true;
				}
			}
			else
			{
				this->stop();
				this->running_engine = nullptr;
			}
		}
	};

	inline void update(float p_delta)
	{
		this->frame_executed = false;
		if (this->running_engine)
		{
			if (!engine_should_close(this->running_engine))
			{
				Clock* l_clock = engine_clock(this->running_engine);
				size_t l_frame_before = l_clock->framecount;
				engine_singleframe(this->running_engine, p_delta);
				if (l_clock->framecount != l_frame_before)
				{
					this->frame_executed = true;
				}
			}
			else
			{
				this->stop();
				this->running_engine = nullptr;
			}
		}

	};

	inline static void update(void* p_thiz, float p_delta)
	{
	};

private:

};

struct EngineRunner
{
	com::OptionalPool<EngineRunningModule> engines;

	inline void update()
	{
		for (size_t i = 0; i < engines.size(); i++)
		{
			if (engines[i].hasValue)
			{
				engines[i].value.update();
			}
		}
	}

	inline void free()
	{
		for (size_t i = 0; i < engines.size(); i++)
		{
			if (engines[i].hasValue)
			{
				engines[i].value.stop();
			}
		}
		engines.free();
	}

	inline bool frame_executed(com::TPoolToken<Optional<EngineRunningModule>> p_engine)
	{
		if (this->engines[p_engine].hasValue)
		{
			return this->engines[p_engine].value.frame_executed;
		}
		return false;
	};

	inline EngineRunningModule& get_enginemodule(com::TPoolToken<Optional<EngineRunningModule>>& p_engine_module)
	{
		return this->engines[p_engine_module].value;
	};

};






struct NodeMovement2
{
	enum class State
	{
		UNDEFINED = 0,
		POSITION_LOCAL = 1,
		ROTATION_LOCAL = 2,
		SCALE_LOCAL = 3

	} state = State::UNDEFINED;

	enum class Direction
	{
		UNDEFINED = 0,
		X = 1, Y = 2, Z = 3
	} direction = Direction::UNDEFINED;

	float movement_step = 1.0f;
	float rotation_step_deg = 5.0f;
	float scale_step = 0.1f;

	struct Gizmo
	{
		SceneNodeToken gizmo_scene_node;

		inline void create(SceneNodeToken p_parent, EditorScene& p_editor_scene)
		{
			if (this->gizmo_scene_node.val == -1)
			{
				this->gizmo_scene_node = p_editor_scene.editor_scenenode_allocator.allocate_node(p_editor_scene.engine_scene, p_parent);
				printf("Allocated gizmo : ");
				printf("%lld", this->gizmo_scene_node.val);
				printf("\n");

				MeshRenderer l_ms;
				l_ms.initialize(StringSlice("materials/editor_gizmo.json"), StringSlice("models/arrow.obj"));
				SceneKernel::add_component<MeshRenderer>(p_editor_scene.engine_scene, this->gizmo_scene_node, l_ms);
			}

		}

		inline void set(NodeMovement2::Direction p_nodemovmeent_direction, EngineHandle p_engine)
		{
			switch (p_nodemovmeent_direction)
			{
			case Direction::X:
			{
				this->set_color(Math::vec4f(1.0f, 0.0f, 0.0f, 1.0f), p_engine);
			}
			break;
			case Direction::Y:
			{
				this->set_color(Math::vec4f(0.0f, 1.0f, 0.0f, 1.0f), p_engine);
			}
			break;
			case Direction::Z:
			{
				this->set_color(Math::vec4f(0.0f, 0.0f, 1.0f, 1.0f), p_engine);
			}
			break;
			}
		}

		inline void free(EngineHandle p_engine)
		{
			if (this->gizmo_scene_node.val != -1)
			{
				SceneKernel::remove_node(engine_scene(p_engine), this->gizmo_scene_node);
				this->gizmo_scene_node.reset();
			}
		};

		inline void set_color(Math::vec4f& p_color, EngineHandle p_engine)
		{
			MeshRenderer& l_ms_ptr = SceneKernel::get_component<MeshRenderer>(engine_scene(p_engine), this->gizmo_scene_node);
			engine_render_middleware(p_engine)->get_renderable_object(l_ms_ptr).material.set_uniform_parameter(engine_render_middleware(p_engine)->render, 0, GPtr::fromType<Math::vec4f>(&p_color));
		};
	} gizmo;

	inline void perform_node_movement(SceneNodeToken& p_node, EngineRunningModule& p_engine_running_module)
	{
		InputHandle l_input = engine_input(p_engine_running_module.running_engine);

		if (l_input.get_state(InputKey::InputKey_T, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
		{
			this->state = State::POSITION_LOCAL;
			printf("NodeMovement : Position mode enabled\n");
		}
		else if (l_input.get_state(InputKey::InputKey_R, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
		{
			this->state = State::ROTATION_LOCAL;
			printf("NodeMovement : Rotation mode enabled\n");
		}
		else if (l_input.get_state(InputKey::InputKey_S, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
		{
			this->state = State::SCALE_LOCAL;
			printf("NodeMovement : Scale mode enabled\n");
		}

		if (l_input.get_state(InputKey::InputKey_X, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
		{
			this->set_direction(Direction::X, p_engine_running_module);
			printf("NodeMovement : X axis \n");
		}
		else if (l_input.get_state(InputKey::InputKey_Y, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
		{
			this->set_direction(Direction::Y, p_engine_running_module);
			printf("NodeMovement : Y axis \n");
		}
		else if (l_input.get_state(InputKey::InputKey_Z, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
		{
			this->set_direction(Direction::Z, p_engine_running_module);
			printf("NodeMovement : Z axis \n");
		}


		if (this->direction != Direction::UNDEFINED &&
			this->state != State::UNDEFINED)
		{
			Scene* p_scene = p_engine_running_module.editor_scene.engine_scene;

			if (SceneKernel::check_scenetoken_validity(p_scene, p_node)) 
			{ 
			

				NTreeResolve<SceneNode> l_node = SceneKernel::resolve_node(p_scene, p_node);

				switch (this->state)
				{
				case State::POSITION_LOCAL:
				{

					float l_value = 0.0f;

					if (l_input.get_state(InputKey::InputKey_LEFT_CONTROL, KeyState::KeyStateFlag_PRESSED))
					{
						if (l_input.get_state(InputKey::InputKey_UP, KeyState::KeyStateFlag_PRESSED))
						{
							l_value = this->movement_step;
						}
						else if (l_input.get_state(InputKey::InputKey_DOWN, KeyState::KeyStateFlag_PRESSED))
						{
							l_value = -this->movement_step;
						}
					}
					else
					{
						if (l_input.get_state(InputKey::InputKey_UP, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
						{
							l_value = this->movement_step;
						}
						else if (l_input.get_state(InputKey::InputKey_DOWN, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
						{
							l_value = -this->movement_step;
						}

					}


					Math::vec4f l_delta = Math::vec4f(0.0f, 0.0f, 0.0f, 0.0f);

					switch (this->direction)
					{
					case Direction::X:
					{
						l_delta.x = l_value;
					}
					break;
					case Direction::Y:
					{
						l_delta.y = l_value;
					}
					break;
					case Direction::Z:
					{
						l_delta.z = l_value;
					}
					break;
					}

					if (!Math::EqualsVec(l_delta, Math::vec4f(0.0f, 0.0f, 0.0f, 0.0f)))
					{
						l_delta.Vec3 = Math::rotate(l_delta.Vec3, SceneKernel::get_localrotation(l_node));
						p_engine_running_module.editor_scene.set_localposition(l_node, SceneKernel::get_localposition(l_node) + l_delta.Vec3);
					}
				}
				break;
				case State::ROTATION_LOCAL:
				{

					float l_value = 0.0f;

					if (l_input.get_state(InputKey::InputKey_LEFT_CONTROL, KeyState::KeyStateFlag_PRESSED))
					{
						if (l_input.get_state(InputKey::InputKey_UP, KeyState::KeyStateFlag_PRESSED))
						{
							l_value = this->rotation_step_deg * DEG_TO_RAD;
						}
						else if (l_input.get_state(InputKey::InputKey_DOWN, KeyState::KeyStateFlag_PRESSED))
						{
							l_value = -this->rotation_step_deg * DEG_TO_RAD;
						}
					}
					else
					{
						if (l_input.get_state(InputKey::InputKey_UP, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
						{
							l_value = this->rotation_step_deg * DEG_TO_RAD;
						}
						else if (l_input.get_state(InputKey::InputKey_DOWN, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
						{
							l_value = -this->rotation_step_deg * DEG_TO_RAD;
						}
					}



					Math::vec3f l_axis = Math::vec3f(0.0f, 0.0f, 0.0f);
					switch (this->direction)
					{
					case Direction::X:
					{
						l_axis.x = 1.0f;
					}
					break;
					case Direction::Y:
					{
						l_axis.y = 1.0f;
					}
					break;
					case Direction::Z:
					{
						l_axis.z = 1.0f;
					}
					break;
					}

					Math::quat l_delta = Math::rotateAround(l_axis, l_value);

					if (!Math::Equals(l_delta, Math::QuatConst::IDENTITY))
					{
						p_engine_running_module.editor_scene.set_localrotation(l_node, mul(SceneKernel::get_localrotation(l_node), Math::rotateAround(l_axis, l_value)));
					}
				}
				break;
				}


				this->set_gizmo_position(l_node, p_scene);

			}	
		}
	};

	inline void set_current_value(SceneNodeToken& p_node, EngineRunningModule& p_engine_running_module, const float p_value)
	{
		if (this->direction != Direction::UNDEFINED &&
			this->state != State::UNDEFINED)
		{
			Scene* p_scene = p_engine_running_module.editor_scene.engine_scene;
			NTreeResolve<SceneNode> l_node = SceneKernel::resolve_node(p_scene, p_node);

			switch (this->state)
			{
			case State::POSITION_LOCAL:
			{

				Math::vec3f l_new_local_position = SceneKernel::get_localposition(l_node);

				switch (this->direction)
				{
				case Direction::X:
				{
					l_new_local_position.x = p_value;
				}
				break;
				case Direction::Y:
				{
					l_new_local_position.y = p_value;
				}
				break;
				case Direction::Z:
				{
					l_new_local_position.z = p_value;
				}
				break;
				}

				p_engine_running_module.editor_scene.set_localposition(l_node, l_new_local_position);
			}
			break;
			case State::ROTATION_LOCAL:
			{
				Math::quat l_initial_rotation_quat = SceneKernel::get_localrotation(p_node, p_scene);
				Math::quat l_final_localrotation;

				switch (this->direction)
				{
				case Direction::X:
				{
					Math::vec3f l_euler = Math::eulerAngle<float>(l_initial_rotation_quat);
					l_euler.x = p_value;
					l_final_localrotation = Math::fromEulerAngle(l_euler);
				}
				break;
				case Direction::Y:
				{
					Math::vec3f l_euler = Math::eulerAngle<float>(l_initial_rotation_quat);
					l_euler.y = p_value;
					l_final_localrotation = Math::fromEulerAngle(l_euler);
				}
				break;
				case Direction::Z:
				{
					Math::vec3f l_euler = Math::eulerAngle<float>(l_initial_rotation_quat);
					l_euler.z = p_value;
					l_final_localrotation = Math::fromEulerAngle(l_euler);
				}
				break;
				}

				p_engine_running_module.editor_scene.set_localrotation(l_node, l_final_localrotation);
			}
			break;
			}


			this->set_gizmo_position(l_node, p_scene);

		}
	}

	inline void set_gizmo_position(NTreeResolve<SceneNode>& l_node, Scene* p_scene)
	{
		SceneKernel::set_worldposition(this->gizmo.gizmo_scene_node, p_scene, SceneKernel::get_worldposition(l_node, p_scene));
		{
			Math::quat l_local_rotation = Math::QuatConst::IDENTITY;


			switch (this->direction)
			{
			case Direction::X:
			{
				l_local_rotation = Math::rotateAround(
					Math::VecConst<float>::UP,
					90.0f * DEG_TO_RAD
				);
			}
			break;
			case Direction::Y:
			{
				l_local_rotation = Math::rotateAround(
					Math::VecConst<float>::RIGHT,
					-90.0f * DEG_TO_RAD
				);
			}
			break;
			}

			SceneKernel::set_worldrotation(this->gizmo.gizmo_scene_node, p_scene,
				mul(
					SceneKernel::get_worldrotation(l_node, p_scene),
					l_local_rotation
				)
			);


		}

	}

	inline void exit(EngineHandle p_engine)
	{
		this->state = State::UNDEFINED;
		this->direction = Direction::UNDEFINED;
		this->gizmo.free(p_engine);
	}

private:
	inline void set_direction(Direction p_new_direction, EngineRunningModule& p_engine_running_module)
	{
		if (this->direction != p_new_direction)
		{
			this->direction = p_new_direction;
			this->on_direction_changed(this->direction, p_engine_running_module);
		}
	};

	inline void on_direction_changed(Direction p_new_direction, EngineRunningModule& p_engine_running_module)
	{
		if (p_new_direction != Direction::UNDEFINED)
		{
			this->gizmo.create(SceneNodeToken(0), p_engine_running_module.editor_scene);
			this->gizmo.set(p_new_direction, p_engine_running_module.running_engine);
		}
	};
};

struct SceneNodeSelection
{
	struct SelectedNodeRenderer
	{
		SceneNodeToken node;
		size_t original_material;

		inline void set_original_meshrenderer(Scene* p_scene, RenderMiddlewareHandle p_render_middleware)
		{
			p_render_middleware->set_material(SceneKernel::get_component<MeshRenderer>(p_scene, this->node), this->original_material);
		};
	};

	SceneNodeToken root_selected_node;
	com::Vector<SelectedNodeRenderer> selected_nodes_renderer;
	EngineHandle selected_node_engine;

	enum class SelectedNodeState
	{
		NODE_NOT_SELECTED = 0,
		NODE_SELECTED = 1
	};

	inline SelectedNodeState set_selected_node(EngineHandle p_engine, size_t p_node)
	{
		if (this->root_selected_node.val == p_node)
		{
			return this->calculate_selectednode_state();
		}

		if (p_node == -1 || SceneKernel::check_scenetoken_validity(engine_scene(p_engine), SceneNodeToken(p_node)))
		{
			EngineHandle l_old_engine = this->selected_node_engine;
			this->selected_node_engine = p_engine;
			SceneNodeToken l_old = this->root_selected_node;
			this->root_selected_node = SceneNodeToken(p_node);
			this->on_root_selected_node_changed(l_old, this->root_selected_node, l_old_engine, this->selected_node_engine);
		}
		else
		{
			return this->set_selected_node(p_engine, -1);
		}

		return this->calculate_selectednode_state();
	};

	inline bool get_selected_node(SceneNodeToken* out_selected_node)
	{
		if (this->root_selected_node.val != -1)
		{
			*out_selected_node = this->root_selected_node;
			return true;
		}
		return false;
	};

	inline void print_selected_node(EngineHandle p_engine)
	{
		if (this->root_selected_node.val != -1)
		{
			Serialization::JSON::Deserializer l_deserializer;
			l_deserializer.allocate();
			l_deserializer.start();
			l_deserializer.push_field("parent", SceneKernel::resolve_node(engine_scene(p_engine), this->root_selected_node).node->parent.val);
			l_deserializer.start_object("local_position");
			JSONSerializer<Math::vec3f>::serialize(l_deserializer, SceneKernel::get_localposition(this->root_selected_node, engine_scene(p_engine)));
			l_deserializer.end_object();
			l_deserializer.start_object("local_rotation");
			JSONSerializer<Math::quat>::serialize(l_deserializer, SceneKernel::get_localrotation(this->root_selected_node, engine_scene(p_engine)));
			l_deserializer.end_object();
			l_deserializer.start_object("local_scale");
			JSONSerializer<Math::vec3f>::serialize(l_deserializer, SceneKernel::get_localscale(this->root_selected_node, engine_scene(p_engine)));
			l_deserializer.end_object();
			l_deserializer.end();

			printf(l_deserializer.output.Memory.Memory);
			printf("\n");
			l_deserializer.free();

			printf("Components : ");

			com::Vector<SceneNodeComponentToken>& l_components = SceneKernel::get_components(engine_scene(p_engine), this->root_selected_node);
			for (size_t i = 0; i < l_components.Size; i++)
			{
				SceneNodeComponentHeader* l_header = SceneKernel::resolve_component(engine_scene(p_engine), l_components[i]);
				const char* l_component_name;
				if (SceneComponentUtils::get_name_from_id(l_header->type->id, &l_component_name))
				{
					printf(l_component_name);
					printf(",");
				}
			}
			printf("\n");
		}
	};

	inline SelectedNodeState calculate_selectednode_state()
	{
		if (this->root_selected_node.val == -1)
		{
			return SelectedNodeState::NODE_NOT_SELECTED;
		}
		else
		{
			return SelectedNodeState::NODE_SELECTED;
		}
	};

	inline void on_root_selected_node_changed(SceneNodeToken p_old, SceneNodeToken p_new, EngineHandle p_old_engine, EngineHandle p_new_engine)
	{
		for (size_t i = 0; i < this->selected_nodes_renderer.Size; i++)
		{
			Scene* l_scene = engine_scene(p_old_engine);
			if (SceneKernel::check_scenetoken_validity(l_scene, this->selected_nodes_renderer[i].node))
			{
				this->selected_nodes_renderer[i].set_original_meshrenderer(l_scene, engine_render_middleware(p_old_engine));
			}
		}
		this->selected_nodes_renderer.clear();

		if (p_new.val != -1)
		{
			struct SceneForeach2 : public SceneKernel::SceneNodeForeach<MainToolConstants::SceneNodeEditorFilter>
			{
				com::Vector<SelectedNodeRenderer>* out_selected_node_renderers;
				Scene* scene = nullptr;
				RenderMiddleware* render_middleware;

				inline SceneForeach2(com::Vector<SelectedNodeRenderer>* p_selected_node_renderers, Scene* p_scene, RenderMiddleware* p_render_middleware, MainToolConstants::SceneNodeEditorFilter& p_filter) :
					SceneKernel::SceneNodeForeach<MainToolConstants::SceneNodeEditorFilter>(p_filter) {
					this->out_selected_node_renderers = p_selected_node_renderers;
					this->scene = p_scene;
					this->render_middleware = p_render_middleware;
				}


				inline void foreach_internal(NTreeResolve<SceneNode>& p_node)
				{
					MeshRenderer* l_mesh_renderer;
					if (SceneKernel::get_component<MeshRenderer>(this->scene, SceneNodeToken(p_node.node->index.val), &l_mesh_renderer))
					{
						SelectedNodeRenderer l_selected_node_renderer;
						l_selected_node_renderer.node = SceneNodeToken(p_node.node->index.val);
						l_selected_node_renderer.original_material = l_mesh_renderer->material.key;
						this->render_middleware->set_material(*l_mesh_renderer, Hash<StringSlice>::hash(StringSlice("materials/editor_selected.json")));
						this->out_selected_node_renderers->push_back(l_selected_node_renderer);
					};
				};
			};

			SceneKernel::traverse(engine_scene(p_new_engine), this->root_selected_node,
				SceneForeach2(&this->selected_nodes_renderer, engine_scene(p_new_engine), engine_render_middleware(p_new_engine), MainToolConstants::sceneNodeEditorFilter));
		}

	};

	inline void clear()
	{
		this->set_selected_node(this->selected_node_engine, -1);
	}
};

struct ToolState2
{
	EngineRunner engine_runner;

	struct SelectedEngine
	{
		com::TPoolToken<Optional<EngineRunningModule>> token;

		inline bool is_valid(EngineRunner& p_engine_runner) {
			if (this->token.val == -1) { return false; }
			return p_engine_runner.engines[this->token].hasValue;
		};
	} selected_engine;

	SceneNodeSelection selected_node;
	SceneNodeSelection::SelectedNodeState selected_node_state = SceneNodeSelection::SelectedNodeState::NODE_NOT_SELECTED;

	NodeMovement2 node_movement;

	inline void open_engine_with_scene(StringSlice& p_scene_path)
	{
		EngineRunningModule l_engine;
		l_engine.start();
		l_engine.load_scene(p_scene_path);
		this->set_selected_engine(this->engine_runner.engines.alloc_element(l_engine));
	};

	inline void set_selected_engine(const com::TPoolToken<Optional<EngineRunningModule>> p_engine)
	{
		com::TPoolToken<Optional<EngineRunningModule>> l_old_engine = this->selected_engine.token;
		this->selected_engine.token = p_engine;
		this->on_engine_changed(l_old_engine, p_engine);
	};

	inline void on_engine_changed(const com::TPoolToken<Optional<EngineRunningModule>> p_old, const com::TPoolToken<Optional<EngineRunningModule>> p_new)
	{
		this->selected_node.clear();
	};

	inline SceneNodeToken create_node(const size_t p_parent_index, const Math::Transform& p_local_transform)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			if (SceneKernel::check_scenetoken_validity(this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.engine_scene, SceneNodeToken(p_parent_index)))
			{
				SceneNodeToken l_token = this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.create_node(SceneNodeToken(p_parent_index), p_local_transform);
				return l_token;
			}
		}

		return SceneNodeToken(-1);
	};

	inline void remove_node()
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				this->set_selected_node(-1); //This is to remove all data that is attached to the node components by the editor
				this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.remove_node(l_selected_node);
			}
		}
	};

	inline SceneNodeToken duplicate_node()
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeToken l_selected_node = this->selected_node.root_selected_node;
			if (SceneKernel::check_scenetoken_validity(this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.engine_scene, l_selected_node))
			{
				this->set_selected_node(-1); //This is to remove all data that is attached to the node components by the editor
				SceneNodeToken l_created_node = this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.duplicate_node(l_selected_node);
				this->set_selected_node(l_created_node.val);
				return l_created_node;
			}
		}
		return SceneNodeToken();
	};

	inline void set_selected_node(const size_t p_node_index)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeSelection::SelectedNodeState l_selectednode_state = this->selected_node.set_selected_node(this->engine_runner.engines[this->selected_engine.token].value.running_engine, p_node_index);
			if (this->selected_node_state != l_selectednode_state)
			{
				SceneNodeSelection::SelectedNodeState l_old = this->selected_node_state;
				this->selected_node_state = l_selectednode_state;
				this->on_selectednodestate_changed(l_old, this->selected_node_state);
			}
		}
	};

	inline void set_parent(const size_t p_parent)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.set_parent(l_selected_node, SceneNodeToken(p_parent));
			}
		}
	};

	inline void add_component(StringSlice& p_component_name)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				const SceneNodeComponent_TypeInfo* l_component_type;
				if (SceneComponentUtils::get_type_from_name(p_component_name, &l_component_type))
				{
					this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.add_component(l_component_type, l_selected_node);
				}
			}
		}
	};

	inline void remove_component(StringSlice& p_component_name)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				const SceneNodeComponent_TypeInfo* l_component_type;
				if (SceneComponentUtils::get_type_from_name(p_component_name, &l_component_type))
				{
					this->set_selected_node(-1);
					this->engine_runner.get_enginemodule(this->selected_engine.token).editor_scene.remove_component(l_component_type, l_selected_node);
				}
			}
		}
	};

	inline void print_selected_node()
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			this->selected_node.print_selected_node(this->engine_runner.engines[this->selected_engine.token].value.running_engine);
		}
	};

	inline void set_current_value(float p_value)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				this->node_movement.set_current_value(l_selected_node, this->engine_runner.engines[this->selected_engine.token].value, p_value);
			}
		}
	};

	inline void set_movement_step(float p_step)
	{
		this->node_movement.movement_step = p_step;
	};

	inline void set_rotation_step_deg(float p_step)
	{
		this->node_movement.rotation_step_deg = p_step;
	};

	inline void on_selectednodestate_changed(SceneNodeSelection::SelectedNodeState p_old, SceneNodeSelection::SelectedNodeState p_new)
	{
		switch (p_new)
		{
		case SceneNodeSelection::SelectedNodeState::NODE_NOT_SELECTED:
		{
			this->node_movement.exit(this->engine_runner.engines[this->selected_engine.token].value.running_engine);
		}
		break;
		}
	};

	inline void persist_currentscene()
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			this->set_selected_node(-1);
			this->engine_runner.engines[this->selected_engine.token].value.editor_scene.persist_current_scene(this->engine_runner.engines[this->selected_engine.token].value.running_engine);
		}
	};

	inline void meshrenderer_set_material(StringSlice& p_material_path)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			EngineRunningModule& l_engine = this->engine_runner.get_enginemodule(this->selected_engine.token);
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				this->set_selected_node(-1);
				MeshRenderer& l_mesh_renderer = SceneKernel::get_component<MeshRenderer>(l_engine.editor_scene.engine_scene, l_selected_node);
				engine_render_middleware(l_engine.running_engine)->set_material(l_mesh_renderer, Hash<StringSlice>::hash(p_material_path));
			}
		}
	};

	inline void meshrenderer_set_mesh(StringSlice& p_material_path)
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			EngineRunningModule& l_engine = this->engine_runner.get_enginemodule(this->selected_engine.token);
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				MeshRenderer& l_mesh_renderer = SceneKernel::get_component<MeshRenderer>(l_engine.editor_scene.engine_scene, l_selected_node);
				engine_render_middleware(l_engine.running_engine)->set_mesh(l_mesh_renderer, Hash<StringSlice>::hash(p_material_path));
			}
		}
	};

	inline void udpate()
	{
		if (this->selected_engine.is_valid(this->engine_runner))
		{
			SceneNodeToken l_selected_node;
			if (this->selected_node.get_selected_node(&l_selected_node))
			{
				this->node_movement.perform_node_movement(l_selected_node, this->engine_runner.engines[this->selected_engine.token].value);
			}

			EngineHandle l_engine = this->engine_runner.engines[this->selected_engine.token].value.running_engine;

			if (engine_input(l_engine).get_state(InputKey::InputKey_LEFT_CONTROL, KeyState::KeyStateFlag_PRESSED))
			{
				if (engine_input(l_engine).get_state(InputKey::InputKey_P, KeyState::KeyStateFlag_PRESSED))
				{
					this->engine_runner.engines[this->selected_engine.token].value.editor_scene._undo();
				}
			}
			else if (engine_input(l_engine).get_state(InputKey::InputKey_P, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->engine_runner.engines[this->selected_engine.token].value.editor_scene._undo();
			}
		}
	}
};


struct CommandHandler2
{
	inline static void handle_command(ToolState2& p_tool_state, String<>& p_command)
	{
		com::Vector<StringSlice> l_commands = p_command.split(" ");
		size_t l_depth = 0;

		if (l_commands[l_depth].equals(StringSlice("ns")))
		{
			l_depth++;
			size_t l_selected_node = FromString<size_t>::from_str(StringSlice(l_commands[l_depth]));
			p_tool_state.set_selected_node(l_selected_node);
		}
		else if (l_commands[l_depth].equals(StringSlice("np")))
		{
			l_depth++;
			p_tool_state.print_selected_node();
		}
		else if (l_commands[l_depth].equals(StringSlice("mts")))
		{
			l_depth++;
			float l_step = FromString<float>::from_str(StringSlice(l_commands[l_depth]));
			p_tool_state.set_movement_step(l_step);
		}
		else if (l_commands[l_depth].equals(StringSlice("mv")))
		{
			l_depth++;
			float l_value = FromString<float>::from_str(StringSlice(l_commands[l_depth]));
			p_tool_state.set_current_value(l_value);
		}
		else if (l_commands[l_depth].equals(StringSlice("mrs")))
		{
			l_depth++;
			float l_step = FromString<float>::from_str(StringSlice(l_commands[l_depth]));
			p_tool_state.set_rotation_step_deg(l_step);
		}
		else if (l_commands[l_depth].equals(StringSlice("create")))
		{
			l_depth++;

			SceneNodeToken l_created_node = p_tool_state.create_node(0, Math::Transform());

			String<> l_str; l_str.allocate(0);
			l_str.append(l_created_node.val);
			printf("Created node : ");
			printf(l_str.c_str());
			printf("\n");
			l_str.free();
		}
		else if (l_commands[l_depth].equals(StringSlice("parent")))
		{
			l_depth++;
			size_t l_parent = FromString<size_t>::from_str(StringSlice(l_commands[l_depth]));
			p_tool_state.set_parent(l_parent);
		}
		else if (l_commands[l_depth].equals(StringSlice("delete")))
		{
			l_depth++;
			p_tool_state.remove_node();
		}
		else if (l_commands[l_depth].equals(StringSlice("duplicate")))
		{
			l_depth++;
			SceneNodeToken l_created_node = p_tool_state.duplicate_node();
			if (l_created_node.val != -1)
			{
				String<> l_str; l_str.allocate(0);
				l_str.append(l_created_node.val);
				printf("Created node : ");
				printf(l_str.c_str());
				printf("\n");
				l_str.free();
			}
		}
		else if (l_commands[l_depth].equals("addc"))
		{
			l_depth++;
			p_tool_state.add_component(l_commands[l_depth]);
		}
		else if (l_commands[l_depth].equals("delc"))
		{
			l_depth++;
			p_tool_state.remove_component(l_commands[l_depth]);
		}
		else if (l_commands[l_depth].equals("MeshRenderer"))
		{
			l_depth++;
			if (l_commands[l_depth].equals("mat"))
			{
				l_depth++;
				p_tool_state.meshrenderer_set_material(l_commands[l_depth]);
			}
			else if (l_commands[l_depth].equals("mesh"))
			{
				l_depth++;
				p_tool_state.meshrenderer_set_mesh(l_commands[l_depth]);
			}
		}
		else if (l_commands[l_depth].equals(StringSlice("s")))
		{
			l_depth++;
			p_tool_state.persist_currentscene();
		}

		l_commands.free();
	}

};

struct CustomCommandLine
{
#define CustomCommandLineCheckRaw(inputEnum, ch) \
else if (p_input.get_state(InputKey::InputKey_##inputEnum, KeyState::KeyStateFlag_PRESSED_THIS_FRAME)) \
{ \
	l_char = #ch; \
}

#define CustomCommandLineCheckChar(cM, cm) \
else if (p_input.get_state(InputKey::InputKey_##cM, KeyState::KeyStateFlag_PRESSED_THIS_FRAME)) \
{ \
	if (maj_holded) { l_char = #cM; } \
	else { l_char = #cm; } \
}

#define CustomCommandLineCheckNumber(cn, cnl) \
else if (p_input.get_state(InputKey::InputKey_##cnl, KeyState::KeyStateFlag_PRESSED_THIS_FRAME)) \
{ \
	l_char = #cn; \
}

	String<> buffer;
	com::Vector<String<>> last_entered_buffer;
	bool maj_holded = false;

	enum class State
	{
		WAITING = 0,
		LISTENING = 1,
		COMPLETED = 2
	} state = State::WAITING;

	inline void allocate()
	{
		this->buffer.allocate(0);
		this->last_entered_buffer.allocate(0);
	};

	inline void free()
	{
		this->buffer.free();
	};

	inline State update(InputHandle p_input)
	{
		if (this->state == State::WAITING)
		{
			if (p_input.get_state(InputKey::InputKey_SPACE, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->set_state(State::LISTENING);
				return this->state;
			}
		}
		else if (this->state == State::LISTENING)
		{
			if (p_input.get_state(InputKey::InputKey_ESCAPE, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->set_state(State::WAITING);
				return this->state;
			}
			else if (p_input.get_state(InputKey::InputKey_UP, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->peek_last_entered_buffer();
				printf(this->buffer.Memory.Memory);
				printf("\n");
				return this->state;
			}

			char* l_char = "\0";

			if (p_input.get_state(InputKey::InputKey_ENTER, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->set_state(State::COMPLETED);
			}

			this->maj_holded = p_input.get_state(InputKey::InputKey_LEFT_SHIFT, KeyState::KeyStateFlag_PRESSED);


			if (p_input.get_state(InputKey::InputKey_BACKSPACE, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				if (this->buffer.Memory.Size > 1)
				{
					this->buffer.remove(this->buffer.Memory.Size - 2, this->buffer.Memory.Size - 1);
				}
				printf(this->buffer.Memory.Memory);
				printf("\n");
			}
			else if (p_input.get_state(InputKey::InputKey_SPACE, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				l_char = " ";
			}
			CustomCommandLineCheckChar(A, a)
				CustomCommandLineCheckChar(B, b)
				CustomCommandLineCheckChar(C, c)
				CustomCommandLineCheckChar(D, d)
				CustomCommandLineCheckChar(E, e)
				CustomCommandLineCheckChar(F, f)
				CustomCommandLineCheckChar(G, g)
				CustomCommandLineCheckChar(H, h)
				CustomCommandLineCheckChar(I, i)
				CustomCommandLineCheckChar(J, j)
				CustomCommandLineCheckChar(K, k)
				CustomCommandLineCheckChar(L, l)
				CustomCommandLineCheckChar(M, m)
				CustomCommandLineCheckChar(N, n)
				CustomCommandLineCheckChar(O, o)
				CustomCommandLineCheckChar(P, p)
				CustomCommandLineCheckChar(Q, q)
				CustomCommandLineCheckChar(R, r)
				CustomCommandLineCheckChar(S, s)
				CustomCommandLineCheckChar(T, t)
				CustomCommandLineCheckChar(U, u)
				CustomCommandLineCheckChar(V, v)
				CustomCommandLineCheckChar(W, w)
				CustomCommandLineCheckChar(X, x)
				CustomCommandLineCheckChar(Y, y)
				CustomCommandLineCheckChar(Z, z)
				CustomCommandLineCheckNumber(0, ZERO)
				CustomCommandLineCheckNumber(1, ONE)
				CustomCommandLineCheckNumber(2, TWO)
				CustomCommandLineCheckNumber(3, THREE)
				CustomCommandLineCheckNumber(4, FOUR)
				CustomCommandLineCheckNumber(5, FIVE)
				CustomCommandLineCheckNumber(6, SIX)
				CustomCommandLineCheckNumber(7, SEVEN)
				CustomCommandLineCheckNumber(8, EIGHT)
				CustomCommandLineCheckNumber(9, NINE)
			else if (p_input.get_state(InputKey::InputKey_MINUS, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				if (this->maj_holded)
				{
					l_char = "_";
				}
				else
				{
					l_char = "-";
				}
			}
			CustomCommandLineCheckRaw(MINUS, -)
				CustomCommandLineCheckRaw(PERIOD, .)
				CustomCommandLineCheckRaw(SLASH, / )
				if (l_char != "\0")
				{
					this->buffer.append(StringSlice(l_char));
					printf(this->buffer.Memory.Memory);
					printf("\n");
				}
		}


		return this->state;
	};

	inline void set_state(State p_new_state)
	{
		if (this->state != p_new_state)
		{
			State l_old = this->state;
			this->state = p_new_state;
			this->on_state_changed(l_old, p_new_state);
		}
	}

	inline void on_state_changed(State p_old, State p_new)
	{
		switch (p_new)
		{
		case State::LISTENING:
		{
			this->buffer.clear();
		}
		break;
		case State::COMPLETED:
		{
			this->last_entered_buffer.push_back(this->buffer.clone());
		}
		break;
		}
	}

	inline void peek_last_entered_buffer()
	{
		if (this->last_entered_buffer.Size > 0)
		{
			this->buffer.clear();
			this->buffer.append(this->last_entered_buffer[this->last_entered_buffer.Size - 1]);
			this->last_entered_buffer[this->last_entered_buffer.Size - 1].free();
			this->last_entered_buffer.erase_at(this->last_entered_buffer.Size - 1, 1);
		}
	}
};


int main()
{
	while (true)
	{
		printf("Scene : ");
		String<> l_scene_path;
		l_scene_path.allocate(255);
		l_scene_path.Memory.Size = l_scene_path.Memory.Capacity;
		fgets(l_scene_path.Memory.Memory, (int)l_scene_path.Memory.capacity_in_bytes() - 1, stdin);
		l_scene_path.Memory.Size = strlen(l_scene_path.Memory.Memory) + 1;
		l_scene_path.remove_chars('\n');

		ToolState2 l_tool_state;
		CustomCommandLine l_custom_command_line;
		l_custom_command_line.allocate();

		l_tool_state.open_engine_with_scene(l_scene_path.toSlice());

		//while at least one engine
		while (l_tool_state.selected_engine.is_valid(l_tool_state.engine_runner))
		{
			if (l_tool_state.engine_runner.frame_executed(l_tool_state.selected_engine.token))
			{
				if (l_custom_command_line.update(engine_input(l_tool_state.engine_runner.engines[l_tool_state.selected_engine.token].value.running_engine)) == CustomCommandLine::State::COMPLETED)
				{
					CommandHandler2::handle_command(l_tool_state, l_custom_command_line.buffer);
					l_custom_command_line.set_state(CustomCommandLine::State::WAITING);
				}

				if (l_custom_command_line.state == CustomCommandLine::State::WAITING)
				{
					l_tool_state.udpate();
				}
			}

			l_tool_state.engine_runner.update();
		}

		l_custom_command_line.free();
	}
	return 0;
}
