; ModuleID = '../llvm/test/compiler-course/maslova_u_lab2/test.ll'
source_filename = "../llvm/test/compiler-course/maslova_u_lab2/test.ll"

define i32 @test_srem(i32 %a, i32 %b) {
  %res = srem i32 %a, %b
  ret i32 %res
}

define i32 @test_urem(i32 %a, i32 %b) {
  %res = urem i32 %a, %b
  ret i32 %res
}

define float @test_frem(float %a, float %b) {
  %res = frem float %a, %b
  ret float %res
}
