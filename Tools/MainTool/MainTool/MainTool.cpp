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
#include "Middleware/RenderMiddleware.hpp"

struct InterpretorFile
{
	File<FilePathMemoryLayout::STRING> file;
	size_t user_start_read;
	String<> user_input;

	enum State
	{
		UNDEFINED = 0,
		WAITING_FOR_MACHINE = 1,
		WAITING_FOR_USER = 2
	} state = State::UNDEFINED;

	inline void allocate()
	{
		FilePath<FilePathMemoryLayout::STRING> l_path;
		l_path.allocate(99999);
		l_path.path.append("C:/Users/loicd/Desktop/test_file.txt");
		this->file.allocate(FileType::CONTENT, l_path);
		this->file.create_or_open();
		this->user_input.allocate(99999);
	};

	inline void free()
	{
		this->file.free();
		this->user_input.free();
	};

	inline void start()
	{
		if (this->state == State::UNDEFINED)
		{
			this->user_start_read = FileAlgorithm::get_char_nb(this->file.path.path.c_str());
			this->state = State::WAITING_FOR_MACHINE;
			this->machine_end();
		}
	}

	inline void machine_start()
	{
		if (this->state == State::WAITING_FOR_USER)
		{
			this->state = State::WAITING_FOR_MACHINE;
		}
	};

	inline void machine_end()
	{
		if (this->state == State::WAITING_FOR_MACHINE)
		{
			this->state = State::WAITING_FOR_USER;

			this->user_input.clear();
			this->user_start_read = FileAlgorithm::get_char_nb(this->file.path.path.c_str());
		}
	};

	inline void machine_write(StringSlice& p_str)
	{
		if (this->state == State::WAITING_FOR_MACHINE)
		{
			this->file.append(p_str);
			this->file.append(StringSlice("\n"));
		}
	}

	inline bool has_input_ended()
	{
		if (this->state == State::WAITING_FOR_USER)
		{
			this->file.read(this->user_start_read, FileAlgorithm::get_char_nb(this->file.path.path.c_str()) - this->user_start_read, this->user_input);
			if (this->user_input.Memory[this->user_input.Memory.Size - 1] == '\n')
			{
				return true;
			}
		}

		return false;
	};

	inline String<> allocate_formatted_input()
	{
		this->user_input.Memory[this->user_input.Memory.Size] = '\0';
		String<> l_str;
		l_str.allocate(0);
		l_str.append(this->user_input.toSlice());
		l_str.remove_chars('\t');
		l_str.remove_chars('\r');
		l_str.remove_chars('\n');
		return l_str;
	}
};








struct EditorSceneEventHeader
{
	unsigned int Type;
};

struct EditorSceneEventMoveNode
{
	inline static const unsigned int Type = 1;

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
		SceneKernel::set_localposition(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->new_localposition);
	};

	inline void _undo(Scene* p_scene)
	{
		SceneKernel::set_localposition(SceneKernel::resolve_node(p_scene, this->scene_node).element, p_scene, this->old_localposition);
	};
};

struct EditorSceneEventRotateNode
{
	inline static const unsigned int Type = EditorSceneEventMoveNode::Type + 1;

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
	// Scene proxy_scene;
	com::Vector<EditorSceneEvent> undo_events;
	String<> engine_scene_asset_path;

	inline void allocate(Scene* p_engine_scene, StringSlice& p_engine_asset_path)
	{
		this->engine_scene = p_engine_scene;
		this->engine_scene_asset_path.allocate(p_engine_asset_path.size());
		this->engine_scene_asset_path.append(p_engine_asset_path);
		//this->proxy_scene = SceneKernel::clone(p_engine_scene);
	};

	inline void free()
	{
		//SceneKernel::free_scene(&this->proxy_scene);
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
		AssetServerHandle l_asset_server = engine_assetserver(p_engine);

		SceneAsset l_dup_asset = SceneSerializer2::Scene_to_SceneAsset(*this->engine_scene);
		com::Vector<char> l_scene_json = SceneSerializer2::SceneAsset_to_JSON(l_dup_asset, l_asset_server);
		
		File<FilePathMemoryLayout::STRING> l_scene_file;
		FilePath<FilePathMemoryLayout::STRING> l_scene_file_path;
		l_scene_file_path.allocate(0);
		l_scene_file_path.path.append(l_asset_server.get_asset_basepath().c_str());
		l_scene_file_path.path.append("scenes/alala_scene.json");
		l_scene_file.allocate(FileType::CONTENT, l_scene_file_path);
		l_scene_file.create_override();
		l_scene_file.append(StringSlice(l_scene_json.Memory));
		
		l_scene_file.free();

		l_scene_json.free();
		l_dup_asset.free();
	};

	inline void set_localposition(NTreeResolve<SceneNode>& p_scene_node, Math::vec3f& p_local_position)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventMoveNode(SceneNodeToken(p_scene_node.node->index), SceneKernel::get_localposition(p_scene_node.element), p_local_position));
		this->undo_events.push_back(l_event);

		((EditorSceneEventMoveNode*)l_event.object)->_do(this->engine_scene);
		//((EditorSceneEventMoveNode*)l_event.object)->_do(&this->proxy_scene);
	};

	inline void set_localrotation(NTreeResolve<SceneNode>& p_scene_node, Math::quat& p_local_rotation)
	{
		EditorSceneEvent l_event;
		l_event.allocate(EditorSceneEventRotateNode(SceneNodeToken(p_scene_node.node->index), SceneKernel::get_localrotation(p_scene_node.element), p_local_rotation));
		this->undo_events.push_back(l_event);

		((EditorSceneEventRotateNode*)l_event.object)->_do(this->engine_scene);
		//((EditorSceneEventRotateNode*)l_event.object)->_do(&this->proxy_scene);
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



		//TODO test
		// SceneAsset l_dup_asset = SceneAssetBuilder::build_from_scene(*engine_scene(this->running_engine));
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
};













struct NodeMovement
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

private:

	struct SelectedNodeRenderer
	{
		SceneNodeToken node;
		Math::vec4f original_color;

		inline void set_original_meshrenderer(Scene* p_scene, RenderMiddlewareHandle p_render_middleware)
		{
			p_render_middleware->get_renderable_object(SceneKernel::get_component<MeshRenderer>(p_scene, this->node))->default_material.set_color(p_render_middleware->render, this->original_color);
			//	p_render_middleware->set_material(p_scene.get_component<MeshRenderer>(this->node), this->original_material);
		};
	};

	SceneNodeToken root_selected_node;
	com::Vector<SelectedNodeRenderer> selected_nodes_renderer;
	Scene* selected_node_scene;

public:

	inline void set_selected_node(Scene* p_scene, SceneNodeToken& p_node, RenderMiddlewareHandle p_render_middleware)
	{
		for (size_t i = 0; i < this->selected_nodes_renderer.Size; i++)
		{
			this->selected_nodes_renderer[i].set_original_meshrenderer(this->selected_node_scene, p_render_middleware);
		}
		this->selected_nodes_renderer.clear();

		this->root_selected_node = SceneNodeToken();
		this->selected_node_scene = p_scene;

		if (p_node.Index == -1)
		{
			this->exit(p_render_middleware);
		}
		else
		{
			this->root_selected_node = p_node;
			this->selected_node_scene = p_scene;

			struct SceneForeach
			{
				com::Vector<SelectedNodeRenderer>* out_selected_node_renderers;
				Scene* scene = nullptr;
				RenderMiddleware* render_middleware;

				inline SceneForeach(com::Vector<SelectedNodeRenderer>* p_selected_node_renderers, Scene* p_scene, RenderMiddleware* p_render_middleware) {
					this->out_selected_node_renderers = p_selected_node_renderers;
					this->scene = p_scene;
					this->render_middleware = p_render_middleware;
				}

				inline void foreach(NTreeResolve<SceneNode>& p_node)
				{

					MeshRenderer* l_mesh_renderer = SceneKernel::get_component<MeshRenderer>(this->scene, p_node.node->index);
					if (l_mesh_renderer)
					{
						DefaultMaterial* l_renderableobject_material = &this->render_middleware->get_renderable_object(l_mesh_renderer)->default_material;

						SelectedNodeRenderer l_selected_node_renderer;
						l_selected_node_renderer.node = p_node.node->index;
						l_selected_node_renderer.original_color = l_renderableobject_material->get_color(this->render_middleware->render);

						l_renderableobject_material->set_color(this->render_middleware->render, Math::vec4f(3.0f, 3.0f, 3.0f, 1.0f));
						// this->render_middleware->set_material(l_mesh_renderer, Hash<StringSlice>::hash(StringSlice("materials/editor_selected.json")));
						this->out_selected_node_renderers->push_back(l_selected_node_renderer);
					}
				};
			};

			p_scene->tree.traverse(this->root_selected_node, SceneForeach(&this->selected_nodes_renderer, p_scene, p_render_middleware));
		}


	};

	inline void handle_scene_undo(EngineRunningModule& p_engine_running_module)
	{
		InputHandle l_input = engine_input(p_engine_running_module.running_engine);

		if (l_input.get_state(InputKey::InputKey_P, KeyState::KeyStateFlag_PRESSED))
		{
			p_engine_running_module.editor_scene._undo();
		}
	};

	inline void perform_node_movement(EngineRunningModule& p_engine_running_module)
	{
		if (this->root_selected_node.Index != -1)
		{
			InputHandle l_input = engine_input(p_engine_running_module.running_engine);

			if (l_input.get_state(InputKey::InputKey_T, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->state = NodeMovement::State::POSITION_LOCAL;
				printf("NodeMovement : Position mode enabled\n");
			}
			else if (l_input.get_state(InputKey::InputKey_R, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->state = NodeMovement::State::ROTATION_LOCAL;
				printf("NodeMovement : Rotation mode enabled\n");
			}
			else if (l_input.get_state(InputKey::InputKey_S, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->state = NodeMovement::State::SCALE_LOCAL;
				printf("NodeMovement : Scale mode enabled\n");
			}

			if (l_input.get_state(InputKey::InputKey_X, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->direction = NodeMovement::Direction::X;
				printf("NodeMovement : X axis \n");
			}
			else if (l_input.get_state(InputKey::InputKey_Y, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->direction = NodeMovement::Direction::Y;
				printf("NodeMovement : Y axis \n");
			}
			else if (l_input.get_state(InputKey::InputKey_Z, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{
				this->direction = NodeMovement::Direction::Z;
				printf("NodeMovement : Z axis \n");
			}


			if (this->direction != NodeMovement::Direction::UNDEFINED &&
				this->state != NodeMovement::State::UNDEFINED)
			{

				Scene* p_scene = p_engine_running_module.editor_scene.engine_scene;
				NTreeResolve<SceneNode> l_node = SceneKernel::resolve_node(p_scene, this->root_selected_node);

				switch (this->state)
				{
				case NodeMovement::State::POSITION_LOCAL:
				{

					float l_value = 0.0f;

					if (l_input.get_state(InputKey::InputKey_UP, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
					{
						l_value = this->movement_step;
					}
					else if (l_input.get_state(InputKey::InputKey_DOWN, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
					{
						l_value = -this->movement_step;
					}


					Math::vec4f l_delta = Math::vec4f(0.0f, 0.0f, 0.0f, 0.0f);

					switch (this->direction)
					{
					case NodeMovement::Direction::X:
					{
						l_delta.x = l_value;
					}
					break;
					case NodeMovement::Direction::Y:
					{
						l_delta.y = l_value;
					}
					break;
					case NodeMovement::Direction::Z:
					{
						l_delta.z = l_value;
					}
					break;
					}

					l_delta = mul(SceneKernel::get_worldtolocal(l_node.element, p_scene), l_delta);

					if (!Math::EqualsVec(l_delta, Math::vec4f(0.0f, 0.0f, 0.0f, 0.0f)))
					{
						p_engine_running_module.editor_scene.set_localposition(l_node, SceneKernel::get_localposition(l_node.element) + l_delta.Vec3);
						// SceneKernel::set_localposition(l_node.element, p_scene, SceneKernel::get_localposition(l_node.element) + l_delta.Vec3);
					}
				}
				break;
				case NodeMovement::State::ROTATION_LOCAL:
				{

					float l_value = 0.0f;

					if (l_input.get_state(InputKey::InputKey_UP, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
					{
						l_value = this->rotation_step_deg * DEG_TO_RAD;
					}
					else if (l_input.get_state(InputKey::InputKey_DOWN, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
					{
						l_value = -this->rotation_step_deg * DEG_TO_RAD;
					}


					Math::vec3f l_axis = Math::vec3f(0.0f, 0.0f, 0.0f);
					switch (this->direction)
					{
					case NodeMovement::Direction::X:
					{
						l_axis.x = 1.0f;
					}
					break;
					case NodeMovement::Direction::Y:
					{
						l_axis.y = 1.0f;
					}
					break;
					case NodeMovement::Direction::Z:
					{
						l_axis.z = 1.0f;
					}
					break;
					}

					Math::quat l_delta = Math::rotateAround(l_axis, l_value);
					if (!Math::Equals(l_delta, Math::QuatConst::IDENTITY))
					{
						p_engine_running_module.editor_scene.set_localrotation(l_node, mul(SceneKernel::get_localrotation(l_node.element), Math::rotateAround(l_axis, l_value)));
					}
				}
				break;
				}

			}
		}

	};

	inline void exit(RenderMiddlewareHandle p_render_middleware)
	{
		for (size_t i = 0; i < this->selected_nodes_renderer.Size; i++)
		{
			this->selected_nodes_renderer[i].set_original_meshrenderer(this->selected_node_scene, p_render_middleware);
		}
		this->selected_nodes_renderer.clear();

		this->root_selected_node = SceneNodeToken();
		this->selected_node_scene = nullptr;

		this->state = State::UNDEFINED;
	}
};

struct ToolState
{
	com::TPoolToken<Optional<EngineRunningModule>> selectged_engine;
	EngineRunner engine_runner;
	NodeMovement node_movement;
};

struct CommandHandler
{
	inline static const StringSlice start_slice = StringSlice("start");
	inline static const StringSlice stop_slice = StringSlice("stop");
	inline static const StringSlice select_slice = StringSlice("select");
	inline static const StringSlice load_slice = StringSlice("load");
	inline static const StringSlice print_slice = StringSlice("print");
	inline static const StringSlice persist_slice = StringSlice("persist");
	inline static const StringSlice n_slice = StringSlice("n");
	inline static const StringSlice engine_slice = StringSlice("engine");
	inline static const StringSlice scene_slice = StringSlice("scene");
	inline static const StringSlice node_slice = StringSlice("node");
	// inline static const StringSlice movement_slice = StringSlice("movement");
	inline static const StringSlice set_local_position_slice = StringSlice("set_local_position");

	inline static void handle_engine_commands(InterpretorFile& p_command_file, ToolState& p_tool_state, com::Vector<StringSlice>& p_command_stack, size_t p_depth)
	{
		if (p_command_stack[p_depth].equals(CommandHandler::start_slice))
		{
			EngineRunningModule l_engine;
			l_engine.start();
			l_engine.update(0);
			com::PoolToken l_allocated_engine = p_tool_state.engine_runner.engines.alloc_element(l_engine);
			p_tool_state.selectged_engine = l_allocated_engine.Index;
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::stop_slice))
		{
			if (p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].hasValue)
			{
				p_tool_state.node_movement.exit(engine_render_middleware(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine));
				p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.stop();
				p_tool_state.engine_runner.engines.release_element(p_tool_state.selectged_engine);
			}
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::select_slice))
		{
			size_t l_engine_index = FromString<size_t>::from_str(p_command_stack[p_depth + 1]);

			if (l_engine_index == -1 || p_tool_state.selectged_engine.Index != -1)
			{
				p_tool_state.node_movement.exit(engine_render_middleware(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine));
			}
			if (l_engine_index == -1 || p_tool_state.engine_runner.engines[l_engine_index].hasValue)
			{
				p_tool_state.selectged_engine = l_engine_index;
			}
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::print_slice))
		{
			for (size_t i = 0; i < p_tool_state.engine_runner.engines.size(); i++)
			{
				if (p_tool_state.engine_runner.engines[i].hasValue)
				{
					String<> l_message; l_message.allocate(0);
					l_message.append(i);
					l_message.append(",");
					p_command_file.machine_write(l_message.toSlice());
					l_message.free();
				}
			}
		}
	}

	inline static void handle_scene_commands(InterpretorFile& p_command_file, ToolState& p_tool_state, com::Vector<StringSlice>& p_command_stack, size_t p_depth)
	{
		Optional<EngineRunningModule> l_running_rengine = p_tool_state.engine_runner.engines[p_tool_state.selectged_engine];

		if (p_command_stack[p_depth].equals(CommandHandler::load_slice))
		{
			if (l_running_rengine.hasValue)
			{
				p_tool_state.node_movement.exit(engine_render_middleware(l_running_rengine.value.running_engine));
				l_running_rengine.value.load_scene(p_command_stack[p_depth + 1]);
				l_running_rengine.value.update(0);
			}
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::print_slice))
		{
			if (l_running_rengine.hasValue)
			{
				Scene* p_scene = engine_scene(l_running_rengine.value.running_engine);

				struct ScenePrintForeach
				{
					String<>* message;

					inline ScenePrintForeach(String<>* p_message) {
						this->message = p_message;
					};

					inline void foreach(NTreeResolve<SceneNode>& p_node)
					{
						this->message->append(p_node.node->index);
						this->message->append(",");
					};
				};

				String<> l_message; l_message.allocate(0);

				ScenePrintForeach l_f = ScenePrintForeach(&l_message);
				p_scene->tree.traverse(com::PoolToken(0), l_f);

				p_command_file.machine_write(l_message.toSlice());

				l_message.free();
			}
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::persist_slice))
		{
			if (l_running_rengine.hasValue)
			{
				l_running_rengine.value.editor_scene.persist_current_scene(l_running_rengine.value.running_engine);
			}
		}
	}

	inline static void handle_node_commands(InterpretorFile& p_command_file, ToolState& p_tool_state, com::Vector<StringSlice>& p_command_stack, size_t p_depth)
	{
		if (p_command_stack[p_depth].equals(CommandHandler::select_slice))
		{
			size_t l_selected_node = FromString<size_t>::from_str(StringSlice(p_command_stack[p_depth + 1]));
			p_tool_state.node_movement.set_selected_node(engine_scene(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine),
				SceneNodeToken(l_selected_node), engine_render_middleware(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine));
		}
	};

};

int main()
{
	HANDLE l_stdin = GetStdHandle(STD_INPUT_HANDLE);

	size_t l_current_engine = -1;

	InterpretorFile l_interop;
	l_interop.allocate();
	l_interop.start();

	ToolState tool_state;

	while (true)
	{
		tool_state.engine_runner.update();

		if (l_interop.has_input_ended())
		{
			l_interop.machine_start();

			String<> l_str = l_interop.allocate_formatted_input();

			com::Vector<StringSlice> l_words = l_str.split(" ");
			{
				if (l_words[0].equals(CommandHandler::engine_slice))
				{
					CommandHandler::handle_engine_commands(l_interop, tool_state, l_words, 1);
				}
				else if (l_words[0].equals(CommandHandler::scene_slice))
				{
					CommandHandler::handle_scene_commands(l_interop, tool_state, l_words, 1);
				}
				else if (l_words[0].equals(CommandHandler::node_slice))
				{
					CommandHandler::handle_node_commands(l_interop, tool_state, l_words, 1);
				}
			}

			l_words.free();

			l_str.free();

			l_interop.machine_end();
		}

		if (tool_state.selectged_engine.Index != -1)
		{
			if (tool_state.engine_runner.engines[tool_state.selectged_engine].hasValue)
			{
				EngineRunningModule& l_engine_module = tool_state.engine_runner.engines[tool_state.selectged_engine].value;
				if (l_engine_module.frame_executed)
				{
					tool_state.node_movement.handle_scene_undo(l_engine_module);
					tool_state.node_movement.perform_node_movement(l_engine_module);
				}

			};

		}

	}

	l_interop.free();


	return 0;
};