; ModuleID = '2014-09-23-gai.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ST = type { i32, double, %struct.RT }
%struct.RT = type { i8, [10 x [20 x i32]], i8 }

; Function Attrs: nounwind uwtable
define i32 @test() #0 {
entry:
  %s = alloca %struct.ST, align 8
  %r = alloca [16 x %struct.RT], align 16
  %x = alloca [11 x i32], align 16
  %call = call signext i8 (...)* @bar()
  %conv = sext i8 %call to i32
  %arrayidx = getelementptr inbounds [11 x i32]* %x, i32 0, i64 6
  store i32 %conv, i32* %arrayidx, align 4
  %Z = getelementptr inbounds %struct.ST* %s, i32 0, i32 2
  %B = getelementptr inbounds %struct.RT* %Z, i32 0, i32 1
  %arrayidx1 = getelementptr inbounds [10 x [20 x i32]]* %B, i32 0, i64 5
  %arrayidx2 = getelementptr inbounds [20 x i32]* %arrayidx1, i32 0, i64 13
  %0 = load i32* %arrayidx2, align 4
  %arrayidx3 = getelementptr inbounds [16 x %struct.RT]* %r, i32 0, i64 10
  %c = getelementptr inbounds %struct.RT* %arrayidx3, i32 0, i32 2
  %1 = load i8* %c, align 1
  %conv4 = sext i8 %1 to i32
  %add = add nsw i32 %0, %conv4
  %arrayidx5 = getelementptr inbounds [11 x i32]* %x, i32 0, i64 6
  %2 = load i32* %arrayidx5, align 4
  %add6 = add nsw i32 %add, %2
  ret i32 %add6
}

declare signext i8 @bar(...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 (tags/RELEASE_34/final)"}
