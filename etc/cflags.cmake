set (CMAKE_CXX_STANDARD 20)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SANITIZING_FLAGS -fno-sanitize-recover=all -fsanitize=undefined -fsanitize=address)

# ask for more warnings from the compiler
set (CMAKE_BASE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
# -Wno-unqualified-std-cast-call
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wextra -Weffc++ -Werror -Wshadow -Wpointer-arith -Wcast-qual -Wformat=2 -Wno-non-virtual-dtor")
