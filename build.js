
const fs = require('fs');
const path = require('path');
const cp = require('child_process');

const program_start_date = Date.now();

const module_types = 
{
    header_lib: "header_lib",
    static_lib: "static_lib",
    executable: "executable"
};

const build_types = 
{
    debug: "debug",
    release_debug: "release_debug",
    release: "release"
};

let func_execute_command = (p_command) => {
    console.log("Executed command : " + p_command);
    cp.execSync(p_command, {stdio: 'inherit'});
};

let func_file_get_higher_modificationtime_recursive = function(p_file_name, out_modification_time)
{
    let l_file_stat = fs.statSync(p_file_name);

    if(l_file_stat.mtimeMs >= out_modification_time)
    {
        out_modification_time = l_file_stat.mtimeMs;
    }

    if(l_file_stat.isDirectory())
    {
        let l_sub_files = fs.readdirSync(p_file_name);
        for(let i=0;i<l_sub_files.length;i++)
        {
            out_modification_time = func_file_get_higher_modificationtime_recursive(path.join(p_file_name, l_sub_files[i]), out_modification_time);
        }
    }

    return out_modification_time;
};

let func_push_last_build_time_to_history = function(p_module_name, p_save_history)
{
    if(!p_save_history[p_module_name])
    {
        p_save_history[p_module_name] = {};
    }
    p_save_history[p_module_name].last_build_time = program_start_date;
};


let func_execute_module_build_command = (p_module_type, p_output_file, p_build_type, p_main_file, p_include_folders, p_linked_libs) => {

    let l_command_flags = "";
    let l_preprocessor_flags = "";
    let l_file_generation_flags = `/Fe"${p_output_file}" /Fo"${p_output_file}" `;
    
    switch(p_build_type)
    {
        case build_types.debug:
        {
            l_command_flags += "/Od ";
            l_command_flags += "/Zi ";
            l_preprocessor_flags += "/D CONFIGURATION_TYPE=0 ";
        }
        break;
        case build_types.release_debug:
        {
            l_command_flags += "/O2 ";
            l_command_flags += "/Zi ";
            l_preprocessor_flags += "/D CONFIGURATION_TYPE=1 ";
        }
        break;
        case build_types.release:
        {
            l_command_flags += "/O2 ";
            l_preprocessor_flags += "/D CONFIGURATION_TYPE=2 ";
        }
        break;
    }
    
    let l_indluce_folder_flag = "";
    for(let i=0;i<p_include_folders.length;i++)
    {
        l_indluce_folder_flag += `/I ${p_include_folders[i]} `;
    }

    let l_include_libs_flag = "";
    for(let i=0;i<p_linked_libs.length;i++)
    {
        l_include_libs_flag += `${p_linked_libs[i]} `;
    }
    
    
    if(p_module_type === module_types.executable)
    {
        let l_command = `cl ${l_command_flags} ${l_preprocessor_flags} ${l_file_generation_flags} ${l_indluce_folder_flag} ${p_main_file} ${l_include_libs_flag}`;
        func_execute_command(l_command);   
    }
    else if(p_module_type === module_types.static_lib)
    {
        let l_command = `cl /c /EHsc ${l_command_flags} ${l_preprocessor_flags} ${l_file_generation_flags} ${l_indluce_folder_flag} ${p_main_file} ${l_include_libs_flag}`;
        func_execute_command(l_command);
    }
};

let func_build_header_lib = (build_name_str, p_build_configurations, out_executable_dependencies_parameters) => 
{
    let l_build_configuration = p_build_configurations[build_name_str];
    
    if(l_build_configuration.module_type === module_types.header_lib)
    {
        if(!out_executable_dependencies_parameters[build_name_str])
        {
            let l_module_parameters = {};
                l_module_parameters.module_type = l_build_configuration.module_type;
                l_module_parameters.root_folder = l_build_configuration.root_folder;
                l_module_parameters.include_folders = l_build_configuration.include_folders;
                l_module_parameters.last_modification_time = func_file_get_higher_modificationtime_recursive(l_build_configuration.root_folder, 0);
                out_executable_dependencies_parameters[build_name_str] = l_module_parameters;
        }
    };
};

let func_build_static_lib_get_depedency_linked_folders_recursive = (p_build_configuration, p_build_configurations, out_include_folders) => {

    if(p_build_configuration.dependencies)
    {
        for(let i=0;i<p_build_configuration.dependencies.length;i++)
        {
            out_include_folders = func_build_static_lib_get_depedency_linked_folders_recursive(p_build_configurations[p_build_configuration.dependencies[i]], p_build_configurations, out_include_folders);
        }
    }
    return out_include_folders.concat(p_build_configuration.include_folders);
}

let func_build_static_lib_get_depedency_watched_folders_recursive = (p_build_configuration, p_build_configurations, out_root_folders) => {

    if(p_build_configuration.dependencies)
    {
        for(let i=0;i<p_build_configuration.dependencies.length;i++)
        {
            out_root_folders = func_build_static_lib_get_depedency_watched_folders_recursive(p_build_configurations[p_build_configuration.dependencies[i]], p_build_configurations, out_root_folders);
        }
    }
    return func_watchedfolder_add(p_build_configuration.module_type, p_build_configuration.root_folder, p_build_configuration.include_folders, out_root_folders);
}

let func_module_mustbe_rebuild = function(p_build_name, p_root_folders, p_save_history)
{
    if(p_save_history[p_build_name])
    {
        let l_modification_time = 0;
        
        for(let i=0;i<p_root_folders.length;i++)
        {
            l_modification_time = func_file_get_higher_modificationtime_recursive(p_root_folders[i], l_modification_time);
        }

        if(l_modification_time > p_save_history[p_build_name].last_build_time)
        {
            return true;
        }
        return false;
    }
    return true;
};

let func_build_static_lib = (build_name_str, p_build_configurations, out_executable_dependencies_parameters) => {
    
    let l_build_configuration = p_build_configurations[build_name_str];
    
    if(l_build_configuration.module_type === module_types.static_lib)
    {
        if(!out_executable_dependencies_parameters[build_name_str])
        {
            const l_output_lib = path.join(l_build_configuration.output_folder, l_build_configuration.name);

            let l_watched_folders_deps = func_build_static_lib_get_depedency_watched_folders_recursive(l_build_configuration, p_build_configurations, []);
            l_watched_folders_deps = l_watched_folders_deps.concat(l_build_configuration.root_folder);

            if(func_module_mustbe_rebuild(build_name_str, l_watched_folders_deps, p_build_configurations.Header.save_history))
            {
                const l_build_start_time = Date.now();

                //TODO -> build the .lib file
                let l_lib_include_folders = func_build_static_lib_get_depedency_linked_folders_recursive(l_build_configuration, p_build_configurations, []);
                func_execute_module_build_command(module_types.static_lib, l_output_lib, l_build_configuration.build_type, l_build_configuration.lib_main_file, l_lib_include_folders, []);
                let l_lib_command = `lib ${l_output_lib}.obj`;
                func_execute_command(l_lib_command);

                func_push_last_build_time_to_history(build_name_str, p_build_configurations.Header.save_history);

                console.log(`${build_name_str} : ${Date.now() - l_build_start_time} ms.`);
            }

            let l_module_parameters = {};
            l_module_parameters.module_type = l_build_configuration.module_type;
            l_module_parameters.root_folder = l_build_configuration.root_folder;
            l_module_parameters.include_folders = l_build_configuration.include_folders;
            l_module_parameters.static_lib_file = `${l_output_lib}.lib`;
            l_module_parameters.last_modification_time = func_file_get_higher_modificationtime_recursive(l_build_configuration.root_folder, 0);
            out_executable_dependencies_parameters[build_name_str] = l_module_parameters;
        }
    }

};

let func_build_dependencies_recursive = (p_build_configuration, p_build_configurations, p_build_type, out_executable_build_dependecy_parameters) => {

    if(p_build_configuration.dependencies)
    {
        for(let i=0;i<p_build_configuration.dependencies.length;i++)
        {
            let l_dependency_configuration = p_build_configurations[p_build_configuration.dependencies[i]];

            func_build_dependencies_recursive(l_dependency_configuration, p_build_configurations, p_build_type, out_executable_build_dependecy_parameters);

            switch(l_dependency_configuration.module_type)
            {
                case module_types.header_lib:
                    {
                        func_build_header_lib(p_build_configuration.dependencies[i], p_build_configurations, out_executable_build_dependecy_parameters);
                    }
                    break;
                case module_types.static_lib:
                    {
                        func_build_static_lib(p_build_configuration.dependencies[i], p_build_configurations, out_executable_build_dependecy_parameters);
                    }
                    break;
            }
        }
    }

}

let func_watchedfolder_add = (p_module_type, p_root_folder, p_include_folders, in_out_watched_folders) => {
    if(p_module_type == module_types.header_lib)
    {
        in_out_watched_folders = in_out_watched_folders.concat(p_root_folder);
    }
    else if(p_module_type == module_types.static_lib)
    {
        in_out_watched_folders = in_out_watched_folders.concat(p_include_folders);
    }
    return in_out_watched_folders;
}

let func_build_executable = (build_name_str, p_build_configurations) => {

    let l_build_configuration = p_build_configurations[build_name_str];

    let l_executable_build_dependecy_parameters = {};


    if(l_build_configuration.module_type === module_types.executable)
    {

        // building dependencies first
        func_build_dependencies_recursive(l_build_configuration, p_build_configurations, l_build_configuration.build_type, l_executable_build_dependecy_parameters);

        let l_include_folders = l_build_configuration.include_folders;
        let l_rebuild_watch_folders = [l_build_configuration.root_folder];
        let l_include_libs = l_build_configuration.include_libs;
        Object.keys(l_executable_build_dependecy_parameters).forEach(p_key => {
            let l_dependency_parameter = l_executable_build_dependecy_parameters[p_key];

            l_rebuild_watch_folders = func_watchedfolder_add(l_dependency_parameter.module_type, l_dependency_parameter.root_folder, l_dependency_parameter.include_folders, l_rebuild_watch_folders);

            l_include_folders = l_include_folders.concat(l_dependency_parameter.include_folders);
            if(l_dependency_parameter.static_lib_file)
            {
                l_include_libs = l_include_libs.concat(l_dependency_parameter.static_lib_file); 
            }
        });

        if(func_module_mustbe_rebuild(build_name_str, l_rebuild_watch_folders, p_build_configurations.Header.save_history))
        {
            const l_build_start_time = Date.now();
    
            if(!fs.existsSync(l_build_configuration.output_folder))
            {
                fs.mkdirSync(l_build_configuration.output_folder);
            }
        
            const l_output_executable = path.join(l_build_configuration.output_folder, l_build_configuration.name);
            func_execute_module_build_command(module_types.executable, l_output_executable, l_build_configuration.build_type, l_build_configuration.main_file, l_include_folders, l_include_libs);

            console.log(`${build_name_str} : ${Date.now() - l_build_start_time} ms.`);
    
            func_push_last_build_time_to_history(build_name_str, p_build_configurations.Header.save_history);
        }
    }
};

const root_folder = __dirname;
const build_folder = path.join(root_folder, "/out/");
const build_meta_folder = path.join(root_folder, ".build/");
const build_time_save_history_file = path.join(build_meta_folder, "history.json");

if (!fs.existsSync(build_meta_folder)) {
    fs.mkdirSync(build_meta_folder);
}
if (!fs.existsSync(build_time_save_history_file)) {
    fs.writeFileSync(build_time_save_history_file, "{}");
}

let build_time_save_history = JSON.parse(fs.readFileSync(build_time_save_history_file).toString());


const builds =
{
    Env: "Env",
    Common: "Common",
    Common2: "Common2",
    Math: "Math",
    Collision_release: "Collision_release",
    Collision_test_debug: "Collision_test_debug",
    Collision_test_release: "Collision_test_release",
    Common2_test_debug: "Common2_test_debug",
    // Common2_test_release: "Common2_test_release"
};

const build_configurations = {
    Header: {
        save_history_file_path: build_time_save_history_file,
        save_history: build_time_save_history
    },
    Env: {
        module_type: module_types.header_lib,
        include_folders: [
            path.join(root_folder, "/env")
        ],
        root_folder: path.join(root_folder, "/env"),
    },
    Common: {
        module_type: module_types.header_lib,
        include_folders: [ path.join(root_folder, "/Common") ],
        root_folder: path.join(root_folder, "/Common/Common"),
    },
    Common2: {
        module_type: module_types.header_lib,
        include_folders: [ path.join(root_folder, "/Rewrite/Common2") ],
        root_folder: path.join(root_folder, "/Rewrite/Common2"),
    },
    Math: {
        module_type: module_types.header_lib,
        include_folders: [ path.join(root_folder, "/Math/") ],
        root_folder: path.join(root_folder, "/Math/"),
        dependencies: [
            builds.Common
        ]
    },
    Collision_release: {
        module_type: module_types.static_lib,
        name: builds.Collision_release,
        build_type: build_types.release,
        output_folder: build_folder,
        include_folders: [ path.join(root_folder, "/Collision/interface") ],
        lib_main_file: path.join(root_folder, "/Collision/Collision/collision.cpp"),
        root_folder: path.join(root_folder, "/Collision/Collision"),
        dependencies: [
            builds.Common2,
            builds.Math
        ]
    },
    Collision_test_debug: 
    {
        module_type: module_types.executable,
        build_type: build_types.debug,
        name: builds.Collision_test_debug,
        output_folder: build_folder,
        include_folders: [],
        include_libs: [],
        dependencies: [
            builds.Env,
            builds.Collision_release
        ],
        root_folder: path.join(root_folder, "/Collision/test/"),
        main_file: path.join(root_folder, "/Collision/test/test.cpp")
    },
    Collision_test_release: 
    {
        module_type: module_types.executable,
        build_type: build_types.release,
        name: builds.Collision_test_release,
        output_folder: build_folder,
        include_folders: [],
        include_libs: [],
        dependencies: [
            builds.Env,
            builds.Collision_release
        ],
        root_folder: path.join(root_folder, "/Collision/test/"),
        main_file: path.join(root_folder, "/Collision/test/test.cpp")
    },
    Common2_test_debug:
    {
        module_type: module_types.executable,
        build_type: build_types.debug,
        name: builds.Common2_test_debug,
        output_folder: build_folder,
        include_folders: [],
        include_libs: [],
        dependencies: [
            builds.Env,
            builds.Common2
        ],
        root_folder: path.join(root_folder, "/Rewrite/Common2/test/"),
        main_file: path.join(root_folder, "/Rewrite/Common2/test/test.cpp")
    }
};

func_build_executable(builds.Collision_test_debug, build_configurations);

fs.writeFileSync(build_time_save_history_file, JSON.stringify(build_configurations.Header.save_history));