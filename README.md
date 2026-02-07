# libmigdb - GDB Machine Interface Library

A C/C++ library for interfacing with GDB's Machine Interface (MI) protocol.

## About

This library provides a programmatic interface to control GDB through its MI protocol. MI responses are machine-readable (unlike the standard CLI), making them suitable for IDE integration, automated testing, and debugging tools.

### Advantages

* Internationalization (i18n) independent responses
* Structured, parseable output format
* Version-stable API across GDB updates
* Suitable for programmatic control

### Disadvantages

* Complex response structure
* Not human-readable
* Requires parsing logic

## Supported Platforms

* Linux (console and X11)
* Any POSIX-compliant system with GDB support

## Building and Installation

### Prerequisites

* GCC or compatible C compiler
* Make
* GDB (tested with GDB 6.x through 14.x)
* For X11 examples: X11 development libraries

### Build

```bash
cd src
make
```

### Install

```bash
cd src
sudo make install PREFIX=/usr/local
```

**Note:** Default `PREFIX` is `/usr`. For user installations, use `/usr/local` or `~/.local`.

## Examples

Switch to the `examples/` directory and run `make`. Each example demonstrates different debugging scenarios:

* **linux_test**: Linux console - breakpoints and watchpoints
* **remote_test**: Remote debugging via TCP/IP
* **x11_cpp_test**: C++ wrapper usage (X11)
* **x11_fr_test**: Stack frames and variable objects (X11)
* **x11_test**: Breakpoints and watchpoints (X11)
* **x11_wp_test**: Watchpoints (X11)

Read the comments at the beginning of each example for usage details.

## Documentation

An API reference is available in the `doc/` directory. Review the examples alongside the reference for best results.

## Debugging Strategies

### 1. Remote Debugging (TCP/IP)
Use `gdbserver` on the remote machine and connect via TCP/IP. Note: watchpoints may have limited support in remote mode.

### 2. X11 Local Debugging
The library spawns an xterm child process for inferior program I/O. See `gmi_start_xterm()`.

### 3. Linux Console Debugging
Opens a new virtual terminal (VT) for the debugged program. See `gmi_look_for_free_vt()`.

### 4. Same-Terminal Debugging
Uses the current terminal with suspend/resume for I/O handling. More complex, less functional.

## License

This project is licensed under the **GNU General Public License v2.0 (GPLv2)**.

See [LICENSE](LICENSE) for full details.

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Follow the existing code style
4. Submit a pull request

## Project Status

This library supports GDB's MI protocol and is actively maintained. The MI protocol has remained relatively stable across GDB versions 6.x through 14.x.

## Contact

For issues, feature requests, or contributions, please use the GitHub issue tracker.

---

**Historical Note:** This library was originally created to add remote debugging support to SETEdit. Legacy platform notes (DJGPP, RHIDE) have been moved to `DJGPP.why.obsolete` for historical reference.
