# wake-on-lan

Minimal Wake-on-LAN tool. Sends a magic packet over UDP to wake a machine by its MAC address.

## Build

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
