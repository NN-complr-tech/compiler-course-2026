// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/pikhotskiy_r_lab4_MLIR%shlibext --pass-pipeline="builtin.module(pikhotskiy-call-count)" %s | FileCheck %s

module {
  // CHECK-LABEL: func.func @entry
  // CHECK-SAME: call_count = 0 : i64
  func.func @entry() {
    func.call @worker() : () -> ()
    func.call @worker() : () -> ()
    func.call @recursive() : () -> ()
    func.call @declared_only() : () -> ()
    func.return
  }

  // CHECK-LABEL: func.func @worker
  // CHECK-SAME: call_count = 2 : i64
  func.func @worker() {
    func.return
  }

  // CHECK-LABEL: func.func @recursive
  // CHECK-SAME: call_count = 1 : i64
  func.func @recursive() {
    func.call @recursive() : () -> ()
    func.return
  }

  // CHECK-LABEL: func.func private @declared_only
  // CHECK-SAME: call_count = 1 : i64
  func.func private @declared_only()
}
