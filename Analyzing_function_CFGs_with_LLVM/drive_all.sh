#!/bin/bash
rm -f *.dot *.ll

LLVM_BIN=/home/jiading/LLVM/llvm-3.3.build/Debug+Asserts/bin/
TEST=sample
$LLVM_BIN/clang -S -O0 -emit-llvm -o $TEST.ll -x cl $TEST.cl
$LLVM_BIN/opt -S -mem2reg < $TEST.ll > $TEST.2.ll 
$LLVM_BIN/opt -S -dot-cfg < $TEST.2.ll

xdot cfg.test.dot &

./bb_toposort_sccs -topo $TEST.2.ll
./bb_toposort_sccs -po $TEST.2.ll
./bb_toposort_sccs -scc $TEST.2.ll
./bb_toposort_sccs -pred $TEST.2.ll
