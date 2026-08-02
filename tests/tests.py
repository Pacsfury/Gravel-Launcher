import subprocess
from pathlib import Path

def execute(file):
    subprocess.run(['./gravel', 'run', "./tests/" + file])

    if Path("./tests/" + file.replace(".grv", ".txt")).read_text(encoding="utf-8").strip() != Path("output.ll").read_text(encoding="utf-8").strip():
        print("file " + file + " is not passing the tests\n")
    else:
        print(file + " passed\n")

files = [f.name for f in Path('./tests').iterdir() if f.is_file() and f.name.endswith(".grv")]

for file in files:
    execute(file)
print("Test ended")