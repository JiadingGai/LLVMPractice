#define DEBUG_TYPE "mxpa_ms4"

// LLVM includes
#include "llvm/Support/Debug.h"

// MxPA includes
#include "inline_devicefn.h"
#include "kernel_info_reader.h"

namespace SpmdKernel {

using namespace llvm;

namespace Coarsening {

  char DevFnInliner::ID = 0;
  
  namespace {
    static
    RegisterPass<DevFnInliner> DFIModulePass("mxpa_inliner", 
      "Mark all non-kernel functions alwaysinline");
  }

  bool DevFnInliner::runOnModule(Module &M) {
    bool Changed = false;

    CL_KernelInfo CLKI(M);
    std::vector<std::string> KernelNames = CLKI.get_kernel_names();

    for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
      if (FI->isDeclaration())
        continue;

      std::string fnm = FI->getName().str();
      bool IsKernel = (std::find(KernelNames.begin(), KernelNames.end(), fnm) != KernelNames.end());

      if (!IsKernel) {
        AttributeSet DeviceFnAttr = FI->getAttributes();

        //if (!DeviceFnAttr.hasAttribute(AttributeSet::FunctionIndex, Attribute::AlwaysInline)) {
          AttrBuilder B;
          B.addAttribute(Attribute::NoInline);
          AttributeSet NoInlineAttr = AttributeSet::get(FI->getContext(), AttributeSet::FunctionIndex, B);
          FI->removeAttributes(AttributeSet::FunctionIndex, NoInlineAttr);
          FI->addFnAttr(Attribute::AlwaysInline);
          FI->setLinkage(GlobalValue::InternalLinkage);
          Changed = true;
        //}
      } else {
        AttributeSet KernelAttr = FI->getAttributes();
        
        //if (KernelAttr.hasAttribute(AttributeSet::FunctionIndex, Attribute::AlwaysInline)) {
          AttrBuilder B;
          B.addAttribute(Attribute::AlwaysInline);


          AttributeSet InlineAttr = AttributeSet::get(FI->getContext(), AttributeSet::FunctionIndex, B);
          FI->removeAttributes(AttributeSet::FunctionIndex, InlineAttr);
          FI->addFnAttr(Attribute::NoInline);
          FI->setLinkage(GlobalValue::ExternalLinkage);
          Changed = true;
          
        //}
        DEBUG_WITH_TYPE("mxpa_ms4",errs() << "Function: " << FI->getName() << "\n");
      }
    }

    return Changed;
  }
  
}
}
