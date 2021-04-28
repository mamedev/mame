// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of Apple Oric_Jasmin floppy images

#include "emu.h"
#include "fs_oric_jasmin.h"
#include "oric_dsk.h"

// Floppy only, format is 41 tracks, 1/2 heads, 17 sectors.
// Filesystem has no subdirectories.
//
// References are pair of bytes (track + head*41, sector)
//
// Track 20 sector 1 has the free bitmap and the volume name
//
//   offset 3*(track + head*41): bitmap from track, 24 bits LSB first,
//                               sector 1 in bit 16, sector 17 in bit 0.
//                               0x01ffff is empty track. Special value
//                               0x800000 marks a full track.
//   offset f6-f7: 0x8080
//   offset f8-ff: volume name, padded with 0x20
//
//
// Track 20 sector 2 has the first directory sector.
// Directory sector format:
//   offset 00-01: reference to the sector, (0, 0) for the first one
//   offset 02-03: reference to the next sector, (0, 0) on the first sector if it's the end, (ff, 00) for the end of another sector
//   offset 04+  : 14 file entries, 18 bytes each
//     offset 00-01: reference to the first sector of the inode, (ff, xx) when no entry
//     offset 02   : U/L for (U)nlocked or (L)ocked file
//     offset 03-0e: filename.ext, space padiing between filename and '.'
//     offset 0f   : S/D for (S)equential (normal) or (D)irect-access files
//     offset 10-11: number of sectors used by the file, including inode, little-endian
//
// Inodes:
//     offset 00-01: reference to the next inode sector, (ff, 00) on the last one
//     offset 02-03: loading address for the file on the first sector, ffff otherwise
//     offset 04-05: length of the file in bytes on the first sector, ffff otherwise
//     offset 06+  : reference to data sectors, (ff, ff) when done

void fs_oric_jasmin::enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	if(has(form_factor, variants, floppy_image::FF_3, floppy_image::DSDD))
		fe.add(this, FLOPPY_ORIC_JASMIN_FORMAT, 356864, "oric_jasmin_ds", "Oric Jasmin dual-sided");
	if(has(form_factor, variants, floppy_image::FF_3, floppy_image::SSDD))
		fe.add(this, FLOPPY_ORIC_JASMIN_FORMAT, 178432, "oric_jasmin_ss", "Oric Jasmin single-sided");
}

std::unique_ptr<filesystem_t> fs_oric_jasmin::mount(fsblk_t &blockdev) const
{
	return std::make_unique<impl>(blockdev);
}

bool fs_oric_jasmin::can_format() const
{
	return true;
}

bool fs_oric_jasmin::can_read() const
{
	return false;
}

bool fs_oric_jasmin::can_write() const
{
	return false;
}


void fs_oric_jasmin::impl::format()
{
	std::string volume_name = "UNTITLED";
	m_blockdev.set_block_size(256);
	u32 blocks = m_blockdev.block_count();

	m_blockdev.fill(0x6c);

	u32 bblk = 20*17;
	auto fmap = m_blockdev.get(bblk);
	u32 off = 0;
	for(u32 blk = 0; blk != blocks; blk += 17) {
		if(blk == bblk) {
			fmap.w8(off    , 0xff);
			fmap.w8(off + 1, 0x7f);
			fmap.w8(off + 2, 0x00);
		} else {
			fmap.w8(off    , 0xff);
			fmap.w8(off + 1, 0xff);
			fmap.w8(off + 2, 0x01);
		}
		off += 3;
	}

	for(u32 blk = blocks; blk != 17*42*2; blk += 17) {
		fmap.w8(off    , 0x00);
		fmap.w8(off + 1, 0x00);
		fmap.w8(off + 2, 0x80);
		off += 3;
	}

	fmap.w8(0xf6, 0x80);
	fmap.w8(0xf7, 0x80);
	fmap.fill(0xf8, 0x20, 8);
	fmap.wstr(0xf8, volume_name);

	auto bdir = m_blockdev.get(20*17+1);
	bdir.fill(0xff);
	bdir.w16l(0, 0x0000);
	bdir.w16l(2, 0x0000);
}

const filesystem_manager_type FS_ORIC_JASMIN = &filesystem_manager_creator<fs_oric_jasmin>;;
