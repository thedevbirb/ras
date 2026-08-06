#ifndef CORE_INCLUDE_H
#define CORE_INCLUDE_H

// Core primitives of the codebase. Depends only on `base`.
// Ideally this contains backend-agnostic logic-only, in practice this is a best effort.
// While there aren't strict dependencies on RISCV-related logic, there is some dependency on ELF contents.

#include "core_initialize.h"
#include "core_options.h"
#include "core_token.h"
#include "core_source.h"
#include "core_diagnostic.h"

#include "core_ir.h"

#endif // CORE_INCLUDE_H

