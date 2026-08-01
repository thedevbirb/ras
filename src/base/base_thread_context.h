#ifndef BASE_THREAD_CONTEXT_H
#define BASE_THREAD_CONTEXT_H

// Shared context in a OS thread.
typedef struct Thread_Context Thread_Context;
struct Thread_Context
{
	// Scratch arenas
	Arena *arenas[2];

	// Thread name
	U8  thread_name[32];
	U64 thread_name_size;
};

//------------------------------------------------------------------------------
// Thread Context Functions
//-----------------------------------------------------------------------------

internal Thread_Context *Thread_Context_alloc(void);
internal void		 Thread_Context_release(Thread_Context *thread_context);
internal void	         Thread_Context_select(Thread_Context *thread_context);
internal Thread_Context *Thread_Context_selected(void);

// Scratch Arenas
internal Arena *Thread_Context_scratch_get(Arena **conflicts, U64 count);
#define Arena__scratch_begin_m(conflicts, count) Arena_Temporary__begin(Thread_Context_scratch_get((conflicts), (count)))
#define Arena__scratch_end_m(scratch)		 Arena_Temporary__end(scratch)


#endif // BASE_THREAD_CONTEXT_H
