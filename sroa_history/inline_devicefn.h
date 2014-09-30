#ifndef MXPA_MS4_H
#define MXPA_MS4_H

// LLVM includes
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace SpmdKernel {
namespace Coarsening {
  class DevFnInliner : public llvm::ModulePass {
  public:
    static char ID;
    DevFnInliner() : llvm::ModulePass(ID) {
      /* empty */; 
    }

    bool runOnModule(llvm::Module &M);

  };
}
}

#endif
