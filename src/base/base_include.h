#ifndef BASE_INCLUDE_H
#define BASE_INCLUDE_H

#include "base_context_cracking.h"
#include "base_core.h"
#include "base_math.h"

#include "base_memory.h"
#include "base_arena.h"
#include "base_thread_context.h"
#include "base_string.h"

#if OS_LINUX
# include "base_linux.h"
#elif OS_MAC
# include "base_macos.h"
#else
# error "operating system backend not found for base layer"
#endif

#endif /* BASE_INCLUDE_H */
