/*****************************************************************************
 *
 *   Portable Xerox AltoII CPU core
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2.h"
#include "a2roms.h"

#define	DEBUG_UCODE_CONST_DATA	0	//!< define to 1 to dump decoded micro code and constants

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ALTO2 = &device_creator<alto2_cpu_device>;
//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

DEVICE_ADDRESS_MAP_START( ucode_map, 32, alto2_cpu_device )
	AM_RANGE(0,                          ALTO2_UCODE_RAM_BASE - 1)          AM_READ     ( crom_r )
	AM_RANGE(ALTO2_UCODE_RAM_BASE,       ALTO2_UCODE_SIZE - 1)              AM_READWRITE( cram_r, cram_w )
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( const_map, 16, alto2_cpu_device )
	AM_RANGE(0,                          ALTO2_CONST_SIZE - 1)              AM_READ     ( const_r )
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( iomem_map, 16, alto2_cpu_device )
	AM_RANGE(0,                          ALTO2_IO_PAGE_BASE - 1)            AM_READWRITE( ioram_r, ioram_w )
	// page 0376
	AM_RANGE(0177000,                    0177015)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177016,                    0177016)                           AM_READWRITE( utilout_r, utilout_w )    // UTILOUT register
	AM_RANGE(0177017,                    0177017)                           AM_READWRITE( noop_r, noop_w )          // unused range
	AM_RANGE(0177020,                    0177023)                           AM_READWRITE( xbus_r,  xbus_w )         // XBUS[0-3] registers
	AM_RANGE(0177024,                    0177024)                           AM_READ     ( mear_r )                  // MEAR (memory error address register)
	AM_RANGE(0177025,                    0177025)                           AM_READWRITE( mesr_r,  mesr_w  )        // MESR (memory error status register)
	AM_RANGE(0177026,                    0177026)                           AM_READWRITE( mecr_r,  mecr_w  )        // MECR (memory error control register)
	AM_RANGE(0177027,                    0177027)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177030,                    0177033)                           AM_READ     ( utilin_r )                // UTILIN register
	AM_RANGE(0177034,                    0177037)                           AM_READ     ( kbd_ad_r )                // KBD_AD[0-3] matrix
	AM_RANGE(0177040,                    0177057)                           AM_READWRITE( bank_reg_r, bank_reg_w )  // BANK[0-17] registers (4 bit)
	AM_RANGE(0177060,                    0177077)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177100,                    0177101)                           AM_READWRITE( noop_r, noop_w )          // { Summagraphics tablet X, Y }
	AM_RANGE(0177102,                    0177137)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177140,                    0177157)                           AM_READWRITE( noop_r, noop_w )          // { Organ keyboard }
	AM_RANGE(0177160,                    0177177)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177200,                    0177204)                           AM_READWRITE( noop_r, noop_w )          // { PROM programmer }
	AM_RANGE(0177205,                    0177233)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177234,                    0177237)                           AM_READWRITE( noop_r, noop_w )          // { Experimental cursor control }
	AM_RANGE(0177240,                    0177257)                           AM_READWRITE( noop_r, noop_w )          // { Alto-II debugger }
//  AM_RANGE(0177244,                    0177247)                           AM_READWRITE( noop_r, noop_w )          // { Graphics keyboard }
	AM_RANGE(0177260,                    0177377)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	// page 0377
//	AM_RANGE(0177400,                    0177405)                           AM_READWRITE( noop_r, noop_w )          // { Maxc2 maintenance interface }
	AM_RANGE(0177400,                    0177400)                           AM_READWRITE( noop_r, noop_w )          // { Alto DLS input }
	AM_RANGE(0177401,                    0177417)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177420,                    0177420)                           AM_READWRITE( noop_r, noop_w )          // { "" }
	AM_RANGE(0177421,                    0177437)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177440,                    0177440)                           AM_READWRITE( noop_r, noop_w )          // { "" }
	AM_RANGE(0177441,                    0177457)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177460,                    0177460)                           AM_READWRITE( noop_r, noop_w )          // { "" }
	AM_RANGE(0177461,                    0177577)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177600,                    0177677)                           AM_READWRITE( noop_r, noop_w )          // { Alto DLS output }
	AM_RANGE(0177700,                    0177700)                           AM_READWRITE( noop_r, noop_w )          // { EIA interface output bit }
	AM_RANGE(0177701,                    0177701)                           AM_READWRITE( noop_r, noop_w )          // { EIA interface input bit }
	AM_RANGE(0177702,                    0177717)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177720,                    0177737)                           AM_READWRITE( noop_r, noop_w )          // { TV camera interface }
	AM_RANGE(0177740,                    0177763)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177764,                    0177773)                           AM_READWRITE( noop_r, noop_w )          // { Redactron tape drive }
	AM_RANGE(0177774,                    0177775)                           AM_READWRITE( noop_r, noop_w )          // UNUSED RANGE
	AM_RANGE(0177776,                    0177776)                           AM_READWRITE( noop_r, noop_w )          // { Digital-Analog Converter, Joystick }
	AM_RANGE(0177777,                    0177777)                           AM_READWRITE( noop_r, noop_w )          // { Digital-Analog Converter, Joystick }

	AM_RANGE(0200000,                    0377777)                           AM_READWRITE( ioram_r, ioram_w )
ADDRESS_MAP_END

//-------------------------------------------------
//  alto2_cpu_device - constructor
//-------------------------------------------------

alto2_cpu_device::alto2_cpu_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	cpu_device(mconfig, ALTO2, "Xerox Alto-II", tag, owner, clock, "alto2", __FILE__),
#if	ALTO2_DEBUG
	m_log_types(LOG_DISK|LOG_KSEC|LOG_KWD|LOG_KBD),
	m_log_level(6),
	m_log_newline(true),
#endif
	m_ucode_config("ucode", ENDIANNESS_BIG, 32, 12, -2 ),
	m_const_config("const", ENDIANNESS_BIG, 16,  8, -1 ),
	m_iomem_config("iomem", ENDIANNESS_BIG, 16, 17, -1 ),
	m_ucode(0),
	m_const(0),
	m_iomem(0),
	m_ucode_crom(0),
	m_const_data(0),
	m_icount(0),
	m_task_mpc(),
	m_task_next2(),
	m_ntime(),
	m_task(0),
	m_next_task(0),
	m_next2_task(0),
	m_mpc(0),
	m_mir(0),
	m_rsel(0),
	m_next(0),
	m_next2(0),
	m_r(),
	m_s(),
	m_bus(0),
	m_t(0),
	m_alu(0),
	m_aluc0(0),
	m_l(0),
	m_shifter(0),
	m_laluc0(0),
	m_m(0),
	m_cram_addr(0),
	m_task_wakeup(0),
	m_active_callback(),
	m_reset_mode(0xffff),
	m_rdram_flag(false),
	m_wrtram_flag(false),
	m_s_reg_bank(),
	m_bank_reg(),
	m_ether_enable(true),
	m_ewfct(false),
	m_dsp_time(0),
	m_dsp_state(0),
	m_unload_time(0),
	m_unload_word(0),
	m_bitclk_time(0),
	m_bitclk_index(0),
	m_ctl2k_u3(0),
	m_ctl2k_u38(0),
	m_ctl2k_u76(0),
	m_cram3k_a37(0),
	m_madr_a64(0),
	m_madr_a65(0),
	m_madr_a90(0),
	m_madr_a91(0),
	m_bs(),
	m_f1(),
	m_f2(),
	m_ram_related(),
	m_cycle(0),
	m_hw(),
	m_mouse(),
	m_drive(),
	m_dsk(),
	m_sysclka0(),
	m_sysclka1(),
	m_sysclkb0(),
	m_sysclkb1(),
	m_dsp(),
	m_disp_a38(0),
	m_disp_a63(0),
	m_disp_a66(0),
	m_mem(),
	m_emu(),
	m_ether_a41(0),
	m_ether_a42(0),
	m_ether_a49(0),
	m_eth()
{
	m_is_octal = true;
}

#if	ALTO2_DEBUG
// FIXME: define types (sections) and print the section like [emu] [kwd] ...
// FIXME: use the level to suppress messages if logging is less verbose than level
void alto2_cpu_device::logprintf(int type, int level, const char* format, ...)
{
	static const char* type_name[] = {
		"[CPU]",
		"[EMU]",
		"[T01]",
		"[T02]",
		"[T03]",
		"[KSEC]",
		"[T05]",
		"[T06]",
		"[ETH]",
		"[MRT]",
		"[DWT]",
		"[CURT]",
		"[DHT]",
		"[DVT]",
		"[PART]",
		"[KWD]",
		"[T17]",
		"[MEM]",
		"[RAM]",
		"[DRIVE]",
		"[DISK]",
		"[DISPL]",
		"[MOUSE]",
		"[HW]",
		"[KBD]"
	};
	if (!(m_log_types & type))
		return;
	if (level > m_log_level)
		return;
	if (m_log_newline) {
		// last line had a \n - print type name
		for (int i = 0; i < sizeof(type_name)/sizeof(type_name[0]); i++)
			if (type & (1 << i))
				logerror("%-7s ", type_name[i]);
	}
	va_list ap;
	va_start(ap, format);
	vlogerror(format, ap);
	va_end(ap);
	m_log_newline = format[strlen(format) - 1] == '\n';
}
#endif

//-------------------------------------------------
// driver interface to set diablo_hd_device
//-------------------------------------------------

void alto2_cpu_device::set_diablo(int unit, diablo_hd_device* ptr)
{
	m_drive[unit] = ptr;
}

//-------------------------------------------------
//  device_rom_region - device-specific (P)ROMs
//-------------------------------------------------

ROM_START( alto2_cpu )
	ROM_REGION( 16 * 02000, "ucode_proms", 0 )
	ROM_LOAD( "55x.3",     0*02000, 0x400, CRC(de870d75) SHA1(2b98cc769d8302cb39948711424d987d94e4159b) )	//!< 00000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
	ROM_LOAD( "64x.3",     1*02000, 0x400, CRC(51b444c0) SHA1(8756e51f7f3253a55d75886465beb7ee1be6e1c4) )	//!< 00000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
	ROM_LOAD( "65x.3",     2*02000, 0x400, CRC(741d1437) SHA1(01f7cf07c2173ac93799b2475180bfbbe7e0149b) )	//!< 00000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
	ROM_LOAD( "63x.3",     3*02000, 0x400, CRC(f22d5028) SHA1(c65a42baef702d4aff2d9ad8e363daec27de6801) )	//!< 00000-01777 F1(0),F1(1)',F1(2)',F1(3)'
	ROM_LOAD( "53x.3",     4*02000, 0x400, CRC(3c89a740) SHA1(95d812d489b2bde03884b2f126f961caa6c8ec45) )	//!< 00000-01777 F2(0),F2(1)',F2(2)',F2(3)'
	ROM_LOAD( "60x.3",     5*02000, 0x400, CRC(a35de0bf) SHA1(7fa4aead44dcf5393bbfd1706c0ada24aa6fd3ac) )	//!< 00000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
	ROM_LOAD( "61x.3",     6*02000, 0x400, CRC(f25bcb2d) SHA1(acb57f3104a8dc4ba750dd1bf22ccc81cce9f084) )	//!< 00000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
	ROM_LOAD( "62x.3",     7*02000, 0x400, CRC(1b20a63f) SHA1(41dc86438e91c12b0fe42ffcce6b2ac2eb9e714a) )	//!< 00000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'

	// extended memory Mesa 5.1 micro code PROMs, 8 x 4bit
	ROM_LOAD( "xm51.u54",  8*02000, 02000, CRC(11086ae9) SHA1(c394e3fadbfb91801ddc1a70cb25dc6f606c4f76) )	//!< 00000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
	ROM_LOAD( "xm51.u74",  9*02000, 02000, CRC(be8224f2) SHA1(ea9abcc3832b26a094319796901237e1e3f238b6) )	//!< 00000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
	ROM_LOAD( "xm51.u75", 10*02000, 02000, CRC(dfe3e3ac) SHA1(246fd29f92150a5d5d7627fbb4f2504c7b6cd5ec) )	//!< 00000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
	ROM_LOAD( "xm51.u73", 11*02000, 02000, CRC(6c20fa46) SHA1(a054330c65048011f12209aaed5c6da73d95f029) )	//!< 00000-01777 F1(0),F1(1)',F1(2)',F1(3)'
	ROM_LOAD( "xm51.u52", 12*02000, 02000, CRC(0a31eec8) SHA1(4e2ad5daa5e6a6f2143ee4de00c7b625d096fb02) )	//!< 00000-01777 F2(0),F2(1)',F2(2)',F2(3)'
	ROM_LOAD( "xm51.u70", 13*02000, 02000, CRC(5c64ee54) SHA1(0eb16d1b5e5967be7c1bf8c8ef6efdf0518a752c) )	//!< 00000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
	ROM_LOAD( "xm51.u71", 14*02000, 02000, CRC(7283bf71) SHA1(819fdcc407ed0acdd8f12b02db6efbcab7bec19a) )	//!< 00000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
	ROM_LOAD( "xm51.u72", 15*02000, 02000, CRC(a28e5251) SHA1(44dd8ad4ad56541b5394d30ce3521b4d1d561394) )	//!< 00000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'

	// constant PROMs, 4 x 4bit
	// UINT16 src = BITS(addr, 3,2,1,4,5,6,7,0);
	ROM_REGION( 4 * 0400, "const_proms", 0 )
	ROM_LOAD( "madr.a6",   0*00400, 00400, CRC(c2c196b2) SHA1(8b2a599ac839ec2a070dbfef2f1626e645c858ca) )	//!< 0000-0377 C(00)',C(01)',C(02)',C(03)'
	ROM_LOAD( "madr.a5",   1*00400, 00400, CRC(42336101) SHA1(c77819cf40f063af3abf66ea43f17cc1a62e928b) )	//!< 0000-0377 C(04)',C(05)',C(06)',C(07)'
	ROM_LOAD( "madr.a4",   2*00400, 00400, CRC(b957e490) SHA1(c72660ad3ada4ca0ed8697c6bb6275a4fe703184) )	//!< 0000-0377 C(08)',C(09)',C(10)',C(11)'
	ROM_LOAD( "madr.a3",   3*00400, 00400, CRC(e0992757) SHA1(5c45ea824970663cb9ee672dc50861539c860249) )	//!< 0000-0377 C(12)',C(13)',C(14)',C(15)'

	// extended memory Mesa 4.1 (?) micro code PROMs, 8 x 4bit (unused)
	ROM_REGION( 8 * 02000, "xm_mesa_4.1", 0 )
	ROM_LOAD( "xm654.41",  0*02000, 02000, CRC(beace302) SHA1(0002fea03a0261f57365095c4b87385d833f7063) )	//!< 00000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
	ROM_LOAD( "xm674.41",  1*02000, 02000, CRC(7db5c097) SHA1(364bc41951baa3ad274031bd49abec1cf5b7a980) )	//!< 00000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
	ROM_LOAD( "xm675.41",  2*02000, 02000, CRC(26eac1e7) SHA1(9220a1386afae8de96bdb2cf084afbadeeb61d42) )	//!< 00000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
	ROM_LOAD( "xm673.41",  3*02000, 02000, CRC(8173d7e3) SHA1(7fbacf6dccb60dfe9cef88a248c3a1660efddcf4) )	//!< 00000-01777 F1(0),F1(1)',F1(2)',F1(3)'
	ROM_LOAD( "xm652.41",  4*02000, 02000, CRC(ddfa94bb) SHA1(38625e269400aaf38cd07b5dbf36c0087a0f1b92) )	//!< 00000-01777 F2(0),F2(1)',F2(2)',F2(3)'
	ROM_LOAD( "xm670.41",  5*02000, 02000, CRC(1cd187f3) SHA1(0fd5eff7c6b5c2383aa20148a795b80286554675) )	//!< 00000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
	ROM_LOAD( "xm671.41",  6*02000, 02000, CRC(f21b1ad7) SHA1(1e18bdb35de7802892ac373c128f900786d40886) )	//!< 00000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
	ROM_LOAD( "xm672.41",  7*02000, 02000, CRC(110ee075) SHA1(bb72fceba5ce9e5e8c8a0024915006bdd011a3f3) )	//!< 00000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'

	ROM_REGION( 0400, "2kctl_u3", 0 )
	ROM_LOAD( "2kctl.u3",   00000, 00400, CRC(5f8d89e8) SHA1(487cd944ab074290aea73425e81ef4900d92e250) )	//!< 3601-1 256x4 BPROM; Emulator address modifier

	ROM_REGION( 0400, "2kctl_u38", 0 )
	ROM_LOAD( "2kctl.u38",  00000, 00040, CRC(fc51b1d1) SHA1(e36c2a12a5da377394264899b5ae504e2ffda46e) )	//!< 82S23 32x8 BPROM; task priority and initial address

	ROM_REGION( 0400, "2kctl_u76", 0 )
	ROM_LOAD( "2kctl.u76",  00000, 00400, CRC(1edef867) SHA1(928b8a15ac515a99109f32672441832173883b81) )	//!< 3601-1 256x4 BPROM; 2KCTL replacement for u51 (1KCTL)

	ROM_REGION( 0040, "alu_a10", 0 )
	ROM_LOAD( "alu.a10",    00000, 00040, CRC(e0857892) SHA1(dcd389767139f0acc1f87cf074459115abc5b90b) )

	ROM_REGION( 0400, "3kcram_a37", 0 )
	ROM_LOAD( "3kcram.a37", 00000, 00400, CRC(9417360d) SHA1(bfcdbc56ee4ffafd0f2f672c0c869a55d6dd194b) )

	ROM_REGION( 0400, "madr_a32", 0 )
	ROM_LOAD( "madr.a32",   00000, 00400, CRC(a0e3b4a7) SHA1(24e50afdeb637a6a8588f8d3a3493c9188b8da2c) )	//! P3601 256x4 BPROM; mouse motion signals MX1, MX2, MY1, MY2

	ROM_REGION( 0400, "madr_a64", 0 )
	ROM_LOAD( "madr.a64",   00000, 00400, CRC(a66b0eda) SHA1(4d9088f592caa3299e90966b17765be74e523144) )	//! P3601 256x4 BPROM; memory addressing

	ROM_REGION( 0400, "madr_a65", 0 )
	ROM_LOAD( "madr.a65",   00000, 00400, CRC(ba37febd) SHA1(82e9db1cb65f451755295f0d179e6f8fe3349d4d) )	//! P3601 256x4 BPROM; memory addressing

	ROM_REGION( 0400, "madr_a90", 0 )
	ROM_LOAD( "madr.a90",   00000, 00400, CRC(7a2d8799) SHA1(c3760dba147740729d33b9b88e59088a4cc7437a) )

	ROM_REGION( 0400, "madr_a91", 0 )
	ROM_LOAD( "madr.a91",   00000, 00400, CRC(dd556aeb) SHA1(900f333a091e3ccde0843019c25f25fba62e6023) )

	ROM_REGION( 0400, "displ_a38", 0 )
	ROM_LOAD( "displ.a38",  00000, 00400, CRC(fd30beb7) SHA1(65e4a19ba4ff748d525122128c514abedd55d866) )	//!< P3601 256x4 BPROM; display FIFO control: STOPWAKE, MBEMPTY

	ROM_REGION( 0040, "displ_a63", 0 )
	ROM_LOAD( "displ.a63",  00000, 00040, CRC(82a20d60) SHA1(39d90703568be5419ada950e112d99227873fdea) )	//!< 82S23 32x8 BPROM; display HBLANK, HSYNC, SCANEND, HLCGATE ...

	ROM_REGION( 0400, "displ_a66", 0 )
	ROM_LOAD( "displ.a66",  00000, 00400, CRC(9f91aad9) SHA1(69b1d4c71f4e18103112e8601850c2654e9265cf) )	//!< P3601 256x4 BPROM; display VSYNC and VBLANK

	ROM_REGION( 0400, "ether_a41", 0 )
	ROM_LOAD( "enet.a41",   00000, 00400, CRC(d5de8d86) SHA1(c134a4c898c73863124361a9b0218f7a7f00082a) )

	ROM_REGION( 0400, "ether_a42", 0 )
	ROM_LOAD( "enet.a42",   00000, 00400, CRC(9d5c81bd) SHA1(ac7e63332a3dad0bef7cd0349b24e156a96a4bf0) )

	ROM_REGION( 0400, "ether_a49", 0 )
	ROM_LOAD( "enet.a49",   00000, 00400, CRC(4d2dcdb2) SHA1(583327a7d70cd02702c941c0e43c1e9408ff7fd0) )
ROM_END

const rom_entry *alto2_cpu_device::device_rom_region() const
{
	return ROM_NAME( alto2_cpu );
}

//-------------------------------------------------
// device_memory_interface overrides
//-------------------------------------------------

const address_space_config*alto2_cpu_device::memory_space_config(address_spacenum spacenum) const
{
	if (AS_0 == spacenum)
		return &m_ucode_config;
	if (AS_1 == spacenum)
		return &m_const_config;
	if (AS_2 == spacenum)
		return &m_iomem_config;
	return NULL;
}

/**
 * @brief list of microcode PROM loading options
 */
static const prom_load_t pl_ucode[] = {
	{	// 0000-01777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
		"55x.3",
		0,
		"de870d75",
		"2b98cc769d8302cb39948711424d987d94e4159b",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	28,
/* dmap */	DMAP_DEFAULT,
/* dand */	ZERO,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
		"64x.3",
		0,
		"51b444c0",
		"8756e51f7f3253a55d75886465beb7ee1be6e1c4",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	24,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 ALUF(3)',BS(0)',BS(1)',BS(2)'
		"65x.3",
		0,
		"741d1437",
		"01f7cf07c2173ac93799b2475180bfbbe7e0149b",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	20,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 F1(0),F1(1)',F1(2)',F1(3)'
		"63x.3",
		0,
		"f22d5028",
		"c65a42baef702d4aff2d9ad8e363daec27de6801",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	16,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 F2(0),F2(1)',F2(2)',F2(3)'
		"53x.3",
		0,
		"3c89a740",
		"95d812d489b2bde03884b2f126f961caa6c8ec45",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	12,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 LOADT',LOADL,NEXT(0)',NEXT(1)'
		"60x.3",
		0,
		"a35de0bf",
		"7fa4aead44dcf5393bbfd1706c0ada24aa6fd3ac",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	013,						// invert D0 and D2-D3
/* width */	4,
/* shift */	8,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
		"61x.3",
		0,
		"f25bcb2d",
		"acb57f3104a8dc4ba750dd1bf22ccc81cce9f084",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	4,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 0000-01777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'
		"62x.3",
		0,
		"1b20a63f",
		"41dc86438e91c12b0fe42ffcce6b2ac2eb9e714a",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	0,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	}

#if	(ALTO2_UCODE_ROM_PAGES > 1)
	,
	{	// 02000-03777 RSEL(0)',RSEL(1)',RSEL(2)',RSEL(3)'
		"xm51.u54",
		0,
		"11086ae9",
		"c394e3fadbfb91801ddc1a70cb25dc6f606c4f76",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	28,
/* dmap */	DMAP_DEFAULT,
/* dand */	ZERO,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 RSEL(4)',ALUF(0)',ALUF(1)',ALUF(2)'
		"xm51.u74",
		0,
		"be8224f2",
		"ea9abcc3832b26a094319796901237e1e3f238b6",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	24,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 ALUF(3)',BS(0)',BS(1)',BS(2)'
		"xm51.u75",
		0,
		"dfe3e3ac",
		"246fd29f92150a5d5d7627fbb4f2504c7b6cd5ec",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	20,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 F1(0),F1(1)',F1(2)',F1(3)'
		"xm51.u73",
		0,
		"6c20fa46",
		"a054330c65048011f12209aaed5c6da73d95f029",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	16,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 F2(0),F2(1)',F2(2)',F2(3)'
		"xm51.u52",
		0,
		"0a31eec8",
		"4e2ad5daa5e6a6f2143ee4de00c7b625d096fb02",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	007,						// keep D0, invert D1-D3
/* width */	4,
/* shift */	12,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 LOADT',LOADL,NEXT(0)',NEXT(1)'
		"xm51.u70",
		0,
		"5c64ee54",
		"0eb16d1b5e5967be7c1bf8c8ef6efdf0518a752c",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	013,						// invert D0 and D2-D3
/* width */	4,
/* shift */	8,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 NEXT(2)',NEXT(3)',NEXT(4)',NEXT(5)'
		"xm51.u71",
		0,
		"7283bf71",
		"819fdcc407ed0acdd8f12b02db6efbcab7bec19a",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	4,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	},
	{	// 02000-03777 NEXT(6)',NEXT(7)',NEXT(8)',NEXT(9)'
		"xm51.u72",
		0,
		"a28e5251",
		"44dd8ad4ad56541b5394d30ce3521b4d1d561394",
/* size */	ALTO2_UCODE_PAGE_SIZE,
/* amap */	AMAP_DEFAULT,
/* axor */	ALTO2_UCODE_PAGE_MASK,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	0,
/* dmap */	DMAP_DEFAULT,
/* dand */	KEEP,
/* type */	sizeof(UINT32)
	}
#endif	// (UCODE_ROM_PAGES > 1)
};

/**
 * @brief list of constant PROM loading options
 */
static const prom_load_t pl_const[] = {
	{	// constant prom D0-D3
		"madr.a6",
		"c3.3",
		"c2c196b2",
		"8b2a599ac839ec2a070dbfef2f1626e645c858ca",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	0,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	ZERO,
/* type */	sizeof(UINT16)
	},
	{	// constant prom D4-D7
		"madr.a5",
		"c2.3",
		"42336101",
		"c77819cf40f063af3abf66ea43f17cc1a62e928b",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	4,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	KEEP,
/* type */	sizeof(UINT16)
	},
	{	// constant prom D8-D11
		"madr.a4",
		"c1.3",
		"b957e490",
		"c72660ad3ada4ca0ed8697c6bb6275a4fe703184",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	8,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	KEEP,
/* type */	sizeof(UINT16)
	},
	{	// constant PROM D12-D15
		"madr.a3",
		"c0.3",
		"e0992757",
		"5c45ea824970663cb9ee672dc50861539c860249",
/* size */	ALTO2_CONST_SIZE,
/* amap */	AMAP_CONST_PROM,			// descramble constant address
/* axor */	0,
/* dxor */	017,						// invert D0-D3
/* width */	4,
/* shift */	12,
/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
/* dand */	KEEP,
/* type */	sizeof(UINT16)
	}
};

//! 82S23 32x8 BPROM; display HBLANK, HSYNC, SCANEND, HLCGATE ...
static const prom_load_t pl_displ_a63 =
{
	"displ.a63",
	0,
	"82a20d60",
	"39d90703568be5419ada950e112d99227873fdea",
	/* size */	0040,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	8,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

//! P3601 256x4 BPROM; display FIFO control: STOPWAKE, MBEMPTY
static const prom_load_t pl_displ_a38 =
{
	"displ.a38",
	0,
	"fd30beb7",
	"65e4a19ba4ff748d525122128c514abedd55d866",
	/* size */	0400,
	/* amap */	AMAP_REVERSE_0_7,			// reverse address lines A0-A7
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

//! P3601 256x4 BPROM; display VSYNC and VBLANK
static const prom_load_t pl_displ_a66 =
{
	"displ.a66",
	0,
	"9f91aad9",
	"69b1d4c71f4e18103112e8601850c2654e9265cf",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

//! 3601-1 256x4 BPROM; Emulator address modifier
static const prom_load_t pl_2kctl_u3 =
{
	"2kctl.u3",
	0,
	"5f8d89e8",
	"487cd944ab074290aea73425e81ef4900d92e250",
	/* size */	0400,
	/* amap */	AMAP_REVERSE_0_7,			// reverse address lines A0-A7
	/* axor */	0377,						// invert address lines A0-A7
	/* dxor */	017,						// invert data lines D0-D3
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

//! 82S23 32x8 BPROM; task priority and initial address
static const prom_load_t pl_2kctl_u38 =
{
	"2kctl.u38",
	0,
	"fc51b1d1",
	"e36c2a12a5da377394264899b5ae504e2ffda46e",
	/* size */	0040,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	8,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

//! 3601-1 256x4 BPROM; 2KCTL replacement for u51 (1KCTL)
static const prom_load_t pl_2kctl_u76 =
{
	"2kctl.u76",
	0,
	"1edef867",
	"928b8a15ac515a99109f32672441832173883b81",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0077,						// invert address lines A0-A5
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

//! ALUF to ALU 741818 functions and carry in mapper
static const prom_load_t pl_alu_a10 =
{
	"alu.a10",
	0,
	"e0857892",
	"dcd389767139f0acc1f87cf074459115abc5b90b",
	/* size */	0040,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_3kcram_a37 =
{
	"3kcram.a37",
	0,
	"9417360d",
	"bfcdbc56ee4ffafd0f2f672c0c869a55d6dd194b",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	017,						// invert D0-D3
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_madr_a32 =
{
	"madr.a32",
	0,
	"a0e3b4a7",
	"24e50afdeb637a6a8588f8d3a3493c9188b8da2c",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	017,						// invert D0-D3
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_REVERSE_0_3,			// reverse D0-D3 to D3-D0
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_madr_a64 =
{
	"madr.a64",
	0,
	"a66b0eda",
	"4d9088f592caa3299e90966b17765be74e523144",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	017,						// invert D0-D3
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_madr_a65 =
{
	"madr.a65",
	0,
	"ba37febd",
	"82e9db1cb65f451755295f0d179e6f8fe3349d4d",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	017,						// invert D0-D3
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_madr_a90 =
{
	"madr.a90",
	0,
	"7a2d8799",
	"c3760dba147740729d33b9b88e59088a4cc7437a",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	017,						// invert D0-D3
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_madr_a91 =
{
	"madr.a91",
	0,
	"dd556aeb",
	"900f333a091e3ccde0843019c25f25fba62e6023",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	017,						// invert D0-D3
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_enet_a41 =
{	/* P3601 256x4 BPROM; Ethernet phase encoder 1 "PE1" */
	"enet.a41",
	0,
	"d5de8d86",
	"c134a4c898c73863124361a9b0218f7a7f00082a",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_enet_a42 =
{	/* P3601 256x4 BPROM; Ethernet phase encoder 2 "PE2" */
	"enet.a42",
	0,
	"9d5c81bd",
	"ac7e63332a3dad0bef7cd0349b24e156a96a4bf0",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_enet_a49 =
{	/* P3601 256x4 BPROM; Ethernet FIFO control "AFIFO" */
	"enet.a49",
	0,
	"4d2dcdb2",
	"583327a7d70cd02702c941c0e43c1e9408ff7fd0",
	/* size */	0400,
	/* amap */	AMAP_REVERSE_0_7,				// reverse address lines A0-A7
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

// FIXME
void alto2_cpu_device::device_start()
{
	m_ucode = &space(AS_0);
	m_const = &space(AS_1);
	m_iomem = &space(AS_2);

	// decode micro code PROMs to CROM
	m_ucode_crom = prom_load(pl_ucode, memregion("ucode_proms")->base(), ALTO2_UCODE_ROM_PAGES, 8);

	// allocate micro code CRAM
	m_ucode_cram = global_alloc_array(UINT8, sizeof(UINT32) * ALTO2_UCODE_RAM_PAGES * ALTO2_UCODE_PAGE_SIZE);
	// fill with inverted bits value
	for (offs_t offset = 0; offset < ALTO2_UCODE_RAM_PAGES * ALTO2_UCODE_PAGE_SIZE; offset++)
		*reinterpret_cast<UINT32 *>(m_ucode_cram + offset * 4) = ALTO2_UCODE_INVERTED;

	// decode constant PROMs to const data
	m_const_data = prom_load(pl_const, memregion("const_proms")->base(), 1, 4);

	m_disp_a38 = prom_load(&pl_displ_a38, memregion("displ_a38")->base());
	m_disp_a63 = prom_load(&pl_displ_a63, memregion("displ_a63")->base());
	m_disp_a66 = prom_load(&pl_displ_a66, memregion("displ_a66")->base());
	m_ctl2k_u3 = prom_load(&pl_2kctl_u3, memregion("2kctl_u3")->base());
	m_ctl2k_u38 = prom_load(&pl_2kctl_u38, memregion("2kctl_u38")->base());
	m_ctl2k_u76 = prom_load(&pl_2kctl_u76, memregion("2kctl_u76")->base());
	m_alu_a10 = prom_load(&pl_alu_a10, memregion("alu_a10")->base());
	m_cram3k_a37 = prom_load(&pl_3kcram_a37, memregion("3kcram_a37")->base());
	m_madr_a32 = prom_load(&pl_madr_a32, memregion("madr_a32")->base());
	m_madr_a64 = prom_load(&pl_madr_a64, memregion("madr_a64")->base());
	m_madr_a65 = prom_load(&pl_madr_a65, memregion("madr_a65")->base());
	m_madr_a90 = prom_load(&pl_madr_a90, memregion("madr_a90")->base());
	m_madr_a91 = prom_load(&pl_madr_a91, memregion("madr_a91")->base());
	m_ether_a41 = prom_load(&pl_enet_a41, memregion("ether_a41")->base());
	m_ether_a42 = prom_load(&pl_enet_a42, memregion("ether_a42")->base());
	m_ether_a49 = prom_load(&pl_enet_a49, memregion("ether_a49")->base());

	save_item(NAME(m_task_mpc));
	save_item(NAME(m_task_next2));
	save_item(NAME(m_ntime));
	save_item(NAME(m_task));
	save_item(NAME(m_next_task));
	save_item(NAME(m_next2_task));
	save_item(NAME(m_mpc));
	save_item(NAME(m_mir));
	save_item(NAME(m_rsel));
	save_item(NAME(m_next));
	save_item(NAME(m_next2));
	save_item(NAME(m_r));
	save_item(NAME(m_s));
	save_item(NAME(m_bus));
	save_item(NAME(m_t));
	save_item(NAME(m_alu));
	save_item(NAME(m_aluc0));
	save_item(NAME(m_l));
	save_item(NAME(m_shifter));
	save_item(NAME(m_laluc0));
	save_item(NAME(m_m));
	save_item(NAME(m_cram_addr));
	save_item(NAME(m_task_wakeup));
	save_item(NAME(m_reset_mode));
	save_item(NAME(m_rdram_flag));
	save_item(NAME(m_wrtram_flag));
	save_item(NAME(m_s_reg_bank));
	save_item(NAME(m_bank_reg));
	save_item(NAME(m_ether_enable));
	save_item(NAME(m_ewfct));
	save_item(NAME(m_dsp_time));
	save_item(NAME(m_dsp_state));
	save_item(NAME(m_unload_time));
	save_item(NAME(m_unload_word));
#if	(USE_BITCLK_TIMER == 0)
	save_item(NAME(m_bitclk_time));
	save_item(NAME(m_bitclk_index));
#endif
	save_item(NAME(m_mouse.x));
	save_item(NAME(m_mouse.y));
	save_item(NAME(m_mouse.dx));
	save_item(NAME(m_mouse.dy));
	save_item(NAME(m_mouse.latch));
	save_item(NAME(m_dsk.drive));
	save_item(NAME(m_dsk.kaddr));
	save_item(NAME(m_dsk.kadr));
	save_item(NAME(m_dsk.kstat));
	save_item(NAME(m_dsk.kcom));
	save_item(NAME(m_dsk.krecno));
	save_item(NAME(m_dsk.shiftin));
	save_item(NAME(m_dsk.shiftout));
	save_item(NAME(m_dsk.datain));
	save_item(NAME(m_dsk.dataout));
	save_item(NAME(m_dsk.krwc));
	save_item(NAME(m_dsk.kfer));
	save_item(NAME(m_dsk.wdtskena));
	save_item(NAME(m_dsk.wdinit0));
	save_item(NAME(m_dsk.wdinit));
	save_item(NAME(m_dsk.strobe));
	save_item(NAME(m_dsk.bitclk));
	save_item(NAME(m_dsk.datin));
	save_item(NAME(m_dsk.bitcount));
	save_item(NAME(m_dsk.carry));
	save_item(NAME(m_dsk.seclate));
	save_item(NAME(m_dsk.seekok));
	save_item(NAME(m_dsk.ok_to_run));
	save_item(NAME(m_dsk.ready_mf31a));
	save_item(NAME(m_dsk.seclate_mf31b));
#if	0
	save_item(NAME(m_dsk.ff_21a));
	save_item(NAME(m_dsk.ff_21a_old));
	save_item(NAME(m_dsk.ff_21b));
	save_item(NAME(m_dsk.ff_22a));
	save_item(NAME(m_dsk.ff_22b));
	save_item(NAME(m_dsk.ff_43b));
	save_item(NAME(m_dsk.ff_53a));
	save_item(NAME(m_dsk.ff_43a));
	save_item(NAME(m_dsk.ff_53b));
	save_item(NAME(m_dsk.ff_44a));
	save_item(NAME(m_dsk.ff_44b));
	save_item(NAME(m_dsk.ff_45a));
	save_item(NAME(m_dsk.ff_45b));
#endif
	save_item(NAME(m_dsp.hlc));
	save_item(NAME(m_dsp.a63));
	save_item(NAME(m_dsp.a66));
	save_item(NAME(m_dsp.setmode));
	save_item(NAME(m_dsp.inverse));
	save_item(NAME(m_dsp.halfclock));
	save_item(NAME(m_dsp.clr));
	save_item(NAME(m_dsp.fifo));
	save_item(NAME(m_dsp.fifo_wr));
	save_item(NAME(m_dsp.fifo_rd));
	save_item(NAME(m_dsp.dht_blocks));
	save_item(NAME(m_dsp.dwt_blocks));
	save_item(NAME(m_dsp.curt_blocks));
	save_item(NAME(m_dsp.curt_wakeup));
	save_item(NAME(m_dsp.vblank));
	save_item(NAME(m_dsp.xpreg));
	save_item(NAME(m_dsp.csr));
	save_item(NAME(m_dsp.curword));
	save_item(NAME(m_dsp.curdata));
	save_item(NAME(m_mem.mar));
	save_item(NAME(m_mem.rmdd));
	save_item(NAME(m_mem.wmdd));
	save_item(NAME(m_mem.md));
	save_item(NAME(m_mem.cycle));
	save_item(NAME(m_mem.access));
	save_item(NAME(m_mem.error));
	save_item(NAME(m_mem.mear));
	save_item(NAME(m_mem.mecr));
	save_item(NAME(m_emu.ir));
	save_item(NAME(m_emu.skip));
	save_item(NAME(m_emu.cy));
	save_item(NAME(m_eth.fifo));
	save_item(NAME(m_eth.fifo_rd));
	save_item(NAME(m_eth.fifo_wr));
	save_item(NAME(m_eth.status));
	save_item(NAME(m_eth.rx_crc));
	save_item(NAME(m_eth.tx_crc));
	save_item(NAME(m_eth.rx_count));
	save_item(NAME(m_eth.tx_count));
	save_item(NAME(m_eth.duckbreath));

	state_add( A2_DRIVE,   "DRIVE",   m_dsk.drive).formatstr("%1u");
	state_add( A2_KADDR,   "KADDR",   m_dsk.kaddr).formatstr("%06O");
	state_add( A2_KADR,    "KADR",    m_dsk.kadr).formatstr("%06O");
	state_add( A2_KSTAT,   "KSTAT",   m_dsk.kstat).formatstr("%06O");
	state_add( A2_KCOM,    "KCOM",    m_dsk.kcom).formatstr("%06O");
	state_add( A2_KRECNO,  "KRECNO",  m_dsk.krecno).formatstr("%02O");
	state_add( A2_SHIFTIN, "SHIFTIN", m_dsk.shiftin).formatstr("%07O");
	state_add( A2_SHIFTOUT,"SHIFTOUT",m_dsk.shiftout).formatstr("%07O");
	state_add( A2_DATAIN,  "DATAIN",  m_dsk.datain).formatstr("%06O");
	state_add( A2_DATAOUT, "DATAOUT", m_dsk.dataout).formatstr("%06O");
	state_add( A2_KRWC,    "KRWC",    m_dsk.krwc).formatstr("%1u");
	state_add( A2_KFER,    "KFER",    m_dsk.kfer).formatstr("%1u");
	state_add( A2_WDTSKENA,"WDTSKENA",m_dsk.wdtskena).formatstr("%1u");
	state_add( A2_WDINIT0, "WDINIT0", m_dsk.wdinit0).formatstr("%1u");
	state_add( A2_WDINIT,  "WDINIT",  m_dsk.wdinit).formatstr("%1u");
	state_add( A2_STROBE,  "STROBE",  m_dsk.strobe).formatstr("%1u");
	state_add( A2_BITCLK,  "BITCLK",  m_dsk.bitclk).formatstr("%1u");
	state_add( A2_DATIN,   "DATIN",   m_dsk.datin).formatstr("%06O");
	state_add( A2_BITCNT,  "BITCNT",  m_dsk.bitcount).formatstr("%02O");
	state_add( A2_CARRY,   "CARRY",   m_dsk.carry).formatstr("%1u");
	state_add( A2_SECLATE, "SECLATE", m_dsk.seclate).formatstr("%1u");
	state_add( A2_SEEKOK,  "SEEKOK",  m_dsk.seekok).formatstr("%1u");
	state_add( A2_OKTORUN, "OKTORUN", m_dsk.ok_to_run).formatstr("%1u");
	state_add( A2_READY,   "READY",   m_dsk.kstat).formatstr("%1u");
	state_add_divider(-1);
	state_add( A2_TASK,    "TASK",    m_task).formatstr("%03O");
	state_add( A2_MPC,     "MPC",     m_mpc).formatstr("%06O");
	state_add( A2_NEXT,    "NEXT",    m_next).formatstr("%06O");
	state_add( A2_NEXT2,   "NEXT2",   m_next2).formatstr("%06O");
	state_add( A2_BUS,     "BUS",     m_bus).formatstr("%06O");
	state_add( A2_T,       "T",       m_t).formatstr("%06O");
	state_add( A2_ALU,     "ALU",     m_alu).formatstr("%06O");
	state_add( A2_ALUC0,   "ALUC0",   m_aluc0).formatstr("%1u");
	state_add( A2_L,       "L",       m_l).formatstr("%06O");
	state_add( A2_SHIFTER, "SHIFTER", m_shifter).formatstr("%06O");
	state_add( A2_LALUC0,  "LALUC0",  m_laluc0).formatstr("%1u");
	state_add( A2_M,       "M",       m_m).formatstr("%06O");
	state_add_divider(-1);
	state_add( A2_AC3,     "AC(3)",   m_r[000]).formatstr("%06O");
	state_add( A2_AC2,     "AC(2)",   m_r[001]).formatstr("%06O");
	state_add( A2_AC1,     "AC(1)",   m_r[002]).formatstr("%06O");
	state_add( A2_AC0,     "AC(0)",   m_r[003]).formatstr("%06O");
	state_add( A2_R04,     "R04",     m_r[004]).formatstr("%06O");
	state_add( A2_R05,     "R05",     m_r[005]).formatstr("%06O");
	state_add( A2_PC,      "PC",      m_r[006]).formatstr("%06O");
	state_add( A2_R07,     "R07",     m_r[007]).formatstr("%06O");
	state_add( A2_R10,     "R10",     m_r[010]).formatstr("%06O");
	state_add( A2_R11,     "R11",     m_r[011]).formatstr("%06O");
	state_add( A2_R12,     "R12",     m_r[012]).formatstr("%06O");
	state_add( A2_R13,     "R13",     m_r[013]).formatstr("%06O");
	state_add( A2_R14,     "R14",     m_r[014]).formatstr("%06O");
	state_add( A2_R15,     "R15",     m_r[015]).formatstr("%06O");
	state_add( A2_R16,     "R16",     m_r[016]).formatstr("%06O");
	state_add( A2_R17,     "R17",     m_r[017]).formatstr("%06O");
	state_add( A2_R20,     "R20",     m_r[020]).formatstr("%06O");
	state_add( A2_R21,     "R21",     m_r[021]).formatstr("%06O");
	state_add( A2_R22,     "R22",     m_r[022]).formatstr("%06O");
	state_add( A2_R23,     "R23",     m_r[023]).formatstr("%06O");
	state_add( A2_R24,     "R24",     m_r[024]).formatstr("%06O");
	state_add( A2_R25,     "R25",     m_r[025]).formatstr("%06O");
	state_add( A2_R26,     "R26",     m_r[026]).formatstr("%06O");
	state_add( A2_R27,     "R27",     m_r[027]).formatstr("%06O");
	state_add( A2_R30,     "R30",     m_r[030]).formatstr("%06O");
	state_add( A2_R31,     "R31",     m_r[031]).formatstr("%06O");
	state_add( A2_R32,     "R32",     m_r[032]).formatstr("%06O");
	state_add( A2_R33,     "R33",     m_r[033]).formatstr("%06O");
	state_add( A2_R34,     "R34",     m_r[034]).formatstr("%06O");
	state_add( A2_R35,     "R35",     m_r[035]).formatstr("%06O");
	state_add( A2_R36,     "R36",     m_r[036]).formatstr("%06O");
	state_add( A2_R37,     "R37",     m_r[037]).formatstr("%06O");
	state_add_divider(-1);
	state_add( A2_S00,     "S00",     m_s[0][000]).formatstr("%06O");
	state_add( A2_S01,     "S01",     m_s[0][001]).formatstr("%06O");
	state_add( A2_S02,     "S02",     m_s[0][002]).formatstr("%06O");
	state_add( A2_S03,     "S03",     m_s[0][003]).formatstr("%06O");
	state_add( A2_S04,     "S04",     m_s[0][004]).formatstr("%06O");
	state_add( A2_S05,     "S05",     m_s[0][005]).formatstr("%06O");
	state_add( A2_S06,     "S06",     m_s[0][006]).formatstr("%06O");
	state_add( A2_S07,     "S07",     m_s[0][007]).formatstr("%06O");
	state_add( A2_S10,     "S10",     m_s[0][010]).formatstr("%06O");
	state_add( A2_S11,     "S11",     m_s[0][011]).formatstr("%06O");
	state_add( A2_S12,     "S12",     m_s[0][012]).formatstr("%06O");
	state_add( A2_S13,     "S13",     m_s[0][013]).formatstr("%06O");
	state_add( A2_S14,     "S14",     m_s[0][014]).formatstr("%06O");
	state_add( A2_S15,     "S15",     m_s[0][015]).formatstr("%06O");
	state_add( A2_S16,     "S16",     m_s[0][016]).formatstr("%06O");
	state_add( A2_S17,     "S17",     m_s[0][017]).formatstr("%06O");
	state_add( A2_S20,     "S20",     m_s[0][020]).formatstr("%06O");
	state_add( A2_S21,     "S21",     m_s[0][021]).formatstr("%06O");
	state_add( A2_S22,     "S22",     m_s[0][022]).formatstr("%06O");
	state_add( A2_S23,     "S23",     m_s[0][023]).formatstr("%06O");
	state_add( A2_S24,     "S24",     m_s[0][024]).formatstr("%06O");
	state_add( A2_S25,     "S25",     m_s[0][025]).formatstr("%06O");
	state_add( A2_S26,     "S26",     m_s[0][026]).formatstr("%06O");
	state_add( A2_S27,     "S27",     m_s[0][027]).formatstr("%06O");
	state_add( A2_S30,     "S30",     m_s[0][030]).formatstr("%06O");
	state_add( A2_S31,     "S31",     m_s[0][031]).formatstr("%06O");
	state_add( A2_S32,     "S32",     m_s[0][032]).formatstr("%06O");
	state_add( A2_S33,     "S33",     m_s[0][033]).formatstr("%06O");
	state_add( A2_S34,     "S34",     m_s[0][034]).formatstr("%06O");
	state_add( A2_S35,     "S35",     m_s[0][035]).formatstr("%06O");
	state_add( A2_S36,     "S36",     m_s[0][036]).formatstr("%06O");
	state_add( A2_S37,     "S37",     m_s[0][037]).formatstr("%06O");

	state_add(STATE_GENPC, "curpc", m_mpc).formatstr("%03X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_aluc0).formatstr("%5s").noshow();

	m_icountptr = &m_icount;

	hard_reset();
}

//! read microcode CROM
READ32_MEMBER ( alto2_cpu_device::crom_r )
{
	return *reinterpret_cast<UINT32 *>(m_ucode_crom + offset * 4);
}

//! read microcode CRAM
READ32_MEMBER ( alto2_cpu_device::cram_r )
{
	return *reinterpret_cast<UINT32 *>(m_ucode_cram + offset * 4);
}

//! write microcode CRAM
WRITE32_MEMBER( alto2_cpu_device::cram_w )
{
	*reinterpret_cast<UINT32 *>(m_ucode_cram + offset * 4) = data;
}

//! read constants PROM
READ16_MEMBER ( alto2_cpu_device::const_r )
{
	return *reinterpret_cast<UINT16 *>(m_const_data + offset * 2);
}

#define	PUT_EVEN(dword,word)			A2_PUT32(dword,32, 0,15,word)
#define	GET_EVEN(dword)					A2_GET32(dword,32, 0,15)
#define	PUT_ODD(dword,word)				A2_PUT32(dword,32,16,31,word)
#define	GET_ODD(dword)					A2_GET32(dword,32,16,31)

//! read i/o space RAM
READ16_MEMBER ( alto2_cpu_device::ioram_r )
{
	offs_t dword_addr = offset / 2;
	return static_cast<UINT16>(offset & 1 ? GET_ODD(m_mem.ram[dword_addr]) : GET_EVEN(m_mem.ram[dword_addr]));
}

//! write i/o space RAM
WRITE16_MEMBER( alto2_cpu_device::ioram_w )
{
	offs_t dword_addr = offset / 2;
	if (offset & 1)
		PUT_ODD(m_mem.ram[dword_addr], data);
	else
		PUT_EVEN(m_mem.ram[dword_addr], data);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

// FIXME
void alto2_cpu_device::device_reset()
{
	soft_reset();
}

/**
 * @brief callback is called by the drive timer whenever a new sector starts
 *
 * @param unit the unit number
 */
static void disk_sector_start(void* cookie, int unit)
{
	alto2_cpu_device* cpu = reinterpret_cast<alto2_cpu_device *>(cookie);
	cpu->next_sector(unit);
}

void alto2_cpu_device::interface_post_reset()
{

	// set the disk unit sector callbacks
	for (int unit = 0; unit < diablo_hd_device::DIABLO_UNIT_MAX; unit++) {
		diablo_hd_device* dhd = m_drive[unit];
		dhd->set_sector_callback(this, &disk_sector_start);
	}
}

//-------------------------------------------------
//  execute_set_input - act on a changed input/
//  interrupt line
//-------------------------------------------------

// FIXME
void alto2_cpu_device::execute_set_input(int inputnum, int state)
{
}

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

// FIXME
void alto2_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		string.printf("%s%s%s%s",
					  m_aluc0 ? "C":"-",
					  m_laluc0 ? "c":"-",
					  m_shifter == 0 ? "0":"-",
					  static_cast<INT16>(m_shifter) < 0 ? "<":"-");
		break;
	}
}

// FIXME
void alto2_cpu_device::fatal(int exitcode, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	emu_fatalerror error(exitcode, format, ap);
	va_end(ap);
}

/** @brief task names */
const char* alto2_cpu_device::task_name(int task)
{
	switch (task) {
	case 000: return "emu";
	case 001: return "task01";
	case 002: return "task02";
	case 003: return "task03";
	case 004: return "ksec";
	case 005: return "task05";
	case 006: return "task06";
	case 007: return "ether";
	case 010: return "mrt";
	case 011: return "dwt";
	case 012: return "curt";
	case 013: return "dht";
	case 014: return "dvt";
	case 015: return "part";
	case 016: return "kwd";
	case 017: return "task17";
	}
	return "???";
}

/** @brief register names (as used by the microcode) */
const char* alto2_cpu_device::r_name(UINT8 reg)
{
	switch (reg) {
	case 000: return "ac(3)";
	case 001: return "ac(2)";
	case 002: return "ac(1)";
	case 003: return "ac(0)";
	case 004: return "nww";
	case 005: return "r05";
	case 006: return "pc";
	case 007: return "r07";
	case 010: return "xh";
	case 011: return "r11";
	case 012: return "ecntr";
	case 013: return "epntr";
	case 014: return "r14";
	case 015: return "r15";
	case 016: return "r16";
	case 017: return "r17";
	case 020: return "curx";
	case 021: return "curdata";
	case 022: return "cba";
	case 023: return "aecl";
	case 024: return "slc";
	case 025: return "mtemp";
	case 026: return "htab";
	case 027: return "ypos";
	case 030: return "dwa";
	case 031: return "kwdctw";
	case 032: return "cksumrw";
	case 033: return "knmarw";
	case 034: return "dcbr";
	case 035: return "dwax";
	case 036: return "mask";
	case 037: return "r37";
	}
	return "???";
}

/** @brief ALU function names */
const char* alto2_cpu_device::aluf_name(UINT8 aluf)
{
	switch (aluf) {
	case 000: return "bus";
	case 001: return "t";
	case 002: return "bus or t";
	case 003: return "bus and t";
	case 004: return "bus xor t";
	case 005: return "bus + 1";
	case 006: return "bus - 1";
	case 007: return "bus + t";
	case 010: return "bus - t";
	case 011: return "bus - t - 1";
	case 012: return "bus + t + 1";
	case 013: return "bus + skip";
	case 014: return "bus, t";
	case 015: return "bus and not t";
	case 016: return "0 (undef)";
	case 017: return "0 (undef)";
	}
	return "???";
}

/** @brief BUS source names */
const char* alto2_cpu_device::bs_name(UINT8 bs)
{
	switch (bs) {
	case 000: return "read_r";
	case 001: return "load_r";
	case 002: return "no_source";
	case 003: return "task_3";
	case 004: return "task_4";
	case 005: return "read_md";
	case 006: return "mouse";
	case 007: return "disp";
	}
	return "???";
}

/** @brief F1 function names */
const char* alto2_cpu_device::f1_name(UINT8 f1)
{
	switch (f1) {
	case 000: return "nop";
	case 001: return "load_mar";
	case 002: return "task";
	case 003: return "block";
	case 004: return "l_lsh_1";
	case 005: return "l_rsh_1";
	case 006: return "l_lcy_8";
	case 007: return "const";
	case 010: return "task_10";
	case 011: return "task_11";
	case 012: return "task_12";
	case 013: return "task_13";
	case 014: return "task_14";
	case 015: return "task_15";
	case 016: return "task_16";
	case 017: return "task_17";
	}
	return "???";
}

/** @brief F2 function names */
const char* alto2_cpu_device::f2_name(UINT8 f2)
{
	switch (f2) {
	case 000: return "nop";
	case 001: return "bus=0";
	case 002: return "shifter<0";
	case 003: return "shifter=0";
	case 004: return "bus";
	case 005: return "alucy";
	case 006: return "load_md";
	case 007: return "const";
	case 010: return "task_10";
	case 011: return "task_11";
	case 012: return "task_12";
	case 013: return "task_13";
	case 014: return "task_14";
	case 015: return "task_15";
	case 016: return "task_16";
	case 017: return "task_17";
	}
	return "???";
}

void alto2_cpu_device::watch_read(UINT32 addr, UINT32 data)
{
	LOG((LOG_MEM,0,"mem: rd[%06o] = %06o\n", addr, data));
}

void alto2_cpu_device::watch_write(UINT32 addr, UINT32 data)
{
	LOG((LOG_MEM,0,"mem: wr[%06o] = %06o\n", addr, data));
}

/** @brief fatal exit on unitialized dynamic phase BUS source */
void alto2_cpu_device::fn_bs_bad_0()
{
	fatal(9,"fatal: bad early bus source pointer for task %s, mpc:%05o bs:%s\n",
		task_name(m_task), m_mpc, bs_name(MIR_BS(m_mir)));
}

/** @brief fatal exit on unitialized latching phase BUS source */
void alto2_cpu_device::fn_bs_bad_1()
{
	fatal(9,"fatal: bad late bus source pointer for task %s, mpc:%05o bs: %s\n",
		task_name(m_task), m_mpc, bs_name(MIR_BS(m_mir)));
}

/** @brief fatal exit on unitialized dynamic phase F1 function */
void alto2_cpu_device::fn_f1_bad_0()
{
	fatal(9,"fatal: bad early f1 function pointer for task %s, mpc:%05o f1: %s\n",
		task_name(m_task), m_mpc, f1_name(MIR_F1(m_mir)));
}

/** @brief fatal exit on unitialized latching phase F1 function */
void alto2_cpu_device::fn_f1_bad_1()
{
	fatal(9,"fatal: bad late f1 function pointer for task %s, mpc:%05o f1: %s\n",
		task_name(m_task), m_mpc, f1_name(MIR_F1(m_mir)));
}

/** @brief fatal exit on unitialized dynamic phase F2 function */
void alto2_cpu_device::fn_f2_bad_0()
{
	fatal(9,"fatal: bad early f2 function pointer for task %s, mpc:%05o f2: %s\n",
		task_name(m_task), m_mpc, f2_name(MIR_F2(m_mir)));
}

/** @brief fatal exit on unitialized latching phase F2 function */
void alto2_cpu_device::fn_f2_bad_1()
{
	fatal(9,"fatal: bad late f2 function pointer for task %s, mpc:%05o f2: %s\n",
		  task_name(m_task), m_mpc, f2_name(MIR_F2(m_mir)));
}

#if	ALTO2_DEBUG
typedef struct {
	UINT16 first, last;
	const char* name;
}	memory_range_name_t;

memory_range_name_t memory_range_name_table[] = {
	{0177016, 0177017,	"UTILOUT    Printer output (Std. Hardware)"},
	{0177020, 0177023,	"XBUS       Utility input bus (Alto II Std. Hardware)"},
	{0177024, 0177024,	"MEAR       Memory Error Address Register (Alto II Std. Hardware)"},
	{0177025, 0177025,	"MESR       Memory error status register (Alto II Std. Hardware)"},
	{0177026, 0177026,	"MECR       Memory error control register (Alto II Std. Hardware)"},
	{0177030, 0177033,	"UTILIN     Printer status, mouse, keyset (all 4 locations return same thing)"},
	{0177034, 0177037,	"KBDAD      Undecoded keyboard (Std. Hardware)"},
	{0177740, 0177757,	"BANKREGS   Extended memory option bank registers"},
	{0177100, 0177100,	"-          Sumagraphics tablet X"},
	{0177101, 0177101,	"-          Sumagraphics tablet Y"},
	{0177140, 0177157,	"-          Organ keyboard"},
	{0177200, 0177204,	"-          PROM programmer"},
	{0177234, 0177237,	"-          Experimental ursor control"},
	{0177240, 0177257,	"-          Alto II debugger"},
	{0177244, 0177247,	"-          Graphics keyboard"},
	{0177400, 0177405,	"-          Maxc2 maintenance interface"},
	{0177400, 0177400,	"-          Alto DLS input (0)"},
	{0177420, 0177420,	"-          Alto DLS input (1)"},
	{0177440, 0177440,	"-          Alto DLS input (2)"},
	{0177460, 0177460,	"-          Alto DLS input (3)"},
	{0177600, 0177677,	"-          Alto DLS output"},
	{0177700, 0177700,	"-          EIA interface output bit"},
	{0177701, 0177701,	"EIALOC     EIA interface input bit"},
	{0177720, 0177737,	"-          TV Camera Interface"},
	{0177764, 0177773,	"-          Redactron tape drive"},
	{0177776, 0177776,	"-          Digital-Analog Converter, Joystick"},
	{0177777, 0177777,	"-          Digital-Analog Converter, Joystick"}
};

static const char* memory_range_name(offs_t offset)
{
	int _min = 0;
	int _max = sizeof(memory_range_name_table) / sizeof(memory_range_name_table[0]) - 1;
	int _mid;

	offset %= ALTO2_IO_PAGE_SIZE;
	offset += ALTO2_IO_PAGE_BASE;

	/* binary search in table of memory ranges */
	while (_max >= _min)
	{
		_mid = (_min + _max) / 2;
		if (memory_range_name_table[_mid].last < offset)
			_min = _mid + 1;
		else if (memory_range_name_table[_mid].first > offset)
			_max = _mid - 1;
		else if (memory_range_name_table[_mid].first <= offset &&
				 memory_range_name_table[_mid].last >= offset)
			return memory_range_name_table[_mid].name;
	}
	return "-          UNUSED";
}

#endif

/**
 * @brief read the open bus for unused MMIO range
 */
READ16_MEMBER( alto2_cpu_device::noop_r )
{
	LOG((LOG_CPU,0,"	MMIO rd %s\n", memory_range_name(offset)));
	return 0177777;
}

/**
 * @brief write nowhere for unused MMIO range
 */
WRITE16_MEMBER( alto2_cpu_device::noop_w )
{
	LOG((LOG_CPU,0,"	MMIO wr %s\n", memory_range_name(offset)));
}

/**
 * @brief read bank register in memory mapped I/O range
 *
 * The bank registers are stored in a 16x4-bit RAM 74S189.
 */
READ16_MEMBER( alto2_cpu_device::bank_reg_r )
{
	int task = offset & 017;
	int bank = m_bank_reg[task] | 0177760;
	return bank;
}

/**
 * @brief write bank register in memory mapped I/O range
 *
 * The bank registers are stored in a 16x4-bit RAM 74S189.
 */
WRITE16_MEMBER( alto2_cpu_device::bank_reg_w )
{
	int task = offset & 017;
	m_bank_reg[task] = data & 017;
	LOG((LOG_CPU,0,"	write bank[%02o]=%#o normal:%o extended:%o (%s)\n",
		task, data,
		GET_BANK_NORMAL(data),
		GET_BANK_EXTENDED(data),
		task_name(task)));
}

/**
 * @brief bs_read_r early: drive bus by R register
 */
void alto2_cpu_device::bs_read_r_0()
{
	UINT16 r = m_r[m_rsel];
	LOG((LOG_CPU,2,"	←R%02o; %s (%#o)\n", m_rsel, r_name(m_rsel), r));
	m_bus &= r;
}

/**
 * @brief bs_load_r early: load R places 0 on the BUS
 */
void alto2_cpu_device::bs_load_r_0()
{
	UINT16 r = 0;
	LOG((LOG_CPU,2,"	R%02o←; %s (BUS&=0)\n", m_rsel, r_name(m_rsel)));
	m_bus &= r;
}

/**
 * @brief bs_load_r late: load R from SHIFTER
 */
void alto2_cpu_device::bs_load_r_1()
{
	if (MIR_F2(m_mir) != f2_emu_load_dns) {
		m_r[m_rsel] = m_shifter;
		LOG((LOG_CPU,2,"	R%02o←; %s = SHIFTER (%#o)\n", m_rsel, r_name(m_rsel), m_shifter));
		/* HACK: programs writing r37 with xxx3 make the cursor
		 * display go nuts. Until I found the real reason for this
		 * obviously buggy display, I just clear the two
		 * least significant bits of r37 if they are set at once.
		 */
		if (m_rsel == 037 && ((m_shifter & 3) == 3)) {
			printf("writing r37 = %#o\n", m_shifter);
			m_r[037] &= ~3;
		}
	}
}

/**
 * @brief bs_read_md early: drive BUS from read memory data
 */
void alto2_cpu_device::bs_read_md_0()
{
#if	ALTO2_DEBUG
	UINT32 mar = m_mem.mar;
#endif
	UINT16 md = read_mem();
	LOG((LOG_CPU,2,"	←MD; BUS&=MD (%#o=[%#o])\n", md, mar));
	m_bus &= md;
}

/**
 * @brief bs_mouse early: drive bus by mouse
 */
void alto2_cpu_device::bs_mouse_0()
{
	UINT16 r = mouse_read();
	LOG((LOG_CPU,2,"	←MOUSE; BUS&=MOUSE (%#o)\n", r));
	m_bus &= r;
}

/**
 * @brief bs_disp early: drive bus by displacement (which?)
 */
void alto2_cpu_device::bs_disp_0()
{
	UINT16 r = 0177777;
	LOG((LOG_CPU,0,"BS ←DISP not handled by task %s mpc:%04x\n", task_name(m_task), m_mpc));
	LOG((LOG_CPU,2,"	←DISP; BUS&=DISP ?? (%#o)\n", r));
	m_bus &= r;
}

/**
 * @brief f1_load_mar late: load memory address register
 *
 * Load memory address register from the ALU output;
 * start main memory reference (see section 2.3).
 */
void alto2_cpu_device::f1_load_mar_1()
{
	UINT8 bank = m_bank_reg[m_task];
	UINT32 msb;
	if (MIR_F2(m_mir) == f2_load_md) {
		msb = GET_BANK_EXTENDED(bank) << 16;
		LOG((LOG_CPU,7, "	XMAR %#o\n", msb | m_alu));
	} else {
		msb = GET_BANK_NORMAL(bank) << 16;

	}
	load_mar(m_rsel, msb | m_alu);
}

#if	USE_PRIO_F9318
/** @brief F9318 input lines */
typedef enum {
	PRIO_IN_EI = (1<<8),
	PRIO_IN_I7 = (1<<7),
	PRIO_IN_I6 = (1<<6),
	PRIO_IN_I5 = (1<<5),
	PRIO_IN_I4 = (1<<4),
	PRIO_IN_I3 = (1<<3),
	PRIO_IN_I2 = (1<<2),
	PRIO_IN_I1 = (1<<1),
	PRIO_IN_I0 = (1<<0),
	/* masks */
	PRIO_I7 = PRIO_IN_I7,
	PRIO_I6_I7 = (PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I5_I7 = (PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I4_I7 = (PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I3_I7 = (PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I2_I7 = (PRIO_IN_I2 | PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I1_I7 = (PRIO_IN_I1 | PRIO_IN_I2 | PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I0_I7 = (PRIO_IN_I0 | PRIO_IN_I1 | PRIO_IN_I2 | PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
}	f9318_in_t;

/** @brief F9318 output lines */
typedef enum {
	PRIO_OUT_Q0 = (1<<0),
	PRIO_OUT_Q1 = (1<<1),
	PRIO_OUT_Q2 = (1<<2),
	PRIO_OUT_EO = (1<<3),
	PRIO_OUT_GS = (1<<4),
	/* masks */
	PRIO_OUT_QZ = (PRIO_OUT_Q0 | PRIO_OUT_Q1 | PRIO_OUT_Q2)
}	f9318_out_t;

/**
 * @brief F9318 priority encoder 8 to 3-bit
 *
 * Emulation of the F9318 chip (pin compatible with 74348).
 *
 * <PRE>
 *            F9318
 *         +---+-+---+
 *         |   +-+   |         +---------------------------------+----------------+
 *    I4' -|1      16|-  Vcc   |              input              |     output     |
 *         |         |         +---------------------------------+----------------+
 *    I5' -|2      15|-  EO'   |      EI I0 I1 I2 I3 I4 I5 I6 I7 | GS Q0 Q1 Q2 EO |
 *         |         |         +---------------------------------+----------------+
 *    I6' -|3      14|-  GS'   | (a)  H  x  x  x  x  x  x  x  x  | H  H  H  H  H  |
 *         |         |         | (b)  L  H  H  H  H  H  H  H  H  | H  H  H  H  L  |
 *    I7' -|4      13|-  I3'   +---------------------------------+----------------+
 *         |         |         | (c)  L  x  x  x  x  x  x  x  L  | L  L  L  L  H  |
 *    EI' -|5      12|-  I2'   | (d)  L  x  x  x  x  x  x  L  H  | L  H  L  L  H  |
 *         |         |         | (e)  L  x  x  x  x  x  L  H  H  | L  L  H  L  H  |
 *    Q2' -|6      11|-  I1'   | (f)  L  x  x  x  x  L  H  H  H  | L  H  H  L  H  |
 *         |         |         | (g)  L  x  x  x  L  H  H  H  H  | L  L  L  H  H  |
 *    Q1' -|7      10|-  I0'   | (h)  L  x  x  L  H  H  H  H  H  | L  H  L  H  H  |
 *         |         |         | (i)  L  x  L  H  H  H  H  H  H  | L  L  H  H  H  |
 *   GND  -|8       9|-  Q0'   | (j)  L  L  H  H  H  H  H  H  H  | L  H  H  H  H  |
 *         |         |         +---------------------------------+----------------+
 *         +---------+
 * </PRE>
 */
static __inline f9318_out_t f9318(f9318_in_t in)
{
	f9318_out_t out;

	if (in & PRIO_IN_EI) {
		out = PRIO_OUT_EO | PRIO_OUT_GS | PRIO_OUT_QZ;
		LOG((LOG_CPU,2,"	f9318 case (a) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (0 == (in & PRIO_I7)) {
		out = PRIO_OUT_EO;
		LOG((LOG_CPU,2,"	f9318 case (c) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I7 == (in & PRIO_I6_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0;
		LOG((LOG_CPU,2,"	f9318 case (d) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I6_I7 == (in & PRIO_I5_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q1;
		LOG((LOG_CPU,2,"	f9318 case (e) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I5_I7 == (in & PRIO_I4_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0 | PRIO_OUT_Q1;
		LOG((LOG_CPU,2,"	f9318 case (f) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I4_I7 == (in & PRIO_I3_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q2;
		LOG((LOG_CPU,2,"	f9318 case (g) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I3_I7 == (in & PRIO_I2_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0 | PRIO_OUT_Q2;
		LOG((LOG_CPU,2,"	f9318 case (h) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I2_I7 == (in & PRIO_I1_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q1 | PRIO_OUT_Q2;
		LOG((LOG_CPU,2,"	f9318 case (i) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I1_I7 == (in & PRIO_I0_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0 | PRIO_OUT_Q1 | PRIO_OUT_Q2;
		LOG((LOG_CPU,2,"	f9318 case (j) in:%#o out:%#o\n", in, out));
		return out;
	}

	out = PRIO_OUT_QZ | PRIO_OUT_GS;
	LOG((LOG_CPU,2,"	f9318 case (b) in:%#o out:%#o\n", in, out));
	return out;
}
#endif

/**
 * @brief f1_task early: task switch
 *
 * The priority encoder finds the highest task requesting service
 * and switches the task number after the next cycle.
 *
 * <PRE>
 *	CT       PROM    NEXT'     RDCT'
 *	1 2 4 8  DATA   6 7 8 9   1 2 4 8
 *	---------------------------------
 *	0 0 0 0  0367   1 1 1 1   0 1 1 1
 *	1 0 0 0  0353   1 1 1 0   1 0 1 1
 *	0 1 0 0  0323   1 1 0 1   0 0 1 1
 *	1 1 0 0  0315   1 1 0 0   1 1 0 1
 *	0 0 1 0  0265   1 0 1 1   0 1 0 1
 *	1 0 1 0  0251   1 0 1 0   1 0 0 1
 *	0 1 1 0  0221   1 0 0 1   0 0 0 1
 *	1 1 1 0  0216   1 0 0 0   1 1 1 0
 *	0 0 0 1  0166   0 1 1 1   0 1 1 0
 *	1 0 0 1  0152   0 1 1 0   1 0 1 0
 *	0 1 0 1  0122   0 1 0 1   0 0 1 0
 *	1 1 0 1  0114   0 1 0 0   1 1 0 0
 *	0 0 1 1  0064   0 0 1 1   0 1 0 0
 *	1 0 1 1  0050   0 0 1 0   1 0 0 0
 *	0 1 1 1  0020   0 0 0 1   0 0 0 0
 *	1 1 1 1  0017   0 0 0 0   1 1 1 1
 *
 * The various task wakeups are encoded using two 8:3-bit priority encoders F9318,
 * which are pin-compatible to the 74348 (inverted inputs and outputs).
 * Their part numbers are U1 and U2.
 * The two encoders are chained (EO of U1 goes to EI of U2):
 *
 * The outputs are fed into some NAND gates (74H10 and 74H00) to decode
 * the task number to latch (CT1-CT4) after a F1 TASK. The case where all
 * of RDCT1' to RDCT8' are high (1) is decoded as RESET'.
 *
 * signal	function
 * --------------------------------------------------
 * CT1		(U1.Q0' & U2.Q0' & RDCT1')'
 * CT2		(U1.Q1' & U2.Q1' & RDCT2')'
 * CT4		(U1.Q2' & U2.Q2' & RDCT4')'
 * CT8		(U1.GS' & RDCT8')'
 * RESET'	RDCT1' & RDCT2' & RDCT4' & RDCT8'
 *
 * In the tables below "x" is RDCTx' of current task
 *
 * signal          input   output, if first 0        CT1  CT2  CT4  CT8
 * ----------------------------------------------------------------------------------------
 * WAKE17' (T19?)   4 I7   Q2:0 Q1:0 Q0:0 GS:0 EO:1  1    1    1    1
 * WAKEKWDT'        3 I6   Q2:0 Q1:0 Q0:1 GS:0 EO:1  x    1    1    1
 * WAKEPART'        2 I5   Q2:0 Q1:1 Q0:0 GS:0 EO:1  1    x    1    1
 * WAKEDVT'         1 I4   Q2:0 Q1:1 Q0:1 GS:0 EO:1  x    x    1    1
 * WAKEDHT'        13 I3   Q2:1 Q1:0 Q0:0 GS:0 EO:1  1    1    x    1
 * WAKECURT'       12 I2   Q2:1 Q1:0 Q0:1 GS:0 EO:1  x    1    x    1
 * WAKEDWT'        11 I1   Q2:1 Q1:1 Q0:0 GS:0 EO:1  1    x    x    1
 * WAKEMRT'        10 I0   Q2:1 Q1:1 Q0:1 GS:0 EO:1  x    x    x    1
 * otherwise               Q2:1 Q1:1 Q0:1 GS:1 EO:0  x    x    x    x
 *
 * signal          input   output, if first 0
 * ----------------------------------------------------------------------------------------
 * WAKEET'          4 I7   Q2:0 Q1:0 Q0:0 GS:0 EO:1  1    1    1    x
 * WAKE6'           3 I6   Q2:0 Q1:0 Q0:1 GS:0 EO:1  x    1    1    x
 * WAKE5'           2 I5   Q2:0 Q1:1 Q0:0 GS:0 EO:1  1    x    1    x
 * WAKEKST'         1 I4   Q2:0 Q1:1 Q0:1 GS:0 EO:1  x    x    1    x
 * WAKE3' (T23?)   13 I3   Q2:1 Q1:0 Q0:0 GS:0 EO:1  1    1    x    x
 * WAKE2'          12 I2   Q2:1 Q1:0 Q0:1 GS:0 EO:1  x    1    x    x
 * WAKE1'          11 I1   Q2:1 Q1:1 Q0:0 GS:0 EO:1  1    x    x    x
 * 0 (GND)         10 I0   Q2:1 Q1:1 Q0:1 GS:0 EO:1  x    x    x    x
 * </PRE>
 */
void alto2_cpu_device::f1_task_0()
{
#if	USE_PRIO_F9318
	/* Doesn't work yet */
	register f9318_in_t wakeup_hi;
	register f9318_out_t u1;
	register f9318_in_t wakeup_lo;
	register f9318_out_t u2;
	register int addr = 017;
	register int rdct1, rdct2, rdct4, rdct8;
	register int ct1, ct2, ct4, ct8;
	register int wakeup, ct;

	LOG((LOG_CPU,2, "	TASK %02o:%s\n", m_task, task_name(m_task)));

	if (m_task > task_emu && (m_task_wakeup & (1 << m_task)))
		addr = m_task;
	LOG((LOG_CPU,2,"	ctl2k_u38[%02o] = %04o\n", addr, ctl2k_u38[addr] & 017));

	rdct1 = (ctl2k_u38[addr] >> U38_RDCT1) & 1;
	rdct2 = (ctl2k_u38[addr] >> U38_RDCT2) & 1;
	rdct4 = (ctl2k_u38[addr] >> U38_RDCT4) & 1;
	rdct8 = (ctl2k_u38[addr] >> U38_RDCT8) & 1;

	/* wakeup signals are active low */
	wakeup = ~m_task_wakeup;

	/* U1
	 * task wakeups 017 to 010 on I7 to I0
	 * EI is 0 (would be 1 at reset)
	 */
	wakeup_hi = (wakeup >> 8) & PRIO_I0_I7;
	u1 = f9318(wakeup_hi);

	/* U2
	 * task wakeups 007 to 001 on I7 to I1, I0 is 0
	 * EO of U1 chained to EI
	 */
	wakeup_lo = wakeup & PRIO_I0_I7;
	if (u1 & PRIO_OUT_EO)
		wakeup_lo |= PRIO_IN_EI;
	u2 = f9318(wakeup_lo);

	/* CT1 = (U1.Q0' & U2.Q0' & RDCT1')' */
	ct1 = !((u1 & PRIO_OUT_Q0) && (u2 & PRIO_OUT_Q0) && rdct1);
	LOG((LOG_CPU,2,"	  CT1:%o U1.Q0':%o U2.Q0':%o RDCT1':%o\n",
		ct1, (u1 & PRIO_OUT_Q0)?1:0, (u2 & PRIO_OUT_Q0)?1:0, rdct1));
	/* CT2 = (U1.Q1' & U2.Q1' & RDCT2')' */
	ct2 = !((u1 & PRIO_OUT_Q1) && (u2 & PRIO_OUT_Q1) && rdct2);
	LOG((LOG_CPU,2,"	  CT2:%o U1.Q1':%o U2.Q1':%o RDCT2':%o\n",
		ct2, (u1 & PRIO_OUT_Q1)?1:0, (u2 & PRIO_OUT_Q1)?1:0, rdct2));
	/* CT4 = (U1.Q2' & U2.Q2' & RDCT4')' */
	ct4 = !((u1 & PRIO_OUT_Q2) && (u2 & PRIO_OUT_Q2) && rdct4);
	LOG((LOG_CPU,2,"	  CT4:%o U1.Q2':%o U2.Q2':%o RDCT4':%o\n",
		ct4, (u1 & PRIO_OUT_Q2)?1:0, (u2 & PRIO_OUT_Q2)?1:0, rdct4));
	/* CT8 */
	ct8 = !((u1 & PRIO_OUT_GS) && rdct8);
	LOG((LOG_CPU,2,"	  CT8:%o U1.GS':%o RDCT8':%o\n",
		ct8, (u1 & PRIO_OUT_GS)?1:0, rdct8));

	ct = 8*ct8 + 4*ct4 + 2*ct2 + ct1;

	if (ct != m_next_task) {
		LOG((LOG_CPU,2, "		switch to %02o\n", ct));
		m_next2_task = ct;
	} else {
		LOG((LOG_CPU,2, "		no switch\n"));
	}
#else	/* USE_PRIO_F9318 */
	int i;

	LOG((LOG_CPU,2, "	TASK %02o:%s", m_task, task_name(m_task)));
	for (i = 15; i >= 0; i--) {
		if (m_task_wakeup & (1 << i)) {
			m_next2_task = i;
			if (m_next2_task != m_next_task) {
				LOG((LOG_CPU,2, " switch to %02o:%s\n", m_next2_task, task_name(m_next2_task)));
			} else {
				LOG((LOG_CPU,2, " no switch\n"));
			}
			return;
		}
	}
	fatal(3, "no tasks requesting service\n");
#endif	/* !USE_PRIO_F9318 */
}

#ifdef	f1_block0_unused
/**
 * @brief f1_block early: block task
 *
 * The task request for the active task is cleared
 */
void alto2_cpu_device::f1_block_0()
{
	CPU_CLR_TASK_WAKEUP(m_task);
	LOG((LOG_CPU,2, "	BLOCK %02o:%s\n", m_task, task_name(m_task)));
}
#endif

/**
 * @brief f2_bus_eq_zero late: branch on bus equals zero
 */
void alto2_cpu_device::f2_bus_eq_zero_1()
{
	UINT16 r = m_bus == 0 ? 1 : 0;
	LOG((LOG_CPU,2, "	BUS=0; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_shifter_lt_zero late: branch on shifter less than zero
 */
void alto2_cpu_device::f2_shifter_lt_zero_1()
{
	UINT16 r = (m_shifter & 0100000) ? 1 : 0;
	LOG((LOG_CPU,2, "	SH<0; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_shifter_eq_zero late: branch on shifter equals zero
 */
void alto2_cpu_device::f2_shifter_eq_zero_1()
{
	UINT16 r = m_shifter == 0 ? 1 : 0;
	LOG((LOG_CPU,2, "	SH=0; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_bus late: branch on bus bits BUS[6-15]
 */
void alto2_cpu_device::f2_bus_1()
{
	UINT16 r = A2_GET32(m_bus,16,6,15);
	LOG((LOG_CPU,2, "	BUS; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_alucy late: branch on latched ALU carry
 */
void alto2_cpu_device::f2_alucy_1()
{
	UINT16 r = m_laluc0;
	LOG((LOG_CPU,2, "	ALUCY; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_load_md late: load memory data
 *
 * Deliver BUS data to memory.
 */
void alto2_cpu_device::f2_load_md_1()
{
#if	ALTO2_DEBUG
	UINT16 mar = m_mem.mar;
#endif
	if (MIR_F1(m_mir) == f1_load_mar) {
		/* part of an XMAR */
		LOG((LOG_CPU,2, "	XMAR %#o (%#o)\n", mar, m_bus));
	} else {
		write_mem(m_bus);
		LOG((LOG_CPU,2, "	MD← BUS ([%#o]=%#o)\n", mar, m_bus));
	}
}

/**
 * @brief read the microcode ROM/RAM halfword
 *
 * Note: HALFSEL is selecting the even (0) or odd (1) half of the
 * microcode RAM 32-bit word. Here's how the demultiplexers (74298)
 * u8, u18, u28 and u38 select the bits:
 *
 *           SN74298
 *         +---+-+---+
 *         |   +-+   |
 *    B2  -|1      16|-  Vcc
 *         |         |
 *    A2  -|2      15|-  QA
 *         |         |
 *    A1  -|3      14|-  QB
 *         |         |
 *    B1  -|4      13|-  QC
 *         |         |
 *    C2  -|5      12|-  QD
 *         |         |
 *    D2  -|6      11|-  CLK
 *         |         |
 *    D1  -|7      10|-  SEL
 *         |         |
 *   GND  -|8       9|-  C1
 *         |         |
 *         +---------+
 *
 *	chip	out pin	BUS	in pin HSEL=0		in pin HSEL=1
 *	--------------------------------------------------------------
 *	u8	QA  15	0	A1   3 DRSEL(0)'	A2   2 DF2(0)
 *	u8	QB  14	1	B1   4 DRSEL(1)'	B2   1 DF2(1)'
 *	u8	QC  13	2	C1   9 DRSEL(2)'	C2   5 DF2(2)'
 *	u8	QD  12	3	D1   7 DRSEL(3)'	D2   6 DF2(3)'
 *
 *	u18	QA  15	4	A1   3 DRSEL(4)'	A2   2 LOADT'
 *	u18	QB  14	5	B1   4 DALUF(0)'	B2   1 LOADL
 *	u18	QC  13	6	C1   9 DALUF(1)'	C2   5 NEXT(00)'
 *	u18	QD  12	7	D1   7 DALUF(2)'	D2   6 NEXT(01)'
 *
 *	u28	QA  15	8	A1   3 DALUF(3)'	A2   2 NEXT(02)'
 *	u28	QB  14	9	B1   4 DBS(0)'		B2   1 NEXT(03)'
 *	u28	QC  13	10	C1   9 DBS(1)'		C2   5 NEXT(04)'
 *	u28	QD  12	11	D1   7 DBS(2)'		D2   6 NEXT(05)'
 *
 *	u38	QA  15	12	A1   3 DF1(0)		A2   2 NEXT(06)'
 *	u38	QB  14	13	B1   4 DF1(1)'		B2   1 NEXT(07)'
 *	u38	QC  13	14	C1   9 DF1(2)'		C2   5 NEXT(08)'
 *	u38	QD  12	15	D1   7 DF1(3)'		D2   6 NEXT(09)'
 *
 * The HALFSEL signal to the demultiplexers is the inverted bit BUS(5):
 * BUS(5)=1, HALFSEL=0, A1,B1,C1,D1 inputs, upper half of the 32-bit word
 * BUS(5)=0, HALFSEL=1, A2,B2,C2,D2 inputs, lower half of the 32-bit word
 */
void alto2_cpu_device::rdram()
{
	UINT32 addr, val;
	UINT32 bank = GET_CRAM_BANKSEL(m_cram_addr) % ALTO2_UCODE_RAM_PAGES;
	UINT32 wordaddr = GET_CRAM_WORDADDR(m_cram_addr);

	m_rdram_flag = 0;
	if (GET_CRAM_RAMROM(m_cram_addr)) {
		/* read ROM 0 at current mpc */
		addr = m_mpc & 01777;
		LOG((LOG_CPU,0,"	rdram: ROM [%05o] ", addr));
	} else {
		/* read RAM 0,1,2 */
		addr = ALTO2_UCODE_RAM_BASE + bank * ALTO2_UCODE_PAGE_SIZE + wordaddr;
		LOG((LOG_CPU,0,"	rdram: RAM%d [%04o] ", bank, wordaddr));
	}

	if (addr >= ALTO2_UCODE_SIZE) {
		val = 0177777;	/* ??? */
		LOG((LOG_CPU,0,"invalid address (%06o)\n", val));
		return;
	}
	val = m_ucode->read_dword(m_ucode->address_to_byte(addr)) ^ ALTO2_UCODE_INVERTED;
	if (GET_CRAM_HALFSEL(m_cram_addr)) {
		val = val >> 16;
		LOG((LOG_CPU,0,"upper:%06o\n", val));
	} else {
		val = val & 0177777;
		LOG((LOG_CPU,0,"lower:%06o\n", val));
	}
	m_bus &= val;
}

/**
 * @brief write the microcode RAM from M register and ALU
 *
 * Note: M is a latch (MYL, i.e. memory L) on the CRAM board that latches
 * the ALU whenever LOADL and GOODTASK are met. GOODTASK is the Emulator
 * task and something I have not yet found out about: TASKA' and TASKB'.
 *
 * There's also an undumped PROM u21 which is addressed by GOODTASK and
 * 7 other signals...
 */
void alto2_cpu_device::wrtram()
{
	UINT32 addr;
	UINT32 bank = GET_CRAM_BANKSEL(m_cram_addr) % ALTO2_UCODE_RAM_PAGES;
	UINT32 wordaddr = GET_CRAM_WORDADDR(m_cram_addr);

	m_wrtram_flag = 0;

	/* write RAM 0,1,2 */
	addr = ALTO2_UCODE_RAM_BASE + bank * ALTO2_UCODE_PAGE_SIZE + wordaddr;
	LOG((LOG_CPU,0,"	wrtram: RAM%d [%04o] upper:%06o lower:%06o", bank, wordaddr, m_m, m_alu));
	if (addr >= ALTO2_UCODE_SIZE) {
		LOG((LOG_CPU,0," invalid address\n"));
		return;
	}
	LOG((LOG_CPU,0,"\n"));
	m_ucode->write_dword(m_ucode->address_to_byte(addr), ((m_m << 16) | m_alu) ^ ALTO2_UCODE_INVERTED);
}

#if	USE_ALU_74181
/**
 * <PRE>
 * Functional description of the 4-bit ALU 74181
 *
 * The 74181 is a 4-bit high speed parallel Arithmetic Logic Unit (ALU).
 * Controlled by four Function Select inputs (S0-S3) and the Mode Control
 * input (M), it can perform all the 16 possible logic operations or 16
 * different arithmetic operations on active HIGH or active LOW operands.
 * The Function Table lists these operations.
 *
 * When the Mode Control input (M) is HIGH, all internal carries are
 * inhibited and the device performs logic operations on the individual
 * bits as listed. When the Mode Control input is LOW, the carries are
 * enabled and the device performs arithmetic operations on the two 4-bit
 * words. The device incorporates full internal carry lookahead and
 * provides for either ripple carry between devices using the Cn+4 output,
 * or for carry lookahead between packages using the signals P' (Carry
 * Propagate) and G' (Carry Generate). In the ADD mode, P' indicates that
 * F' is 15 or more, while G' indicates that F' is 16 or more. In the
 * SUBTRACT mode, P' indicates that F' is zero or less, while G' indicates
 * that F' is less than zero. P' and G' are not affected by carry in.
 * When speed requirements are not stringent, it can be used in a simple
 * ripple carry mode by connecting the Carry output (Cn+4) signal to the
 * Carry input (Cn) of the next unit. For high speed operation the device
 * is used in conjunction with the 74182 carry lookahead circuit. One
 * carry lookahead package is required for each group of four 74181 devices.
 * Carry lookahead can be provided at various levels and offers high speed
 * capability over extremely long word lengths.
 *
 * The A=B output from the device goes HIGH when all four F' outputs are
 * HIGH and can be used to indicate logic equivalence over four bits when
 * the unit is in the subtract mode. The A=B output is open collector and
 * can be wired-AND with other A=B outputs to give a comparison for more
 * than four bits. The A=B signal can also be used with the Cn+4 signal
 * to indicated A>B and A<B.
 *
 * The Function Table lists the arithmetic operations that are performed
 * without a carry in. An incoming carry adds a one to each operation.
 * Thus, select code 0110 generates A minus B minus 1 (2's complement
 * notation) without a carry in and generates A minus B when a carry is
 * applied. Because subtraction is actually performed by the complementary
 * addition (1's complement), a carry out means borrow; thus a carry is
 * generated when there is no underflow and no carry is generated when
 * there is underflow. As indicated, this device can be used with either
 * active LOW inputs producing active LOW outputs or with active HIGH
 * inputs producing active HIGH outputs. For either case the table lists
 * the operations that are performed to the operands labeled inside the
 * logic symbol.
 *
 * The AltoI/II use four 74181s and a 74182 carry lookahead circuit,
 * and the inputs and outputs are all active HIGH.
 *
 * Active HIGH operands:
 *
 * +-------------------+-------------+------------------------+------------------------+
 * |    Mode Select    |   Logic     | Arithmetic w/o carry   | Arithmetic w/ carry    |
 * |      Inputs       |             |                        |                        |
 * |  S3  S2  S1  S0   |   (M=1)     | (M=0) (Cn=1)           | (M=0) (Cn=0)           |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   0   0   | A'          | A                      | A + 1                  |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   0   1   | A' | B'     | A | B                  | (A | B) + 1            |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   1   0   | A' & B      | A | B'                 | (A | B') + 1           |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   1   1   | logic 0     | - 1                    | -1 + 1                 |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   0   0   | (A & B)'    | A + (A & B')           | A + (A & B') + 1       |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   0   1   | B'          | (A | B) + (A & B')     | (A | B) + (A & B') + 1 |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   1   0   | A ^ B       | A - B - 1              | A - B - 1 + 1          |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   1   1   | A & B'      | (A & B) - 1            | (A & B) - 1 + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   0   0   | A' | B      | A + (A & B)            | A + (A & B) + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   0   1   | A' ^ B'     | A + B                  | A + B + 1              |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   1   0   | B           | (A | B') + (A & B)     | (A | B') + (A & B) + 1 |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   1   1   | A & B       | (A & B) - 1            | (A & B) - 1 + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   0   0   | logic 1     | A + A                  | A + A + 1              |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   0   1   | A | B'      | (A | B) + A            | (A | B) + A + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   1   0   | A | B       | (A | B') + A           | (A | B') + A + 1       |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   1   1   | A           | A - 1                  | A - 1 + 1              |
 * +-------------------+-------------+------------------------+------------------------+
 * </PRE>
 */
#define	SMC(s3,s2,s1,s0,m,c) (32*(s3)+16*(s2)+8*(s1)+4*(s0)+2*(m)+(c))

/**
 * @brief Compute the 74181 ALU operation smc
 * @param smc S function, arithmetic/logic flag, carry
 * @return resulting ALU output
 */
UINT32 alto2_cpu_device::alu_74181(UINT32 smc)
{
	register UINT32 a = m_bus;
	register UINT32 b = m_t;
	register UINT32 s = 0;
	register UINT32 f = 0;

	switch (smc) {
	case SMC(0,0,0,0, 0, 0): // 0000: A + 1
		s = 0;
		f = a + 1;
		break;

	case SMC(0,0,0,0, 0, 1): // 0000: A
		s = 0;
		f = a;
		break;

	case SMC(0,0,0,1, 0, 0): // 0001: (A | B) + 1
		s = 0;
		f = (a | b) + 1;
		break;

	case SMC(0,0,0,1, 0, 1): // 0001: A | B
		s = 0;
		f = a | b;
		break;

	case SMC(0,0,1,0, 0, 0): // 0010: (A | B') + 1
		s = 0;
		f = (a | ~b) + 1;
		break;

	case SMC(0,0,1,0, 0, 1): // 0010: A | B'
		s = 0;
		f = a | ~b;
		break;

	case SMC(0,0,1,1, 0, 0): // 0011: -1 + 1
		s = 1;
		f = -1 + 1;
		break;

	case SMC(0,0,1,1, 0, 1): // 0011: -1
		s = 1;
		f = -1;
		break;

	case SMC(0,1,0,0, 0, 0): // 0100: A + (A & B') + 1
		s = 0;
		f = a + (a & ~b) + 1;
		break;

	case SMC(0,1,0,0, 0, 1): // 0100: A + (A & B')
		s = 0;
		f = a + (a & ~b);
		break;

	case SMC(0,1,0,1, 0, 0): // 0101: (A | B) + (A & B') + 1
		s = 0;
		f = (a | b) + (a & ~b) + 1;
		break;

	case SMC(0,1,0,1, 0, 1): // 0101: (A | B) + (A & B')
		s = 0;
		f = (a | b) + (a & ~b);
		break;

	case SMC(0,1,1,0, 0, 0): // 0110: A - B - 1 + 1
		s = 1;
		f = a - b - 1 + 1;
		break;

	case SMC(0,1,1,0, 0, 1): // 0110: A - B - 1
		s = 1;
		f = a - b - 1;
		break;

	case SMC(0,1,1,1, 0, 0): // 0111: (A & B) - 1 + 1
		s = 1;
		f = (a & b) - 1 + 1;
		break;

	case SMC(0,1,1,1, 0, 1): // 0111: (A & B) - 1
		s = 1;
		f = (a & b) - 1;
		break;

	case SMC(1,0,0,0, 0, 0): // 1000: A + (A & B) + 1
		s = 0;
		f = a + (a & b) + 1;
		break;

	case SMC(1,0,0,0, 0, 1): // 1000: A + (A & B)
		s = 0;
		f = a + (a & b);
		break;

	case SMC(1,0,0,1, 0, 0): // 1001: A + B + 1
		s = 0;
		f = a + b + 1;
		break;

	case SMC(1,0,0,1, 0, 1): // 1001: A + B
		s = 0;
		f = a + b;
		break;

	case SMC(1,0,1,0, 0, 0): // 1010: (A | B') + (A & B) + 1
		s = 0;
		f = (a | ~b) + (a & b) + 1;
		break;

	case SMC(1,0,1,0, 0, 1): // 1010: (A | B') + (A & B)
		s = 0;
		f = (a | ~b) + (a & b);
		break;

	case SMC(1,0,1,1, 0, 0): // 1011: (A & B) - 1 + 1
		s = 1;
		f = (a & b) - 1 + 1;
		break;

	case SMC(1,0,1,1, 0, 1): // 1011: (A & B) - 1
		s = 1;
		f = (a & b) - 1;
		break;

	case SMC(1,1,0,0, 0, 0): // 1100: A + A + 1
		s = 0;
		f = a + a + 1;
		break;

	case SMC(1,1,0,0, 0, 1): // 1100: A + A
		s = 0;
		f = a + a;
		break;

	case SMC(1,1,0,1, 0, 0): // 1101: (A | B) + A + 1
		s = 0;
		f = (a | b) + a + 1;
		break;

	case SMC(1,1,0,1, 0, 1): // 1101: (A | B) + A
		s = 0;
		f = (a | b) + a;
		break;

	case SMC(1,1,1,0, 0, 0): // 1110: (A | B') + A + 1
		s = 0;
		f = (a | ~b) + a + 1;
		break;

	case SMC(1,1,1,0, 0, 1): // 1110: (A | B') + A
		s = 0;
		f = (a | ~b) + a;
		break;

	case SMC(1,1,1,1, 0, 0): // 1111: A - 1 + 1
		s = 1;
		f = a - 1 + 1;
		break;

	case SMC(1,1,1,1, 0, 1): // 1111: A - 1
		s = 1;
		f = a - 1;
		break;

	case SMC(0,0,0,0, 1, 0): // 0000: A'
	case SMC(0,0,0,0, 1, 1):
		f = ~a;
		break;

	case SMC(0,0,0,1, 1, 0): // 0001: A' | B'
	case SMC(0,0,0,1, 1, 1):
		f = ~a | ~b;
		break;

	case SMC(0,0,1,0, 1, 0): // 0010: A' & B
	case SMC(0,0,1,0, 1, 1):
		f = ~a & b;
		break;

	case SMC(0,0,1,1, 1, 0): // 0011: logic 0
	case SMC(0,0,1,1, 1, 1):
		f = 0;
		break;

	case SMC(0,1,0,0, 1, 0): // 0100: (A & B)'
	case SMC(0,1,0,0, 1, 1):
		f = ~(a & b);
		break;

	case SMC(0,1,0,1, 1, 0): // 0101: B'
	case SMC(0,1,0,1, 1, 1):
		f = ~b;
		break;

	case SMC(0,1,1,0, 1, 0): // 0110: A ^ B
	case SMC(0,1,1,0, 1, 1):
		f = a ^ b;
		break;

	case SMC(0,1,1,1, 1, 0): // 0111: A & B'
	case SMC(0,1,1,1, 1, 1):
		f = a & ~b;
		break;

	case SMC(1,0,0,0, 1, 0): // 1000: A' | B
	case SMC(1,0,0,0, 1, 1):
		f = ~a | b;
		break;

	case SMC(1,0,0,1, 1, 0): // 1001: A' ^ B'
	case SMC(1,0,0,1, 1, 1):
		f = ~a ^ ~b;
		break;

	case SMC(1,0,1,0, 1, 0): // 1010: B
	case SMC(1,0,1,0, 1, 1):
		f = b;
		break;

	case SMC(1,0,1,1, 1, 0): // 1011: A & B
	case SMC(1,0,1,1, 1, 1):
		f = a & b;
		break;

	case SMC(1,1,0,0, 1, 0): // 1100: logic 1
	case SMC(1,1,0,0, 1, 1):
		f = ~0;
		break;

	case SMC(1,1,0,1, 1, 0): // 1101: A | B'
	case SMC(1,1,0,1, 1, 1):
		f = a | ~b;
		break;

	case SMC(1,1,1,0, 1, 0): // 1110: A | B
	case SMC(1,1,1,0, 1, 1):
		f = a | b;
		break;

	case SMC(1,1,1,1, 1, 0): // 1111: A
	case SMC(1,1,1,1, 1, 1):
		f = a;
		break;
	}
	if (smc & 2) {
		m_aluc0 = ((f >> 16) ^ s) & 1;
	} else {
		m_aluc0 = 1;
	}
	return f;
}
#endif

/** @brief flag that tells whether to load the T register from BUS or ALU */
#define	TSELECT	1

/** @brief flag that tells wheter operation was 0: logic (M=1) or 1: arithmetic (M=0) */
#define	ALUM2	2

/** @brief execute the CPU for at most nsecs nano seconds */
void alto2_cpu_device::execute_run()
{
	m_next = m_task_mpc[m_task];		// get current task's next mpc and address modifier
	m_next2 = m_task_next2[m_task];

	do {
		int do_bs, flags;
		UINT32 alu;
		UINT8 aluf;
		UINT8 bs;
		UINT8 f1;
		UINT8 f2;

		/*
		 * Subtract the microcycle time from the display time accu.
		 * If it underflows, call the display state machine and add
		 * the time for 24 pixel clocks to the accu.
		 * This is very close to every seventh CPU cycle.
		 */
		m_dsp_time -= ALTO2_UCYCLE;
		if (m_dsp_time < 0) {
			m_dsp_state = display_state_machine(m_dsp_state);
			m_dsp_time += ALTO2_DISPLAY_BITTIME(24);
		}
		if (m_unload_time >= 0) {
			/*
			 * Subtract the microcycle time from the unload time accu.
			 * If it underflows, call the unload word function which adds
			 * the time for 16 or 32 pixel clocks to the accu, or ends
			 * the unloading by leaving m_unload_time at -1.
			 */
			m_unload_time -= ALTO2_UCYCLE;
			if (m_unload_time < 0) {
				m_unload_word = unload_word(m_unload_word);
			}
		}
#if	(USE_BITCLK_TIMER == 0)
		if (m_bitclk_time >= 0) {
			/*
			 * Subtract the microcycle time from the bitclk time accu.
			 * If it underflows, call the disk bitclk function which adds
			 * the time for one bit as clocks to the accu, or ends
			 * the bitclk sequence by leaving m_bitclk_time at -1.
			 */
			m_bitclk_time -= ALTO2_UCYCLE;
			disk_bitclk(0, m_bitclk_index);
			m_bitclk_index++;
		}
#endif

		m_cycle++;
		/* nano seconds per cycle */
		m_ntime[m_task] += ALTO2_UCYCLE;

		/* next instruction's mpc */
		m_mpc = m_next;
		m_mir = m_ucode->read_dword(m_ucode->address_to_byte(m_mpc));
		m_rsel = MIR_RSEL(m_mir);
		m_next = MIR_NEXT(m_mir) | m_next2;
		m_next2 = A2_GET32(m_ucode->read_dword(m_ucode->address_to_byte(m_next)), 32, NEXT0, NEXT9) | (m_next2 & ~ALTO2_UCODE_PAGE_MASK);
		aluf = MIR_ALUF(m_mir);
		bs = MIR_BS(m_mir);
		f1 = MIR_F1(m_mir);
		f2 = MIR_F2(m_mir);
		LOG((LOG_CPU,2,"%s-%04o: %011o r:%02o aluf:%02o bs:%02o f1:%02o f2:%02o t:%o l:%o next:%05o next2:%05o\n",
			task_name(m_task), m_mpc, m_mir, m_rsel, aluf, bs, f1, f2, MIR_T(m_mir), MIR_L(m_mir), m_next, m_next2));
		debugger_instruction_hook(this, m_mpc);

		/*
		 * This bus source decoding is not performed if f1 = 7 or f2 = 7.
		 * These functions use the BS field to provide part of the address
		 * to the constant ROM
		 */
		do_bs = !(f1 == f1_const || f2 == f2_const);

		if (f1 == f1_load_mar) {
			if (check_mem_load_mar_stall(m_rsel)) {
				LOG((LOG_CPU,3, "	MAR← stall\n"));
				m_next2 = m_next;
				m_next = m_mpc;
				continue;
			}
		} else if (f2 == f2_load_md) {
			if (check_mem_write_stall()) {
				LOG((LOG_CPU,3, "	MD← stall\n"));
				m_next2 = m_next;
				m_next = m_mpc;
				continue;
			}
		}
		if (do_bs && bs == bs_read_md) {
			if (check_mem_read_stall()) {
				LOG((LOG_CPU,3, "	←MD stall\n"));
				m_next2 = m_next;
				m_next = m_mpc;
				continue;
			}
		}

		m_bus = 0177777;

		if (m_rdram_flag)
			rdram();

		/*
		 * The constant memory is gated to the bus by F1 = 7, F2 = 7, or BS >= 4
		 */
		if (!do_bs || bs >= 4) {
			int addr = 8 * m_rsel + bs;
			UINT16 data = m_const->read_dword(m_const->address_to_byte(addr));
			LOG((LOG_CPU,2,"	%#o; BUS &= CONST[%03o]\n", data, addr));
			m_bus &= data;
		}

		/*
		 * early f2 has to be done before early bs, because the
		 * emulator f2 acsource or acdest may change rsel
		 */
		((*this).*m_f2[0][m_task][f2])();

		/*
		 * early bs can be done now
		 */
		if (do_bs)
			((*this).*m_bs[0][m_task][bs])();

		/*
		 * early f1
		 */
		((*this).*m_f1[0][m_task][f1])();

		/* compute the ALU function */
		switch (aluf) {
		/**
		 * 00: ALU ← BUS
		 * PROM data for S3-0:1111 M:1 C:0
		 * 74181 function F=A
		 * T source is ALU
		 */
		case aluf_bus__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,1,1,1, 1, 0));
#else
			alu = m_bus;
			m_aluc0 = 1;
#endif
			flags = TSELECT;
			LOG((LOG_CPU,2,"	ALU← BUS (%#o := %#o)\n", alu, m_bus));
			break;

		/**
		 * 01: ALU ← T
		 * PROM data for S3-0:1010 M:1 C:0
		 * 74181 function F=B
		 * T source is BUS
		 */
		case aluf_treg:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,1,0, 1, 0));
#else
			alu = m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((LOG_CPU,2,"	ALU← T (%#o := %#o)\n", alu, m_t));
			break;

		/**
		 * 02: ALU ← BUS | T
		 * PROM data for S3-0:1110 M:1 C:0
		 * 74181 function F=A|B
		 * T source is ALU
		 */
		case aluf_bus_or_t__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,1,1,0, 1, 0));
#else
			alu = m_bus | m_t;
			m_aluc0 = 1;
#endif
			flags = TSELECT;
			LOG((LOG_CPU,2,"	ALU← BUS OR T (%#o := %#o | %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 03: ALU ← BUS & T
		 * PROM data for S3-0:1011 M:1 C:0
		 * 74181 function F=A&B
		 * T source is BUS
		 */
		case aluf_bus_and_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,1,1, 1, 0));
#else
			alu = m_bus & m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((LOG_CPU,2,"	ALU← BUS AND T (%#o := %#o & %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 04: ALU ← BUS ^ T
		 * PROM data for S3-0:0110 M:1 C:0
		 * 74181 function F=A^B
		 * T source is BUS
		 */
		case aluf_bus_xor_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,0, 1, 0));
#else
			alu = m_bus ^ m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((LOG_CPU,2,"	ALU← BUS XOR T (%#o := %#o ^ %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 05: ALU ← BUS + 1
		 * PROM data for S3-0:0000 M:0 C:0
		 * 74181 function F=A+1
		 * T source is ALU
		 */
		case aluf_bus_plus_1__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,0,0, 0, 0));
#else
			alu = m_bus + 1;
			m_aluc0 = (alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((LOG_CPU,2,"	ALU← BUS + 1 (%#o := %#o + 1)\n", alu, m_bus));
			break;

		/**
		 * 06: ALU ← BUS - 1
		 * PROM data for S3-0:1111 M:0 C:1
		 * 74181 function F=A-1
		 * T source is ALU
		 */
		case aluf_bus_minus_1__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,1,1,1, 0, 1));
#else
			alu = m_bus + 0177777;
			m_aluc0 = (~m_alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((LOG_CPU,2,"	ALU← BUS - 1 (%#o := %#o - 1)\n", alu, m_bus));
			break;

		/**
		 * 07: ALU ← BUS + T
		 * PROM data for S3-0:1001 M:0 C:1
		 * 74181 function F=A+B
		 * T source is BUS
		 */
		case aluf_bus_plus_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,0,1, 0, 1));
#else
			alu = m_bus + m_t;
			m_aluc0 = (m_alu >> 16) & 1;
#endif
			flags = ALUM2;
			LOG((LOG_CPU,2,"	ALU← BUS + T (%#o := %#o + %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 10: ALU ← BUS - T
		 * PROM data for S3-0:0110 M:0 C:0
		 * 74181 function F=A-B
		 * T source is BUS
		 */
		case aluf_bus_minus_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,0, 0, 0));
#else
			alu = m_bus + ~m_t + 1;
			m_aluc0 = (~m_alu >> 16) & 1;
#endif
			flags = ALUM2;
			LOG((LOG_CPU,2,"	ALU← BUS - T (%#o := %#o - %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 11: ALU ← BUS - T - 1
		 * PROM data for S3-0:0110 M:0 C:1
		 * 74181 function F=A-B-1
		 * T source is BUS
		 */
		case aluf_bus_minus_t_minus_1:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,0, 0, 1));
#else
			alu = m_bus + ~m_t;
			m_aluc0 = (~m_alu >> 16) & 1;
#endif
			flags = ALUM2;
			LOG((LOG_CPU,2,"	ALU← BUS - T - 1 (%#o := %#o - %#o - 1)\n", alu, m_bus, m_t));
			break;

		/**
		 * 12: ALU ← BUS + T + 1
		 * PROM data for S3-0:1001 M:0 C:0
		 * 74181 function F=A+B+1
		 * T source is ALU
		 */
		case aluf_bus_plus_t_plus_1__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,0,1, 0, 0));
#else
			alu = m_bus + m_t + 1;
			m_aluc0 = (m_alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((LOG_CPU,2,"	ALU← BUS + T + 1 (%#o := %#o + %#o + 1)\n", alu, m_bus, m_t));
			break;

		/**
		 * 13: ALU ← BUS + SKIP
		 * PROM data for S3-0:0000 M:0 C:SKIP
		 * 74181 function F=A (SKIP=1) or F=A+1 (SKIP=0)
		 * T source is ALU
		 */
		case aluf_bus_plus_skip__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,0,0, 0, m_emu.skip^1));
#else
			alu = m_bus + m_emu.skip;
			m_aluc0 = (m_alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((LOG_CPU,2,"	ALU← BUS + SKIP (%#o := %#o + %#o)\n", alu, m_bus, m_emu.skip));
			break;

		/**
		 * 14: ALU ← BUS,T
		 * PROM data for S3-0:1011 M:1 C:0
		 * 74181 function F=A&B
		 * T source is ALU
		 */
		case aluf_bus_and_t__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,1,1, 1, 0));
#else
			alu = m_bus & m_t;
			m_aluc0 = 1;
#endif
			flags = TSELECT;
			LOG((LOG_CPU,2,"	ALU← BUS,T (%#o := %#o & %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 15: ALU ← BUS & ~T
		 * PROM data for S3-0:0111 M:1 C:0
		 * 74181 function F=A&~B
		 * T source is BUS
		 */
		case aluf_bus_and_not_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,1, 1, 0));
#else
			alu = m_bus & ~m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((LOG_CPU,2,"	ALU← BUS AND NOT T (%#o := %#o & ~%#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 16: ALU ← ???
		 * PROM data for S3-0:???? M:? C:?
		 * 74181 perhaps F=0 (0011/0/0)
		 * T source is BUS
		 */
		case aluf_undef_16:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,1,1, 0, 0));
#else
			alu = 0;
			m_aluc0 = 1;
#endif
			flags = ALUM2;
			LOG((LOG_CPU,0,"	ALU← 0 (illegal aluf in task %s, mpc:%05o aluf:%02o)\n", task_name(m_task), m_mpc, aluf));
			break;

		/**
		 * 17: ALU ← ???
		 * PROM data for S3-0:???? M:? C:?
		 * 74181 perhaps F=~0 (0011/0/1)
		 * T source is BUS
		 */
		case aluf_undef_17:
		default:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,1,1, 0, 1));
#else
			alu = 0177777;
			m_aluc0 = 1;
#endif
			flags = ALUM2;
			LOG((LOG_CPU,0,"	ALU← 0 (illegal aluf in task %s, mpc:%05o aluf:%02o)\n", task_name(m_task), m_mpc, aluf));
		}
		m_alu = static_cast<UINT16>(alu);

		/* WRTRAM now, before L is changed */
		if (m_wrtram_flag)
			wrtram();

		switch (f1) {
		case f1_l_lsh_1:
			if (m_task == task_emu) {
				if (f2 == f2_emu_magic) {
					m_shifter = ((m_l << 1) | (m_t >> 15)) & 0177777;
					LOG((LOG_CPU,2,"	SHIFTER ←L MLSH 1 (%#o := %#o<<1|%#o)\n", m_shifter, m_l, m_t >> 15));
					break;
				}
				if (f2 == f2_emu_load_dns) {
					/* shifter is done in F2 */
					break;
				}
			}
			m_shifter = (m_l << 1) & 0177777;
			LOG((LOG_CPU,2,"	SHIFTER ←L LSH 1 (%#o := %#o<<1)\n", m_shifter, m_l));
			break;

		case f1_l_rsh_1:
			if (m_task == task_emu) {
				if (f2 == f2_emu_magic) {
					m_shifter = ((m_l >> 1) | (m_t << 15)) & 0177777;
					LOG((LOG_CPU,2,"	SHIFTER ←L MRSH 1 (%#o := %#o>>1|%#o)\n", m_shifter, m_l, (m_t << 15) & 0100000));
					break;
				}
				if (f2 == f2_emu_load_dns) {
					/* shifter is done in F2 */
					break;
				}
			}
			m_shifter = m_l >> 1;
			LOG((LOG_CPU,2,"	SHIFTER ←L RSH 1 (%#o := %#o>>1)\n", m_shifter, m_l));
			break;

		case f1_l_lcy_8:
			m_shifter = ((m_l >> 8) | (m_l << 8)) & 0177777;
			LOG((LOG_CPU,2,"	SHIFTER ←L LCY 8 (%#o := bswap %#o)\n", m_shifter, m_l));
			break;

		default:
			/* shifter passes L, if F1 is not one of L LSH 1, L RSH 1 or L LCY 8 */
			m_shifter = m_l;
		}

		/* late F1 is done now, if any */
		((*this).*m_f1[1][m_task][f1])();

		/* late F2 is done now, if any */
		((*this).*m_f2[1][m_task][f2])();

		/* late BS is done now, if no constant was put on the bus */
		if (do_bs)
			((*this).*m_bs[1][m_task][bs])();

		/*
		 * update L register and LALUC0, and also M register,
		 * if a RAM related task is active
		 */
		if (MIR_L(m_mir)) {
			/* load L from ALU */
			m_l = m_alu;
			if (flags & ALUM2) {
				m_laluc0 = m_aluc0;
				LOG((LOG_CPU,2, "	L← ALU (%#o); LALUC0← ALUC0 (%o)\n", m_alu, m_aluc0));
			} else {
				m_laluc0 = 0;
				LOG((LOG_CPU,2, "	L← ALU (%#o); LALUC0← %o\n", m_alu, 0));
			}
			if (m_ram_related[m_task]) {
				/* load M from ALU, if 'GOODTASK' */
				m_m = m_alu;
				/* also writes to S[bank][0], which can't be read */
				m_s[m_s_reg_bank[m_task]][0] = m_alu;
				LOG((LOG_CPU,2, "	M← ALU (%#o)\n", m_alu));
			}
		}

		/* update T register, if LOADT is set */
		if (MIR_T(m_mir)) {
			m_cram_addr = m_alu;
			if (flags & TSELECT) {
				LOG((LOG_CPU,2, "	T← ALU (%#o)\n", m_alu));
				m_t = m_alu;
			} else {
				LOG((LOG_CPU,2, "	T← BUS (%#o)\n", m_bus));
				m_t = m_bus;
			}
		}

		if (m_task != m_next2_task) {
			/* switch now? */
			if (m_task == m_next_task) {
				/* one more microinstruction */
				m_next_task = m_next2_task;
			} else {
				/* save this task's mpc */
				m_task_mpc[m_task] = m_next;
				m_task_next2[m_task] = m_next2;
				m_task = m_next_task;
				LOG((LOG_CPU,1, "task switch to %02o:%s (cycle %lld)\n", m_task, task_name(m_task), cycle()));
				/* get new task's mpc */
				m_next = m_task_mpc[m_task];
				/* get address modifier after task switch (?) */
				m_next2 = m_task_next2[m_task];

				/*
				 * let the task know it becomes active now
				 * and (most probably) reset the wakeup
				 */
				((*this).*m_active_callback[m_task])();
			}
		}
	} while (m_icount-- > 0);

	/* save this task's mpc and address modifier */
	m_task_mpc[m_task] = m_next;
	m_task_next2[m_task] = m_next2;
}

/** @brief reset the various registers */
void alto2_cpu_device::hard_reset()
{
	/* all tasks start in ROM0 */
	m_reset_mode = 0xffff;

	memset(&m_ram_related, 0, sizeof(m_ram_related));

	// install standard handlers in all tasks
	for (int task = 0; task < ALTO2_TASKS; task++) {

		// every task starts at mpc = task number, in either ROM0 or RAM0
		m_task_mpc[task] = (m_ctl2k_u38[task] >> 4) ^ 017;
		m_active_callback[task] = &alto2_cpu_device::noop;
		if (0 == (m_reset_mode & (1 << task)))
			m_task_mpc[task] |= ALTO2_UCODE_RAM_BASE;

		set_bs(task, bs_read_r,			&alto2_cpu_device::bs_read_r_0,	0);
		set_bs(task, bs_load_r,			&alto2_cpu_device::bs_load_r_0,	&alto2_cpu_device::bs_load_r_1);
		set_bs(task, bs_no_source,		0, 0);
		set_bs(task, bs_task_3,			&alto2_cpu_device::fn_bs_bad_0,	&alto2_cpu_device::fn_bs_bad_1);	// task specific
		set_bs(task, bs_task_4,			&alto2_cpu_device::fn_bs_bad_0,	&alto2_cpu_device::fn_bs_bad_1);	// task specific
		set_bs(task, bs_read_md,		&alto2_cpu_device::bs_read_md_0, 0);
		set_bs(task, bs_mouse,			&alto2_cpu_device::bs_mouse_0, 0);
		set_bs(task, bs_disp,			&alto2_cpu_device::bs_disp_0, 0);

		set_f1(task, f1_nop,			0, 0);
		set_f1(task, f1_load_mar,		0, &alto2_cpu_device::f1_load_mar_1);
		set_f1(task, f1_task,			&alto2_cpu_device::f1_task_0, 0);
		set_f1(task, f1_block,			&alto2_cpu_device::fn_f1_bad_0, &alto2_cpu_device::fn_f1_bad_1);	// not all tasks have the f1_block
		set_f1(task, f1_l_lsh_1,		0, 0);			// inlined in execute()
		set_f1(task, f1_l_rsh_1,		0, 0);			// inlined in execute()
		set_f1(task, f1_l_lcy_8,		0, 0);			// inlined in execute()
		set_f1(task, f1_const,			0, 0);
		set_f1(task, f1_task_10,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_11,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_12,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_13,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_14,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_15,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_16,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_17,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific

		set_f2(task, f2_nop,			0, 0);
		set_f2(task, f2_bus_eq_zero,	0, &alto2_cpu_device::f2_bus_eq_zero_1);
		set_f2(task, f2_shifter_lt_zero,0, &alto2_cpu_device::f2_shifter_lt_zero_1);
		set_f2(task, f2_shifter_eq_zero,0, &alto2_cpu_device::f2_shifter_eq_zero_1);
		set_f2(task, f2_bus,			0, &alto2_cpu_device::f2_bus_1);
		set_f2(task, f2_alucy,			0, &alto2_cpu_device::f2_alucy_1);
		set_f2(task, f2_load_md,		0, &alto2_cpu_device::f2_load_md_1);
		set_f2(task, f2_const,			0, 0);
		set_f2(task, f2_task_10,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_11,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_12,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_13,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_14,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_15,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_16,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_17,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
	}

	init_memory();
	init_disk();
	init_disp();
	init_kbd();
	init_hw();

	init_emu(task_emu);
	init_ksec(task_ksec);
	init_ether(task_ether);
	init_mrt(task_mrt);
	init_dwt(task_dwt);
	init_curt(task_curt);
	init_dht(task_dht);
	init_dvt(task_dvt);
	init_part(task_part);
	init_kwd(task_kwd);

	m_dsp_time = 0;			// reset the display state machine values
	m_dsp_state = 020;

	m_task = 0;						// start with task 0
	m_task_wakeup |= 1 << 0;		// set wakeup flag
}

/** @brief software initiated reset (STARTF) */
int alto2_cpu_device::soft_reset()
{

	for (int task = 0; task < ALTO2_TASKS; task++) {
		// every task starts at mpc = task number, in either ROM0 or RAM0
		m_task_mpc[task] = (m_ctl2k_u38[task] >> 4) ^ 017;
		if (0 == (m_reset_mode & (1 << task)))
			m_task_mpc[task] |= ALTO2_UCODE_RAM_BASE;
	}
	m_next2_task = 0;		// switch to task 0
	m_reset_mode = 0xffff;	// all tasks start in ROM0 again

	m_dsp_time = 0;			// reset the display state machine values
	m_dsp_state = 020;

	return m_next_task;		// return next task (?)
}

/**************************************************************************************************
;	A L T O I I C O D E 3 . M U
; Copyright Xerox Corporation 1979

;***Derived from ALTOIICODE2.MU, as last modified by
;***Tobol, August 5, 1976 12:13 PM -- fix DIOG2 bug
;***modified by Ingalls, September 6, 1977
; BitBLT fixed (LREG bug) and extended for new memory
;***modified by Boggs and Taft September 15, 1977  10:10 PM
; Modified MRT to refresh 16K chips and added XMSTA and XMLDA.
; Fixed two bugs in DEXCH and a bug in the interval timer.
; Moved symbol and constant definitions into AltoConsts23.mu.
; MRT split and moved into two 'get' files.
;***modified by Boggs and Taft November 21, 1977  5:10 PM
; Fixed a bug in the Ethernet input main loop.
;***modified by Boggs November 28, 1977  3:53 PM
; Mess with the information returned by VERS

;Get the symbol and constant definitions
#AltoConsts23.mu;

;LABEL PREDEFINITIONS

;The reset locations of the tasks:

!17,20,NOVEM,,,,KSEC,,,EREST,MRT,DWT,CURT,DHT,DVT,PART,KWDX,;

;Locations which may need to be accessible from the Ram, or Ram
;  locations which are accessed from the Rom (TRAP1):
!37,20,START,RAMRET,RAMCYCX,,,,,,,,,,,,,TRAP1;

;Macro-op dispatch table:
!37,20,DOINS,DOIND,EMCYCLE,NOPAR,JSRII,U5,U6,U7,,,,,,,RAMTRAP,TRAP;

;Parameterless macro-op sub-table:
!37,40,DIR,EIR,BRI,RCLK,SIO,BLT,BLKS,SIT,JMPR,RDRM,WTRM,DIRS,VERS,DREAD,DWRITE,DEXCH,MUL,DIV,DIOG1,DIOG2,BITBLT,XMLDA,XMSTA,,,,,,,,,;

;Cycle dispatch table:
!37,20,L0,L1,L2,L3,L4,L5,L6,L7,L8,R7,R6,R5,R4,R3X,R2X,R1X;

;some global R-Registers
$NWW		$R4;		State of interrupt system
$R37		$R37;		Used by MRT, interval timer and EIA
$MTEMP		$R25;		Public temporary R-Register


;The Display Controller

; its R-Registers:
$CBA		$R22;
$AECL		$R23;
$SLC		$R24;
$HTAB		$R26;
$YPOS		$R27;
$DWA		$R30;
$CURX		$R20;
$CURDATA	$R21;

; its task specific functions:
$EVENFIELD	$L024010,000000,000000; F2 = 10 DHT DVT
$SETMODE	$L024011,000000,000000; F2 = 11 DHT
$DDR		$L026010,000000,124100; F2 = 10 DWT

!1,2,DVT1,DVT11;
!1,2,MOREB,NOMORE;
!1,2,NORMX,HALFX;
!1,2,NODD,NEVEN;
!1,2,DHT0,DHT1;
!1,2,NORMODE,HALFMODE;
!1,2,DWTZ,DWTY;
!1,2,DOTAB,NOTAB;
!1,2,XNOMORE,DOMORE;

;Display Vertical Task

DVT:	MAR← L← DASTART+1;
	CBA← L, L← 0;
	CURDATA← L;
	SLC← L;
	T← MD;			CAUSE A VERTICAL FIELD INTERRUPT
	L← NWW OR T;
	MAR← CURLOC;		SET UP THE CURSOR
	NWW← L, T← 0-1;
	L← MD XOR T;		HARDWARE EXPECTS X COMPLEMENTED
	T← MD, EVENFIELD;
	CURX← L, :DVT1;

DVT1:	L← BIAS-T-1, TASK, :DVT2;	BIAS THE Y COORDINATE
DVT11:	L← BIAS-T, TASK;

DVT2:	YPOS← L, :DVT;

;Display Horizontal Task.
;11 cycles if no block change, 17 if new control block.

DHT:	MAR← CBA-1;
	L← SLC -1, BUS=0;
	SLC← L, :DHT0;

DHT0:	T← 37400;		MORE TO DO IN THIS BLOCK
	SINK← MD;
	L← T← MD AND T, SETMODE;
	HTAB← L LCY 8, :NORMODE;

NORMODE:L← T← 377 . T;
	AECL← L, :REST;

HALFMODE: L← T←  377 . T;
	AECL← L, :REST, T← 0;

REST:	L← DWA + T,TASK;	INCREMENT DWA BY 0 OR NWRDS
NDNX:	DWA← L, :DHT;

DHT1:	L← T← MD+1, BUS=0;
	CBA← L, MAR← T, :MOREB;

NOMORE:	BLOCK, :DNX;
MOREB:	T← 37400;
	L← T← MD AND T, SETMODE;
	MAR← CBA+1, :NORMX, EVENFIELD;

NORMX:	HTAB← L LCY 8, :NODD;
HALFX:	HTAB← L LCY 8, :NEVEN;

NODD:	L←T← 377 . T;
	AECL← L, :XREST;	ODD FIELD, FULL RESOLUTION

NEVEN:	L← 377 AND T;		EVEN FIELD OR HALF RESOLUTION
	AECL←L, T←0;

XREST:	L← MD+T;
	T←MD-1;
DNX:	DWA←L, L←T, TASK;
	SLC←L, :DHT;

;Display Word Task

DWT:	T← DWA;
	T←-3+T+1;
	L← AECL+T,BUS=0,TASK;	AECL CONTAINS NWRDS AT THIS TIME
	AECL←L, :DWTZ;

DWTY:	BLOCK;
	TASK, :DWTF;

DWTZ:	L←HTAB-1, BUS=0,TASK;
	HTAB←L, :DOTAB;

DOTAB:	DDR←0, :DWTZ;
NOTAB:	MAR←T←DWA;
	L←AECL-T-1;
	ALUCY, L←2+T;
	DWA←L, :XNOMORE;

DOMORE:	DDR←MD, TASK;
	DDR←MD, :NOTAB;

XNOMORE:DDR← MD, BLOCK;
	DDR← MD, TASK;

DWTF:	:DWT;

;Alto Ethernet Microcode, Version III, Boggs and Metcalfe

;4-way branches using NEXT6 and NEXT7
!17,20,EIFB00,EODOK,EOEOK,ENOCMD,EIFB01,EODPST,EOEPST,EOREST,EIFB10,EODCOL,EOECOL,EIREST,EIFB11,EODUGH,EOEUGH,ERBRES;

;2-way branches using NEXT7
;EOCDW1, EOCDWX, and EIGO are all related.  Be careful!
!7,10,,EIFOK,,EOCDW1,,EIFBAD,EOCDWX,EIGO;

;Miscellaneous address constraints
!7,10,,EOCDW0,EODATA,EIDFUL,EIDZ4,EOCDRS,EIDATA,EPOST;
!7,10,,EIDOK,,,EIDMOR,EIDPST;
!1,1,EIFB1;
!1,1,EIFRST;

;2-way branches using NEXT9
!1,2,EOINPR,EOINPN;
!1,2,EODMOR,EODEND;
!1,2,EOLDOK,EOLDBD;
!1,2,EIFCHK,EIFPRM;
!1,2,EOCDWT,EOCDGO;
!1,2,ECNTOK,ECNTZR;
!1,2,EIFIGN,EISET;
!1,2,EIFNBC,EIFBC;

;R Memory Locations

$ECNTR	$R12;	Remaining words in buffer
$EPNTR	$R13;	points BEFORE next word in buffer

;Ethernet microcode Status codes

$ESIDON	$377;	Input Done
$ESODON	$777;	Output Done
$ESIFUL	$1377;	Input Buffer full - words lost from tail of packet
$ESLOAD	$1777;	Load location overflowed
$ESCZER	$2377;	Zero word count for input or output command
$ESABRT	$2777;	Abort - usually caused by reset command
$ESNEVR	$3377;	Never Happen - Very bad if it does

;Main memory locations in page 1 reserved for Ethernet

$EPLOC	$600;	Post location
$EBLOC	$601;	Interrupt bit mask

$EELOC	$602;	Ending count location
$ELLOC	$603;	Load location

$EICLOC	$604;	Input buffer Count
$EIPLOC	$605;	Input buffer Pointer

$EOCLOC	$606;	Output buffer Count
$EOPLOC	$607;	Output buffer Pointer

$EHLOC	$610;	Host Address

;Function Definitions

$EIDFCT	$L000000,014004,000100;	BS = 4,	 Input data
$EILFCT	$L016013,070013,000100;	F1 = 13, Input Look
$EPFCT	$L016014,070014,000100;	F1 = 14, Post
$EWFCT	$L016015,000000,000000;	F1 = 15, Wake-Up

$EODFCT	$L026010,000000,124000;	F2 = 10, Output data
$EOSFCT	$L024011,000000,000000;	F2 = 11, Start output
$ERBFCT	$L024012,000000,000000;	F2 = 12, Rest branch
$EEFCT	$L024013,000000,000000;	F2 = 13, End of output
$EBFCT	$L024014,000000,000000;	F2 = 14, Branch
$ECBFCT	$L024015,000000,000000;	F2 = 15, Countdown branch
$EISFCT	$L024016,000000,000000;	F2 = 16, Start input

; - Whenever a label has a pending branch, the list of possible
;   destination addresses is shown in brackets in the comment field.
; - Special functions are explained in a comment near their first use.
; - To avoid naming conflicts, all labels and special functions
;   have "E" as the first letter.

;Top of Ethernet Task loop

;Ether Rest Branch Function - ERBFCT
;merge ICMD and OCMD Flip Flops into NEXT6 and NEXT7
;ICMD and OCMD are set from AC0 [14:15] by the SIO instruction
;	00  neither
;	01  OCMD - Start output
;	10  ICMD - Start input
;	11  Both - Reset interface

;in preparation for a hack at EIREST, zero EPNTR

EREST:	L← 0,ERBFCT;		What's happening ?
	EPNTR← L,:ENOCMD;	[ENOCMD,EOREST,EIREST,ERBRES]

ENOCMD:	L← ESNEVR,:EPOST;	Shouldn't happen
ERBRES:	L← ESABRT,:EPOST;	Reset Command

;Post status and halt.  Microcode status in L.
;Put microstatus,,hardstatus in EPLOC, merge c(EBLOC) into NWW.
;Note that we write EPLOC and read EBLOC in one operation

;Ether Post Function - EPFCT.  Gate the hardware status
;(LOW TRUE) to Bus [10:15], reset interface.

EPOST:	MAR← EELOC;
	EPNTR← L,TASK;		Save microcode status in EPNTR
	MD← ECNTR;		Save ending count

	MAR← EPLOC;		double word reference
	T← NWW;
	MD← EPNTR,EPFCT;	BUS AND EPNTR with Status
	L← MD OR T,TASK;	NWW OR c(EBLOC)
	NWW← L,:EREST;		Done.  Wait for next command

;This is a subroutine called from both input and output (EOCDGO
;and EISET).  The return address is determined by testing ECBFCT,
;which will branch if the buffer has any words in it, which can
;only happen during input.

ESETUP:	NOP;
	L← MD,BUS=0;		check for zero length
	T← MD-1,:ECNTOK;	[ECNTOK,ECNTZR] start-1

ECNTZR:	L← ESCZER,:EPOST;	Zero word count.  Abort

;Ether Countdown Branch Function - ECBFCT.
;NEXT7 = Interface buffer not empty.

ECNTOK:	ECNTR← L,L← T,ECBFCT,TASK;
	EPNTR← L,:EODATA;	[EODATA,EIDATA]

;Ethernet Input

;It turns out that starting the receiver for the first time and
;restarting it after ignoring a packet do the same things.

EIREST:	:EIFIGN;		Hack

;Address filtering code.

;When the first word of a packet is available in the interface
;buffer, a wakeup request is generated.  The microcode then
;decides whether to accept the packet.  Decision must be reached
;before the buffer overflows, within about 14*5.44 usec.
;if EHLOC is zero, machine is 'promiscuous' - accept all packets
;if destination byte is zero, it is a 'broadcast' packet, accept.
;if destination byte equals EHLOC, packet is for us, accept.

;EIFRST is really a subroutine that can be called from EIREST
;or from EIGO, output countdown wait.  If a packet is ignored
;and EPNTR is zero, EIFRST loops back and waits for more
;packets, else it returns to the countdown code.

;Ether Branch Function - EBFCT
;NEXT7 = IDL % OCMD % ICMD % OUTGONE % INGONE (also known as POST)
;NEXT6 = COLLision - Can't happen during input

EIFRST:	MAR← EHLOC;		Get Ethernet address
	T← 377,EBFCT;		What's happening?
	L← MD AND T,BUS=0,:EIFOK;[EIFOK,EIFBAD] promiscuous?

EIFOK:	MTEMP← LLCY8,:EIFCHK;	[EIFCHK,EIFPRM] Data wakeup

EIFBAD:	ERBFCT,TASK,:EIFB1;	[EIFB1] POST wakeup; xCMD FF set?
EIFB1:	:EIFB00;		[EIFB00,EIFB01,EIFB10,EIFB11]

EIFB00:	:EIFIGN;		IDL or INGONE, restart rcvr
EIFB01:	L← ESABRT,:EPOST;	OCMD, abort
EIFB10:	L← ESABRT,:EPOST;	ICMD, abort
EIFB11:	L← ESABRT,:EPOST;	ICMD and OCMD, abort

EIFPRM:	TASK,:EIFBC;		Promiscuous. Accept

;Ether Look Function - EILFCT.  Gate the first word of the
;data buffer to the bus, but do not increment the read pointer.

EIFCHK:	L← T← 177400,EILFCT;	Mask off src addr byte (BUS AND)
	L← MTEMP-T,SH=0;	Broadcast?
	SH=0,TASK,:EIFNBC;	[EIFNBC,EIFBC] Our Address?

EIFNBC:	:EIFIGN;		[EIFIGN,EISET]

EIFBC:	:EISET;			[EISET] Enter input main loop

;Ether Input Start Function - EISFCT.  Start receiver.  Interface
;will generate a data wakeup when the first word of the next
;packet arrives, ignoring any packet currently passing.

EIFIGN:	SINK← EPNTR,BUS=0,EPFCT;Reset; Called from output?
	EISFCT,TASK,:EOCDWX;	[EOCDWX,EIGO] Restart rcvr

EOCDWX:	EWFCT,:EOCDWT;		Return to countdown wait loop

EISET:	MAR← EICLOC,:ESETUP;	Double word reference

;Input Main Loop

;Ether Input Data Function - EIDFCT.  Gate a word of data to
;the bus from the interface data buffer, increment the read ptr.
;		* * * * * W A R N I N G * * * * *
;The delay from decoding EIDFCT to gating data to the bus is
;marginal.  Some logic in the interface detects the situation
;(which only happens occasionally) and stops SysClk for one cycle.
;Since memory data must be available during cycle 4, and SysClk
;may stop for one cycle, this means that the MD← EIDFCT must
;happen in cycle 3.  There is a bug in this logic which occasionally
;stops the clock in the instruction following the EIDFCT, so
;the EIDFCT instruction should not be the last one of the task,
;or it may screw up someone else (such as RDRAM).

;EIDOK, EIDMOR, and EIDPST must have address bits in the pattern:
;xxx1   xxx4        xxx5
;ECBFCT is used to force an unconditional branch on NEXT7

EIDATA:	T← ECNTR-1, BUS=0;
	MAR← L← EPNTR+1, EBFCT;	[EIDMOR,EIDPST] What's happening
EIDMOR:	EPNTR← L, L← T, ECBFCT;	[EIDOK,EIDPST] Guaranteed to branch
EIDOK:	MD← EIDFCT, TASK;	[EIDZ4] Read a word from the interface
EIDZ4:	ECNTR← L, :EIDATA;

; We get to EIDPST for one of two reasons:
; (1) The buffer is full.  In this case, an EBFCT (NEXT[7]) is pending.
;     We want to post "full" if this is a normal data wakeup (no branch)
;     but just "input done" if hardware input terminated (branch).
; (2) Hardware input terminated while the buffer was not full.
;     In this case, an unconditional branch on NEXT[7] is pending, so
;     we always terminate with "input done".
EIDPST:	L← ESIDON, :EIDFUL;	[EIDFUL,EPOST] Presumed to be INGONE
EIDFUL:	L← ESIFUL, :EPOST;	Input buffer overrun

;Ethernet output

;It is possible to get here due to a collision.  If a collision
;happened, the interface was reset (EPFCT) to shut off the
;transmitter.  EOSFCT is issued to guarantee more wakeups while
;generating the countdown.  When this is done, the interface is
;again reset, without really doing an output.

EOREST:	MAR← ELLOC;		Get load
	L← R37;			Use clock as random # gen
	EPNTR← LRSH1;		Use bits [6:13]
	L← MD,EOSFCT;		L← current load
	SH<0,ECNTR← L;		Overflowed?
	MTEMP← LLSH1,:EOLDOK;	[EOLDOK,EOLDBD]

EOLDBD:	L← ESLOAD,:EPOST;	Load overlow

EOLDOK:	L← MTEMP+1;		Write updated load
	MAR← ELLOC;
	MTEMP← L,TASK;
	MD← MTEMP,:EORST1;	New load = (old lshift 1) + 1

EORST1:	L← EPNTR;		Continue making random #
	EPNTR← LRSH1;
	T← 377;
	L← EPNTR AND T,TASK;
	EPNTR← L,:EORST2;

;At this point, EPNTR has 0,,random number, ENCTR has old load.

EORST2:	MAR← EICLOC;		Has an input buffer been set up?
	T← ECNTR;
	L← EPNTR AND T;		L← Random & Load
	SINK← MD,BUS=0;
	ECNTR← L,SH=0,EPFCT,:EOINPR;[EOINPR,EOINPN]

EOINPR:	EISFCT,:EOCDWT;		[EOCDWT,EOCDGO] Enable in under out

EOINPN:	:EOCDWT;		[EOCDWT,EOCDGO] No input.

;Countdown wait loop.  MRT will generate a wakeup every
;37 usec which will decrement ECNTR.  When it is zero, start
;the transmitter.

;Ether Wake Function - EWFCT.  Sets a flip flop which will cause
;a wakeup to this task the next time MRT wakes up (every 37 usec).
;Wakeup is cleared when Ether task next runs.  EWFCT must be
;issued in the instruction AFTER a task.

EOCDWT:	L← 177400,EBFCT;	What's happening?
	EPNTR← L,ECBFCT,:EOCDW0;[EOCDW0,EOCDRS] Packet coming in?
EOCDW0:	L← ECNTR-1,BUS=0,TASK,:EOCDW1; [EOCDW1,EIGO]
EOCDW1:	ECNTR← L,EWFCT,:EOCDWT;	[EOCDWT,EOCDGO]

EOCDRS:	L← ESABRT,:EPOST;	[EPOST] POST event

EIGO:	:EIFRST;		[EIFRST] Input under output

;Output main loop setup

EOCDGO:	MAR← EOCLOC;		Double word reference
	EPFCT;			Reset interface
	EOSFCT,:ESETUP;		Start Transmitter

;Ether Output Start Function - EOSFCT.  The interface will generate
;a burst of data requests until the interface buffer is full or the
;memory buffer is empty, wait for silence on the Ether, and begin
;transmitting.  Thereafter it will request a word every 5.44 us.

;Ether Output Data Function - EODFCT.  Copy the bus into the
;interface data buffer, increment the write pointer, clears wakeup
;request if the buffer is now nearly full (one slot available).

;Output main loop

EODATA:	L← MAR← EPNTR+1,EBFCT;	What's happening?
	T← ECNTR-1,BUS=0,:EODOK; [EODOK,EODPST,EODCOL,EODUGH]
EODOK:	EPNTR← L,L← T,:EODMOR;	[EODMOR,EODEND]
EODMOR:	ECNTR← L,TASK;
	EODFCT← MD,:EODATA;	Output word to transmitter

EODPST:	L← ESABRT,:EPOST;	[EPOST] POST event

EODCOL:	EPFCT,:EOREST;		[EOREST] Collision

EODUGH:	L← ESABRT,:EPOST;	[EPOST] POST + Collision

;Ether EOT Function - EEFCT.  Stop generating output data wakeups,
;the interface has all of the packet.  When the data buffer runs
;dry, the interface will append the CRC and then generate an
;OUTGONE post wakeup.

EODEND:	EEFCT;			Disable data wakeups
	TASK;			Wait for EEFCT to take
	:EOEOT;			Wait for Outgone

;Output completion.  We are waiting for the interface buffer to
;empty, and the interface to generate an OUTGONE Post wakeup.

EOEOT:	EBFCT;			What's happening?
	:EOEOK;			[EOEOK,EOEPST,EOECOL,EOEUGH]

EOEOK:	L← ESNEVR,:EPOST;	Runaway Transmitter. Never Never.

EOEPST:	L← ESODON,:EPOST;	POST event.  Output done

EOECOL:	EPFCT,:EOREST;		Collision

EOEUGH:	L← ESABRT,:EPOST;	POST + Collision


;Memory Refresh Task,
;Mouse Handler,
;EIA Handler,
;Interval Timer,
;Calender Clock, and
;part of the cursor.

!17,20,TX0,TX6,TX3,TX2,TX8,TX5,TX1,TX7,TX4,,,,,,,;
!1,2,DOTIMER,NOTIMER;
!1,2,NOTIMERINT,TIMERINT;
!1,2,DOCUR,NOCUR;
!1,2,SHOWC,WAITC;
!1,2,SPCHK,NOSPCHK;

!1,2,NOCLK,CLOCK;
!1,1,MRTLAST;
!1,2,CNOTLAST,CLAST;

$CLOCKTEMP	$R11;
$REFIIMSK	$7777;

;		* * * A T T E N T I O N * * *
;There are two versions of the Memory refresh code:
;	AltoIIMRT4K.mu 		for refreshing 4K chips
;	AltoIIMRT16K.mu		for refreshing 16K chips
;You must name one or the other 'AltoIIMRT.mu'.
;I suggest the following convention for naming the resulting .MB file:
;	AltoIICode3.MB for the 4K version
;	AltoIICode3XM.MB for the 16K version

#AltoIIMRT.mu;

CLOCK:	MAR← CLOCKLOC;		R37 OVERFLOWED.
	NOP;
	L← MD+1;		INCREMENT CLOCK IM MEMORY
	MAR← CLOCKLOC;
	MTEMP← L, TASK;
	MD← MTEMP, :NOCLK;

DOCUR:	L← T← YPOS;		CHECK FOR VISIBLE CURSOR ON THIS SCAN
	SH<0, L← 20-T-1;	 ***x13 change: the constant 20 was 17
	SH<0, L← 2+T, :SHOWC;	[SHOWC,WAITC]

WAITC:	YPOS← L, L← 0, TASK, :MRTLAST;	SQUASHES PENDING BRANCH
SHOWC:	MAR← CLOCKLOC+T+1, :CNOTLAST;

CNOTLAST: T← CURX, :CURF;
CLAST:	T← 0;
CURF:	YPOS← L, L← T;
	CURX← L;
	L← MD, TASK;
	CURDATA← L, :MRT;

;AFTER THIS DISPATCH, T WILL CONTAIN XCHANGE, L WILL CONTAIN YCHANGE-1

TX1:	L← T← ONE +T, :M00;		Y=0, X=1
TX2:	L← T← ALLONES, :M00;		Y=0, X=-1
TX3:	L← T← 0, :M00;			Y=1, X=0
TX4:	L← T← ONE AND T, :M00;		Y=1, X=1
TX5:	L← T← ALLONES XOR T, :M00;	Y=1, X=-1
TX6:	T← 0, :M00;			Y=-1, X=0
TX7:	T← ONE, :M00;			Y=-1, X=1
TX8:	T← ALLONES, :M00;		Y=-1, X=-1

M00:	MAR← MOUSELOC;			START THE FETCH OF THE COORDINATES
	MTEMP← L;			YCHANGE -1
	L← MD+ T;			X+ XCHANGE
	T← MD;				Y
	T← MTEMP+ T+1;			Y+ (YCHANGE-1) + 1
	MTEMP← L, L← T;
	MAR← MOUSELOC;			NOW RESTORE THE UPDATED COORDINATES
	CLOCKTEMP← L;
	MD← MTEMP, TASK;
	MD← CLOCKTEMP, :MRTA;


;CURSOR TASK

;Cursor task specific functions
$XPREG		$L026010,000000,124000; F2 = 10
$CSR		$L026011,000000,124000; F2 = 11

CURT:	XPREG← CURX, TASK;
	CSR← CURDATA, :CURT;


;PREDEFINITION FOR PARITY TASK.
;THE CODE IS AT THE END OF THE FILE
!17,20,PR0,,PR2,PR3,PR4,PR5,PR6,PR7,PR8,,,,,,,;

;NOVA EMULATOR

$SAD	$R5;
$PC	$R6;		USED BY MEMORY INIT


!7,10,Q0,Q1,Q2,Q3,Q4,Q5,Q6,Q7;
!1,2,FINSTO,INCPC;
!1,2,EReRead,FINJMP;		***X21 addition.
!1,2,EReadDone,EContRead;	***X21 addition.
!1,2,EtherBoot,DiskBoot;	***X21 addition.

NOVEM:	IR←L←MAR←0, :INXB,SAD← L;  LOAD SAD TO ZERO THE BUS. STORE PC AT 0
Q0:	L← ONE, :INXA;		EXECUTED TWICE
Q1:	L← TOTUWC, :INXA;
Q2:	L←402, :INXA;		FIRST READ HEADER INTO 402, THEN
Q3:	L← 402, :INXA;		STORE LABEL AT 402
Q4:	L← ONE, :INXA;		STORE DATA PAGE STARTING AT 1
Q5:	L←377+1, :INXE;		Store Ethernet Input Buffer Length ***X21.
Q6:	L←ONE, :INXE;		Store Ethernet Input Buffer Pointer ***X21.
Q7:	MAR← DASTART;		CLEAR THE DISPLAY POINTER
	L← 0;
	R37← L;
	MD← 0;
	MAR← 177034;		FETCH KEYBOARD
	L← 100000;
	NWW← L, T← 0-1;
	L← MD XOR T, BUSODD;	*** X21 change.
	MAR← BDAD, :EtherBoot;	[EtherBoot, DiskBoot]  *** X21 change.
				; BOOT DISK ADDRESS GOES IN LOCATION 12
DiskBoot: SAD← L, L← 0+1;
	MD← SAD;
	MAR← KBLKADR, :FINSTO;


; Ethernet boot section added in X21.
$NegBreathM1	$177175;
$EthNovaGo	$3;	First data location of incoming packet

EtherBoot: L←EthNovaGo, :EReRead; [EReRead, FINJMP]

EReRead:MAR← EHLOC;	Set the host address to 377 for breath packets
	TASK;
	MD← 377;

	MAR← EPLOC;	Zero the status word and start 'er up
	SINK← 2, STARTF;
	MD ← 0;

EContRead: MAR← EPLOC;	See if status is still 0
	T← 377;		Status for correct read
	L← MD XOR T, TASK, BUS=0;
	SAD← L, :EReadDone; [EReadDone, EContRead]

EReadDone: MAR← 2;	Check the packet type
	T← NegBreathM1;	-(Breath-of-life)-1
	T←MD+T+1;
	L←SAD OR T;
	SH=0, :EtherBoot;


; SUBROUTINE USED BY INITIALIZATION TO SET UP BLOCKS OF MEMORY
$EIOffset	$576;

INXA:	T←ONE, :INXCom;		***X21 change.
INXE:	T←EIOffset, :INXCom;	***X21 addition.

INXCom: MAR←T←IR← SAD+T;	*** X21 addition.
	PC← L, L← 0+T+1;	*** X21 change.
INXB:	MD← PC;
	SINK← DISP, BUS,TASK;
	SAD← L, :Q0;


;REGISTERS USED BY NOVA EMULATOR
$AC0	$R3;	AC'S ARE BACKWARDS BECAUSE THE HARDWARE SUPPLIES THE
$AC1	$R2;	COMPLEMENT ADDRESS WHEN ADDRESSING FROM IR
$AC2	$R1;
$AC3	$R0;
$XREG	$R7;


;PREDEFINITIONS FOR NOVA

!17,20,GETAD,G1,G2,G3,G4,G5,G6,G7,G10,G11,G12,G13,G14,G15,G16,G17;
!17,20,XCTAB,XJSR,XISZ,XDSZ,XLDA,XSTA,CONVERT,,,,,,,,,;
!3,4,SHIFT,SH1,SH2,SH3;
!1,2,MAYBE,NOINT;
!1,2,DOINT,DIS0;
!1,2,SOMEACTIVE,NOACTIVE;
!1,2,IEXIT,NIEXIT;
!17,1,ODDCX;
!1,2,EIR0,EIR1;
!7,1,INTCODE;
!1,2,INTSOFF,INTSON;	***X21 addition for DIRS
!7,10,EMCYCRET,RAMCYCRET,CYX2,CYX3,CYX4,CONVCYCRET,,;
!7,2,MOREBLT,FINBLT;
!1,2,DOIT,DISABLED;

; ALL INSTRUCTIONS RETURN TO START WHEN DONE

START:	T← MAR←PC+SKIP;
START1:	L← NWW, BUS=0;	BUS# 0 MEANS DISABLED OR SOMETHING TO DO
	:MAYBE, SH<0, L← 0+T+1;  	SH<0 MEANS DISABLED
MAYBE:	PC← L, L← T, :DOINT;
NOINT:	PC← L, :DIS0;

DOINT:	MAR← WWLOC, :INTCODE;	TRY TO CAUSE AN INTERRUPT

;DISPATCH ON FUNCTION FIELD IF ARITHMETIC INSTRUCTION,
;OTHERWISE ON INDIRECT BIT AND INDEX FIELD

DIS0:	L← T← IR← MD;	SKIP CLEARED HERE

;DISPATCH ON SHIFT FIELD IF ARITHMETIC INSTRUCTION,
;OTHERWISE ON THE INDIRECT BIT OR IR[3-7]

DIS1:	T← ACSOURCE, :GETAD;

;GETAD MUST BE 0 MOD 20
GETAD: T← 0, :DOINS;			PAGE 0
G1:	T← PC -1, :DOINS;		RELATIVE
G2:	T← AC2, :DOINS;			AC2 RELATIVE
G3:	T← AC3, :DOINS;			AC3 RELATIVE
G4:	T← 0, :DOINS;			PAGE 0 INDIRECT
G5:	T← PC -1, :DOINS;		RELATIVE INDIRECT
G6:	T← AC2, :DOINS;			AC2 RELATIVE INDIRECT
G7:	T← AC3, :DOINS;			AC3 RELATIVE INDIRECT
G10:	L← 0-T-1, TASK, :SHIFT;		COMPLEMENT
G11:	L← 0-T, TASK, :SHIFT;		NEGATE
G12:	L← 0+T, TASK, :SHIFT;		MOVE
G13:	L← 0+T+1, TASK, :SHIFT;		INCREMENT
G14:	L← ACDEST-T-1, TASK, :SHIFT;	ADD COMPLEMENT
G15:	L← ACDEST-T, TASK, :SHIFT;	SUBTRACT
G16:	L← ACDEST+T, TASK, :SHIFT;	ADD
G17:	L← ACDEST AND T, TASK, :SHIFT;

SHIFT:	DNS← L LCY 8, :START; 	SWAP BYTES
SH1:	DNS← L RSH 1, :START;	RIGHT 1
SH2:	DNS← L LSH 1, :START;	LEFT 1
SH3:	DNS← L, :START;		NO SHIFT

DOINS:	L← DISP + T, TASK, :SAVAD, IDISP;	DIRECT INSTRUCTIONS
DOIND:	L← MAR← DISP+T;				INDIRECT INSTRUCTIONS
	XREG← L;
	L← MD, TASK, IDISP, :SAVAD;

BRI:	L← MAR← PCLOC	;INTERRUPT RETURN BRANCH
BRI0:	T← 77777;
	L← NWW AND T, SH < 0;
	NWW← L, :EIR0;	BOTH EIR AND BRI MUST CHECK FOR INTERRUPT
;			REQUESTS WHICH MAY HAVE COME IN WHILE
;			INTERRUPTS WERE OFF

EIR0:	L← MD, :DOINT;
EIR1:	L← PC, :DOINT;

;***X21 addition
; DIRS - 61013 - Disable Interrupts and Skip if they were On
DIRS:	T←100000;
	L←NWW AND T;
	L←PC+1, SH=0;

; DIR - 61000 - Disable Interrupts
DIR:	T← 100000, :INTSOFF;
INTSOFF: L← NWW OR T, TASK, :INTZ;

INTSON: PC←L, :INTSOFF;

;EIR - 61001 - Enable Interrupts
EIR:	L← 100000, :BRI0;

;SIT - 61007 - Start Interval Timer
SIT:	T← AC0;
	L← R37 OR T, TASK;
	R37← L, :START;


FINJSR:	L← PC;
	AC3← L, L← T, TASK;
FINJMP:	PC← L, :START;
SAVAD:	SAD← L, :XCTAB;

;JSRII - 64400 - JSR double indirect, PC relative.  Must have X=1 in opcode
;JSRIS - 65000 - JSR double indirect, AC2 relative.  Must have X=2 in opcode
JSRII:	MAR← DISP+T;	FIRST LEVEL
	IR← JSRCX;	<JSR 0>
	T← MD, :DOIND;	THE IR← INSTRUCTION WILL NOT BRANCH


;TRAP ON UNIMPLEMENTED OPCODES.  SAVES  PC AT
;TRAPPC, AND DOES A JMP@ TRAPVEC ! OPCODE.
TRAP:	XREG← L LCY 8;	THE INSTRUCTION
TRAP1:	MAR← TRAPPC;***X13 CHANGE: TAG 'TRAP1' ADDED
	IR← T← 37;
	MD← PC;
	T← XREG.T;
	T← TRAPCON+T+1, :DOIND;	T NOW CONTAINS 471+OPCODE
;				THIS WILL DO JMP@ 530+OPCODE

;***X21 CHANGE: ADDED TAG RAMTRAP
RAMTRAP: SWMODE, :TRAP;

; Parameterless operations come here for dispatch.

!1,2,NPNOTRAP,NPTRAP;

NOPAR:	XREG←L LCY 8;	***X21 change. Checks < 27.
	T←27;		***IIX3. Greatest defined op is 26.
	L←DISP-T;
	ALUCY;
	SINK←DISP, SINK←X37, BUS, TASK, :NPNOTRAP;

NPNOTRAP: :DIR;

NPTRAP: :TRAP1;

;***X21 addition for debugging w/ expanded DISP Prom
U5:	:RAMTRAP;
U6:	:RAMTRAP;
U7:	:RAMTRAP;

;MAIN INSTRUCTION TABLE.  GET HERE:
;		(1) AFTER AN INDIRECTION
;		(2) ON DIRECT INSTRUCTIONS

XCTAB:	L← SAD, TASK, :FINJMP;	JMP
XJSR:	T← SAD, :FINJSR;	JSR
XISZ:	MAR← SAD, :ISZ1;	ISZ
XDSZ:	MAR← SAD, :DSZ1;	DSZ
XLDA:	MAR← SAD, :FINLOAD;	LDA 0-3
XSTA:	MAR← SAD;		/*NORMAL
XSTA1:	L← ACDEST, :FINSTO;	/*NORMAL

;	BOUNDS-CHECKING VERSION OF STORE
;	SUBST ";**<CR>" TO "<CR>;**" TO ENABLE THIS CODE:
;**	!1,2,XSTA1,XSTA2;
;**	!1,2,DOSTA,TRAPSTA;
;**XSTA:	MAR← 10;	LOCS 10,11 CONTAINS HI,LO BOUNDS
;**	T← SAD
;**	L← MD-T;	HIGHBOUND-ADDR
;**	T← MD, ALUCY;
;**	L← SAD-T, :XSTA1;	ADDR-LOWBOUND
;**XSTA1:	TASK, :XSTA3;
;**XSTA2:	ALUCY, TASK;
;**XSTA3:	L← 177, :DOSTA;
;**TRAPSTA:	XREG← L, :TRAP1;	CAUSE A SWAT
;**DOSTA:	MAR← SAD;	DO THE STORE NORMALLY
;**	L← ACDEST, :FINSTO;
;**

DSZ1:	T← ALLONES, :FINISZ;
ISZ1:	T← ONE, :FINISZ;

FINSTO:	SAD← L,TASK;
FINST1:	MD←SAD, :START;

FINLOAD: NOP;
LOADX:	L← MD, TASK;
LOADD:	ACDEST← L, :START;

FINISZ:	L← MD+T;
	MAR← SAD, SH=0;
	SAD← L, :FINSTO;

INCPC:	MD← SAD;
	L← PC+1, TASK;
	PC← L, :START;

;DIVIDE.  THIS DIVIDE IS IDENTICAL TO THE NOVA DIVIDE EXCEPT THAT
;IF THE DIVIDE CANNOT BE DONE, THE INSTRUCTION FAILS TO SKIP, OTHERWISE
;IT DOES.  CARRY IS UNDISTURBED.

!1,2,DODIV,NODIV;
!1,2,DIVL,ENDDIV;
!1,2,NOOVF,OVF;
!1,2,DX0,DX1;
!1,2,NOSUB,DOSUB;

DIV:	T← AC2;
DIVX:	L← AC0 - T;	DO THE DIVIDE ONLY IF AC2>AC0
	ALUCY, TASK, SAD← L, L← 0+1;
	:DODIV, SAD← L LSH 1;		SAD← 2.  COUNT THE LOOP BY SHIFTING

NODIV:	:FINBLT;		***X21 change.
DODIV:	L← AC0, :DIV1;

DIVL:	L← AC0;
DIV1:	SH<0, T← AC1;	WILL THE LEFT SHIFT OF THE DIVIDEND OVERFLOW?
	:NOOVF, AC0← L MLSH 1, L← T← 0+T;	L← AC1, T← 0

OVF:	AC1← L LSH 1, L← 0+INCT, :NOV1;		L← 1. SHIFT OVERFLOWED
NOOVF:	AC1← L LSH 1 , L← T;			L← 0. SHIFT OK

NOV1:	T← AC2, SH=0;
	L← AC0-T, :DX0;

DX1:	ALUCY;		DO THE TEST ONLY IF THE SHIFT DIDN'T OVERFLOW.  IF
;			IT DID, L IS STILL CORRECT, BUT THE TEST WOULD GO
;			THE WRONG WAY.
	:NOSUB, T← AC1;

DX0:	:DOSUB, T← AC1;

DOSUB:	AC0← L, L← 0+INCT;	DO THE SUBTRACT
	AC1← L;			AND PUT A 1 IN THE QUOTIENT

NOSUB:	L← SAD, BUS=0, TASK;
	SAD← L LSH 1, :DIVL;

ENDDIV:	L← PC+1, TASK, :DOIT; ***X21 change. Skip if divide was done.


;MULTIPLY.  THIS IS AN EXACT EMULATION OF NOVA HARDWARE MULTIPLY.
;AC2 IS THE MULTIPLIER, AC1 IS THE MULTIPLICAND.
;THE PRODUCT IS IN AC0 (HIGH PART), AND AC1 (LOW PART).
;PRECISELY: AC0,AC1 ← AC1*AC2  + AC0

!1,2,DOMUL,NOMUL;
!1,2,MPYL,MPYA;
!1,2,NOADDIER,ADDIER;
!1,2,NOSPILL,SPILL;
!1,2,NOADDX,ADDX;
!1,2,NOSPILLX,SPILLX;


MUL:	L← AC2-1, BUS=0;
MPYX:	XREG←L,L← 0, :DOMUL;	GET HERE WITH AC2-1 IN L. DON'T MUL IF AC2=0
DOMUL:	TASK, L← -10+1;
	SAD← L;		COUNT THE LOOP IN SAD

MPYL:	L← AC1, BUSODD;
	T← AC0, :NOADDIER;

NOADDIER: AC1← L MRSH 1, L← T, T← 0, :NOSPILL;
ADDIER:	L← T← XREG+INCT;
	L← AC1, ALUCY, :NOADDIER;

SPILL:	T← ONE;
NOSPILL: AC0← L MRSH 1;
	L← AC1, BUSODD;
	T← AC0, :NOADDX;

NOADDX:	AC1← L MRSH 1, L← T, T← 0, :NOSPILLX;
ADDX:	L← T← XREG+ INCT;
	L← AC1,ALUCY, :NOADDX;

SPILLX:	T← ONE;
NOSPILLX: AC0← L MRSH 1;
	L← SAD+1, BUS=0, TASK;
	SAD← L, :MPYL;

NOMUL:	T← AC0;
	AC0← L, L← T, TASK;	CLEAR AC0
	AC1← L;			AND REPLACE AC1 WITH AC0
MPYA:	:FINBLT;		***X21 change.

;CYCLE AC0 LEFT BY DISP MOD 20B, UNLESS DISP=0, IN WHICH
;CASE CYCLE BY AC1 MOD 20B
;LEAVES AC1=CYCLE COUNT-1 MOD 20B

$CYRET		$R5;	Shares space with SAD.
$CYCOUT		$R7;	Shares space with XREG.

!1,2,EMCYCX,ACCYCLE;
!1,1,Y1;
!1,1,Y2;
!1,1,Y3;
!1,1,Z1;
!1,1,Z2;
!1,1,Z3;

EMCYCLE: L← DISP, SINK← X17, BUS=0;	CONSTANT WITH BS=7
CYCP:	T← AC0, :EMCYCX;

ACCYCLE: T← AC1;
	L← 17 AND T, :CYCP;

EMCYCX: CYCOUT←L, L←0, :RETCYCX;

RAMCYCX: CYCOUT←L, L←0+1;

RETCYCX: CYRET←L, L←0+T;
	SINK←CYCOUT, BUS;
	TASK, :L0;

;TABLE FOR CYCLE
R4:	CYCOUT← L MRSH 1;
Y3:	L← T← CYCOUT, TASK;
R3X:	CYCOUT← L MRSH 1;
Y2:	L← T← CYCOUT, TASK;
R2X:	CYCOUT← L MRSH 1;
Y1:	L← T← CYCOUT, TASK;
R1X:	CYCOUT← L MRSH 1, :ENDCYCLE;

L4:	CYCOUT← L MLSH 1;
Z3:	L← T← CYCOUT, TASK;
L3:	CYCOUT← L MLSH 1;
Z2:	L← T← CYCOUT, TASK;
L2:	CYCOUT← L MLSH 1;
Z1:	L← T← CYCOUT, TASK;
L1:	CYCOUT← L MLSH 1, :ENDCYCLE;
L0:	CYCOUT← L, :ENDCYCLE;

L8:	CYCOUT← L LCY 8, :ENDCYCLE;
L7:	CYCOUT← L LCY 8, :Y1;
L6:	CYCOUT← L LCY 8, :Y2;
L5:	CYCOUT← L LCY 8, :Y3;

R7:	CYCOUT← L LCY 8, :Z1;
R6:	CYCOUT← L LCY 8, :Z2;
R5:	CYCOUT← L LCY 8, :Z3;

ENDCYCLE: SINK← CYRET, BUS, TASK;
	:EMCYCRET;

EMCYCRET: L←CYCOUT, TASK, :LOADD;

RAMCYCRET: T←PC, BUS, SWMODE, :TORAM;

; Scan convert instruction for characters. Takes DWAX (Destination
; word address)-NWRDS in AC0, and a pointer to a .AL-format font
; in AC3. AC2+displacement contains a pointer to a two-word block
; containing NWRDS and DBA (Destination Bit Address).

$XH		$R10;
$DWAX		$R35;
$MASK		$R36;

!1,2,HDLOOP,HDEXIT;
!1,2,MERGE,STORE;
!1,2,NFIN,FIN;
!17,2,DOBOTH,MOVELOOP;

CONVERT: MAR←XREG+1;	Got here via indirect mechanism which
;			left first arg in SAD, its address in XREG.
	T←17;
	L←MD AND T;

	T←MAR←AC3;
	AC1←L;		AC1←DBA&#17
	L←MD+T, TASK;
	AC3←L;		AC3←Character descriptor block address(Char)

	MAR←AC3+1;
	T←177400;
	IR←L←MD AND T;		IR←XH
	XH←L LCY 8, :ODDCX;	XH register temporarily contains HD
ODDCX:	L←AC0, :HDENTER;

HDLOOP: T←SAD;			(really NWRDS)
	L←DWAX+T;

HDENTER: DWAX←L;		DWAX ← AC0+HD*NWRDS
	L←XH-1, BUS=0, TASK;
	XH←L, :HDLOOP;

HDEXIT:	T←MASKTAB;
	MAR←T←AC1+T;		Fetch the mask.
	L←DISP;
	XH←L;			XH register now contains XH
	L←MD;
	MASK←L, L←0+T+1, TASK;
	AC1←L;			***X21. AC1 ← (DBA&#17)+1

	L←5;			***X21. Calling conventions changed.
	IR←SAD, TASK;
	CYRET←L, :MOVELOOP;	CYRET←CALL5

MOVELOOP: L←T←XH-1, BUS=0;
	MAR←AC3-T-1, :NFIN;	Fetch next source word
NFIN:	XH←L;
	T←DISP;			(really NWRDS)
	L←DWAX+T;		Update destination address
	T←MD;
	SINK←AC1, BUS;
	DWAX←L, L←T, TASK, :L0;	Call Cycle subroutine

CONVCYCRET: MAR←DWAX;
	T←MASK, BUS=0;
	T←CYCOUT.T, :MERGE;	Data for first word. If MASK=0
				; then store the word rather than
				; merging, and do not disturb the
				; second word.
MERGE:	L←XREG AND NOT T;	Data for second word.
	T←MD OR T;		First word now merged,
	XREG←L, L←T;
	MTEMP←L;
	MAR←DWAX;			restore it.
	SINK←XREG, BUS=0, TASK;
	MD←MTEMP, :DOBOTH;	XREG=0 means only one word
				; is involved.

DOBOTH: MAR←DWAX+1;
	T←XREG;
	L←MD OR T;
	MAR←DWAX+1;
	XREG←L, TASK;		***X21. TASK added.
STORE:	MD←XREG, :MOVELOOP;

FIN:	L←AC1-1;		***X21. Return AC1 to DBA&#17.
	AC1←L;			*** ... bletch ...
	IR←SH3CONST;
	L←MD, TASK, :SH1;

;RCLK - 61003 - Read the Real Time Clock into AC0,AC1
RCLK:	MAR← CLOCKLOC;
	L← R37;
	AC1← L, :LOADX;

;SIO - 61004 - Put AC0 on the bus, issue STARTF to get device attention,
;Read Host address from Ethernet interface into AC0.
SIO:	L← AC0, STARTF;
	T← 77777;		***X21 sets AC0[0] to 0
	L← RSNF AND T;
LTOAC0:	AC0← L, TASK, :TOSTART;

;EngNumber is a constant returned by VERS that contains a discription
;of the Alto and it's Microcode. The composition of EngNumber is:
;	bits 0-3	Alto engineering number
;	bits 4-7	Alto build
;	bits 8-15	Version number of Microcode
;Use of the Alto Build number has been abandoned.
;the engineering number (EngNumber) is in the MRT files because it
; it different for Altos with and without Extended memory.
VERS:	T← EngNumber;		***V3 change
	L← 3+T, :LTOAC0;	***V3 change

;XMLDA - Extended Memory Load Accumulator.
;	AC0 ← @AC1 in the alternate bank
XMLDA:	XMAR← AC1, :FINLOAD;	***V3 change

;XMSTA - Extended Memory Store Accumulator
;	@AC1 ← AC0 in the alternate bank
XMSTA:	XMAR← AC1, :XSTA1;	***V3 change

;BLT - 61005 - Block Transfer
;BLKS - 61006 - Block Store
; Accepts in
;	AC0/ BLT: Address of first word of source block-1
;	     BLKS: Data to be stored
;	AC1/ Address of last word of destination block
;	AC3/ NEGATIVE word count
; Leaves
;	AC0/ BLT: Address of last word of source block+1
;	     BLKS: Unchanged
;	AC1/ Unchanged
;	AC2/ Unchanged
;	AC3/ 0
; These instructions are interruptable.  If an interrupt occurs,
; the PC is decremented by one, and the ACs contain the intermediate
; so the instruction can be restarted when the interrupt is dismissed.

!1,2,PERHAPS, NO;

BLT:	L← MAR← AC0+1;
	AC0← L;
	L← MD, :BLKSA;

BLKS:	L← AC0;
BLKSA:	T← AC3+1, BUS=0;
	MAR← AC1+T, :MOREBLT;

MOREBLT: XREG← L, L← T;
	AC3← L, TASK;
	MD← XREG;		STORE
	L← NWW, BUS=0;		CHECK FOR INTERRUPT
	SH<0, :PERHAPS, L← PC-1;	Prepare to back up PC.

NO:	SINK← DISP, SINK← M7, BUS, :DISABLED;

PERHAPS: SINK← DISP, SINK← M7, BUS, :DOIT;

DOIT:	PC←L, :FINBLT;	***X21. Reset PC, terminate instruction.

DISABLED: :DIR;	GOES TO BLT OR BLKS

FINBLT:	T←777;	***X21. PC in [177000-177777] means Ram return
	L←PC+T+1;
	L←PC AND T, TASK, ALUCY;
TOSTART: XREG←L, :START;

RAMRET: T←XREG, BUS, SWMODE;
TORAM:	:NOVEM;

;PARAMETERLESS INSTRUCTIONS FOR DIDDLING THE WCS.

;JMPRAM - 61010 - JUMP TO THE RAM ADDRESS SPECIFIED BY AC1
JMPR:	T←AC1, BUS, SWMODE, :TORAM;


;RDRAM - 61011 - READ THE RAM WORD ADDRESSED BY AC1 INTO AC0
RDRM:	T← AC1, RDRAM;
	L← ALLONES, TASK, :LOADD;


;WRTRAM - 61012 - WRITE AC0,AC3 INTO THE RAM LOCATION ADDRESSED BY AC1
WTRM:	T← AC1;
	L← AC0, WRTRAM;
	L← AC3, :FINBLT;

;DOUBLE WORD INSTRUCTIONS

;DREAD - 61015
;	AC0← rv(AC3); AC1← rv(AC3 xor 1)

DREAD:	MAR← AC3;		START MEMORY CYCLE
	NOP;			DELAY
DREAD1:	L← MD;			FIRST READ
	T←MD;			SECOND READ
	AC0← L, L←T, TASK;	STORE MSW
	AC1← L, :START;		STORE LSW


;DWRITE - 61016
;	rv(AC3)← AC0; rv(AC3 xor 1)← AC1

DWRITE:	MAR← AC3;		START MEMORY CYCLE
	NOP;			DELAY
	MD← AC0, TASK;		FIRST WRITE
	MD← AC1, :START;	SECOND WRITE


;DEXCH - 61017
;	t← rv(AC3); rv(AC3)← AC0; AC0← t
;	t← rv(AC3 xor 1); rv(AC3 xor 1)← AC1; AC1← t

DEXCH:	MAR← AC3;		START MEMORY CYCLE
	NOP;			DELAY
	MD← AC0;		FIRST WRITE
	MD← AC1,:DREAD1;	SECOND WRITE, GO TO READ


;DIOGNOSE INSTRUCTIONS

;DIOG1 - 61022
;	Hamming Code← AC2
;	rv(AC3)← AC0; rv(AC3 xor 1)← AC1

DIOG1:	MAR← ERRCTRL;		START WRITE TO ERROR CONTROL
	NOP;			DELAY
	MD← AC2,:DWRITE;	WRITE HAMMING CODE, GO TO DWRITE

;DIOG2 - 61023
;	rv(AC3)← AC0
;	rv(AC3)← AC0 xor AC1

DIOG2:	MAR← AC3;		START MEMORY CYCLE
	T← AC0;			SETUP FOR XOR
	L← AC1 XORT;		DO XOR
	MD← AC0;		FIRST WRITE
	MAR← AC3;		START MEMORY CYCLE
	AC0← L, TASK;		STORE XOR WORD
	MD← AC0, :START;	SECOND WRITE

;INTERRUPT SYSTEM.  TIMING IS 0 CYCLES IF DISABLED, 18 CYCLES
;IF THE INTERRUPTING CHANEL IS INACTIVE, AND 36+6N CYCLES TO CAUSE
;AN INTERRUPT ON CHANNEL N

INTCODE:PC← L, IR← 0;
	T← NWW;
	T← MD OR T;
	L← MD AND T;
	SAD← L, L← T, SH=0;		SAD HAD POTENTIAL INTERRUPTS
	NWW← L, L←0+1, :SOMEACTIVE;	NWW HAS NEW WW

NOACTIVE: MAR← WWLOC;		RESTORE WW TO CORE
	L← SAD;			AND REPLACE IT WITH SAD IN NWW
	MD← NWW, TASK;
INTZ:	NWW← L, :START;

SOMEACTIVE: MAR← PCLOC;	STORE PC AND SET UP TO FIND HIGHEST PRIORITY REQUEST
	XREG← L, L← 0;
	MD← PC, TASK;

ILPA:	PC← L;
ILP:	T← SAD;
	L← T← XREG AND T;
	SH=0, L← T, T← PC;
	:IEXIT, XREG← L LSH 1;

NIEXIT:	L← 0+T+1, TASK, :ILPA;
IEXIT:	MAR← PCLOC+T+1;		FETCH NEW PC. T HAS CHANNEL #, L HAS MASK

	XREG← L;
	T← XREG;
	L← NWW XOR T;	TURN OFF BIT IN WW FOR INTERRUPT ABOUT TO HAPPEN
	T← MD;
	NWW← L, L← T;
	PC← L, L← T← 0+1, TASK;
	SAD← L MRSH 1, :NOACTIVE;	SAD← 1B5 TO DISABLE INTERRUPTS

;
;	************************
;	* BIT-BLT - 61024 *
;	************************
;	Modified September 1977 to support Alternate memory banks
;	Last modified Sept 6, 1977 by Dan Ingalls
;
;	/* NOVA REGS
;	AC2 -> BLT DESCRIPTOR TABLE, AND IS PRESERVED
;	AC1 CARRIES LINE COUNT FOR RESUMING AFTER AN
;		INTERRUPT. MUST BE 0 AT INITIAL CALL
;	AC0 AND AC3 ARE SMASHED TO SAVE S-REGS
;
;	/* ALTO REGISTER USAGE
;DISP CARRIES:	TOPLD(100), SOURCEBANK(40), DESTBANK(20),
;		SOURCE(14), OP(3)
$MASK1		$R0;
$YMUL		$R2;	HAS TO BE AN R-REG FOR SHIFTS
$RETN		$R2;
$SKEW		$R3;
$TEMP		$R5;
$WIDTH		$R7;
$PLIER		$R7;	HAS TO BE AN R-REG FOR SHIFTS
$DESTY		$R10;
$WORD2		$R10;
$STARTBITSM1	$R35;
$SWA		$R36;
$DESTX		$R36;
$LREG		$R40;	HAS TO BE R40 (COPY OF L-REG)
$NLINES		$R41;
$RAST1		$R42;
$SRCX		$R43;
$SKMSK		$R43;
$SRCY		$R44;
$RAST2		$R44;
$CONST		$R45;
$TWICE		$R45;
$HCNT		$R46;
$VINC		$R46;
$HINC		$R47;
$NWORDS		$R50;
$MASK2		$R51;	WAS $R46;
;
$LASTMASKP1	$500;	MASKTABLE+021
$170000		$170000;
$CALL3		$3;	SUBROUTINE CALL INDICES
$CALL4		$4;
$DWAOFF		$2;	BLT TABLE OFFSETS
$DXOFF		$4;
$DWOFF		$6;
$DHOFF		$7;
$SWAOFF		$10;
$SXOFF		$12;
$GRAYOFF	$14;	GRAY IN WORDS 14-17
$LASTMASK	$477;	MASKTABLE+020	**NOT IN EARLIER PROMS!


;	BITBLT SETUP - CALCULATE RAM STATE FROM AC2'S TABLE
;----------------------------------------------------------
;
;	/* FETCH COORDINATES FROM TABLE
	!1,2,FDDX,BLITX;
	!1,2,FDBL,BBNORAM;
	!17,20,FDBX,,,,FDX,,FDW,,,,FSX,,,,,;	FDBL RETURNS (BASED ON OFFSET)
;	        (0)     4    6      12
BITBLT:	L← 0;
	SINK←LREG, BUSODD;	SINK← -1 IFF NO RAM
	L← T← DWOFF, :FDBL;
BBNORAM: TASK, :NPTRAP;		TRAP IF NO RAM
;
FDW:	T← MD;			PICK UP WIDTH, HEIGHT
	WIDTH← L, L← T, TASK, :NZWID;
NZWID:	NLINES← L;
	T← AC1;
	L← NLINES-T;
	NLINES← L, SH<0, TASK;
	:FDDX;
;
FDDX:	L← T← DXOFF, :FDBL;	PICK UP DEST X AND Y
FDX:	T← MD;
	DESTX← L, L← T, TASK;
	DESTY← L;
;
	L← T← SXOFF, :FDBL;	PICK UP SOURCE X AND Y
FSX:	T← MD;
	SRCX← L, L← T, TASK;
	SRCY← L, :CSHI;
;
;	/* FETCH DOUBLEWORD FROM TABLE (L← T← OFFSET, :FDBL)
FDBL:	MAR← AC2+T;
	SINK← LREG, BUS;
FDBX:	L← MD, :FDBX;
;
;	/* CALCULATE SKEW AND HINC
	!1,2,LTOR,RTOL;
CSHI:	T← DESTX;
	L← SRCX-T-1;
	T← LREG+1, SH<0;	TEST HORIZONTAL DIRECTION
	L← 17.T, :LTOR;	SKEW ← (SRCX - DESTX) MOD 16
RTOL:	SKEW← L, L← 0-1, :AH, TASK;	HINC ← -1
LTOR:	SKEW← L, L← 0+1, :AH, TASK;	HINC ← +1
AH:	HINC← L;
;
;	CALCULATE MASK1 AND MASK2
	!1,2,IFRTOL,LNWORDS;
	!1,2,POSWID,NEGWID;
CMASKS:	T← DESTX;
	T← 17.T;
	MAR← LASTMASKP1-T-1;
	L← 17-T;		STARTBITS ← 16 - (DESTX.17)
	STARTBITSM1← L;
	L← MD, TASK;
	MASK1← L;		MASK1 ← @(MASKLOC+STARTBITS)
	L← WIDTH-1;
	T← LREG-1, SH<0;
	T← DESTX+T+1, :POSWID;
POSWID:	T← 17.T;
	MAR← LASTMASK-T-1;
	T← ALLONES;		MASK2 ← NOT
	L← HINC-1;
	L← MD XOR T, SH=0, TASK;	@(MASKLOC+(15-((DESTX+WIDTH-1).17)))
	MASK2← L, :IFRTOL;
;	/* IF RIGHT TO LEFT, ADD WIDTH TO X'S AND EXCH MASK1, MASK2
IFRTOL:	T← WIDTH-1;	WIDTH-1
	L← SRCX+T;
	SRCX← L;		SRCX ← SCRX + (WIDTH-1)
	L← DESTX+T;
	DESTX← L;	DESTX ← DESTX + (WIDTH-1)
	T← DESTX;
	L← 17.T, TASK;
	STARTBITSM1← L;	STARTBITS ← (DESTX.17) + 1
	T← MASK1;
	L← MASK2;
	MASK1← L, L← T,TASK;	EXCHANGE MASK1 AND MASK2
	MASK2←L;
;
;	/* CALCULATE NWORDS
	!1,2,LNW1,THIN;
LNWORDS:T← STARTBITSM1+1;
	L← WIDTH-T-1;
	T← 177760, SH<0;
	T← LREG.T, :LNW1;
LNW1:	L← CALL4;		NWORDS ← (WIDTH-STARTBITS)/16
	CYRET← L, L← T, :R4, TASK; CYRET←CALL4
;	**WIDTH REG NOW FREE**
CYX4:	L← CYCOUT, :LNW2;
THIN:	T← MASK1;	SPECIAL CASE OF THIN SLICE
	L←MASK2.T;
	MASK1← L, L← 0-1;	MASK1 ← MASK1.MASK2, NWORDS ← -1
LNW2:	NWORDS← L;	LOAD NWORDS
;	**STARTBITSM1 REG NOW FREE**
;
;	/* DETERMINE VERTICAL DIRECTION
	!1,2,BTOT,TTOB;
	T← SRCY;
	L← DESTY-T;
	T← NLINES-1, SH<0;
	L← 0, :BTOT;	VINC ← 0 IFF TOP-TO-BOTTOM
BTOT:	L← ALLONES;	ELSE -1
BTOT1:	VINC← L;
	L← SRCY+T;		GOING BOTTOM TO TOP
	SRCY← L;			ADD NLINES TO STARTING Y'S
	L← DESTY+T;
	DESTY← L, L← 0+1, TASK;
	TWICE←L, :CWA;
;
TTOB:	T← AC1, :BTOT1;		TOP TO BOT, ADD NDONE TO STARTING Y'S
;	**AC1 REG NOW FREE**;
;
;	/* CALCULATE WORD ADDRESSES - DO ONCE FOR SWA, THEN FOR DWAX
CWA:	L← SRCY;	Y HAS TO GO INTO AN R-REG FOR SHIFTING
	YMUL← L;
	T← SWAOFF;		FIRST TIME IS FOR SWA, SRCX
	L← SRCX;
;	**SRCX, SRCY REG NOW FREE**
DOSWA:	MAR← AC2+T;		FETCH BITMAP ADDR AND RASTER
	XREG← L;
	L←CALL3;
	CYRET← L;		CYRET←CALL3
	L← MD;
	T← MD;
	DWAX← L, L←T, TASK;
	RAST2← L;
	T← 177760;
	L← T← XREG.T, :R4, TASK;	SWA ← SWA + SRCX/16
CYX3:	T← CYCOUT;
	L← DWAX+T;
	DWAX← L;
;
	!1,2,NOADD,DOADD;
	!1,2,MULLP,CDELT;	SWA ← SWA + SRCY*RAST1
	L← RAST2;
	SINK← YMUL, BUS=0, TASK;	NO MULT IF STARTING Y=0
	PLIER← L, :MULLP;
MULLP:	L← PLIER, BUSODD;		MULTIPLY RASTER BY Y
	PLIER← L RSH 1, :NOADD;
NOADD:	L← YMUL, SH=0, TASK;	TEST NO MORE MULTIPLIER BITS
SHIFTB:	YMUL← L LSH 1, :MULLP;
DOADD:	T← YMUL;
	L← DWAX+T;
	DWAX← L, L←T, :SHIFTB, TASK;
;	**PLIER, YMUL REG NOW FREE**
;
	!1,2,HNEG,HPOS;
	!1,2,VPOS,VNEG;
	!1,1,CD1;	CALCULATE DELTAS = +-(NWORDS+2)[HINC] +-RASTER[VINC]
CDELT:	L← T← HINC-1;	(NOTE T← -2 OR 0)
	L← T← NWORDS-T, SH=0;	(L←NWORDS+2 OR T←NWORDS)
CD1:	SINK← VINC, BUSODD, :HNEG;
HNEG:	T← RAST2, :VPOS;
HPOS:	L← -2-T, :CD1;	(MAKES L←-(NWORDS+2))
VPOS:	L← LREG+T, :GDELT, TASK;	BY NOW, LREG = +-(NWORDS+2)
VNEG:	L← LREG-T, :GDELT, TASK;	AND T = RASTER
GDELT:	RAST2← L;
;
;	/* END WORD ADDR LOOP
	!1,2,ONEMORE,CTOPL;
	L← TWICE-1;
	TWICE← L, SH<0;
	L← RAST2, :ONEMORE;	USE RAST2 2ND TIME THRU
ONEMORE:	RAST1← L;
	L← DESTY, TASK;	USE DESTY 2ND TIME THRU
	YMUL← L;
	L← DWAX;		USE DWAX 2ND TIME THRU
	T← DESTX;	CAREFUL - DESTX=SWA!!
	SWA← L, L← T;	USE DESTX 2ND TIME THRU
	T← DWAOFF, :DOSWA;	AND DO IT AGAIN FOR DWAX, DESTX
;	**TWICE, VINC REGS NOW FREE**
;
;	/* CALCULATE TOPLD
	!1,2,CTOP1,CSKEW;
	!1,2,HM1,H1;
	!1,2,NOTOPL,TOPL;
CTOPL:	L← SKEW, BUS=0, TASK;	IF SKEW=0 THEN 0, ELSE
CTX:	IR← 0, :CTOP1;
CTOP1:	T← SRCX;	(SKEW GR SRCX.17) XOR (HINC EQ 0)
	L← HINC-1;
	T← 17.T, SH=0;	TEST HINC
	L← SKEW-T-1, :HM1;
H1:	T← HINC, SH<0;
	L← SWA+T, :NOTOPL;
HM1:	T← LREG;		IF HINC=-1, THEN FLIP
	L← 0-T-1, :H1;	THE POLARITY OF THE TEST
NOTOPL:	SINK← HINC, BUSODD, TASK, :CTX;	HINC FORCES BUSODD
TOPL:	SWA← L, TASK;		(DISP ← 100 FOR TOPLD)
	IR← 100, :CSKEW;
;	**HINC REG NOW FREE**
;
;	/* CALCULATE SKEW MASK
	!1,2,THINC,BCOM1;
	!1,2,COMSK,NOCOM;
CSKEW:	T← SKEW, BUS=0;	IF SKEW=0, THEN COMP
	MAR← LASTMASKP1-T-1, :THINC;
THINC:	L←HINC-1;
	SH=0;			IF HINC=-1, THEN COMP
BCOM1:	T← ALLONES, :COMSK;
COMSK:	L← MD XOR T, :GFN;
NOCOM:	L← MD, :GFN;
;
;	/* GET FUNCTION
GFN:	MAR← AC2;
	SKMSK← L;

	T← MD;
	L← DISP+T, TASK;
	IR← LREG, :BENTR;		DISP ← DISP .OR. FUNCTION

;	BITBLT WORK - VERT AND HORIZ LOOPS WITH 4 SOURCES, 4 FUNCTIONS
;-----------------------------------------------------------------------
;
;	/* VERTICAL LOOP: UPDATE SWA, DWAX
	!1,2,DO0,VLOOP;
VLOOP:	T← SWA;
	L← RAST1+T;	INC SWA BY DELTA
	SWA← L;
	T← DWAX;
	L← RAST2+T, TASK;	INC DWAX BY DELTA
	DWAX← L;
;
;	/* TEST FOR DONE, OR NEED GRAY
	!1,2,MOREV,DONEV;
	!1,2,BMAYBE,BNOINT;
	!1,2,BDOINT,BDIS0;
	!1,2,DOGRAY,NOGRAY;
BENTR:	L← T← NLINES-1;		DECR NLINES AND CHECK IF DONE
	NLINES← L, SH<0;
	L← NWW, BUS=0, :MOREV;	CHECK FOR INTERRUPTS
MOREV:	L← 3.T, :BMAYBE, SH<0;	CHECK DISABLED   ***V3 change
BNOINT:	SINK← DISP, SINK← lgm10, BUS=0, :BDIS0, TASK;
BMAYBE:	SINK← DISP, SINK← lgm10, BUS=0, :BDOINT, TASK;	TEST IF NEED GRAY(FUNC=8,12)
BDIS0:	CONST← L, :DOGRAY;   ***V3 change
;
;	/* INTERRUPT SUSPENSION (POSSIBLY)
	!1,1,DOI1;	MAY GET AN OR-1
BDOINT:	:DOI1;	TASK HERE
DOI1:	T← AC2;
	MAR← DHOFF+T;		NLINES DONE = HT-NLINES-1
	T← NLINES;
	L← PC-1;		BACK UP THE PC, SO WE GET RESTARTED
	PC← L;
	L← MD-T-1, :BLITX, TASK;	...WITH NO LINES DONE IN AC1
;
;	/* LOAD GRAY FOR THIS LINE (IF FUNCTION NEEDS IT)
	!1,2,PRELD,NOPLD;
DOGRAY:	T← CONST-1;
	T← GRAYOFF+T+1;
	MAR← AC2+T;
	NOP;	UGH
	L← MD;
NOGRAY:	SINK← DISP, SINK← lgm100, BUS=0, TASK;	TEST TOPLD
	CONST← L, :PRELD;
;
;	/* NORMAL COMPLETION
NEGWID:	L← 0, :BLITX, TASK;
DONEV:	L← 0, :BLITX, TASK;	MAY BE AN OR-1 HERE!
BLITX:	AC1← L, :FINBLT;
;
;	/* PRELOAD OF FIRST SOURCE WORD (DEPENDING ON ALIGNMENT)
	!1,2,AB1,NB1;
PRELD:	SINK← DISP, SINK← lgm40, BUS=0;	WHICH BANK
	T← HINC, :AB1;
NB1:	MAR← SWA-T, :XB1;	(NORMAL BANK)
AB1:	XMAR← SWA-T, :XB1;	(ALTERNATE BANK)
XB1:	NOP;
	L← MD, TASK;
	WORD2← L, :NOPLD;
;
;
;	/* HORIZONTAL LOOP - 3 CALLS FOR 1ST, MIDDLE AND LAST WORDS
	!1,2,FDISPA,LASTH;
	%17,17,14,DON0,,DON2,DON3;		CALLERS OF HORIZ LOOP
;	NOTE THIS IGNORES 14-BITS, SO lgm14 WORKS LIKE L←0 FOR RETN
	!14,1,LH1;	IGNORE RESULTING BUS
NOPLD:	L← 3, :FDISP;		CALL #3 IS FIRST WORD
DON3:	L← NWORDS;
	HCNT← L, SH<0;		HCNT COUNTS WHOLE WORDS
DON0:	L← HCNT-1, :DO0;	IF NEG, THEN NO MIDDLE OR LAST
DO0:	HCNT← L, SH<0;		CALL #0 (OR-14!) IS MIDDLE WORDS
;	UGLY HACK SQUEEZES 2 INSTRS OUT OF INNER LOOP:
	L← DISP, SINK← lgm14, BUS, TASK, :FDISPA;	(WORKS LIKE L←0)
LASTH:	:LH1;	TASK AND BUS PENDING
LH1:	L← 2, :FDISP;		CALL #2 IS LAST WORD
DON2:	:VLOOP;
;
;
;	/* HERE ARE THE SOURCE FUNCTIONS
	!17,20,,,,F0,,,,F1,,,,F2,,,,F3;	IGNORE OP BITS IN FUNCTION CODE
	!17,20,,,,F0A,,,,F1A,,,,F2A,,,, ;	SAME FOR WINDOW RETURNS
	!3,4,OP0,OP1,OP2,OP3;
	!1,2,AB2,NB2;
FDISP:	SINK← DISP, SINK←lgm14, BUS, TASK;
FDISPA:	RETN← L, :F0;
F0:	SINK← DISP, SINK← lgm40, BUS=0, :WIND;	FUNC 0 - WINDOW
F1:	SINK← DISP, SINK← lgm40, BUS=0, :WIND;	FUNC 1 - NOT WINDOW
F1A:	T← CYCOUT;
	L← ALLONES XOR T, TASK, :F3A;
F2:	SINK← DISP, SINK← lgm40, BUS=0, :WIND;	FUNC 2 - WINDOW .AND. GRAY
F2A:	T← CYCOUT;
	L← ALLONES XOR T;
	SINK← DISP, SINK← lgm20, BUS=0;	WHICH BANK
	TEMP← L, :AB2;		TEMP ← NOT WINDOW
NB2:	MAR← DWAX, :XB2;	(NORMAL BANK)
AB2:	XMAR← DWAX, :XB2;	(ALTERNATE BANK)
XB2:	L← CONST AND T;		WINDOW .AND. GRAY
	T← TEMP;
	T← MD .T;		DEST.AND.NOT WINDOW
	L← LREG OR T, TASK, :F3A;		(TRANSPARENT)
F3:	L← CONST, TASK, :F3A;	FUNC 3 - CONSTANT (COLOR)
;
;
;	/* AFTER GETTING SOURCE, START MEMORY AND DISPATCH ON OP
	!1,2,AB3,NB3;
F3A:	CYCOUT← L;	(TASK HERE)
F0A:	SINK← DISP, SINK← lgm20, BUS=0;	WHICH BANK
	SINK← DISP, SINK← lgm3, BUS, :AB3;	DISPATCH ON OP
NB3:	T← MAR← DWAX, :OP0;	(NORMAL BANK)
AB3:	T← XMAR← DWAX, :OP0;	(ALTERNATE BANK)
;
;
;	/* HERE ARE THE OPERATIONS - ENTER WITH SOURCE IN CYCOUT
	%16,17,15,STFULL,STMSK;	MASKED OR FULL STORE (LOOK AT 2-BIT)
;				OP 0 - SOURCE
OP0:	SINK← RETN, BUS;	TEST IF UNMASKED
OP0A:	L← HINC+T, :STFULL;	ELSE :STMSK
OP1:	T← CYCOUT;		OP 1 - SOURCE .OR. DEST
	L← MD OR T, :OPN;
OP2:	T← CYCOUT;		OP 2 - SOURCE .XOR. DEST
	L← MD XOR T, :OPN;
OP3:	T← CYCOUT;		OP 3 - (NOT SOURCE) .AND. DEST
	L← 0-T-1;
	T← LREG;
	L← MD AND T, :OPN;
OPN:	SINK← DISP, SINK← lgm20, BUS=0, TASK;	WHICH BANK
	CYCOUT← L, :AB3;
;
;
;	/* STORE MASKED INTO DESTINATION
	!1,2,STM2,STM1;
	!1,2,AB4,NB4;
STMSK:	L← MD;
	SINK← RETN, BUSODD, TASK;	DETERMINE MASK FROM CALL INDEX
	TEMP← L, :STM2;		STACHE DEST WORD IN TEMP
STM1:	T←MASK1, :STM3;
STM2:	T←MASK2, :STM3;
STM3:	L← CYCOUT AND T;  ***X24. Removed TASK clause.
	CYCOUT← L, L← 0-T-1;	AND INTO SOURCE
	T← LREG;		T← MASK COMPLEMENTED
	T← TEMP .T;		AND INTO DEST
	L← CYCOUT OR T;		OR TOGETHER THEN GO STORE
	SINK← DISP, SINK← lgm20, BUS=0, TASK;	WHICH BANK
	CYCOUT← L, :AB4;
NB4:	T← MAR← DWAX, :OP0A;	(NORMAL BANK)
AB4:	T← XMAR← DWAX, :OP0A;	(ALTERNATE BANK)
;
;	/* STORE UNMASKED FROM CYCOUT (L=NEXT DWAX)
STFULL:	MD← CYCOUT;
STFUL1:	SINK← RETN, BUS, TASK;
	DWAX← L, :DON0;
;
;
;	/* WINDOW SOURCE FUNCTION
;	TASKS UPON RETURN, RESULT IN CYCOUT
	!1,2,DOCY,NOCY;
	!17,1,WIA;
	!1,2,NZSK,ZESK;
	!1,2,AB5,NB5;
WIND:	L← T← SKMSK, :AB5;	ENTER HERE (8 INST TO TASK)
NB5:	MAR← SWA, :XB5;		(NORMAL BANK)
AB5:	XMAR← SWA, :XB5;	(ALTERNATE BANK)
XB5:	L← WORD2.T, SH=0;
	CYCOUT← L, L← 0-T-1, :NZSK;	CYCOUT← OLD WORD .AND. MSK
ZESK:	L← MD, TASK;	ZERO SKEW BYPASSES LOTS
	CYCOUT← L, :NOCY;
NZSK:	T← MD;
	L← LREG.T;
	TEMP← L, L←T, TASK;	TEMP← NEW WORD .AND. NOTMSK
	WORD2← L;
	T← TEMP;
	L← T← CYCOUT OR T;		OR THEM TOGETHER
	CYCOUT← L, L← 0+1, SH=0;	DONT CYCLE A ZERO ***X21.
	SINK← SKEW, BUS, :DOCY;
DOCY:	CYRET← L LSH 1, L← T, :L0;	CYCLE BY SKEW ***X21.
NOCY:	T← SWA, :WIA;	(MAY HAVE OR-17 FROM BUS)
CYX2:	T← SWA;
WIA:	L← HINC+T;
	SINK← DISP, SINK← lgm14, BUS, TASK;	DISPATCH TO CALLER
	SWA← L, :F0A;

;	THE DISK CONTROLLER

;	ITS REGISTERS:
$DCBR		$R34;
$KNMAR		$R33;
$CKSUMR		$R32;
$KWDCT		$R31;
$KNMARW		$R33;
$CKSUMRW	$R32;
$KWDCTW		$R31;

;	ITS TASK SPECIFIC FUNCTIONS AND BUS SOURCES:
$KSTAT		$L020012,014003,124100;	DF1 = 12 (LHS) BS = 3 (RHS)
$RWC		$L024011,000000,000000;	NDF2 = 11
$RECNO		$L024012,000000,000000;	NDF2 = 12
$INIT		$L024010,000000,000000;	NDF2 = 10
$CLRSTAT	$L016014,000000,000000;	NDF1 = 14
$KCOMM		$L020015,000000,124000;	DF1 = 15 (LHS only) Requires bus def
$SWRNRDY	$L024014,000000,000000;	NDF2 = 14
$KADR		$L020016,000000,124000;	DF1 = 16 (LHS only) Requires bus def
$KDATA		$L020017,014004,124100;	DF1 = 17 (LHS)  BS = 4 (RHS)
$STROBE		$L016011,000000,000000;	NDF1 = 11
$NFER		$L024015,000000,000000;	NDF2 = 15
$STROBON	$L024016,000000,000000;	NDF2 = 16
$XFRDAT		$L024013,000000,000000;	NDF2 = 13
$INCRECNO	$L016013,000000,000000;	NDF1 = 13

;	THE DISK CONTROLLER COMES IN TWO PARTS. THE SECTOR
;	TASK HANDLES DEVICE CONTROL AND COMMAND UNDERSTANDING
;	AND STATUS REPORTING AND THE LIKE. THE WORD TASK ONLY
;	RUNS AFTER BEING ENABLED BY THE SECTOR TASK AND
;	ACTUALLY MOVES DATA WORDS TO AND FRO.

;   THE SECTOR TASK

;	LABEL PREDEFINITIONS:
!1,2,COMM,NOCOMM;
!1,2,COMM2,IDLE1;
!1,2,BADCOMM,COMM3;
!1,2,COMM4,ILLSEC;
!1,2,COMM5,WHYNRDY;
!1,2,STROB,CKSECT;
!1,2,STALL,CKSECT1;
!1,2,KSFINI,CKSECT2;
!1,2,IDLE2,TRANSFER;
!1,2,STALL2,GASP;
!1,2,INVERT,NOINVERT;

KSEC:	MAR← KBLKADR2;
KPOQ:	CLRSTAT;	RESET THE STORED DISK ADDRESS
	MD←L←ALLONES+1, :GCOM2;	ALSO CLEAR DCB POINTER

GETCOM:	MAR←KBLKADR;	GET FIRST DCB POINTER
GCOM1:	NOP;
	L←MD;
GCOM2:	DCBR←L,TASK;
	KCOMM←TOWTT;	IDLE ALL DATA TRANSFERS

	MAR←KBLKADR3;	GENERATE A SECTOR INTERRUPT
	T←NWW;
	L←MD OR T;

	MAR←KBLKADR+1;	STORE THE STATUS
	NWW←L, TASK;
	MD←KSTAT;

	MAR←KBLKADR;	WRITE THE CURRENT DCB POINTER
	KSTAT←5;	INITIAL STATUS IS INCOMPLETE
	L←DCBR,TASK,BUS=0;
	MD←DCBR, :COMM;

;	BUS=0 MAPS COMM TO NOCOMM

COMM:	T←2;	GET THE DISK COMMAND
	MAR←DCBR+T;
	T←TOTUWC;
	L←MD XOR T, TASK, STROBON;
	KWDCT←L, :COMM2;

;	STROBON MAPS COMM2 TO IDLE1

COMM2:	T←10;	READ NEW DISK ADDRESS
	MAR←DCBR+T+1;
	T←KWDCT;
	L←ONE AND T;
	L←-400 AND T, SH=0;
	T←MD, SH=0, :INVERT;

;	SH=0 MAPS INVERT TO NOINVERT

INVERT:	L←2 XOR T, TASK, :BADCOMM;
NOINVERT: L←T, TASK, :BADCOMM;

;	SH=0 MAPS BADCOMM TO COMM3

COMM3:	KNMAR←L;

	MAR←KBLKADR2;	WRITE THE NEW DISK ADDRESS
	T←SECT2CM;	CHECK FOR SECTOR > 13
	L←T←KDATA←KNMAR+T;	NEW DISK ADDRESS TO HARDWARE
	KADR←KWDCT,ALUCY;	DISK COMMAND TO HARDWARE
	L←MD XOR T,TASK, :COMM4;	COMPARE OLD AND NEW DISK ADDRESSES

;	ALUCY MAPS COMM4 TO ILLSEC

COMM4:	CKSUMR←L;

	MAR←KBLKADR2;	WRITE THE NEW DISK ADDRESS
	T←CADM,SWRNRDY;	SEE IF DISK IS READY
	L←CKSUMR AND T, :COMM5;

;	SWRNRDY MAPS COMM5 TO WHYNRDY

COMM5:	MD←KNMAR;	COMPLETE THE WRITE
	SH=0,TASK;
	:STROB;

;	SH=0 MAPS STROB TO CKSECT

CKSECT:	T←KNMAR,NFER;
	L←KSTAT XOR T, :STALL;

;	NFER MAPS STALL TO CKSECT1

CKSECT1: CKSUMR←L,XFRDAT;
	T←CKSUMR, :KSFINI;

;	XFRDAT MAPS KSFINI TO CKSECT2

CKSECT2: L←SECTMSK AND T;
KSLAST:	BLOCK,SH=0;
GASP:	TASK, :IDLE2;

;	SH=0 MAPS IDLE2 TO TRANSFER

TRANSFER: KCOMM←TOTUWC;	TURN ON THE TRANSFER

!1,2,ERRFND,NOERRFND;
!1,2,EF1,NEF1;

DMPSTAT: T←COMERR1;	SEE IF STATUS REPRESENTS ERROR
	L←KSTAT AND T;
	MAR←DCBR+1;	WRITE FINAL STATUS
	KWDCT←L,TASK,SH=0;
	MD←KSTAT,:ERRFND;

;	SH=0 MAPS ERRFND TO NOERRFND

NOERRFND: T←6;	PICK UP NO-ERROR INTERRUPT WORD

INTCOM:	MAR←DCBR+T;
	T←NWW;
	L←MD OR T;
	SINK←KWDCT,BUS=0,TASK;
	NWW←L,:EF1;

;	BUS=0 MAPS EF1 TO NEF1

NEF1:	MAR←DCBR,:GCOM1;	FETCH ADDRESS OF NEXT CONTROL BLOCK

ERRFND:	T←7,:INTCOM;	PICK UP ERROR INTERRUPT WORD

EF1:	:KSEC;

NOCOMM:	L←ALLONES,CLRSTAT,:KSLAST;

IDLE1:	L←ALLONES,:KSLAST;

IDLE2:	KSTAT←LOW14, :GETCOM;	NO ACTIVITY THIS SECTOR

BADCOMM: KSTAT←7;	ILLEGAL COMMAND ONLY NOTED IN KBLK STAT
	BLOCK;
	TASK,:EF1;

WHYNRDY: NFER;
STALL:	BLOCK, :STALL2;

;	NFER MAPS STALL2 TO GASP

STALL2:	TASK;
	:DMPSTAT;

ILLSEC:	KSTAT←7, :STALL;	ILLEGAL SECTOR SPECIFIED

STROB:	CLRSTAT;
	L←ALLONES,STROBE,:CKSECT1;

KSFINI:	KSTAT←4, :STALL;	COMMAND FINISHED CORRECTLY


;DISK WORD TASK
;WORD TASK PREDEFINITIONS
!37,37,,,,RP0,INPREF1,CKP0,WP0,,PXFLP1,RDCK0,WRT0,REC1,,REC2,REC3,,,REC0RC,REC0W,R0,,CK0,W0,,R2,,W2,,REC0,,KWD;
!1,2,RW1,RW2;
!1,2,CK1,CK2;
!1,2,CK3,CK4;
!1,2,CKERR,CK5;
!1,2,PXFLP,PXF2;
!1,2,PREFDONE,INPREF;
!1,2,,CK6;
!1,2,CKSMERR,PXFLP0;

KWD:	BLOCK,:REC0;

;	SH<0 MAPS REC0 TO REC0
;	ANYTHING=INIT MAPS REC0 TO KWD

REC0:	L←2, TASK;	LENGTH OF RECORD 0 (ALLOW RELEASE IF BLOCKED)
	KNMARW←L;

	T←KNMARW, BLOCK, RWC;	 GET ADDR OF MEMORY BLOCK TO TRANSFER
	MAR←DCBR+T+1, :REC0RC;

;	WRITE MAPS REC0RC TO REC0W
;	INIT MAPS REC0RC TO KWD

REC0RC:	T←MFRRDL,BLOCK, :REC12A;	FIRST RECORD READ DELAY
REC0W:	T←MFR0BL,BLOCK, :REC12A;	FIRST RECORD 0'S BLOCK LENGTH

REC1:	L←10, INCRECNO;	 LENGTH OF RECORD 1
	T←4, :REC12;
REC2:	L←PAGE1, INCRECNO;	 LENGTH OF RECORD 2
	T←5, :REC12;
REC12:	MAR←DCBR+T, RWC;	 MEM BLK ADDR FOR RECORD
	KNMARW←L, :RDCK0;

;	RWC=WRITE MAPS RDCK0 INTO WRT0
;	RWC=INIT MAPS RDCK0 INTO KWD

RDCK0:	T←MIRRDL, :REC12A;
WRT0:	T←MIR0BL, :REC12A;

REC12A:	L←MD;
	KWDCTW←L, L←T;
COM1:	KCOMM← STUWC, :INPREF0;

INPREF:	L←CKSUMRW+1, INIT, BLOCK;
INPREF0: CKSUMRW←L, SH<0, TASK, :INPREF1;

;	INIT MAPS INPREF1 TO KWD

INPREF1: KDATA←0, :PREFDONE;

;	SH<0 MAPS PREFDONE TO INPREF

PREFDONE: T←KNMARW;	COMPUTE TOP OF BLOCK TO TRANSFER
KWDX:	L←KWDCTW+T,RWC;		(ALSO USED FOR RESET)
	KNMARW←L,BLOCK,:RP0;

;	RWC=CHECK MAPS RP0 TO CKP0
;	RWC=WRITE MAPS RP0 AND CKP0 TO WP0
;	RWC=INIT MAPS RP0, CKP0, AND WP0 TO KWD

RP0:	KCOMM←STRCWFS,:WP1;

CKP0:	L←KWDCTW-1;	ADJUST FINISHING CONDITION BY 1 FOR CHECKING ONLY
	KWDCTW←L,:RP0;

WP0:	KDATA←ONE;	WRITE THE SYNC PATTERN
WP1:	L←KBLKADR,TASK,:RW1;	INITIALIZE THE CHECKSUM AND ENTER XFER LOOP


XFLP:	T←L←KNMARW-1;	BEGINNING OF MAIN XFER LOOP
	KNMARW←L;
	MAR←KNMARW,RWC;
	L←KWDCTW-T,:R0;

;	RWC=CHECK MAPS R0 TO CK0
;	RWC=WRITE MAPS R0 AND CK0 TO W0
;	RWC=INIT MAPS R0, CK0, AND W0 TO KWD

R0:	T←CKSUMRW,SH=0,BLOCK;
	MD←L←KDATA XOR T,TASK,:RW1;

;	SH=0 MAPS RW1 TO RW2

RW1:	CKSUMRW←L,:XFLP;

W0:	T←CKSUMRW,BLOCK;
	KDATA←L←MD XOR T,SH=0;
	TASK,:RW1;

;	AS ALREADY NOTED, SH=0 MAPS RW1 TO RW2

CK0:	T←KDATA,BLOCK,SH=0;
	L←MD XOR T,BUS=0,:CK1;

;	SH=0 MAPS CK1 TO CK2

CK1:	L←CKSUMRW XOR T,SH=0,:CK3;

;	BUS=0 MAPS CK3 TO CK4

CK3:	TASK,:CKERR;

;	SH=0 MAPS CKERR TO CK5

CK5:	CKSUMRW←L,:XFLP;

CK4:	MAR←KNMARW, :CK6;

;	SH=0 MAPS CK6 TO CK6

CK6:	CKSUMRW←L,L←0+T;
	MTEMP←L,TASK;
	MD←MTEMP,:XFLP;

CK2:	L←CKSUMRW-T,:R2;

;	BUS=0 MAPS R2 TO R2

RW2:	CKSUMRW←L;

	T←KDATA←CKSUMRW,RWC;	THIS CODE HANDLES THE FINAL CHECKSUM
	L←KDATA-T,BLOCK,:R2;

;	RWC=CHECK NEVER GETS HERE
;	RWC=WRITE MAPS R2 TO W2
;	RWC=INIT MAPS R2 AND W2 TO KWD

R2:	L←MRPAL, SH=0;	SET READ POSTAMBLE LENGTH, CHECK CKSUM
	KCOMM←TOTUWC, :CKSMERR;

;	SH=0 MAPS CKSMERR TO PXFLP0

W2:	L←MWPAL, TASK;	SET WRITE POSTAMBLE LENGTH
	CKSUMRW←L, :PXFLP;

CKSMERR: KSTAT←0,:PXFLP0;	0 MEANS CHECKSUM ERROR .. CONTINUE

PXFLP:	L←CKSUMRW+1, INIT, BLOCK;
PXFLP0:	CKSUMRW←L, TASK, SH=0, :PXFLP1;

;	INIT MAPS PXFLP1 TO KWD

PXFLP1:	KDATA←0,:PXFLP;

;	SH=0 MAPS PXFLP TO PXF2

PXF2:	RECNO, BLOCK;	DISPATCH BASED ON RECORD NUMBER
	:REC1;

;	RECNO=2 MAPS REC1 INTO REC2
;	RECNO=3 MAPS REC1 INTO REC3
;	RECNO=INIT MAPS REC1 INTO KWD

REC3:	KSTAT←4,:PXFLP;	4 MEANS SUCCESS!!!

CKERR:	KCOMM←TOTUWC;	TURN OFF DATA TRANSFER
	L←KSTAT←6, :PXFLP1;	SHOW CHECK ERROR AND LOOP

;The Parity Error Task
;Its label predefinition is way earlier
;It dumps the following interesting registers:
;614/ DCBR	Disk control block
;615/ KNMAR	Disk memory address
;616/ DWA	Display memory address
;617/ CBA	Display control block
;620/ PC	Emulator program counter
;621/ SAD	Emulator temporary register for indirection

PART:	T← 10;
	L← ALLONES;		TURN OFF MEMORY INTERRUPTS
	MAR← ERRCTRL, :PX1;
PR8:	L← SAD, :PX;
PR7:	L← PC, :PX;
PR6:	L← CBA, :PX;
PR5:	L← DWA, :PX;
PR4:	L← KNMAR, :PX;
PR3:	L← DCBR, :PX;
PR2:	L← NWW OR T, TASK;	T CONTAINS 1 AT THIS POINT
PR0:	NWW← L, :PART;

PX:	MAR← 612+T;
PX1:	MTEMP← L, L← T;
	MD← MTEMP;
	CURDATA← L;		THIS CLOBBERS THE CURSOR FOR ONE
	T← CURDATA-1, BUS;	FRAME WHEN AN ERROR OCCURS
	:PR0;

; AltoIIMRT4K.mu
;
; last modified December 1, 1977  1:14 AM
;
; This is the part of the Memory Refresh Task which
; is specific to Alto IIs WITHOUT Extended memory.
;
; Copyright Xerox Corporation 1979
$EngNumber	$20000;		ALTO 2 WITHOUT EXTENDED MEMORY

MRT:	SINK← MOUSE, BUS;	MOUSE DATA IS ANDED WITH 17B
MRTA:	L← T← -2, :TX0;		DISPATCH ON MOUSE CHANGE
TX0:	L← T← R37 AND NOT T;	UPDATE REFRESH ADDRESS
	T← 3+T+1, SH=0;
	L← REFIIMSK ANDT, :DOTIMER;
NOTIMER:R37← L; 		STORE UPDATED REFRESH ADDRESS
TIMERTN:L← REFZERO AND T;
	SH=0;			TEST FOR CLOCK TICK
	:NOCLK;
NOCLK:	MAR← R37;		FIRST FEFRESH CYCLE
	L← CURX;
	T← 2, SH=0;
	MAR← R37 XORT, :DOCUR;  SECOND REFRESH CYCLE
NOCUR:	CURDATA← L, TASK;
MRTLAST:CURDATA← L, :MRT;

DOTIMER:R37← L;			SAVE REFRESH ADDRESS
	MAR←EIALOC;		INTERVAL TIMER/EIA INTERFACE
	L←2 AND T;
	SH=0, L←T←REFZERO.T;	***V3 CHANGE (USED TO BE BIAS)
	CURDATA←L, :SPCHK;	CURDATA←CURRENT TIME WITHOUT CONTROL BITS

SPCHK:	SINK←MD, BUS=0, TASK;	CHECK FOR EIA LINE SPACING
SPIA:	:NOTIMERINT, CLOCKTEMP←L;

NOSPCHK:L←MD;			CHECK FOR TIME=NOW
	MAR←TRAPDISP-1;		CONTAINS TIME AT WHICH INTERRUPT SHOULD HAPPEN
	MTEMP←L;		IF INTERRUPT IS CAUSED,
	L← MD-T;		LINE STATE WILL BE STORED
	SH=0, TASK, L←MTEMP, :SPIA;

TIMERINT:MAR← ITQUAN;		STORE THE THING IN CLOCKTEMP AT ITQUAN
	L← CURDATA;
	R37← L;
	T←NWW;			AND CAUSE AN INTERRUPT ON THE CHANNELS
	MD←CLOCKTEMP;		SPECIFIED BY ITQUAN+1
	L←MD OR T, TASK;
	NWW←L;

NOTIMERINT: T←R37, :TIMERTN;

;The rest of MRT, starting at the label CLOCK is unchanged

; AltoIIMRT16K.mu
;
; last modified December 1, 1977  1:13 AM
;
; This is the part of the Memory Refresh Task which
; is specific to Alto IIs with Extended memory.
;
; Copyright Xerox Corporation 1979
$EngNumber	$30000;		ALTO II WITH EXTENDED MEMORY
;
; This version assumes MRTACT is cleared by BLOCK, not MAR← R37
; R37 [4-13] are the low bits of the TOD clock
; R37 [8-14] are the refresh address bits
; Each time MRT runs, four refresh addresses are generated, though
; R37 is incremented only once.  Sprinkled throughout the execution
; of this code are the following operations having to do with refresh:
;	MAR← R37
;	R37← R37 +4		NOTE THAT R37 [14] DOES NOT CHANGE
;	MAR← R37 XOR 2		TOGGLES BIT 14
;	MAR← R37 XOR 200	TOGGLES BIT 8
;	MAR← R37 XOR 202	TOGGLES BITS 8 AND 14

MRT:	MAR← R37;		**FIRST REFRESH CYCLE**
	SINK← MOUSE, BUS;	MOUSE DATA IS ANDED WITH 17B
MRTA:	L← T← -2, :TX0;		DISPATCH ON MOUSE CHANGE
TX0:	L← R37 AND NOT T, T← R37;INCREMENT CLOCK
	T← 3+T+1, SH=0;		IE. T← T +4.  IS INTV TIMER ON?
	L← REFIIMSK AND T, :DOTIMER; [DOTIMER,NOTIMER] ZERO HIGH 4 BITS
NOTIMER: R37← L; 		STORE UPDATED CLOCK
NOTIMERINT: T← 2;		NO STATE AT THIS POINT IN PUBLIC REGS
	MAR← R37 XOR T,T← R37;	**SECOND REFRESH CYCLE**
	L← REFZERO AND T;	ONLY THE CLOKCK BITS, PLEASE
	SH=0, TASK;		TEST FOR CLOCK OVERFLOW
	:NOCLK;			[NOCLK,CLOCK]
NOCLK:	T ← 200;
	MAR← R37 XOR T;		**THIRD FEFRESH CYCLE**
	L← CURX, BLOCK;		CLEARS WAKEUP REQUEST FF
	T← 2 OR T, SH=0;	NEED TO CHECK CURSOR?
	MAR← R37 XOR T, :DOCUR;	**FOURTH REFRESH CYCLE**
NOCUR:	CURDATA← L, TASK;
MRTLAST:CURDATA← L, :MRT;	END OF MAIN LOOP

DOTIMER:R37← L;			STORE UPDATED CLOCK
	MAR← EIALOC;		INTERVAL TIMER/EIA INTERFACE
	L← 2 AND T;
	SH=0, L← T← REFZERO.T;	***V3 CHANGE (USED TO BE BIAS)
	CURDATA←L, :SPCHK;	CURDATA← CURRENT TIME WITHOUT CONTROL BITS

SPCHK:	SINK← MD, BUS=0, TASK;	CHECK FOR EIA LINE SPACING
SPIA:	:NOTIMERINT, CLOCKTEMP← L;

NOSPCHK:L←MD;			CHECK FOR TIME = NOW
	MAR←TRAPDISP-1;		CONTAINS TIME AT WHICH INTERRUPT SHOULD HAPPEN
	MTEMP←L;		IF INTERRUPT IS CAUSED,
	L← MD-T;		LINE STATE WILL BE STORED
	SH=0, TASK, L←MTEMP, :SPIA;

TIMERINT:MAR← ITQUAN;		STORE THE THING IN CLOCKTEMP AT ITQUAN
	L← CURDATA;
	R37← L;
	T←NWW;			AND CAUSE AN INTERRUPT ON THE CHANNELS
	MD←CLOCKTEMP;		SPECIFIED BY ITQUAN+1
	L←MD OR T, TASK;
	NWW←L,:NOTIMERINT;

;The rest of MRT, starting at the label CLOCK is unchanged
**************************************************************************************************/
