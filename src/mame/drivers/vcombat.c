/*
Virtual Combat hardware games.

Driver by Jason Eckhardt and Andrew Gardner.

----

There are two known games on this hardware.  Both are developed by
Kyle Hodgetts.

Virtual Combat (c) VR8 Inc. 1993
http://arcade.sonzogni.com/VRCombat/

Shadow Fighters (German) (c) Sega? 1989?

----

There are two boards to this hardware.  The upper, which contains the
graphics ROMs and the i860, and the lower which contains the main
and sound CPUs.  Virtual Combat sports two upper boards which
output a different rasterization of the scene for each stereo eye.

UPPER: (Virtual Combat has an identical MIDDLE board also).
    Intel I860 XR processor
    MB8298-25P-SK RAMS x12 (silkscreen said 62256)
    Analog device ADV476KN50E (silkscreen said BT476)
    20 MHZ Oscillator
    8-way DIP switch
    574200D x4
    PAL palce24v10 x2 (next to the i860)
    Bt476 RAMDAC

LOWER:
    Motorola MC68000P12 x2
    12 MHz Oscillator x2
    Harris ADC0804LCN x2
    4 MB8298-25P-SK RAMS (in groups of 2 off by themselves)
    1 CXK58257SP-10L at each end of the SNDCPU ROMS and the CPU ROMS (4 chips total)
    Motorola MC6845P CRT controller
    2x 27C010A containing sound code
    Xx 27C040 containing sound data (VOC files)
    Dallas DS1220Y - closest to pin 64 of CPU - read as a 2716 - (silkscreened "6116")
    Xx 27c040 containing program code, etc.

----

NOTES : Shadow Fighters appears to have been dumped from an earlier
            revision of the hardware.  There are no IC labels, and
            lots of factory rework has been done to the bottom board.
        Because the board was so early for Shadow Fighters, there were
            no IC locations silkscreened on the PCB.  The locations
            from Virtual Combat have been used.
        The Shadow Fighters bottom board has an extra 20 mhz xtal on it.
        The data stored in "samples" is simply a series of
            Creative Media VOC files concatenated to eachother.
        The sound program ("sound") is about 640 bytes long.
        The hardware is said to run at medium resolution.
        The SRAM module dump can likely be thrown away for both games.
        The PAL that's dumped for Shadow Fighters looks pretty bad.

TODO :  This is a partially working driver.  Most of the memory maps for
            all four CPUs are complete.  An Intel i860XR CPU core has now
            been incorporated into MAME.

    --------                Notes/questions                  ----------
     - Most of the memory maps and input ports are complete now.
       For "main", the only I/O locations that seem to be left
       mysterious are:
         0x600010,
         0x60000C: Only written; maybe mc6845? Often accessed
               together, and 16 consecutive words of data are
               written to 0x60000C, making me think it might be
               6845-related.  May or may not be important for
               reasonable emulation.
         0x60001C: Only read; might be a latch of some sort.  It is
               read each time M0's interrupt service routine syncs
               with the i860's to have them update the frame buffer.
               Might this be a HW blank bit so things look clean when
               the i860's do their updates?
               The two other times I see it read are just before
               and after one of the pallette setups is done.
         0x600018: ? No info yet.
         0x704000: (VC only) Likely analog axis for VR headset
         0x703000: (VC only) Likely analog axis for VR headset
         0x702000: (Shadow only). I think this is another IN port. It
               is always read at the same time as the other two
               ports.

      - Assuming a single framebuffer, as I think we are at the moment,
        how are all the CPUs writes to the framebuffer prioritized,
        if at all?  The zero values written by the 68k side erase
        the i860-generated FB data (i.e., zero maps to black in the
        pallette).  On the other hand, treating the zero values as
        transparent doesn't quite look right either.  Could just the
        i860s each have their own framebuffers?  After
        all, since each eye of the binocular sees a slightly different
        picture, the i860's might just maintain their own.  Then maybe
        the 68k shares with each of the two, since I think it just
        generates a single picture (not sure about that either).
    ----------------------------------------------
*/

#include <stdio.h>
#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "cpu/i860/i860.h"
#include "video/generic.h"
#include "video/tlc34076.h"

static UINT16* framebuffer_main;
static UINT16* framebuffer_secondary;
static UINT16* vid_0_shared_RAM;
static UINT16* vid_1_shared_RAM;


static VIDEO_UPDATE( vcombat )
{
	int x, y;
	int count = 0;
	const rgb_t *pens = tlc34076_get_pens();

	/* TODO: It looks like the leftmost chunk of the ground should really be on the right side? */
	/*       But the i860 draws the background correctly, so it may be an original game issue. */
	/*       There's also some garbage in the upper-left corner. Might be related to this 'wraparound'. */
	/*       Or maybe it's related to the 68k's alpha?  It might come from the 68k side of the house.  */

	/* Main eye */
	for(y = 0; y < 256; y++)
	{
		for(x = 384; x < 640; x++)
		{
			UINT32 color;
			if (x % 2) color = (framebuffer_main[count] & 0xff00) >> 8;
			else	   color =  framebuffer_main[count] & 0x00ff;

			/* Vcombat's screen renders 'flopped' - very likely because VR headset displays may reflect off mirrors.
               Shadfgtr isn't flopped, so it's not a constant feature of the hardware. */
			if(x < video_screen_get_visible_area(screen)->max_x && y < video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			if (x % 2) count++;
		}
	}

	/* Early out for shadow fighters */
	if (!framebuffer_secondary)
		return 0;

	/* Secondary eye */
	count = 0;
	for(y = 0; y < 256; y++)
	{
		for(x = 0; x < 256; x++)
		{
			UINT32 color;
			if (x % 2) color = (framebuffer_secondary[count] & 0xff00) >> 8;
			else	   color =  framebuffer_secondary[count] & 0x00ff;

			/* Vcombat's screen renders 'flopped' - very likely because VR headset displays may reflect off mirrors.
               Shadfgtr isn't flopped, so it's not a constant feature of the hardware. */
			if(x < video_screen_get_visible_area(screen)->max_x && y < video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			if (x % 2) count++;
		}
	}

	return 0;
}


/* Maybe there's a blend chip between the 68k and the framebuffer? */
static WRITE16_HANDLER( main_video_write )
{
	/* Doesn't seem to make sense for shadow fighters. Maybe some of that factory rework disables this?  */
	/* Doesn't always seem to work for vcombat either.  More testing needed.  */
	if (data != 0x00000000) {
		framebuffer_main[offset] = data;
	}
}

static READ16_HANDLER( control_1_r )
{
	return (input_port_read(space->machine, "IN0") << 8);
}

static READ16_HANDLER( control_2_r )
{
	return (input_port_read(space->machine, "IN1") << 8);
}

static void wiggle_i860_common(int n, UINT16 data, const device_config *device)
{
	int bus_hold = (data & 0x03) == 0x03;
	int reset = data & 0x10;
	assert(n >= 0 && n < 2);
	if (!device)
		return;
	if (bus_hold)
	{
		fprintf(stderr, "M0 asserting bus HOLD to i860 %s\n", device->tag);
		i860_set_pin(device->token, DEC_PIN_BUS_HOLD, 1);
	}
	else
	{
		fprintf(stderr, "M0 clearing bus HOLD to i860 %s\n", device->tag);
		i860_set_pin(device->token, DEC_PIN_BUS_HOLD, 0);
	}

	if (reset)
	{
		fprintf(stderr, "M0 asserting RESET to i860 %s\n", device->tag);
		i860_set_pin(device->token, DEC_PIN_RESET, 1);
	}
	else
		i860_set_pin(device->token, DEC_PIN_RESET, 0);
}

static WRITE16_HANDLER( wiggle_i860p0_pins_w )
{
	wiggle_i860_common(0, data, cputag_get_cpu(space->machine, "vid_0"));
}

static WRITE16_HANDLER( wiggle_i860p1_pins_w )
{
	wiggle_i860_common(1, data, cputag_get_cpu(space->machine, "vid_1"));
}

static READ16_HANDLER( main_irqiack_r )
{
	//fprintf(stderr, "M0: irq iack\n");
	cpu_set_input_line(cputag_get_cpu(space->machine, "main"), M68K_IRQ_1, CLEAR_LINE);
	//cpu_set_input_line(cputag_get_cpu(space->machine, "main"), INPUT_LINE_RESET, CLEAR_LINE);
	return 0;
}

static READ16_HANDLER( sound_resetmain_r )
{
	//fprintf(stderr, "M1: reset line to M0\n");
	//cpu_set_input_line(cputag_get_cpu(space->machine, "main"), INPUT_LINE_RESET, PULSE_LINE);
	return 0;
}

static WRITE64_HANDLER( v0_fb_w )
{
	/* The frame buffer seems to sit on a 32-bit data bus, while the
       i860 uses a 64-bit data bus.  Adjust accordingly.  */
	char *p = (char *)framebuffer_main;
	int m = mem_mask;
	int o = (offset << 2);
	if (m & 0xff000000) {
		p[o+3] = (data >> 24) & 0xff;
	}
	if (m & 0x00ff0000) {
		p[o+2] = (data >> 16) & 0xff;
	}
	if (m & 0x0000ff00) {
		p[o+1] = (data >> 8) & 0xff;
	}
	if (m & 0x000000ff) {
		p[o+0] = (data >> 0) & 0xff;
	}
}

/* This is just temporary so we can see what each i860 is doing to the
   framebuffer.  */
static WRITE64_HANDLER( v1_fb_w )
{
	/* The frame buffer seems to sit on a 32-bit data bus, while the
       i860 uses a 64-bit data bus.  Adjust accordingly.  */
	char *p = (char *)framebuffer_secondary;
	int m = mem_mask;
	int o = (offset << 2);
	if (m & 0xff000000) {
		p[o+3] = (data >> 24) & 0xff;
	}
	if (m & 0x00ff0000) {
		p[o+2] = (data >> 16) & 0xff;
	}
	if (m & 0x0000ff00) {
		p[o+1] = (data >> 8) & 0xff;
	}
	if (m & 0x000000ff) {
		p[o+0] = (data >> 0) & 0xff;
	}
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM
	AM_RANGE(0x300000, 0x30ffff) AM_RAM_WRITE(main_video_write)

	AM_RANGE(0x400000, 0x43ffff) AM_RAM AM_BASE(&vid_0_shared_RAM) AM_SHARE(2)	/* First i860 shared RAM */
	AM_RANGE(0x440000, 0x440003) AM_RAM AM_SHARE(6)		/* M0->P0 i860 #1 com 1 */
	AM_RANGE(0x480000, 0x480003) AM_RAM AM_SHARE(7)		/* M0<-P0 i860 #1 com 2 */
	AM_RANGE(0x4c0000, 0x4c0003) AM_WRITE(wiggle_i860p0_pins_w)	/* i860 #1 stop/start/reset */

	AM_RANGE(0x500000, 0x53ffff) AM_RAM AM_BASE(&vid_1_shared_RAM) AM_SHARE(3)	/* Second i860 shared RAM */
	AM_RANGE(0x540000, 0x540003) AM_RAM AM_SHARE(8)		/* M0->P1 i860 #2 com 1 */
	AM_RANGE(0x580000, 0x580003) AM_RAM AM_SHARE(9)		/* M0<-P1 i860 #2 com 2 */
	AM_RANGE(0x5c0000, 0x5c0003) AM_WRITE(wiggle_i860p1_pins_w)	/* i860 #2 stop/start/reset */

	AM_RANGE(0x600000, 0x600001) AM_READ(control_1_r)	/* IN0 port */
	AM_RANGE(0x600004, 0x600005) AM_RAM AM_SHARE(5)		/* M0<-M1 */
	AM_RANGE(0x600008, 0x600009) AM_READ(control_2_r)	/* IN1 port */
	/* AM_RANGE(0x60000c, 0x60000d)                        See notes at top of driver.  */
	/* AM_RANGE(0x600010, 0x600011)                        See notes at top of driver.  */
	AM_RANGE(0x700000, 0x7007ff) AM_RAM					/* TODO: Non-volatile RAM */
	AM_RANGE(0x701000, 0x701001) AM_READ(main_irqiack_r)
	AM_RANGE(0x705000, 0x705001) AM_RAM AM_SHARE(4)		/* M1->M0 */

	//AM_RANGE(0x703000, 0x703001)      /* Headset rotation axis? */
	//AM_RANGE(0x704000, 0x704001)      /* Headset rotation axis? */

	AM_RANGE(0x706000, 0x70601f) AM_READWRITE(tlc34076_lsb_r, tlc34076_lsb_w)
ADDRESS_MAP_END


/* The first i860 - middle board */
static ADDRESS_MAP_START( vid_0_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM_WRITE(v0_fb_w)		/* Shared framebuffer - half of the bits lost to 32-bit bus */
	AM_RANGE(0x20000000, 0x20000007) AM_RAM AM_SHARE(6) 		/* M0<-P0 com 1 (0x440000 in 68k-land) */
	AM_RANGE(0x40000000, 0x401fffff) AM_ROM AM_REGION("gfx", 0)
	AM_RANGE(0x80000000, 0x80000007) AM_RAM AM_SHARE(7) 		/* M0->P0 com 2 (0x480000 in 68k-land) */
	AM_RANGE(0xc0000000, 0xc0000fff) AM_NOP       				/* Dummy D$ flush page. */
	AM_RANGE(0xfffc0000, 0xffffffff) AM_RAM AM_SHARE(2)			/* Shared RAM with main */
ADDRESS_MAP_END


/* The second i860 - top board */
static ADDRESS_MAP_START( vid_1_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM_WRITE(v1_fb_w)		/* Half of the bits lost to 32-bit bus */
	AM_RANGE(0x20000000, 0x20000007) AM_RAM AM_SHARE(8) 		/* M0->P1 com 1 (0x540000 in 68k-land) */
	AM_RANGE(0x40000000, 0x401fffff) AM_ROM AM_REGION("gfx", 0)
	AM_RANGE(0x80000000, 0x80000007) AM_RAM AM_SHARE(9)			/* M0<-P1 com 2      (0x580000 in 68k-land) */
	AM_RANGE(0xc0000000, 0xc0000fff) AM_NOP       				/* Dummy D$ flush page. */
	AM_RANGE(0xfffc0000, 0xffffffff) AM_RAM AM_SHARE(3)			/* Shared RAM with main */
ADDRESS_MAP_END


/* Sound CPU */
static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x081000, 0x081fff) AM_RAM
	AM_RANGE(0x140000, 0x140001) AM_READ(sound_resetmain_r)   /* Ping M0's reset line */
	AM_RANGE(0x180000, 0x180001) AM_RAM AM_SHARE(4)   /* M1<-M0 */
	AM_RANGE(0x1C0000, 0x1C0001) AM_RAM AM_SHARE(5)   /* M1->M0 */
ADDRESS_MAP_END


static MACHINE_RESET( vcombat )
{
	/* Setup the Bt476 VGA RAMDAC palette chip */
	tlc34076_reset(6);

	i860_set_pin(cputag_get_cpu(machine, "vid_0")->token, DEC_PIN_BUS_HOLD, 1);
	i860_set_pin(cputag_get_cpu(machine, "vid_1")->token, DEC_PIN_BUS_HOLD, 1);
}

static MACHINE_RESET( shadfgtr )
{
	/* Setup the Bt476 VGA RAMDAC palette chip */
	tlc34076_reset(6);

	i860_set_pin(cputag_get_cpu(machine, "vid_0")->token, DEC_PIN_BUS_HOLD, 1);
}


static DIRECT_UPDATE_HANDLER( vid_0_direct_handler )
{
	if (address >= 0xfffc0000 && address <= 0xffffffff)
	{
		direct->raw = direct->decrypted = ((UINT8*)vid_0_shared_RAM) - 0xfffc0000;
		return ~0;
	}
	return address;
}

static DIRECT_UPDATE_HANDLER( vid_1_direct_handler )
{
	if (address >= 0xfffc0000 && address <= 0xffffffff)
	{
		direct->raw = direct->decrypted = ((UINT8*)vid_1_shared_RAM) - 0xfffc0000;
		return ~0;
	}
	return address;
}


static DRIVER_INIT( vcombat )
{
	UINT8 *ROM = memory_region(machine, "main");

	/* The two i860s execute out of RAM */
	memory_set_direct_update_handler(cputag_get_address_space(machine, "vid_0", ADDRESS_SPACE_PROGRAM), vid_0_direct_handler);
	memory_set_direct_update_handler(cputag_get_address_space(machine, "vid_1", ADDRESS_SPACE_PROGRAM), vid_1_direct_handler);

	/* Allocate the two framebuffers */
	framebuffer_main = auto_malloc(0x10000 * sizeof(UINT16));
	framebuffer_secondary = auto_malloc(0x10000 * sizeof(UINT16));

	/* pc==4016 : jump 4038 ... There's something strange about how it waits at 402e (interrupts all masked out)
       I think what is happening here is that M0 snags the first time
       it hits this point based on a counter test just above this
       instruction.  That counter gets updated just past this instruction.
       However, the only way this can be passed is if the M0 CPU is
       reset (the IPL=7, but irq 7 is a nop).  I am almost sure that M1
       reads a latch, which resets M0 (probably to ensure M0 and M1 are
       both alive) and gets passed this snag.  I tried to hook up a reset
       which should work, but asserting the reset line on the m68k doesn't
       seem to do anything.  Maybe the 68k emulator doesn't respond to
       that, or I didn't do it correctly.  But I think that is what needs
       to be done.  But it isn't crucial for emulation.  Shadow does not
       have this snag. */
	ROM[0x4017] = 0x66;
}

static DRIVER_INIT( shadfgtr )
{
	/* Allocate just one framebuffer */
	framebuffer_main = auto_malloc(0x10000 * sizeof(UINT16));
	framebuffer_secondary = NULL;

	/* The i860 executes out of RAM */
	memory_set_direct_update_handler(cputag_get_address_space(machine, "vid_0", ADDRESS_SPACE_PROGRAM), vid_0_direct_handler);
}


static INPUT_PORTS_START( vcombat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )   /* Left button */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )   /* Right button */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( shadfgtr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY /* ? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY /* ? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static MACHINE_DRIVER_START( vcombat )
	MDRV_CPU_ADD("main", M68000, XTAL_12MHz)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT("main", irq1_line_assert)

	/* The middle board i860 */
	MDRV_CPU_ADD("vid_0", I860, XTAL_20MHz)
	MDRV_CPU_PROGRAM_MAP(vid_0_map,0)

	/* The top board i860 */
	MDRV_CPU_ADD("vid_1", I860, XTAL_20MHz)
	MDRV_CPU_PROGRAM_MAP(vid_1_map,0)

	/* Sound CPU */
	MDRV_CPU_ADD("sound", M68000, XTAL_12MHz)
	MDRV_CPU_PROGRAM_MAP(sound_map,0)

	MDRV_MACHINE_RESET(vcombat)

/* Temporary hack for experimenting with timing. */
#if 0
	//MDRV_QUANTUM_TIME(HZ(1200))
	MDRV_QUANTUM_PERFECT_CPU("main")
#endif

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MDRV_VIDEO_UPDATE(vcombat)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( shadfgtr )
	MDRV_CPU_ADD("main", M68000, XTAL_12MHz)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT("main", irq1_line_assert)

	/* The middle board i860 */
	MDRV_CPU_ADD("vid_0", I860, XTAL_20MHz)
	MDRV_CPU_PROGRAM_MAP(vid_0_map,0)

	/* Sound CPU */
	MDRV_CPU_ADD("sound", M68000, XTAL_12MHz)
	MDRV_CPU_PROGRAM_MAP(sound_map,0)

	MDRV_MACHINE_RESET(shadfgtr)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MDRV_VIDEO_UPDATE(vcombat)
MACHINE_DRIVER_END


/* Perhaps add this to the core romload.h file?  */
#define ROM_LOAD64_WORD(name,offset,length,hash) ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6))

ROM_START( vcombat )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "ep8v2.b49", 0x00000, 0x80000, CRC(98d5a45d) SHA1(099e314f11c93ad6e642ceaa311e2a5b6fd7193c) )
	ROM_LOAD16_BYTE( "ep7v2.b51", 0x00001, 0x80000, CRC(06185bcb) SHA1(855b11ae7644d6c7c1c935b2f5aec484071ca870) )

	ROM_REGION( 0x40000, "sound", 0 )
	ROM_LOAD16_BYTE( "ep1v2.b42", 0x00000, 0x20000, CRC(560b2e6c) SHA1(e35c0466a1e14beab080e3155f873e9c2a1c028b) )
	ROM_LOAD16_BYTE( "ep6v2.b33", 0x00001, 0x20000, CRC(37928a5d) SHA1(7850be26dbd356cdeef2a0d87738de16420f6291) )

	ROM_REGION( 0x180000, "samples", 0 )
	ROM_LOAD16_BYTE( "ep2v2.b41", 0x000000, 0x80000, CRC(7dad3458) SHA1(deae5ebef0346250d3f9744933423253a336bb67) )
	ROM_LOAD16_BYTE( "ep4v2.b37", 0x000001, 0x80000, CRC(b0be2e91) SHA1(66f3a9f5abeb4b95ac806e4bb165f938dca38b2d) )
	ROM_LOAD16_BYTE( "ep3v2.b40", 0x100000, 0x40000, CRC(8c491526) SHA1(95c6bcbe0adcfffb12fd2b86c9f4ca26aa188bbf) )
	ROM_LOAD16_BYTE( "ep5v2.b36", 0x100001, 0x40000, CRC(7592b2eb) SHA1(92a540726306d7adbf207fe86a4c4fa66958f90b) )

	ROM_REGION( 0x800, "user1", 0 ) /* The SRAM module */
	ROM_LOAD( "ds1220y.b53", 0x000, 0x800, CRC(b21cfe5f) SHA1(898ace3cd0913ea4b0dc84320219777773ef856f) )

	/* These roms are identical on both of the upper boards */
	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD64_WORD( "9.u55",  0x000000, 0x80000, CRC(a276e18b) SHA1(6d60e519196a4858b82241504592413df498e12f) )
	ROM_LOAD64_WORD( "10.u57", 0x000002, 0x80000, CRC(8921f20e) SHA1(6e9ca2eaad3e1108ba0e1d7792fd5d0305bec201) )
	ROM_LOAD64_WORD( "11.u54", 0x000004, 0x80000, CRC(a83094ce) SHA1(c3512375fecdb5e7eb02a4aa140ae4efe0233cb8) )
	ROM_LOAD64_WORD( "12.u56", 0x000006, 0x80000, CRC(0cdffd4f) SHA1(65ace78711b3ef6e0ff9a7ad7343b5558e652f6c) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal1_w2.u51", 0x000, 0x1f1, CRC(af497420) SHA1(03aa82189d91ae194dd5a6e7b9dbdb7cd473ddb6) )
	ROM_LOAD( "pal2_w2.u52", 0x200, 0x1f1, CRC(4a6df05d) SHA1(236b951e5daf927c050d0f35558c171a020156ab) )
ROM_END

ROM_START( shadfgtr )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b49", 0x00000, 0x80000, CRC(2d9d31a1) SHA1(45854915bcb9db2e4076a7f26a0a349077cd10bc) )
	ROM_LOAD16_BYTE( "shadfgtr.b51", 0x00001, 0x80000, CRC(03d0f075) SHA1(06013a4363305a23d7e8ba8fe2fa961cd540391d) )

	ROM_REGION( 0x40000, "sound", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b42", 0x00000, 0x20000, CRC(f8605dcd) SHA1(1b29f47856ccc757bc96674682ae48f87e6b0e54) )
	ROM_LOAD16_BYTE( "shadfgtr.b33", 0x00001, 0x20000, CRC(291d59ac) SHA1(cc4904c2ac8ef6a12033c10893246a438ac44014) )

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b41", 0x00000, 0x80000, CRC(9e4b4df3) SHA1(8101197275e9f728acdeef85737eecbdec132b27) )
	ROM_LOAD16_BYTE( "shadfgtr.b37", 0x00001, 0x80000, CRC(98446ba2) SHA1(1c8cc0d9c5de54d9e53699a5ab281579d15edc96) )

	ROM_REGION( 0x800, "user1", 0 )	/* The SRAM module */
	ROM_LOAD( "shadfgtr.b53", 0x000, 0x800, CRC(e766a3ab) SHA1(e7696ec08d5c86f64d768480f43edbd19ded162d) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD64_WORD( "shadfgtr.u55", 0x000000, 0x80000, CRC(e807631d) SHA1(9027ff7dc60b808434dac292c08f0630d3d52186) )
	ROM_LOAD64_WORD( "shadfgtr.u57", 0x000002, 0x80000, CRC(60d701d7) SHA1(936473b5e3b2e9e9e3b50cf977fc5a670a097850) )
	ROM_LOAD64_WORD( "shadfgtr.u54", 0x000004, 0x80000, CRC(c45d68d6) SHA1(a133e4f13d3af18bccf0d060a659d64ac699b159) )
	ROM_LOAD64_WORD( "shadfgtr.u56", 0x000006, 0x80000, CRC(fb76db5a) SHA1(fa546f465df113c13037abed1162bfa6f9b1dc9b) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "shadfgtr.u51", 0x000, 0x1f1, CRC(bab58337) SHA1(c4a79c8e53aeadb7f64d49d214b607b5b36f144e) )
	/* The second upper-board PAL couldn't be read */
ROM_END

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT      MONITOR COMPANY      FULLNAME           FLAGS */
GAME( 1993, vcombat,  0,      vcombat,  vcombat,  vcombat,  ORIENTATION_FLIP_X,  "VR8 Inc.",     "Virtual Combat",  GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1989, shadfgtr, 0,      shadfgtr, shadfgtr, shadfgtr, ROT0,                "DUTECH Inc.",  "Shadow Fighters", GAME_NOT_WORKING | GAME_NO_SOUND )
