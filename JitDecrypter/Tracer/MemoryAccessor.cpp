#include "MemoryAccessor.h"


MemoryAccessor::MemoryAccessor()
{
}

MemoryAccessor::~MemoryAccessor()
{
}

void MemoryAccessor::ProtectMemory(DWORD_PTR address, SIZE_T size)
{
  DWORD oldProtect = 0;
  VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
}

DWORD_PTR MemoryAccessor::WriteByte(DWORD_PTR address, BYTE value)
{
  BYTE* memory = (BYTE*)address;
  *memory = value;
  address++;
  return address;
}

DWORD_PTR MemoryAccessor::WriteDword(DWORD_PTR address, DWORD value)
{
  DWORD* memory = (DWORD*)address;
  *memory = value;
  address += 4;
  return address;
}

BYTE MemoryAccessor::ReadByte(DWORD_PTR address)
{
  BYTE* memory = (BYTE*)address;
  return *memory;
}

DWORD MemoryAccessor::ReadDword(DWORD_PTR address)
{
  DWORD* memory = (DWORD*)address;
  return *memory;
}
