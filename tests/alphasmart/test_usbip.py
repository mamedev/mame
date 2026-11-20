# license:BSD-3-Clause
"""Wire-level checks against the bridge, independent of MAME/controller code."""
import asyncio
import importlib.util
import pathlib
import struct
import unittest

path = pathlib.Path(__file__).resolve().parents[2] / 'scripts/alphasmart/usbip.py'
spec = importlib.util.spec_from_file_location('bridge', path)
bridge = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bridge)


class Firmware:
    async def ready(self):
        pass

    async def request(self, op):
        return 0, b''

    async def state(self):
        return b'\1\1\2\x80'

    async def control(self, setup, data=b''):
        return b'\x12\x01USB'

    async def packet(self, op, ep=0, data=b''):
        await asyncio.Event().wait()


class WireTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.export = bridge.Export(Firmware())
        self.export.device = bytes.fromhex('12011001000000081e0801bd000101020301')
        self.export.config = bytes.fromhex('0902190001010080320904000001ff00000007058102080000')
        self.server = await asyncio.start_server(self.export.client, '127.0.0.1', 0)
        self.port = self.server.sockets[0].getsockname()[1]

    async def asyncTearDown(self):
        self.server.close()
        await self.server.wait_closed()

    async def connect(self):
        return await asyncio.open_connection('127.0.0.1', self.port)

    async def test_import_transfer_and_unlink(self):
        r,w = await self.connect()
        w.write(struct.pack('!HHI32s', 0x111, 0x8003, 0, b'1-1'))
        await w.drain()
        reply = await r.readexactly(320)
        self.assertEqual(reply[:8], struct.pack('!HHI', 0x111, 3, 0))
        self.assertEqual(reply[308:312], bytes.fromhex('081ebd01'))
        setup = struct.pack('<BBHHH', 0x80, 6, 256, 0, 5)
        w.write(struct.pack('!10I8s', 1, 10, 65537, 1, 0, 0, 5, 0, 0, 0, setup))
        await w.drain()
        urb = struct.unpack('!5Ii4I8x', await r.readexactly(48))
        self.assertEqual(urb, (3,10,0,0,0,0,5,0,0,0))
        self.assertEqual(await r.readexactly(5), b'\x12\x01USB')
        w.write(struct.pack('!10I8s', 1, 11, 65537, 1, 1, 0, 8, 0, 0, 0, bytes(8)))
        await w.drain()
        await asyncio.sleep(.02)
        w.write(struct.pack('!6I24x', 2, 12, 65537, 0, 0, 11))
        await w.drain()
        unlink = struct.unpack('!5Ii24x', await r.readexactly(48))
        self.assertEqual(unlink, (4,12,0,0,0,-104))
        with self.assertRaises(TimeoutError):
            await asyncio.wait_for(r.read(1), .05)  # no RET_SUBMIT for unlinked URB
        w.close()
        await w.wait_closed()
        await asyncio.sleep(.01)
        self.assertFalse(self.export.active)

    async def test_packet_cancellation_drains_reply(self):
        received = asyncio.Event()
        async def peer(r,w):
            for n in range(2):
                header = await r.readexactly(20)
                magic, seq, op, ep, count = bridge.HEADER.unpack(header)
                await r.readexactly(count)
                received.set()
                await asyncio.sleep(.03)
                w.write(bridge.HEADER.pack(magic,seq,0,ep,1)+bytes([op]))
                await w.drain()
            w.close()
        server = await asyncio.start_server(peer, '127.0.0.1', 0)
        async with server:
            r,w = await asyncio.open_connection('127.0.0.1', server.sockets[0].getsockname()[1])
            chip = bridge.Chip(r,w)
            task = asyncio.create_task(chip.request(3))
            await received.wait()
            task.cancel()
            with self.assertRaises(asyncio.CancelledError):
                await task
            self.assertEqual(await chip.request(5), (0,b'\5'))
            w.close()
            await w.wait_closed()


if __name__ == '__main__':
    unittest.main()
