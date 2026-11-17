# Bibim

[한국어](README.ko.md)

Bibim is a 2D game engine written in C++, originally developed by Halak Shin (2010–2014).
It targets Windows (Direct3D 9) and Android / iOS (OpenGL ES 2.0), and comes with C# tools
for asset processing and debugging.

## Platforms

| Platform | Renderer | Build files | Status |
|---|---|---|---|
| Windows | Direct3D 9 | `build/vs2008` – `build/vs2019` (Visual Studio) | Main platform |
| Android | OpenGL ES 2.0 | `build/eclipse` (Java side only; native build files are not in this repository) | Previously supported |
| iOS | OpenGL ES 2.0 | `build/xcode6` | Previously supported |
| Linux | OpenGL ES 2.0 | – | Port in progress |

## Features

| Area | Modules (`src/`) |
|---|---|
| Core | `Foundation` (math, strings, reflection), `Module` (service tree), `Component`, `Time` (timelines), `Threading`, `Diagnostics` |
| Graphics | `Graphics` (Direct3D 9 / OpenGL ES 2.0), `GFX` (particles), `Typography` (FreeType fonts), `Animation` (sprites, easing) |
| UI | `UI` (widgets, layout, events, effects) |
| Gameplay | `Gameplay`, `Collision2D` |
| Scripting | Lua 5.2 via lua_tinker, plus Bibim's own `Scripting` VM |
| I/O | `Asset` (local files, MPQ archives, network asset server), `FileSystem`, `IO`, `Net` |
| Platform | `Framework` (game loop, window), `Input` (keyboard, mouse, IME), `Audio` |

## Repository layout

| Path | Contents |
|---|---|
| `include/Bibim/` | Public headers |
| `src/` | Engine sources, one directory per module |
| `extlibs/` | Bundled third-party libraries |
| `build/` | IDE projects per toolchain, and `Doxyfile` |
| `lib/` | Build output and the property sheets that game projects import |
| `tools/` | C# tools (`Bibim.Toolkit`, `Bibim.Baaaaaab`) |
| `bin/` | Prebuilt tools |
| `docs/` | Notes |

## Building on Windows

Requirements:

- Visual Studio (the instructions below use the VS2019 solution)
- Legacy DirectX SDK (June 2010) — the projects read `$(DXSDK_DIR)`, which its installer sets

Steps:

1. Run `BIBIM_DIR.bat` in the repository root once. It sets the `BIBIM_DIR` environment variable
   to this directory. Restart Visual Studio afterwards.
2. Open `build/vs2019/Bibim.sln` and build the Win32 `Debug` and/or `Release` configuration.
   The engine and the bundled libraries are written to `lib/vs2019/Win32/<Configuration>/`.
3. In your game project, import the property sheet `lib/vs2019/Win32/<Configuration>/Bibim.props`.
   It adds the include directories, library directories and libraries the engine needs.

`publish.py` regenerates those property sheets, the single-file source `src/Bibim.cpp` and
`include/Bibim/All.h`. Run it after adding source files or changing dependencies.

## Tools

Prebuilt tools are in `bin/`; their sources are in `tools/` (`tools/Bibim.Toolkit.sln`).

- **Bibim.Toolkit** — asset pipeline library (converts source assets into Bibim's `.ab` format).
- **Bibim.Baaaaaab** — desktop tool with a console, a UI inspector and an asset server.

Debug builds on Windows ask the asset server on localhost for each asset first and fall back to local
files. Start the server when you want the game to receive assets from their source files:

```
bin\Bibim.Baaaaaab.exe console /class:AssetServer
```

### Running the console programs on Linux (Mono)

`Bibim.Baaaaaab.Cli` runs the same console programs as `Bibim.Baaaaaab.exe console` without WPF, so it
also works on Linux with Mono. Each line on standard input is sent to the running program as a command
(for example `exit`). Closing standard input, or pressing Ctrl+C when it runs in a terminal, stops it the same
way as closing the window; a second Ctrl+C stops it immediately. Because closing standard input stops it, a
server started without a terminal (for example with `< /dev/null`, `nohup` or systemd) exits right away;
keep standard input open instead, for example `tail -f /dev/null | mono ...`.

```
xbuild /p:Configuration=Release tools/Bibim.Baaaaaab.Cli/Bibim.Baaaaaab.Cli.csproj
cd bin
mono ../tools/Bibim.Baaaaaab.Cli/bin/x86/Release/Bibim.Baaaaaab.Cli.exe console /class:AssetServer
```

Run it from `bin/` so that it loads `bin/Plugin/*.dll`, like the Windows tool. Some recipes do not work on
Linux yet (for example texture cooking and Lua/HLSL compilation); see
[#27](https://github.com/Darkttd/bibim/issues/27).

## Projects using Bibim

- [Maple](https://github.com/Darkttd/Maple) — *Moemoe Nemologic*, a picross puzzle game

## Source conventions

- **Platform-specific code** lives in files named with a suffix, for example `GameFramework.Windows.cpp`,
  `GraphicsDevice.GLES2.cpp` or `Thread.Unix.cpp`. Each file is guarded by the matching
  `BIBIM_PLATFORM_*` / `BIBIM_USE_*` macro from `include/Bibim/Config.h`.
- **Line endings and BOM**: keep each file's existing line endings (most files use CRLF) and byte order
  mark. Do not reformat whole files.
- **Final newline**: every text file ends with a newline (CRLF or LF, matching the file). CI checks this
  with `.github/scripts/check-final-newline.sh`, skipping `extlibs/`, `tools/References/` and `bin/`.
- **Encoding**: source files that contain non-ASCII text (such as Korean comments) must be UTF-8
  *with* BOM. The Visual Studio projects do not pass `/utf-8`, so MSVC relies on the BOM to read
  them as UTF-8.
- **No tabs**: sources in `src/` and `include/` use spaces only, 4 per indent level. This includes
  preprocessor indentation after `#` (`#    define`) and alignment in the middle of a line. CI checks this
  with `.github/scripts/check-no-tabs.sh`, skipping the Lua and LuaTinker headers copied into `include/`
  (`lua.h`, `luaconf.h`, `lauxlib.h`, `lualib.h`, `lua_tinker.h`).
- **Language of new code**: new code and comments are written in English (the Latin alphabet) only.
  Existing Korean comments are kept as they are. The only exception is code that handles non-alphabet
  text (Korean, Japanese and so on) and its tests, for example test strings for fonts or text input.

## License

Bibim is being placed under the MIT License, as agreed with the original author. The pull request that
adds the `LICENSE` file is in progress upstream: [halak/bibim#2](https://github.com/halak/bibim/pull/2).

The third-party libraries in `extlibs/` keep their own licenses; see `LICENSE-BUNDLE`.
Note that **irrKlang** (`extlibs/irrKlang-1.4.0`) is not open source: its binaries may be redistributed
only in non-commercial products. See `extlibs/irrKlang-1.4.0/readme.txt`.
