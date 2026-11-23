# AlphaSmart 3000 USB connection

The AS3000 board uses a Philips **PDIUSBD11**, not a D12. The D11 is an
I2C USB **device** controller. Its one USB connector cannot host a USB printer.
The driver connects the D11 to the firmware's bit-banged open-drain bus
(PE1 SDA, PE2 SCL), PF1 interrupt input, and PE0 suspend input (inferred from the firmware's suspend/resume handling).
USB descriptors, HID reports, and AlphaSmart Manager communications come from
emulated firmware. The bridge does not synthesize them or patch the ROM.

## Build and launch

Build from this checkout:

```sh
make SUBTARGET=asma3k SOURCES=src/mame/skeleton/alphasma3k.cpp OPTIMIZE=1 NOWERROR=1 -j2
```

Run with your normal ROM/NVRAM and applet options, plus:

```sh
./asma3k asma3k -usb socket.127.0.0.1:13240
```

Leave throttling enabled. USB software uses wall-clock timeouts while the
controller runs on emulated time. Boot past any fresh-NVRAM recovery prompts
before connecting the bridge.

In another terminal, start the Python 3.11+ bridge (standard library only):

```sh
python3 scripts/alphasmart/usbip.py
```

MAME listens on TCP port 13240; the bridge connects to it and exports USB/IP on
127.0.0.1:3240, with bus ID `1-1`. MAME has no libusb/usbredir dependency.
The bridge discovers the firmware's device and configuration descriptors first.

On Linux, after loading `vhci-hcd`:

```sh
usbip list -r 127.0.0.1
sudo usbip attach -r 127.0.0.1 -b 1-1
usbip port
```

The device then appears through the ordinary Linux USB stack. Run neotools
with its existing PyUSB/libusb backend and appropriate USB device permissions.
No changes to neotools are required by the transport.

The firmware disconnects/reconnects when changing between keyboard and
communications modes. USB/IP does not automatically re-import a disconnected
device. For automatic local re-import, run the bridge with `--attach` under an
account allowed to write the vhci_hcd attach sysfs attribute (typically root):

```sh
sudo python3 scripts/alphasmart/usbip.py --attach
```

This option invokes `/usr/sbin/usbip`; override with `--usbip PATH` if needed.
MAME itself should run as your normal user. To attach manually after a mode
change, repeat `usbip attach`. To detach, use the port shown by `usbip port`:

```sh
sudo usbip detach -p PORT
```

The bridge's `--port` and MAME-side `--mame-port` are independently configurable.
For a non-default USB/IP port use `usbip --tcp-port PORT ...`.
One importing host is supported at a time. Expose these sockets only to trusted
local programs; neither protocol authenticates clients.

## Running without sudo

The bridge itself needs no privilege. Only importing into `vhci_hcd` requires
access normally restricted to root. For Linux 6.1 the relevant controls are
`/sys/devices/platform/vhci_hcd.0/attach` and `detach`; the stock `usbip`
command also stores connection records under `/run/vhci_hcd`.

An administrator can delegate these to a dedicated `usbip` group (or a local
trusted group such as `plugdev`). For a single-user development machine,
this is a one-boot setup after loading `vhci-hcd`:

```sh
sudo groupadd -f usbip
sudo usermod -aG usbip "$USER"
sudo chgrp usbip /sys/devices/platform/vhci_hcd.0/attach /sys/devices/platform/vhci_hcd.0/detach
sudo chmod g+w /sys/devices/platform/vhci_hcd.0/attach /sys/devices/platform/vhci_hcd.0/detach
sudo install -d -o root -g usbip -m 2770 /run/vhci_hcd
```

Log out and back in to acquire the group, then run the bridge with `--attach`
without sudo. Existing root-created connection records may still be unreadable
to `usbip port`; reconnecting as the regular user creates that user's records.
The sysfs permissions reset on module reload and `/run` resets on reboot.
A system udev rule/startup service plus a tmpfiles rule can make this persistent.
This delegates control of virtual USB attachment/detachment for the controller,
not just this one emulated device.

Access for neotools to the resulting `/dev/bus/usb` device is separate; use
USB udev permissions for vendor `081e`, including products `bd04` and `bd01`.
A `plugdev` membership by itself does not grant VHCI sysfs access.

There is no known drop-in tool for the custom D11P socket protocol. Stock
`usbip` already supplies the host-side client; `usbipd` exports kernel-backed
USB devices rather than interpreting MAME packets. Virtual-device frameworks
such as `Sawchord/usbip-device` could supply USB/IP machinery, but still need
a MAME-specific backend and firmware-driven descriptor/mode-switch handling.

## Verified firmware integration

With stock System 3.01 and AlphaWord Plus, the device enumerated as
`081e:bd04`, switched to communications mode `081e:bd01`, and passed the
existing neotools handshake, version query, and free ROM/RAM query through
Linux `vhci_hcd`. The bridge also provides a kernel-independent firmware check:

```sh
python3 scripts/alphasmart/usbip.py --probe
```

Connect this only after firmware has booted; it changes the device to
communications mode and checks the Manager protocol response `02 20`.
The SoC interrupt-mask fix is necessary to wake the sleeping firmware when
the cable is attached. Save states and firmware flashing are not validated.
The ASNEO driver and its different USB hardware are not implemented here.

## Socket packet protocol

The bridge-to-MAME protocol is intentionally at USB packet level, so that
firmware reads/writes the real eight-byte endpoint FIFOs. Each message starts
with five big-endian uint32 values:

| Field | Request | Reply |
|---|---|---|
| magic | `0x44313150` (`D11P`) | same |
| sequence | caller-assigned | same |
| operation/status | operation below | ACK=0, NAK=1, STALL=2, unavailable/invalid=3 |
| endpoint | USB endpoint number 0–3 | same |
| payload length | 0–8 bytes | 0–8 bytes |

Payload follows the header. Operations are state=0, cable attach=1,
bus reset=2, SETUP=3, OUT=4, IN=5, cable detach=6. State replies contain
four bytes: VBUS present, SoftConnect active with VBUS, endpoint enable,
and address/enable register. SETUP must contain eight bytes on endpoint zero.
IN requests have no payload and return one packet, including a zero-length
packet. NAK never consumes a packet. The caller retries after allowing the
firmware to run. Hard reset clears chip registers; USB bus reset preserves
the datasheet's persistent mode/clock settings and raises reset interrupt.

The bridge implements USB/IP discovery, import, asynchronous submissions,
short packets, STALLs, and unlink. Isochronous transfers are unsupported;
the D11 only provides eight-byte control and generic bulk/interrupt endpoints.
Live sockets cannot be restored from a saved state.

## Checks

```sh
c++ -std=c++17 -Wall -Wextra -Werror tests/alphasmart/pdiusbd11.cpp -o /tmp/pdiusbd11-test
/tmp/pdiusbd11-test
python3 -m unittest discover -s tests/alphasmart -v
```

The C++ tests drive the actual I2C pins. Python tests speak USB/IP over actual
loopback sockets and exercise cancellation and stream synchronization. They
need permission to create local sockets.

Register reference: Philips, *PDIUSBD11 USB device with serial interface*,
product specification, 22 July 1999, especially endpoint mapping on page 3 and
command/register definitions on pages 5–10. USB/IP framing follows the
[Linux kernel protocol documentation](https://docs.kernel.org/usb/usbip_protocol.html).
