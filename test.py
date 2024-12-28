#!/usr/bin/python3

import asyncio
import sys
import os

async def run(repetitions):
    # Adjust the command path based on the platform
    if os.name == 'nt':  # Windows
        cmd = '.\\client.exe'  # Use Windows executable
    else:  # Linux/Unix
        cmd = './client'  # Use Unix-style executable

    to_wait = []

    # Stress test, simulate multiple concurrent connections
    for i in range(repetitions):
        try:
            proc = await asyncio.create_subprocess_shell(
                cmd,
                stdout=asyncio.subprocess.DEVNULL,
                stderr=asyncio.subprocess.DEVNULL
            )
            to_wait.append(proc)
            print(f"{i} executed")
        except FileNotFoundError:
            print(f"Error: Command '{cmd}' not found. Ensure the client executable is present.")
            return

    # Wait for all processes to complete
    for i, proc in enumerate(to_wait):
        await proc.communicate()
        print(f'[{cmd!r} exited with {proc.returncode}]')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        repetitions = 10
    else:
        repetitions = int(sys.argv[1])

    asyncio.run(run(repetitions))
