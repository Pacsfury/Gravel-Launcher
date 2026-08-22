import subprocess
import sys
from pathlib import Path

def execute(file):
    output_path = Path("output.ll")
    if output_path.exists():
        output_path.unlink()

    try:
        subprocess.run(['./gravel', 'run', f"./tests/{file}"], check=True, capture_output=True)
    except subprocess.CalledProcessError as e:
        print(f"Compiler failed with file: {file}")
        print(e.stderr.decode().strip())
        return 1

    if not output_path.exists():
        print(f"Error: {file} did not generate output.ll")
        return 1

    expected_path = Path(f"./tests/{file.replace('.grv', '.txt')}")
    if not expected_path.exists():
        print(f"Warning: Expected verification file {expected_path.name} does not exist")
        return 1

    if expected_path.read_text(encoding="utf-8").strip() != output_path.read_text(encoding="utf-8").strip():
        print(f"File {file} failed verification.")

        print(f'\n\n{expected_path.read_text(encoding="utf-8").strip()}\n\n{output_path.read_text(encoding="utf-8").strip()}\n\n')
        return 1
    
    print(f"{file} passed successfully.")
    return 0

def test():
    files = [f.name for f in Path('./tests').iterdir() if f.is_file() and f.name.endswith(".grv")]
    for file in files:
        if execute(file) == 1:
            return 1
    return 0

if __name__ == "__main__":
    if test() == 1:
        print("\nError detected during testing.")
        sys.exit(1)
    else:
        print("\nAll tests completed successfully.")
        sys.exit(0)
