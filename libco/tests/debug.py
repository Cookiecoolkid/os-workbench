import subprocess
import os

# Set the LIBRARY_PATH environment variable
env = os.environ.copy()
# env["LD_LIBRARY_PATH"] = "~/.ssh/os-workbench/libco/"
os.environ["LD_LIBRARY_PATH"] = os.path.expanduser("~/.ssh/os-workbench/libco/")
# The name of the program to debug
program = "./libco-test-64"

# Start gdb
gdb_commands = [
    "gdb",
    # "-ex", "break stack_switch_call",  # Set a breakpoint at stack_switch_call
    # "-ex", "break co_yield", 
    "-ex", "break test_2",
    "-ex", "run",  # Run the program
    # "-ex", "layout split",
    # "-ex", "watch ((unsigned long long) $rsp & 0xF) != 0 && ((unsigned long long) $rsp & 0xF) != 8",
    program  # The program to debug
]

subprocess.run(gdb_commands, env=env)

