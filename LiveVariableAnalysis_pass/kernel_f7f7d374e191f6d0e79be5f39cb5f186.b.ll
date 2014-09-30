; ModuleID = 'kernel_f7f7d374e191f6d0e79be5f39cb5f186.b.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.tlb_data = type { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, i32, i32 }

@xDim = global i32 1, align 4
@yDim = global i32 2, align 4
@zDim = global i32 4, align 4
@xid = global i32 0, align 4
@yid = global i32 0, align 4
@zid = global i32 0, align 4
@TLB = common global %struct.tlb_data zeroinitializer, align 4

; Function Attrs: nounwind uwtable
define void @test_fn(<8 x i32>* %sSharedStorage, <8 x i32>* %src, i32* %offsets, i32* %alignmentOffsets, <8 x i32>* %results) #0 {
entry:
  %sSharedStorage.addr = alloca <8 x i32>*, align 8
  %src.addr = alloca <8 x i32>*, align 8
  %offsets.addr = alloca i32*, align 8
  %alignmentOffsets.addr = alloca i32*, align 8
  %results.addr = alloca <8 x i32>*, align 8
  %tid = alloca i32, align 4
  %lid = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp = alloca <8 x i32>, align 32
  store <8 x i32>* %sSharedStorage, <8 x i32>** %sSharedStorage.addr, align 8
  store <8 x i32>* %src, <8 x i32>** %src.addr, align 8
  store i32* %offsets, i32** %offsets.addr, align 8
  store i32* %alignmentOffsets, i32** %alignmentOffsets.addr, align 8
  store <8 x i32>* %results, <8 x i32>** %results.addr, align 8
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  %call1 = call i32 @get_local_id(i32 0)
  store i32 %call1, i32* %lid, align 4
  %0 = load i32* %lid, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then
  %1 = load i32* %i, align 4
  %cmp2 = icmp slt i32 %1, 256
  br i1 %cmp2, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %3 = load <8 x i32>** %src.addr, align 8
  %arrayidx = getelementptr inbounds <8 x i32>* %3, i64 %idxprom
  %4 = load <8 x i32>* %arrayidx, align 32
  %5 = load i32* %i, align 4
  %idxprom3 = sext i32 %5 to i64
  %6 = load <8 x i32>** %sSharedStorage.addr, align 8
  %arrayidx4 = getelementptr inbounds <8 x i32>* %6, i64 %idxprom3
  store <8 x i32> %4, <8 x i32>* %arrayidx4, align 32
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32* %i, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %if.end

if.end:                                           ; preds = %for.end, %entry
  call void @barrier(i32 1)
  %8 = load i32* %tid, align 4
  %idxprom5 = sext i32 %8 to i64
  %9 = load i32** %offsets.addr, align 8
  %arrayidx6 = getelementptr inbounds i32* %9, i64 %idxprom5
  %10 = load i32* %arrayidx6, align 4
  %11 = load <8 x i32>** %sSharedStorage.addr, align 8
  %12 = bitcast <8 x i32>* %11 to i32*
  %13 = load i32* %tid, align 4
  %idxprom7 = sext i32 %13 to i64
  %14 = load i32** %alignmentOffsets.addr, align 8
  %arrayidx8 = getelementptr inbounds i32* %14, i64 %idxprom7
  %15 = load i32* %arrayidx8, align 4
  %idx.ext = zext i32 %15 to i64
  %add.ptr = getelementptr inbounds i32* %12, i64 %idx.ext
  %call9 = call <8 x i32> @vload8(i32 %10, i32* %add.ptr)
  store <8 x i32> %call9, <8 x i32>* %tmp, align 32
  %16 = load <8 x i32>* %tmp, align 32
  %17 = load i32* %tid, align 4
  %idxprom10 = sext i32 %17 to i64
  %18 = load <8 x i32>** %results.addr, align 8
  %arrayidx11 = getelementptr inbounds <8 x i32>* %18, i64 %idxprom10
  store <8 x i32> %16, <8 x i32>* %arrayidx11, align 32
  ret void
}

declare i32 @get_global_id(i32) #1

declare i32 @get_local_id(i32) #1

declare void @barrier(i32) #1

declare <8 x i32> @vload8(i32, i32*) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!llvm.ident = !{!1}

!0 = metadata !{void (<8 x i32>*, <8 x i32>*, i32*, i32*, <8 x i32>*)* @test_fn}
!1 = metadata !{metadata !"clang version 3.4 (tags/RELEASE_34/final)"}
