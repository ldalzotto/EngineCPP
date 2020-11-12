
#include <string>
#include <Common/Thread/thread.hpp>
#include <Common/Container/vector.hpp>

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