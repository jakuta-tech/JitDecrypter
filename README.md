# Fatmike's Just-In-Time Decrypter

A just-in-time decrypter for windows executables (x86 and x64) that performs code decryption on execution time of an encrypted code section.    

**Keywords**:  
Just-In-time Decrypter, Decrypter, Protector, Code Section, PE, Portable Executable, EXE, Windows

## Motivation

Conventional methods for encrypted code execution typically decrypt entire code sections at once, leaving the complete plaintext code exposed in memory. This project demonstrates an alternative technique: the just-in-time (JIT) decryption of single instructions immediately prior to their execution.  
For simplicity, this implementation operates on 16-byte blocks, but the underlying technique is fully capable of true single-instruction decryption, as demonstrated in my [crackme #5](https://crackmes.one/crackme/66ca5b91b899a3b9dd02af52) and [crackme #7](https://crackmes.one/crackme/67814b594d850ac5f7dc4fc9).

## Requirements
 
## Implementation

## Solution Overview

### Builder Project

### JitDecrypter Project