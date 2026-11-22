# LuauDesk — Standalone Luau Playground

LuauDesk is a small Windows desktop app for writing and running **standalone Luau** scripts.
It does not attach to Roblox, inject into the Roblox client, inspect game memory, or bypass Roblox security.

## What is included

- Native Windows desktop window
- Multiline script editor
- Execute button
- Console output and basic error reporting
- Clear Output button
- Luau runtime and compiler fetched from the official Luau repository during CMake configuration

## Requirements

- Windows 10 or 11, 64-bit
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.24 or newer
- Git available on PATH
- Internet access for the first configure/build (CMake downloads Luau source)

## Build

1. Extract this folder.
2. Double-click `build.bat`.
3. Wait for CMake to download Luau and compile the project.
4. The executable should be at `build\\Release\\LuauDesk.exe`.

You can also open the folder in Visual Studio using **File → Open → Folder** and build the `LuauDesk` target in Release / x64.

## First test

The editor starts with:

```lua
print("Hello from Luau!")
local total = 20 + 22
print("Answer:", total)
```

Click **Execute**. The console should show the two print lines.

## Notes

- This is a starter project. It is not a Roblox executor and cannot run Roblox-specific APIs such as `game`, `workspace`, or `Players`.
- Scripts run in a fresh Luau state each time. The standard Luau libraries are opened; Roblox services are not provided.
- Do not run scripts from strangers. A script can consume resources or perform unwanted actions within the standalone runtime.
- The project uses the official Luau C++ libraries. If the upstream Luau CMake target names or API change, the build may need a small update.
