#ifndef MXPA_BARRIER_INST_H
#define MXPA_BARRIER_INST_H

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"

using namespace llvm;

// BarrierInst - A useful wrapper class for inspecting calls to barrier 
// functions. This allows the standard isa/dyncast/cast functionality to
// work with calls to barrier functions.
// FIXME: will barrier become an extended barrier intrinsic some day?
class BarrierInst: public CallInst {
  BarrierInst() LLVM_DELETED_FUNCTION;
  BarrierInst(const BarrierInst&) LLVM_DELETED_FUNCTION;
  void operator=(const BarrierInst&) LLVM_DELETED_FUNCTION;
  int BarrierKind;// always assumed conditional until proven otherwise.
  static StringRef barrier_func_nm;
public:
  /// getBarrierID - Return the barrier ID of this barrier call
  Intrinsic::ID getIntrinsicID() const {
    return (Intrinsic::ID)getCalledFunction()->getIntrinsicID();
  }

  // Methods for support type inquiry through isa, cast, and dyn_cast
  static inline bool classof(const CallInst *I) {
    if (const Function *CF = I->getCalledFunction()) {
      if (CF->getName() == barrier_func_nm) {
        return true;
      }
    }
    return false;
  }

  static inline bool classof(const Value *V) {
    return isa<CallInst>(V) && classof(cast<CallInst>(V));
  }

  // Methods for support barrier type inquiry
  int getBarrierKind() const { return BarrierKind; }
  void setBarrierKind(int Bk)  { 
    BarrierKind = Bk;
    return;
  }
 
  // Construct a BarrierInst given a range of arguments
  static BarrierInst *createBarrier(Value *Source, Instruction *InsertBefore, 
                                    BasicBlock *InsertAtEnd = NULL) 
  {
    assert(((!InsertBefore && InsertAtEnd) || (InsertBefore && !InsertAtEnd)) &&
          "createBarrier needs either InsertBefore or InsertAtEnd");

    BasicBlock *BB = InsertBefore ? InsertBefore->getParent() : InsertAtEnd;
    Module *M = BB->getParent()->getParent();

    // FIXME: What if the barrier to be inserted already exists?
#if 0  
    if (InsertBefore != &InsertBefore->getParent()->front() && 
        isa<BarrierInst>(InsertBefore->getPrevNode()))
      return cast<BarrierInst>(InsertBefore->getPrevNode());
#endif

    Type *VoidTy = Type::getVoidTy(M->getContext());
    Type *Int32Ty = Type::getInt32Ty(M->getContext());
    // prototype barrier as "void barrier(int)"
    Value *BarrierFunc = M->getOrInsertFunction(barrier_func_nm, VoidTy, 
                                                Int32Ty, NULL);
    CallInst *Result = NULL;
    Value *Int32Cast = Source;
    if (InsertBefore) {
      if (Source->getType() != Int32Ty) {
        Int32Cast = new BitCastInst(Source, Int32Ty, "", InsertBefore);
      }
      Result = CallInst::Create(BarrierFunc, Int32Cast, "", InsertBefore);
    } else {
      if (Source->getType() != Int32Ty) {
        Int32Cast = new BitCastInst(Source, Int32Ty, "", InsertAtEnd);
      }
      Result = CallInst:: Create(BarrierFunc, Int32Cast, "");
    }
    
    return cast<BarrierInst>(Result);
  }

  static bool hasOnlyBarrier(const llvm::BasicBlock *bb) {
      return endsWithBarrier(bb) && bb->size() == 2;
  }

  static bool hasBarrier(const llvm::BasicBlock *bb)
  {
    for (llvm::BasicBlock::const_iterator i = bb->begin(), e = bb->end();
         i != e; ++i)
      {
        if (llvm::isa<BarrierInst>(i)) return true;
      }
    return false;
  }

  static bool endsWithBarrier(const BasicBlock *BB) 
  {
    const TerminatorInst *TI = BB->getTerminator();
    if (TI == NULL) return false;
    return BB->size() > 1 && TI->getPrevNode() != NULL && 
        isa<BarrierInst>(TI->getPrevNode());
  }
};

#endif
