#define KNOB_IMPLEMENTATION
#include "knob.h"

static const char *raylib_modules[] = {
    "rcore",
    "raudio",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};

#ifdef _WIN32
#define LIB_EXT ".lib"
#define ZIG_PATH ".\\Tools\\FoundryTools_windows_x64\\"
#else
#define ZIG_PATH "./Tools/linux_x64/"
#define LIB_EXT ".a"
#endif

#define RAYLIB_PATH "."PATH_SEP"Libraries"PATH_SEP"raylib"
#define RAYLIB_BUILD_PATH "."PATH_SEP"build"PATH_SEP"raylib"
bool build_raylib(Knob_File_Paths* end_cmd,Knob_Config* config)
{
    bool result = true;
    Knob_Cmd cmd = {0};

    Knob_Procs procs = {0};
    char* compiler = knob_temp_sprintf("%s%s",ZIG_PATH,GET_COMPILER_NAME(config->compiler));
    for (size_t i = 0; i < KNOB_ARRAY_LEN(raylib_modules); ++i) {
        const char *input_path = knob_temp_sprintf(RAYLIB_PATH PATH_SEP"src"PATH_SEP"%s.c", raylib_modules[i]);
        const char *output_path = knob_temp_sprintf("%s"PATH_SEP"%s.o", RAYLIB_BUILD_PATH, raylib_modules[i]);
        output_path = knob_temp_sprintf("%s"PATH_SEP"%s.o", RAYLIB_BUILD_PATH, raylib_modules[i]);
        knob_cmd_append(end_cmd,output_path);

        if (knob_needs_rebuild(output_path, &input_path, 1)) {
            cmd.count = 0;
            knob_cmd_append(&cmd, compiler,compiler_names[config->compiler][1]);
            knob_cmd_append(&cmd, "--debug", "-std=c11", "-fno-sanitize=undefined","-fno-omit-frame-pointer");
            knob_cmd_append(&cmd, "-target");
            #ifdef _WIN32
            knob_cmd_append(&cmd, "x86_64-windows");
            #endif
            #ifdef __linux__
            knob_cmd_append(&cmd, "x86_64-linux-gnu");
            knob_cmd_append(&cmd, "-D_GLFW_X11");
            knob_cmd_append(&cmd, "-lX11");
            knob_cmd_append(&cmd, "-I/usr/include");
            knob_cmd_append(&cmd, "-L/usr/lib");
            #endif
            knob_cmd_append(&cmd, "-DPLATFORM_DESKTOP","-fPIC");
            knob_cmd_append(&cmd, "-I"RAYLIB_PATH"/src/external/glfw/include");
            knob_cmd_append(&cmd, "-I"RAYLIB_PATH"/src");
            knob_cmd_append(&cmd, "-c", input_path);
            knob_cmd_append(&cmd, "-o", output_path);

            Knob_Proc proc = knob_cmd_run_async(cmd,NULL,NULL);
            knob_da_append(&procs, proc);
        }
    }
    cmd.count = 0;

    if (!knob_procs_wait(procs)) knob_return_defer(false);

defer:
    knob_cmd_free(cmd);
    return result;
}

#define IMGUI_PATH "Libraries"PATH_SEP"imgui"
bool build_rlImGui(Knob_File_Paths* end_cmd,Knob_Config* config)
{
    bool result = true;
    Knob_Cmd cmd = {0};

    Knob_Config conf = {0};
    knob_config_init(&conf);
    conf.compiler = config->compiler;
    conf.compiler_path = config->compiler_path;
    conf.debug_mode = config->debug_mode;

    Knob_File_Paths files = {0};
    knob_da_mult_append(&files,
        //UI
        IMGUI_PATH"/imgui.cpp",
        IMGUI_PATH"/imgui_draw.cpp",
        IMGUI_PATH"/imgui_tables.cpp",
        IMGUI_PATH"/imgui_widgets.cpp",
        IMGUI_PATH"/imgui_demo.cpp",
        //------------------------------
        "Libraries/rlImGui/rlImGui.cpp",
    );
    knob_config_add_files(&conf,files.items,files.count);
    files.count = 0;

    knob_da_mult_append(&files,
        RAYLIB_PATH"/src/external/glfw/include",
        RAYLIB_PATH"/src",
        IMGUI_PATH,
        "Libraries/rlImGui",
    );
    knob_config_add_includes(&conf,files.items,files.count);
    files.count = 0;

    knob_config_add_define(&conf,"-fPIC");

    conf.build_to = RAYLIB_BUILD_PATH;
    knob_config_build(&conf,&files);
    for(int i =0; i < files.count; ++i){
        const char* output_path = files.items[i];
        knob_da_append(end_cmd,output_path);
    }

defer:
    knob_cmd_free(cmd);
    return result;
}

MAIN(bd_base){

    if(!knob_mkdir_if_not_exists("build")){ return 1;}
    if(!knob_mkdir_if_not_exists("Deployment")){ return 1;}
    if(!knob_mkdir_if_not_exists("build/sqlite")){ return 1;}
    // if(!knob_mkdir_if_not_exists(RAYLIB_BUILD_PATH)) { return 1; }
    // if(!knob_mkdir_if_not_exists(RAYLIB_BUILD_PATH PATH_SEP "Libraries")) { return 1; }
    // if(!knob_mkdir_if_not_exists(RAYLIB_BUILD_PATH PATH_SEP "imgui")) { return 1; }
    // if(!knob_mkdir_if_not_exists(RAYLIB_BUILD_PATH PATH_SEP "Libraries/rlImGui")) { return 1; }

    if(knob_file_exists("Deployment/imgui.ini")){
        knob_file_del("Deployment/imgui.ini");
    }
    Knob_Cmd cmd = {0};
    Knob_Config config = {0};
    knob_config_init(&config);
    config.compiler = COMPILER_ZIG;
    config.compiler_path = ZIG_PATH;

    const char* program = knob_shift_args(&argc,&argv);
    char* subcommand = NULL;
    if(argc <= 0){
        subcommand = "build";
    } else {
        subcommand = (char*)knob_shift_args(&argc,&argv);
    }
    if(knob_cstr_match(subcommand,"clean")){
        // knob_clean_dir("."PATH_SEP"build");
    }
    if(knob_cstr_match(subcommand,"build")){

    }


    Knob_File_Paths o_files = {0};
    // if(!build_raylib(&o_files,&config))return 1;
    // if(!build_rlImGui(&o_files,&config))return 1;

    Knob_File_Paths files = {0};
    knob_da_mult_append(&files,
        "src",
        "./sqlite/sqlite3.c"
    );
    config.build_to = "."PATH_SEP"build";
    knob_config_add_files(&config,files.items,files.count);
    files.count = 0;
    knob_da_mult_append(&files,
        ".",
        "./src",
        "./sqlite"
        // "./Libraries/raylib/src",
        // "./Libraries/imgui",
        // "./Libraries/rlImGui",
    );
    knob_config_add_includes(&config,files.items,files.count);
    Knob_File_Paths out_files = {0};
    if(!knob_config_build(&config,&out_files))return 1;

    cmd.count = 0;

    knob_cmd_add_compiler(&cmd,&config);
    knob_cmd_add_includes(&cmd,&config);
    for(int i =0; i < o_files.count;++i){
        knob_cmd_append(&cmd,o_files.items[i]);
    }
    for(int i =0; i < out_files.count;++i){
        knob_cmd_append(&cmd,out_files.items[i]);
    }

    knob_cmd_append(&cmd,"-o","./Deployment/bd.com");
    for(int i =0; i < config.cpp_flags.count;++i){
        knob_cmd_append(&cmd,config.cpp_flags.items[i]);
    }
    knob_cmd_append(&cmd,"-lm");
    knob_cmd_append(&cmd,"-lstdc++");
    if(config.target == TARGET_WIN64_MINGW || config.target == TARGET_WIN64_MSVC){
        knob_cmd_append(&cmd,"-lgdi32");
        knob_cmd_append(&cmd,"-lwinmm");
    }
    Knob_String_Builder render = {0};
    knob_cmd_render(cmd,&render);
    knob_log(KNOB_INFO,"CMD: %s",render.items);
    if(!knob_cmd_run_sync(cmd)) return 1;

    return 0;
}