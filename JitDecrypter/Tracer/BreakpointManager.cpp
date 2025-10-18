#include <Windows.h>
#include "BreakpointManager.h"
#include <iostream>

extern "C" void ForceBreakpoint();  // Defined in Assembly (ForceBreakpoint.asm)

BreakpointManager::BreakpointManager()
{
  _isMemoryBreakPointActive = false;
  _raisedBreakPointException = false;
  _oldProtect = 0;
}

BreakpointManager::~BreakpointManager()
{
}

void BreakpointManager::ForceBreakpointException()
{
  _raisedBreakPointException = true;
  ForceBreakpoint();
  /*
  // Inline assembly can only be used in x86
  _asm
  {
    int 3;
    nop;
  }
  */
}

bool BreakpointManager::BreakPointExceptionForced()
{
  if (!_raisedBreakPointException) return false;
  _raisedBreakPointException = false;
  return true;
}

void BreakpointManager::SetHardwareBreakPoint(PCONTEXT& context, SIZE_T address)
{
  context->Dr0 = address;
  context->Dr7 = 1 << 0; // Enable breakpoint
  context->EFlags &= ~(1 << 8); // Remove trap flag
}

void BreakpointManager::RemoveHardwareBreakPoint(PCONTEXT& context)
{
  context->Dr0 = 0;
  context->Dr7 = 0; // Disable breakpoint
  context->EFlags &= ~(1 << 8); // Remove trap flag
}

void BreakpointManager::SetBreakPoint(SIZE_T address)
{
  if (HasBreakpoint(address)) return;

  BYTE breakPoint = 0xCC;
  _memoryAccessor.ProtectMemory(address, 1);
  BYTE originalInstruction = _memoryAccessor.ReadByte(address);
  if (originalInstruction == breakPoint) return; // BP might have been set by other debugger
  _breakpoints[address] = originalInstruction;
  _memoryAccessor.WriteByte(address, breakPoint);
}

void BreakpointManager::RemoveBreakPoint(SIZE_T address)
{
  auto iter = _breakpoints.find(address);
  if (iter == _breakpoints.end()) return;

  BYTE originalInstruction = iter->second;
  _memoryAccessor.ProtectMemory(address, 1);
  _memoryAccessor.WriteByte(address, originalInstruction);
}

bool BreakpointManager::HasBreakpoint(SIZE_T address)
{
  auto iter = _breakpoints.find(address);
  return (iter != _breakpoints.end());
}

void BreakpointManager::SetMemoryBreakPoint(SIZE_T address, SIZE_T size)
{
  _isMemoryBreakPointActive = true;
  VirtualProtect((LPVOID)address, size, PAGE_GUARD | PAGE_EXECUTE_READWRITE, &_oldProtect);
}

void BreakpointManager::NotifyMemoryBreakPointRemoved()
{
  _isMemoryBreakPointActive = false;
}

void BreakpointManager::SingleStep(PCONTEXT& context)
{
  context->Dr0 = context->Dr6 = context->Dr7 = 0;
  context->EFlags |= (1 << 8); // Set trap flag
}
