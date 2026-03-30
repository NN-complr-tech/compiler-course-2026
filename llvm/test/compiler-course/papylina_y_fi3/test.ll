define dso_local noundef i32 @_Z3mulii(i32 noundef %a, i32 noundef %b){
entry:
  %mul = mul nsw i32 %b, %a
  ret i32 %mul
}