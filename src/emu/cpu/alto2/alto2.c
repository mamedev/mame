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

//	AM_RANGE(0200000,                    0377777)                           AM_READWRITE( ioram_r, ioram_w )
ADDRESS_MAP_END

//-------------------------------------------------
//  alto2_cpu_device - constructor
//-------------------------------------------------

alto2_cpu_device::alto2_cpu_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	cpu_device(mconfig, ALTO2, "Xerox Alto-II", tag, owner, clock, "alto2", __FILE__),
#if	ALTO2_DEBUG
	m_log_types(LOG_DISK|LOG_KSEC|LOG_KWD),
	m_log_level(8),
	m_log_newline(true),
#endif
	m_ucode_config("ucode", ENDIANNESS_BIG, 32, 12, -2 ),
	m_const_config("const", ENDIANNESS_BIG, 16,  8, -1 ),
	m_iomem_config("iomem", ENDIANNESS_BIG, 16, 16, -1 ),
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

alto2_cpu_device::~alto2_cpu_device()
{
	// call all subdevice's exit code
	exit_kwd();
	exit_part();
	exit_dvt();
	exit_dht();
	exit_curt();
	exit_dwt();
	exit_mrt();
	exit_ether();
	exit_ksec();
	exit_emu();
	exit_hw();
	exit_mouse();
	exit_kbd();
	exit_disp();
	exit_disk();
	exit_memory();

	if (m_ucode_crom) {
		global_free(m_ucode_crom);
		m_ucode_crom = 0;
	}
	if (m_ucode_cram) {
		global_free(m_ucode_cram);
		m_ucode_cram = 0;
	}
	if (m_const_data) {
		global_free(m_const_data);
		m_const_data = 0;
	}

	if (m_ctl2k_u3) {
		global_free(m_ctl2k_u3);
		m_ctl2k_u3 = 0;
	}
	if (m_ctl2k_u38) {
		global_free(m_ctl2k_u38);
		m_ctl2k_u38 = 0;
	}
	if (m_ctl2k_u76) {
		global_free(m_ctl2k_u76);
		m_ctl2k_u76 = 0;
	}
	if (m_cram3k_a37) {
		global_free(m_cram3k_a37);
		m_cram3k_a37 = 0;
	}
	if (m_madr_a64) {
		global_free(m_madr_a64);
		m_madr_a64 = 0;
	}
	if (m_madr_a65) {
		global_free(m_madr_a65);
		m_madr_a65 = 0;
	}
	if (m_madr_a90) {
		global_free(m_madr_a90);
		m_madr_a90 = 0;
	}
	if (m_madr_a91) {
		global_free(m_madr_a91);
		m_madr_a91 = 0;
	}
	if (m_alu_a10) {
		global_free(m_alu_a10);
		m_alu_a10 = 0;
	}

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
				logerror("%-7s %11lld ", type_name[i], cycle());
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
	// get a pointer to the IO address space
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
	state_add( A2_SHIFTIN, "SHIFTIN", m_dsk.shiftin).formatstr("%06O");
	state_add( A2_SHIFTOUT,"SHIFTOUT",m_dsk.shiftout).formatstr("%06O");
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

#define	RD_CROM(addr) crom_r(space(AS_0), addr, 0xffffffff)
#define	RD_CRAM(addr) cram_r(space(AS_0), addr, 0xffffffff)
#define	WR_CRAM(addr,data) cram_w(space(AS_0), addr, data)

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

#if	ALTO2_DEBUG
void alto2_cpu_device::watch_read(UINT32 addr, UINT32 data)
{
	LOG((LOG_MEM,0,"mem: rd[%06o] = %06o\n", addr, data));
}

void alto2_cpu_device::watch_write(UINT32 addr, UINT32 data)
{
	LOG((LOG_MEM,0,"mem: wr[%06o] = %06o\n", addr, data));
}
#endif

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
	val = RD_CRAM(addr) ^ ALTO2_UCODE_INVERTED;
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
	WR_CRAM(addr, ((m_m << 16) | m_alu) ^ ALTO2_UCODE_INVERTED);
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
		m_mir = RD_CROM(m_mpc);
		m_rsel = MIR_RSEL(m_mir);
		m_next = MIR_NEXT(m_mir) | m_next2;
		m_next2 = A2_GET32(RD_CROM(m_next), 32, NEXT0, NEXT9) | (m_next2 & ~ALTO2_UCODE_PAGE_MASK);
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
			// There is something going wrong with using:
			// m_const->read_word(m_const->address_to_byte(addr));
			// because for addr=0160 it returns const[0161] instead of const[0160]
			// For now fall back to reading the const data from the byte array
			UINT16 data = m_const_data[2*addr+0] | (m_const_data[2*addr+1] << 8);
			m_bus &= data;
			LOG((LOG_CPU,2,"	%#o; BUS &= %#o CONST[%03o]\n", m_bus, data, addr));
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
	init_mouse();
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
