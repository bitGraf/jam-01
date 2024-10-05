# Set name of this target
set(target_name "jam-app")

message(----[${target_name}]-----------------------------------------)

# get all *.cpp files
file(GLOB_RECURSE SRC_CPP_FILES *.cpp)
file(GLOB_RECURSE SRC_C_FILES *.c)
set(SRC_FILES "${SRC_C_FILES};${SRC_CPP_FILES}")


# print all src files
message("Source Files:")
foreach(filename ${SRC_FILES})
    file(RELATIVE_PATH filename_trimmed ${CMAKE_SOURCE_DIR} ${filename})
    message("   > ${filename_trimmed}")
endforeach()

# Add executable
add_executable(${target_name} ${SRC_FILES})

# Include directories
target_include_directories(${target_name} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/deps/SDL2/include"
    "${CMAKE_SOURCE_DIR}/deps/nlohmann"
)

# Link libraries
target_link_directories(${target_name} PUBLIC "${CMAKE_SOURCE_DIR}/deps/SDL2/lib/x64")
target_link_libraries(${target_name} PUBLIC 
    utils
    laml
    SDL2.lib
    SDL2main.lib
    SDL2_image.lib
    SDL2_mixer.lib
    SDL2_ttf.lib
)

set(SDL_Libs
    SDL2.dll
    SDL2_image.dll
    SDL2_mixer.dll
    SDL2_ttf.dll)
foreach(filename ${SDL_Libs})
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/deps/SDL2/lib/x64/${filename}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/
    )
endforeach()