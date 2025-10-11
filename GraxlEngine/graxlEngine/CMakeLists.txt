# CMakeList.txt : CMake project for GrashCpp, include source and define
# project specific logic here.
#

# Add source to this project's executable.

# -------------- Fetch all libarys --------------

include(FetchContent)
FetchContent_Declare(
	raylib
	GIT_REPOSITORY https://github.com/raysan5/raylib.git
	GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

FETCHCONTENT_DECLARE(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG  v1.15.3
)
FETCHCONTENT_MAKEAVAILABLE(spdlog)

FETCHCONTENT_DECLARE(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.92.1
)
FETCHCONTENT_MAKEAVAILABLE(imgui)

FETCHCONTENT_DECLARE(
	magic_enum
	GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
	GIT_TAG v0.9.7
)
FETCHCONTENT_MAKEAVAILABLE(magic_enum)

add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_demo.cpp
)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

add_subdirectory("lib/rlImGui")





# -------- Bulding GraxlEngine library --------

# Find all engine src files 
file(GLOB_RECURSE GRAXL_ENGINE_SOURCES "src/*.cpp")
file(GLOB_RECURSE GRAXL_ENGINE_HEADERS "include/*.h")

# Create the engine library
add_library(graxlEngine STATIC ${GRAXL_ENGINE_SOURCES} ${GRAXL_ENGINE_HEADERS})
target_compile_features(graxlEngine PRIVATE cxx_std_20)

target_include_directories(graxlEngine PUBLIC "include/")

target_link_libraries(graxlEngine PUBLIC raylib)
target_link_libraries(graxlEngine PRIVATE spdlog::spdlog)
target_link_libraries(graxlEngine PRIVATE imgui)
target_link_libraries(graxlEngine PRIVATE rlImGui)
target_link_libraries(graxlEngine PRIVATE magic_enum)

# TODO: Add tests and install targets if needed.
