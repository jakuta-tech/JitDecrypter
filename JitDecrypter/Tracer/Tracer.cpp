#include <iostream>
#include "Tracer.h"
#include "SectionInfo.h"
#include "BreakpointManager.h"

Tracer::Tracer()
{
#define CALL_FIRST 1  
#define CALL_LAST 0
  _breakpointManager = nullptr;
  _exceptionHandler = nullptr;
}

Tracer::~Tracer()
{
  Unload();
}

Tracer& Tracer::Instance()
{
  static Tracer _instance;
  return _instance;
}

bool Tracer::StartTracing(SectionInfo* traceSection, SectionInfo* ignoreSection)
{
  if (traceSection == nullptr || ignoreSection == nullptr) return false;
  Initialize(traceSection, ignoreSection);
  _breakpointManager->ForceBreakpointException();
  return true;
}

void Tracer::StopTracing()
{
  Unload();
}

void Tracer::Initialize(SectionInfo* traceSection, SectionInfo* ignoreSection)
{
  ApplySectionInfo(traceSection, _traceSection);
  ApplySectionInfo(ignoreSection, _ignoreSection);

  _isUnloaded = false;
  _breakpointManager = new BreakpointManager();
  _exceptionHandler = AddVectoredExceptionHandler(CALL_FIRST, VectoredHandlerBreakPoint);
}

void Tracer::Unload()
{
  _isUnloaded = true;
  RestoreInstructions();

  if (_exceptionHandler != nullptr)
  {
    RemoveVectoredExceptionHandler(_exceptionHandler);
    _exceptionHandler = nullptr;
  }
  if (_breakpointManager != nullptr)
  {
    delete _breakpointManager;
    _breakpointManager = nullptr;
  }

  _traceSection.Clear();
  _ignoreSection.Clear();
}

void Tracer::ApplySectionInfo(SectionInfo* source, SectionInfo& target)
{
  target.SetSectionStart(source->GetSectionStart());
  target.SetSectionEnd(source->GetSectionEnd());
  target.SetSectionSize(source->GetSectionSize());
  DWORD oldProtect = 0;
  VirtualProtect((LPVOID)target.GetSectionStart(), target.GetSectionSize(), PAGE_EXECUTE_READWRITE, &oldProtect);
}

LONG WINAPI Tracer::VectoredHandlerBreakPoint(struct _EXCEPTION_POINTERS* exceptionInfo)
{
  LONG result = Tracer::Instance().HandleException(exceptionInfo);
  return result;
}

LONG Tracer::HandleException(struct _EXCEPTION_POINTERS* exceptionInfo)
{
  if (_isUnloaded) return EXCEPTION_CONTINUE_EXECUTION;

  PCONTEXT context = exceptionInfo->ContextRecord;
  if (exceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
  {
    //std::cout << "EXCEPTION_BREAKPOINT" << std::endl;
    return HandleBreakpointException(context);
  }
  else if (exceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
  {
    //std::cout << "EXCEPTION_SINGLE_STEP" << std::endl;
    return HandleSingleStepException(context);
  }
  else if (exceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE)
  {
    //std::cout << "EXCEPTION_GUARD_PAGE" << std::endl;
    return HandleGuardPageException(exceptionInfo);
  }
  else
  {
    return EXCEPTION_CONTINUE_EXECUTION;
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

LONG Tracer::HandleBreakpointException(PCONTEXT context)
{
  if (_breakpointManager->BreakPointExceptionForced())
  {
    IncreaseInstructionPointer(context); // Goto nop (see: ForceBreakpoint.asm)
    _breakpointManager->SetHardwareBreakPoint(context, GetInstructionPointer(context));
    return EXCEPTION_CONTINUE_EXECUTION;
  }
  else if (_breakpointManager->HasBreakpoint(GetInstructionPointer(context)))
  {
    _breakpointManager->RemoveBreakPoint(GetInstructionPointer(context));
    _breakpointManager->SetHardwareBreakPoint(context, GetInstructionPointer(context));
    return EXCEPTION_CONTINUE_EXECUTION;
  }
  else
  {
    return EXCEPTION_CONTINUE_SEARCH;
  }
}

LONG Tracer::HandleSingleStepException(PCONTEXT context)
{
  if (IsInTraceSection(GetInstructionPointer(context)))
  {
    DWORD_PTR currentEip = GetInstructionPointer(context);
    CryptInstruction(currentEip);
    SetInstructionPointer(context, currentEip);
    _breakpointManager->SingleStep(context);
  }
  else if (IsInIgnoreSection(GetInstructionPointer(context)))
  {
    // Just single step over the ignore section
    _breakpointManager->SingleStep(context);
  }
  else // Use memory breakpoint to regain control when EIP is back in _traceSection
  {
    if (!_breakpointManager->IsMemoryBreakPointActive())
    {
      _breakpointManager->SetMemoryBreakPoint(_traceSection.GetSectionStart(), _traceSection.GetSectionSize());
    }
  }
  return EXCEPTION_CONTINUE_EXECUTION;
}

LONG Tracer::HandleGuardPageException(struct _EXCEPTION_POINTERS* exceptionInfo)
{
  if (!_breakpointManager->IsMemoryBreakPointActive()) return EXCEPTION_CONTINUE_EXECUTION;
  _breakpointManager->NotifyMemoryBreakPointRemoved();

  if (exceptionInfo->ExceptionRecord->NumberParameters < 1)
  {
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  DWORD_PTR reason = exceptionInfo->ExceptionRecord->ExceptionInformation[0];
  if (reason == 0 || reason == 1) // Break reason: read or write
  {
    _breakpointManager->SetMemoryBreakPoint(_traceSection.GetSectionStart(), _traceSection.GetSectionSize());
  }
  else if (reason == 8) // Break reason: execution
  {
    PCONTEXT context = exceptionInfo->ContextRecord;
    if (IsInTraceSection(GetInstructionPointer(context)))
    {
      DWORD_PTR currentEip = GetInstructionPointer(context);
      CryptInstruction(currentEip);
      SetInstructionPointer(context, currentEip);
      _breakpointManager->SingleStep(context);
    }
    else
    {
      _breakpointManager->SingleStep(context);
    }
  }
  return EXCEPTION_CONTINUE_EXECUTION;
}

BOOL Tracer::IsInTraceSection(DWORD_PTR address)
{
  return (address >= _traceSection.GetSectionStart() && address <= _traceSection.GetSectionEnd());
}

BOOL Tracer::IsInIgnoreSection(DWORD_PTR address)
{
  return (address >= _ignoreSection.GetSectionStart() && address <= _ignoreSection.GetSectionEnd());
}

void Tracer::CryptInstruction(DWORD_PTR eip)
{
  RestoreInstructions();
  if (eip != 0)
  {
    DecryptInstruction(eip);
  }
}

void Tracer::RestoreInstructions()
{
  for (auto iter = _decryptionCache.begin(); iter != _decryptionCache.end(); ++iter)
  {
    DWORD_PTR cachedEip = *iter;
    EnryptInstruction(cachedEip);
  }
  _decryptionCache.clear();
}

void Tracer::EnryptInstruction(DWORD_PTR eip)
{
  const BYTE key = 0xCC;
  BYTE* codePtr = (BYTE*)eip;
  for (DWORD i = 0; i < _blockSize; i++)
  {
    codePtr[i] ^= key;
  }
}

void Tracer::DecryptInstruction(DWORD_PTR eip)
{
  _decryptionCache.insert(eip);

  const BYTE key = 0xCC;
  BYTE* codePtr = (BYTE*)eip;
  for (DWORD i = 0; i < _blockSize; i++)
  {
    codePtr[i] ^= key;
  }
}

void Tracer::SetInstructionPointer(PCONTEXT& context, DWORD_PTR value)
{
#ifdef _WIN64
  context->Rip = value;
  FlushInstructionCache(GetCurrentProcess(), (LPCVOID)context->Rip, _blockSize);
#else
  context->Eip = value;
  FlushInstructionCache(GetCurrentProcess(), (LPCVOID)context->Eip, _blockSize);
#endif  
}

DWORD_PTR Tracer::GetInstructionPointer(PCONTEXT& context)
{
#ifdef _WIN64
  return context->Rip;
#else
  return context->Eip;
#endif
}

void Tracer::IncreaseInstructionPointer(PCONTEXT& context)
{
#ifdef _WIN64
  context->Rip++;
  FlushInstructionCache(GetCurrentProcess(), (LPCVOID)context->Rip, _blockSize);
#else
  context->Eip++;
  FlushInstructionCache(GetCurrentProcess(), (LPCVOID)context->Eip, _blockSize);
#endif
}