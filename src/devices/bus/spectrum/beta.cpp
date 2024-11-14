// license:BSD-3-Clause
// copyright-holders:Nigel Barnes, David Haywood, MetalliC
/*********************************************************************

    Technology Research Beta Disk interface & clones
    these are designed for the 48k Spectrum models

    There are multiple versions of this

    'hand made' PCB with V2 ROM:
     - possible prototype / low production run
     - 4Kbyte ROM
     - D0 and D7 ROM lines swapped
     - FORMAT, COPY etc. must be loaded from a disk to be used
     - disks are password protected
     - uses FD1771 disk controller
     - disk format: FM, 7 or 10 256-byte sectors per track, 2 system sectors (max 30 files)
     - supports up to 3 drives
       https://www.youtube.com/watch?v=gSJIuZjbFYs

    Original Beta Disk release with V3 ROM:
     similar to above but:
     - uses a FD1793 controller
     - disk format: MFM, 16 256-byte sectors per track, 9 system sectors (max 128 files)
     - port FF bits was changed
     - supports up to 4 drives

    Re-release dubbed "Beta Disk plus" with V4 ROM:
     similar to above but:
     - 8Kbyte ROM
     - FORMAT and COPY commands moved into ROM
     - adds 'magic button' to dump the running state of the machine to disk
     - disk password system removed
     - adds auto boot feature

    Many clones exist, some specific to the various Spectrum clones.

    Original Beta Disk (V2) clones
     - Sandy FDD2 SP-DOS (straight clone?)

    Original Beta Disk (V3) clones
     - AC DOS P.Z.APINA (straight clone?)

    Beta Disk plus (V4) clones
     - CAS DOS Cheyenne Advanced System (supposedly straight clone)
     - common Brazilian clones: usually use FD1797 instead of FD1793, no ROM bits D0/D7 swap, DOS ROM area 3CXX might be disabled, equiped with printer port.
       - CBI-95
       - SYNCHRON IDS91
       - SYNCHRON IDS2001ne (added interface disable jumper)
       - ARCADE AR-20
     - MIDAS Gammadisk - 3 versions known to exists:
        - straight clone
        - added i8255 PPI for joystick and printer, uses 32KByte ROM with modified BASIC48 ROM in one half and TR-DOS 4.12 in another
        - same as above plus 6850 ACIA
     - number of straight clones with "Vision Desktop" boot ROM

    Some units also exist that allow population of both V3 and V4
    ROM types with a switch (unofficial, for compatibility?)

    ---

    NOTE:

    beta128.cpp could be modified to expand on this, as it builds
    on the features of the betaplus, but for now I've kept them
    separate as the enable / disable mechanisms are different and
    remaining mappings of devices unconfirmed

    ---

    Based on older BDI schematics, the logic is:

    memory access 0x3CXX (any type of access: code or data, read or write) -> temporary use BDI ROM (NOT permanent latch/switch like in beta128)
    memory access <0x4000 area and BDI ROM_latch==true -> use BDI ROM

    IO write to port 0bxxxxxx00 -> D7 - BDI I/O access, 0=enable, 1=disable (when BDI I/O enabled - /IORQ passthrough to next expansions is blocked)
                       CBI clones: D6 - 1=disable permanent BDI ROM mapping at 3c00-3cff, 0=enable (like original BDI)
                       Gammadisk : D6 - 0 - enable Gammadisk I/O (/IORQ passthrough blocked), 1=disable all Gammadisk I/O (enable /IORQ passthrough)
                                   D7 - (have effect if D6 is 0) 0=enable FDC I/O, 1=enable PPI and ACIA I/O;

    BDI I/O ports:

    IO write to port 0b1xxxx111 ->
      V2: D7 BDI ROM_latch (0=enable, 1=disble), D4 - FDC HLT, D3 - SIDE, D0-2 - floppy drive select (bitmask, active low).
      V3-V4: D7 BDI ROM_latch (0=enable, 1=disble), D6 - FDC DDEN, D4 - SIDE, D3 - FDC HLT, D2 - FDC /MR (reset), D0-1 - floppy drive select (binary value).
      CBI clones: D5 - printer port /STROBE
    IO read port 0b1xxxx111 <- D7 - FDC INTRQ, D6 - FDC DRQ
      CBI clones: D5 - printer port BUSY, D4 - printer standard select jumper (ABICOMP / MSX1.1)
    IO read/write ports 0b0YYxx111 - access FDC ports YY

    So mostly the same as beta128, except for new BDI ROM_latch bit


*********************************************************************/

#include "emu.h"
#include "beta.h"

#include "formats/trd_dsk.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_BETAV2,   spectrum_betav2_device,   "spectrum_betav2",   "TR Beta Disk Interface V2 (FD1771 based)")
DEFINE_DEVICE_TYPE(SPECTRUM_BETAV3,   spectrum_betav3_device,   "spectrum_betav3",   "TR Beta Disk Interface V3 (FD1793 based)")
DEFINE_DEVICE_TYPE(SPECTRUM_BETAPLUS, spectrum_betaplus_device, "spectrum_betaplus", "TR Beta Disk Plus Interface")
DEFINE_DEVICE_TYPE(SPECTRUM_BETACLONE,spectrum_betaclone_device,"spectrum_betaclone","Beta Disk Plus clones")
DEFINE_DEVICE_TYPE(SPECTRUM_BETACBI,  spectrum_betacbi_device,  "spectrum_betacbi",  "CBI-95 Disk Interface")
DEFINE_DEVICE_TYPE(SPECTRUM_GAMMA,    spectrum_gamma_device,    "spectrum_gamma",    "MIDAS Gammadisk Interface")

//-------------------------------------------------
//  SLOT_INTERFACE( beta_floppies )
//-------------------------------------------------

static void beta_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("3dsdd", FLOPPY_3_DSDD);
}

//-------------------------------------------------
//  floppy_formats
//-------------------------------------------------

void spectrum_betav2_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TRD_FORMAT);
}

//-------------------------------------------------
//  ROM( beta )
//-------------------------------------------------

ROM_START(betav2)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd20")
	ROM_SYSTEM_BIOS(0, "trd20", "TR-DOS v2.0")
	ROMX_LOAD("trd20.bin", 0x0000, 0x1000, CRC(dd269fb2) SHA1(ab394a19461f314fffd592645a273b85e76fadec), ROM_BIOS(0))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
	ROM_SYSTEM_BIOS(1, "sandy", "SP-DOS v1.0 (Sandy)") // match 2.0 ROM, only name text and checksum was changed
	ROMX_LOAD("sandy.bin", 0x0000, 0x1000, CRC(a51b820f) SHA1(1c5ec9f56651ae527cdc092af1c602803104d5c3), ROM_BIOS(1))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
ROM_END


// there is an alt set CRC(48f9149f) SHA1(52774757096fdc93ea94c55306481f6f41204e96) with differences at 30e, 3cd, 404, 7bd, it appears to be a bad dump
// 30c : call $2acd        call $6acd
// 3cc : ld hl, $5b00      ld hl, $5b01
// 402 : call $2acd        call $6bcd
// 7bd : ld ($5d02), hl    inc hl, ld (bc),a, ld e,l

// the Profisoft version appears to be different code, maybe based on a different revision or even modified hw?

ROM_START(betav3)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd30")
	ROM_SYSTEM_BIOS(0, "trd30", "TR-DOS v3.0")
	ROMX_LOAD("trd30.bin", 0x0000, 0x1000, CRC(c814179d) SHA1(b617ab59639beaa7b5d8e5b4e4a543a8eb0217c8), ROM_BIOS(0))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
//  ROM_SYSTEM_BIOS(1, "trd30a", "TR-DOS v3.0 (set 2)")
//  ROMX_LOAD("trd30_alt.bin", 0x0000, 0x1000, CRC(48f9149f) SHA1(52774757096fdc93ea94c55306481f6f41204e96), ROM_BIOS(1))
//  ROM_RELOAD(0x1000,0x1000)
//  ROM_RELOAD(0x2000,0x1000)
//  ROM_RELOAD(0x3000,0x1000)
	ROM_SYSTEM_BIOS(1, "trd30p", "TR-DOS v3.0 (Profisoft)") // is this a clone device?
	ROMX_LOAD("trd30ps.bin", 0x0000, 0x1000, CRC(b0f175a3) SHA1(ac95bb4d89072224deef58a1655e8029f811a7fa), ROM_BIOS(1))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
	ROM_SYSTEM_BIOS(2, "acdos", "AC DOS v1.0") // match 3.0 ROM, only name text and checksum was changed
	ROMX_LOAD("acdos.bin", 0x0000, 0x1000, CRC(c6d90a1e) SHA1(9480e5402bb8611b88657107a8aad889f353071b), ROM_BIOS(2))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
ROM_END

ROM_START(betaplus)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd412")
	ROM_SYSTEM_BIOS(0, "trd409", "TR-DOS v4.09")
	ROMX_LOAD("trd409.bin", 0x0000, 0x2000, CRC(18365bdc) SHA1(a0e7c80905423c54bd497575026ea8821061175a), ROM_BIOS(0))
	ROM_RELOAD(0x2000,0x2000)
	ROM_SYSTEM_BIOS(1, "trd411", "TR-DOS v4.11")
	ROMX_LOAD("trd411.bin", 0x0000, 0x2000, CRC(26902902) SHA1(cb90fc31bf62d5968730db23a600344338e31e7e), ROM_BIOS(1))
	ROM_RELOAD(0x2000,0x2000)
	ROM_SYSTEM_BIOS(2, "trd412", "TR-DOS v4.12")
	ROMX_LOAD("trd412.bin", 0x0000, 0x2000, CRC(f72efdf4) SHA1(b53ea4020be846ffc02ce586f7b52938c2c76f98), ROM_BIOS(2))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_SYSTEM_BIOS(3, "trd4127", "TR-DOS v4.12 (FD1797)") // 6 bytes was changed for FD1797 compatibility (type II commands sector size bit), more probable from clone board.
	ROMX_LOAD("trd4127.bin", 0x0000, 0x2000, CRC(4313234c) SHA1(214a10c9d2edf386c4685c0379efa2e87ff0e20c), ROM_BIOS(3))
	ROM_RELOAD(0x2000,0x2000)
	ROM_SYSTEM_BIOS(4, "cas86", "CAS DOS 1986") // v4.12 with name text changed
	ROMX_LOAD("cas1986.bin", 0x0000, 0x2000, CRC(da0cfdcc) SHA1(a2dbcf2f899250b9e7c5c7b31969667024457a72), ROM_BIOS(4))
	ROM_RELOAD(0x2000,0x2000)
	ROM_SYSTEM_BIOS(5, "cas87", "CAS DOS 1987") // 4.12 with texts translated
	ROMX_LOAD("cas1987.bin", 0x0000, 0x2000, CRC(a6a9450c) SHA1(b54b173a9f11ed730804d94584a1679193993e3c), ROM_BIOS(5))
	ROM_RELOAD(0x2000,0x2000)
	ROM_SYSTEM_BIOS(6, "icdos", "IC-DOS JUMBO v2.0") // v4.11 with name text changed
	ROMX_LOAD("icdos.bin", 0x0000, 0x2000, CRC(1398d13a) SHA1(bf8cee39eb4b2a09a3f07db4cb8e7952bce4d3dd), ROM_BIOS(6))
	ROM_RELOAD(0x2000,0x2000)
ROM_END

ROM_START(betaclone)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("vision10")
	ROM_SYSTEM_BIOS(0, "vision10", "Vision 1.0 TR-DOS v4.12")
	ROMX_LOAD("vision10.bin", 0x0000, 0x4000, CRC(20f49ff5) SHA1(d391324cb39e9fc6b93b867a71caec019c353518), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "vision14", "Vision 1.4 TR-DOS v4.40")
	ROMX_LOAD("vision14.bin", 0x0000, 0x4000, CRC(9903f577) SHA1(579627a2865caf3079cbae360bdaf0309d4a2911), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "vision17b", "Vision 1.7 TR-DOS v4.82")
	ROMX_LOAD("vision17b.bin", 0x0000, 0x4000, CRC(ecee1ff1) SHA1(ffde2a0f9b13328b6462fbf4eb95cfe8202a2d3f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "vision17c", "Vision 1.7 TR-DOS v4.83")
	ROMX_LOAD("vision17c.bin", 0x0000, 0x4000, CRC(c192a6b1) SHA1(d9eb1e06e28b2fb45c704cffd4a8b8a912041d08), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "vision18", "Vision 1.8 TR-DOS v4.80")
	ROMX_LOAD("vision18.bin", 0x0000, 0x4000, CRC(3ee7a488) SHA1(57167b82a6a3ffa8b25e97fa8cc208da361f1cc7), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "vision19", "Vision 1.9 TR-DOS v4.81")
	ROMX_LOAD("vision19.bin", 0x0000, 0x4000, CRC(1a793d53) SHA1(cdbd0ac2262ede3852998174f88ae333419a570f), ROM_BIOS(5))
ROM_END

ROM_START(betacbi)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("cbi24")
	ROM_SYSTEM_BIOS(0, "cbi23", "CBI-95 v2.3")
	ROMX_LOAD("cbi95-23.bin", 0x0000, 0x4000, CRC(7fbefca6) SHA1(aa0c10162d440bc569648c524bbfc0b6d7c0b0dd), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "cbi24", "CBI-95 v2.4")
	ROMX_LOAD("cbi95-24.bin", 0x0000, 0x4000, CRC(afe4cec3) SHA1(6a30ab6997ac91798830f5f5d9645ba150050760), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ids91", "SYNCHRON IDS91")
	ROMX_LOAD("ids91.bin", 0x0000, 0x4000, CRC(d569c67a) SHA1(50de5dbd49d8e03c97e416d4e0586870b90a4b87), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "ids2001", "SYNCHRON IDS2001ne")
	ROMX_LOAD("ids2001.bin", 0x0000, 0x4000, CRC(a1701f84) SHA1(32bd5f980bec168677513c568c8381a9a59aada3), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "ar20", "ARCADE AR-20")
	ROMX_LOAD("ar-20.bin", 0x0000, 0x4000, CRC(0ceeaa4d) SHA1(3e5f7d43218b5bd2a6e2aa28220c77cb651f5daa), ROM_BIOS(4))
ROM_END

ROM_START(gamma)
	ROM_REGION(0x8000, "rom", 0)
	ROM_DEFAULT_BIOS("gamma")
	ROM_SYSTEM_BIOS(0, "gamma", "Gammadisk v4.1")
	ROMX_LOAD("gamma.bin", 0x0000, 0x8000, CRC(00e4a835) SHA1(dffeb9f47b3eca94afbb61373eba003efd4675ff), ROM_BIOS(0))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_betav2_device::device_add_mconfig_base(machine_config& config)
{
	FLOPPY_CONNECTOR(config, "fdc:0", beta_floppies, "525qd", spectrum_betav2_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", beta_floppies, "525qd", spectrum_betav2_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", beta_floppies, nullptr, spectrum_betav2_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", beta_floppies, nullptr, spectrum_betav2_device::floppy_formats).enable_sound(true);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

void spectrum_betav2_device::device_add_mconfig(machine_config &config)
{
	FD1771(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->hld_wr_callback().set(FUNC(spectrum_betav2_device::fdc_hld_w));

	device_add_mconfig_base(config);
}

void spectrum_betav3_device::device_add_mconfig(machine_config& config)
{
	FD1793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->hld_wr_callback().set(FUNC(spectrum_betav3_device::fdc_hld_w));

	device_add_mconfig_base(config);
}

void spectrum_betacbi_device::device_add_mconfig(machine_config& config)
{
	FD1797(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->hld_wr_callback().set(FUNC(spectrum_betacbi_device::fdc_hld_w));

	device_add_mconfig_base(config);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this](u8 data) { m_centronics_busy = data; });
}

void spectrum_gamma_device::device_add_mconfig(machine_config& config)
{
	FD1793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->hld_wr_callback().set(FUNC(spectrum_gamma_device::fdc_hld_w));

	device_add_mconfig_base(config);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("JOY");
	m_ppi->out_pb_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_ppi->out_pc_callback().set([this](u8 data) { m_centronics->write_strobe(BIT(data, 7)); });

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this](u8 data) { m_centronics_busy = data; });
	m_centronics->set_output_latch(cent_data_out);

	ACIA6850(config, m_acia, 0); // schematics missing, wiring unknown
}

const tiny_rom_entry *spectrum_betav2_device::device_rom_region() const
{
	return ROM_NAME(betav2);
}

const tiny_rom_entry *spectrum_betav3_device::device_rom_region() const
{
	return ROM_NAME(betav3);
}

const tiny_rom_entry *spectrum_betaplus_device::device_rom_region() const
{
	return ROM_NAME(betaplus);
}

const tiny_rom_entry *spectrum_betaclone_device::device_rom_region() const
{
	return ROM_NAME(betaclone);
}

const tiny_rom_entry *spectrum_betacbi_device::device_rom_region() const
{
	return ROM_NAME(betacbi);
}

const tiny_rom_entry *spectrum_gamma_device::device_rom_region() const
{
	return ROM_NAME(gamma);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_betav2_device - constructor
//-------------------------------------------------

spectrum_betav2_device::spectrum_betav2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
	, m_control(0)
{
}

spectrum_betav2_device::spectrum_betav2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav2_device(mconfig, SPECTRUM_BETAV2, tag, owner, clock)
{
}

spectrum_betav3_device::spectrum_betav3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav2_device(mconfig, type, tag, owner, clock)
{
}

spectrum_betav3_device::spectrum_betav3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav3_device(mconfig, SPECTRUM_BETAV3, tag, owner, clock)
{
}

spectrum_betaplus_device::spectrum_betaplus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav3_device(mconfig, type, tag, owner, clock)
{
}

spectrum_betaplus_device::spectrum_betaplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betaplus_device(mconfig, SPECTRUM_BETAPLUS, tag, owner, clock)
{
}

spectrum_betaclone_device::spectrum_betaclone_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betaplus_device(mconfig, type, tag, owner, clock)
{
}

spectrum_betaclone_device::spectrum_betaclone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betaclone_device(mconfig, SPECTRUM_BETACLONE, tag, owner, clock)
{
}

spectrum_betacbi_device::spectrum_betacbi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betaclone_device(mconfig, type, tag, owner, clock)
	, m_centronics(*this, "centronics")
	, m_centronics_busy(0)
{
}

spectrum_betacbi_device::spectrum_betacbi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betacbi_device(mconfig, SPECTRUM_BETACBI, tag, owner, clock)
{
}

spectrum_gamma_device::spectrum_gamma_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betaplus_device(mconfig, type, tag, owner, clock)
	, m_ppi(*this, "ppi")
	, m_acia(*this, "acia")
	, m_centronics(*this, "centronics")
	, m_centronics_busy(0)
{
}

spectrum_gamma_device::spectrum_gamma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_gamma_device(mconfig, SPECTRUM_GAMMA, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_betav2_device::device_start()
{
	save_item(NAME(m_romcs));
	save_item(NAME(m_masterdisable));
	save_item(NAME(m_control));
	save_item(NAME(m_motor_active));

#if 0 // we do this in realtime instead
	for (int i = 0; i < m_rom->bytes(); i++)
	{
		uint8_t* rom = m_rom->base();

		rom[i] = bitswap<8>(rom[i],0,6,5,4,3,2,1,7);
	}
#endif
}

void spectrum_betacbi_device::device_start()
{
	spectrum_betav2_device::device_start();
	save_item(NAME(m_centronics_busy));
}

void spectrum_gamma_device::device_start()
{
	spectrum_betav2_device::device_start();
	save_item(NAME(m_centronics_busy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_betav2_device::device_reset()
{
	// always paged in on boot, no mode switch like beta128.
	m_romcs = 1;
	m_control = 0;
	m_masterdisable = 0;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_betav2_device::romcs()
{
	return m_romcs || m_exp->romcs();
}

void spectrum_betav2_device::fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if ((offset & 0xff00) == 0x3c00)
			m_romcs = 1;
		else
			m_romcs = 0;

		if (!(m_control & 0x80))
		{
			if (offset < 0x4000)
				m_romcs = 1;
		}
	}
}

void spectrum_betacbi_device::fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if ((offset & 0xff00) == 0x3c00 && !(m_masterdisable & 0x40))
			m_romcs = 1;
		else
			m_romcs = 0;

		if (!(m_control & 0x80))
		{
			if (offset < 0x4000)
				m_romcs = 1;
		}
	}
}

void spectrum_betav2_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);
	fetch(offset);
}

void spectrum_betav2_device::pre_data_fetch(offs_t offset)
{
	m_exp->pre_data_fetch(offset);
	fetch(offset);
}

uint8_t spectrum_betav2_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (!(m_masterdisable & 0x80))
	{
		switch (offset & 0x87)
		{
		case 0x07:
			data = m_fdc->read((offset >> 5) & 0x03);
			break;

		case 0x87:
			data &= 0x3f; // actually open bus
			data |= m_fdc->drq_r() ? 0x40 : 0;
			data |= m_fdc->intrq_r() ? 0x80 : 0;
			break;
		}
	}
	else
		data = m_exp->iorq_r(offset);

	return data;
}

void spectrum_betav2_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 3) == 0)
		m_masterdisable = data;

	if (!(m_masterdisable & 0x80))
	{
		switch (offset & 0x87)
		{
		case 0x07:
			m_fdc->write((offset >> 5) & 0x03, data);
			break;

		case 0x87:
			m_control = data;

			floppy_image_device* floppy = nullptr;
			for (int i = 0; i < 3; i++)
				if (!(data & (1 << i)))
				{
					floppy = m_floppy[i]->get_device();
					break;
				}

			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 3) ? 0 : 1);

			m_fdc->hlt_w(BIT(data, 4));

			motors_control();
			break;
		}
	}
	else
		m_exp->iorq_w(offset, data);
}

void spectrum_betav3_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 3) == 0)
		m_masterdisable = data;

	if (!(m_masterdisable & 0x80))
	{
		switch (offset & 0x87)
		{
		case 0x07:
			m_fdc->write((offset >> 5) & 0x03, data);
			break;

		case 0x87:
			m_control = data;

			floppy_image_device* floppy = m_floppy[data & 3]->get_device();

			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 4) ? 0 : 1);
			m_fdc->dden_w(BIT(data, 6));

			m_fdc->hlt_w(BIT(data, 3));

			m_fdc->mr_w(BIT(data, 2));
			motors_control();
			break;
		}
	}
	else
		m_exp->iorq_w(offset, data);
}

uint8_t spectrum_betacbi_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (!(m_masterdisable & 0x80))
	{
		switch (offset & 0x87)
		{
		case 0x07:
			data = m_fdc->read((offset >> 5) & 0x03);
			break;

		case 0x87:
			data &= 0x0f; // actually open bus
			data |= 0x10; // some switch, unk purpose
			data |= m_centronics_busy ? 0x20: 0;
			data |= m_fdc->drq_r() ? 0x40 : 0;
			data |= m_fdc->intrq_r() ? 0x80 : 0;
			break;
		}
	}
	else
		data = m_exp->iorq_r(offset);

	return data;
}

void spectrum_betacbi_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 3) == 0)
		m_masterdisable = data;

	if (!BIT(offset, 2))
	{
		m_centronics->write_data0(BIT(data, 0));
		m_centronics->write_data1(BIT(data, 1));
		m_centronics->write_data2(BIT(data, 2));
		m_centronics->write_data3(BIT(data, 3));
		m_centronics->write_data4(BIT(data, 4));
		m_centronics->write_data5(BIT(data, 5));
		m_centronics->write_data6(BIT(data, 6));
		m_centronics->write_data7(BIT(data, 7));
	}

	if (!(m_masterdisable & 0x80))
	{
		switch (offset & 0x87)
		{
		case 0x07:
			m_fdc->write((offset >> 5) & 0x03, data);
			break;

		case 0x87:
			m_control = data;

			floppy_image_device* floppy = m_floppy[data & 3]->get_device();

			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 4) ? 0 : 1);
			m_fdc->dden_w(BIT(data, 6));

			m_fdc->hlt_w(BIT(data, 3));

			m_fdc->mr_w(BIT(data, 2));
			motors_control();

			m_centronics->write_strobe(!BIT(data, 5));
			break;
		}
	}
	else
		m_exp->iorq_w(offset, data);
}

uint8_t spectrum_gamma_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (!(m_masterdisable & 0x40))
	{
		if (!(m_masterdisable & 0x80))
		{
			switch (offset & 0x9f)
			{
			case 0x1f:
				data = m_fdc->read((offset >> 5) & 0x03);
				break;

			case 0x9f:
				data &= 0x3f; // actually open bus
				data |= m_fdc->drq_r() ? 0x40 : 0;
				data |= m_fdc->intrq_r() ? 0x80 : 0;
				break;
			}
		}
		else
		{
			switch (offset & 0x9f)
			{
			case 0x1f:
				data = m_ppi->read((offset >> 5) & 0x03);
				break;

			case 0x9f:
				data = m_acia->read((offset >> 6) & 0x01);
				break;
			}
		}
	}
	else
		data = m_exp->iorq_r(offset);

	return data;
}

void spectrum_gamma_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 3) == 0)
		m_masterdisable = data;

	if (!(m_masterdisable & 0x40))
	{
		if (!(m_masterdisable & 0x80))
		{
			switch (offset & 0x9f)
			{
			case 0x1f:
				m_fdc->write((offset >> 5) & 0x03, data);
				break;

			case 0x9f:
				m_control = data;

				floppy_image_device* floppy = m_floppy[data & 3]->get_device();

				m_fdc->set_floppy(floppy);
				if (floppy)
					floppy->ss_w(BIT(data, 4) ? 0 : 1);
				m_fdc->dden_w(BIT(data, 6));

				m_fdc->hlt_w(BIT(data, 3));

				m_fdc->mr_w(BIT(data, 2));
				motors_control();
				break;
			}
		}
		else
		{
			switch (offset & 0x9f)
			{
			case 0x1f:
				m_ppi->write((offset >> 5) & 0x03, data);
				break;

			case 0x9f:
				m_acia->write((offset >> 6) & 0x01, data);
				break;
			}
		}
	}
	else
		m_exp->iorq_w(offset, data);
}

uint8_t spectrum_betav2_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		data = m_rom->base()[offset & 0x3fff];
		data = bitswap<8>(data,0,6,5,4,3,2,1,7); // proper dumps have bits 0 and 7 swapped?
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

uint8_t spectrum_betaclone_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0x3fff];

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

uint8_t spectrum_gamma_device::mreq_r(offs_t offset)
{
	return m_rom->base()[(offset & 0x3fff) + (m_romcs ? 0x4000 : 0)];
}

void spectrum_betav2_device::fdc_hld_w(int state)
{
	m_fdc->set_force_ready(state); // HLD connected to RDY pin
	m_motor_active = state;
	motors_control();
}

void spectrum_betav2_device::motors_control()
{
	for (int i = 0; i < 3; i++)
	{
		floppy_image_device* floppy = m_floppy[i]->get_device();
		if (!floppy)
			continue;
		if (m_motor_active && !(m_control & (1 << i)))
			floppy->mon_w(CLEAR_LINE);
		else
			floppy->mon_w(ASSERT_LINE);
	}
}

void spectrum_betav3_device::motors_control()
{
	for (int i = 0; i < 4; i++)
	{
		floppy_image_device* floppy = m_floppy[i]->get_device();
		if (!floppy)
			continue;
		if (m_motor_active && (m_control & 3) == i)
			floppy->mon_w(CLEAR_LINE);
		else
			floppy->mon_w(ASSERT_LINE);
	}
}

// Betaplus has a 'magic button' for dumping RAM

INPUT_PORTS_START(betaplus)
	PORT_START("BUTTON") // don't use F12, it clashes with the 'exit from debugger' button
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Magic Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spectrum_betaplus_device::magic_button), 0)
INPUT_PORTS_END

INPUT_PORTS_START(gamma)
	PORT_INCLUDE(betaplus)

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(spectrum_gamma_device::busy_r))
INPUT_PORTS_END


ioport_constructor spectrum_betaplus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(betaplus);
}

ioport_constructor spectrum_gamma_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gamma);
}

INPUT_CHANGED_MEMBER(spectrum_betaplus_device::magic_button)
{
	if (newval && !oldval)
	{
		m_slot->nmi_w(ASSERT_LINE);
		m_romcs = 1;
		m_control &= ~0x80;
	}
	else
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
}
