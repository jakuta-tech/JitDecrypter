#pragma once
#include <Windows.h>

class MemoryAccessor
{
public:
  MemoryAccessor();
  ~MemoryAccessor();

  void ProtectMemory(DWORD_PTR address, SIZE_T size);
  DWORD_PTR WriteByte(DWORD_PTR address, BYTE value);
  DWORD_PTR WriteDword(DWORD_PTR address, DWORD value);
  BYTE ReadByte(DWORD_PTR address);
  DWORD ReadDword(DWORD_PTR address);
};
