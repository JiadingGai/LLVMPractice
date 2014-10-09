#define DEBUG_TYPE "MYSROA"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

STATISTIC(NumReplaced, "Number of alloca's broken up");

namespace {
  struct MYSROA : public ModulePass {
    static char ID; 

    MYSROA() : ModulePass(ID) {
    }

    bool runOnModule(Module &M);

    void getAnalysisUsage(AnalysisUsage& AU) const {
    }
  };

  RegisterPass<MYSROA> X("mysroa", "Jiading Gai's Scalar Replacement of Aggregates");
}

char MYSROA::ID = 0;
bool MYSROA::runOnModule(Module &M) {
  bool Changed = false;
  for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
    if (F->isDeclaration())
      continue;

  std::vector<AllocaInst*> WorkList;

  // Scan the entry basic block, adding any alloca's and mallocs to the WorkList
  BasicBlock &BB = F->getEntryBlock();
  for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) { 
    if (AllocaInst *A = dyn_cast<AllocaInst>(I)) {
      WorkList.push_back(A);
    }
  }

  // Process the worklist
  while (!WorkList.empty()) {
    AllocaInst *AI = WorkList.back();
    WorkList.pop_back();

    // We cannot transform the allocation instruction if it is an array allocation 
    // (allocations OF arrays are okay though), and an allocation of a scalar value 
    // cannot be decomposed.
    if (AI->isArrayAllocation() || (!isa<StructType>(AI->getAllocatedType()) && !isa<ArrayType>(AI->getAllocatedType())))
      continue;

    const ArrayType *AT = dyn_cast<ArrayType>(AI->getAllocatedType());

    // Loop over the use list of the alloca. We can only transform it if there
    // are only getelementptr instructions (with a zero first index) and free
    // instructions.
    //
    bool CannotTransform = false;
    for (Value::use_iterator I = AI->use_begin(), E = AI->use_end(); I != E; ++I) {
      Instruction *User = cast<Instruction>(*I);
      if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(User)) {
        DEBUG_WITH_TYPE("MYSROA", errs() << "[Alloca Users] " << *GEPI << "\n");

        if (GEPI->getNumOperands() <= 2 || GEPI->getOperand(1) != Constant::getNullValue(Type::getInt32Ty(F->getContext())) || !isa<Constant>(GEPI->getOperand(2)) || isa<ConstantExpr>(GEPI->getOperand(2))) {
          DEBUG(errs() << "Cannot transform: " << *AI << " due to user: " << User);
          CannotTransform = true;
          break;
        }

        // If this is an array access, check to make sure that index falls within 
        // the array. Otherwise, we won't do the optimization.
        if (AT && (cast<ConstantInt>(GEPI->getOperand(2))->getZExtValue() >= AT->getNumElements())) {
          DEBUG(errs() << "Cannot Transform" << *AI << "due to users: " << User);
          CannotTransform = true;
          break;
        }

      } else {
          DEBUG(errs() << "Cannot transform: " << *AI << " due to user: " << User);
          CannotTransform = true;
          break;
      }
    }

    if (CannotTransform) continue;

    DEBUG(errs() << "Found inst to xform: " << *AI << "\n");
    Changed = true;

    std::vector<AllocaInst*> ElementAllocas;
    if (const StructType *ST = dyn_cast<StructType>(AI->getAllocatedType())) {
      ElementAllocas.reserve(ST->getNumContainedTypes());
      for (unsigned i = 0, e = ST->getNumContainedTypes(); i != e; ++i) {
        AllocaInst *NA = new AllocaInst(ST->getContainedType(i), 0, AI->getName() + "." + utostr(i), AI);
        ElementAllocas.push_back(NA);
        WorkList.push_back(NA);
      }
    } else {
      ElementAllocas.reserve(AT->getNumElements());
      Type *ElTy = AT->getElementType();
      for (unsigned i = 0, e = AT->getNumElements(); i != e; ++i) {
        AllocaInst *NA = new AllocaInst(ElTy, 0, AI->getName() + "." + utostr(i), AI);
        ElementAllocas.push_back(NA);
        WorkList.push_back(NA);
      }
    }

    std::vector<Instruction*> Users;
    for (Value::use_iterator I = AI->use_begin(), E = AI->use_end(); I != E; ++I) {
      Instruction *User = dyn_cast<Instruction>(*I);
      Users.push_back(User);
    }

    for (std::vector<Instruction*>::iterator UI = Users.begin(), EI = Users.end(); UI != EI; ++UI) {
      Instruction *User = dyn_cast<Instruction>(*UI);
      if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(User)) {
        // We now know that the GEP is of the form: GEP <ptr>, 0, <cst>
        int64_t Idx = dyn_cast<ConstantInt>(GEPI->getOperand(2))->getSExtValue();
        assert(Idx < ElementAllocas.size() && "Index out of range?");
        AllocaInst *AllocaToUse = ElementAllocas[Idx];

        Value *RepValue;
        if (GEPI->getNumOperands() == 3) {
          RepValue = AllocaToUse;
        } else {
          StringRef OldName = GEPI->getName();
          GEPI->setName("");
          RepValue = GetElementPtrInst::Create(AllocaToUse, std::vector<Value*>(GEPI->op_begin() + 3, GEPI->op_end()), OldName, GEPI);
        }

        GEPI->replaceAllUsesWith(RepValue);
        GEPI->getParent()->getInstList().erase(GEPI);
      } else {
        assert(0 && "Unexpected instruction type!\n");
      }
    }

    AI->getParent()->getInstList().erase(AI);
    NumReplaced++;
  }

  DEBUG_WITH_TYPE("MYSROA", errs() << "[FunctionName] " << F->getName() << "\n");
  }

  return Changed;
}
