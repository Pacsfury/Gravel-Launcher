import subprocess
from pathlib import Path

def execute(file):
    subprocess.run(['./gravel', 'run', "./tests/" + file], check=True)

    if Path("./tests/" + file.replace(".grv", ".txt")).read_text(encoding="utf-8").strip() != Path("output.ll").read_text(encoding="utf-8").strip():
        print("file " + file + " is not passing the tests\n")
        return 1
    else:
        print(file + " passed\n")
        return 0

files = [f.name for f in Path('./tests').iterdir() if f.is_file() and f.name.endswith(".grv")]

def test():
    for file in files:
        if execute(file) == 1:
            return 1

test()

print("Test ended")