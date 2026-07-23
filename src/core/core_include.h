#ifndef CORE_INCLUDE_H
#define CORE_INCLUDE_H

// Core primitives of the codebase. Depends only on `base`.
// Ideally this contains backend-agnostic logic-only, in practice this is a best effort.
// While there aren't strict dependencies on RISCV-related logic, there is some dependency on ELF contents.

#include "core_initialize.h"
#include "core_token.h"
#include "core_source.h"
#include "core_diagnostic.h"

// TODO(low, refactor) These all depend on each other in practice, and they need forward declaration.
// I don't know in practice how to untagle this.
#include "core_utils.h"
#include "core_fragment.h"
#include "core_fixup.h"
#include "core_section.h"
#include "core_expression.h"
#include "core_symbol.h"

#endif // CORE_INCLUDE_H

