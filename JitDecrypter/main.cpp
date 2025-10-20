#include <iostream>
#include <format>
#include "Tracer\Tracer.h"
#include "ProtectedCode\ProtectedCodeExecutor.h"

// --- main program : Builder.exe will be executed as post build event in the Builder project; make sure to rebuild the solution after making changes!
int main()
{
  SectionInfo* textSection = Tracer::Instance().CreateSectionInfo(".text", (DWORD_PTR)GetModuleHandle(nullptr));
  SectionInfo* protectedSection = Tracer::Instance().CreateSectionInfo(".prot", (DWORD_PTR)GetModuleHandle(nullptr));

  std::cout << "Unprotected code : Calling protected code..." << std::endl;

  // Tracing protected section .prot (protected methods from .prot may be called)
  Tracer::Instance().StartTracing(protectedSection, textSection);
  ProtectedCodeExecutor* executor = new ProtectedCodeExecutor();
  executor->EnterText();
  Tracer::Instance().StopTracing();

  std::cout << "Unprotected code : Calling protected code again..." << std::endl;

  // Tracing protected section .prot (protected methods from .prot may be called)
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