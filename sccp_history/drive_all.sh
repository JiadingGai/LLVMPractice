#!/bin/bash
make
#gdb --args opt -S -load ./mysccp.so -mysccp -debug-only=mysccp test/2003-08-26-InvokeHandling.ll 
opt -S -load ./mysccp.so -mysccp -debug-only=mysccp < test/2003-08-26-InvokeHandling.ll
#gcov mysccp.cpp
