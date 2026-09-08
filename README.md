# palera1n for usbliter8

A macOS/Linux host port of palera1n with a libusb transport for devices that
have already been placed in **usbliter8 pwned DFU**. It is based on the
requested `mrstickman3/palera1n-for-usbliter8` repository, with the broken
Intel-macOS build and device routing repaired.

> **Important:** a normal Mac or PC cannot run the usbliter8 SecureROM
> exploit. The exploit requires compatible RP2350 hardware and a Lightning to
> USB-A connection. This program runs after that hardware has produced the
> `PWND:[usbliter8]` DFU marker.

## Supported usbliter8 CPIDs

The host transport recognizes every CPID with an implementation in the
upstream usbliter8 project:

| CPID | SoC | Examples |
| --- | --- | --- |
| `0x8006` | S4/S5 | Apple Watch Series 4/5, first-generation Watch SE, HomePod mini |
| `0x8020` | A12 | iPhone XS/XR, iPad Air 3, iPad mini 5, iPad 8, Apple TV 4K (2nd gen) |
| `0x8030` | A13 | iPhone 11 family, iPhone SE (2nd gen), iPad 9, Studio Display |

A12X/Z (`0x8027`) is deliberately reported as not implemented: the upstream
usbliter8 exploit does not provide an A12X/Z implementation, and pretending
that it is supported can leave a device in an unusable state. The transport
also refuses ordinary and checkm8-pwned DFU devices unless the serial string
contains the exact `PWND:[usbliter8]` marker.

The post-exploitation palera1n pipeline is intended for iPhone, iPad, and
Apple TV devices. Apple Watch, HomePod, and Studio Display are listed because
their SoCs are affected by usbliter8, but they are not claimed to be
jailbreakable by palera1n.

## Intel Mac build

Install the host tools and libraries with Homebrew:

```sh
brew install make pkg-config libusb libimobiledevice libirecovery libusbmuxd \
  libimobiledevice-glue libplist mbedtls readline xz vim
```

This project uses GNU Make syntax. Homebrew installs it as `gmake` on macOS;
use `make` instead if GNU Make is already your default. Build from the
repository root:

```sh
gmake
```

The Makefile now:

- works with Intel Homebrew (`/usr/local`) as well as Apple Silicon Homebrew;
- does not invoke `brew` when it is absent;
- obtains libusb's include and linker flags through `pkg-config`;
- downloads only the host checkra1n payload that is needed; and
- creates missing build directories automatically.

Compilation does not require `sudo`. Install the resulting command with:

```sh
sudo gmake install
```

Use `PREFIX=/some/path` to install somewhere else. Run `gmake clean` to remove
local build output and downloaded resources.

## Usage

1. Flash a supported usbliter8 RP2350 board and use it to put the device in
   pwned DFU.
2. Move the device back to the Mac/PC. Confirm that its DFU serial string ends
   in `PWND:[usbliter8]`.
3. Run the host tool with the usbliter8 mode enabled:

```sh
sudo palera1n --pwned-dfu -l -v
```

`--pwned-dfu` prevents the host from trying checkm8 on A12/A13-class devices.
`-l` selects the rootless pipeline. `--override-pongo FILE` can be used to
send a different raw Pongo/iBoot image; otherwise the embedded image is used.

Useful diagnostics:

```sh
palera1n --version
palera1n --diagnostics
palera1n --device-info
```

This remains a tethered workflow: the device must be connected to the
usbliter8 hardware again whenever the boot chain needs to be re-established.

## Project layout

- `src/usbliter8_boot.c` — safe libusb DFU download and boot request;
- `src/device_support.c` — centralized CPID/device-path table;
- `src/dfuhelper.c` — pwned DFU detection and routing;
- `src/embedded_pongo_helper.c` — portable embedded-image extraction; and
- `Makefile` — macOS/Linux dependency and resource build.

## Credits

- Paradigm Shift — usbliter8 research and exploit;
- palera1n team — original jailbreak project; and
- the contributors to libimobiledevice, libirecovery, libusb, and mbedTLS.

Use this software only on devices you own or are authorized to test. A boot
chain exploit does not bypass the Secure Enclave or device data encryption.
