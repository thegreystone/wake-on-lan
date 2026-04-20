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
wol <MAC> [broadcast-address] [port]
```

| Argument           | Default           | Notes                                                                 |
|--------------------|-------------------|-----------------------------------------------------------------------|
| `MAC`              | *(required)*      | `AA:BB:CC:DD:EE:FF` or `AA-BB-CC-DD-EE-FF`                           |
| `broadcast-address`| `255.255.255.255` | Use subnet-directed (e.g. `192.168.1.255`) if your router blocks global broadcast across VLANs |
| `port`             | `9`               | Standard WOL port (UDP discard); port `7` is an alternative           |

**Example**
```sh
wol AA:BB:CC:DD:EE:FF 192.168.1.255
```
