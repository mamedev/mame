// license:BSD-3-Clause copyright-holders:Michael R. Furman
/*
 * fdos_dsk.cpp  -  FDOS compatible disk images
 *
 * Created on: 24/08/2022
 *
 * This FDOS floppy disk image support leverages the wd177x_format support with
 * verification of the disk formatting and the the ability to support format
 * variations.
 *
 * The FDOS Disk format is as follows:
 *
 * Geometry:: 10 Sectors * 35 Tracks * 256 Bytes * 1 Side
 * Sectors Numbered from 0-9
 * Gap Bytes: Verified from FDOS source code
 *
 * Disk Format::
 *
 * Operating System:: Tracks 0-1
 * Directory:: Track 2
 * Data:: Tracks 3-34
 *
 *
 * Note: Due to hardware differences between the original DC-1 (WD1771)
 * controller for which FDOS was designed and DC-4 (WD1797) or DC-5 (WD2797)
 * Controllers (DC-5 is Emulated in MAME), the machine can only be booted with
 * a specific disk image from deramp.com (Mike Douglas) at the following
 * location.  The other disks available in the same directory can then be read
 * once FDOS is loaded. This disk image also works on real hardware with a DC-4
 * or PT FD-2A controllers.
 *
 * https://deramp.com/downloads/swtpc/software/FDOS/Disk%20Images/FDOSMPS.DSK
 *
 * This disk contains the all of these required patches:
 *
 * https://deramp.com/downloads/swtpc/software/FDOS/Disk%20Images/Patches/
 */

#include "fdos_dsk.h"

#include "imageutl.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <tuple>


namespace
{
	class fdos_formats : public wd177x_format
	{
	public:
		struct dirent_entry_fdos
		{
			char filename[8] = {};
			char password[8] = {};
			uint8_t start_track = 0;
			uint8_t start_sector = 0;
			uint8_t num_sectors[2] = {};
			uint8_t file_type = 0;
			uint8_t start_addr[2] = {};
			uint8_t end_addr[2] = {};
			uint8_t exec_addr[2] = {};
			uint8_t basic_high_line[2] = {};
			uint8_t spares[3] = {};
		};

		static const format formats[];
	};
}

fdos_format::fdos_format() : wd177x_format(fdos_formats::formats)
{
}

const char *fdos_format::name() const noexcept
{
	return "fdos";
}

const char *fdos_format::description() const noexcept
{
	return "FDOS compatible disk image";
}

const char *fdos_format::extensions() const noexcept
{
	return "dsk";
}

int fdos_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	int type = find_size(io, form_factor, variants);

	if (type != -1)
		return FIFID_SIZE;
	return 0;
}


int fdos_format::find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return -1;

	uint8_t boot0[256];
	fdos_formats::dirent_entry_fdos info;
	size_t actual;
	std::error_condition ec;

	for (int i=0; fdos_formats::formats[i].form_factor; i++) {
		const format &f = fdos_formats::formats[i];

		// Format Check
		// Check byte 0 and byte 3 of Track 0 Sector 0
		// 00320 2400 BD 240C START  JSR   BOOT
		// 00330 2403 DE OB   RESTRT LDX   PROGX
		// Should be  $BD and $DE respectively
		// There could be additional variations
		std::tie(ec, actual) = read_at(io, 0, &boot0, f.sector_base_size);
		if (ec || actual != f.sector_base_size)
			return -1;
		if (boot0[0] != 0xbd && boot0[3] != 0xde)
			continue;

		LOG_FORMATS("FDOS floppy dsk: size %u bytes, %u total sectors, %u remaining bytes, expected form factor %x\n",
				size,
				size / f.sector_base_size,
				size % f.sector_base_size,
				form_factor);

		// Directory entries start at Track 2 Sector 0
		std::tie(ec, actual) = read_at(io, 2 * f.sector_count * f.sector_base_size, &info, sizeof(struct fdos_formats::dirent_entry_fdos));
		if (ec || actual != sizeof(struct fdos_formats::dirent_entry_fdos))
			continue;

		// First directory entry should be "$DOS"
		if (memcmp(info.filename, "$DOS    ", 8) != 0)
			continue;
		if (memcmp(info.password, "        ", 8) != 0)
			continue;
		if (info.start_track != 0)
			continue;
		if (info.start_sector != 0)
			continue;
		if (get_u16be(info.num_sectors) != 0x0014)
			continue;
		// $DOS File type is supposed to be $11 but some disks (FDOSMPS) have $00
		if (info.file_type != 0 && info.file_type != 0x11)
			continue;
		if (get_u16be(info.start_addr) != 0x2400)
			continue;
		if (get_u16be(info.end_addr) != 0x2fff)
			continue;
		// FDOS entry is supposed to be $2600 but some disks have $2400
		uint16_t exec = get_u16be(info.exec_addr);
		if (exec != 0x2600 && exec != 0x2400)
			continue;


		unsigned int format_size = 0;
		for (int track=0; track < f.track_count; track++) {
			for (int head=0; head < f.head_count; head++) {
				const format &tf = get_track_format(f, head, track);
				format_size += compute_track_size(tf);
			}
		}

		if (format_size != size)
			continue;


		LOG_FORMATS("FDOS matching format index %d\n", i);
		return i;
	}
	return -1;
}

const fdos_formats::format fdos_formats::formats[] = {
	{ // 0 89.6K 5 1/4 inch single density fdos format
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 35, 1, 256, {}, 0, {}, 11, 11, 11
	},
	{}
};

const fdos_format FLOPPY_FDOS_FORMAT;
