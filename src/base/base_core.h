#ifndef BASE_CORE_H
#define BASE_CORE_H

#include <stddef.h>
#include <stdint.h>


//------------------------------------------------------------------------------
// Keywords
//------------------------------------------------------------------------------

#define internal      static
#define global        static
#define local_persist static

/* Decleare thread local storage (TLS).
 * For example, checkout: https://gcc.gnu.org/onlinedocs/gcc/Thread-Local.html */
#if COMPILER_MSVC
#define thread_local __declspec(thread)
#define thread_local_static static __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
#define thread_local __thread
#define thread_local_static static __thread
#else
#error thread_local not defined for this compiler.
#endif

//------------------------------------------------------------------------------
// Helper macros
//------------------------------------------------------------------------------

#define stringify__m(s) #s
#define stringify_m(s) stringify__m(s)

#define glue__m(a,b) a##b
#define glue_m(a,b) glue__m(a,b)

// `sizeof` on a fixed-size array returns the total bytes used.
#define array_count_m(a) sizeof((a)) / sizeof((a)[0])
#define struct_field_size_m(type, member) (sizeof(((type *)0)->member))

#define unused_m(x) (void)(x);
#define swap_m(T, a, b) do { T __t__ = a; a = b; b = __t__; } while (0)

// Is the given value a sign-extended 32-bit value?
//
// Let M = (S64)0x7fffffff = 0x000000007FFFFFFF (bits 0–30 set). Then ~M = 0xFFFFFFFF80000000 (bits 31–63 set).
// The expression x & ~M keeps only bits 31–63 and discards the lower 31.
//
// Two acceptable cases remain:
//
// x & ~M == 0  - bits 31–63 are all zero. x is in [0, 0x7FFFFFFF]: a non-negative 32-bit value, zero-extended (which is
// also valid sign-extension for non-negatives).
// x & ~M == ~M - bits 31–63 are all ones. x is in [0xFFFFFFFF80000000, 0xFFFFFFFFFFFFFFFF] = [-2^31, -1]: a negative
// 32-bit value, sign-extended.
#define sign_extended_32_bit_is_m(x) (((x) &~ (S64)0x7fffffff) == 0 || (((x) &~ (S64)0x7fffffff) == ~(S64)0x7fffffff))

// Is the given value a zero-extended 32-bit value?  Or a negated one?
#define zero_extended_32_bit_is_m(x) (((x) &~ (S64)0xffffffff) == 0 || (((x) &~ (S64)0xffffffff) == ~(S64)0xffffffff))

// Compiler-exposed typeof.

#if COMPILER_CLANG || COMPILER_GCC
# define typeof_m(expr) __typeof__(expr)
#elif COMPILER_MSVC && _MSC_VER >= 1938
# define typeof_m(expr) __typeof__(expr)
#endif

#ifdef typeof_m
#define struct_field_typeof_m(T, member) (typeof((T){0}.member))
#endif


// Assertions

#if COMPILER_MSVC
# define trap_m() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
# define trap_m() __builtin_trap()
#else
# error "unknown trap intrinsic for this compiler"
#endif

#if defined(stderr)
#define print_assert_failed_m(x) fprintf(stderr, "ASSERT_FAILED at %s:%d: %s\n", __FILE__, __LINE__, stringify_m(x))
#else
#define print_assert_failed_m(x) (void)0
#endif

#define assert_always_m(x)               \
do                                       \
{                                        \
	if(!(x))                         \
	{                                \
		print_assert_failed_m(x);\
		trap_m();                \
	}                                \
} while(0)

#if defined(BUILD_DEBUG)
# define assert_m(x) assert_always_m(x)
#else
# define assert_m(x) (void)(x)
#endif

#define todo_m()        assert_always_m(0 && "todo");
#define unreachable_m() assert_always_m(0 && "unreachable");

#define assert_static_m(c, id) typedef int glue_m(id, __LINE__)[(c) ? 1 : -1]

//------------------------------------------------------------------------------
// Base types
//------------------------------------------------------------------------------

// Some debuggers (e.g. CLion) have more ergonomic displaying of bytes when typedef'd
// from an `unsigned char` rather than a `uint8_t`.
#ifdef U8_AS_UNSIGNED_CHAR
typedef unsigned char U8;
#else
typedef uint8_t  U8;
#endif
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;
typedef S8       B8;
typedef S16      B16;
typedef S32      B32;
typedef S64      B64;
typedef float    F32;
typedef double   F64;


internal B32 U8__octal_prefix(U8 byte)         { B32 result = '0' <= byte && byte < '4'; return result; }
internal B32 U8__octal(U8 byte)                { B32 result = '0' <= byte && byte < '7'; return result; }
internal B32 U8__ascii_digit_is(U8 byte)       { B32 result = '0' <= byte && byte < '9'; return result; }
internal B32 U8__ascii_lower_is(U8 character)  { B32 result = character >= 'a' && character <= 'z'; return result; }
internal B32 U8__ascii_upper_is(U8 character)  { B32 result = character >= 'A' && character <= 'Z'; return result; }
internal B32 U8__ascii_letter_is(U8 character) { B32 result = U8__ascii_lower_is(character) || U8__ascii_upper_is(character); return result; }


// Safe casts

internal  U8  U8_cast_safe(U16 x);
internal U16 U16_cast_safe(U32 x);
internal U32 U32_cast_safe(U64 x);
internal S32 S32_cast_safe(S64 x);

internal S64 S64_from_U64_cast_safe(U64 x);
internal U64 U64_from_S64_cast_safe(S64 x);

// Bit-patterns

internal B32 S64_bits_range_in(S64 signed_integer, U8 bits);

internal U64 count_bits_set32(U32 val);
internal U64 count_bits_set64(U64 val);

internal U64 ctz32(U32 val);
internal U64 ctz64(U64 val);
internal U64 clz32(U32 val);
internal U64 clz64(U64 val);

// Return the 0-index
internal U64 msb64(U64 val);
internal U64 msb32(U64 val);

internal U8 count_trailing_zeros(U64 x);
internal U8 count_leading_zeros(U64 x);

// Pointer casts

#define U8_pointer_m(pointer)       (U8 *)(void *)pointer
#define U8_pointer_const_m(pointer) (const U8 *)(const void *)pointer

// Units

#define KiB(n)  (((U64)(n)) << 10)
#define MiB(n)  (((U64)(n)) << 20)
#define GiB(n)  (((U64)(n)) << 30)
#define TiB(n)  (((U64)(n)) << 40)

#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000)

// Basic constants

#define U64_max (U64)0xffffffffffffffffull
#define U32_max (U32)0xffffffff
#define U16_max (U16)0xffff
#define U8_max  (U8)0xff

#define S64_max (S64)0x7fffffffffffffffll
#define S32_max (S32)0x7fffffff
#define S16_max (S16)0x7fff
#define S8_max   (S8)0x7f

#define S64_min (S64)0x8000000000000000ll
#define S32_min (S32)0x80000000
#define S16_min (S16)0x8000
#define S8_min   (S8)0x80


//------------------------------------------------------------------------------
// Type -> Alignment
//------------------------------------------------------------------------------

#if COMPILER_MSVC
# define cc_align_of(T) __alignof(T)
#elif COMPILER_CLANG
# define cc_align_of(T) __alignof(T)
#elif COMPILER_GCC
# define cc_align_of(T) __alignof__(T)
#else
# error cc_align_of not defined for this compiler.
#endif

#if COMPILER_MSVC
# define cc_align_type(x) __declspec(align(x))
#elif COMPILER_CLANG || COMPILER_GCC
# define cc_align_type(x) __attribute__((aligned(x)))
#else
# error cc_align_type not defined for this compiler.
#endif

//------------------------------------------------------------------------------
// Linkage Keyword Macros
//------------------------------------------------------------------------------

#if OS_WINDOWS
# define shared_function C_LINKAGE __declspec(DLLexport)
#else
# define shared_function C_LINKAGE
#endif

#if LANG_CPP
# define C_LINKAGE_BEGIN extern "C"{
# define C_LINKAGE_END }
# define C_LINKAGE extern "C"
#else
# define C_LINKAGE_BEGIN
# define C_LINKAGE_END
# define C_LINKAGE
#endif

//------------------------------------------------------------------------------
// Linked List Building Macros
//------------------------------------------------------------------------------

// Intrusive linked-list helpers for doubly-linked lists (DLL),
// singly-linked doubly-headed list (queues, SLL_queue), and
// singly-linked singly-headed list (stacks, SLL_Stack).
//
// Naming suffixes (N, P, Z):
//   N  — name of the "next"     member in the node struct
//   P  — name of the "previous" member in the node struct
//   Z  — nil / zero sentinel value (the null pointer representation)
//
//   _npz_m — accepts explicit next (N), previous (P) member names, and sentinel (Z)
//   _np_m  — accepts explicit next (N) and previous (P) member names (sentinel = 0)
//   _nz_m  — accepts explicit next (N) member name and sentinel (Z)
//   _n_m   — accepts explicit next (N) member name (sentinel = 0)
//   _m     — simplest form: sentinel = 0, member names default to next/previous

#define nil_check_m(sentinel, pointer) ((pointer) == 0 || (pointer) == sentinel)
#define nil_set_m(sentinel, pointer)   ((pointer) = sentinel)

// -- Doubly-linked lists -----------------------------------------------------

// Join two separate nodes
#define DLL_join_npz_m(sentinel, before, after, next, previous)                         \
        (nil_check_m(sentinel, before)                                                  \
         ? (nil_check_m(sentinel, after)                                                \
                 ? (0)                                                                  \
                 : ((after)->previous = before))                                        \
         : (nil_check_m(sentinel, after)                                                \
                 ? ((before)->next = after)                                             \
                 : ((before)->next = after, after->previous = before)))

#define DLL_insert_npz_m(sentinel, first, last, position, node, next, previous)          \
    (nil_check_m(sentinel, first)                                                        \
        ? ((first) = (last) = (node),                                                    \
           nil_set_m(sentinel, (node)->next),                                            \
           nil_set_m(sentinel, (node)->previous))                                        \
        : nil_check_m(sentinel, position)                                                \
            ? ((node)->next = (first),                                                   \
               (first)->previous = (node),                                               \
               (first) = (node),                                                         \
               nil_set_m(sentinel, (node)->previous))                                    \
            : ((position) == (last))                                                     \
                ? ((last)->next = (node),                                                \
                   (node)->previous = (last),                                            \
                   (last) = (node),                                                      \
                   nil_set_m(sentinel, (node)->next))                                    \
                : (nil_check_m(sentinel, (position)->next)                               \
                       ? (0)                                                             \
                       : ((position)->next->previous = (node)),                          \
                   (node)->next = (position)->next,                                      \
                   (position)->next = (node),                                            \
                   (node)->previous = (position)))

#define DLL_push_back_npz_m(sentinel, first, last, node, next, previous) \
    DLL_insert_npz_m(sentinel, first, last, last, node, next, previous)

#define DLL_push_front_npz_m(sentinel, first, last, node, next, previous) \
    DLL_insert_npz_m(sentinel, last, first, first, node, previous, next)

#define DLL_remove_npz_m(sentinel, first, last, node, next, previous)       \
    (((node) == (first) ? (first) = (node)->next : (0)),                    \
     ((node) == (last)  ? (last)  = (last)->previous  : (0)),               \
     (nil_check_m(sentinel, (node)->previous)                               \
         ? (0)                                                              \
         : ((node)->previous->next = (node)->next)),                        \
     (nil_check_m(sentinel, (node)->next)                                   \
         ? (0)                                                              \
         : ((node)->next->previous = (node)->previous)))

// -- Doubly-linked lists — field-name variant (nil = 0) --------------------

#define DLL_insert_np_m(first, last, position, node, next, previous) \
    DLL_insert_npz_m(0, first, last, position, node, next, previous)

#define DLL_push_back_np_m(first, last, node, next, previous) \
    DLL_push_back_npz_m(0, first, last, node, next, previous)

#define DLL_push_front_np_m(first, last, node, next, previous) \
    DLL_push_front_npz_m(0, first, last, node, next, previous)

#define DLL_remove_np_m(first, last, node, next, previous) \
    DLL_remove_npz_m(0, first, last, node, next, previous)

// -- Doubly-linked lists — sentinel (members = next/previous) -------------------

#define DLL_insert_z_m(sentinel, first, last, position, node) \
    DLL_insert_npz_m(sentinel, first, last, position, node, next, previous)

#define DLL_push_back_z_m(sentinel, first, last, node, next) \
    DLL_push_back_npz_m(sentinel, first, last, node, next, previous)

#define DLL_push_front_z_m(sentinel, first, last, node, next) \
    DLL_push_front_npz_m(sentinel, first, last, node, next, previous)

#define DLL_remove_z_m(sentinel, first, last, node, next) \
    DLL_remove_npz_m(sentinel, first, last, node, next, previous)

// -- Doubly-linked lists — simple variant (nil = 0, members = next/previous) ----

#define DLL_insert_m(first, last, position, node) \
    DLL_insert_npz_m(0, first, last, position, node, next, previous)

#define DLL_push_back_m(first, last, node) \
    DLL_push_back_npz_m(0, first, last, node, next, previous)

#define DLL_push_front_m(first, last, node) \
    DLL_push_front_npz_m(0, first, last, node, next, previous)

#define DLL_remove_m(first, last, node) \
    DLL_remove_npz_m(0, first, last, node, next, previous)

// -- Singly-linked queues (doubly-headed) — sentinel variant ----------------

#define SLL_queue_push_nz_m(sentinel, first, last, node, next)         \
    (nil_check_m(sentinel, first)                                      \
        ? ((first) = (last) = (node),                                  \
           nil_set_m(sentinel, (node)->next))                          \
        : ((last)->next = (node),                                      \
           (last) = (node),                                            \
           nil_set_m(sentinel, (node)->next)))

#define SLL_queue_push_front_nz_m(sentinel, first, last, node, next)         \
    (nil_check_m(sentinel, first)                                            \
        ? ((first) = (last) = (node),                                        \
           nil_set_m(sentinel, (node)->next))                                \
        : ((node)->next = (first),                                           \
           (first) = (node)))

#define SLL_queue_pop_nz_m(sentinel, first, last, next)        \
    ((first) == (last)                                         \
        ? (nil_set_m(sentinel, first),                         \
           nil_set_m(sentinel, last))                          \
        : ((first) = (first)->next))

// -- Singly-linked queues — field-name variant (nil = 0) -------------------

#define SLL_queue_push_n_m(first, last, node, next) \
    SLL_queue_push_nz_m(0, first, last, node, next)

#define SLL_queue_push_front_n_m(first, last, node, next) \
    SLL_queue_push_front_nz_m(0, first, last, node, next)

#define SLL_queue_pop_n_m(first, last, next) \
    SLL_queue_pop_nz_m(0, first, last, next)

// -- Singly-linked queues — sentinel variant (member = next) ---------

#define SLL_queue_push_z_m(sentinel, first, last, node) \
    SLL_queue_push_nz_m(sentinel, first, last, node, next)

#define SLL_queue_push_front_z_m(sentinel, first, last, node) \
    SLL_queue_push_front_nz_m(sentinel, first, last, node, next)

#define SLL_queue_pop_z_m(sentinel, first, last) \
    SLL_queue_pop_nz_m(sentinel, first, last, next)

// -- Singly-linked queues — simple variant (nil = 0, member = next) ---------

#define SLL_queue_push_m(first, last, node) \
    SLL_queue_push_nz_m(0, first, last, node, next)

#define SLL_queue_push_front_m(first, last, node) \
    SLL_queue_push_front_nz_m(0, first, last, node, next)

#define SLL_queue_pop_m(first, last) \
    SLL_queue_pop_nz_m(0, first, last, next)

// -- Singly-linked stacks (singly-headed) — sentinel variant -----------------

#define SLL_stack_push_nz_m(sentinel, first, node, next)    \
    (nil_check_m(sentinel, first)                           \
        ? ((first) = (node),                                \
           nil_set_m(sentinel, (node)->next))               \
        : ((node)->next = (first),                          \
           (first) = (node)))

#define SLL_stack_pop_nz_m(sentinel, first, next)  \
    (nil_check_m(sentinel, first)                  \
        ? (0)                                      \
        : ((first) = (first)->next))

// -- Singly-linked stacks — field-name variant (sentinel = 0) ---------------

#define SLL_stack_push_n_m(first, node, next) \
    SLL_stack_push_nz_m(0, first, node, next)

#define SLL_stack_pop_n_m(first, next) \
    SLL_stack_pop_nz_m(0, first, next)

// -- Singly-linked stacks — simple variant (sentinel = 0, member = next) ----

#define SLL_stack_push_m(first, node) \
    SLL_stack_push_nz_m(0, first, node, next)

#define SLL_stack_pop_m(first) \
    SLL_stack_pop_nz_m(0, first, next)


//------------------------------------------------------------------------------
// Address Sanitizer Markup
//------------------------------------------------------------------------------

#if COMPILER_MSVC
# if defined(__SANITIZE_ADDRESS__)
#  define ASAN_ENABLED 1
#  define ASAN_NO_ADDR __declspec(no_sanitize_address)
# endif
#elif COMPILER_CLANG
# if defined(__has_feature)
#  if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#   define ASAN_ENABLED 1
#  endif
# endif
# define ASAN_NO_ADDR   __attribute__((no_sanitize("address")))
# define UBSAN_NO_ALIGN __attribute__((no_sanitize("alignment")))
#endif

#ifndef  ASAN_NO_ADDR
# define ASAN_NO_ADDR
#endif
#ifndef  UBSAN_NO_ALIGN
# define UBSAN_NO_ALIGN
#endif

#if ASAN_ENABLED
C_LINKAGE void __asan_poison_memory_region(void const volatile *addr, size_t size);
C_LINKAGE void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
# define asan_poison(addr, size)   __asan_poison_memory_region((addr), (size))
# define asan_unpoison(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
# define asan_poison(addr, size)   ((void)(addr), (void)(size))
# define asan_unpoison(addr, size) ((void)(addr), (void)(size))
#endif

// -----------------------------------------------------------------------------
// For loop construction macros
// -----------------------------------------------------------------------------

#define each_index_t_m(list, T, index) (T index = 0; index < list.count; index += 1)
#define each_index_m(list, index)      each_index_t_m(list, U64, index)

#define each_node_zt_m(first, element, T, sentinel) (T *element = first; element != sentinel; element = element->next)
#ifdef typeof_m
#       define each_node_m(first, element)              each_node_zt_m(first, element, typeof_m(*first), 0)
#       define each_node_z_m(first, element, sentinel)  each_node_zt_m(first, element, typeof_m(*first), sentinel)
#else
#       define each_node_z_m(first, element, sentinel)  each_node_zt_m(first, element, T, sentinel)
#endif

// -----------------------------------------------------------------------------
// Memory operation macros
// -----------------------------------------------------------------------------

#if COMPILER_MSVC
#pragma intrinsic(memset, memcpy, memmove, memcmp)
#define memory_zero(pointer, size)                memset((pointer), 0, (size))
#define memory_zero_struct(pointer)               memset((pointer), 0, sizeof(*(pointer)))
#define memory_zero_array(x)                      memset((x), 0, sizeof(x))
#define memory_copy(destination, source, size)    memcpy((destination), (source), (size))
#define memory_move(destination, source, size)    memmove((destination), (source), (size))
#define memory_fill(pointer, value, size)         memset((pointer), (int)(value), (size))
#define memory_match(pointer_a, pointer_b, size)  memcmp((pointer_a), (pointer_b), (size))
#define memory_match_struct(a, b)                 (memcmp((a), (b), sizeof(*(a))) == 0)
#elif COMPILER_CLANG || COMPILER_GCC
#define memory_zero(pointer, size)                __builtin_memset((pointer), 0, (size))
#define memory_zero_struct(pointer)               __builtin_memset((pointer), 0, sizeof(*(pointer)))
#define memory_zero_array(x)                      __builtin_memset((x), 0, sizeof(x))
#define memory_copy(destination, source, size)    __builtin_memcpy((destination), (source), (size))
#define memory_move(destination, source, size)    __builtin_memmove((destination), (source), (size))
#define memory_fill(pointer, value, size)         __builtin_memset((pointer), (int)(value), (size))
#define memory_match(pointer_a, pointer_b, size)  __builtin_memcmp((pointer_a), (pointer_b), (size))
#define memory_match_struct(a, b)                 (__builtin_memcmp((a), (b), sizeof(*(a))) == 0)
#else
#error "memory intrinsics not defined for this compiler"
#endif

#endif // BASE_CORE_H
