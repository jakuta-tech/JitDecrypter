# Fatmike's Just-In-Time Decrypter

A just-in-time decrypter for Windows executables (x86 and x64) that is capable of single-instruction decryption of an encrypted code section at execution time.    

**Keywords**:  
Just-In-time Decrypter, Single-Instruction Decrypter, Decrypter, Protector, Code Section, PE, Portable Executable, EXE, Windows

## Motivation

Conventional methods for encrypted code execution typically decrypt entire code sections at once, leaving the complete plaintext code exposed in memory. This project demonstrates an alternative technique: the just-in-time (JIT) decryption of single instructions immediately prior to their execution.  
For simplicity, this implementation operates on 16-byte blocks, but the underlying implementation is fully capable of true single-instruction decryption, as demonstrated in some of my crackmes (see below).

## Requirements

- Capability for the programmer to designate specific code segments for encryption
- Just-in-time decryption of individual assembly instructions, precisely at the moment of execution 

## Concept

### Marking code for encryption

Marking code for encryption is achieved through dedicated code sections. In Visual Studio, this can be implemented by applying:

```cpp
__declspec(code_seg(".prot"))
```
at either the ```class ``` or ```method ``` level in the header file.

However, compiler optimizations may override these directives. To prevent this, the ```noinline``` specifier can be used:

```cpp
__declspec(noinline) void ProtectedMethod();
```
Additionally, optimization can be disabled for the relevant implementations in the .cpp file:

```cpp
#pragma optimize( "", off )

ProtectedClass::ProtectedClass()
{
}

ProtectedClass::~ProtectedClass()
{
}

void ProtectedClass::ProtectedMethod()
{
}

#pragma optimize( "", on )
```
This ensures that the instrumentation is preserved and the code executes as intended.

### Just-In-Time Decryption

All components required for just-in-time decryption are encapsulated within the *Tracer* class. The functionality can be activated via the *StartTracing* method and deactivated using *StopTracing*. This approach minimizes the visible overhead within the actual source code logic.

```cpp
// Tracing protected section .prot (protected methods from .prot may be called)
Tracer::Instance().StartTracing(protectedSection, textSection);
ProtectedClass* protectedClass = new ProtectedClass();
protectedClass->ProtectedMethod();
delete protectedClass;
Tracer::Instance().StopTracing();
```

**StartTracing**

- Activates tracing and allows the execution of protected code within a protected section (*.prot* in the provided example).  
  
**StopTracing**

- Deactivates tracing, disabling execution of protected code and allowing unprotected code to be executed again.

**ProtectedClass**

- An encrypted class that is linked into the *.prot* section.
  
**ProtectedMethod**

- An encrypted method that is linked into the *.prot* section.

### Encrypting the code

Once the target executable has been built, encryption is applied to the designated code section (e.g., *.prot*) using the *Builder* application, which is also part of the solution. (see below: **Builder Project**)

## Solution Overview

### Builder Project

*Builder.exe* is an auxiliary application responsible for encrypting the *.prot* section after the target application has been built. The encryption algorithm and key used in *Builder* must match those used in the decrypter implementation in the *JitDecrypter* project.  
*Builder.exe* is configured to run automatically as a post-build event. Therefore, ensure that the solution is **rebuilt** after making changes to the code.

### JitDecrypter Project

This project provides components for **just-in-time (JIT) decryption** of code in encrypted sections, enabling encrypted code to run on demand while standard, non-encrypted code continues to execute normally. It also includes demonstration code as a proof of concept.  

#### Core Class: Tracer

The central component of the solution is the *Tracer* class. Activating it via the *StartTracing* method enables execution of protected code in an encrypted section, while deactivating it with *StopTracing* restores normal code execution.

#### Demo Code

As a demonstration, the following code is included:

- The user can input text.
- The input is then processed to compute its CRC32 value.

Both functions are executed within the encrypted *.prot* section.

## Simplifications

### Relocations

*Relocations* are disabled to prevent the Windows loader from modifying relative virtual addresses (RVAs) within the encrypted code.
This can be configured in the project settings under:
**Linker → Advanced → Randomize Base Address** and **Linker → Advanced → Fixed Base Address**.  

To support relocations, *Builder.exe* must exclude relocated RVAs from encryption, and *JitDecrypter.exe* must determine at runtime whether an instruction should be decrypted.
Alternatively, metadata could be generated by *Builder.exe* and embedded into the resource section of *JitDecrypter.exe*, providing the necessary information for decryption decisions.

### Code decryption

For simplicity, the current implementation decrypts code in fixed 16-byte blocks rather than by instruction length. While this reduces implementation complexity, it is not instruction-accurate and will decrypt bytes that are not part of a single instruction boundary.  

For true single-instruction decryption, *Builder.exe* should generate metadata for each RVA in the protected code section. The metadata should contain the RVA and the corresponding instruction length for every instruction in the protected section. Instruction lengths can be determined using a length-disassembler engine (LDE) or a disassembler such as [Zydis](https://github.com/zyantific/zydis).

Afterwards *Builder.exe* could embed the metadata in the target’s resources or in an additional section (so *JitDecrypter.exe* can read it at runtime).

## Additional Considerations

In this conceptual solution it is unsatisfactory that the user must manage not only the sections to be encrypted but also the invocation of the *Tracer* class. In a production-ready implementation, this responsibility could be delegated to a separate *Protector* application:

**Target**
  - Within the *Target* code, only the sections intended for encryption need to be marked.  
  
**Protector**
  - The *Protector* wraps the *Target* with a loader stub capable of loading the *Target* via manual mapping.
  - By loading the *Target* itself, the loader stub gains full control and can apply the *Tracer* to the encrypted sections of the *Target* automatically.  
  
  This design simplifies usage for the developer and centralizes protection responsibilities within the Protector application.

  ### Implementation

  As proof of concept these additional considerations were implemented in some of my crackmes:
  
**crackmes.one**
  - [Fatmike's Crackme #5](https://crackmes.one/crackme/66ca5b91b899a3b9dd02af52)
  - [Fatmike's Crackme #7](https://crackmes.one/crackme/67814b594d850ac5f7dc4fc9)  
  
**crackmy.app**
  - [Fatmike's Crackme #5](https://crackmy.app/crackmes/fatmike-s-crackme-5-by-fatmike-46575)
  - [Fatmike's Crackme #7](https://crackmy.app/crackmes/fatmike-s-crackme-7-2025-2634)
