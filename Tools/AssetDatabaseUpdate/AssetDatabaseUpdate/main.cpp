
#include <string>
#include <Common/Thread/thread.hpp>
#include <Common/Container/vector.hpp>
#include <Common/File/file.hpp>

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

const char* PROJECT_PATH = "E:/GameProjects/CPPTestVS/";

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


struct GetAssetsPath : public WorkerThread::Task
{
	com::Vector<String<>> found_paths;

	inline void free()
	{
		for (size_t i = 0; i < this->found_paths.Size; i++)
		{
			this->found_paths[i].free();
		}
		this->found_paths.free();
	};

	inline void tick_internal() override
	{
		struct AssetRootFilter
		{
			inline static bool filter(const String<>& p_file)
			{
				size_t l_index;
				return p_file.find(StringSlice(".assetroot"), 0, &l_index);
			}
		};

		struct AssetFilter
		{
			inline static bool filter(const String<>& p_file)
			{
				size_t l_index;
				return p_file.find(".spv", 0, &l_index);
			}
		};

		com::Vector<String<>> l_asset_root;
		File::find_file<AssetRootFilter>(StringSlice(PROJECT_PATH), l_asset_root, true);
		if (l_asset_root.Size > 0)
		{
			com::Vector<String<>> l_asset_paths;
			l_asset_paths.allocate(0);
			{
				StringSlice l_asset_folder = l_asset_root[0].toSlice();
				size_t l_index;
				if (l_asset_folder.find(".assetroot", &l_index))
				{
					l_asset_folder.End = l_index;
					File::find_file<AssetFilter>(l_asset_folder, l_asset_paths, true);

					for (size_t i = 0; i < l_asset_paths.Size; i++)
					{
						this->found_paths.push_back(l_asset_paths[i]);
					}
				}
			}
			l_asset_paths.free();
		}

		for (size_t i = 0; i < l_asset_root.Size; i++)
		{
			l_asset_root[i].free();
		}
		l_asset_root.free();

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


						for (size_t i = 0; i < this->asset_path_task->found_paths.Size; i++)
						{
							this->editor->window.list.addItem(QString(this->asset_path_task->found_paths[i].Memory.Memory));
						}

						this->asset_path_task->free();
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