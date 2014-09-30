; ModuleID = '<stdin>'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test(i32* %A, i32* %B, i32* %C, i32 %n) #0 {
entry:
  %arrayidx = getelementptr inbounds i32* %A, i64 0
  %0 = load i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32* %B, i64 0
  %1 = load i32* %arrayidx1, align 4
  %add = add nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds i32* %C, i64 0
  store i32 %add, i32* %arrayidx2, align 4
  br label %do.body

do.body:                                          ; preds = %do.cond39, %entry
  %k.0 = phi i32 [ 0, %entry ], [ %add38, %do.cond39 ]
  %call = call i32 (i32, ...)* bitcast (i32 (...)* @get_global_id to i32 (i32, ...)*)(i32 0)
  %cmp = icmp sgt i32 %n, undef
  br i1 %cmp, label %if.then, label %if.else21

if.then:                                          ; preds = %do.body
  %idxprom = sext i32 %call to i64
  %arrayidx3 = getelementptr inbounds i32* %C, i64 %idxprom
  %2 = load i32* %arrayidx3, align 4
  %mul = mul nsw i32 2, %2
  %idxprom4 = sext i32 %call to i64
  %arrayidx5 = getelementptr inbounds i32* %C, i64 %idxprom4
  store i32 %mul, i32* %arrayidx5, align 4
  %cmp6 = icmp sgt i32 %call, undef
  br i1 %cmp6, label %if.then7, label %if.else

if.then7:                                         ; preds = %if.then
  %idxprom8 = sext i32 %call to i64
  %arrayidx9 = getelementptr inbounds i32* %A, i64 %idxprom8
  %3 = load i32* %arrayidx9, align 4
  %idxprom10 = sext i32 %call to i64
  %arrayidx11 = getelementptr inbounds i32* %B, i64 %idxprom10
  %4 = load i32* %arrayidx11, align 4
  %add12 = add nsw i32 %3, %4
  %idxprom13 = sext i32 %call to i64
  %arrayidx14 = getelementptr inbounds i32* %C, i64 %idxprom13
  store i32 %add12, i32* %arrayidx14, align 4
  br label %if.end

if.else:                                          ; preds = %if.then
  %idxprom15 = sext i32 %call to i64
  %arrayidx16 = getelementptr inbounds i32* %A, i64 %idxprom15
  %5 = load i32* %arrayidx16, align 4
  %idxprom17 = sext i32 %call to i64
  %arrayidx18 = getelementptr inbounds i32* %B, i64 %idxprom17
  %6 = load i32* %arrayidx18, align 4
  %sub = sub nsw i32 %5, %6
  %idxprom19 = sext i32 %call to i64
  %arrayidx20 = getelementptr inbounds i32* %C, i64 %idxprom19
  store i32 %sub, i32* %arrayidx20, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then7
  br label %if.end37

if.else21:                                        ; preds = %do.body
  br label %do.body22

do.body22:                                        ; preds = %do.cond, %if.else21
  %k.1 = phi i32 [ %k.0, %if.else21 ], [ %k.2, %do.cond ]
  %idxprom23 = sext i32 %k.1 to i64
  %arrayidx24 = getelementptr inbounds i32* %A, i64 %idxprom23
  %7 = load i32* %arrayidx24, align 4
  %idxprom25 = sext i32 %k.1 to i64
  %arrayidx26 = getelementptr inbounds i32* %B, i64 %idxprom25
  %8 = load i32* %arrayidx26, align 4
  %add27 = add nsw i32 %7, %8
  %idxprom28 = sext i32 %k.1 to i64
  %arrayidx29 = getelementptr inbounds i32* %C, i64 %idxprom28
  store i32 %add27, i32* %arrayidx29, align 4
  %cmp30 = icmp slt i32 %call, %k.1
  br i1 %cmp30, label %if.then31, label %if.else33

if.then31:                                        ; preds = %do.body22
  %mul32 = mul nsw i32 2, %call
  br label %if.end35

if.else33:                                        ; preds = %do.body22
  %mul34 = mul nsw i32 2, %k.1
  br label %if.end35

if.end35:                                         ; preds = %if.else33, %if.then31
  %k.2 = phi i32 [ %mul32, %if.then31 ], [ %mul34, %if.else33 ]
  br label %do.cond

do.cond:                                          ; preds = %if.end35
  %cmp36 = icmp slt i32 %k.2, undef
  br i1 %cmp36, label %do.body22, label %do.end

do.end:                                           ; preds = %do.cond
  br label %if.end37

if.end37:                                         ; preds = %do.end, %if.end
  %k.3 = phi i32 [ %k.0, %if.end ], [ %k.2, %do.end ]
  %add38 = add nsw i32 %k.3, 2
  br label %do.cond39

do.cond39:                                        ; preds = %if.end37
  %cmp40 = icmp slt i32 %n, undef
  br i1 %cmp40, label %do.body, label %do.end41

do.end41:                                         ; preds = %do.cond39
  ret void
}

declare i32 @get_global_id(...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*, i32*, i32*, i32)* @test}
