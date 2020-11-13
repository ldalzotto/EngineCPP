
#include <string>
#include <Common/Thread/thread.hpp>
#include <Common/Container/vector.hpp>
#include <Common/File/file.hpp>
#include <AssetServer/asset_server.hpp>

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
		QAction* l_push_to_database_action = new QAction(QString("Push to database"), &this->window);
		l_file_menu->addAction(l_push_to_database_action);
		this->window.connect(l_push_to_database_action, &QAction::triggered, [=]() {this->callbacks.on_pushtodatabase_clicked(); });

		QAction* l_compile_action = new QAction(QString("Compile"), &this->window);
		l_file_menu->addAction(l_compile_action);
		this->window.connect(l_compile_action, &QAction::triggered, [=]() {this->callbacks.on_compile_clicked(); });



		this->window.setCentralWidget(&this->list);
		// .setParent(this->window.centralWidget());

		this->window.show();
	}

	inline void set_statusbarmessage(const std::string& p_message)
	{
		this->window.statusBar()->showMessage(QString(p_message.c_str()));
	};

};

struct PushAssetsToDb : WorkerThread::Task
{
	struct In
	{
		AssetServerHandle asset_server;
	} in;

	struct Out
	{
		com::Vector<String<>> updated_files;

		inline void free()
		{
			for (size_t i = 0; i < this->updated_files.Size; i++)
			{
				this->updated_files[i].free();
			}
			this->updated_files.free();
		}

	} out;

	inline void free()
	{
		this->out.free();
	};

	PushAssetsToDb(AssetServerHandle p_asset_server_handle)
	{
		this->in.asset_server = p_asset_server_handle;
	};

	inline void tick_internal() override
	{
		struct AssetFilter
		{
			inline static bool filter(const String<>& p_file)
			{
				size_t l_index;
				return p_file.find(".spv", 0, &l_index) || p_file.find(".obj", 0, &l_index);
			}
		};

		std::string l_folder_path = this->in.asset_server.get_asset_basepath();
		StringSlice l_asset_folder_absolute = StringSlice(l_folder_path.c_str());

		File::find_file<AssetFilter>(l_asset_folder_absolute, this->out.updated_files, true);

		for (size_t i = 0; i < this->out.updated_files.Size; i++)
		{
			std::string l_relative_assetpath = std::string(this->out.updated_files[0].Memory.Memory).substr(l_asset_folder_absolute.End, this->out.updated_files[0].Memory.Size - 1);
			this->in.asset_server.insert_or_update_resource_fromfile(l_relative_assetpath);
		}

		this->state = State::ENDED;
	};

	inline void onEnded() override
	{

	};
};

#include "../Render/Render/Render.cpp"
#include "obj_reader.hpp"



struct CompileAssets : WorkerThread::Task
{
	struct In
	{
		AssetServerHandle asset_server;
	} in;

	struct Out
	{
		com::Vector<String<>> updated_files;

		inline void free()
		{
			for (size_t i = 0; i < this->updated_files.Size; i++)
			{
				this->updated_files[i].free();
			}
			this->updated_files.free();
		}

	} out;

	inline CompileAssets(AssetServerHandle p_asset_server_handle)
	{
		this->in.asset_server = p_asset_server_handle;
	};

	inline void free()
	{
		this->out.free();
	};

	inline void tick_internal() override
	{
		struct AssetFilter
		{
			inline static bool filter(const String<>& p_file)
			{
				size_t l_index;
				return p_file.find(".spv", 0, &l_index) || p_file.find(".obj", 0, &l_index);
			}
		};

		std::string l_folder_path = this->in.asset_server.get_asset_basepath();
		StringSlice l_asset_folder_absolute = StringSlice(l_folder_path.c_str());

	

		File::find_file<AssetFilter>(l_asset_folder_absolute, this->out.updated_files, true);
		for (size_t i = 0; i < this->out.updated_files.Size; i++)
		{
			// Assimp::Importer l_importer;
			// const aiScene* l_scene = l_importer.ReadFile(this->out.updated_files[i].c_str(), aiProcess_JoinIdenticalVertices);
			// const aiScene* l_scene = l_importer.ReadFileFromMemory(p_mesh_byes.Memory, p_mesh_byes.capacity_in_bytes(), 0);

			std::string l_relative_assetpath = std::string(this->out.updated_files[i].Memory.Memory).substr(l_asset_folder_absolute.End, this->out.updated_files[0].Memory.Size - 1);
			size_t tmp;
			if (this->out.updated_files[i].find(".obj", 0, &tmp))
			{
				RenderHeap2::Resource::MeshResourceAllocator::MeshAsset l_mesh_resource;
				com::Vector<char> l_mesh_resource_bytes;

				ObjReader::ReadObj(std::string(this->out.updated_files[i].c_str()), l_mesh_resource.vertices, l_mesh_resource.indices);

				l_mesh_resource.sertialize_to(l_mesh_resource_bytes);

				this->in.asset_server.insert_or_update_resource(l_relative_assetpath, l_mesh_resource_bytes);

				l_mesh_resource.free();
				l_mesh_resource_bytes.free();
			}
			else if (this->out.updated_files[i].find(".spv", 0, &tmp))
			{
				this->in.asset_server.insert_or_update_resource_fromfile(l_relative_assetpath);
			}
		}

		this->state = State::ENDED;
	}

	inline void onEnded() override
	{

	};
};

struct AssetDatabaseRefreshEditor
{
	QApplication* app;
	AssetServerHandle asset_server;
	WorkerThread worker_thread;

	struct MainWindowCallbacks
	{
		QTimer* asset_path_task_timer = nullptr;
		PushAssetsToDb* asset_path_task = nullptr;

		QTimer* asset_compile_task_timer = nullptr;
		CompileAssets* asset_compile_task = nullptr;

		AssetDatabaseRefreshEditor* editor = nullptr;

		MainWindowCallbacks()
		{
			this->asset_path_task_timer = nullptr;
			this->asset_path_task = nullptr;

			this->asset_compile_task_timer = nullptr;
			this->asset_compile_task = nullptr;

			this->editor = nullptr;
		};

		inline MainWindowCallbacks(AssetDatabaseRefreshEditor& p_editor)
		{
			this->editor = &p_editor;
		};

		inline void on_compile_clicked()
		{
			if (this->asset_path_task == nullptr && this->asset_compile_task == nullptr)
			{
				this->asset_compile_task = new CompileAssets(this->editor->asset_server);
				this->editor->worker_thread.push_task(this->asset_compile_task);

				this->asset_compile_task_timer = new QTimer();
				this->asset_compile_task_timer->setInterval(16);
				this->asset_compile_task_timer->start();


				this->editor->window.window.connect(this->asset_compile_task_timer, &QTimer::timeout, [=]() {
					if (this->asset_compile_task->state == PushAssetsToDb::State::ENDED)
					{
						this->asset_compile_task_timer->stop();
						this->editor->window.window.disconnect(this->asset_compile_task_timer);
						delete this->asset_compile_task_timer;
						this->asset_compile_task_timer = nullptr;


						for (size_t i = 0; i < this->asset_compile_task->out.updated_files.Size; i++)
						{
							this->editor->window.list.addItem(QString(this->asset_compile_task->out.updated_files[i].Memory.Memory));
						}

						this->asset_compile_task->free();
						delete this->asset_compile_task;
						this->asset_compile_task = nullptr;
					}
					});
			}
		}

		inline void on_pushtodatabase_clicked()
		{
			if (this->asset_path_task == nullptr && this->asset_compile_task == nullptr)
			{
				this->asset_path_task = new PushAssetsToDb(this->editor->asset_server);
				this->editor->worker_thread.push_task(this->asset_path_task);

				this->asset_path_task_timer = new QTimer();
				this->asset_path_task_timer->setInterval(16);
				this->asset_path_task_timer->start();


				this->editor->window.window.connect(this->asset_path_task_timer, &QTimer::timeout, [=]() {
					if (this->asset_path_task->state == PushAssetsToDb::State::ENDED)
					{
						this->asset_path_task_timer->stop();
						this->editor->window.window.disconnect(this->asset_path_task_timer);
						delete this->asset_path_task_timer;
						this->asset_path_task_timer = nullptr;


						for (size_t i = 0; i < this->asset_path_task->out.updated_files.Size; i++)
						{
							this->editor->window.list.addItem(QString(this->asset_path_task->out.updated_files[i].Memory.Memory));
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

	inline void allocate(QApplication& p_app, const std::string& p_executable_path)
	{
		this->app = &p_app;
		this->asset_server.allocate(p_executable_path);
		this->worker_thread.allocate();
		this->window.allocate(MainWindowCallbacks(*this));



		this->app->exec();
	}
};

int main(int argc, char** argv)
{
	QApplication l_app(argc, argv);
	AssetDatabaseRefreshEditor l_editor;

	l_editor.allocate(l_app, std::string(argv[0]));
}