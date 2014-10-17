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
  
  private:
    bool isSafeArrayElementUse(Value *ptr);
    bool isSafeUseOfAllocation(Instruction *User);
    bool isSafeStructAllocaToPromote(AllocaInst *AI);
    bool isSafeArrayAllocaToPromote(AllocaInst *AI);
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

      errs() << "[Gai] " << isa<ArrayType>(AI->getAllocatedType()) << " is safe to promote " << isSafeArrayAllocaToPromote(AI) << "\n";
      // Check that all of the users of the allocation are capable of being transformed
      if (isa<StructType>(AI->getAllocatedType())) {
        if (!isSafeStructAllocaToPromote(AI))
          continue;
      } else if (!isSafeArrayAllocaToPromote(AI))
        continue;

      DEBUG_WITH_TYPE("MYSROA", errs() << "Found inst to xform: " << *AI << "\n");
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
        const ArrayType *AT = dyn_cast<ArrayType>(AI->getAllocatedType());
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

/// isSafeUseOfAllocation - Check to see if this user is an allowed use for an
/// aggregate allocation
///
bool MYSROA::isSafeUseOfAllocation(Instruction *User) {
  Function *F = User->getParent()->getParent();
  if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(User)) {
    // The GEP is safe to transform if it is of the form GEP <ptr>, 0, <cst>
    if (GEPI->getNumOperands() <=2 || GEPI->getOperand(1) != Constant::getNullValue(Type::getInt64Ty(F->getContext())) || !isa<Constant>(GEPI->getOperand(2)) || isa<ConstantExpr>(GEPI->getOperand(2)))
      return false;
  } else {
    return false;
  }
  return true;
}

/// isSafeArrayElementUse - Check to see if this use is an allowd use for a
/// getelementptr instruction of an array aggregate allocation.
///
bool MYSROA::isSafeArrayElementUse(Value *Ptr) {
  for (Value::use_iterator I = Ptr->use_begin(), E = Ptr->use_end(); I != E; ++I) {
    Instruction *User = cast<Instruction>(*I);
    switch (User->getOpcode()) {
    case Instruction::Load: return true;
    case Instruction::Store: return User->getOperand(0) != Ptr;
    case Instruction::GetElementPtr: {
      GetElementPtrInst *GEP = cast<GetElementPtrInst>(User);
      if (GEP->getNumOperands() > 1) {
        if (!isa<Constant>(GEP->getOperand(1)) || !cast<Constant>(GEP->getOperand(1))->isNullValue())
          return false; // Using pointer arithmetic to navigate the array ...

        // Check to see if there are any structure indexes involved in this GEP.
        // If so, then we can safely break the array up until at least the
        // structure
        for (unsigned i = 2, e = GEP->getNumOperands(); i != e; ++i)
          if (GEP->getOperand(i)->getType()->isIntegerTy())
            break;
      }
      return isSafeArrayElementUse(GEP);
    }
    default:
      DEBUG_WITH_TYPE("MYSROA", errs() << " Transformation preventing inst: " << *User);
      return false;
    }
  }
  return true; // All users look ok 
}

/// isSafeStructAllocaToPromote - Check to see if the specified allocation of a
/// structure can be broken down into elements
bool MYSROA::isSafeStructAllocaToPromote(AllocaInst *AI) {
  // Loop over the use list of the alloca. We can only transform it if all
  // of the users are safe to transform
  //
  for (Value::use_iterator I = AI->use_begin(), E = AI->use_end(); I != E; ++I)
    if (!isSafeUseOfAllocation(cast<Instruction>(*I))) {
      DEBUG_WITH_TYPE("MYSROA", errs() << "Cannot transform: " << *AI << "  due to user: " << *I);
      return false;
    }
  return true;
}

/// isSafeArrayAllocaToPromote - Check to see if the specified allocation of a
/// structure can be broken down into elements
/// 
bool MYSROA::isSafeArrayAllocaToPromote(AllocaInst *AI) {
  const ArrayType *AT = cast<ArrayType>(AI->getAllocatedType());  
  int64_t NumElements = AT->getNumElements();

  // Loop Over the use list of the alloca. We can only transform it if all of
  // the users are safe to transform. Array allocas have extra contraints to
  // meet though.
  for (Value::use_iterator I = AI->use_begin(), E = AI->use_end(); I != E; ++I) {
    Instruction *User = cast<Instruction>(*I);
    if (!isSafeUseOfAllocation(User)) {
      return false;
    }

    // Check to make sure that getelementptr follow the extra rules for arrays:
    if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(User)) {
      // Check to make sure that index falls within the array. If not,
      // something funny is going on, so we won't do the optimization.
      //
      if ((cast<ConstantInt>(GEPI->getOperand(2))->getZExtValue() >= NumElements)) 
        return false;

      // Check to make sure that the only thing that uses the resultant pointer
      // is safe for an array access. For example, code that looks like:
      //   P = &A[0]; P = P + 1;
      // is legal, and should prevent promotion.
      //
      if (!isSafeArrayElementUse(GEPI)) {
        return false;
      }
    }
  }
  return true;
}
