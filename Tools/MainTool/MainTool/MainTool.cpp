#include <stdio.h>

#include "Common/Container/pool.hpp"
#include "Common/Thread/thread.hpp"
#include "Common/File/file.hpp"
#include "Engine/engine.hpp"
#include "Common/Functional/ToString.hpp"
#include "Scene/../../Scene/Scene/scene.cpp"
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

struct EngineRunningModule
{
	ExternalHooks engine_hooks;
	EngineHandle running_engine = nullptr;
	bool frame_executed = false;

	inline void start()
	{
		this->engine_hooks.closure = this;
		this->engine_hooks.ext_update = EngineRunningModule::update;
		this->running_engine = engine_create("", this->engine_hooks);
	};

	inline void stop()
	{
		engine_destroy(this->running_engine);
	};

	inline void editor_update()
	{
		if (this->running_engine)
		{
			engine_poll_events(this->running_engine);
		}
	}

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
		com::PoolToken node;
		size_t original_material;

		inline void set_original_meshrenderer(SceneHandle& p_scene, RenderMiddlewareHandle p_render_middleware)
		{
			p_render_middleware->set_material(p_scene.get_component<MeshRenderer>(this->node), this->original_material);
		};
	};

	com::PoolToken root_selected_node;
	com::Vector<SelectedNodeRenderer> selected_nodes_renderer;
	SceneHandle selected_node_scene;

public:

	inline void set_selected_node(SceneHandle& p_scene, com::PoolToken& p_node, RenderMiddlewareHandle p_render_middleware)
	{
		for (size_t i = 0; i < this->selected_nodes_renderer.Size; i++)
		{
			this->selected_nodes_renderer[i].set_original_meshrenderer(this->selected_node_scene, p_render_middleware);
		}
		this->selected_nodes_renderer.clear();

		this->root_selected_node = com::PoolToken();
		this->selected_node_scene = SceneHandle();

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
				SceneHandle scene;
				RenderMiddleware* render_middleware;

				inline SceneForeach(com::Vector<SelectedNodeRenderer>* p_selected_node_renderers, SceneHandle p_scene, RenderMiddleware* p_render_middleware) {
					this->out_selected_node_renderers = p_selected_node_renderers;
					this->scene = p_scene;
					this->render_middleware = p_render_middleware;
				}

				inline void foreach(NTreeResolve<SceneNode>& p_node)
				{
					
					MeshRenderer* l_mesh_renderer = this->scene.get_component<MeshRenderer>(p_node.node->index);
					if (l_mesh_renderer)
					{
						SelectedNodeRenderer l_selected_node_renderer;
						l_selected_node_renderer.node = p_node.node->index;
						l_selected_node_renderer.original_material = l_mesh_renderer->material;

						this->render_middleware->set_material(l_mesh_renderer, Hash<StringSlice>::hash(StringSlice("materials/editor_selected.json")));
						this->out_selected_node_renderers->push_back(l_selected_node_renderer);
					}
				};
			};

			((Scene*)p_scene.handle)->tree.traverse(this->root_selected_node, SceneForeach(&this->selected_nodes_renderer, p_scene, p_render_middleware));
		}


	};

	inline void perform_node_movement(EngineHandle p_engine)
	{
		if (this->root_selected_node.Index != -1)
		{
			InputHandle l_input = engine_input(p_engine);

			/*
			if (l_input.get_state(InputKey::InputKey_L, KeyState::KeyStateFlag_PRESSED_THIS_FRAME))
			{

			}
			*/

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

				Scene* p_scene = (Scene*)engine_scene(p_engine).handle;
				NTreeResolve<SceneNode> l_node = p_scene->resolve_node(this->root_selected_node);

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

					l_delta = mul(l_node.element->get_worldtolocal(), l_delta);



					l_node.element->set_localposition(l_node.element->get_localposition() + l_delta.Vec3);
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

					l_node.element->set_localrotation(mul(l_node.element->get_localrotation(), Math::rotateAround(l_axis, l_value)));
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

		this->root_selected_node = com::PoolToken();
		this->selected_node_scene = SceneHandle();

		this->state = State::UNDEFINED;
	}
};

struct ToolState
{
	com::PoolToken selectged_engine;
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
				p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.stop();
				p_tool_state.engine_runner.engines.release_element(p_tool_state.selectged_engine);
			}
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::select_slice))
		{
			size_t l_engine_index = FromString<size_t>::from_str(p_command_stack[p_depth + 1]);
			if (p_tool_state.engine_runner.engines[l_engine_index].hasValue)
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
		if (p_command_stack[p_depth].equals(CommandHandler::load_slice))
		{
			if (p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].hasValue)
			{
				com::Vector<char> l_scene = engine_assetserver(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine).get_resource(Hash<StringSlice>::hash(p_command_stack[p_depth + 1]));
				size_t l_pointer = 0;
				engine_scene(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine).feed_with_asset(SceneSerializer::deserialize_from_binary(l_scene));
				l_scene.free();
				p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.update(0);
			}
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::print_slice))
		{
			if (p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].hasValue)
			{
				Scene* p_scene = (Scene*)engine_scene(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine).handle;

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
	}

	inline static void handle_node_commands(InterpretorFile& p_command_file, ToolState& p_tool_state, com::Vector<StringSlice>& p_command_stack, size_t p_depth)
	{
		if (p_command_stack[p_depth].equals(CommandHandler::select_slice))
		{
			size_t l_selected_node = FromString<size_t>::from_str(StringSlice(p_command_stack[p_depth + 1]));
			p_tool_state.node_movement.set_selected_node(engine_scene(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine),
				com::PoolToken(l_selected_node), engine_render_middleware(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine));
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
					tool_state.node_movement.perform_node_movement(l_engine_module.running_engine);
				}

			};

		}

	}

	l_interop.free();


	return 0;
};