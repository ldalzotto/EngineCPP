#pragma once

#include <string>
#include <Common/Thread/thread.hpp>
#include <Common/Container/vector.hpp>
#include <Common/File/file.hpp>
#include <Common/Container/tree.hpp>
#include <AssetServer/asset_server.hpp>

#include<iostream>
#include<fstream>
#include<sstream>

#include "../Render/Render/Render.cpp"
#include "obj_reader.hpp"

#define QT_NO_KEYWORDS 1

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qtreeview.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtCore/qtimer.h>

template<class Task, class OnTaskEnded>
struct QTAsyncTask
{
	QTimer* timer = nullptr;
	Task task;
	QObject* timer_parent = nullptr;
	OnTaskEnded on_ended;

	inline QTAsyncTask() {};

	inline void run(const Task& p_task, const OnTaskEnded& p_on_ended, WorkerThread& p_worker_thread, QObject& p_timer_parent_object)
	{
		if (this->timer == nullptr)
		{
			this->task = p_task;
			this->timer_parent = &p_timer_parent_object;
			this->on_ended = p_on_ended;

			p_worker_thread.push_task(&this->task);

			this->timer = new QTimer();
			this->timer->setInterval(16);
			this->timer->start();

			this->timer_parent->connect(this->timer, &QTimer::timeout, [=]() {
				if (this->task.state == Task::State::ENDED)
				{
					this->timer->stop();
					this->timer_parent->disconnect(this->timer);
					delete this->timer;
					this->timer = nullptr;

					// this->on_ended();
					this->on_ended.on_ended(this->task);

					this->task.free();
				}
				});
		}
	};
};

struct AssetSynchronizationPanel
{
	QTreeView tree_view;
	QStandardItemModel tree_view_model;
	com::Vector<QStandardItem*> tree_items;
	FileTree tree;

	inline void allocate()
	{
		// this->tree_view = QTreeView();
		this->tree_view.setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	};

	inline void free()
	{

	};

	inline void refresh_tree(FileTree& p_files)
	{
		this->free_tree();
		this->allocate_tree(p_files.move());
	};

private:
	inline void allocate_tree(FileTree& p_files)
	{
		this->tree_items.resize(p_files.Indices.Memory.Size);
		this->tree_items.Size = this->tree_items.Capacity;
		
		this->tree = p_files;

		
		for (size_t i = 0; i < this->tree_items.Size; i++)
		{
			this->tree_items[i] = new QStandardItem();
			this->tree_items[i]->setData(QVariant(i));
		}
		

		struct TreeForeach
		{
			AssetSynchronizationPanel* panel;
			inline TreeForeach() { };
			inline TreeForeach(AssetSynchronizationPanel* p_panel)
			{
				this->panel = p_panel;
			};
			inline void foreach(NTreeResolve<File<FilePathMemoryLayout::STRING>>& p_node, com::PoolToken<NTreeNode>& p_index)
			{
				if (p_node.node->has_parent())
				{
					QStandardItem* l_parent = this->panel->tree_items[p_node.node->parent];
					QStandardItem* l_child = this->panel->tree_items[p_index.Index];
					StringSlice l_filename = p_node.element->path.get_filename();
					l_child->setText(QString(l_filename.Memory + l_filename.Begin));
					l_parent->appendRow(l_child);
					// QStandardItem& l_parent = this->panel->tree_items[p_node.node->parent];
				}
			};
		};

		QStandardItem* l_parent_item = this->tree_view_model.invisibleRootItem();
		QStandardItem* l_child_item = this->tree_items[0];
		StringSlice l_filename = this->tree.resolve(com::PoolToken<NTreeNode>(0)).element->path.get_filename();
		l_child_item->setText(QString(l_filename.Memory + l_filename.Begin));
		l_parent_item->appendRow(l_child_item);

		this->tree.traverse(com::PoolToken<NTreeNode>(0), TreeForeach(this));

		this->tree_view.setModel(&this->tree_view_model);

	};

	inline void free_tree()
	{
		this->tree_items.free();
		this->tree_view_model.clear();

		this->tree.free();
	}
};

template<class Callbacks>
struct MainWindow
{
	QMainWindow window;
	Callbacks callbacks;

	AssetSynchronizationPanel asset_synch_panel;

	inline MainWindow() {}

	inline void allocate(const Callbacks& p_callbacks)
	{
		this->callbacks = p_callbacks;

		this->asset_synch_panel.allocate();

		QMenu* l_file_menu = this->window.menuBar()->addMenu(QString("&File"));

		QAction* l_compile_action = new QAction(QString("Compile"), &this->window);
		l_file_menu->addAction(l_compile_action);
		this->window.connect(l_compile_action, &QAction::triggered, [=]() {this->callbacks.on_compile_clicked(); });

		QAction* l_refresh_tree_action = new QAction(QString("Refresh"), &this->window);
		l_file_menu->addAction(l_refresh_tree_action);
		this->window.connect(l_refresh_tree_action, &QAction::triggered, [=]() {this->callbacks.on_refresh_clicked(); });


		this->window.setCentralWidget(&this->asset_synch_panel.tree_view);

		this->window.show();
	}

	inline void set_statusbarmessage(const std::string& p_message)
	{
		this->window.statusBar()->showMessage(QString(p_message.c_str()));
	};
};

struct RefreshAssetTree : WorkerThread::Task
{
	struct In
	{
		AssetServerHandle asset_server;
	} in;

	struct Out
	{
		FileTree file_tree;
	} out;

	inline RefreshAssetTree() {};
	inline RefreshAssetTree(AssetServerHandle p_asset_server_handle)
	{
		this->in.asset_server = p_asset_server_handle;
	};

	inline void tick_internal() override
	{
		std::string l_folder_path = this->in.asset_server.get_asset_basepath();
		StringSlice l_root_asset_folder_absolute = StringSlice(l_folder_path.c_str());

		File<> l_root_file;
		l_root_file.allocate(FileType::FOLDER, FilePath<FilePathMemoryLayout::SLICE>(l_root_asset_folder_absolute));
		{
			this->out.file_tree.allocate(l_root_file);
		}
		l_root_file.free();

		this->state = State::ENDED;
	}

	inline void onEnded() override
	{

	};

	inline void free()
	{

	}
};

struct CompileAssets : WorkerThread::Task
{
	struct In
	{
		AssetServerHandle asset_server;
		com::Vector<File<FilePathMemoryLayout::STRING>> involved_files;

		inline void free()
		{
			for (size_t i = 0; i < this->involved_files.Size; i++)
			{
				this->involved_files[i].free();
			}
			this->involved_files.free();
		}
	} in;

	struct Out
	{
		com::Vector<File<FilePathMemoryLayout::STRING>> updated_files;

		inline void free()
		{
			for (size_t i = 0; i < this->updated_files.Size; i++)
			{
				this->updated_files[i].free();
			}
			this->updated_files.free();
		}

	} out;

	inline CompileAssets() {};

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
		std::string l_folder_path = this->in.asset_server.get_asset_basepath();
		StringSlice l_root_asset_folder_absolute = StringSlice(l_folder_path.c_str());

		File<> l_root_file;
		l_root_file.allocate(FileType::FOLDER, FilePath<FilePathMemoryLayout::SLICE>(l_root_asset_folder_absolute));
#if 1
		{
			struct ForEachFile
			{
				Out* out;
				In* in;
				File<>* root_file;

				ForEachFile() {};
				ForEachFile(File<>& p_root_file, Out& p_out, In& p_in)
				{
					this->root_file = &p_root_file;
					this->out = &p_out;
					this->in = &p_in;
				};

				inline void foreach(File<FilePathMemoryLayout::STRING>& p_file, size_t p_depth)
				{
					StringSlice l_asset_folder_relative = StringSlice(p_file.path.path.c_str(), this->root_file->path.path.End, strlen(p_file.path.path.c_str()));

					if (p_file.type == FileType::CONTENT)
					{
						size_t tmp;
						if (p_file.extension.find(StringSlice("obj"), &tmp))
						{
							RenderHeap2::Resource::MeshResourceAllocator::MeshAsset l_mesh_resource;
							com::Vector<char> l_mesh_resource_bytes;

							ObjReader::ReadObj(std::string(p_file.path.path.c_str()), l_mesh_resource.vertices, l_mesh_resource.indices);

							l_mesh_resource.sertialize_to(l_mesh_resource_bytes);

							this->in->asset_server.insert_or_update_resource(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin), l_mesh_resource_bytes);

							l_mesh_resource.free();
							l_mesh_resource_bytes.free();

							// this->out->updated_files.push_back(p_file);
						}
						/*
						else if (p_file.extension.find(StringSlice("vert"), &tmp))
						{
							shaderc::Compiler compiler;
							shaderc::CompileOptions options;
							std::ifstream l_file_stream(p_file.path.path.c_str());
							std::ostringstream ss;
							ss << l_file_stream.rdbuf(); // reading data

							shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(ss.str(), shaderc_shader_kind::shaderc_glsl_vertex_shader, "vs", options);
							if (result.GetCompilationStatus() == shaderc_compilation_status_success) {
								com::Vector<uint32_t> l_compiled_shader;
								const uint32_t* l_it = result.begin() - 1;
								while (l_it != result.end())
								{
									l_compiled_shader.push_back(*l_it);
									l_it += 1;
								}
								com::Vector<char> l_compiled_shader_as_char;
								l_compiled_shader_as_char.Memory = (char*)l_compiled_shader.Memory;
								l_compiled_shader_as_char.Capacity = l_compiled_shader.Capacity * sizeof(uint32_t);
								l_compiled_shader_as_char.Size = l_compiled_shader.Size * sizeof(uint32_t);

								this->in->asset_server.insert_or_update_resource(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin), l_compiled_shader_as_char);
							}
							else
							{
								printf(result.GetErrorMessage().c_str());
							}

						}

						else if (p_file.extension.find(StringSlice("frag"), &tmp))
						{
							shaderc::Compiler compiler;
							shaderc::CompileOptions options;
							std::ifstream l_file_stream(p_file.path.path.c_str());
							std::ostringstream ss;
							ss << l_file_stream.rdbuf(); // reading data

							shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(ss.str(), shaderc_shader_kind::shaderc_fragment_shader, "fs", options);
							if (result.GetCompilationStatus() == shaderc_compilation_status_success) {
								com::Vector<uint32_t> l_compiled_shader;
								const uint32_t* l_it = result.begin() - 1;
								while (l_it != result.end())
								{
									l_compiled_shader.push_back(*l_it);
									l_it += 1;
								}
								com::Vector<char> l_compiled_shader_as_char;
								l_compiled_shader_as_char.Memory = (char*)l_compiled_shader.Memory;
								l_compiled_shader_as_char.Capacity = l_compiled_shader.Capacity * sizeof(uint32_t);
								l_compiled_shader_as_char.Size = l_compiled_shader.Size * sizeof(uint32_t);

								this->in->asset_server.insert_or_update_resource(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin), l_compiled_shader_as_char);
							}
							else
							{
								printf(result.GetErrorMessage().c_str());
							}
						}
						*/
						else if (p_file.extension.find(StringSlice("spv"), &tmp))
						{
							this->in->asset_server.insert_or_update_resource_fromfile(std::string(l_asset_folder_relative.Memory + l_asset_folder_relative.Begin));

							// this->out->updated_files.push_back(p_file);
						}
					}
				};
			};

			l_root_file.walk(true, ForEachFile(l_root_file, this->out, this->in));
		}
		l_root_file.free();
#endif

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

		struct OnCompileAssetsCompleted
		{
			MainWindow<MainWindowCallbacks>* window = nullptr;

			inline OnCompileAssetsCompleted() {};
			inline OnCompileAssetsCompleted(MainWindow<MainWindowCallbacks>& p_main_window) {
				this->window = &p_main_window;
			};

			inline void on_ended(CompileAssets& p_task)
			{
				/*
				for (size_t i = 0; i < p_task.out.updated_files.Size; i++)
				{
					this->window->list.addItem(QString(p_task.out.updated_files[i].path.path.Memory.Memory));
				}
				*/
			};
		};

		struct OnRefreshTreeCompleted
		{
			MainWindow<MainWindowCallbacks>* window = nullptr;

			inline OnRefreshTreeCompleted() {};
			inline OnRefreshTreeCompleted(MainWindow<MainWindowCallbacks>& p_main_window) {
				this->window = &p_main_window;
			};
			inline void on_ended(RefreshAssetTree& p_task)
			{
				this->window->asset_synch_panel.refresh_tree(p_task.out.file_tree);				
			};
		};

		QTAsyncTask<CompileAssets, OnCompileAssetsCompleted> compile_task;
		QTAsyncTask<RefreshAssetTree, OnRefreshTreeCompleted> refreshtree_task;

		AssetDatabaseRefreshEditor* editor = nullptr;

		MainWindowCallbacks()
		{
			this->editor = nullptr;
		};

		inline MainWindowCallbacks(AssetDatabaseRefreshEditor& p_editor)
		{
			this->editor = &p_editor;
		};

		inline void on_compile_clicked()
		{
			this->compile_task.run(CompileAssets(this->editor->asset_server), OnCompileAssetsCompleted(this->editor->window), this->editor->worker_thread, this->editor->window.window);
		}

		inline void on_refresh_clicked()
		{
			this->refreshtree_task.run(RefreshAssetTree(this->editor->asset_server), OnRefreshTreeCompleted(this->editor->window), this->editor->worker_thread, this->editor->window.window);
		}

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