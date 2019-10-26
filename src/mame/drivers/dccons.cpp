// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/*
    dccons.cpp - Sega Dreamcast driver
    by R. Belmont & Angelo Salese

    SH-4 @ 200 MHz
    ARM7DI @ 2.8223 MHz (no T or M extensions)
    PowerVR 3D video
    AICA audio
    GD-ROM drive (modified ATAPI interface)

            NTSC/N  NTSC/I   PAL/N   PAL/I   VGA
        (x/240) (x/480) (x/240)  (x/480) (640x480)
    VTOTAL   262     524      312     624    524
    HTOTAL   857     857      863     863    857

    PCLKs = 26917135 (NTSC 480 @ 59.94), 26944080 (VGA 480 @ 60.0), 13458568 (NTSC 240 @ 59.94),
            25925600 (PAL 480 @ 50.00), 13462800 (PAL 240 @ 50.00)

    TODO:
    - RTC error always pops up at start-up, no flash plus bug with ticks (needs rewrite)
    - Inputs doesn't work most of the time;
    - Candy Stripe: fills the log with "ATAPI_FEATURES_FLAG_OVL not supported", black screen
    - Carrier: Jaleco logo uses YUV, but y size is halved?
    - Close To: Hangs at FMV
    - F355 Challenge: black screen after Sega logo;
    - Gundam - Side Story 0079: currently hangs at Bandai logo (regression)
    - Idol Janshi wo Tsukucchaou: pixel aspect is way wrong (stretched and offsetted horizontally)
    - Power Stone: hangs at Capcom logo;
    - Sega GT: no cursor on main menu;
    - Tetris 4D: hangs at BPS FMV (bp 0C0B0C4E)

    Notes:
    - DC US and DC PAL flash ROMs are definitely hacked, they are set to have Chinese instead of Japanese.
    - 0x1a002 of flash ROM returns the region type (0x30=Japan, 0x31=USA, 0x32=Europe). Amusingly, if the value
      on a non-jp console is different than these ones, the system shows a black swirl (and nothing boots).
    - gdi file for DCLP (a Dreamcast tester) doesn't have first two tracks info, they are:
      1 0 4 2048 FILE0001.DUP 0
      2 1798 0 2352 FILE0002.DUP 0
      serial i/o also fails on that, work ram addresses that needs to be patched with 0x0009 (nop) are:
      0xc0196da
      0xc0196ec
    SH4 TEST:
        UBC test (0101):
            ok
        FPU test (0201):
            NG
        0xc03fe24 work ram flag check (1=error, 0=ok)
        Cache test (03xx):
            Cache Read/Write test (0301)
            NG
            Cache RAM mode Check (0305)
            NG
        MMU test (04xx):
            asserts
        TMU test (0501):
            *_reg check -> ok
            TCNT* reload -> NG
            TCNT* underflow irq -> NG
        MULT test (0601)
            ok
        DIVU test (0701)
            ok
        Store Queue test (0801):
            ok
        SCIF test (0901)
            NG
        Private Instruction test (0a01)
            NG
        Critical test (0dxx)
            Critical (Store Queue) test (0d01):
                ok
            Critical (Write back) test (0d02):
                ok
            Critical (ADD,CMP/EQ) test (0d03):
                ok
            Critical (OC_OIX) test (0d04):
                NG
            Critical (MAX current) test (0d05):
                ok (very slow!)
            Critical (IC Cross Talk) test (0d06):
                NG
            Critical (Cache D-array) test (0d07):
                NG
        SH-4 BUG (0exx)
            SH4_BUG 64bit FMOV
                ok
            SH4 BUG FIX (64bitFMOV)
                ok
    MEM TEST:
        AICA (0102)
            ok
        Work RAM (0204):
            ok
        PV64 area (0303):
            ok
        PV32 area (0403):
            ok
    CLX TEST:
        CLX internal RAM (0101):
            ok
        <Torus> check (0401):
            (sets up RGB888 mode 2, assuming it's critically failed)
    TA TEST:
        TA_YUVINT (0101):
            ok -> IST_EOXFER_YUV
        TA_OENDINT (0102):
            ok -> IST_EOXFER_OPLST
        TA_OMENDINT (0103):
            ok -> IST_EOXFER_OPMV
        TA_TENDINT (0104):
            ok -> IST_EOXFER_TRLST
        TA_TMENDINT (0105):
            ok -> IST_EOXFER_TRMV
        TA_PTENDINT (0106):
            ok -> IST_EOXFER_PTLST
        TA_ISPINT (0107):
            NG -> ISP/TSP Parameter Overflow (error)
        TA_OBJINT (0108):
            NG -> OBJect list pointer Overflow (error)
        TA_IPINT (0109):
            NG -> TA: Illegal parameter (error)
        YUV Converter (0201):
            ok
    DDT i/f TEST:
        Sort, Normal DMA (1) (0101)
            hangs (wants Sort DMA irq, of course)
        Sort, Normal DMA (2)
            ...
        Through
            NG, hangs again
        DC_DEINT (0201)
            ok
        DC_SDTEINT (0202)
            ok
        DC_SDTERINT (0203)
            ok (but returns error count ++, think it's a bug in the SW)
    G2 TEST
        DMA (0101):
            G2 EXT AREA DETECT:
            "!!!! ch00 ERROR DETECT !!!!"
        Interrupt (0301):
            G2 EXT AREA DETECT
            DMA END INT
                hangs
        Ext Interrupt (06xx)
            AICA INT  (0601)
                error detect
            Modem INT (0602)
                error detect
    AICA TEST
        Sound RAM (01xx)
            Pattern R/W check (0101)
                ok
        Register (02xx)
            CH Data (0201)
                ok
            EXT Input (0202)
                ok
            DSP Data (0203)
                ok
        S_Clock (03xx)
            50MSEC (0301)
                NG -> ~0xa58 in 0x702814, must be > 0x889 and < 0x8b0
            25MSEC (0302)
                NG -> ~0x372 in 0x702814, must be > 0x443 and < 0x45a
        Timer (04xx)
            Timer A (0401)
                NG
            Timer B (0402)
                NG
            Timer C (0403)
                NG
        DMA (05xx)
            SRAM -> CH Reg (0501)
                ok
            SRAM -> Comm Reg (0502)
                ok
            SRAM -> DSP Reg (0503)
                ok
            CH Reg -> SRAM (0504)
                ok
            Comm Reg -> SRAM (0505)
                ok
            DSP Reg -> SRAM (0506)
                ok
            Clear SRAM (0507)
                ok
            Clear CH Reg (0508)
                ok
            Clear Comm Reg (0509)
                ok
            Clear DSP Reg (050a)
                ok
        Interrupt (06xx)
            Sampling clock (0601)
                NG (irq 0x400)
            Timer A (0602)
                randomly NG/ok
            Timer B (0603)
                ok
            Timer C (0604)
                ok
            DMA End (0605)
                ok
            Midi Out (0606)
                NG
            Main CPU (0607)
                ok
        RTC (07xx)
            Write Protect (0701)
                ok
            RW Comp (0702)
                ok
            Clock (0703)
                NG
        ARM7 (08xx)
            Load & Start (0801)
                NG
            Timer & Intr (0802)
                NG
            DMA (0803)
                NG
            Ch-Reg R/W (0804)
                ok
            SRAM incr (0805)
                NG
            SRAM pattern (0806)
                ok
        EG (09xx)
            LSA-Reg Left (0901)
                ok/NG
            LSA-Reg Right (0902)
                ok/NG
            LSA-Reg Left & Right (0903)
                ok/NG
        MIDI (0axx)
            OEMP bit (0a01)
                NG
            OFLL bit (0a02)
                NG
    PVRi/f test
        DMA (01xx)
            CPU trig (0101)
                ok
            INT trig
                ok
        Interrupt (02xx)
            PVR DMA end Int (0201)
                ok
            PVR DMA IA Int (0202)
                NG
            PVR DMA end (0203)
                NG
    Flash test:
        (SH-4 jumps to la la land as soon as this is started)


*/

#include "emu.h"
#include "includes/dccons.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "cpu/sh/sh4.h"
#include "imagedev/chd_cd.h"
#include "machine/aicartc.h"
#include "machine/dc-ctrl.h"
#include "machine/gdrom.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "softlist.h"

#define CPU_CLOCK (200000000)

READ64_MEMBER(dc_cons_state::dcus_idle_skip_r )
{
	//if (m_maincpu->pc()==0xc0ba52a)
	//  m_maincpu->spin_until_time(attotime::from_usec(2500));
	//  device_spinuntil_int(m_maincpu);

	return dc_ram[0x2303b0/8];
}

READ64_MEMBER(dc_cons_state::dcjp_idle_skip_r )
{
	//if (m_maincpu->pc()==0xc0bac62)
	//  m_maincpu->spin_until_time(attotime::from_usec(2500));
	//  device_spinuntil_int(m_maincpu);

	return dc_ram[0x2302f8/8];
}

void dc_cons_state::init_dc()
{
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x001fffff, true, memregion("maincpu")->base());
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0cffffff, false, dc_ram);
	dreamcast_atapi_init();
}

void dc_cons_state::init_dcus()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2303b0, 0xc2303b7, read64_delegate(*this, FUNC(dc_cons_state::dcus_idle_skip_r)));

	init_dc();
}

void dc_cons_state::init_dcjp()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2302f8, 0xc2302ff, read64_delegate(*this, FUNC(dc_cons_state::dcjp_idle_skip_r)));

	init_dc();
}

void dc_cons_state::init_tream()
{
	// Modchip connected to BIOS ROM chip changes 4 bytes (actually bits) as shown below, which allow to boot any region games.
	u8 *rom = (u8 *)memregion("maincpu")->base();
	rom[0x503] |= 0x40;
	rom[0x50f] |= 0x40;
	rom[0x523] |= 0x40;
	rom[0x531] |= 0x40;

	init_dcus();
}

READ64_MEMBER(dc_cons_state::dc_pdtra_r )
{
	uint64_t out = PCTRA<<32;

	out |= PDTRA & ~0x0303;

	// if both bits are inputs
	if (!(PCTRA & 0x5))
	{
		out |= 0x03;
	}

	// one's input one's output, always pull up both bits
	if (((PCTRA & 5) == 1) || ((PCTRA & 5) == 4))
	{
		if (PDTRA & 3)
		{
			out |= 0x03;
		}
	}

	/*
	cable setting, (0) VGA, (2) TV RGB (3) TV VBS/Y + S/C.
	Note: several games doesn't like VGA setting (i.e. Idol Janshi wo Tsukucchaou, Airforce Delta), default to composite.
	*/
	out |= ioport("SCREEN_TYPE")->read() << 8;

	return out;
}

WRITE64_MEMBER(dc_cons_state::dc_pdtra_w )
{
	PCTRA = (data>>16) & 0xffff;
	PDTRA = (data & 0xffff);
}

#if 0
READ8_MEMBER(dc_cons_state::dc_flash_r)
{
	return m_dcflash->read(offset);
}

WRITE8_MEMBER(dc_cons_state::dc_flash_w)
{
	m_dcflash->write(offset,data);
}
#endif

void dc_cons_state::dc_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().nopw();             // BIOS
	map(0x00200000, 0x0021ffff).rom().region("dcflash", 0);//.rw(FUNC(dc_cons_state::dc_flash_r), FUNC(dc_cons_state::dc_flash_w));
	map(0x005f6800, 0x005f69ff).rw(FUNC(dc_cons_state::dc_sysctrl_r), FUNC(dc_cons_state::dc_sysctrl_w));
	map(0x005f6c00, 0x005f6cff).m(m_maple, FUNC(maple_dc_device::amap));
	map(0x005f7000, 0x005f701f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)).umask64(0x0000ffff0000ffff);
	map(0x005f7080, 0x005f709f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask64(0x0000ffff0000ffff);
	map(0x005f7400, 0x005f74ff).rw(FUNC(dc_cons_state::dc_mess_g1_ctrl_r), FUNC(dc_cons_state::dc_mess_g1_ctrl_w));
	map(0x005f7800, 0x005f78ff).rw(FUNC(dc_cons_state::dc_g2_ctrl_r), FUNC(dc_cons_state::dc_g2_ctrl_w));
	map(0x005f7c00, 0x005f7cff).m(m_powervr2, FUNC(powervr2_device::pd_dma_map));
	map(0x005f8000, 0x005f9fff).m(m_powervr2, FUNC(powervr2_device::ta_map));
	map(0x00600000, 0x006007ff).rw(FUNC(dc_cons_state::dc_modem_r), FUNC(dc_cons_state::dc_modem_w));
	map(0x00700000, 0x00707fff).rw(FUNC(dc_cons_state::dc_aica_reg_r), FUNC(dc_cons_state::dc_aica_reg_w));
	map(0x00710000, 0x0071000f).mirror(0x02000000).rw("aicartc", FUNC(aicartc_device::read), FUNC(aicartc_device::write)).umask64(0x0000ffff0000ffff);
	map(0x00800000, 0x009fffff).rw(FUNC(dc_cons_state::soundram_r), FUNC(dc_cons_state::soundram_w));
//  map(0x01000000, 0x01ffffff) G2 Ext Device #1
//  map(0x02700000, 0x02707fff) AICA reg mirror
//  map(0x02800000, 0x02ffffff) AICA wave mem mirror

//  map(0x03000000, 0x03ffffff) G2 Ext Device #2

	/* Area 1 */
	map(0x04000000, 0x04ffffff).ram().share("dc_texture_ram");      // texture memory 64 bit access
	map(0x05000000, 0x05ffffff).ram().share("frameram"); // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 3 */
	map(0x0c000000, 0x0cffffff).ram().share("dc_ram");
	map(0x0d000000, 0x0dffffff).ram().share("dc_ram");// extra ram on Naomi (mirror on DC)
	map(0x0e000000, 0x0effffff).ram().share("dc_ram");// mirror
	map(0x0f000000, 0x0fffffff).ram().share("dc_ram");// mirror

	/* Area 4 */
	map(0x10000000, 0x107fffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_poly_w));
	map(0x10800000, 0x10ffffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_yuv_w));
	map(0x11000000, 0x117fffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath0_w)).mirror(0x00800000);  // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue

	map(0x12000000, 0x127fffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_poly_w));
	map(0x12800000, 0x12ffffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_yuv_w));
	map(0x13000000, 0x137fffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath1_w)).mirror(0x00800000); // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue

//  map(0x14000000, 0x17ffffff) G2 Ext Device #3

	map(0x8c000000, 0x8cffffff).ram().share("dc_ram");  // another RAM mirror

	map(0xa0000000, 0xa01fffff).rom().region("maincpu", 0);
}

void dc_cons_state::dc_port(address_map &map)
{
	map(0x00000000, 0x00000007).rw(FUNC(dc_cons_state::dc_pdtra_r), FUNC(dc_cons_state::dc_pdtra_w));
}

void dc_cons_state::dc_audio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x001fffff).rw(FUNC(dc_cons_state::soundram_r), FUNC(dc_cons_state::soundram_w));        /* shared with SH-4 */
	map(0x00800000, 0x00807fff).rw(FUNC(dc_cons_state::dc_arm_aica_r), FUNC(dc_cons_state::dc_arm_aica_w));
}

void dc_cons_state::aica_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).ram().share("dc_sound_ram");
}

static INPUT_PORTS_START( dc )
	PORT_START("P1:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 C")

	PORT_START("P1:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 Z")

	PORT_START("P1:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P1 R") PORT_PLAYER(1)

	PORT_START("P1:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P1 L") PORT_PLAYER(1)

	PORT_START("P1:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(1)

	PORT_START("P1:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(1)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("P2:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P2 C")

	PORT_START("P2:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 Z")

	PORT_START("P2:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P2 R") PORT_PLAYER(2)

	PORT_START("P2:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P2 L") PORT_PLAYER(2)

	PORT_START("P2:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(2)

	PORT_START("P2:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(2)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("P3:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("P3 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 C")

	PORT_START("P3:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("P3 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 Z")

	PORT_START("P3:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P3 R") PORT_PLAYER(3)

	PORT_START("P3:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P3 L") PORT_PLAYER(3)

	PORT_START("P3:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(3)

	PORT_START("P3:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(3)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("P4:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("P4 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P4 C")

	PORT_START("P4:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("P4 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 Z")

	PORT_START("P4:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P4 R") PORT_PLAYER(4)

	PORT_START("P4:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P4 L") PORT_PLAYER(4)

	PORT_START("P4:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(4)

	PORT_START("P4:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(4)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("MAMEDEBUG")
	PORT_CONFNAME( 0x01, 0x00, "Bilinear Filtering" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START("SCREEN_TYPE")
	PORT_CONFNAME( 0x03, 0x03, "Screen Connection Type" )
	PORT_CONFSETTING(    0x00, "VGA" )
	PORT_CONFSETTING(    0x02, "Composite" )
	PORT_CONFSETTING(    0x03, "S-Video" )
INPUT_PORTS_END

void dc_cons_state::gdrom_config(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(0, "^^aica", 1.0);
	cdda->add_route(1, "^^aica", 1.0);
}

void dc_cons_state::dc(machine_config &config)
{
	/* basic machine hardware */
	SH4LE(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &dc_cons_state::dc_map);
	m_maincpu->set_addrmap(AS_IO, &dc_cons_state::dc_port);

	TIMER(config, "scantimer").configure_scanline(FUNC(dc_state::dc_scanline), "screen", 0, 1);

	ARM7(config, m_soundcpu, ((XTAL(33'868'800)*2)/3)/8);   // AICA bus clock is 2/3rds * 33.8688.  ARM7 gets 1 bus cycle out of each 8.
	m_soundcpu->set_addrmap(AS_PROGRAM, &dc_cons_state::dc_audio_map);

	MCFG_MACHINE_RESET_OVERRIDE(dc_cons_state,dc_console )

//  MACRONIX_29LV160TMC(config, "dcflash");

	MAPLE_DC(config, m_maple, 0, m_maincpu);
	m_maple->irq_callback().set(FUNC(dc_state::maple_irq));
	dc_controller_device &dcctrl0(DC_CONTROLLER(config, "dcctrl0", 0, m_maple, 0));
	dcctrl0.set_port_tags("P1:0", "P1:1", "P1:A0", "P1:A1", "P1:A2", "P1:A3", "P1:A4", "P1:A5");
	dc_controller_device &dcctrl1(DC_CONTROLLER(config, "dcctrl1", 0, m_maple, 1));
	dcctrl1.set_port_tags("P2:0", "P2:1", "P2:A0", "P2:A1", "P2:A2", "P2:A3", "P2:A4", "P2:A5");
	dc_controller_device &dcctrl2(DC_CONTROLLER(config, "dcctrl2", 0, m_maple, 2));
	dcctrl2.set_port_tags("P3:0", "P3:1", "P3:A0", "P3:A1", "P3:A2", "P3:A3", "P3:A4", "P3:A5");
	dc_controller_device &dcctrl3(DC_CONTROLLER(config, "dcctrl3", 0, m_maple, 3));
	dcctrl3.set_port_tags("P4:0", "P4:1", "P4:A0", "P4:A1", "P4:A2", "P4:A3", "P4:A4", "P4:A5");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(13458568*2, 857, 0, 640, 524, 0, 480); /* TODO: where pclk actually comes? */
	screen.set_screen_update("powervr2", FUNC(powervr2_device::screen_update));
	PALETTE(config, "palette").set_entries(0x1000);
	POWERVR2(config, m_powervr2, 0);
	m_powervr2->irq_callback().set(FUNC(dc_state::pvr_irq));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	AICA(config, m_aica, (XTAL(33'868'800)*2)/3); // 67.7376MHz(2*33.8688MHz), div 3 for audio block
	m_aica->irq().set(FUNC(dc_state::aica_irq));
	m_aica->main_irq().set(FUNC(dc_state::sh4_aica_irq));
	m_aica->set_addrmap(0, &dc_cons_state::aica_map);
	m_aica->add_route(0, "lspeaker", 1.0);
	m_aica->add_route(1, "rspeaker", 1.0);

	AICARTC(config, "aicartc", XTAL(32'768));

	ATA_INTERFACE(config, m_ata, 0);
	m_ata->irq_handler().set(FUNC(dc_cons_state::ata_interrupt));

	ata_slot_device &ata_0(*subdevice<ata_slot_device>("ata:0"));
	ata_0.option_add("gdrom", GDROM);
	ata_0.set_option_machine_config("gdrom", gdrom_config);
	ata_0.set_default_option("gdrom");

	SOFTWARE_LIST(config, "cd_list").set_original("dc");
}


#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

// known undumped or private BIOS revisions:
// "MPR-21068 SEGA JAPAN / 9850 D" from VA0 837-13392-02 (171-7782B) NTSC-J unit
// KABUTO Ver.1.011 CRC 34DA5C88 from pre-release US unit (private)

// MPR-21933-X2 confirmed match MPR-21931 (v1.01d)

// actual mask rom labels can have -X1 or -X2 added, presumable depending on mobo revision and/or type of rom used (5v or 3.3v), contents is the same

#define DREAMCAST_COMMON_BIOS \
	ROM_REGION(0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS(0, "101d", "v1.01d (World)") \
	ROM_LOAD_BIOS(0, "mpr-21931.ic501", 0x000000, 0x200000, CRC(89f2b1a1) SHA1(8951d1bb219ab2ff8583033d2119c899cc81f18c) ) \
	ROM_SYSTEM_BIOS(1, "1022", "v1.022 (World)") \
	ROM_LOAD_BIOS(1, "mpr-23588.ic501", 0x000000, 0x200000, CRC(786168f9) SHA1(ba8bbb90fdb29525f24f17055dc2c7b2d7674437) ) \
	ROM_SYSTEM_BIOS(2, "101c", "v1.01c (World)") \
	ROM_LOAD_BIOS(2, "mpr-21871.ic501", 0x000000, 0x200000, CRC(2f551bc5) SHA1(1ede8d5be49116a4c6f3fe0961175469537a0434) ) \
	ROM_SYSTEM_BIOS(3, "101dch", "v1.01d (Chinese hack)") \
	ROM_LOAD_BIOS(3, "dc101d_ch.bin",   0x000000, 0x200000, CRC(a2564fad) SHA1(edc5d3d70a93c935703d26119b37731fd317d2bf) )
// ^^^ dc101d_eu.bin ^^^ is selfmade chinese translation, doesn't work on real hardware, does it must be here at all ?

/* note: Dreamcast Flash ROMs actually 256KB MBM29F002TC (VA0) or MBM29LV002TC (VA1) devices, only 2nd 128KB half is used, A17 pin tied to VCC
   sector SA5 (1A000 - 1BFFF) is read-only, contain information written during manufacture or repair, fully generated by software tool (except predefined list of creators)
struct factory_sector
{
    struct factory_record {
        // everything 'char' below is decimal numbers in ASCII, unless noted else
        char machine_code1; // '0' - Dreamcast, 0xFF - dev.box
        char machine_code2; // '0' - Dreamcast, 0xFF - dev.box
        char country_code;  // 0 - Japan, 1 - America, 2 - Europe
        char language;      // 0 - Japanese, 1 - English, etc
        char broadcast_format;  // 0 - NTSC, 1 - PAL, 2 - PAL-M, 3 - PAL-N
        char machine_name[32];  // ASCII text 'Dreamcast', trail is 0x20 filled
        char tool_number[4];    // software tool #
        char tool_version[2];   // software tool version
        char tool_type[2];  // software tool type: 0 - for MP(mass production?), 1 - for Repair, 2 - for PP
        char year[4];
        char month[2];
        char day[2];
        char hour[2];
        char min[2];
        char serial_number[8];
        char factory_code[4];
        char total_number[16];
        uint8_t sum;        // byte sum of above
        uint8_t machine_id[8];  // 64bit UID
        uint8_t machine_type;   // FF - Dreamcast
        uint8_t machine_version;// FF - VA0, FE - VA1, FD - VA2, NOTE: present in 1st factory record only, in 2nd always FF
        uint8_t unused[0x40]    // FF filled
    } factory_records[2];       // 2 copies
    uint8_t unused_0[0x36];     // FF filled
    uint8_t unk_version;        // not clear if hardware or bios version, A0 - VA0, 9F - VA1, 9E - VA2
    uint8_t unused_1[9];        // FF filled
    char staff_roll[0xca0];     // list of creators
    uint8_t unused_2[0x420];    // FF filled
    uint8_t random[0xdc0];      // output of RNG {static u32 seed; seed=(seed*0x83d+0x2439)&0x7fff; return (u16)(seed+0xc000);}, where initial seed value is serial_number[7] & 0xf
};

Besides factory sector, each new Dreamcast have "Flash Partition 2" header in SA6 (@1C000) followed by "CID" record:
struct cid_record
{
    uint16_t record_type;           // 0, can be 0-4
    struct cid_data
    {
        uint8_t date[4];            // BCD YYYY/MM/DD
        char t_inferior_code[4];    // '0'-filled in all dumps we have
        char repair_voucher_no[8];  // '0'-filled in all dumps we have
        uint8_t serial_no[8];
        uint8_t factory_code;
        uint8_t order_no[5];
    } cid[2];
    uint16_t crc16;
};
*/

ROM_START(dc)
	DREAMCAST_COMMON_BIOS

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "dcus_ntsc.bin", 0x000000, 0x020000, CRC(4136c25b) SHA1(1efa00ab9d8357a9f91e5be931a3efd6236f2b79) )  // dumped from VA2.4 mobo with 1.022 BIOS
ROM_END

ROM_START( dceu )
	DREAMCAST_COMMON_BIOS

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "dceu_pal.bin",  0x000000, 0x020000, CRC(7a102d05) SHA1(13e444e613dffe0a8bce073a01efa9a1d4626ba7) ) // VA1
	ROM_LOAD( "dceu_pala.bin", 0x000000, 0x020000, CRC(2e8dfa07) SHA1(ca5fd977bbf8f48c28c1027a023b038123d57d39) ) // from VA1 with 1.01d BIOS
ROM_END

ROM_START( dcjp )
	DREAMCAST_COMMON_BIOS
	ROM_SYSTEM_BIOS(4, "1004", "v1.004 (Japan)")    // oldest known mass production version, supports Japan region only
	ROM_LOAD_BIOS(4, "mpr-21068.ic501", 0x000000, 0x200000, CRC(5454841f) SHA1(1ea132c0fbbf07ef76789eadc07908045c089bd6) )

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "dcjp_ntsc.bin", 0x000000, 0x020000, CRC(306023ab) SHA1(5fb66adb6d1b54a552fe9c2bb736e4c6960e447d) ) // from refurbished VA0 with 1.004 BIOS
ROM_END

// unauthorised portable modification
ROM_START( dctream )
	ROM_REGION(0x200000, "maincpu", 0)
	// uses regular mpr-21931 BIOS chip, have region-free mod-chip installed, see driver init.
	ROM_LOAD( "mpr-21931.ic501", 0x000000, 0x200000, CRC(89f2b1a1) SHA1(8951d1bb219ab2ff8583033d2119c899cc81f18c) )

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "dc_flash.bin", 0x000000, 0x020000, CRC(9d5515c4) SHA1(78a86fd4e8b58fc9d3535eef6591178f1b97ecf9) ) // VA1 NTSC-US
ROM_END

// normally, with DIP switch 4 off, HKT-0100/0110/0120 AKA "Katana Set 5.xx", will be booted from flash ROM IC507 (first 2 dumps below)
// otherwise it boots from EPROM which contain system checker software (last 2 dumps)
ROM_START( dcdev )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "1011", "Katana Set5 v1.011 (World)")    // BOOT flash rom update from Katana SDK R9-R11, WinCE SDK v2.1
	ROM_LOAD_BIOS(0, "set5v1.011.ic507", 0x000000, 0x200000, CRC(2186e0e5) SHA1(6bd18fb83f8fdb56f1941e079580e5dd672a6dad) )
	ROM_SYSTEM_BIOS(1, "1001", "Katana Set5 v1.001 (Japan)")    // BOOT flash rom update from WinCE SDK v1.0
	ROM_LOAD_BIOS(1, "set5v1.001.ic507", 0x000000, 0x200000, CRC(5702d38f) SHA1(ea7a3ae1de73683008dd795c252941a4fc81b42e) )

	// 27C160 EPROM (DIP42) IC??? labeled
	// SET5 7676
	// V0.71 98/11/13
	ROM_SYSTEM_BIOS(2, "071", "Katana Set5 Checker v0.71")
	ROM_LOAD_BIOS(2, "set5v0.71.bin", 0x000000, 0x200000, CRC(52d01969) SHA1(28aec4a01419d2d2a664c540bef30ea289ca0644) )
	// SET5 FC52
	// V0.41 98/08/27
	ROM_SYSTEM_BIOS(3, "041", "Katana Set5 Checker v0.41")
	ROM_LOAD_BIOS(3, "set5v0.41.bin", 0x000000, 0x200000, CRC(485877bd) SHA1(dc1af1f1248ffa87d57bc5ef2ea41aac95ecfc5e) )

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "hkt-0120-flash.bin", 0x000000, 0x020000, CRC(7784c304) SHA1(31ef57f550d8cd13e40263cbc657253089e53034) ) // Dev.Boxes have empty (FF filled) flash ROM
ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT       COMPANY FULLNAME */
CONS( 1999, dc,      dcjp,   0,      dc,      dc,    dc_cons_state, init_dcus, "Sega", "Dreamcast (USA, NTSC)", MACHINE_NOT_WORKING )
CONS( 1998, dcjp,    0,      0,      dc,      dc,    dc_cons_state, init_dcjp, "Sega", "Dreamcast (Japan, NTSC)", MACHINE_NOT_WORKING )
CONS( 1999, dceu,    dcjp,   0,      dc,      dc,    dc_cons_state, init_dcus, "Sega", "Dreamcast (Europe, PAL)", MACHINE_NOT_WORKING )
CONS( 200?, dctream, dcjp,   0,      dc,      dc,    dc_cons_state, init_tream,"<unknown>", "Treamcast", MACHINE_NOT_WORKING )
CONS( 1998, dcdev,   0,      0,      dc,      dc,    dc_cons_state, init_dc,   "Sega", "HKT-0120 Sega Dreamcast Development Box", MACHINE_NOT_WORKING )
