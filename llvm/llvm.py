import ctypes
import hashlib
import os
import time
from llvmlite import binding

binding.initialize_native_target()
binding.initialize_native_asmprinter()

CACHE_DIR = ".llvm_cache"
os.makedirs(CACHE_DIR, exist_ok=True)


def get_cached_native_func(ll_filepath):
    with open(ll_filepath, "r") as f:
        llvm_ir = f.read()

    ir_hash = hashlib.sha256(llvm_ir.encode("utf-8")).hexdigest()[:16]
    cache_path = os.path.join(CACHE_DIR, f"{ir_hash}.bc")

    target = binding.Target.from_default_triple()
    target_machine = target.create_target_machine(
        opt=3,
        cpu=binding.get_host_cpu_name(),
        features=binding.get_host_cpu_features().flatten(),
    )

    if os.path.exists(cache_path):
        t0 = time.perf_counter()
        with open(cache_path, "rb") as f:
            bitcode_bytes = f.read()

        mod = binding.parse_bitcode(bitcode_bytes)
    else:
        t0 = time.perf_counter()
        mod = binding.parse_assembly(llvm_ir)

        pto = binding.create_pipeline_tuning_options(speed_level=3)
        pto.loop_vectorization = True
        pto.slp_vectorization = True
        pto.loop_unrolling = True

        pass_builder = binding.create_pass_builder(target_machine, pto)
        mpm = pass_builder.getModulePassManager()
        mpm.run(mod, pass_builder)

        with open(cache_path, "wb") as f:
            f.write(mod.as_bitcode())


    engine = binding.create_mcjit_compiler(mod, target_machine)
    engine.finalize_object()

    func_ptr = engine.get_function_address("main")
    main_func = ctypes.CFUNCTYPE(ctypes.c_int)(func_ptr)

    return main_func, engine


main_func, engine = get_cached_native_func("output.ll")

start_time = time.perf_counter()
result = main_func()
end_time = time.perf_counter()

print(f"\nResult: {result}")
