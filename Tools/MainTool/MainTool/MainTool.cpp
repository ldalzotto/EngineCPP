#include <stdio.h>

#include "Common/Container/pool.hpp"
#include "Common/Thread/thread.hpp"
#include "Common/File/file.hpp"
#include "Engine/engine.hpp"
#include "Common/Functional/ToString.hpp"
#include "Scene/../../Scene/Scene/scene.cpp"


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
		if (this->running_engine)
		{
			if (!engine_should_close(this->running_engine))
			{
				engine_singleframe(this->running_engine);
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
		if (this->running_engine)
		{
			if (!engine_should_close(this->running_engine))
			{
				engine_singleframe(this->running_engine, p_delta);
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
		EngineRunningModule* thiz = (EngineRunningModule*)p_thiz;
	};
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

struct ToolState
{
	com::PoolToken selectged_engine;
	EngineRunner engine_runner;
	com::PoolToken selected_node;
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
	inline static const StringSlice set_local_position_slice = StringSlice("set_local_position");

	inline static void handle_engine_commands(InterpretorFile& p_command_file, ToolState& p_tool_state, com::Vector<StringSlice>& p_command_stack, size_t p_depth)
	{
		if (p_command_stack[p_depth].equals(CommandHandler::start_slice))
		{
			EngineRunningModule l_engine;
			l_engine.start();
			l_engine.update(0);
			p_tool_state.engine_runner.engines.alloc_element(l_engine);
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
			p_tool_state.selected_node = FromString<size_t>::from_str(StringSlice(p_command_stack[p_depth + 1]));
		}
		else if (p_command_stack[p_depth].equals(CommandHandler::set_local_position_slice))
		{
			if (p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].hasValue)
			{
				Scene* p_scene = (Scene*)engine_scene(p_tool_state.engine_runner.engines[p_tool_state.selectged_engine].value.running_engine).handle;

				NTreeResolve<SceneNode> l_node = p_scene->resolve_node(p_tool_state.selected_node);

				String<> tmp; tmp.allocate(0);
				tmp.append(p_command_stack[p_depth + 1]);
				Serialization::JSON::JSONObjectIterator l_it = Serialization::JSON::StartDeserialization(tmp);
				l_node.element->set_localposition(JSONDeserializer<Math::vec3f>::deserialize(l_it));
				tmp.free();
			}
		}
	}
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
				/*
				else if (l_words[0].equals("n"))
				{
					l_engine_module.update();

					if (l_words.Size > 0)
					{
						//TODO Do frame count
					}
				};
				*/
			}

			l_words.free();

			l_str.free();

			l_interop.machine_end();
		}

	}

	l_interop.free();


	return 0;
};