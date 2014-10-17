#!/bin/bash
make
#gdb --args opt -S -load ./mysroa.so -mysroa -debug-only=MYSROA test/2014-09-23-gai.ll 
opt -S -load ./mysroa.so -mysroa -debug-only=MYSROA < test/2014-09-23-gai.ll
#opt -S -load ./mysroa.so -mysroa -debug-only=MYSROA < test/2003-05-29-ArrayFail.ll
#opt -S -load ./mysroa.so -mysroa -debug-only=MYSROA < test/arraytest.ll
#gcov mysroa.cpp
