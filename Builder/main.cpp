#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "SectionEnryption\PEFile.h"
#include "SectionEnryption\FileWriter.h"

// --- Forward declarations ---
bool EncryptSection(const std::string& fileName, const std::string& sectionName, BYTE key);
bool EncryptBuffer(const BYTE* inputBuffer, BYTE* outputBuffer, DWORD length, BYTE key);
bool SaveEncryptedFile(const std::string& fileName, BYTE* data, DWORD dataSize);

// --- main program --- Will be executed as post build event in the Builder project; make sure to rebuild the solution after making changes!
int main()
{
  const std::string fileName = "JitDecrypter.exe";
  const std::string sectionName = ".prot";
  const BYTE key = 0xCC;

  std::cout << "Encrypting section " << sectionName << " of " << fileName << "..." << std::endl;

  if (!EncryptSection(fileName, sectionName, key))
  {
    std::cerr << "Encryption failed!" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Encryption finished successfully." << std::endl;
  std::cout << "Press ENTER to exit..." << std::endl;
  std::cin.get();

  return EXIT_SUCCESS;
}

bool EncryptSection(const std::string& fileName, const std::string& sectionName, BYTE key)
{
  PEFile peFile;
  if (!peFile.OpenFile(fileName.c_str()))
  {
    std::cerr << "Failed to open file: " << fileName << std::endl;
    return false;
  }

  PIMAGE_SECTION_HEADER sectionHeader = peFile.FindSectionByName(sectionName.c_str());
  if (!sectionHeader)
  {
    std::cerr << "Could not find section: " << sectionName << std::endl;
    return false;
  }

  DWORD startOffset = sectionHeader->PointerToRawData;
  DWORD length = sectionHeader->SizeOfRawData;
  BYTE* inputBuffer = reinterpret_cast<BYTE*>(reinterpret_cast<DWORD_PTR>(peFile.DOS_HEADER()) + startOffset);

  std::unique_ptr<BYTE[]> outputBuffer(new BYTE[length]);
  memset(outputBuffer.get(), 0, length);

  if (!EncryptBuffer(inputBuffer, outputBuffer.get(), length, key))
    return false;

  memcpy(inputBuffer, outputBuffer.get(), length);

  return SaveEncryptedFile(fileName, reinterpret_cast<BYTE*>(peFile.DOS_HEADER()), peFile.GetBufferSize());
}

bool EncryptBuffer(const BYTE* inputBuffer, BYTE* outputBuffer, DWORD length, BYTE key)
{
  if (!inputBuffer || !outputBuffer || length == 0)
    return false;

  for (DWORD i = 0; i < length; ++i)
    outputBuffer[i] = inputBuffer[i] ^ key;

  return true;
}

bool SaveEncryptedFile(const std::string& fileName, BYTE* data, DWORD dataSize)
{
  FileWriter fileWriter;
  std::ofstream outfile;

  fileWriter.OpenForWriteBin(fileName.c_str(), outfile);

  fileWriter.Write(outfile, data, dataSize);
  fileWriter.Close(outfile);

  return true;
}