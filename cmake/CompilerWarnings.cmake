# Applies a consistent, reasonably strict warning set to a target across the
# compilers contributors are likely to use (GCC, Clang, MSVC).
function(apply_project_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
        )
    endif()
endfunction()
