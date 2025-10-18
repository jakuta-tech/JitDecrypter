#pragma once
#include <Windows.h>
#include <set>
#include "SectionInfo.h"

class BreakpointManager;

class Tracer
{
public:
  static Tracer& Instance();

  bool StartTracing(SectionInfo* traceSection, SectionInfo* ignoreSection);
  void StopTracing();

private:
  Tracer();
  ~Tracer();

  void Initialize(SectionInfo* traceSection, SectionInfo* ignoreSection);
  void Unload();

  void ApplySectionInfo(SectionInfo* source, SectionInfo& target);

  static LONG WINAPI VectoredHandlerBreakPoint(struct _EXCEPTION_POINTERS* ExceptionInfo);

  LONG HandleException(struct _EXCEPTION_POINTERS* exceptionInfo);
  LONG HandleBreakpointException(PCONTEXT context);
  LONG HandleSingleStepException(PCONTEXT context);
  LONG HandleGuardPageException(struct _EXCEPTION_POINTERS* exceptionInfo);

  BOOL IsInTraceSection(DWORD_PTR address);
  BOOL IsInIgnoreSection(DWORD_PTR address);

  void CryptInstruction(DWORD_PTR eip);
  void RestoreInstructions();
  void EnryptInstruction(DWORD_PTR eip);
  void DecryptInstruction(DWORD_PTR eip);  

  void SetInstructionPointer(PCONTEXT& context, DWORD_PTR value);
  DWORD_PTR GetInstructionPointer(PCONTEXT& context);
  void IncreaseInstructionPointer(PCONTEXT& context);

private:
  BreakpointManager* _breakpointManager;
  PVOID _exceptionHandler;  
  SectionInfo _traceSection;
  SectionInfo _ignoreSection;
  const DWORD _blockSize = 16;
  std::set<DWORD_PTR> _decryptionCache;
  bool _isUnloaded;
};