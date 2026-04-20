# wake-on-lan

[![Build](https://github.com/thegreystone/wake-on-lan/actions/workflows/build.yml/badge.svg)](https://github.com/thegreystone/wake-on-lan/actions/workflows/build.yml)
[![Latest release](https://img.shields.io/github/v/release/thegreystone/wake-on-lan)](https://github.com/thegreystone/wake-on-lan/releases/latest)

Minimal Wake-on-LAN tool. Sends a magic packet over UDP to wake a machine by its MAC address.

## Install pre-built binary

Pre-built binaries are available for Linux x86_64/aarch64, macOS aarch64/x86_64, and Windows x86_64/arm64.

**macOS / Linux**
```sh
curl -fsSL https://raw.githubusercontent.com/thegreystone/wake-on-lan/main/install.sh | sh
```

Installs the latest release to `/usr/local/bin`. To install a specific version or a different directory:
```sh
./install.sh v1.0.0
./install.sh v1.0.0 --dir ~/.local/bin
```

**Windows** (PowerShell)
```powershell
irm https://raw.githubusercontent.com/thegreystone/wake-on-lan/main/install.ps1 | iex
```

Installs to `%LOCALAPPDATA%\Programs\wol` and adds it to the user `PATH`. To install a specific version or a different directory:
```powershell
.\install.ps1 -Version v1.0.0
.\install.ps1 -Version v1.0.0 -Dir C:\Tools
```

## Build from source

**macOS / Linux**
```sh
./build.sh
```

**Linux — fully static binary**
```sh
./build.sh --static
```

**Windows** (MSVC or MinGW)
```bat
build.bat
```

You can also use `make` on macOS/Linux.

## Install

```sh
sudo ./build.sh --install   # installs to /usr/local/bin
```

## Version

The binary embeds the version derived from `git describe --tags --always` at build time.

```sh
wol --version   # or: wol -V
```

To cut a release, tag and push:

```sh
git tag v1.0.0
git push origin v1.0.0
```

## Usage

```
wol <MAC|alias> [broadcast-address] [port]
```

| Argument           | Default           | Notes                                                                 |
|--------------------|-------------------|-----------------------------------------------------------------------|
| `MAC` or `alias`   | *(required)*      | `AA:BB:CC:DD:EE:FF`, `AA-BB-CC-DD-EE-FF`, or a name from `~/.wol`   |
| `broadcast-address`| `255.255.255.255` | Use subnet-directed (e.g. `192.168.1.255`) if your router blocks global broadcast across VLANs |
| `port`             | `9`               | Standard WOL port (UDP discard); port `7` is an alternative           |

**Examples**
```sh
wol AA:BB:CC:DD:EE:FF 192.168.1.255
wol macmini
wol server 192.168.1.255
```

## Aliases (~/.wol)

Create `~/.wol` to define named aliases so you don't have to remember MAC addresses:

```
# format: alias  MAC  [broadcast-address  [port]]
macmini  AA:BB:CC:DD:EE:FF  192.168.1.255
server   11:22:33:44:55:66
gaming   AA:11:BB:22:CC:33  192.168.1.255  7
```

Lines starting with `#` and blank lines are ignored. The broadcast address and port are optional and fall back to the usual defaults if omitted. Command-line arguments still override alias values when supplied.
