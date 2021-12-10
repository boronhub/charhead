
#include "compiler.hpp"

char *CharHead::ReadFile(std::string filename) {
  std::ifstream ifs{filename};
  std::string input;

  while (ifs.good()) {
    input.push_back(ifs.get());
  }
  char *char_array = new char[input.length()];
  strcpy(char_array, input.c_str());

  return char_array;
}

void CharHead::CodeCompiler(char *input) {

  llvm::LLVMContext context;
  llvm::Module *module = new llvm::Module("CharHead Compiler JIT", context);
  auto owner = std::unique_ptr<llvm::Module>(module);
  llvm::IRBuilder<> builder(context);

  llvm::FunctionType *main_type =
      llvm::FunctionType::get(builder.getInt32Ty(), false);
  llvm::Function *main_func = llvm::Function::Create(
      main_type, llvm::Function::ExternalLinkage, "main", module);

  llvm::BasicBlock *entry =
      llvm::BasicBlock::Create(context, "entry", main_func);
  builder.SetInsertPoint(entry);

  std::stack<llvm::BasicBlock *> basicblocks, exit_bbs;

  llvm::ArrayType *cells_type =
      llvm::ArrayType::get(builder.getInt8Ty(), TAPE_SIZE);
  llvm::ConstantAggregateZero *const_array =
      llvm::ConstantAggregateZero::get(cells_type);
  llvm::GlobalVariable *cells = new llvm::GlobalVariable(
      *module, cells_type, false, llvm::GlobalValue::PrivateLinkage,
      const_array, "cells");
  llvm::AllocaInst *cell_ptr =
      builder.CreateAlloca(builder.getInt8PtrTy(), 0, "cell_ptr");

  llvm::ArrayType *op_cells_type =
      llvm::ArrayType::get(builder.getInt8Ty(), OP_TAPE_SIZE);
  llvm::ConstantAggregateZero *op_const_array =
      llvm::ConstantAggregateZero::get(op_cells_type);
  llvm::GlobalVariable *op_cells = new llvm::GlobalVariable(
      *module, op_cells_type, false, llvm::GlobalValue::PrivateLinkage,
      op_const_array, "cells");
  llvm::AllocaInst *op_cell_ptr =
      builder.CreateAlloca(builder.getInt8PtrTy(), 0, "cell_ptr");

  std::vector<llvm::Value *> op_index;
  llvm::Value *op_ptr = builder.CreateGEP(op_cells, op_index);
  builder.CreateStore(op_ptr, op_cell_ptr);

  std::vector<llvm::Value *> index;
  llvm::Value *ptr = builder.CreateGEP(cells, index);
  builder.CreateStore(ptr, cell_ptr);

  llvm::FunctionType *putchar_type =
      llvm::FunctionType::get(builder.getInt32Ty(), builder.getInt8Ty(), false);
  llvm::FunctionCallee putchar_func =
      module->getOrInsertFunction("putchar", putchar_type);

  std::vector<llvm::Type *> getchar_args;
  llvm::FunctionType *getchar_type =
      llvm::FunctionType::get(builder.getInt32Ty(), getchar_args, false);
  llvm::FunctionCallee getchar_func =
      module->getOrInsertFunction("getchar", getchar_type);

  char code_ptr;

  for (int i = 0; input[i] != '\0'; ++i) {
    code_ptr = input[i];
    switch (code_ptr) {
    case '@':
    case '!': {
      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      llvm::Value *value = builder.CreateGEP(
          builder.getInt8Ty(), ptr, builder.getInt32(code_ptr == '!' ? 1 : -1));
      builder.CreateStore(value, cell_ptr);

      break;
    }
    case '#':
    case '$': {
      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      llvm::Value *value = builder.CreateLoad(ptr);
      value =
          builder.CreateAdd(value, builder.getInt8(code_ptr == '#' ? 1 : -1));
      builder.CreateStore(value, ptr);

      break;
    }
    case ';': {
      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      llvm::Value *value = builder.CreateLoad(ptr);
      builder.CreateCall(putchar_func, value);

      break;
    }
    case '_': {
      llvm::Value *value = builder.CreateCall(getchar_func);
      value = builder.CreateIntCast(value, builder.getInt8Ty(), false);
      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      builder.CreateStore(value, ptr);

      break;
    }

    case '.': {
      llvm::Value *ptr = builder.CreateLoad(op_cell_ptr);
      llvm::Value *value = builder.CreateLoad(ptr);
      builder.CreateCall(putchar_func, value);

      break;
    }

    case '(': {
      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      llvm::Value *value = builder.CreateLoad(ptr);
      llvm::Value *cmp = builder.CreateICmpNE(value, builder.getInt8(0));

      llvm::BasicBlock *bb_loop =
          llvm::BasicBlock::Create(context, "", main_func);
      llvm::BasicBlock *bb_exit =
          llvm::BasicBlock::Create(context, "", main_func);
      basicblocks.push(bb_loop);
      exit_bbs.push(bb_exit);

      builder.CreateCondBr(cmp, bb_loop, bb_exit);
      builder.SetInsertPoint(bb_loop);

      break;
    }
    case ')': {
      if (exit_bbs.empty()) {
        fprintf(stderr, "error: loops are not closed\n");
        exit(1);
      }

      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      llvm::Value *value = builder.CreateLoad(ptr);
      llvm::Value *cmp = builder.CreateICmpNE(value, builder.getInt8(0));

      llvm::BasicBlock *bb_loop = basicblocks.top();
      basicblocks.pop();

      llvm::BasicBlock *bb_exit = exit_bbs.top();
      exit_bbs.pop();

      builder.CreateCondBr(cmp, bb_loop, bb_exit);
      builder.SetInsertPoint(bb_exit);

      break;
    }

    case '[': {
      llvm::Value *op_ptr = builder.CreateLoad(op_cell_ptr);
      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      llvm::Value *value = builder.CreateLoad(ptr);
      builder.CreateStore(value, op_ptr);
    }

    case ']': {
      llvm::Value *op_ptr = builder.CreateLoad(op_cell_ptr);
      llvm::Value *ptr = builder.CreateLoad(cell_ptr);
      llvm::Value *value = builder.CreateLoad(op_ptr);
      builder.CreateStore(value, ptr);
    }
    }
  }

  builder.CreateRet(builder.getInt32(0));

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  std::string error_string;

  llvm::EngineBuilder *EB = new llvm::EngineBuilder(std::move(owner));
  llvm::ExecutionEngine *EE =
      EB->setErrorStr(&error_string)
          .setMCJITMemoryManager(std::unique_ptr<llvm::SectionMemoryManager>(
              new llvm::SectionMemoryManager()))
          .create();

  if (!error_string.empty()) {
    std::cerr << error_string << std::endl;
    exit(1);
  }

  EE->finalizeObject();
  std::vector<llvm::GenericValue> args(0);
  llvm::GenericValue gv = EE->runFunction(main_func, args);
  delete EE;
  llvm::llvm_shutdown();
}