cmake_minimum_required(VERSION 3.10)
project(puzzle_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# macOS固有の設定
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")  # macOS Mojave以降をサポート
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")  # Intel/Apple Silicon両対応
endif()

include_directories(include)

add_library(glad src/gl.c)
target_include_directories(glad PUBLIC include)

file(GLOB SRC_FILES src/*.cpp src/*.c)
add_executable(game ${SRC_FILES})

# macOS用の実行可能ファイル設定
if(APPLE)
    set_target_properties(game PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.puzzlegame
        MACOSX_BUNDLE_BUNDLE_VERSION 1.0
        MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    )
endif()

# クロスプラットフォーム対応のライブラリリンク
if(WIN32)
    target_link_libraries(game glad glfw3 opengl32)
elseif(APPLE)
    # macOS用のライブラリリンク
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    target_link_libraries(game glad glfw OpenGL::GL)
    
    # macOSのフレームワーク
    target_link_libraries(game 
        "-framework OpenGL" 
        "-framework Cocoa" 
        "-framework IOKit" 
        "-framework CoreVideo"
    )
elseif(UNIX)
    # Linux用のライブラリリンク
    target_link_libraries(game glad glfw GL dl pthread)
endif()

# リソースファイルを実行ファイルと同じディレクトリにコピー
file(COPY img DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY font DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})