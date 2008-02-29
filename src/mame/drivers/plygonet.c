/*
    Polygonet Commanders (Konami, 1993)

    Preliminary driver by R. Belmont
    Additional work by Andrew Gardner

    This is Konami's first 3D game!

    Hardware:
    68EC020 @ 16 MHz
    Motorola XC56156-40 DSP @ 40 MHz
    Z80 + K054539 for sound
    Network to connect up to 4 PCBs.

    Video hardware:
    TTL text plane similar to Run and Gun.
    Konami K054009(x2) + K054010(x2) (polygon rasterizers)
    Konami K053936 "PSAC2" (3d roz plane, used for backgrounds)
    24.0 MHz crystal to drive the video hardware

    Driver includes:
    - 68020 memory map
    - Z80 + sound system
    - EEPROM
    - service switch
    - TTL text plane

    Driver needs:
    - Handle network at 580800 so game starts
    - Polygon rasterization (K054009 + K054010)
    - Hook up PSAC2 (gfx decode for it is already present and correct)
    - Palettes
    - Controls
    - Priorities.  From the original board it appears they're fixed, in front to back order:
      (all the way in front) TTL text layer -> polygons -> PSAC2 (all the way in back)

Notes:

 (R. Belmont)
 506000 is the DSP control
 506004 is DSP status (in bit 0, where 0 = not ready, 1 = ready)
 50600C and 50600E are the DSP comms ports (read/write)

*/

#include "driver.h"
#include "deprecat.h"
#include "video/konamiic.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "cpu/dsp56k/dsp56k.h"
#include "sound/k054539.h"
#include "machine/eeprom.h"

VIDEO_START( polygonet );
VIDEO_UPDATE( polygonet );

READ32_HANDLER( polygonet_ttl_ram_r );
WRITE32_HANDLER( polygonet_ttl_ram_w );

static int init_eeprom_count;

static int dsp_alive=0;
static UINT32 *dsp_shared_ram;
static UINT16 *dsp56k_shared_ram_16;

static UINT16 *dsp56k_p_mirror;
static UINT16 *dsp56k_bank00_ram ;
static UINT16 *dsp56k_bank01_ram ;
static UINT16 *dsp56k_bank02_ram ;
static UINT16 *dsp56k_bank04_ram ;

static const struct EEPROM_interface eeprom_interface =
{
	7,			/* address bits */
	8,			/* data bits */
	"011000",		/*  read command */
	"010100",		/* write command */
	"0100100000000",/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

static NVRAM_HANDLER( polygonet )
{
	if (read_or_write)
		EEPROM_save(file);
	else
	{
		EEPROM_init(&eeprom_interface);

		if (file)
		{
			init_eeprom_count = 0;
			EEPROM_load(file);
		}
		else
			init_eeprom_count = 10;
	}
}

static READ32_HANDLER( polygonet_eeprom_r )
{
	if (ACCESSING_LSW32)
	{
		return 0x0200 | (EEPROM_read_bit()<<8);
	}
	else
	{
		return (readinputport(0)<<24);
	}

	logerror("unk access to eeprom port (mask %x)\n", mem_mask);
	return 0;
}


static WRITE32_HANDLER( polygonet_eeprom_w )
{
	if (ACCESSING_MSB32)
	{
		EEPROM_write_bit((data & 0x01000000) ? ASSERT_LINE : CLEAR_LINE);
		EEPROM_set_cs_line((data & 0x02000000) ? CLEAR_LINE : ASSERT_LINE);
		EEPROM_set_clock_line((data & 0x04000000) ? ASSERT_LINE : CLEAR_LINE);
		return;
	}

	logerror("unknown write %x (mask %x) to eeprom\n", data, mem_mask);
}

/* TTL tile readback for ROM test */
static READ32_HANDLER( ttl_rom_r )
{
	UINT32 *ROM;
	ROM = (UINT32 *)memory_region(REGION_GFX1);

	return ROM[offset];
}

/* PSAC2 tile readback for ROM test */
static READ32_HANDLER( psac_rom_r )
{
	UINT32 *ROM;
	ROM = (UINT32 *)memory_region(REGION_GFX2);

	return ROM[offset];
}

/* irqs 3, 5, and 7 have valid vectors                */
/* irq 3 does ??? (network)                           */
/* irq 5 does ??? (polygon end of draw or VBL?)       */
/* irq 7 does nothing (it jsrs to a rts and then rte) */

static INTERRUPT_GEN(polygonet_interrupt)
{
	if (cpu_getiloops())
		cpunum_set_input_line(machine, 0, MC68000_IRQ_5, HOLD_LINE);
	else
		cpunum_set_input_line(machine, 0, MC68000_IRQ_3, HOLD_LINE);
}

/* sound CPU communications */

static READ32_HANDLER( sound_r )
{
	int latch = soundlatch3_r(0);

	if (latch == 0xe) latch = 0xf;	/* hack: until 54539 NMI disable found */

	return latch<<8;
}

static WRITE32_HANDLER( sound_w )
{
	if (ACCESSING_MSB)
	{
		soundlatch_w(0, (data>>8)&0xff);
	}
	else
	{
		soundlatch2_w(0, data&0xff);
	}
}

static WRITE32_HANDLER( sound_irq_w )
{
	cpunum_set_input_line(Machine, 2, 0, HOLD_LINE);
}


/* DSP communications are on their way to being correct */

static READ32_HANDLER( dsp_shared_ram_read )
{
	return dsp_shared_ram[offset] ;
}

static WRITE32_HANDLER( dsp_shared_ram_write )
{
	COMBINE_DATA(&dsp_shared_ram[offset]) ;

	logerror("68k WRITING %04x & %04x to shared ram %x & %x [%08x] (@%x)\n", (dsp_shared_ram[offset] & 0xffff0000) >> 16,
																             (dsp_shared_ram[offset] & 0x0000ffff),
																              0xc000 + (offset<<1),
																              0xc000 +((offset<<1)+1),
																		      mem_mask,
																		      activecpu_get_pc());

	// PC increments before it gets here
	if (activecpu_get_pc() == 0x46204 || activecpu_get_pc() == 0x46208 || activecpu_get_pc() == 0x4620c)
	{
		logerror("SKIPPY HACK\n");
		return;
	}

	if (mem_mask == (0x00000000))
	{
		/* write the data to the dsp56k as well */
		dsp56k_shared_ram_16[(offset<<1)]   = (dsp_shared_ram[offset] & 0xffff0000) >> 16 ;
		dsp56k_shared_ram_16[(offset<<1)+1] = (dsp_shared_ram[offset] & 0x0000ffff) ;
	}
	else if (mem_mask == (0x0000ffff))
	{
		/* write to the 'current' dsp56k byte */
		dsp56k_shared_ram_16[(offset<<1)]   = (dsp_shared_ram[offset] & 0xffff0000) >> 16 ;
	}
	else if (mem_mask == (0xffff0000))
	{
		/* write to the 'next' dsp56k byte */
		dsp56k_shared_ram_16[(offset<<1)+1] = (dsp_shared_ram[offset] & 0x0000ffff) ;
	}
}



static WRITE32_HANDLER( dsp_2_w )
{
    /*
       Assumed to be mapped to the DSP56156's Host Interface
       it's write, so there are only 2 options here - the Interrupt Control Register (ICR) - address $0
         or the Interrupt Vector Register (IVR) - address $3...

       addendum - i believe it CAN'T be the ICR because bit 2 is reserved, and 05 would be writing to that...
                  besides, i think dsp_w (mask 0xff000000) is the proper ICR

       addendum - or maybe it's attached to the pins directly attached to the Host Interface Control Logic
                  HA0-HA2, HR/W, HEN, HACK, HREQ

       values passed are 0x00, 0x01, 0x05
    */

	/* HAAAACK !!! Reset the DMA memory pointer for the dsp56k */
	if (data == 0x00000000)
		dsp56k_reset_dma_offset();

	logerror("dsp_2_w  : %08x %08x %x \n", data, mem_mask, activecpu_get_pc()) ;
}





static READ32_HANDLER( dsp_host_interface_r )
{
	UINT8 hi_addr  = offset<<1;

	if (mem_mask == (~0x0000ff00))
		hi_addr++;

	logerror("CALLING dsp_host_interface_read %x = %x [%x] (@%x)\n", hi_addr, dsp56k_host_interface_read(hi_addr), mem_mask, activecpu_get_pc());

	if (mem_mask == (~0x0000ff00))
		return dsp56k_host_interface_read(hi_addr) << 8;
	else
		return dsp56k_host_interface_read(hi_addr) << 24;
}


static WRITE32_HANDLER( dsp_host_interface_w )
{
	UINT8 dsp_data;
	UINT8 hi_addr  = offset<<1;

	if (mem_mask == (~0x0000ff00))				/* The second byte */
	{
		hi_addr++;
		dsp_data = data >> 8;
	}
	else
	{
		dsp_data = data >> 24;
	}

	// !!! HACK - rev up cpu 1 when the HF0 bit is set !!!
	if (dsp_data == 0x08 && hi_addr == 0)
	{
		logerror("hardware RESET sent\n");
		if (dsp_alive == 0)
			cpunum_resume(1, SUSPEND_REASON_DISABLE) ;
		dsp_alive = 1;
	}
	else
	{
		logerror("CALLING dsp_host_interface_write %x %x\n", hi_addr, dsp_data);
		dsp56k_host_interface_write(hi_addr, dsp_data);
	}
}

static READ32_HANDLER( network_r )
{
	return 0x08000000;
}

static WRITE32_HANDLER( network_w )
{
}

/**************************/
/* DSP56k MEMORY HANDLERS */
/**************************/

	/* THE ffe3 story :
       It's a 12-bit general purpose I/O port.  I believe it handles banking...

       XXXX ---- ---- ----  . unusable
       XXXX ???- -?-- ----  . unknown
       XXXX ---- --x- ----  . turned on very early in the software - seems to enable 001c banking
       XXXX ---- ---- --x-  . turned on just before playing with 0181 banking
       XXXX ---- ---x xx--  . (001c banking) believed to bank memory from 0x8000-0xbfff - IMPLEMENTED
       XXXX ---x x--- ---x  . (0181 banking) believed to bank other, strange memory -     IMPLEMENTED

       001c banking is fairly easy - it happens in a loop and writes from 8000 to bfff
       0181 banking is very weird  - it happens in a nested loop and writes from 6000-6fff, 7000-7fff, and 8000-ffbf
                                     bit 0002 turns on *just* before this happens.

       ...All of the bankXX memory read and write memory functions above check these values early on and
          act accordingly...
    */



#define DSP56K_PORTC (dsp56k_get_peripheral_memory(0xffe3))

static READ16_HANDLER( dsp56k_ram_bank00_read )
{
	/* dsp56k_bank00_ram only has banking for the 0x0002 state - therefore its size is : 0x1000 + (0x8 * 0x1000) */

	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x1000 ;

		/* add the non-banked 0x1000 words for 0x0020 state */
		return dsp56k_bank00_ram[0x1000 + memOffset + offset] ;
	}

	if (DSP56K_PORTC & 0x0020)
	{
		/* 0x0020 state sits at the bottom of memory */
		return dsp56k_bank00_ram[offset] ;
	}

	logerror("dsp56k_ram_bank00_read : UNKNOWN BANKING STATE!\n") ;
	return 0x00 ;
}

static WRITE16_HANDLER( dsp56k_ram_bank00_write )
{
	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x1000 ;

		/* add the non-banked 0x1000 words for 0x0020 state */
		COMBINE_DATA(&dsp56k_bank00_ram[0x1000 + memOffset + offset]) ;
	}
	else if (DSP56K_PORTC & 0x0020)
	{
		/* 0x0020 state sits at the bottom of memory */
		COMBINE_DATA(&dsp56k_bank00_ram[offset]) ;
	}
}

static READ16_HANDLER( dsp56k_ram_bank01_read )
{
	/* dsp56k_bank01_ram only has banking for the 0x0002 state - therefore its size is : 0x1000 + (0x8 * 0x1000) */

	/* 0x0002 overrides 0x0020 */
	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x1000 ;

		/* add the non-banked 0x1000 words for 0x0020 state */
		return dsp56k_bank01_ram[0x1000 + memOffset + offset] ;
	}

	if (DSP56K_PORTC & 0x0020)
	{
		/* 0x0020 state sits at the bottom of memory */
		return dsp56k_bank01_ram[offset] ;
	}

	logerror("dsp56k_ram_bank01_read : UNKNOWN BANKING STATE!\n") ;
	return 0x00 ;
}

static WRITE16_HANDLER( dsp56k_ram_bank01_write )
{
	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x1000 ;

		/* add the non-banked 0x1000 words for 0x0020 state */
		COMBINE_DATA(&dsp56k_bank01_ram[0x1000 + memOffset + offset]) ;
	}
	else if (DSP56K_PORTC & 0x0020)
	{
		/* 0x0020 state sits at the bottom of memory */
		COMBINE_DATA(&dsp56k_bank01_ram[offset]) ;
	}

	/* For now, *always* combine P:0x7000-0x7fff with bank01 */
	dsp56k_p_mirror[offset] = data;
}

static READ16_HANDLER( dsp56k_ram_bank02_read )
{
	/* Tons of banking for dsp_bank02_ram - both states - therefore its size is (0x8*0x4000) + (0x8*0x4000) */

//  logerror("read ffe3 %x\n", DSP56K_PORTC);

	/* 0x0002 overrides 0x0020 */
	if (DSP56K_PORTC & 0x0002)
	{
		UINT32 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x4000 ;

		return dsp56k_bank02_ram[(0x4000*0x8) + memOffset + offset] ;
	}

	if (DSP56K_PORTC & 0x0020)
	{
		UINT32 memOffset  = ( (DSP56K_PORTC & 0x001c) >> 2 ) ;
		memOffset        *= 0x4000 ;

		/* 0x0020 state sits at the bottom of memory */
		return dsp56k_bank02_ram[memOffset + offset] ;
	}

	logerror("dsp56k_ram_bank02_read : UNKNOWN BANKING STATE!\n") ;
	return 0x00 ;
}

static WRITE16_HANDLER( dsp56k_ram_bank02_write )
{
//  logerror("write ffe3 %x\n", DSP56K_PORTC);

	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT32 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x4000 ;

//      logerror("memOffset %x\n", memOffset);

		/* add the non-banked 0x1000 words for 0x0020 state */
		COMBINE_DATA(&dsp56k_bank02_ram[(0x4000*0x8) + memOffset + offset]) ;
	}
	else if (DSP56K_PORTC & 0x0020)
	{
		UINT32 memOffset  = ( (DSP56K_PORTC & 0x001c) >> 2 ) ;
		memOffset        *= 0x4000 ;

		/* 0x0020 state sits at the bottom of memory */
		COMBINE_DATA(&dsp56k_bank02_ram[memOffset + offset]) ;
	}
}

static READ16_HANDLER( dsp56k_shared_ram_read )
{
	/* dsp56k_shared_ram_16 only has banking for the 0x0002 state - therefore its size is : 0x2000 + (0x8 * 0x2000) */

	/* 0x0002 overrides 0x0020 */
	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x2000 ;

		/* add the non-banked 0x2000 words for 0x0020 state */
		return dsp56k_shared_ram_16[0x2000 + memOffset + offset] ;
	}

	if (DSP56K_PORTC & 0x0020)
	{
		/* 0x0020 state sita at the bottom of memory - this is the part that is shared */
		return dsp56k_shared_ram_16[offset] ;
	}

	logerror("dsp56k_shared_ram_read : UNKNOWN BANKING STATE!\n") ;
	return 0x00 ;
}

static WRITE16_HANDLER( dsp56k_shared_ram_write )
{
	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x2000 ;

		/* add the non-banked 0x2000 words for 0x0020 state */
		COMBINE_DATA(&dsp56k_shared_ram_16[0x2000 + memOffset + offset]) ;
	}
	else if (DSP56K_PORTC & 0x0020)
	{
		/* 0x0020 state sits at the bottom of memory */
		COMBINE_DATA(&dsp56k_shared_ram_16[offset]) ;

		/* write the data to the 68k as well, yo */
		if (offset % 2)
			dsp_shared_ram[offset>>1] = ((dsp56k_shared_ram_16[offset-1]) << 16) | dsp56k_shared_ram_16[offset] ;
		else
			dsp_shared_ram[offset>>1] = ((dsp56k_shared_ram_16[offset])   << 16) | dsp56k_shared_ram_16[offset+1] ;
	}
}

static READ16_HANDLER( dsp56k_ram_bank04_read )
{
	/* dsp56k_bank04_ram only has banking for the 0x0002 state - therefore its size is : 0x1fc0 + (0x8 * 0x1fc0) */

	/* 0x0002 overrides 0x0020 */
	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x1fc0 ;

		/* add the non-banked 0x1fc0 words for 0x0020 state */
		return dsp56k_bank04_ram[0x1fc0 + memOffset + offset] ;
	}

	if (DSP56K_PORTC & 0x0020)
	{
		/* 0x0020 state sits at the bottom of memory - this is the part that is shared */
		return dsp56k_bank04_ram[offset] ;
	}

	logerror("dsp56k_ram_bank04_read : UNKNOWN BANKING STATE!\n") ;
	return 0x00 ;
}

static WRITE16_HANDLER( dsp56k_ram_bank04_write )
{
	if (DSP56K_PORTC & 0x0002)
	{
		/* 0x0181 for the 0x0002 banking */
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x0001) + ((DSP56K_PORTC & 0x0180) >> 6) ) ;
		memOffset        *= 0x1fc0 ;

		/* add the non-banked 0x1fc0 words for 0x0020 state */
		COMBINE_DATA(&dsp56k_bank04_ram[0x1fc0 + memOffset + offset]) ;
	}
	else if (DSP56K_PORTC & 0x0020)
	{
		UINT16 memOffset  = ( (DSP56K_PORTC & 0x001c) >> 2 ) ;
		memOffset        *= 0x1fc0 ;

		/* 0x0020 state sits at the bottom of memory */
		COMBINE_DATA(&dsp56k_bank04_ram[offset]) ;
	}
}


/**********************************************************************************/

static ADDRESS_MAP_START( polygonet_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM	// program/data ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM	// PSAC2 tilemap
//  AM_RANGE(0x400000, 0x40000f) // NOT SURE - BANKING?
	AM_RANGE(0x440000, 0x440fff) AM_RAM	// PSAC2 lineram
	AM_RANGE(0x480000, 0x480003) AM_READ(polygonet_eeprom_r)
	AM_RANGE(0x4C0000, 0x4C0003) AM_WRITE(polygonet_eeprom_w)
	AM_RANGE(0x500000, 0x503fff) AM_READWRITE(dsp_shared_ram_read, dsp_shared_ram_write) AM_BASE(&dsp_shared_ram)
	AM_RANGE(0x504000, 0x504003) AM_WRITE(dsp_2_w)
	AM_RANGE(0x506000, 0x50600f) AM_READWRITE(dsp_host_interface_r, dsp_host_interface_w)
	AM_RANGE(0x540000, 0x540fff) AM_READWRITE(polygonet_ttl_ram_r, polygonet_ttl_ram_w)
	AM_RANGE(0x541000, 0x54101f) AM_RAM
	AM_RANGE(0x580000, 0x5807ff) AM_RAM	// chip A21K on the PCB
	AM_RANGE(0x580800, 0x580803) AM_READWRITE(network_r, network_w)
//  AM_RANGE(0x600000, 0x600003) // DUNNO - probably sound
	AM_RANGE(0x600004, 0x600007) AM_WRITE(sound_w)
	AM_RANGE(0x600008, 0x60000b) AM_READ(sound_r)
	AM_RANGE(0x640000, 0x640003) AM_WRITE(sound_irq_w)
	AM_RANGE(0x680000, 0x680003) AM_WRITE(watchdog_reset32_w)
	AM_RANGE(0x700000, 0x73ffff) AM_READ(psac_rom_r)
	AM_RANGE(0x780000, 0x79ffff) AM_READ(ttl_rom_r)
	AM_RANGE(0xff8000, 0xffffff) AM_RAM	// work RAM
ADDRESS_MAP_END

/**********************************************************************************/

static ADDRESS_MAP_START( dsp56156_p_map, ADDRESS_SPACE_PROGRAM, 16 )
//  ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_BASE(&dsp56k_p_mirror)   // is it 0x1000 words?
	AM_RANGE(0x8000, 0x87ff) AM_RAM								// the processor memtests here
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp56156_d_map, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x0800, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_READWRITE(dsp56k_ram_bank00_read, dsp56k_ram_bank00_write)
	AM_RANGE(0x7000, 0x7fff) AM_READWRITE(dsp56k_ram_bank01_read, dsp56k_ram_bank01_write)
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(dsp56k_ram_bank02_read, dsp56k_ram_bank02_write)
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(dsp56k_shared_ram_read, dsp56k_shared_ram_write)
	AM_RANGE(0xe000, 0xffbf) AM_READWRITE(dsp56k_ram_bank04_read, dsp56k_ram_bank04_write)
ADDRESS_MAP_END

/**********************************************************************************/

static int cur_sound_region;

static void reset_sound_region(void)
{
	memory_set_bankptr(2, memory_region(REGION_CPU3) + 0x10000 + cur_sound_region*0x4000);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	cur_sound_region = (data & 0x1f);

	reset_sound_region();
}

static INTERRUPT_GEN(audio_interrupt)
{
	cpunum_set_input_line(machine, 2, INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK2)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe22f) AM_READWRITE(K054539_0_r, K054539_0_w)
	AM_RANGE(0xe230, 0xe3ff) AM_RAM
	AM_RANGE(0xe400, 0xe62f) AM_READWRITE(K054539_1_r, K054539_1_w)
	AM_RANGE(0xe630, 0xe7ff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch3_w)
	AM_RANGE(0xf002, 0xf002) AM_READ(soundlatch_r)
	AM_RANGE(0xf003, 0xf003) AM_READ(soundlatch2_r)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0xfff1, 0xfff3) AM_WRITE(MWA8_NOP)
ADDRESS_MAP_END

static const struct K054539interface k054539_interface =
{
	REGION_SOUND1
};

/**********************************************************************************/

static const gfx_layout bglayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4,
	  9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
 	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( plygonet )
	GFXDECODE_ENTRY( REGION_GFX2, 0, bglayout,     0x0000, 64 )
GFXDECODE_END

static MACHINE_DRIVER_START( plygonet )
	MDRV_CPU_ADD(M68EC020, 16000000)	/* 16 MHz (xtal is 32.0 MHz) */
	MDRV_CPU_PROGRAM_MAP(polygonet_map, 0)
	MDRV_CPU_VBLANK_INT_HACK(polygonet_interrupt, 2)

	MDRV_CPU_ADD(DSP56156, 10000000)		/* should be 40.0 MHz */
	MDRV_CPU_FLAGS(CPU_DISABLE)
	MDRV_CPU_PROGRAM_MAP(dsp56156_p_map, 0)
	MDRV_CPU_DATA_MAP(dsp56156_d_map, 0)

	MDRV_CPU_ADD_TAG("sound", Z80, 8000000)
	MDRV_CPU_PROGRAM_MAP(sound_map, 0)
	MDRV_CPU_PERIODIC_INT(audio_interrupt, 480)

	MDRV_GFXDECODE(plygonet)
	MDRV_NVRAM_HANDLER(polygonet)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1 )

	MDRV_PALETTE_LENGTH(32768)

	MDRV_VIDEO_START(polygonet)
	MDRV_VIDEO_UPDATE(polygonet)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.75)
	MDRV_SOUND_ROUTE(1, "right", 0.75)

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.75)
	MDRV_SOUND_ROUTE(1, "right", 0.75)
MACHINE_DRIVER_END

static INPUT_PORTS_START( polygonet )
	PORT_START

	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4)

	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* EEPROM data */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, "Monitors" )
	PORT_DIPSETTING( 0x00, "1 Monitor" )
	PORT_DIPSETTING( 0x10, "2 Monitors" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static DRIVER_INIT(polygonet)
{
	/* set default bankswitch */
	cur_sound_region = 2;
	reset_sound_region();

	/* allocate space for all the fun dsp56k banking */
	dsp56k_bank00_ram    = auto_malloc( (     0x1000  + (0x8*0x1000)) * 2) ;
	dsp56k_bank01_ram    = auto_malloc( (     0x1000  + (0x8*0x1000)) * 2) ;
	dsp56k_bank02_ram    = auto_malloc( ((0x8*0x4000) + (0x8*0x4000)) * 2) ;
	dsp56k_shared_ram_16 = auto_malloc( (     0x2000  + (0x8*0x2000)) * 2) ;
	dsp56k_bank04_ram    = auto_malloc( (     0x1fc0  + (0x8*0x1fc0)) * 2) ;
}

ROM_START( plygonet )
	/* main program */
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	ROM_LOAD32_BYTE( "305a01.4k", 0x000003, 512*1024, CRC(8bdb6c95) SHA1(e981833842f8fd89b9726901fbe2058444204792) )
	ROM_LOAD32_BYTE( "305a02.2k", 0x000002, 512*1024, CRC(4d7e32b3) SHA1(25731526535036972577637d186f02ae467296bd) )
	ROM_LOAD32_BYTE( "305a03.2h", 0x000001, 512*1024, CRC(36e4e3fe) SHA1(e8fcad4f196c9b225a0fbe70791493ff07c648a9) )
	ROM_LOAD32_BYTE( "305a04.4h", 0x000000, 512*1024, CRC(d8394e72) SHA1(eb6bcf8aedb9ba5843204ab8aacb735cbaafb74d) )

	/* Z80 sound program */
	ROM_REGION( 0x30000, REGION_CPU3, 0 )
	ROM_LOAD("305b05.7b", 0x000000, 0x20000, CRC(2d3d9654) SHA1(784a409df47cee877e507b8bbd3610d161d63753) )
	ROM_RELOAD( 0x10000, 0x20000)

	/* TTL text plane tiles */
	ROM_REGION( 0x20000, REGION_GFX1, 0 )
	ROM_LOAD( "305b06.18g", 0x000000, 0x20000, CRC(decd6e42) SHA1(4c23dcb1d68132d3381007096e014ee4b6007086) )

	/* '936 tiles */
 	ROM_REGION( 0x40000, REGION_GFX2, 0 )
	ROM_LOAD( "305b07.20d", 0x000000, 0x40000, CRC(e4320bc3) SHA1(b0bb2dac40d42f97da94516d4ebe29b1c3d77c37) )

	/* sound data */
	ROM_REGION( 0x200000, REGION_SOUND1, 0 )
	ROM_LOAD( "305b08.2e", 0x000000, 0x200000, CRC(874607df) SHA1(763b44a80abfbc355bcb9be8bf44373254976019) )
ROM_END

/*          ROM        parent   machine    inp        init */
GAME( 1993, plygonet, 0,       plygonet, polygonet, polygonet, ROT90, "Konami", "Polygonet Commanders (ver UAA)", GAME_NOT_WORKING | GAME_NO_SOUND )
