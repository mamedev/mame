/******************************************************************************

    pcw16.c
    system driver

    Kevin Thacker [MESS driver]

  Thankyou to:

    - Cliff Lawson @ Amstrad plc for his documentation (Anne ASIC documentation),
                    and extensive help.
            (web.ukonline.co.uk/cliff.lawson/)
            (www.amstrad.com)
    - John Elliot for his help and tips
            (he's written a CP/M implementation for the PCW16)
            (www.seasip.deomon.co.uk)
    - and others who offered their help (Richard Fairhurst, Richard Wildey)

    Hardware:
        - 2mb dram max,
        - 2mb flash-file memory max (in 2 1mb chips),
        - 16MHz Z80 (core combined in Anne ASIC),
        - Anne ASIC (keyboard interface, video (colours), dram/flash/rom paging,
        real time clock, "glue" logic for Super I/O)
        - Winbond Super I/O chip (PC type hardware - FDC, Serial, LPT, Hard-drive)
        - PC/AT keyboard - some keys are coloured to identify special functions, but
        these are the same as normal PC keys
        - PC Serial Mouse - uses Mouse System Mouse protocol
        - PC 1.44MB Floppy drive

    Primary Purpose:
        - built as a successor to the PCW8526/PCW9512 series
        - wordprocessor system (also contains spreadsheet and other office applications)
        - 16MHz processor used so proportional fonts and enhanced wordprocessing features
          are possible, true WYSIWYG wordprocessing.
        - flash-file can store documents.

    To Do:
        - reduce memory usage so it is more MESSD friendly
        - different configurations
        - implement configuration register
        - extract game-port hardware from pc driver - used in any PCW16 progs?
        - extract hard-drive code from PC driver and use in this driver
        - implement printer
        - .. anything else that requires implementing

     Info:
       - to use this driver you need a OS rescue disc.
       (HINT: This also contains the boot-rom)
      - the OS will be installed from the OS rescue disc into the Flash-ROM

    Uses "MEMCARD" dir to hold flash-file data.
    To use the power button, flick the dip switch off/on

 From comp.sys.amstrad.8bit FAQ:

  "Amstrad made the following PCW systems :

  - 1) PCW8256
  - 2) PCW8512
  - 3) PCW9512
  - 4) PCW9512+
  - 5) PcW10
  - 6) PcW16

  1 had 180K drives, 2 had a 180K A drive and a 720K B drive, 3 had only
  720K drives. All subsequent models had 3.5" disks using CP/M format at
  720K until 6 when it switched to 1.44MB in MS-DOS format. The + of
  model 4 was that it had a "real" parallel interface so could be sold
  with an external printer such as the Canon BJ10. The PcW10 wasn't
  really anything more than 4 in a more modern looking case.

  The PcW16 is a radical digression who's sole "raison d'etre" was to
  make a true WYSIWYG product but this meant a change in the screen and
  processor (to 16MHz) etc. which meant that it could not be kept
  compatible with the previous models (though documents ARE compatible)"


TODO:
- Verfiy uart model.


 ******************************************************************************/
/* PeT 19.October 2000
   added/changed printer support
   not working reliable, seams to expect parallelport in epp/ecp mode
   epp/ecp modes in parallel port not supported yet
   so ui disabled */

/* Core includes */
#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/pcw16.h"

/* Components */
#include "machine/pc_lpt.h"		/* PC-Parallel Port */
#include "machine/pckeybrd.h"	/* PC-AT keyboard */
#include "machine/pc_fdc.h"		/* change to superio later */
#include "machine/ins8250.h"	/* pc com port */
#include "sound/beep.h"			/* pcw/pcw16 beeper */
#include "machine/intelfsh.h"

/* Devices */
#include "formats/pc_dsk.h"		/* pc disk images */
#include "imagedev/flopdrv.h"
#include "machine/ram.h"

// interrupt counter
/* controls which bank of 2mb address space is paged into memory */

// output of 4-bit port from Anne ASIC
// code defining which INT fdc is connected to
// interrupt bits
// bit 7: ??
// bit 6: fdc
// bit 5: ??
// bit 3,4; Serial
// bit 2: Vsync state
// bit 1: keyboard int
// bit 0: Display ints

// debugging - write ram as seen by cpu
void pcw16_state::pcw16_refresh_ints()
{
	/* any bits set excluding vsync */
	if ((m_system_status & (~0x04))!=0)
	{
		machine().device("maincpu")->execute().set_input_line(0, HOLD_LINE);
	}
	else
	{
		machine().device("maincpu")->execute().set_input_line(0, CLEAR_LINE);
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(pcw16_state::pcw16_timer_callback)
{
	/* do not increment past 15 */
	if (m_interrupt_counter!=15)
	{
		m_interrupt_counter++;
		/* display int */
		m_system_status |= (1<<0);
	}

	if (m_interrupt_counter!=0)
	{
		pcw16_refresh_ints();
	}
}

static ADDRESS_MAP_START(pcw16_map, AS_PROGRAM, 8, pcw16_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(pcw16_mem_r, pcw16_mem_w)
//  AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank5")
//  AM_RANGE(0x4000, 0x7fff) AM_READ_BANK("bank2") AM_WRITE_BANK("bank6")
//  AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank7")
//  AM_RANGE(0xc000, 0xffff) AM_READ_BANK("bank4") AM_WRITE_BANK("bank8")
ADDRESS_MAP_END


WRITE8_MEMBER(pcw16_state::pcw16_palette_w)
{
	m_colour_palette[offset & 0x0f] = data & 31;
}

/*
static const char *const pcw16_write_handler_dram[4] =
{
	"bank5",
	"bank6",
	"bank7",
	"bank8"
};

static const char *const pcw16_read_handler_dram[4] =
{
	"bank1",
	"bank2",
	"bank3",
	"bank4"
};



// PCW16 Flash interface
// PCW16 can have two 1mb flash chips 

// read flash0 
static int pcw16_flash0_bank_handler_r(running_machine &machine, int bank, int offset)
{
	pcw16_state *state = machine.driver_data<pcw16_state>();
	intel_e28f008sa_device *flash = machine.device<intel_e28f008sa_device>("flash0");
	int flash_offset = (state->m_banks[bank]<<14) | offset;
	return flash->read(flash_offset);
}

// read flash1 
static int pcw16_flash1_bank_handler_r(running_machine &machine, int bank, int offset)
{
	pcw16_state *state = machine.driver_data<pcw16_state>();
	intel_e28f008sa_device *flash = machine.device<intel_e28f008sa_device>("flash1");
	int flash_offset = ((state->m_banks[bank]&0x03f)<<14) | offset;

	return flash->read(flash_offset);
}

// flash 0 
static  READ8_HANDLER(pcw16_flash0_bank_handler0_r)
{
	return pcw16_flash0_bank_handler_r(space.machine(),0, offset);
}

static  READ8_HANDLER(pcw16_flash0_bank_handler1_r)
{
	return pcw16_flash0_bank_handler_r(space.machine(),1, offset);
}

static  READ8_HANDLER(pcw16_flash0_bank_handler2_r)
{
	return pcw16_flash0_bank_handler_r(space.machine(),2, offset);
}

static  READ8_HANDLER(pcw16_flash0_bank_handler3_r)
{
	return pcw16_flash0_bank_handler_r(space.machine(),3, offset);
}

// flash 1 
static  READ8_HANDLER(pcw16_flash1_bank_handler0_r)
{
	return pcw16_flash1_bank_handler_r(space.machine(),0, offset);
}

static  READ8_HANDLER(pcw16_flash1_bank_handler1_r)
{
	return pcw16_flash1_bank_handler_r(space.machine(),1, offset);
}

static  READ8_HANDLER(pcw16_flash1_bank_handler2_r)
{
	return pcw16_flash1_bank_handler_r(space.machine(),2, offset);
}

static  READ8_HANDLER(pcw16_flash1_bank_handler3_r)
{
	return pcw16_flash1_bank_handler_r(space.machine(),3, offset);
}

static const struct { read8_space_func func; const char *name; } pcw16_flash0_bank_handlers_r[4] =
{
	{ FUNC(pcw16_flash0_bank_handler0_r) },
	{ FUNC(pcw16_flash0_bank_handler1_r) },
	{ FUNC(pcw16_flash0_bank_handler2_r) },
	{ FUNC(pcw16_flash0_bank_handler3_r) }
};

static const struct { read8_space_func func; const char *name; } pcw16_flash1_bank_handlers_r[4] =
{
	{ FUNC(pcw16_flash1_bank_handler0_r) },
	{ FUNC(pcw16_flash1_bank_handler1_r) },
	{ FUNC(pcw16_flash1_bank_handler2_r) },
	{ FUNC(pcw16_flash1_bank_handler3_r) }
};

// write flash0 
static void pcw16_flash0_bank_handler_w(running_machine &machine, int bank, int offset, int data)
{
	pcw16_state *state = machine.driver_data<pcw16_state>();
	intel_e28f008sa_device *flash = machine.device<intel_e28f008sa_device>("flash0");

	int flash_offset = (state->m_banks[bank]<<14) | offset;

	flash->write(flash_offset, data);
}

// read flash1 
static void pcw16_flash1_bank_handler_w(running_machine &machine, int bank, int offset, int data)
{
	pcw16_state *state = machine.driver_data<pcw16_state>();
	intel_e28f008sa_device *flash = machine.device<intel_e28f008sa_device>("flash1");

	int flash_offset = ((state->m_banks[bank]&0x03f)<<14) | offset;

	flash->write(flash_offset,data);
}

// flash 0 
static WRITE8_HANDLER(pcw16_flash0_bank_handler0_w)
{
	pcw16_flash0_bank_handler_w(space.machine(),0, offset, data);
}


static WRITE8_HANDLER(pcw16_flash0_bank_handler1_w)
{
	pcw16_flash0_bank_handler_w(space.machine(),1, offset, data);
}

static WRITE8_HANDLER(pcw16_flash0_bank_handler2_w)
{
	pcw16_flash0_bank_handler_w(space.machine(),2, offset, data);
}

static WRITE8_HANDLER(pcw16_flash0_bank_handler3_w)
{
	pcw16_flash0_bank_handler_w(space.machine(),3, offset, data);
}


// flash 1 
static WRITE8_HANDLER(pcw16_flash1_bank_handler0_w)
{
	pcw16_flash1_bank_handler_w(space.machine(),0, offset, data);
}


static WRITE8_HANDLER(pcw16_flash1_bank_handler1_w)
{
	pcw16_flash1_bank_handler_w(space.machine(),1, offset, data);
}

static WRITE8_HANDLER(pcw16_flash1_bank_handler2_w)
{
	pcw16_flash1_bank_handler_w(space.machine(),2, offset, data);
}

static WRITE8_HANDLER(pcw16_flash1_bank_handler3_w)
{
	pcw16_flash1_bank_handler_w(space.machine(),3, offset, data);
}

static const struct { write8_space_func func; const char *name; } pcw16_flash0_bank_handlers_w[4] =
{
	{ FUNC(pcw16_flash0_bank_handler0_w) },
	{ FUNC(pcw16_flash0_bank_handler1_w) },
	{ FUNC(pcw16_flash0_bank_handler2_w) },
	{ FUNC(pcw16_flash0_bank_handler3_w) }
};

static const struct { write8_space_func func; const char *name; } pcw16_flash1_bank_handlers_w[4] =
{
	{ FUNC(pcw16_flash1_bank_handler0_w) },
	{ FUNC(pcw16_flash1_bank_handler1_w) },
	{ FUNC(pcw16_flash1_bank_handler2_w) },
	{ FUNC(pcw16_flash1_bank_handler3_w) }
};

enum PCW16_RAM_TYPE
{
	// rom which is really first block of flash0 
	PCW16_MEM_ROM,
	// flash 0 
	PCW16_MEM_FLASH_1,
	// flash 1 i.e. unexpanded pcw16 
	PCW16_MEM_FLASH_2,
	// dram 
	PCW16_MEM_DRAM,
	// no mem. i.e. unexpanded pcw16 
	PCW16_MEM_NONE
};

READ8_MEMBER(pcw16_state::pcw16_no_mem_r)
{
	return 0x0ff;
}


static void pcw16_set_bank_handlers(running_machine &machine, int bank, PCW16_RAM_TYPE type)
{
    address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);
    pcw16_state *state = machine.driver_data<pcw16_state>();
    switch (type) {
    case PCW16_MEM_ROM:
        // rom
        space.install_read_bank((bank * 0x4000), (bank * 0x4000) + 0x3fff, pcw16_read_handler_dram[bank]);
        space.nop_write((bank * 0x4000), (bank * 0x4000) + 0x3fff);
        break;

    case PCW16_MEM_FLASH_1:
        // sram
        space.install_legacy_read_handler((bank * 0x4000), (bank * 0x4000) + 0x3fff, pcw16_flash0_bank_handlers_r[bank].func, pcw16_flash0_bank_handlers_r[bank].name);
        space.install_legacy_write_handler((bank * 0x4000), (bank * 0x4000) + 0x3fff, pcw16_flash0_bank_handlers_w[bank].func, pcw16_flash0_bank_handlers_w[bank].name);
        break;

    case PCW16_MEM_FLASH_2:
        space.install_legacy_read_handler((bank * 0x4000), (bank * 0x4000) + 0x3fff, pcw16_flash1_bank_handlers_r[bank].func, pcw16_flash1_bank_handlers_r[bank].name);
        space.install_legacy_write_handler((bank * 0x4000), (bank * 0x4000) + 0x3fff, pcw16_flash1_bank_handlers_w[bank].func, pcw16_flash1_bank_handlers_w[bank].name);
        break;

    case PCW16_MEM_NONE:
        space.install_read_handler((bank * 0x4000), (bank * 0x4000) + 0x3fff, read8_delegate(FUNC(pcw16_state::pcw16_no_mem_r),state));
        space.nop_write((bank * 0x4000), (bank * 0x4000) + 0x3fff);
        break;

    default:
    case PCW16_MEM_DRAM:
        // dram
        space.install_read_bank((bank * 0x4000), (bank * 0x4000) + 0x3fff, pcw16_read_handler_dram[bank]);
        space.install_write_bank((bank * 0x4000), (bank * 0x4000) + 0x3fff, pcw16_write_handler_dram[bank]);
        break;
    }

}

static void pcw16_update_bank(running_machine &machine, int bank)
{
    pcw16_state *state = machine.driver_data<pcw16_state>();
    unsigned char *mem_ptr = NULL; // machine.device<ram_device>(RAM_TAG)->pointer();
    int bank_id = 0;
    int bank_offs = 0;
    char bank1[10];
    char bank2[10];

    // get memory bank
    bank_id = state->m_banks[bank];


    if ((bank_id & 0x080)==0)
    {
        bank_offs = 0;

        if (bank_id<4)
        {
            //lower 4 banks are write protected. Use the rom
            //loaded
            mem_ptr = &state->memregion("maincpu")->base()[0x010000];
        }
        else
        {
            intelfsh8_device *flashdev;

            // nvram
            if ((bank_id & 0x040)==0)
            {
                flashdev = machine.device<intelfsh8_device>("flash0");
            }
            else
            {
                flashdev = machine.device<intelfsh8_device>("flash1");
            }

            mem_ptr = (unsigned char *)flashdev->space().get_read_ptr(0);
        }

    }
    else
    {
        bank_offs = 128;
         //dram
        mem_ptr = machine.device<ram_device>(RAM_TAG)->pointer();
    }

    mem_ptr = mem_ptr + ((bank_id - bank_offs)<<14);
    state->m_mem_ptr[bank] = (char*)mem_ptr;
    sprintf(bank1,"bank%d",(bank+1));
    sprintf(bank2,"bank%d",(bank+5));
    state->membank(bank1)->set_base(mem_ptr);
    state->membank(bank2)->set_base(mem_ptr);

    if ((bank_id & 0x080)==0)
    {
        // selections 0-3 within the first 64k are write protected
        if (bank_id<4)
        {
            // rom
            pcw16_set_bank_handlers(machine, bank, PCW16_MEM_ROM);
        }
        else
        {
            // selections 0-63 are for flash-rom 0, selections
            // 64-128 are for flash-rom 1
            if ((bank_id & 0x040)==0)
            {
                pcw16_set_bank_handlers(machine, bank, PCW16_MEM_FLASH_1);
            }
            else
            {
                pcw16_set_bank_handlers(machine, bank, PCW16_MEM_FLASH_2);
            }
        }
    }
    else
    {
        pcw16_set_bank_handlers(machine, bank, PCW16_MEM_DRAM);
    }
}


// update memory h/w 
static void pcw16_update_memory(running_machine &machine)
{
  pcw16_update_bank(machine, 0);
  pcw16_update_bank(machine, 1);
  pcw16_update_bank(machine, 2);
  pcw16_update_bank(machine, 3);

}
*/
UINT8 pcw16_state::read_bank_data(UINT8 type, UINT16 offset)
{
	if(type & 0x80) // DRAM
	{
		UINT8* RAM = machine().device<ram_device>(RAM_TAG)->pointer();
		return RAM[((type & 0x7f)*0x4000) + offset];
	}
	else  // ROM / Flash
	{
		if(type < 4)
		{
			UINT8* ROM = memregion("maincpu")->base();
			return ROM[((type & 0x03)*0x4000)+offset+0x10000];
		}
		if(type < 0x40)  // first flash
		{
			intel_e28f008sa_device* flash = machine().device<intel_e28f008sa_device>("flash0");
			return flash->read(((type & 0x3f)*0x4000)+offset);
		}
		else  // second flash
		{
			intel_e28f008sa_device* flash = machine().device<intel_e28f008sa_device>("flash1");
			return flash->read(((type & 0x3f)*0x4000)+offset);
		}
	}
}

void pcw16_state::write_bank_data(UINT8 type, UINT16 offset, UINT8 data)
{
	if(type & 0x80) // DRAM
	{
		UINT8* RAM = machine().device<ram_device>(RAM_TAG)->pointer();
		RAM[((type & 0x7f)*0x4000) + offset] = data;
	}
	else  // ROM / Flash
	{
		if(type < 4)
			return;  // first four sectors are write protected
		if(type < 0x40)  // first flash
		{
			intel_e28f008sa_device* flash = machine().device<intel_e28f008sa_device>("flash0");
			flash->write(((type & 0x3f)*0x4000)+offset,data);
		}
		else  // second flash
		{
			intel_e28f008sa_device* flash = machine().device<intel_e28f008sa_device>("flash1");
			flash->write(((type & 0x3f)*0x4000)+offset,data);
		}
	}
}

UINT8 pcw16_state::pcw16_read_mem(UINT8 bank, UINT16 offset)
{
	switch(bank)
	{
	case 0:
		return read_bank_data(m_banks[bank],offset);
	case 1:
		return read_bank_data(m_banks[bank],offset);
	case 2:
		return read_bank_data(m_banks[bank],offset);
	case 3:
		return read_bank_data(m_banks[bank],offset);
	}
	return 0xff;
}

void pcw16_state::pcw16_write_mem(UINT8 bank, UINT16 offset, UINT8 data)
{
	switch(bank)
	{
	case 0:
		write_bank_data(m_banks[bank],offset,data);
		break;
	case 1:
		write_bank_data(m_banks[bank],offset,data);
		break;
	case 2:
		write_bank_data(m_banks[bank],offset,data);
		break;
	case 3:
		write_bank_data(m_banks[bank],offset,data);
		break;
	}
}

READ8_MEMBER(pcw16_state::pcw16_mem_r)
{
	if(offset < 0x4000)
		return pcw16_read_mem(0,offset);
	if(offset >= 0x4000 && offset < 0x8000)
		return pcw16_read_mem(1,offset-0x4000);
	if(offset >= 0x8000 && offset < 0xc000)
		return pcw16_read_mem(2,offset-0x8000);
	if(offset >= 0xc000 && offset < 0x10000)
		return pcw16_read_mem(3,offset-0xc000);

	return 0xff;
}

WRITE8_MEMBER(pcw16_state::pcw16_mem_w)
{
	if(offset < 0x4000)
		pcw16_write_mem(0,offset,data);
	if(offset >= 0x4000 && offset < 0x8000)
		pcw16_write_mem(1,offset-0x4000,data);
	if(offset >= 0x8000 && offset < 0xc000)
		pcw16_write_mem(2,offset-0x8000,data);
	if(offset >= 0xc000 && offset < 0x10000)
		pcw16_write_mem(3,offset-0xc000,data);
}

READ8_MEMBER(pcw16_state::pcw16_bankhw_r)
{
//  logerror("bank r: %d \n", offset);

	return m_banks[offset];
}

WRITE8_MEMBER(pcw16_state::pcw16_bankhw_w)
{
	//logerror("bank w: %d block: %02x\n", offset, data);

	m_banks[offset] = data;

	//pcw16_update_memory(machine());
}

WRITE8_MEMBER(pcw16_state::pcw16_video_control_w)
{
	//logerror("video control w: %02x\n", data);

	m_video_control = data;
}

/* PCW16 KEYBOARD */

//unsigned char pcw16_keyboard_status;



#define PCW16_KEYBOARD_PARITY_MASK	(1<<7)
#define PCW16_KEYBOARD_STOP_BIT_MASK (1<<6)
#define PCW16_KEYBOARD_START_BIT_MASK (1<<5)
#define PCW16_KEYBOARD_BUSY_STATUS	(1<<4)
#define PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK (1<<1)
#define PCW16_KEYBOARD_TRANSMIT_MODE (1<<0)

#define PCW16_KEYBOARD_RESET_INTERFACE (1<<2)

#define PCW16_KEYBOARD_DATA	(1<<1)
#define PCW16_KEYBOARD_CLOCK (1<<0)

/* parity table. Used to set parity bit in keyboard status register */

void pcw16_state::pcw16_keyboard_init()
{
	int i;
	int b;

	/* if sum of all bits in the byte is even, then the data
    has even parity, otherwise it has odd parity */
	for (i=0; i<256; i++)
	{
		int data;
		int sum;

		sum = 0;
		data = i;

		for (b=0; b<8; b++)
		{
			sum+=data & 0x01;

			data = data>>1;
		}

		m_keyboard_parity_table[i] = sum & 0x01;
	}


	/* clear int */
	pcw16_keyboard_int(0);
	/* reset state */
	m_keyboard_state = 0;
	/* reset ready for transmit */
	pcw16_keyboard_reset();
}

void pcw16_state::pcw16_keyboard_refresh_outputs()
{
	/* generate output bits */
	m_keyboard_bits_output = m_keyboard_bits;

	/* force clock low? */
	if (m_keyboard_state & PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK)
	{
		m_keyboard_bits_output &= ~PCW16_KEYBOARD_CLOCK;
	}
}

void pcw16_state::pcw16_keyboard_set_clock_state(int state)
{
	m_keyboard_bits &= ~PCW16_KEYBOARD_CLOCK;

	if (state)
	{
		m_keyboard_bits |= PCW16_KEYBOARD_CLOCK;
	}

	pcw16_keyboard_refresh_outputs();
}

void pcw16_state::pcw16_keyboard_int(int state)
{
	m_system_status &= ~(1<<1);

	if (state)
	{
		m_system_status |= (1<<1);
	}

	pcw16_refresh_ints();
}

void pcw16_state::pcw16_keyboard_reset()
{
	/* clock set to high */
	pcw16_keyboard_set_clock_state(1);
}

/* interfaces to a pc-at keyboard */
READ8_MEMBER(pcw16_state::pcw16_keyboard_data_shift_r)
{
	//logerror("keyboard data shift r: %02x\n", m_keyboard_data_shift);
	m_keyboard_state &= ~(PCW16_KEYBOARD_BUSY_STATUS);

	pcw16_keyboard_int(0);
	/* reset for reception */
	pcw16_keyboard_reset();

	/* read byte */
	return m_keyboard_data_shift;
}

/* if force keyboard clock is low it is safe to send */
int pcw16_state::pcw16_keyboard_can_transmit()
{
	/* clock is not forced low */
	/* and not busy - i.e. not already sent a char */
	return ((m_keyboard_bits_output & PCW16_KEYBOARD_CLOCK)!=0);
}

#ifdef UNUSED_FUNCTION
/* issue a begin byte transfer */
static void pcw16_begin_byte_transfer(void)
{
}
#endif

/* signal a code has been received */
void pcw16_state::pcw16_keyboard_signal_byte_received(int data)
{
	/* clear clock */
	pcw16_keyboard_set_clock_state(0);

	/* set code in shift register */
	m_keyboard_data_shift = data;
	/* busy */
	m_keyboard_state |= PCW16_KEYBOARD_BUSY_STATUS;

	/* initialise start, stop and parity bits */
	m_keyboard_state &= ~PCW16_KEYBOARD_START_BIT_MASK;
	m_keyboard_state |=PCW16_KEYBOARD_STOP_BIT_MASK;

	/* "Keyboard data has odd parity, so the parity bit in the
    status register should only be set when the shift register
    data itself has even parity. */

	m_keyboard_state &= ~PCW16_KEYBOARD_PARITY_MASK;

	/* if data has even parity, set parity bit */
	if ((m_keyboard_parity_table[data])==0)
		m_keyboard_state |= PCW16_KEYBOARD_PARITY_MASK;

	pcw16_keyboard_int(1);
}


WRITE8_MEMBER(pcw16_state::pcw16_keyboard_data_shift_w)
{
	//logerror("Keyboard Data Shift: %02x\n", data);
	/* writing to shift register clears parity */
	/* writing to shift register clears start bit */
	m_keyboard_state &= ~(
		PCW16_KEYBOARD_PARITY_MASK |
		PCW16_KEYBOARD_START_BIT_MASK);

	/* writing to shift register sets stop bit */
	m_keyboard_state |= PCW16_KEYBOARD_STOP_BIT_MASK;

	m_keyboard_data_shift = data;

}

READ8_MEMBER(pcw16_state::pcw16_keyboard_status_r)
{
	/* bit 2,3 are bits 8 and 9 of vdu pointer */
	return (m_keyboard_state &
		(PCW16_KEYBOARD_PARITY_MASK |
		 PCW16_KEYBOARD_STOP_BIT_MASK |
		 PCW16_KEYBOARD_START_BIT_MASK |
		 PCW16_KEYBOARD_BUSY_STATUS |
		 PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK |
		 PCW16_KEYBOARD_TRANSMIT_MODE));
}

WRITE8_MEMBER(pcw16_state::pcw16_keyboard_control_w)
{
	//logerror("Keyboard control w: %02x\n",data);

	m_keyboard_previous_state = m_keyboard_state;

	/* if set, set parity */
	if (data & 0x080)
	{
		m_keyboard_state |= PCW16_KEYBOARD_PARITY_MASK;
	}

	/* clear read/write bits */
	m_keyboard_state &=
		~(PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK |
			PCW16_KEYBOARD_TRANSMIT_MODE);
	/* set read/write bits from data */
	m_keyboard_state |= (data & 0x03);

	if (data & PCW16_KEYBOARD_RESET_INTERFACE)
	{
		pcw16_keyboard_reset();
	}

	if (data & PCW16_KEYBOARD_TRANSMIT_MODE)
	{
		/* force clock changed */
		if (((m_keyboard_state^m_keyboard_previous_state) & PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK)!=0)
		{
			/* just cleared? */
			if ((m_keyboard_state & PCW16_KEYBOARD_FORCE_KEYBOARD_CLOCK)==0)
			{

				/* write */
				/* busy */
				m_keyboard_state |= PCW16_KEYBOARD_BUSY_STATUS;
				/* keyboard takes data */
				at_keyboard_write(machine(),m_keyboard_data_shift);
				/* set clock low - no furthur transmissions */
				pcw16_keyboard_set_clock_state(0);
				/* set int */
				pcw16_keyboard_int(1);
			}
		}


	}

	if (((m_keyboard_state^m_keyboard_previous_state) & PCW16_KEYBOARD_TRANSMIT_MODE)!=0)
	{
		if ((m_keyboard_state & PCW16_KEYBOARD_TRANSMIT_MODE)==0)
		{
			if ((m_system_status & (1<<1))!=0)
			{
				pcw16_keyboard_int(0);
			}
		}
	}

	pcw16_keyboard_refresh_outputs();
}


TIMER_DEVICE_CALLBACK_MEMBER(pcw16_state::pcw16_keyboard_timer_callback)
{
	at_keyboard_polling();
	if (pcw16_keyboard_can_transmit())
	{
		int data;

		data = at_keyboard_read();

		if (data!=-1)
		{
//          if (data==4)
//          {
//              pcw16_dump_cpu_ram();
//          }

			pcw16_keyboard_signal_byte_received(data);
		}
	}
	// TODO: fix
	subdevice<ns16550_device>("ns16550_2")->ri_w((machine().root_device().ioport("EXTRA")->read() & 0x040) ? 0 : 1);
}


static const int rtc_days_in_each_month[]=
{
	31,/* jan */
	28, /* feb */
	31, /* march */
	30, /* april */
	31, /* may */
	30, /* june */
	31, /* july */
	31, /* august */
	30, /* september */
	31, /* october */
	30, /* november */
	31	/* december */
};

static const int rtc_days_in_february[] =
{
	29, 28, 28, 28
};

void pcw16_state::rtc_setup_max_days()
{
	/* february? */
	if (m_rtc_months == 2)
	{
		/* low two bits of year select number of days in february */
		m_rtc_days_max = rtc_days_in_february[m_rtc_years & 0x03];
	}
	else
	{
		m_rtc_days_max = (unsigned char)rtc_days_in_each_month[m_rtc_months];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pcw16_state::rtc_timer_callback)
{
	int fraction_of_second;

	/* halt counter? */
	if ((m_rtc_control & 0x01)!=0)
	{
		/* no */

		/* increment 256th's of a second register */
		fraction_of_second = m_rtc_256ths_seconds+1;
		/* add bit 8 = overflow */
		m_rtc_seconds+=(fraction_of_second>>8);
		/* ensure counter is in range 0-255 */
		m_rtc_256ths_seconds = fraction_of_second & 0x0ff;
	}

	if (m_rtc_seconds>59)
	{
		m_rtc_seconds = 0;

		m_rtc_minutes++;

		if (m_rtc_minutes>59)
		{
			m_rtc_minutes = 0;

			m_rtc_hours++;

			if (m_rtc_hours>23)
			{
				m_rtc_hours = 0;

				m_rtc_days++;

				if (m_rtc_days > m_rtc_days_max)
				{
					m_rtc_days = 1;

					m_rtc_months++;

					if (m_rtc_months>12)
					{
						m_rtc_months = 1;

						/* 7 bit year counter */
						m_rtc_years = (m_rtc_years + 1) & 0x07f;

					}

					rtc_setup_max_days();
				}

			}


		}
	}
}

READ8_MEMBER(pcw16_state::rtc_year_invalid_r)
{
	/* year in lower 7 bits. RTC Invalid status is m_rtc_control bit 0
    inverted */
	return (m_rtc_years & 0x07f) | (((m_rtc_control & 0x01)<<7)^0x080);
}

READ8_MEMBER(pcw16_state::rtc_month_r)
{
	return m_rtc_months;
}

READ8_MEMBER(pcw16_state::rtc_days_r)
{
	return m_rtc_days;
}

READ8_MEMBER(pcw16_state::rtc_hours_r)
{
	return m_rtc_hours;
}

READ8_MEMBER(pcw16_state::rtc_minutes_r)
{
	return m_rtc_minutes;
}

READ8_MEMBER(pcw16_state::rtc_seconds_r)
{
	return m_rtc_seconds;
}

READ8_MEMBER(pcw16_state::rtc_256ths_seconds_r)
{
	return m_rtc_256ths_seconds;
}

WRITE8_MEMBER(pcw16_state::rtc_control_w)
{
	/* write control */
	m_rtc_control = data;
}

WRITE8_MEMBER(pcw16_state::rtc_seconds_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_seconds = data;
}

WRITE8_MEMBER(pcw16_state::rtc_minutes_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_minutes = data;
}

WRITE8_MEMBER(pcw16_state::rtc_hours_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_hours = data;
}

WRITE8_MEMBER(pcw16_state::rtc_days_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_days = data;
}

WRITE8_MEMBER(pcw16_state::rtc_month_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_months = data;

	rtc_setup_max_days();
}


WRITE8_MEMBER(pcw16_state::rtc_year_w)
{
	/* TODO: Writing register could cause next to increment! */
	m_rtc_hours = data;

	rtc_setup_max_days();
}


static void pcw16_trigger_fdc_int(running_machine &machine)
{
	pcw16_state *drvstate = machine.driver_data<pcw16_state>();
	int state;

	state = drvstate->m_system_status & (1<<6);

	switch (drvstate->m_fdc_int_code)
	{
		/* nmi */
		case 0:
		{
			/* I'm assuming that the nmi is edge triggered */
			/* a interrupt from the fdc will cause a change in line state, and
            the nmi will be triggered, but when the state changes because the int
            is cleared this will not cause another nmi */
			/* I'll emulate it like this to be sure */

			if (state!=drvstate->m_previous_fdc_int_state)
			{
				if (state)
				{
					/* I'll pulse it because if I used hold-line I'm not sure
                    it would clear - to be checked */
					machine.device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
				}
			}
		}
		break;

		/* attach fdc to int */
		case 1:
		{
			drvstate->pcw16_refresh_ints();
		}
		break;

		/* do not interrupt */
		default:
			break;
	}

	drvstate->m_previous_fdc_int_state = state;
}

READ8_MEMBER(pcw16_state::pcw16_system_status_r)
{
//  logerror("system status r: \n");

	return m_system_status | (ioport("EXTRA")->read() & 0x04);
}

READ8_MEMBER(pcw16_state::pcw16_timer_interrupt_counter_r)
{
	int data;

	data = m_interrupt_counter;

	m_interrupt_counter = 0;
	/* clear display int */
	m_system_status &= ~(1<<0);

	pcw16_refresh_ints();

	return data;
}


WRITE8_MEMBER(pcw16_state::pcw16_system_control_w)
{
	device_t *speaker = machine().device(BEEPER_TAG);
	//logerror("0x0f8: function: %d\n",data);

	/* lower 4 bits define function code */
	switch (data & 0x0f)
	{
		/* no effect */
		case 0x00:
		case 0x09:
		case 0x0a:
		case 0x0d:
		case 0x0e:
			break;

		/* system reset */
		case 0x01:
			break;

		/* connect IRQ6 input to /NMI */
		case 0x02:
		{
			m_fdc_int_code = 0;
		}
		break;

		/* connect IRQ6 input to /INT */
		case 0x03:
		{
			m_fdc_int_code = 1;
		}
		break;

		/* dis-connect IRQ6 input from /NMI and /INT */
		case 0x04:
		{
			m_fdc_int_code = 2;
		}
		break;

		/* set terminal count */
		case 0x05:
		{
			pc_fdc_set_tc_state(machine(), 1);
		}
		break;

		/* clear terminal count */
		case 0x06:
		{
			pc_fdc_set_tc_state(machine(), 0);
		}
		break;

		/* bleeper on */
		case 0x0b:
		{
                        beep_set_state(speaker,1);
		}
		break;

		/* bleeper off */
		case 0x0c:
		{
                        beep_set_state(speaker,0);
		}
		break;

		/* drive video outputs */
		case 0x07:
		{
		}
		break;

		/* float video outputs */
		case 0x08:
		{
		}
		break;

		/* set 4-bit output port to value X */
		case 0x0f:
		{
			/* bit 7 - ?? */
			/* bit 6 - ?? */
			/* bit 5 - green/red led (1==green)*/
			/* bit 4 - monitor on/off (1==on) */

			m_4_bit_port = data>>4;


		}
		break;
	}
}

/**** SUPER I/O connections */

/* write to Super I/O chip. FDC Data Rate. */
WRITE8_MEMBER(pcw16_state::pcw16_superio_fdc_datarate_w)
{
	pc_fdc_w(space, PC_FDC_DATA_RATE_REGISTER,data);
}

/* write to Super I/O chip. FDC Digital output register */
WRITE8_MEMBER(pcw16_state::pcw16_superio_fdc_digital_output_register_w)
{
	pc_fdc_w(space, PC_FDC_DIGITAL_OUTPUT_REGISTER, data);
}

/* write to Super I/O chip. FDC Data Register */
WRITE8_MEMBER(pcw16_state::pcw16_superio_fdc_data_w)
{
	pc_fdc_w(space, PC_FDC_DATA_REGISTER, data);
}

/* write to Super I/O chip. FDC Data Register */
READ8_MEMBER(pcw16_state::pcw16_superio_fdc_data_r)
{
	return pc_fdc_r(space, PC_FDC_DATA_REGISTER);
}

/* write to Super I/O chip. FDC Main Status Register */
READ8_MEMBER(pcw16_state::pcw16_superio_fdc_main_status_register_r)
{
	return pc_fdc_r(space, PC_FDC_MAIN_STATUS_REGISTER);
}

READ8_MEMBER(pcw16_state::pcw16_superio_fdc_digital_input_register_r)
{
	return pc_fdc_r(space, PC_FDC_DIGITIAL_INPUT_REGISTER);
}

static void pcw16_fdc_interrupt(running_machine &machine, int state)
{
	pcw16_state *drvstate = machine.driver_data<pcw16_state>();
	/* IRQ6 */
	/* bit 6 of PCW16 system status indicates floppy ints */
	drvstate->m_system_status &= ~(1<<6);

	if (state)
	{
		drvstate->m_system_status |= (1<<6);
	}

	pcw16_trigger_fdc_int(machine);
}

static device_t * pcw16_get_device(running_machine &machine)
{
	return machine.device("upd765");
}

static const struct pc_fdc_interface pcw16_fdc_interface=
{
	pcw16_fdc_interrupt,
	NULL,
	NULL,
	pcw16_get_device
};


WRITE_LINE_MEMBER(pcw16_state::pcw16_com_interrupt_1)
{
	m_system_status &= ~(1 << 4);

	if ( state ) {
		m_system_status |= (1 << 4);
	}

	pcw16_refresh_ints();
}


WRITE_LINE_MEMBER(pcw16_state::pcw16_com_interrupt_2)
{
	m_system_status &= ~(1 << 3);

	if ( state ) {
		m_system_status |= (1 << 3);
	}

	pcw16_refresh_ints();
}

WRITE_LINE_MEMBER(pcw16_state::pcw16_com_tx_0){ }
WRITE_LINE_MEMBER(pcw16_state::pcw16_com_dtr_0){ }
WRITE_LINE_MEMBER(pcw16_state::pcw16_com_rts_0){ }

WRITE_LINE_MEMBER(pcw16_state::pcw16_com_tx_1){ }
WRITE_LINE_MEMBER(pcw16_state::pcw16_com_dtr_1){ }
WRITE_LINE_MEMBER(pcw16_state::pcw16_com_rts_1){ }

static const ins8250_interface pcw16_com_interface[2]=
{
	{
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_tx_0),
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_dtr_0),
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_rts_0),
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_interrupt_1),
		DEVCB_NULL,
		DEVCB_NULL
	},
	{
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_tx_1),
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_dtr_1),
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_rts_1),
		DEVCB_DRIVER_LINE_MEMBER(pcw16_state,pcw16_com_interrupt_2),
		DEVCB_NULL,
		DEVCB_NULL
	}
};



static ADDRESS_MAP_START(pcw16_io, AS_IO, 8, pcw16_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* super i/o chip */
	AM_RANGE(0x01a, 0x01a) AM_WRITE(pcw16_superio_fdc_digital_output_register_w)
    AM_RANGE(0x01c, 0x01c) AM_READ(pcw16_superio_fdc_main_status_register_r)
	AM_RANGE(0x01d, 0x01d) AM_READWRITE(pcw16_superio_fdc_data_r, pcw16_superio_fdc_data_w)
	AM_RANGE(0x01f, 0x01f) AM_READWRITE(pcw16_superio_fdc_digital_input_register_r, pcw16_superio_fdc_datarate_w)
	AM_RANGE(0x020, 0x027) AM_DEVREADWRITE("ns16550_1", ns16550_device, ins8250_r, ins8250_w)
	AM_RANGE(0x028, 0x02f) AM_DEVREADWRITE("ns16550_2", ns16550_device, ins8250_r, ins8250_w)
	AM_RANGE(0x038, 0x03a) AM_DEVREADWRITE_LEGACY("lpt", pc_lpt_r, pc_lpt_w)
	/* anne asic */
	AM_RANGE(0x0e0, 0x0ef) AM_WRITE(pcw16_palette_w)
	AM_RANGE(0x0f0, 0x0f3) AM_READWRITE(pcw16_bankhw_r, pcw16_bankhw_w)
	AM_RANGE(0x0f4, 0x0f4) AM_READWRITE(pcw16_keyboard_data_shift_r, pcw16_keyboard_data_shift_w)
	AM_RANGE(0x0f5, 0x0f5) AM_READWRITE(pcw16_keyboard_status_r, pcw16_keyboard_control_w)
	AM_RANGE(0x0f7, 0x0f7) AM_READWRITE(pcw16_timer_interrupt_counter_r, pcw16_video_control_w)
	AM_RANGE(0x0f8, 0x0f8) AM_READWRITE(pcw16_system_status_r, pcw16_system_control_w)
	AM_RANGE(0x0f9, 0x0f9) AM_READWRITE(rtc_256ths_seconds_r, rtc_control_w)
	AM_RANGE(0x0fa, 0x0fa) AM_READWRITE(rtc_seconds_r, rtc_seconds_w)
	AM_RANGE(0x0fb, 0x0fb) AM_READWRITE(rtc_minutes_r, rtc_minutes_w)
	AM_RANGE(0x0fc, 0x0fc) AM_READWRITE(rtc_hours_r, rtc_hours_w)
	AM_RANGE(0x0fd, 0x0fd) AM_READWRITE(rtc_days_r, rtc_days_w)
	AM_RANGE(0x0fe, 0x0fe) AM_READWRITE(rtc_month_r, rtc_month_w)
	AM_RANGE(0x0ff, 0x0ff) AM_READWRITE(rtc_year_invalid_r, rtc_year_w)
ADDRESS_MAP_END


static void pcw16_reset(running_machine &machine)
{
	pcw16_state *state = machine.driver_data<pcw16_state>();
	/* initialise defaults */
	state->m_fdc_int_code = 2;
	/* clear terminal count */
	pc_fdc_set_tc_state(machine, 0);
	/* select first rom page */
	state->m_banks[0] = 0;
//  pcw16_update_memory(machine);

	/* temp rtc setup */
	state->m_rtc_seconds = 0;
	state->m_rtc_minutes = 0;
	state->m_rtc_hours = 0;
	state->m_rtc_days_max = 0;
	state->m_rtc_days = 1;
	state->m_rtc_months = 1;
	state->m_rtc_years = 0;
	state->m_rtc_control = 1;
	state->m_rtc_256ths_seconds = 0;

	state->pcw16_keyboard_init();
}


void pcw16_state::machine_start()
{
	device_t *speaker = machine().device(BEEPER_TAG);
	m_system_status = 0;
	m_interrupt_counter = 0;

	pc_fdc_init(machine(), &pcw16_fdc_interface);

	/* initialise keyboard */
	at_keyboard_init(machine(), AT_KEYBOARD_TYPE_AT);
	at_keyboard_set_scan_code_set(3);

	pcw16_reset(machine());

	beep_set_state(speaker,0);
	beep_set_frequency(speaker,3750);
}

static INPUT_PORTS_START(pcw16)
	PORT_START("EXTRA")
	/* vblank */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")
	/* power switch - default is on */
	PORT_DIPNAME(0x40, 0x40, "Power Switch/Suspend")
	PORT_DIPSETTING(0x0, DEF_STR( Off) )
	PORT_DIPSETTING(0x40, DEF_STR( On) )

	PORT_INCLUDE( at_keyboard )		/* IN4 - IN11 */
INPUT_PORTS_END


static const pc_lpt_interface pcw16_lpt_config =
{
	DEVCB_CPU_INPUT_LINE("maincpu", 0)
};

static const floppy_interface pcw16_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(pc),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( pcw16, pcw16_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 16000000)
	MCFG_CPU_PROGRAM_MAP(pcw16_map)
	MCFG_CPU_IO_MAP(pcw16_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))


	MCFG_NS16550_ADD( "ns16550_1", pcw16_com_interface[0], XTAL_1_8432MHz )		/* TODO: Verify uart model */

	MCFG_NS16550_ADD( "ns16550_2", pcw16_com_interface[1], XTAL_1_8432MHz )		/* TODO: Verify uart model */

    /* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(PCW16_SCREEN_WIDTH, PCW16_SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, PCW16_SCREEN_WIDTH-1, 0, PCW16_SCREEN_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(pcw16_state, screen_update_pcw16)

	MCFG_PALETTE_LENGTH(PCW16_NUM_COLOURS)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* printer */
	MCFG_PC_LPT_ADD("lpt", pcw16_lpt_config)
	MCFG_UPD765A_ADD("upd765", pc_fdc_upd765_connected_interface)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(pcw16_floppy_interface)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_INTEL_E28F008SA_ADD("flash0")
	MCFG_INTEL_E28F008SA_ADD("flash1")

	/* video ints */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("video_timer", pcw16_state, pcw16_timer_callback, attotime::from_usec(5830))
	/* rtc timer */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("rtc_timer", pcw16_state, rtc_timer_callback, attotime::from_hz(256))
	/* keyboard timer */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", pcw16_state, pcw16_keyboard_timer_callback, attotime::from_hz(50))
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/* the lower 64k of the flash-file memory is write protected. This contains the boot
    rom. The boot rom is also on the OS rescue disc. Handy! */
ROM_START(pcw16)
	ROM_REGION((0x010000+524288), "maincpu",0)
	ROM_LOAD("pcw045.sys",0x10000, 524288, CRC(c642f498) SHA1(8a5c05de92e7b2c5acdfb038217503ad363285b5))
ROM_END


/*     YEAR  NAME     PARENT    COMPAT  MACHINE    INPUT     INIT    COMPANY          FULLNAME */
COMP( 1995, pcw16,	  0,		0,		pcw16,	   pcw16, driver_device,    0,		"Amstrad plc",   "PCW16", GAME_NOT_WORKING )
