# Fatmike's Just-In-Time Decrypter

A just-in-time decrypter for Windows executables (x86 and x64) that is capable of single-instruction decryption of an encrypted code section at execution time.    

**Keywords**:  
Just-In-time Decrypter, Decrypter, Protector, Code Section, PE, Portable Executable, EXE, Windows

## Motivation

Conventional methods for encrypted code execution typically decrypt entire code sections at once, leaving the complete plaintext code exposed in memory. This project demonstrates an alternative technique: the just-in-time (JIT) decryption of single instructions immediately prior to their execution.  
For simplicity, this implementation operates on 16-byte blocks, but the underlying technique is fully capable of true single-instruction decryption, as demonstrated in my crackmes *Fatmike's Crackme #5* and *Fatmike's Crackme #7*:
- crackmes.one
  - [Fatmike's Crackme #5](https://crackmes.one/crackme/66ca5b91b899a3b9dd02af52)
  - [Fatmike's Crackme #7](https://crackmes.one/crackme/67814b594d850ac5f7dc4fc9)
- crackmy.app
  - [Fatmike's Crackme #5](https://crackmy.app/crackmes/fatmike-s-crackme-5-by-fatmike-46575)
  - [Fatmike's Crackme #7](https://crackmy.app/crackmes/fatmike-s-crackme-7-2025-2634)

## Requirements

## Solution Overview

## Implementation Details