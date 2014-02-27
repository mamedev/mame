// license:?
// copyright-holders:Angelo Salese, R. Belmont, Juergen Bunchmueller
/******************************************************************************
 *
 *  Acorn Archimedes 310
 *
 *  Skeleton: Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *  Enhanced: R. Belmont, June 2007
 *  Angelo Salese, August 2010
 *
 *  TODO:
 *  - try to understand why bios 2 was working at some point and now isn't
 *  - fix RISC OS / Arthur booting, possible causes:
 *  \- missing reset page size hook-up
 *  \- some subtle memory paging fault
 *  \- missing RAM max size
 *  \- ARM bug?
 *
 *
=======================================================================================
 *
 *      Memory map (from http://b-em.bbcmicro.com/arculator/archdocs.txt)
 *
 *  0000000 - 1FFFFFF - logical RAM (32 meg)
 *  2000000 - 2FFFFFF - physical RAM (supervisor only - max 16MB - requires quad MEMCs)
 *  3000000 - 33FFFFF - IOC (IO controllers - supervisor only)
 *  3310000 - FDC - WD1772
 *  33A0000 - Econet - 6854
 *  33B0000 - Serial - 6551
 *  3240000 - 33FFFFF - internal expansion cards
 *  32D0000 - hard disc controller (not IDE) - HD63463
 *  3350010 - printer
 *  3350018 - latch A
 *  3350040 - latch B
 *  3270000 - external expansion cards
 *
 *  3400000 - 3FFFFFF - ROM (read - 12 meg - Arthur and RiscOS 2 512k, RiscOS 3 2MB)
 *  3400000 - 37FFFFF - Low ROM  (4 meg, I think this is expansion ROMs)
 *  3800000 - 3FFFFFF - High ROM (main OS ROM)
 *
 *  3400000 - 35FFFFF - VICD10 (write - supervisor only)
 *  3600000 - 3FFFFFF - MEMC (write - supervisor only)
 *
 *****************************************************************************/
/*
    DASM of code (bios 2 / RISC OS 2)
    0x380d4e0: MEMC: control to 0x10c (page size 32 kbytes, DRAM ram refresh only during flyback)
    0x380d4f0: VIDC: params (screen + sound frequency)
    0x380d51c: IOC: sets control to 0xff, clear IRQA and FIQ masks, sets IRQB mask to 0x80 (keyboard receive full irq)
    0x380d530: IOC: sets timer 0 to 0x4e20, go command
        0x380e0a8: work RAM physical check, max size etc.
    0x380e1f8: IOC: Disables DRAM ram refresh, sets timer 1 to 0x7ffe, go command, then it tests the latch of this timer, enables DRAM refresh
        0x380d00c: Set up default logical space
        0x380d16c: Set up case by case logical space


*/


#include "emu.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "cpu/arm/arm.h"
#include "sound/dac.h"
#include "includes/archimds.h"
#include "machine/i2cmem.h"
//#include "machine/aakart.h"
#include "machine/ram.h"


class a310_state : public archimedes_state
{
public:
	a310_state(const machine_config &mconfig, device_type type, const char *tag)
		: archimedes_state(mconfig, type, tag)
		, m_physram(*this, "physicalram")
		, m_ram(*this, RAM_TAG)
	{ }

	required_shared_ptr<UINT32> m_physram;

	DECLARE_READ32_MEMBER(a310_psy_wram_r);
	DECLARE_WRITE32_MEMBER(a310_psy_wram_w);
	DECLARE_WRITE_LINE_MEMBER(a310_wd177x_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(a310_wd177x_drq_w);
	DECLARE_DRIVER_INIT(a310);
	virtual void machine_start();
	virtual void machine_reset();

protected:
	required_device<ram_device> m_ram;
};


WRITE_LINE_MEMBER(a310_state::a310_wd177x_intrq_w)
{
	if (state)
		archimedes_request_fiq(ARCHIMEDES_FIQ_FLOPPY);
	else
		archimedes_clear_fiq(ARCHIMEDES_FIQ_FLOPPY);
}

WRITE_LINE_MEMBER(a310_state::a310_wd177x_drq_w)
{
	if (state)
		archimedes_request_fiq(ARCHIMEDES_FIQ_FLOPPY_DRQ);
	else
		archimedes_clear_fiq(ARCHIMEDES_FIQ_FLOPPY_DRQ);
}

READ32_MEMBER(a310_state::a310_psy_wram_r)
{
	return m_physram[offset];
}

WRITE32_MEMBER(a310_state::a310_psy_wram_w)
{
	COMBINE_DATA(&m_physram[offset]);
}


DRIVER_INIT_MEMBER(a310_state,a310)
{
	UINT32 ram_size = m_ram->size();

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler( 0x02000000, 0x02000000+(ram_size-1), read32_delegate(FUNC(a310_state::a310_psy_wram_r), this), write32_delegate(FUNC(a310_state::a310_psy_wram_w), this));

	archimedes_driver_init();
}

void a310_state::machine_start()
{
	archimedes_init();

	// reset the DAC to centerline
	//m_dac->write_signed8(0x80);
}

void a310_state::machine_reset()
{
	archimedes_reset();
}

static ADDRESS_MAP_START( a310_mem, AS_PROGRAM, 32, a310_state )
	AM_RANGE(0x00000000, 0x01ffffff) AM_READWRITE(archimedes_memc_logical_r, archimedes_memc_logical_w)
	AM_RANGE(0x02000000, 0x02ffffff) AM_RAM AM_SHARE("physicalram") /* physical RAM - 16 MB for now, should be 512k for the A310 */
	AM_RANGE(0x03000000, 0x033fffff) AM_READWRITE(archimedes_ioc_r, archimedes_ioc_w)
	AM_RANGE(0x03400000, 0x035fffff) AM_READWRITE(archimedes_vidc_r, archimedes_vidc_w)
	AM_RANGE(0x03600000, 0x037fffff) AM_READWRITE(archimedes_memc_r, archimedes_memc_w)
	AM_RANGE(0x03800000, 0x03ffffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITE(archimedes_memc_page_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( a310 )
	PORT_START("dip") /* DIP switches */
	PORT_BIT(0xfd, 0xfd, IPT_UNUSED)

	PORT_START("key0") /* KEY ROW 0 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("1  !") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("2  \"") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("3  #") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("4  $") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("5  %") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("6  &") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("7  '") PORT_CODE(KEYCODE_7)

	PORT_START("key1") /* KEY ROW 1 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("8  *") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("9  (") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("0  )") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("-  _") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("=  +") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("`  ~") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("BACK SPACE") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)

	PORT_START("key2") /* KEY ROW 2 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("q  Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("w  W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("e  E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("r  R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("t  T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("y  Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("u  U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("i  I") PORT_CODE(KEYCODE_I)

	PORT_START("key3") /* KEY ROW 3 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("o  O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("p  P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("[  {") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("]  }") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	PORT_START("key4") /* KEY ROW 4 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("a  A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("s  S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("d  D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("f  F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("g  G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("h  H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("j  J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("k  K") PORT_CODE(KEYCODE_K)

	PORT_START("key5") /* KEY ROW 5 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("l  L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME(";  :") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("'  \"") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("\\  |") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("SHIFT (L)") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("z  Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("x  X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("c  C") PORT_CODE(KEYCODE_C)

	PORT_START("key6") /* KEY ROW 6 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("v  V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("b  B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("n  N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("m  M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME(",  <") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME(".  >") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("/  ?") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("SHIFT (R)") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("key7") /* KEY ROW 7 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("LINE FEED")
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("- (KP)") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(", (KP)") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("ENTER (KP)") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME(". (KP)") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("0 (KP)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("1 (KP)") PORT_CODE(KEYCODE_1_PAD)

	PORT_START("key8") /* KEY ROW 8 */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("2 (KP)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("3 (KP)") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("4 (KP)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("5 (KP)") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("6 (KP)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("7 (KP)") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("8 (KP)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("9 (KP)") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("via1a") /* VIA #1 PORT A */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1                   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2                   )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_4WAY

	PORT_START("tape")/* tape control */
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("TAPE STOP") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("TAPE PLAY") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("TAPE REW") PORT_CODE(KEYCODE_F7)
	PORT_BIT (0xf8, 0x80, IPT_UNUSED)
INPUT_PORTS_END

static const wd17xx_interface a310_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(a310_state, a310_wd177x_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(a310_state, a310_wd177x_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

WRITE_LINE_MEMBER( archimedes_state::a310_kart_tx_w )
{
	if(state)
		archimedes_request_irq_b(ARCHIMEDES_IRQB_KBD_RECV_FULL);
	else
		archimedes_clear_irq_b(ARCHIMEDES_IRQB_KBD_RECV_FULL);
}

WRITE_LINE_MEMBER( archimedes_state::a310_kart_rx_w )
{
	if(state)
		archimedes_request_irq_b(ARCHIMEDES_IRQB_KBD_XMIT_EMPTY);
	else
		archimedes_clear_irq_b(ARCHIMEDES_IRQB_KBD_XMIT_EMPTY);
}

static AAKART_INTERFACE( kart_interface )
{
	DEVCB_DRIVER_LINE_MEMBER(archimedes_state, a310_kart_tx_w),
	DEVCB_DRIVER_LINE_MEMBER(archimedes_state, a310_kart_rx_w)
};

static MACHINE_CONFIG_START( a310, a310_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM, 8000000)        /* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(a310_mem)

	MCFG_AAKART_ADD("kart", 8000000/128, kart_interface) // TODO: frequency
	MCFG_I2CMEM_ADD("i2cmem")
	MCFG_I2CMEM_DATA_SIZE(0x100)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1280, 1024) //TODO: default screen size
	MCFG_SCREEN_VISIBLE_AREA(0*8, 1280 - 1, 0*16, 1024 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(archimedes_state, screen_update)

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("512K, 1M, 4M, 8M, 16M")

	MCFG_WD1772_ADD("wd1772", a310_wd17xx_interface )

	//MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(a310_floppy_interface)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac0", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac3", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac4", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac5", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac6", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac7", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
MACHINE_CONFIG_END

ROM_START(a310)
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "311", "RiscOS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "ic24.rom", 0x000000, 0x80000, CRC(c1adde84) SHA1(12d060e0401dd0523d44453f947bdc55dd2c3240), ROM_BIOS(1) )
	ROMX_LOAD( "ic25.rom", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(1) )
	ROMX_LOAD( "ic26.rom", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(1) )
	ROMX_LOAD( "ic27.rom", 0x180000, 0x80000, CRC(707b0c6c) SHA1(345199a33fed23996374b9db8170a52ab63f0380), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "120", "Arthur/RiscOS 1.20 (25 Sep 1987)" )
	ROMX_LOAD( "arthur.bin",   0x000000, 0x080000, CRC(eb3fda57) SHA1(1181ff9c2c2f3d6d414054ec33b2260404bafc81), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "200", "RiscOS 2.0 (05 Oct 1988)" )
	ROMX_LOAD( "riscos2.bin",  0x000000, 0x080000, CRC(89c4ad36) SHA1(b82a78830dac386f9b649b6d32a34f9c6910546d), ROM_BIOS(3) )

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
ROM_END

ROM_START( a3010 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0296,061-01.ic4" , 0x000000, 0x100000, CRC(b77fe215) SHA1(57b19ea4b97a9b6a240aa61211c2c134cb295aa0))
	ROM_LOAD32_WORD( "0296,062-01.ic15", 0x000002, 0x100000, CRC(d42e196e) SHA1(64243d39d1bca38b10761f66a8042c883bde87a4))
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "battery", ROMREGION_ERASE00 )
	ROM_LOAD( "0296,063-2.ic40", 0x00000, 0x10000, CRC(9ca3a6be) SHA1(75905b031f49960605d55c3e7350d309559ed440))
ROM_END

ROM_START( a3020 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "riscos_3.11_1.bin", 0x000000, 0x100000, CRC(552fc3aa) SHA1(b2f1911e53d7377f2e69e1a870139745d3df494b))
	ROM_LOAD32_WORD( "riscos_3.11_2.bin", 0x000002, 0x100000, CRC(308d5a4a) SHA1(b309e1dd85670a06d77ec504dbbec6c42336329f))
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
ROM_END

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  INIT   COMPANY  FULLNAME */
COMP( 1988, a310, 0,      0,      a310,    a310, a310_state,  a310,   "Acorn", "Archimedes 310", GAME_NOT_WORKING)
COMP( 1988, a3010, a310,  0,      a310,    a310, a310_state,  a310,   "Acorn", "Archimedes 3010", GAME_NOT_WORKING)
COMP( 1988, a3020, a310,  0,      a310,    a310, a310_state,  a310,   "Acorn", "Archimedes 3020", GAME_NOT_WORKING)
