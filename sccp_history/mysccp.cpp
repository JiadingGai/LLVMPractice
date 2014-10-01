#define DEBUG_TYPE "mysccp" 

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InstVisitor.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include <set>
#include <algorithm>

using namespace llvm;

namespace {
  STATISTIC(NumInstRemoved, "Number of instructions removed");
 
  class InstVal {
    enum {undefined, constant, overdefined} LatticeValue;
    Constant *ConstantVal;
    
  public:
    inline InstVal() : LatticeValue(undefined), ConstantVal(0) {}

    inline bool markOverdefined() {
      if (LatticeValue != overdefined) {
        LatticeValue = overdefined;
        return true;
      }
      return false;
    }

    inline bool markConstant(Constant *V) {
      if (LatticeValue != constant) {
        LatticeValue = constant;
        ConstantVal = V;
        return true;
      } else {
        assert(ConstantVal == V && "Marking constant with different value");
      }
      return false;
    }

    inline bool isUndefined() const { 
      return LatticeValue == undefined;
    }

    inline bool isConstant() const {
      return LatticeValue == constant;
    }

    inline bool isOverdefined() const {
      return LatticeValue == overdefined;
    }

    inline Constant *getConstant() const {
      return ConstantVal;
    }
  };
}

namespace {
  class MYSCCP : public FunctionPass, public InstVisitor<MYSCCP> {
    std::set<BasicBlock*> BBExecutable;
    std::map<Value*, InstVal> ValueState;

    std::vector<Instruction*> InstWorkList;
    std::vector<BasicBlock*> BBWorkList;
  public: 
    static char ID;

    MYSCCP() : FunctionPass(ID) {
    }

    bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesCFG();
    }

  private:
    friend class InstVisitor<MYSCCP>;

    inline bool markConstant(Instruction *I, Constant *V) {
      if (ValueState[I].markConstant(V)) {
        DEBUG_WITH_TYPE("mysccp", errs() << "markConstant: " << V << " = " << I);
        InstWorkList.push_back(I);
        return true;
      }
      return false;
    }

    inline bool markOverdefined(Value *V) {
      if (ValueState[V].markOverdefined()) {
        if (Instruction *I = dyn_cast<Instruction>(V)) {
          DEBUG_WITH_TYPE("mysccp", errs() << "markOverdefined " << V);
          InstWorkList.push_back(I);
        }
        return true;
      }
      return false;
    }

    inline InstVal& getValueState(Value *V) {
      std::map<Value*, InstVal>::iterator I = ValueState.find(V);
      if (I != ValueState.end()) 
        return I->second;

      if (Constant *CPV = dyn_cast<Constant>(V)) {
         ValueState[CPV].markConstant(CPV); 
      } else if (isa<Argument>(V)) {
         ValueState[V].markOverdefined();
      } else if (GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
         //FIXME: how do you get the address of a global
         // The address of a global is a constant ...
         // ValueState[GV].markConstant();
      }

      return ValueState[V];
    }

    void markExecutable(BasicBlock *BB) {
      if (BBExecutable.count(BB)) {
        for (BasicBlock::iterator I = BB->begin(); PHINode *PN = dyn_cast<PHINode>(I); ++I) {
          visitPHINode(*PN);
        }
      } else {
        DEBUG_WITH_TYPE("mysccp", errs() << "Marking BB executable: " << *BB);
        BBExecutable.insert(BB);
        BBWorkList.push_back(BB);
      }
    }

    void visitPHINode(PHINode &I);

    void visitReturnInst(ReturnInst &I);
    void visitTerminatorInst(TerminatorInst &TI);
   
    void visitCastInst(CastInst &I);
    void visitBinaryOperator(Instruction &I);
    //FIXME: void visitShiftInst(ShiftInst &I) { visitBinaryOperator(I); }

    void visitStoreInst(Instruction &I) {}
    void visitLoadInst(Instruction &I) { markOverdefined(&I); }
    void visitGetElementPtrInst(GetElementPtrInst &I);
    void visitCallInst(Instruction &I) { markOverdefined(&I); }
    void visitInvokeInst(TerminatorInst &I) { 
      markOverdefined(&I); 
      visitTerminatorInst(I);
    }
    
    void visitAllocaInst(Instruction &I) { markOverdefined(&I); }
    void visitVarArgInst(Instruction &I) { markOverdefined(&I); }
    void visitFreeInst(Instruction &I) {}

    void visitInstruction(Instruction &I) { 
      errs() << "[Warn] " << "SCCP: Don't know how to handle: " << I;
      markOverdefined(&I);
    }
    
    void getFeasibleSuccessors(TerminatorInst &TI, std::vector<bool> &Succs);

    void isEdgeFeasible(BasicBlock *From, BasicBlock *To);

    void OperandChangedState(User *U) {
      Instruction &I = cast<Instruction>(*U);
      if (BBExecutable.count(I.getParent()))
        visit(I);
    }
  };

  RegisterPass<MYSCCP> X("mysccp", "my sparse condidtional constant propagation");
} // end anonymous namespace

char MYSCCP::ID = 0;

bool MYSCCP::runOnFunction(Function &F) {
  bool Changed = false;
  markExecutable(&F.front());


  return Changed;
}

void MYSCCP::getFeasibleSuccessors(TerminatorInst &TI, std::vector<bool> &Succs) 
{
  Succs.resize(TI.getNumSuccessors());

  if (BranchInst *BI = dyn_cast<BranchInst>(&TI)) {
    if (BI->isUnconditional()) {
      Succs[0] = true;
    } else {
      InstVal &BCValue = getValueState(BI->getCondition());
      if (BCValue.isOverdefined()) {
        Succs[0] = Succs[1] = true;
      } else if (BCValue.isConstant()) {
        #if 0
        Succs[BCValue.getConstant() == ConstantBool::False] = true;
        #else
        Succs[BCValue.getConstant()->isZeroValue()] = true;
        #endif
      }
    }
  } else if (InvokeInst *II = dyn_cast<InvokeInst>(&TI)) {
    Succs[0] = Succs[1] = true;
  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(&TI)) {
    InstVal &SCValue = getValueState(SI->getCondition());
    if (SCValue.isOverdefined()) {
      Succs.assign(TI.getNumSuccessors(), true);
    } else if (SCValue.isConstant()) {
      Constant *CPV = SCValue.getConstant();
      for (unsigned i = 1, E = SI->getNumSuccessors(); i != E; ++i) {
        if (SI->getSuccessorValue(i) == CPV) {
          Succs[i] = true;
          return;
        }
      }
      Succs[0] = true;
    }
  } else {
    errs() << "MYSCCP: Don't know how to handle: " << TI;
    Succs.assign(TI.getNumSuccessors(), true);
  }
}

void isEdgeFeasible(BasicBlock *From, BasicBlock *To)
{

}

void MYSCCP::visitPHINode(PHINode &I) 
{

}

void MYSCCP::visitTerminatorInst(TerminatorInst &TI) 
{

}

void MYSCCP::visitCastInst(CastInst &I) 
{

}

void MYSCCP::visitBinaryOperator(Instruction &I) 
{

}

void MYSCCP::visitGetElementPtrInst(GetElementPtrInst &I) 
{

}
