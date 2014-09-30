; ModuleID = 'sample.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test(i32* %A, i32* %B, i32* %C, i32 %n) #0 {
entry:
  %A.addr = alloca i32*, align 8
  %B.addr = alloca i32*, align 8
  %C.addr = alloca i32*, align 8
  %n.addr = alloca i32, align 4
  %foo1 = alloca i32, align 4
  %foo2 = alloca i32, align 4
  %foo3 = alloca i32, align 4
  %foo4 = alloca i32, align 4
  %k = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %A, i32** %A.addr, align 8
  store i32* %B, i32** %B.addr, align 8
  store i32* %C, i32** %C.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32** %A.addr, align 8
  %arrayidx = getelementptr inbounds i32* %0, i64 0
  %1 = load i32* %arrayidx, align 4
  %2 = load i32** %B.addr, align 8
  %arrayidx1 = getelementptr inbounds i32* %2, i64 0
  %3 = load i32* %arrayidx1, align 4
  %add = add nsw i32 %1, %3
  %4 = load i32** %C.addr, align 8
  %arrayidx2 = getelementptr inbounds i32* %4, i64 0
  store i32 %add, i32* %arrayidx2, align 4
  store i32 0, i32* %k, align 4
  br label %do.body

do.body:                                          ; preds = %do.cond39, %entry
  %call = call i32 (i32, ...)* bitcast (i32 (...)* @get_global_id to i32 (i32, ...)*)(i32 0)
  store i32 %call, i32* %i, align 4
  %5 = load i32* %n.addr, align 4
  %6 = load i32* %foo2, align 4
  %cmp = icmp sgt i32 %5, %6
  br i1 %cmp, label %if.then, label %if.else21

if.then:                                          ; preds = %do.body
  %7 = load i32* %i, align 4
  %idxprom = sext i32 %7 to i64
  %8 = load i32** %C.addr, align 8
  %arrayidx3 = getelementptr inbounds i32* %8, i64 %idxprom
  %9 = load i32* %arrayidx3, align 4
  %mul = mul nsw i32 2, %9
  %10 = load i32* %i, align 4
  %idxprom4 = sext i32 %10 to i64
  %11 = load i32** %C.addr, align 8
  %arrayidx5 = getelementptr inbounds i32* %11, i64 %idxprom4
  store i32 %mul, i32* %arrayidx5, align 4
  %12 = load i32* %i, align 4
  %13 = load i32* %foo3, align 4
  %cmp6 = icmp sgt i32 %12, %13
  br i1 %cmp6, label %if.then7, label %if.else

if.then7:                                         ; preds = %if.then
  %14 = load i32* %i, align 4
  %idxprom8 = sext i32 %14 to i64
  %15 = load i32** %A.addr, align 8
  %arrayidx9 = getelementptr inbounds i32* %15, i64 %idxprom8
  %16 = load i32* %arrayidx9, align 4
  %17 = load i32* %i, align 4
  %idxprom10 = sext i32 %17 to i64
  %18 = load i32** %B.addr, align 8
  %arrayidx11 = getelementptr inbounds i32* %18, i64 %idxprom10
  %19 = load i32* %arrayidx11, align 4
  %add12 = add nsw i32 %16, %19
  %20 = load i32* %i, align 4
  %idxprom13 = sext i32 %20 to i64
  %21 = load i32** %C.addr, align 8
  %arrayidx14 = getelementptr inbounds i32* %21, i64 %idxprom13
  store i32 %add12, i32* %arrayidx14, align 4
  br label %if.end

if.else:                                          ; preds = %if.then
  %22 = load i32* %i, align 4
  %idxprom15 = sext i32 %22 to i64
  %23 = load i32** %A.addr, align 8
  %arrayidx16 = getelementptr inbounds i32* %23, i64 %idxprom15
  %24 = load i32* %arrayidx16, align 4
  %25 = load i32* %i, align 4
  %idxprom17 = sext i32 %25 to i64
  %26 = load i32** %B.addr, align 8
  %arrayidx18 = getelementptr inbounds i32* %26, i64 %idxprom17
  %27 = load i32* %arrayidx18, align 4
  %sub = sub nsw i32 %24, %27
  %28 = load i32* %i, align 4
  %idxprom19 = sext i32 %28 to i64
  %29 = load i32** %C.addr, align 8
  %arrayidx20 = getelementptr inbounds i32* %29, i64 %idxprom19
  store i32 %sub, i32* %arrayidx20, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then7
  br label %if.end37

if.else21:                                        ; preds = %do.body
  br label %do.body22

do.body22:                                        ; preds = %do.cond, %if.else21
  %30 = load i32* %k, align 4
  %idxprom23 = sext i32 %30 to i64
  %31 = load i32** %A.addr, align 8
  %arrayidx24 = getelementptr inbounds i32* %31, i64 %idxprom23
  %32 = load i32* %arrayidx24, align 4
  %33 = load i32* %k, align 4
  %idxprom25 = sext i32 %33 to i64
  %34 = load i32** %B.addr, align 8
  %arrayidx26 = getelementptr inbounds i32* %34, i64 %idxprom25
  %35 = load i32* %arrayidx26, align 4
  %add27 = add nsw i32 %32, %35
  %36 = load i32* %k, align 4
  %idxprom28 = sext i32 %36 to i64
  %37 = load i32** %C.addr, align 8
  %arrayidx29 = getelementptr inbounds i32* %37, i64 %idxprom28
  store i32 %add27, i32* %arrayidx29, align 4
  %38 = load i32* %i, align 4
  %39 = load i32* %k, align 4
  %cmp30 = icmp slt i32 %38, %39
  br i1 %cmp30, label %if.then31, label %if.else33

if.then31:                                        ; preds = %do.body22
  %40 = load i32* %i, align 4
  %mul32 = mul nsw i32 2, %40
  store i32 %mul32, i32* %k, align 4
  br label %if.end35

if.else33:                                        ; preds = %do.body22
  %41 = load i32* %k, align 4
  %mul34 = mul nsw i32 2, %41
  store i32 %mul34, i32* %k, align 4
  br label %if.end35

if.end35:                                         ; preds = %if.else33, %if.then31
  br label %do.cond

do.cond:                                          ; preds = %if.end35
  %42 = load i32* %k, align 4
  %43 = load i32* %foo4, align 4
  %cmp36 = icmp slt i32 %42, %43
  br i1 %cmp36, label %do.body22, label %do.end

do.end:                                           ; preds = %do.cond
  br label %if.end37

if.end37:                                         ; preds = %do.end, %if.end
  %44 = load i32* %k, align 4
  %add38 = add nsw i32 %44, 2
  store i32 %add38, i32* %k, align 4
  br label %do.cond39

do.cond39:                                        ; preds = %if.end37
  %45 = load i32* %n.addr, align 4
  %46 = load i32* %foo1, align 4
  %cmp40 = icmp slt i32 %45, %46
  br i1 %cmp40, label %do.body, label %do.end41

do.end41:                                         ; preds = %do.cond39
  ret void
}

declare i32 @get_global_id(...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*, i32*, i32*, i32)* @test}
