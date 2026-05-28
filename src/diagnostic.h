#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#ifndef DIAGNOSTICS_ERRORS_MAX
#define DIAGNOSTICS_ERRORS_MAX 64
#endif

#ifndef DIAGNOSTICS_WARNINGS_MAX
#define DIAGNOSTICS_WARNINGS_MAX 128
#endif

#if 0
/// Represents a single fixit, a replacement of one range of text with another.
class SMFixIt {
  SMRange Range;

  std::string Text;

public:
  LLVM_ABI SMFixIt(SMRange R, const Twine &Replacement);

  SMFixIt(SMLoc Loc, const Twine &Replacement)
      : SMFixIt(SMRange(Loc, Loc), Replacement) {}

  StringRef getText() const { return Text; }
  SMRange getRange() const { return Range; }

  bool operator<(const SMFixIt &Other) const {
    if (Range.Start.getPointer() != Other.Range.Start.getPointer())
      return Range.Start.getPointer() < Other.Range.Start.getPointer();
    if (Range.End.getPointer() != Other.Range.End.getPointer())
      return Range.End.getPointer() < Other.Range.End.getPointer();
    return Text < Other.Text;
  }
};

/// Instances of this class encapsulate one diagnostic report, allowing
/// printing to a raw_ostream as a caret diagnostic.
class SMDiagnostic {
  const SourceMgr *SM = nullptr;
  SMLoc Loc;
  std::string Filename;
  int LineNo = 0;
  int ColumnNo = 0;
  SourceMgr::DiagKind Kind = SourceMgr::DK_Error;
  std::string Message, LineContents;
  std::vector<std::pair<unsigned, unsigned>> Ranges;
  SmallVector<SMFixIt, 4> FixIts;
#endif

typedef enum Diagnostic_Kind
{
	Diagnostic_Kind__Error,
	Diagnostic_Kind__Warning,
	Diagnostic_Kind__Hint,
	Diagnostic_Kind__COUNT,
}
Diagnostic_Kind;

typedef struct Diagnostic_Fix Diagnostic_Fix;
struct Diagnostic_Fix
{
	Vec2_U32 range;
	String8  text;
};



typedef struct Diagnostic Diagnostic;
struct Diagnostic
{
	Source_Manager *source_manager;
	String8         filename;
	String8         message;
	U8             *line;
	U64             location;
	U32             row_index;
	U32             column_index;
	Vec2_U32        ranges[4];
	Diagnostic_Fix  fixes[4];
};

typedef struct Diagnostics Diagnostics;
struct Diagnostics
{
	Diagnostic *data;
	U8          count;
};

Diagnostics
Diagnostics__new(Arena *arena)
{
	Diagnostic *data = Arena__push_array_m(arena, Diagnostic, 256);
	Diagnostics result =
	{
		.data = data,
	};
	return result;
}


// If maximum is reached, the last slot will be overwritten.
internal Diagnostic *
Diagnostics__push(Diagnostics *diagnostics)
{
	Diagnostic *result = 0;
	if (diagnostics->count == U8_max + 1)
	{
		result = &diagnostics->data[U8_max];
		memory_zero_struct(result);
	}
	else
	{
		result = &diagnostics->data[diagnostics->count];
		diagnostics->count += 1;
	}

	return result;
}

internal void
Diagnostic__print(Diagnostic *diagnostic);

#endif // DIAGNOSTIC_H

