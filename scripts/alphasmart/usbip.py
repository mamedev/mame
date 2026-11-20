#!/usr/bin/env python3
# license:BSD-3-Clause
"""Export MAME's firmware-driven PDIUSBD11 via Linux USB/IP (stdlib only)."""
import argparse
import asyncio
import contextlib
import errno
import logging
import struct

MAGIC = 0x44313150
HEADER = struct.Struct('!5I')
BUSID = b'1-1'
LOG = logging.getLogger('alphasmart.usbip')


class USBError(Exception):
    def __init__(self, status):
        self.status = status


class Chip:
    def __init__(self, reader, writer):
        self.reader, self.writer = reader, writer
        self.lock = asyncio.Lock()
        self.control_lock = asyncio.Lock()
        self.sequence = 0

    async def request(self, op, ep=0, data=b''):
        async with self.lock:
            self.sequence += 1
            async def reply():
                self.writer.write(HEADER.pack(MAGIC, self.sequence, op, ep, len(data)) + data)
                await self.writer.drain()
                magic, sequence, status, endpoint, length = HEADER.unpack(
                    await asyncio.wait_for(self.reader.readexactly(20), 5))
                if (magic, sequence, endpoint) != (MAGIC, self.sequence, ep) or length > 8:
                    raise ConnectionError('Invalid MAME packet reply')
                return status, await self.reader.readexactly(length)
            pending = asyncio.create_task(reply())
            try:
                return await asyncio.shield(pending)
            except asyncio.CancelledError:
                # Drain a packet already sent before releasing the stream lock.
                await pending
                raise
            except Exception:
                self.writer.close()
                raise

    async def state(self):
        status, data = await self.request(0)
        if status or len(data) != 4:
            raise ConnectionError('Invalid MAME state reply')
        return data

    async def ready(self, timeout=15, enabled=True):
        async with asyncio.timeout(timeout):
            while True:
                state = await self.state()
                if state[1] and (not enabled or state[3] & 0x80):
                    return
                await asyncio.sleep(.01)

    async def packet(self, op, ep=0, data=b''):
        while True:
            status, response = await self.request(op, ep, data)
            if status == 0:
                return response
            if status != 1:
                raise USBError(-errno.EPIPE if status == 2 else -errno.ENODEV)
            await asyncio.sleep(.001)

    async def control(self, setup, data=b''):
        async with self.control_lock:
            request_type, request, value, index, length = struct.unpack('<BBHHH', setup)
            await self.packet(3, 0, setup)
            result = bytearray()
            if request_type & 0x80:
                while len(result) < length:
                    packet = await self.packet(5)
                    result.extend(packet[:length-len(result)])
                    if len(packet) < 8:
                        break
                await self.packet(4)  # OUT status stage
            else:
                for offset in range(0, len(data), 8):
                    await self.packet(4, 0, data[offset:offset+8])
                await self.packet(5)  # IN status stage (including zero-data requests)
            return bytes(result)

    async def descriptor(self, kind, length):
        return await self.control(struct.pack('<BBHHH', 0x80, 6, kind << 8, 0, length))


class Export:
    def __init__(self, chip):
        self.chip = chip
        self.device = self.config = None
        self.active = False
        self.endpoint_locks = {}
        self.discovery_lock = asyncio.Lock()
        self.detached = asyncio.Event()
        self.detached.set()

    async def discover(self):
        async with self.discovery_lock:
            if not self.active:
                await self._discover()

    async def _discover(self):
        await self.chip.ready(enabled=False)
        await asyncio.sleep(.1)  # USB connection debounce before enumeration
        await self.reset_address()
        async with asyncio.timeout(10):
            self.device = await self.chip.descriptor(1, 18)
            first = await self.chip.descriptor(2, 9)
            if len(self.device) != 18 or len(first) != 9:
                raise ConnectionError('Firmware returned an incomplete USB descriptor')
            total = int.from_bytes(first[2:4], 'little')
            if not 9 <= total <= 4096:
                raise ConnectionError('Invalid configuration length')
            self.config = await self.chip.descriptor(2, total)
            if len(self.config) != total:
                raise ConnectionError('Firmware returned an incomplete USB configuration')
        LOG.info('Firmware USB device %04x:%04x',
                 int.from_bytes(self.device[8:10], 'little'),
                 int.from_bytes(self.device[10:12], 'little'))

    async def reset_address(self):
        await self.chip.request(2)
        await self.chip.ready()
        await asyncio.sleep(.02)  # reset recovery; let firmware leave its reset ISR
        # vhci_hcd handles SET_ADDRESS locally. The exported device must
        # already have an address, just as a physical usbip-host device does.
        async with asyncio.timeout(5):
            await self.chip.control(struct.pack('<BBHHH', 0, 5, 1, 0, 0))

    def interfaces(self):
        result = []
        offset = 0
        while offset + 2 <= len(self.config):
            length, kind = self.config[offset:offset+2]
            if length < 2 or offset + length > len(self.config):
                raise ValueError('Invalid firmware descriptor')
            d = self.config[offset:offset+length]
            if kind == 4 and length >= 9 and d[3] == 0:
                result.append(bytes(d[5:8]) + b'\0')
            offset += length
        return result

    def description(self):
        d = self.device
        return struct.pack('!256s32sIIIHHH6B', b'/mame/asma3k/usb', BUSID,
                           1, 1, 2, *struct.unpack('<HHH', d[8:14]),
                           *d[4:7], self.config[5], d[17], len(self.interfaces()))

    async def bulk(self, direction, ep, data, length, flags=0):
        key = (direction, ep)
        lock = self.endpoint_locks.setdefault(key, asyncio.Lock())
        async with lock:
            if direction:
                result = bytearray()
                while len(result) < length:
                    block = await self.chip.packet(5, ep)
                    result.extend(block[:length-len(result)])
                    if len(block) < 8:
                        break
                return bytes(result)
            for offset in range(0, length, 8):
                await self.chip.packet(4, ep, data[offset:offset+8])
            if length == 0 or (flags & 0x40 and length % 8 == 0):  # URB_ZERO_PACKET
                await self.chip.packet(4, ep)
            return b''

    async def client(self, reader, writer):
        tasks = {}
        imported = False
        monitor = None
        try:
            version, operation, status = struct.unpack('!HHI', await reader.readexactly(8))
            if version != 0x111 or status:
                return
            if operation == 0x8005:  # OP_REQ_DEVLIST
                if not self.active:
                    await self.discover()
                writer.write(struct.pack('!HHII', version, 5, 0, 1) + self.description()
                             + b''.join(self.interfaces()))
                await writer.drain()
                return
            if operation != 0x8003:
                return
            busid = (await reader.readexactly(32)).split(b'\0', 1)[0]
            if busid != BUSID or self.active:
                writer.write(struct.pack('!HHI', version, 3, 1))
                await writer.drain()
                return
            self.active = imported = True
            self.detached.clear()
            async with self.discovery_lock:
                await self.reset_address()
            writer.write(struct.pack('!HHI', version, 3, 0) + self.description())
            await writer.drain()
            LOG.info('USB/IP imported')

            async def watch_disconnect():
                try:
                    while True:
                        await asyncio.sleep(.05)
                        if not (await self.chip.state())[1]:
                            LOG.info('Firmware disconnected USB; closing import for re-enumeration')
                            return
                finally:
                    writer.close()
            monitor = asyncio.create_task(watch_disconnect())

            async def complete(header, payload):
                _, seq, devid, direction, ep, flags, length, frame, packets, interval, setup = header
                response = b''
                result = 0
                try:
                    if packets not in (0, 0xffffffff) or ep > 3:
                        raise USBError(-errno.EOPNOTSUPP)
                    # EP0 includes the full control transfer in one USB/IP URB.
                    if ep == 0:
                        response = await self.chip.control(setup, payload)
                    else:
                        response = await self.bulk(direction, ep, payload, length, flags)
                    if direction and flags & 1 and len(response) < length:  # SHORT_NOT_OK
                        result = -errno.EREMOTEIO
                except USBError as error:
                    result = error.status
                except asyncio.CancelledError:
                    raise
                except Exception:
                    LOG.exception('Transfer failed')
                    result = -errno.EIO
                actual = len(response) if direction else (length if result == 0 else 0)
                writer.write(struct.pack('!5Ii4I8x', 3, seq, 0, 0, 0, result,
                                         actual, 0, 0, 0) + (response if direction else b''))
                await writer.drain()

            while True:
                raw = await reader.readexactly(48)
                command, seq, devid, direction, ep = struct.unpack('!5I', raw[:20])
                if command == 1:  # USBIP_CMD_SUBMIT
                    header = struct.unpack('!10I8s', raw)
                    length, packets = header[6], header[8]
                    if length > 1024 * 1024 or direction > 1 or packets not in (0, 0xffffffff):
                        raise ConnectionError('Unsupported USB/IP transfer size/type')
                    payload = await reader.readexactly(length) if not direction else b''
                    if seq in tasks:
                        raise ConnectionError('Duplicate USB/IP sequence')
                    task = asyncio.create_task(complete(header, payload))
                    tasks[seq] = task
                    task.add_done_callback(lambda task, seq=seq: tasks.pop(seq, None))
                elif command == 2:  # USBIP_CMD_UNLINK
                    target = struct.unpack('!I', raw[20:24])[0]
                    task = tasks.get(target)
                    result = 0
                    if task and not task.done():
                        task.cancel()
                        with contextlib.suppress(asyncio.CancelledError):
                            await task
                        # Successful UNLINK replaces RET_SUBMIT (Linux USB/IP protocol).
                        result = -errno.ECONNRESET
                    writer.write(struct.pack('!5Ii24x', 4, seq, 0, 0, 0, result))
                    await writer.drain()
                else:
                    raise ConnectionError('Unknown USB/IP command')
        except (asyncio.IncompleteReadError, ConnectionError, BrokenPipeError):
            pass
        except Exception:
            LOG.exception('USB/IP connection failed')
        finally:
            if monitor:
                monitor.cancel()
                await asyncio.gather(monitor, return_exceptions=True)
            pending = list(tasks.values())
            for task in pending:
                task.cancel()
            await asyncio.gather(*pending, return_exceptions=True)
            if imported:
                self.active = False
                self.detached.set()
                LOG.info('USB/IP detached')
            writer.close()
            with contextlib.suppress(ConnectionError):
                await writer.wait_closed()


async def run(args):
    reader, writer = await asyncio.open_connection(args.mame_host, args.mame_port)
    chip = Chip(reader, writer)
    await chip.request(1)
    export = Export(chip)
    await export.discover()
    if args.probe:
        try:
            await probe(export)
        finally:
            await chip.request(6)
            writer.close()
            await writer.wait_closed()
        return
    server = await asyncio.start_server(export.client, args.host, args.port)
    LOG.info('USB/IP listening on %s:%s, bus ID %s', args.host, args.port, BUSID.decode())
    async def attach():
        while True:
            await export.detached.wait()
            await export.discover()
            process = await asyncio.create_subprocess_exec(
                args.usbip, '--tcp-port', str(args.port), 'attach', '-r', args.host, '-b', BUSID.decode())
            if await process.wait():
                raise RuntimeError('usbip attach failed; --attach needs permission to write vhci_hcd sysfs')
            # Import starts before usbip exits; detached is cleared by client().
            while export.active:
                await asyncio.sleep(.1)
            await asyncio.sleep(.2)
    try:
        async with server:
            if args.attach:
                await asyncio.gather(server.serve_forever(), attach())
            else:
                await server.serve_forever()
    finally:
        with contextlib.suppress(Exception):
            await chip.request(6)
        writer.close()
        await writer.wait_closed()


async def probe(export):
    """Exercise real firmware without requiring a kernel import (diagnostic)."""
    chip = export.chip
    async with asyncio.timeout(20):
        await chip.control(struct.pack('<BBHHH', 0, 9, 1, 0, 0))
        if int.from_bytes(export.device[10:12], 'little') == 0xbd04:
            for value in range(0xe0, 0xe5):
                await chip.control(struct.pack('<BBHHH', 0x20, 9, 0x200, 1, 1), bytes([value]))
            while (await chip.state())[1]:
                await asyncio.sleep(.001)
            await export.discover()
            await chip.control(struct.pack('<BBHHH', 0, 9, 1, 0, 0))
        endpoints = {}
        offset = 0
        while offset + 2 <= len(export.config):
            length, kind = export.config[offset:offset+2]
            if length < 2:
                raise ValueError('Invalid firmware configuration')
            d = export.config[offset:offset+length]
            if kind == 5 and length >= 7 and d[3] & 3 == 2:
                endpoints[bool(d[2] & 0x80)] = d[2] & 15
            offset += length
        if set(endpoints) != {False, True}:
            raise ValueError('Firmware did not expose communications bulk endpoints')
        await asyncio.sleep(.1)  # equivalent to neotools' re-enumeration wait
        await export.bulk(0, endpoints[False], b'\x01', 1)
        response = await export.bulk(1, endpoints[True], b'', 8)
        if response != b'\x02\x20':
            raise ValueError('Unexpected AlphaSmart Manager protocol response: ' + response.hex())
        LOG.info('PASS: firmware enumeration, comms mode switch and Manager protocol 0x0220')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mame-host', default='127.0.0.1')
    parser.add_argument('--mame-port', type=int, default=13240)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', type=int, default=3240)
    parser.add_argument('--attach', action='store_true', help='import locally and re-import after firmware mode changes (requires vhci sysfs access)')
    parser.add_argument('--usbip', default='/usr/sbin/usbip')
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--probe', action='store_true', help='test firmware enumeration/mode switch/Manager hello without a kernel import, then exit')
    args = parser.parse_args()
    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)
    try:
        asyncio.run(run(args))
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    main()
