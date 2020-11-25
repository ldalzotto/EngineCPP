#include <stdio.h>

#include "Common/Container/vector.hpp"
#include "Common/Thread/thread.hpp"
#include "Common/File/file.hpp"
#include "Engine/engine.hpp"
#include "Common/Functional/ToString.hpp"

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
	com::Vector<EngineRunningModule> engines;

	inline void update()
	{
		for (size_t i = 0; i < engines.Size; i++)
		{
			engines[i].update();
		}
	}

	inline void free()
	{
		for (size_t i = 0; i < engines.Size; i++)
		{
			engines[i].stop();
		}
		engines.free();
	}
};

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
			this->file.append(StringSlice("<\n"));
		}
	};

	inline void machine_end()
	{
		if (this->state == State::WAITING_FOR_MACHINE)
		{
			this->state = State::WAITING_FOR_USER;
			this->file.append(StringSlice(">\n"));

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



int main()
{
	HANDLE l_stdin = GetStdHandle(STD_INPUT_HANDLE);

	EngineRunner l_engine_runner;
	size_t l_current_engine = -1;
	
	// EngineRunningModule l_engine_module;
	// EngineRunningModule l_engine_module_2;

	InterpretorFile l_interop;
	l_interop.allocate();
	l_interop.start();

	while (true)
	{
		l_engine_runner.update();

		if (l_interop.has_input_ended())
		{
			l_interop.machine_start();

			String<> l_str = l_interop.allocate_formatted_input();

			com::Vector<StringSlice> l_words = l_str.split(" ");
			{
				StringSlice l_start_slice = StringSlice("start");
				StringSlice l_stop_slice = StringSlice("stop");
				StringSlice l_load_slice = StringSlice("load");
				StringSlice n_slice = StringSlice("n");
				StringSlice l_engine_slice = StringSlice("engine");
				StringSlice l_scene_slice = StringSlice("scene");

				if (l_words[0].equals(l_engine_slice))
				{
					if (l_words[1].equals(l_start_slice))
					{
						EngineRunningModule l_engine;
						l_engine.start();
						l_engine.update(0);
						l_engine_runner.engines.push_back(l_engine);
						String<> l_message; l_message.allocate(0);
						l_message.append("Engine created : ");
						// String<> l_engine_sizes_str = ToString<size_t>::to_str(engines.Size);
						l_message.append(l_engine_runner.engines.Size - 1);
						l_interop.machine_write(l_message.toSlice());
						l_message.free();
					}
					else
					{
						size_t l_engine_index = FromString<size_t>::from_str(l_words[1]);

						if (l_words[2].equals(l_stop_slice))
						{
							l_engine_runner.engines[l_engine_index].stop();
						}
					}
				}
				else if (l_words[0].equals(l_scene_slice))
				{
					size_t l_engine_index = FromString<size_t>::from_str(l_words[1]);

					if (l_words[2].equals(l_load_slice))
					{
						com::Vector<char> l_scene = engine_assetserver(l_engine_runner.engines[l_engine_index].running_engine).get_resource(Hash<StringSlice>::hash(l_words[3]));
						size_t l_pointer = 0;
						engine_scene(l_engine_runner.engines[l_engine_index].running_engine).feed_with_asset(SceneSerializer::deserialize_from_binary(l_scene));
						l_scene.free();
						l_engine_runner.engines[l_engine_index].update(0);
					}
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