# GLOB can cause issues in the CMake cache and IDEs
# See https://stackoverflow.com/questions/1027247/is-it-better-to-specify-source-files-with-glob-or-each-file-individually-in-cmak/1060061#1060061
file(GLOB_RECURSE project_src
        "*.h"
        "*.hpp"
        "*.cpp"
        "*.c"
)

# Set up filters in Visual Studio (not vscode)
foreach(_source IN ITEMS ${project_src})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel ${CMAKE_CURRENT_LIST_DIR} "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

add_executable (${PROJECT_NAME})

# Use target_sources() to add source dependencies
# You can repeat it as many times as you want in any subdir too!
target_sources(${PROJECT_NAME}
    PRIVATE
        #${project_src}
# [[[cog
#    import cog, os
#    file_paths = []
#    for root, dirs, files in os.walk('.'):
#        for file in files:
#            if file.endswith('.cpp') or file.endswith('.hpp') or file.endswith('.c') or file.endswith('.h'):
#                if "skeleton" in file: continue
#                file_path = os.path.relpath(os.path.join(root, file), '.')
#                file_paths.append(file_path)
#    file_paths.sort(key=lambda x: (x != 'main.cpp', x))
#    for file_path in file_paths:
#               cog.outl("\"%s\"" % file_path)
# ]]]
"main.cpp"
"common/overview.hpp"
"common/style.hpp"
"gui/gui.hpp"
"gui/nk_setup.hpp"
"gui/nuklear.h"
"gui/nuklear_sdl_renderer.h"
"gui/stb_image.h"
"gui/themes.hpp"
"io/portini.h"
"main.hpp"
"misc/color_funcs.hpp"
"tinyfd/tinyfiledialogs.c"
"tinyfd/tinyfiledialogs.h"
# [[[end]]]
# [[[
)

# Keep target_link_libraries() in the same place where you create targets
# (Not a requirement)
if(NO_EXTERNAL_LIBS)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        SDL2::SDL2
    )
else()
    # This downloads SDL2 directly to the binary dir
    # No need for external/!
    FetchContent_Declare(SDL2-static
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG 132b88749c284e4d51d93f1e08f5bb0ff9d705dc
        # Make sure we're building SDL2 statically!
        CMAKE_ARGS -DSDL_STATIC=ON
    )
    # Expose SDL to the project like a find_package()
    # Only possible because SDL uses CMake!
    FetchContent_MakeAvailable(SDL2-static)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        SDL2-static
    )
endif()

# Without the trailing / after assets, it would copy the directory instead of its contents
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/assets/ DESTINATION ${CMAKE_SOURCE_DIR}/bin)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
