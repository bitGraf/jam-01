# Set name of this target
set(target_name "utils")

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

# Create static library
add_library(${target_name} STATIC ${SRC_FILES})

# Include directories
target_include_directories(${target_name} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/deps/SDL2/include"
)

# Link libraries
# target_link_directories(${target_name} PUBLIC path/to/libraries)
target_link_libraries(${target_name} PUBLIC 
    laml)