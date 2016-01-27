// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/*

    dc.c - Sega Dreamcast driver
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
#include "cpu/arm7/arm7.h"
#include "cpu/sh4/sh4.h"
#include "cpu/arm7/arm7core.h"
#include "sound/aica.h"
#include "machine/aicartc.h"
#include "includes/dc.h"
#include "includes/dccons.h"
#include "imagedev/chd_cd.h"
#include "machine/dc-ctrl.h"
#include "machine/gdrom.h"

#define CPU_CLOCK (200000000)

READ64_MEMBER(dc_cons_state::dcus_idle_skip_r )
{
	//if (space.device().safe_pc()==0xc0ba52a)
	//  space.device().execute().spin_until_time(attotime::from_usec(2500));
	//  device_spinuntil_int(&space.device());

	return dc_ram[0x2303b0/8];
}

READ64_MEMBER(dc_cons_state::dcjp_idle_skip_r )
{
	//if (space.device().safe_pc()==0xc0bac62)
	//  space.device().execute().spin_until_time(attotime::from_usec(2500));
	//  device_spinuntil_int(&space.device());

	return dc_ram[0x2302f8/8];
}

DRIVER_INIT_MEMBER(dc_cons_state,dc)
{
	dreamcast_atapi_init();
}

DRIVER_INIT_MEMBER(dc_cons_state,dcus)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2303b0, 0xc2303b7, read64_delegate(FUNC(dc_cons_state::dcus_idle_skip_r),this));

	DRIVER_INIT_CALL(dc);
}

DRIVER_INIT_MEMBER(dc_cons_state,dcjp)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc2302f8, 0xc2302ff, read64_delegate(FUNC(dc_cons_state::dcjp_idle_skip_r),this));

	DRIVER_INIT_CALL(dc);
}

READ64_MEMBER(dc_cons_state::dc_pdtra_r )
{
	UINT64 out = PCTRA<<32;

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

READ64_MEMBER(dc_cons_state::dc_arm_r )
{
	return *((UINT64 *)dc_sound_ram.target()+offset);
}

WRITE64_MEMBER(dc_cons_state::dc_arm_w )
{
	COMBINE_DATA((UINT64 *)dc_sound_ram.target() + offset);
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

static ADDRESS_MAP_START( dc_map, AS_PROGRAM, 64, dc_cons_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_WRITENOP             // BIOS
	AM_RANGE(0x00200000, 0x0021ffff) AM_ROM AM_REGION("dcflash",0)//AM_READWRITE8(dc_flash_r,dc_flash_w, U64(0xffffffffffffffff))
	AM_RANGE(0x005f6800, 0x005f69ff) AM_READWRITE(dc_sysctrl_r, dc_sysctrl_w )
	AM_RANGE(0x005f6c00, 0x005f6cff) AM_DEVICE32( "maple_dc", maple_dc_device, amap, U64(0xffffffffffffffff) )
	AM_RANGE(0x005f7000, 0x005f701f) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs1, write_cs1, U64(0x0000ffff0000ffff) )
	AM_RANGE(0x005f7080, 0x005f709f) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs0, write_cs0, U64(0x0000ffff0000ffff) )
	AM_RANGE(0x005f7400, 0x005f74ff) AM_READWRITE32(dc_mess_g1_ctrl_r, dc_mess_g1_ctrl_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x005f7800, 0x005f78ff) AM_READWRITE(dc_g2_ctrl_r, dc_g2_ctrl_w )
	AM_RANGE(0x005f7c00, 0x005f7cff) AM_DEVICE32("powervr2", powervr2_device, pd_dma_map, U64(0xffffffffffffffff))
	AM_RANGE(0x005f8000, 0x005f9fff) AM_DEVICE32("powervr2", powervr2_device, ta_map, U64(0xffffffffffffffff))
	AM_RANGE(0x00600000, 0x006007ff) AM_READWRITE(dc_modem_r, dc_modem_w )
	AM_RANGE(0x00700000, 0x00707fff) AM_READWRITE32(dc_aica_reg_r, dc_aica_reg_w, U64(0xffffffffffffffff))
	AM_RANGE(0x00710000, 0x0071000f) AM_MIRROR(0x02000000) AM_DEVREADWRITE16("aicartc", aicartc_device, read, write, U64(0x0000ffff0000ffff) )
	AM_RANGE(0x00800000, 0x009fffff) AM_READWRITE(dc_arm_r, dc_arm_w )
//  AM_RANGE(0x01000000, 0x01ffffff) G2 Ext Device #1
//  AM_RANGE(0x02700000, 0x02707fff) AICA reg mirror
//  AM_RANGE(0x02800000, 0x02ffffff) AICA wave mem mirror

//  AM_RANGE(0x03000000, 0x03ffffff) G2 Ext Device #2

	/* Area 1 */
	AM_RANGE(0x04000000, 0x04ffffff) AM_RAM AM_SHARE("dc_texture_ram")      // texture memory 64 bit access
	AM_RANGE(0x05000000, 0x05ffffff) AM_RAM AM_SHARE("frameram") // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 3 */
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM AM_SHARE("dc_ram")
	AM_RANGE(0x0d000000, 0x0dffffff) AM_RAM AM_SHARE("dc_ram")// extra ram on Naomi (mirror on DC)
	AM_RANGE(0x0e000000, 0x0effffff) AM_RAM AM_SHARE("dc_ram")// mirror
	AM_RANGE(0x0f000000, 0x0fffffff) AM_RAM AM_SHARE("dc_ram")// mirror

	/* Area 4 */
	AM_RANGE(0x10000000, 0x107fffff) AM_DEVWRITE("powervr2", powervr2_device, ta_fifo_poly_w)
	AM_RANGE(0x10800000, 0x10ffffff) AM_DEVWRITE8("powervr2", powervr2_device, ta_fifo_yuv_w, U64(0xffffffffffffffff))
	AM_RANGE(0x11000000, 0x117fffff) AM_DEVWRITE("powervr2", powervr2_device, ta_texture_directpath0_w) AM_MIRROR(0x00800000)  // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue

	AM_RANGE(0x12000000, 0x127fffff) AM_DEVWRITE("powervr2", powervr2_device, ta_fifo_poly_w)
	AM_RANGE(0x12800000, 0x12ffffff) AM_DEVWRITE8("powervr2", powervr2_device, ta_fifo_yuv_w, U64(0xffffffffffffffff))
	AM_RANGE(0x13000000, 0x137fffff) AM_DEVWRITE("powervr2", powervr2_device, ta_texture_directpath1_w) AM_MIRROR(0x00800000) // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue

//  AM_RANGE(0x14000000, 0x17ffffff) G2 Ext Device #3

	AM_RANGE(0x8c000000, 0x8cffffff) AM_RAM AM_SHARE("dc_ram")  // another RAM mirror

	AM_RANGE(0xa0000000, 0xa01fffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dc_port, AS_IO, 64, dc_cons_state )
	AM_RANGE(0x00000000, 0x00000007) AM_READWRITE(dc_pdtra_r, dc_pdtra_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( dc_audio_map, AS_PROGRAM, 32, dc_cons_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("dc_sound_ram")        /* shared with SH-4 */
	AM_RANGE(0x00800000, 0x00807fff) AM_READWRITE(dc_arm_aica_r, dc_arm_aica_w)
ADDRESS_MAP_END

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
	PORT_DIPNAME( 0x01, 0x00, "Bilinear Filtering" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_START("SCREEN_TYPE")
	PORT_CONFNAME( 0x03, 0x03, "Screen Connection Type" )
	PORT_CONFSETTING(    0x00, "VGA" )
	PORT_CONFSETTING(    0x02, "Composite" )
	PORT_CONFSETTING(    0x03, "S-Video" )
INPUT_PORTS_END

MACHINE_RESET_MEMBER(dc_cons_state,dc_console)
{
	dc_state::machine_reset();
	m_aica->set_ram_base(dc_sound_ram, 2*1024*1024);
}

WRITE_LINE_MEMBER(dc_cons_state::aica_irq)
{
	m_soundcpu->set_input_line(ARM7_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(dc_cons_state::sh4_aica_irq)
{
	if(state)
		dc_sysctrl_regs[SB_ISTEXT] |= IST_EXT_AICA;
	else
		dc_sysctrl_regs[SB_ISTEXT] &= ~IST_EXT_AICA;

	dc_update_interrupt_status();
}

static MACHINE_CONFIG_FRAGMENT( gdrom_config )
	MCFG_DEVICE_MODIFY("cdda")
	MCFG_SOUND_ROUTE(0, "^^^^lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "^^^^rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dc, dc_cons_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4LE, CPU_CLOCK)
	MCFG_SH4_MD0(1)
	MCFG_SH4_MD1(0)
	MCFG_SH4_MD2(1)
	MCFG_SH4_MD3(0)
	MCFG_SH4_MD4(0)
	MCFG_SH4_MD5(1)
	MCFG_SH4_MD6(0)
	MCFG_SH4_MD7(1)
	MCFG_SH4_MD8(0)
	MCFG_SH4_CLOCK(CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(dc_map)
	MCFG_CPU_IO_MAP(dc_port)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", dc_state, dc_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("soundcpu", ARM7, ((XTAL_33_8688MHz*2)/3)/8)   // AICA bus clock is 2/3rds * 33.8688.  ARM7 gets 1 bus cycle out of each 8.
	MCFG_CPU_PROGRAM_MAP(dc_audio_map)

	MCFG_MACHINE_RESET_OVERRIDE(dc_cons_state,dc_console )

//  MCFG_MACRONIX_29LV160TMC_ADD("dcflash")

	MCFG_MAPLE_DC_ADD( "maple_dc", "maincpu", dc_maple_irq )
	MCFG_DC_CONTROLLER_ADD("dcctrl0", "maple_dc", 0, ":P1:0", ":P1:1", ":P1:A0", ":P1:A1", ":P1:A2", ":P1:A3", ":P1:A4", ":P1:A5")
	MCFG_DC_CONTROLLER_ADD("dcctrl1", "maple_dc", 1, ":P2:0", ":P2:1", ":P2:A0", ":P2:A1", ":P2:A2", ":P2:A3", ":P2:A4", ":P2:A5")
	MCFG_DC_CONTROLLER_ADD("dcctrl2", "maple_dc", 2, ":P3:0", ":P3:1", ":P3:A0", ":P3:A1", ":P3:A2", ":P3:A3", ":P3:A4", ":P3:A5")
	MCFG_DC_CONTROLLER_ADD("dcctrl3", "maple_dc", 3, ":P4:0", ":P4:1", ":P4:A0", ":P4:A1", ":P4:A2", ":P4:A3", ":P4:A4", ":P4:A5")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(13458568*2, 857, 0, 640, 524, 0, 480) /* TODO: where pclk actually comes? */
	MCFG_SCREEN_UPDATE_DEVICE("powervr2", powervr2_device, screen_update)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_POWERVR2_ADD("powervr2", WRITE8(dc_state, pvr_irq))

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("aica", AICA, 0)
	MCFG_AICA_MASTER
	MCFG_AICA_IRQ_CB(WRITELINE(dc_cons_state, aica_irq))
	MCFG_AICA_MAIN_IRQ_CB(WRITELINE(dc_cons_state, sh4_aica_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)

	MCFG_AICARTC_ADD("aicartc", XTAL_32_768kHz)

	MCFG_DEVICE_ADD("ata", ATA_INTERFACE, 0)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(dc_cons_state, ata_interrupt))

	MCFG_DEVICE_MODIFY("ata:0")
	MCFG_SLOT_OPTION_ADD("gdrom", GDROM)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("gdrom", gdrom_config)
	MCFG_SLOT_DEFAULT_OPTION("gdrom")
MACHINE_CONFIG_END


#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1))

// actual mask rom labels can have -X1 or -X2 added, presumable depending on mobo revision and/or type of rom used (5v or 3.3v), contents is the same
// known to exists undumped MPR-21086 (VA0 NTSC-J 3010) and MPR-21933 (VA0 US) boot roms

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

ROM_START(dc)
	DREAMCAST_COMMON_BIOS

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "dcus_ntsc.bin", 0x000000, 0x020000, BAD_DUMP CRC(e6862dd0) SHA1(24875ce85c011600e73b1c3fd2b341824cbf8544) )  // dumped from VA2.4 mobo with 1.022 BIOS
ROM_END

ROM_START( dceu )
	DREAMCAST_COMMON_BIOS

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "dceu_pal.bin", 0x000000, 0x020000, BAD_DUMP CRC(b7e5aeeb) SHA1(11e02433e13b793ec7ffe0ae2356750bb8a575b4) )
ROM_END

ROM_START( dcjp )
	DREAMCAST_COMMON_BIOS
	ROM_SYSTEM_BIOS(4, "1004", "v1.004 (Japan)")    // oldest known mass production version, supports Japan region only
	ROM_LOAD_BIOS(4, "mpr-21068.ic501", 0x000000, 0x200000, CRC(5454841f) SHA1(1ea132c0fbbf07ef76789eadc07908045c089bd6) )

	ROM_REGION(0x020000, "dcflash", 0)
	/* ROM_LOAD( "dcjp_ntsc.bad", 0x000000, 0x020000, BAD_DUMP CRC(307a7035) SHA1(1411423a9d071340ea52c56e19c1aafc4e1309ee) )      // Hacked Flash */
	ROM_LOAD( "dcjp_ntsc.bin", 0x000000, 0x020000, BAD_DUMP CRC(5f92bf76) SHA1(be78b834f512ab2cf3d67b96e377c9f3093ff82a) )         // hacked PAL flash
	ROM_FILL( 0x1a004, 1, 0x30 ) // patch broadcast back to NTSC
ROM_END

// normally, with DIP switch 4 off, HKT-100/110/120 AKA "Katana Set 5.xx", will be booted from flash ROM IC507 (first 2 dumps below)
// otherwise it boots from EPROM which contain system checker software (last dump)
ROM_START( dcdev )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "1011", "Katana Set5 v1.011 (World)")    // BOOT flash rom update from Katana SDK R9-R11, WinCE SDK v2.1
	ROM_LOAD_BIOS(0, "set5v1.011.ic507", 0x000000, 0x200000, CRC(2186e0e5) SHA1(6bd18fb83f8fdb56f1941e079580e5dd672a6dad) )
	ROM_SYSTEM_BIOS(1, "1001", "Katana Set5 v1.001 (Japan)")    // BOOT flash rom update from WinCE SDK v1.0
	ROM_LOAD_BIOS(1, "set5v1.001.ic507", 0x000000, 0x200000, CRC(5702d38f) SHA1(ea7a3ae1de73683008dd795c252941a4fc81b42e) )

	// 27C160 EPROM (DIP42) IC??? labeled
	// SET5 FC52
	// V0.41 98/08/27
	// also known to exists v0.71 98/11/13
	ROM_SYSTEM_BIOS(2, "041", "Katana Set5 Checker v0.41")
	ROM_LOAD_BIOS(2, "set5v0.41.bin", 0x000000, 0x200000, CRC(485877bd) SHA1(dc1af1f1248ffa87d57bc5ef2ea41aac95ecfc5e) )

	ROM_REGION(0x020000, "dcflash", 0)
	ROM_LOAD( "hkt-0120-flash.bin", 0x000000, 0x020000, CRC(7784c304) SHA1(31ef57f550d8cd13e40263cbc657253089e53034) )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT      COMPANY FULLNAME */
CONS( 1999, dc,     dcjp,   0,      dc,     dc, dc_cons_state,     dcus,   "Sega", "Dreamcast (USA, NTSC)", MACHINE_NOT_WORKING )
CONS( 1998, dcjp,   0,      0,      dc,     dc, dc_cons_state,     dcjp,   "Sega", "Dreamcast (Japan, NTSC)", MACHINE_NOT_WORKING )
CONS( 1999, dceu,   dcjp,   0,      dc,     dc, dc_cons_state,     dcus,   "Sega", "Dreamcast (Europe, PAL)", MACHINE_NOT_WORKING )
CONS( 1998, dcdev,  0,      0,      dc,     dc, dc_cons_state,     dc,     "Sega", "HKT-0120 Sega Dreamcast Development Box", MACHINE_NOT_WORKING )
