
#include <string>
#include <vector>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qpushbutton.h>
#include <QtGui/qfont.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qlistwidget.h>
#include <QtCore/qtimer.h>

#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

const std::string PROJECT_PATH = "E:/GameProjects/CPPTestVS/";

template<class ElementType>
struct ConcurrentVector
{

public:

	enum State
	{
		FREE = 0,
		LOCKED = 1
	} state;
	std::vector<ElementType> items;

	ConcurrentVector() = default;

	inline ConcurrentVector(const std::vector<ElementType>& p_vector)
	{
		this->items = p_vector;
		this->state = State::FREE;
	}

	inline std::vector<ElementType>& grab_vector_wait()
	{
		while (this->state == State::LOCKED) {}
		this->state = State::LOCKED;
		return this->items;
	};

	inline void release_vector()
	{
		this->state = State::FREE;
	};
};

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

		// void* stack;
		// void(*tick)(Task* p_task);

		inline Task()
		{
			this->state = State::ENDED;
			//	this->stack = nullptr;
			//	this->tick = nullptr;
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
		/*
		inline Task(void* p_stack, void(*p_tick)(Task* p_task))
		{
			this->stack = p_stack;
			this->tick = p_tick;
		};
		*/
	};

	ConcurrentVector<Task*> tasks;

	inline void allocate()
	{
		auto l_tasks = this->tasks.grab_vector_wait();
		l_tasks.reserve(5);
		this->tasks.release_vector();

		this->handle = CreateThread(NULL, 0, WorkerThread::ThreadMain, this, 0, (LPDWORD)this);
	}

	inline void exit()
	{
		CloseHandle(this->handle);
	}

	inline void push_task(Task* p_task)
	{
		auto& l_tasks = this->tasks.grab_vector_wait();
		p_task->state = Task::State::RUNNING;
		l_tasks.push_back(p_task);

		if (l_tasks.size() == 1)
		{
			this->tasks.release_vector();
			this->awake();
		}
		else
		{
			this->tasks.release_vector();
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
				auto l_tasks = this->tasks.grab_vector_wait();
				{
					l_nb_of_tasks = l_tasks.size();
				}
				this->tasks.release_vector();
			}


			for (size_t i = 0; i < l_nb_of_tasks; i++)
			{
				Task*& l_current_task = this->tasks.items[i];
				l_current_task->tick();
			}

			for (int i = l_nb_of_tasks - 1; i >= 0; i--)
			{
				Task*& l_current_task = this->tasks.items[i];
				if (l_current_task->state == Task::State::ENDED)
				{
					auto& l_vector = this->tasks.grab_vector_wait();
					{
						l_vector.erase(l_vector.begin() + i);
					}
					this->tasks.release_vector();
				}
			}


			{
				auto l_tasks = this->tasks.grab_vector_wait();
				{
					if (l_tasks.size() == 0)
					{
						this->tasks.release_vector();
						this->sleep();
					}
					else
					{
						this->tasks.release_vector();
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

	inline void onExit()
	{

	}
};

template<class Callbacks>
struct MainWindow
{
	QMainWindow window;
	QListWidget list;
	Callbacks callbacks;

	inline MainWindow() {}

	inline void allocate(const Callbacks& p_callbacks)
	{
		this->callbacks = p_callbacks;

		QMenu* l_file_menu = this->window.menuBar()->addMenu(QString("&File"));
		QAction* l_findroot_action = new QAction(QString("Find root"), &this->window);
		l_file_menu->addAction(l_findroot_action);
		this->window.connect(l_findroot_action, &QAction::triggered, [=]() {this->callbacks.on_findroot_clicked(); });

		this->window.setCentralWidget(&this->list);
		// .setParent(this->window.centralWidget());

		this->window.show();
	}

	inline void set_statusbarmessage(const std::string& p_message)
	{
		this->window.statusBar()->showMessage(QString(p_message.c_str()));
	};

};

struct File
{
	template<class FindFileFilterFn>
	inline static void find_file(const std::string& p_path, std::vector<std::string>& out_paths_buffer, bool p_recursive)
	{
		std::string l_current_file = p_path;
		WIN32_FIND_DATA l_find_data;
		HANDLE l_find = INVALID_HANDLE_VALUE;

		std::string l_found_file;

		l_find = FindFirstFile((l_current_file + "*").c_str(), &l_find_data);
		while (l_find != INVALID_HANDLE_VALUE)
		{
			if (p_recursive && (l_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (p_recursive)
				{
					if (std::string(".").compare(l_find_data.cFileName) != 0 && std::string("..").compare(l_find_data.cFileName) != 0)
					{
						find_file<FindFileFilterFn>(l_current_file + l_find_data.cFileName + "/", out_paths_buffer, p_recursive);
					}
				}

			}
			else
			{
				if (FindFileFilterFn::filter(l_current_file + l_find_data.cFileName))
				{
					l_found_file = l_current_file + l_find_data.cFileName;
					out_paths_buffer.push_back(l_found_file);
				}
			}

			if (!FindNextFile(l_find, &l_find_data))
			{
				goto end;
			};
		}

	end:
		FindClose(l_find);
	};
};

struct GetAssetsPath : public WorkerThread::Task
{
	std::vector<std::string> found_paths;

	inline GetAssetsPath()
	{
	}

	inline void tick_internal() override
	{
		struct AssetRootFilter
		{
			inline static bool filter(const std::string& p_file)
			{
				return p_file.find(".assetroot", 0) != std::string::npos;
			}
		};

		struct AssetFilter
		{
			inline static bool filter(const std::string& p_file)
			{
				return p_file.find(".spv", 0) != std::string::npos;
			}
		};

		// this->editor->window.set_statusbarmessage("Finding root...");
		std::vector < std::string > l_asset_root;
		File::find_file<AssetRootFilter>(PROJECT_PATH, l_asset_root, true);
		if (l_asset_root.size() > 0)
		{
			// this->editor->window.set_statusbarmessage("...root found !");

			std::vector < std::string > l_asset_paths;
			File::find_file<AssetFilter>(l_asset_root[0].substr(0, l_asset_root[0].find(".assetroot")), l_asset_paths, true);

			for (size_t i = 0; i < l_asset_paths.size(); i++)
			{
				this->found_paths.push_back(l_asset_paths[i]);
			}
		}

		this->state = State::ENDED;
	};

	inline void onEnded() override
	{

	};
};


struct AssetDatabaseRefreshEditor
{
	QApplication* app;
	WorkerThread worker_thread;

	struct MainWindowCallbacks
	{
		QTimer* asset_path_task_timer = nullptr;
		GetAssetsPath* asset_path_task = nullptr;
		AssetDatabaseRefreshEditor* editor = nullptr;

		MainWindowCallbacks()
		{
			this->asset_path_task_timer = nullptr;
			this->asset_path_task = nullptr;
			this->editor = nullptr;
		};

		inline MainWindowCallbacks(AssetDatabaseRefreshEditor& p_editor)
		{
			this->editor = &p_editor;
		};

		inline void on_findroot_clicked()
		{
			if (this->asset_path_task == nullptr)
			{
				this->asset_path_task = new GetAssetsPath();
				this->editor->worker_thread.push_task(this->asset_path_task);



				this->asset_path_task_timer = new QTimer();
				this->asset_path_task_timer->setInterval(16);
				this->asset_path_task_timer->start();


				this->editor->window.window.connect(this->asset_path_task_timer, &QTimer::timeout, [=]() {
					if (this->asset_path_task->state == GetAssetsPath::State::ENDED)
					{
						this->asset_path_task_timer->stop();
						this->editor->window.window.disconnect(this->asset_path_task_timer);
						delete this->asset_path_task_timer;
						this->asset_path_task_timer = nullptr;


						for (size_t i = 0; i < this->asset_path_task->found_paths.size(); i++)
						{
							this->editor->window.list.addItem(QString(this->asset_path_task->found_paths[i].c_str()));
						}

						delete this->asset_path_task;
						this->asset_path_task = nullptr;
					}
					});
			}
		};

	};

	MainWindow<MainWindowCallbacks> window;

	inline void allocate(QApplication& p_app)
	{
		this->app = &p_app;
		this->worker_thread.allocate();
		this->window.allocate(MainWindowCallbacks(*this));



		this->app->exec();
	}
};

int main(int argc, char** argv)
{
	QApplication l_app(argc, argv);
	AssetDatabaseRefreshEditor l_editor;

	l_editor.allocate(l_app);
}