#define DEBUG_TYPE "mxpa_lva"

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/CFG.h"
// MXPA
#include "LVA.h"

// std
#include <utility>

using namespace llvm;

namespace LLVMPractice {

  // 
  void LiveVariableAnalysis::initInstrToLatticeBit(Function &F) 
  {
    unsigned InstrIdx = 0;

    for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I)
    {
      instToLatticeBit.insert(std::make_pair(&*I, InstrIdx));
      InstrIdx++;
    }

    for (Function::iterator ib = F.begin(), eb = F.end(); ib != eb; ++ib)
    {
      BasicBlock *BB = &*ib;
      for (BasicBlock::iterator iI = BB->begin(), iE = BB->end(); iI != iE; ++iI) 
      {
        Instruction *I = &*iI;
        instToLatticeBit.insert(std::make_pair(&*I, InstrIdx));
        InstrIdx++;
      }
    }
  }
  
  void LiveVariableAnalysis::initBlocksInfo(unsigned defcount, Function &F)
  {
    for (Function::iterator ib = F.begin(), ie = F.end(); ib != ie; ++ib)
    {
      BasicBlock *BB = &*ib;
      struct BasicBlockLivenessInfo *BBLI = new BasicBlockLivenessInfo(BB, defcount);
      blockToInfo.insert(std::make_pair(&*ib, BBLI));
    }
  }
  
  void LiveVariableAnalysis::initBlockDef(BasicBlockLivenessInfo *blockInfo)
  {
    BasicBlock *BB = blockInfo->block;

    for (BasicBlock::iterator iI = BB->begin(), iE = BB->end(); iI != iE; ++iI)
    {
      blockInfo->def->set(getLatticeBit(&*iI));  
    }
  }
  
  void LiveVariableAnalysis::initEntryArgDef(BasicBlockLivenessInfo *blockInfo, unsigned argCount)
  {
    for (unsigned i = 0; i < argCount; ++i)
    {
      blockInfo->def->set(i);
    }
  }
  
  void LiveVariableAnalysis::markUses(Value::use_iterator use_begin, Value::use_iterator use_end, Instruction *defInst, unsigned defBit) 
  {
    for (Value::use_iterator u = use_begin, e = use_end; u != e; ++u)
    {
      Instruction *useInst = dyn_cast<Instruction>(*u);
      if (!useInst) continue;
      BasicBlock *useBlock = useInst->getParent();

      if (!defInst) {
        if (defInst->getParent() == useBlock && !isa<PHINode>(useInst)) {
          continue;
        }
      }

      struct BasicBlockLivenessInfo *useBBLI = blockToInfo[useBlock];
      useBBLI->use->set(getLatticeBit(defInst));
    }
  
  }
  
  void LiveVariableAnalysis::initBlocksUse(Function &F)
  {
    for (Function::iterator ib = F.begin(), ie = F.end(); ib != ie; ++ib)
    {
      BasicBlock *BB = &*ib;
      for (BasicBlock::iterator iI = BB->begin(), iE = BB->end(); iI != iE; ++iI)
      {
        Instruction *I = &*iI;
        markUses(I->use_begin(), I->use_end(), I, getLatticeBit(I));
      }
    }

    for (Function::arg_iterator ia = F.arg_begin(), ie = F.arg_end(); ia != ie; ++ia)
    {
      markUses(ia->use_begin(), ia->use_end(), NULL, getLatticeBit(&*ia));
    }
  }
  
  BitVector LiveVariableAnalysis::getMaskPhiValues(BasicBlock *phiBlock)
  {
    BitVector mask = BitVector(instToLatticeBit.size(), true);
    for (BasicBlock::iterator instIt = phiBlock->begin(); instIt != phiBlock->end(); instIt++) {
      if (!isa<PHINode>(&*instIt))
        continue;
        PHINode *phiNode = dyn_cast<PHINode>(&*instIt);
        for (unsigned crtIncomingIdx = 0; crtIncomingIdx < phiNode->getNumIncomingValues(); crtIncomingIdx++) {
            Value *crtValue = phiNode->getIncomingValue(crtIncomingIdx);
            Instruction *crtInst = dyn_cast<Instruction>(crtValue);
            if (!crtInst) continue;
            BasicBlock *defBlock = crtInst->getParent();

            // check if the incoming value is only used in a phi instr
            bool usedInNonPhi = false;
            for (Value::use_iterator useIt = crtInst->use_begin(); useIt != crtInst->use_end(); useIt++) {
                Instruction *useInst = dyn_cast<Instruction>(*useIt);
                if (!useInst) continue;
                BasicBlock *useBlock = useInst->getParent();
                // use/def in the same basic block hides upwards use
                if (useBlock == defBlock) continue;
                // ignore phi uses
                if (isa<PHINode>(useInst)) continue;
                // should be in the same basic block as phi
                if (useBlock != phiBlock) continue;

                usedInNonPhi = true;
            }

            if (!usedInNonPhi) {
                mask.reset( getLatticeBit(crtValue) );
            }
        }
    }
    return mask; 
  
  }
  
  void LiveVariableAnalysis::initMask(FlowMask &mask, Function &F)
  {
     for (Function::iterator blockIt = F.begin(); blockIt != F.end(); blockIt++) {
        BasicBlock *phiBlock = &*blockIt;

        BitVector initialMask = getMaskPhiValues(phiBlock);

        // now that we have masked out all the phi dependent uses, create a mask
        // for each crtBlock -> phiBlock control flow
        // ie.  %i = phi i32 [ %b, %bb1 ], [ %c, %bb2 ]
        // we should change the mask for %b to 1 for %bb1 -> phiBlock
        // and leave the mask for %b at 0 for %bb2 -> phiBlock
        for (BasicBlock::iterator instIt = phiBlock->begin(); instIt != phiBlock->end(); instIt++) {
            if (!isa<PHINode>(&*instIt))
                continue;
            PHINode *phiNode = dyn_cast<PHINode>(&*instIt);
            for (unsigned crtIncomingIdx = 0; crtIncomingIdx < phiNode->getNumIncomingValues(); crtIncomingIdx++) {
                BasicBlock *crtBlock = phiNode->getIncomingBlock(crtIncomingIdx);
                Value *crtValue = phiNode->getIncomingValue(crtIncomingIdx);
                
                if (!isa<Instruction>(crtValue)) 
                  continue;

                std::pair<BasicBlock *, BasicBlock *> maskKey = std::make_pair(crtBlock, phiBlock);
                std::map<std::pair<BasicBlock *, BasicBlock *>, BitVector *>::iterator maskIt = mask.find(maskKey);
                if (maskIt == mask.end()) {
                    mask[maskKey] = new BitVector(initialMask);
                }
                mask[maskKey]->set(getLatticeBit(crtValue));
            }
        }
    } 
  }
  
  void LiveVariableAnalysis::printBitVector(BitVector *bv)
  {
    for (unsigned i = 0; i < bv->size(); ++i)
    {
      errs() << (bv->test(i) ? '1' : '0');
    }
  }
  
  void LiveVariableAnalysis::printBitVectorDetailed(BitVector *bv)
  {
     for (unsigned bit = 0; bit < bv->size(); bit++) {
        Value *instr = 0;
        for (LatticeEncoding::iterator it = instToLatticeBit.begin(); it != instToLatticeBit.end(); it++) {
            if (it->second == bit) {
                instr = it->first;
            }
        }
        assert(instr);
        dbgs() << (bv->test(bit) ? '1' : '0') << " - " << *instr << "\n";
    } 
  }
  
  static RegisterPass<LiveVariableAnalysis> X("practice_lva", "Live Variable Analysis");
  
  char LiveVariableAnalysis::ID = 0;
  
  bool LiveVariableAnalysis::doInitialization(Module &M) 
  {
    Mod = &M;
    return false;
  }

  bool LiveVariableAnalysis::runOnFunction(Function &F)
  {
    //1. Initialize instToLatticeBit
    instToLatticeBit.clear();
    initInstrToLatticeBit(F);
    unsigned defcount = instToLatticeBit.size();
  
    DEBUG(errs() << "total defcount is " << defcount << "\n");

    //2. Initialize blockToInfo
    for (BlockInfoMapping::iterator ib = blockToInfo.begin(), ie = blockToInfo.end(); ib != ie; ++ib) 
    {
      delete ib->second;
    }
    blockToInfo.clear();
    initBlocksInfo(defcount, F);

    //3. Initialize def and use sets
    initEntryArgDef(blockToInfo[&*F.begin()], F.arg_size());
    for (BlockInfoMapping::iterator BIMit = blockToInfo.begin(), BIMie = blockToInfo.end(); BIMit != BIMie; ++BIMit)
    {
      initBlockDef(BIMit->second);
    }
    initBlocksUse(F);

    //4. Flowmask
    FlowMask mask;
    initMask(mask, F);

    // compute fixed-point liveness information - no sorting of
    // blocks in quasi-topological order, works anyway
    bool inChanged = true;
    while (inChanged) {
        inChanged = false;
        // out[B] = U(in[S] & mask[B][S]) where B < S
        for (Function::iterator B = F.begin(); B != F.end(); B++) {
            (blockToInfo[&*B]->out)->reset();
            for (succ_iterator succIt = succ_begin(&*B); succIt != succ_end(&*B); succIt++) {
                BasicBlock *S = *succIt;
                std::pair<BasicBlock *, BasicBlock *> key = std::make_pair(&*B, S);
                if (mask.find(key) != mask.end()) {
                    *(blockToInfo[&*B]->out) |= ((*(blockToInfo[S]->in)) &= (*(mask[key])));
                } else {
                    *(blockToInfo[&*B]->out) |= *(blockToInfo[S]->in);
                }
            }
            // in[B] = use[B] U (out[B] - def[B])
            BitVector oldIn = *(blockToInfo[&*B]->in);
            //*(blockToInfo[B]->in) = (*(blockToInfo[B]->use) | (*(blockToInfo[B]->out) & ~(*(blockToInfo[B]->def))));
            if (*(blockToInfo[&*B]->in) != oldIn) {
                inChanged = true;
            }
        }
    }

    for (FlowMask::iterator i = mask.begin(), e = mask.end(); i != e; ++i) {
        delete i->second;
    }

    return true;
  }

  bool LiveVariableAnalysis::doFinalization(Module &M)
  {
    for (BlockInfoMapping::iterator it = blockToInfo.begin(); it != blockToInfo.end(); it++) {
        delete it->second;
    }
    return false; 
  }
}
