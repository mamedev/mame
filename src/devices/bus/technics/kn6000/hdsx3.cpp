// license:GPL2+
// copyright-holders:Felipe Sanches

/***************************************************************************

    HD-SX3 hard disk expansion unit for the SX-KN6000 and SX-KN6500

    An optional unit, rarely encountered. No example has been available for
    inspection, so nothing here is derived from the hardware itself.

    What is known comes from three places. The KN6500 service manual shows the
    expansion connector CN106, 70 pins, labelled "TO HDD", carrying HDDCS,
    HDDINT, PP.INT, the audio clocks DACCK/BCK/LRCK, the DO1/DO2 outputs, the
    A/D bus and +/-15 V; the chip-select decoder on the same sheet emits
    EXP.CS0 and EXP.CS1. The host side is present in the keyboards' own
    firmware, which contains the strings "HD-SX3 MAIN MENU", "TT_EXTAPR",
    "TT_HDDEXT" and "HDDTEST_SW". And the firmware below carries its own
    identification: "PROTECT HDD", "HDD Format will erase all files at once",
    and "Version: 1.1 (REV3) Date: 07-21-2001".

    The ROM is the payload of Panasonic's own firmware update disk for this
    unit, decompressed from the SLIDE4K stream it ships in - the same kind of
    source as the program ROMs of the keyboards themselves. It links at
    0x97800000: the startup code clears BSS at 0x979126D8 and copies its data
    segment from 0x978B3E84 to 0x97910000. It begins with a four byte "XAPR"
    signature followed by a 31 entry export table, of which the last ten
    entries all point at a bare return instruction.

    Whether the code runs on the keyboard's own MN103002A through the
    expansion chip selects, or on a processor inside the unit, is not yet
    established. Until that is settled the connector's signals are not
    modelled and this device only carries the firmware.

***************************************************************************/

#include "emu.h"
#include "hdsx3.h"

namespace {

class hdsx3_device : public device_t, public device_kn6000_expansion_interface
{
public:
	hdsx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

hdsx3_device::hdsx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HDSX3, tag, owner, clock)
	, device_kn6000_expansion_interface(mconfig, *this)
{
}

void hdsx3_device::device_start()
{
}

ROM_START(hdsx3)
	ROM_REGION32_LE(0xc0000, "firmware", 0)
	ROM_LOAD("hd-sx3_v1_1.bin", 0x000000, 0x0c0000, CRC(83b8a6f1) SHA1(88699a7e9584e0c30c175babd1482e5aa586ad3d))
ROM_END

const tiny_rom_entry *hdsx3_device::device_rom_region() const
{
	return ROM_NAME(hdsx3);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HDSX3, device_kn6000_expansion_interface, hdsx3_device, "hdsx3", "HD-SX3 Hard Disk Expansion")
