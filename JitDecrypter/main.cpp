#include <iostream>
#include <format>
#include "PEImage\PEImage.h"
#include "Tracer\Tracer.h"
#include "ProtectedCode\ProtectedCodeExecutor.h"

// --- Forward declarations ---
SectionInfo* CreateSectionInfo(const char* sectionName, DWORD_PTR imageBase);

// --- main program --- Builder.exe will be executed as post build event in the Builder project; make sure to rebuild the solution after making changes!
int main()
{
  SectionInfo* textSection = CreateSectionInfo(".text", (DWORD_PTR)GetModuleHandle(nullptr));
  SectionInfo* protectedSection = CreateSectionInfo(".prot", (DWORD_PTR)GetModuleHandle(nullptr));

  std::cout << "Unprotected code : Calling protected code..." << std::endl;

  // Tracing proteced section .prot (protected functions from .prot may be called)
  Tracer::Instance().StartTracing(protectedSection, textSection);
  ProtectedCodeExecutor* executor = new ProtectedCodeExecutor();
  executor->EnterText();
  Tracer::Instance().StopTracing();

  std::cout << "Unprotected code : Calling protected code again..." << std::endl;

  // Tracing proteced section .prot (protected functions from .prot may be called)
  Tracer::Instance().StartTracing(protectedSection, textSection);
  DWORD checksum = executor->GetCrc32();
  delete executor;
  Tracer::Instance().StopTracing();

  std::cout << "Unprotected code : The calculated CRC32 is " << std::format("0x{:08X}", checksum) << std::endl;
  std::cout << "Unprotected code : End of Demo." << std::endl;

  delete textSection;
  delete protectedSection;

  std::cout << "Press ENTER to exit..." << std::endl;
  std::cin.ignore();
  std::cin.get();
}

SectionInfo* CreateSectionInfo(const char* sectionName, DWORD_PTR imageBase)
{
  PEImage peImage(imageBase);
  PIMAGE_SECTION_HEADER section = peImage.FindSection(sectionName);
  if (section == nullptr) return nullptr;

  SectionInfo* traceInfo = new SectionInfo();
  DWORD_PTR sectionStart = imageBase + section->VirtualAddress;
  DWORD_PTR sectionSize = section->Misc.VirtualSize;
  DWORD_PTR sectionEnd = sectionStart + sectionSize - 1;

  traceInfo->SetSectionStart(sectionStart);
  traceInfo->SetSectionEnd(sectionEnd);
  traceInfo->SetSectionSize(sectionSize);

  return traceInfo;
}