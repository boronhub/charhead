
#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <fstream>
#include <iostream>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <stack>
#include <string>

#include <vector>

class CharHead {
public:
  char *ReadFile(std::string filename);
  void CodeCompiler(char *input);

private:
  const unsigned int TAPE_SIZE = 30000;
  const unsigned int OP_TAPE_SIZE = 256;
};

#endif