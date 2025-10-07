# SmallShell
A minimalistic shell written in C that supports some basic shell features.

## Features
- Commands can be run in the background by adding & to the end of the line
- Input/Output redirection using < and >
- $$ expands to the current PID
- Supports PATH based command lookups
- SIGTSTP (Ctrl + z) is used to toggle whether background commands are accepted
- SIGINT (Ctrl + c) is used to kill the current running command. It does not kill SmallShell or background processes
- A line starting with a # is a comment and will be ignored

## Built-in Commands
- `status` returns the previous commands exit code
- `cd` changes the current working directory
- `exit` exits the shell

## Requirements
- Make
- POSIX-compatible system

## Installation
```bash
git clone https://github.com/CrazyheadJake/SmallShell.git
make
```

## Usage
```bash
./smallsh
```