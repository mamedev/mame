#!/usr/bin/env python3
"""Decode Gals Panic II type 0x20 images from logical big-endian subdata ROM.

Export with export_rom.lua first. The driver's imlist is for the Asia data;
other sets require their own offsets. PNG output requires Pillow.
"""

import argparse
from array import array
from dataclasses import dataclass
import json
from pathlib import Path
import re
import struct
import sys


@dataclass
class DecodedImage:
    offset: int
    width: int
    height: int
    end: int
    words: array
    packets: int
    final_run_remaining: int = 0

    def packed(self):
        words = array("H", self.words)
        if sys.byteorder == "little":
            words.byteswap()
        return words.tobytes()

    def metadata(self):
        return dict(offset=f"0x{self.offset:08x}", width=self.width,
                    height=self.height, pixels=len(self.words),
                    consumed=self.end - self.offset, packets=self.packets,
                    final_run_remaining=self.final_run_remaining)


def decode(data, offset, limit=None):
    """Consume exactly the header's pixel count, excluding word-alignment padding."""
    limit = len(data) if limit is None else limit
    if not 0 <= offset <= limit - 6 <= len(data) - 6:
        raise ValueError("header outside the supplied ROM range")
    kind, last_y, last_x = struct.unpack_from(">HHH", data, offset)
    if kind != 0x20:
        raise ValueError(f"unsupported image type 0x{kind:04x}; expected 0x0020")
    width, height = last_x + 1, last_y + 1
    if not (1 <= width <= 512 and 1 <= height <= 512):
        raise ValueError(f"implausible dimensions {width}x{height}")
    total = width * height
    words = array("H")
    position, packets = offset + 6, 0
    while len(words) < total:
        if position >= limit:
            raise ValueError("truncated packet control byte")
        control = data[position]
        position += 1
        count = (control & 0x7f) + 1
        # Like the 68000, stop as soon as the requested rectangle is filled.
        # Several genuine assets' final repeat runs extend past this boundary.
        write_count = min(count, total - len(words))
        size = 2 if control & 0x80 else write_count * 2
        if position + size > limit:
            raise ValueError("truncated packet pixels")
        if control & 0x80:
            color = struct.unpack_from(">H", data, position)[0]
            words.extend([color] * write_count)
        else:
            literal = array("H", data[position:position + size])
            if sys.byteorder == "little":
                literal.byteswap()
            words.extend(literal)
        position += size
        packets += 1
    return DecodedImage(offset, width, height, position, words, packets,
                        count - write_count)


def asia_offsets(driver):
    text = driver.read_text()
    table = text.split("imlist[794] = {", 1)[1].split("};", 1)[0]
    offsets = [int(value, 16) for value in re.findall(r"0x[0-9a-fA-F]+", table)]
    if len(offsets) != 794:
        raise ValueError("expected 794 entries in the Asia image table")
    return offsets


def save_png(decoded, path, rotate=0, scale=1):
    from PIL import Image
    rgb = bytearray()
    for value in decoded.words:
        # palette_device::GRB_555: G in bits 10..14, R in 5..9, B in 0..4.
        for component in ((value >> 5) & 31, (value >> 10) & 31, value & 31):
            rgb.append((component << 3) | (component >> 2))
    image = Image.frombytes("RGB", (decoded.width, decoded.height), bytes(rgb))
    if rotate:
        image = image.rotate(-rotate, expand=True)
    if scale != 1:
        image = image.resize((image.width * scale, image.height * scale),
                             Image.Resampling.NEAREST)
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    selection = parser.add_mutually_exclusive_group(required=True)
    selection.add_argument("--index", type=lambda x: int(x, 0),
                           help="Asia imlist index; do not use for Japanese data")
    selection.add_argument("--offset", type=lambda x: int(x, 0),
                           help="verified image header offset for any set")
    selection.add_argument("--validate-table", action="store_true",
                           help="validate all Asia type 0x20 entries")
    parser.add_argument("--driver", type=Path,
                        default=Path(__file__).resolve().parents[1] /
                        "src/mame/kaneko/galpani2.cpp")
    parser.add_argument("--output", type=Path, help="PNG filename")
    parser.add_argument("--words", type=Path, help="decoded big-endian 16-bit words")
    parser.add_argument("--manifest", type=Path, help="validation manifest filename")
    parser.add_argument("--rotate", type=int, choices=[0, 90, 180, 270], default=0,
                        help="clockwise display rotation")
    parser.add_argument("--scale", type=int, default=1)
    args = parser.parse_args()
    if args.scale < 1:
        parser.error("scale must be positive")
    data = args.rom.read_bytes()
    offsets = asia_offsets(args.driver) if args.offset is None else None
    if offsets and data[offsets[0]:offsets[0] + 6] != bytes.fromhex("002000ef013f"):
        parser.error("ROM does not match the Asia image table; use a verified --offset")
    if args.validate_table:
        records, errors, skipped = [], [], 0
        for index, start in enumerate(offsets):
            if struct.unpack_from(">H", data, start)[0] != 0x20:
                skipped += 1
                continue
            end = offsets[index + 1] if index + 1 < len(offsets) else len(data)
            try:
                decoded = decode(data, start, end)
                padding = end - decoded.end
                # Some streams include an extra complete black repeat packet;
                # the CPU's rectangle limit makes it unreachable.
                if padding > 4 and end % 0x200000:
                    raise ValueError(f"unexpected {padding} trailing bytes")
                records.append(dict(index=index, padding=padding,
                                    trailing_hex=data[decoded.end:end].hex()
                                    if padding <= 4 else None,
                                    **decoded.metadata()))
            except ValueError as error:
                errors.append(dict(index=index, offset=f"0x{start:08x}",
                                   error=str(error)))
        report = dict(valid=len(records), other_types=skipped, errors=errors,
                      images=records)
        if args.manifest:
            args.manifest.write_text(json.dumps(report, indent=2) + "\n")
        print(json.dumps({key: value for key, value in report.items()
                          if key != "images"}, indent=2))
        return bool(errors)
    if args.index is not None:
        if not 0 <= args.index < len(offsets):
            parser.error("index outside the Asia image table")
        start = offsets[args.index]
        limit = offsets[args.index + 1] if args.index + 1 < len(offsets) else len(data)
    else:
        start, limit = args.offset, None
    decoded = decode(data, start, limit)
    if args.output:
        save_png(decoded, args.output, args.rotate, args.scale)
    if args.words:
        args.words.write_bytes(decoded.packed())
    print(json.dumps(decoded.metadata(), indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
