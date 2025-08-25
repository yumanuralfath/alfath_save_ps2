# VMC Reader - PlayStation 2 Memory Card Reader

<!--toc:start-->

- [VMC Reader - PlayStation 2 Memory Card Reader](#vmc-reader-playstation-2-memory-card-reader)
  - [Features](#features)
  - [Project Structure](#project-structure)
  - [Building](#building)
    - [Quick Build](#quick-build)
    - [Debug Build (with debug symbols)](#debug-build-with-debug-symbols)
    - [Release Build (optimized)](#release-build-optimized)
    - [Clean Build Files](#clean-build-files)
  - [Usage](#usage)
  - [Sample Output](#sample-output)
  - [Key Improvements](#key-improvements)
    - [Fixed Issues](#fixed-issues)
    - [Game ID Matching](#game-id-matching)
  - [Technical Details](#technical-details)
    - [VMC Format Support](#vmc-format-support)
    - [FAT Analysis](#fat-analysis)
    - [Database](#database)
  - [Development](#development)
    - [Adding New Games](#adding-new-games)
    - [Memory Debugging](#memory-debugging)
    - [Code Formatting](#code-formatting)
  - [Installation](#installation)
    - [System-wide Install](#system-wide-install)
    - [Uninstall](#uninstall)
  - [Requirements](#requirements)
  - [License](#license)
  - [Contributing](#contributing)
  - [Troubleshooting](#troubleshooting) - ["Invalid VMC magic" Error](#invalid-vmc-magic-error) - ["Unknown Game" Titles](#unknown-game-titles) - [Build Errors](#build-errors)
  <!--toc:end-->

A C program to read and analyze PlayStation 2 Virtual Memory Card (VMC) files, displaying save game information with proper game title lookup.

## Features

- **Complete VMC parsing**: Reads superblock, FAT, and directory structures
- **Game title lookup**: Built-in database of PS2 game titles
- **Smart save parsing**: Automatically extracts game IDs from save names
- **Memory usage analysis**: Shows used/free space correctly
- **Clean directory listing**: Displays save games with creation/modification dates

## Project Structure

```
vmcreader/
├── vmc.h           # Header file with all structures and prototypes
├── main.c          # Main program entry point
├── vmc_core.c      # Core VMC reading functions
├── vmc_display.c   # Display and formatting functions
├── titles_db.c     # Game titles database
├── Makefile        # Build configuration
└── README.md       # This file
```

## Building

### Quick Build

```bash
make
```

### Debug Build (with debug symbols)

```bash
make debug
```

### Release Build (optimized)

```bash
make release
```

### Clean Build Files

```bash
make clean
```

## Usage

```bash
./vmcreader <vmc_file>
```

Example:

```bash
./vmcreader mc0.bin
```

## Sample Output

```
Successfully opened VMC file: mc0.bin
File size: 67108864 bytes (64.00 MB)

=== VMC Information ===
Magic: Sony PS2 Memory Card Format
Version: 1.2.0.0
Page size: 512 bytes
Cluster size: 1024 bytes
Pages per cluster: 2
Clusters per card: 65536
Allocation offset: 265
Root directory cluster: 0
Max allocatable clusters: 65001
Card type: 2
Card flags: 0x2B
Used clusters: 150 (0.15 MB)
Free clusters: 64851 (63.33 MB)
Total allocatable space: 63.48 MB
========================

=== Root Directory ===
Expected entries: 11
Save Name                        Type       Size Created          Modified         Game Title
---------                        ----       ---- -------          --------         ----------
BESLES-556732014OPT              DIR           5 2025/08/17-03:10:14 2025/08/17-03:10:35 Final Fantasy X-2
BESLES-556732014000              DIR           5 2025/08/17-03:14:30 2025/08/17-03:14:53 Final Fantasy X-2
BASLUS-21050                     DIR           5 2025/08/17-03:18:43 2025/08/17-03:18:43 Grand Theft Auto: Vice City
BASLUS-21846DAT0                 DIR           5 2024/10/10-04:41:11 2024/10/10-04:55:28 Guitar Hero III: Legends of Rock
BASCUS-97436GAMEDATA             DIR           6 2025/08/21-22:12:35 2025/08/21-22:55:58 Grand Theft Auto: San Andreas

Total game saves found: 9
```

## Key Improvements

### Fixed Issues

1. **Title Lookup**: Now uses internal C database instead of external files
2. **Free Space Calculation**: Correctly calculates free clusters using FAT flags
3. **Smart Game ID Extraction**: Handles common PS2 save naming conventions
4. **Memory Management**: Proper allocation and cleanup

### Game ID Matching

The program intelligently extracts game IDs from save names by removing common suffixes:

- `BESLES-556732014OPT` → `BESLES-55673` → matches database
- `BASCUS-97436GAMEDATA` → `BASCUS-97436` → matches database
- `BASLUS-21846DAT0` → `BASLUS-21846` → matches database

## Technical Details

### VMC Format Support

- **Magic**: `Sony PS2 Memory Card Format`
- **Versions**: 1.0.0.0, 1.1.0.0, 1.2.0.0
- **Cluster sizes**: 512B, 1024B, 2048B
- **Card sizes**: 8MB to 512MB

### FAT Analysis

- Reads indirect FAT clusters (IFC)
- Follows cluster chains properly
- Identifies free clusters using flag 0x7F + cluster 0xFFFFFF

### Database

- Built-in database with common PS2 games
- Extensible structure for adding more titles
- Supports partial matching for save variations

## Development

### Adding New Games

Edit `titles_db.c` and add entries to the `ps2_titles[]` array:

```c
{"SLUS-12345", "Game Title", "Publisher", "Genre", "Language", "Developer", "Region", "Date", "SLUS-12345"},
```

### Memory Debugging

```bash
make memcheck
```

### Code Formatting

```bash
make format
```

## Installation

### System-wide Install

```bash
sudo make install
```

### Uninstall

```bash
sudo make uninstall
```

## Requirements

- GCC compiler
- Standard C library
- POSIX-compatible system (Linux, macOS, Windows with MinGW)

## License

This project is open source. Feel free to modify and distribute.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Troubleshooting

### "Invalid VMC magic" Error

- Ensure the file is a valid PS2 VMC file
- Check file isn't corrupted or compressed

### "Unknown Game" Titles

- Game might not be in the database
- Add the game ID to `titles_db.c`
- Save name might have unusual suffix format

### Build Errors

- Ensure GCC is installed
- Check that all source files are present
- Try `make clean` followed by `make`
