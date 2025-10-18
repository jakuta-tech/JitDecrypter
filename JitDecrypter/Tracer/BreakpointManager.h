#pragma once
#include <Windows.h>
#include <map>
#include "MemoryAccessor.h"

class BreakpointManager
{
public:
  BreakpointManager();
  ~BreakpointManager();

  void ForceBreakpointException();
  bool BreakPointExceptionForced();

  void SetHardwareBreakPoint(PCONTEXT& context, SIZE_T address);
  void RemoveHardwareBreakPoint(PCONTEXT& context);

  void SetBreakPoint(SIZE_T address);
  void RemoveBreakPoint(SIZE_T address);
  bool HasBreakpoint(SIZE_T address);

  void SetMemoryBreakPoint(SIZE_T address, SIZE_T size);
  bool IsMemoryBreakPointActive() { return _isMemoryBreakPointActive; }
  void NotifyMemoryBreakPointRemoved();

  void SingleStep(PCONTEXT& context);

private:
  MemoryAccessor _memoryAccessor;
  std::map<SIZE_T, BYTE> _breakpoints;
  bool _isMemoryBreakPointActive;
  bool _raisedBreakPointException;
};
