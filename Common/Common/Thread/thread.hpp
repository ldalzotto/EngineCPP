#pragma once

#include "Common/Include/platform_include.hpp"
#include "Common/Container/vector.hpp"

struct WorkerThread
{
	HANDLE handle;

	enum State
	{
		RUNNING = 0,
		WAITING = 1
	} state;

	struct Task
	{
		enum State
		{
			RUNNING = 0,
			ENDED = 1
		} state;

		inline Task()
		{
			this->state = State::ENDED;
		};

		inline void tick()
		{
			this->tick_internal();
			if (this->state == State::ENDED)
			{
				this->onEnded();
			}
		};

		virtual void tick_internal() = 0;
		virtual void onEnded() = 0;
	};

	com::ConcurrentVector<Task*> tasks;

	inline void allocate()
	{
		this->tasks.allocate(5);
		this->handle = CreateThread(NULL, 0, WorkerThread::ThreadMain, this, 0, (LPDWORD)this);
	}

	inline void free()
	{
		this->tasks.free();
		CloseHandle(this->handle);
	}

	inline void push_task(Task* p_task)
	{
		auto& l_tasks = this->tasks.mutex.get();
		p_task->state = Task::State::RUNNING;
		l_tasks.push_back(p_task);

		if (l_tasks.Size == 1)
		{
			this->tasks.mutex.give();
			this->awake();
		}
		else
		{
			this->tasks.mutex.give();
		}
	}

private:
	inline static DWORD ThreadMain(LPVOID p_param)
	{
		WorkerThread* l_workerthread = (WorkerThread*)p_param;
		l_workerthread->main();
		return 0;
	}


	inline void main()
	{
		while (true)
		{
			size_t l_nb_of_tasks = 0;
			{
				auto& l_tasks = this->tasks.mutex.get();
				{
					l_nb_of_tasks = l_tasks.Size;
				}
				this->tasks.mutex.give();
			}


			for (size_t i = 0; i < l_nb_of_tasks; i++)
			{
				Task*& l_current_task = this->tasks.get_unsafe()[i];
				l_current_task->tick();
			}

			for (int i = (int)l_nb_of_tasks - 1; i >= 0; i--)
			{
				Task*& l_current_task = this->tasks.get_unsafe()[i];
				if (l_current_task->state == Task::State::ENDED)
				{
					auto& l_vector = this->tasks.mutex.get();
					{
						l_vector.erase_at(i, 1);
					}
					this->tasks.mutex.give();
				}
			}


			{
				auto& l_tasks = this->tasks.mutex.get();
				{
					if (l_tasks.Size == 0)
					{
						this->tasks.mutex.give();
						this->sleep();
					}
					else
					{
						this->tasks.mutex.give();
					}
				}
			}

		}
	}

	inline void awake()
	{
		if (this->state != State::RUNNING)
		{
			this->state = State::RUNNING;
			ResumeThread(this->handle);
		}
	}

	inline void sleep()
	{
		if (this->state != State::WAITING)
		{
			this->state = State::WAITING;
			SuspendThread(this->handle);
		}
	}
};

struct Thread
{
	HANDLE handle;

	enum State
	{
		RUNNING = 0,
		WAITING = 1
	} state;

	template<class MainFn, class MainParam>
	inline void allocate(MainParam* p_param)
	{
		this->handle = CreateThread(NULL, 0, &MainFn::main, p_param, 0, (LPDWORD)this);
	}

	inline void free()
	{
		CloseHandle(this->handle);
	}

	inline void awake()
	{
		if (this->state != State::RUNNING)
		{
			this->state = State::RUNNING;
			ResumeThread(this->handle);
		}
	}

	inline void sleep()
	{
		if (this->state != State::WAITING)
		{
			this->state = State::WAITING;
			SuspendThread(this->handle);
		}
	}
};