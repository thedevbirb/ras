#include "base_core.c"
#include "base_math.c"
#include "base_arena.c"
#include "base_thread_context.c"
#include "base_string.c"

#if OS_LINUX
# include "base_linux.c"
#elif OS_MAC
# include "base_macos.c"
#else
# error "operating system backend not found for base layer"
#endif
