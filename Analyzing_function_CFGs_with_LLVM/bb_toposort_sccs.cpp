//------------------------------------------------------------------------------
// bb_toposort_sccs LLVM sample. Demonstrates:
//
// * How to implement DFS & topological sort over the control-flow graph (CFG)
//   of a function.
// * How to use po_iterator for post-order iteration over basic blocks.
// * How to use scc_iterator for post-order iteration over strongly-connected
//   components in the graph of basic blocks.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/RegionPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Debug.h"
#include <string>
#include <vector>
using namespace llvm;


// Runs a topological sort on the basic blocks of the given function. Uses
// the simple recursive DFS from "Introduction to algorithms", with 3-coloring
// of vertices. The coloring enables detecting cycles in the graph with a simple
// test.
class TopoSorter {
public:
  void runToposort(const Function &F) {
    outs() << "Topological sort of " << F.getName() << ":\n";
    // Initialize the color map by marking all the vertices white.
    for (Function::const_iterator I = F.begin(), IE = F.end(); I != IE; ++I) {
      ColorMap[I] = TopoSorter::WHITE;
    }

    // The BB graph has a single entry vertex from which the other BBs should
    // be discoverable - the function entry block.
    bool success = recursiveDFSToposort(&F.getEntryBlock());
    if (success) {
      // Now we have all the BBs inside SortedBBs in reverse topological order.
      for (BBVector::const_reverse_iterator RI = SortedBBs.rbegin(),
                                            RE = SortedBBs.rend();
                                            RI != RE; ++RI) {
        outs() << "  " << (*RI)->getName() << "\n";
      }
    } else {
      outs() << "  Sorting failed\n";
    }
  }
private:
  enum Color {WHITE, GREY, BLACK};
  // Color marks per vertex (BB).
  typedef DenseMap<const BasicBlock *, Color> BBColorMap;
  // Collects vertices (BBs) in "finish" order. The first finished vertex is
  // first, and so on.
  typedef SmallVector<const BasicBlock *, 32> BBVector;
  BBColorMap ColorMap;
  BBVector SortedBBs;

  // Helper function to recursively run topological sort from a given BB.
  // Returns true if the sort succeeded and false otherwise; topological sort
  // may fail if, for example, the graph is not a DAG (detected a cycle).
  bool recursiveDFSToposort(const BasicBlock *BB) {
    ColorMap[BB] = TopoSorter::GREY;
    // For demonstration, using the lowest-level APIs here. A BB's successors
    // are determined by looking at its terminator instruction.
    const TerminatorInst *TInst = BB->getTerminator();
    for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I) {
      BasicBlock *Succ = TInst->getSuccessor(I);
      Color SuccColor = ColorMap[Succ];
      if (SuccColor == TopoSorter::WHITE) {
        if (!recursiveDFSToposort(Succ))
          return false;
      } else if (SuccColor == TopoSorter::GREY) {
        // This detects a cycle because grey vertices are all ancestors of the
        // currently explored vertex (in other words, they're "on the stack").
        outs() << "  Detected cycle: edge from " << BB->getName() << 
                  " to " << Succ->getName() << "\n";
        return false;
      }
    }
    // This BB is finished (fully explored), so we can add it to the vector.
    ColorMap[BB] = TopoSorter::BLACK;
    SortedBBs.push_back(BB);
    return true;
  }
};

class PrintRegionPass: public RegionPass {
private:
  std::string Banner;
  raw_ostream &Out;

public:
  static char ID;
  PrintRegionPass() : RegionPass(ID), Out(dbgs()) {}
  PrintRegionPass(const std::string &B, raw_ostream &o) :
    RegionPass(ID), Banner(B), Out(o) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }

  virtual bool runOnRegion(Region *R, RGPassManager &RGM) {
    Out << Banner;
    for (Region::block_iterator I = R->block_begin(), E = R->block_end();
         I != E; ++I) {
      (*I)->print(Out);
    }
    return false;
  }
};

char PrintRegionPass::ID = 0;

class AnalyzeBBGraph : public FunctionPass {
public:
  AnalyzeBBGraph(const std::string &AnalysisKind) 
    : FunctionPass(ID), AnalysisKind(AnalysisKind)
  {
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<DominatorTree>();
    AU.addRequired<LoopInfo>();
  }

  virtual bool runOnFunction(Function &F) {
    if (AnalysisKind == "-topo") {
      TopoSorter TS;
      TS.runToposort(F);
    } else if (AnalysisKind == "-po") {
      // Use LLVM's post-order iterator to produce a reverse topological sort.
      // Note that this doesn't detect cycles so if the graph is not a DAG, the
      // result is not a true topological sort.
      outs() << "Basic blocks of " << F.getName() << " in post-order:\n";
      for (po_iterator<BasicBlock *> I = po_begin(&F.getEntryBlock()),
                                     IE = po_end(&F.getEntryBlock());
                                     I != IE; ++I) {
        outs() << "  " << (*I)->getName() << "\n";
      }                        
    } else if (AnalysisKind == "-scc") {
      // Use LLVM's Strongly Connected Components (SCCs) iterator to produce
      // a reverse topological sort of SCCs.
      outs() << "SCCs for " << F.getName() << " in post-order:\n";
      for (scc_iterator<Function *> I = scc_begin(&F),
                                    IE = scc_end(&F);
                                    I != IE; ++I) {
        // Obtain the vector of BBs in this SCC and print it out.
        const std::vector<BasicBlock *> &SCCBBs = *I;
        outs() << "  SCC: ";
        for (std::vector<BasicBlock *>::const_iterator BBI = SCCBBs.begin(),
                                                       BBIE = SCCBBs.end();
                                                       BBI != BBIE; ++BBI) {
          outs() << (*BBI)->getName() << "  ";
        }
        outs() << "\n";
      }
    } else if (AnalysisKind == "-pred") {
      // Use LLVM's pred_iterator to produce all the predecessors for a
      // function's exit basic block.
      BasicBlock *the_exit_bb = &(F.back());
      outs() << "All the predecessors for \"" << the_exit_bb->getName() << "\" are\n";
      for (pred_iterator i = pred_begin(the_exit_bb), e =
              pred_end(the_exit_bb); i != e; ++i) {
        outs() << (*i)->getName() << "   ";
      }
      outs() << "\n";
      F.viewCFG();
    } else if (AnalysisKind == "-loop") {
      // Print LLVM's LoopInfo to print all loops to make sure LoopInfo pass
      // is used correctly.
      LoopInfo *li = &getAnalysis<LoopInfo>();
    
    } 
    else {
      outs() << "Unknown analysis kind: " << AnalysisKind << "\n";
    }
    return false;
  }

  // The address of this member is used to uniquely identify the class. This is
  // used by LLVM's own RTTI mechanism.
  static char ID;
private:
  std::string AnalysisKind;
};

char AnalyzeBBGraph::ID = 0;
PassRegistry &Registry = *PassRegistry::getPassRegistry();
INITIALIZE_PASS_BEGIN(AnalyzeBBGraph, "Analyze BB Graph", 
                      "Analyze BB Graph", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTree)
INITIALIZE_PASS_DEPENDENCY(LoopInfo)
INITIALIZE_PASS_END(AnalyzeBBGraph, "Analyze BB Graph", 
                    "Analyze BB Graph", false, false)

int main(int argc, char **argv) {
  if (argc < 3) {
    // Using very basic command-line argument parsing here...
    errs() << "Usage: " << argv[0] << " -[topo|po|scc|pred|loop] <IR file>\n";
    return 1;
  }

  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  Module *Mod = ParseIRFile(argv[2], Err, getGlobalContext());
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }
  Function *Func = Mod->getFunction("test");


  // Create a pass manager and fill it with the passes we want to run.
  PassManager PM;
  PM.add(new AnalyzeBBGraph(std::string(argv[1])));
  PM.run(*Mod);
#if 0
  std::string Banner("Print Region Pass\n");
  PrintRegionPass *PRP = new PrintRegionPass();

  RGPassManager RGPM;
  RGPM.add(PRP);
  RGPM.runOnFunction(*Func);
#endif
  return 0;
}
