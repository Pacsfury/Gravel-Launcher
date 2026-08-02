from llvmlite import binding


binding.initialize_native_target()
binding.initialize_native_asmprinter()


with open("output.ll", "r") as f:
    llvm_ir = f.read()


mod = binding.parse_assembly(llvm_ir)
mod.verify()


target_machine = binding.Target.from_default_triple().create_target_machine()
engine = binding.create_mcjit_compiler(mod, target_machine)
engine.finalize_object()


func_ptr = engine.get_function_address("main")


import ctypes
main_func = ctypes.CFUNCTYPE(ctypes.c_int)(func_ptr)
result = main_func()
print(f"\nExecution result: {result}")


