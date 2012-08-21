/***************************************************************************

    6883sam.c

    Motorola 6883 Synchronous Address Multiplexer

    The Motorola 6883 SAM has 16 bits worth of state, but the state is changed
    by writing into a 32 byte address space; odd bytes set bits and even bytes
    clear bits.  Here is the layout:

        31  Set     TY  Map Type            0: RAM/ROM  1: All RAM
        30  Clear   TY  Map Type
        29  Set     M1  Memory Size         00: 4K      10: 64K Dynamic
        28  Clear   M1  Memory Size         01: 16K     11: 64K Static
        27  Set     M0  Memory Size
        26  Clear   M0  Memory Size
        25  Set     R1  MPU Rate            00: Slow    10: Fast
        24  Clear   R1  MPU Rate            01: Dual    11: Fast
        23  Set     R0  MPU Rate
        22  Clear   R0  MPU Rate
        21  Set     P1  Page #1             0: Low      1: High
        20  Clear   P1  Page #1
        19  Set     F6  Display Offset
        18  Clear   F6  Display Offset
        17  Set     F5  Display Offset
        16  Clear   F5  Display Offset
        15  Set     F4  Display Offset
        14  Clear   F4  Display Offset
        13  Set     F3  Display Offset
        12  Clear   F3  Display Offset
        11  Set     F2  Display Offset
        10  Clear   F2  Display Offset
         9  Set     F1  Display Offset
         8  Clear   F1  Display Offset
         7  Set     F0  Display Offset
         6  Clear   F0  Display Offset
         5  Set     V2  VDG Mode
         4  Clear   V2  VDG Mode
         3  Set     V1  VDG Mode
         2  Clear   V1  VDG Mode
         1  Set     V0  VDG Mode
         0  Clear   V0  VDG Mode

    All parts of the SAM are fully emulated except R1/R0 (the changes in the
    MPU rate are approximated) and M1/M0

***************************************************************************/


#include "machine/6883sam.h"

const device_type SAM6883 = &device_creator<sam6883_device>;


//-------------------------------------------------
//  ctor
//-------------------------------------------------

sam6883_device::sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SAM6883, "SAM6883", tag, owner, clock)
{
	memset(m_banks, '\0', sizeof(m_banks));
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam6883_device::device_start()
{
	const sam6883_interface *config = (const sam6883_interface *) static_config();

	/* find the CPU */
	m_cpu = machine().device<cpu_device>(config->m_cpu_tag);
	m_cpu_space = m_cpu->space(config->m_cpu_space);

	/* resolve callbacks */
	m_res_input_func.resolve(config->m_input_func, *this);

	/* install SAM handlers */
	m_cpu_space->install_read_handler(0xFFC0, 0xFFDF, 0, 0, read8_delegate(FUNC(sam6883_device::read), this));
	m_cpu_space->install_write_handler(0xFFC0, 0xFFDF, 0, 0, write8_delegate(FUNC(sam6883_device::write), this));

	/* save state support */
	save_item(NAME(m_sam_state));
	save_item(NAME(m_counter));
	save_item(NAME(m_counter_xdiv));
	save_item(NAME(m_counter_ydiv));
}



//-------------------------------------------------
//  configure_bank - bank configuration
//-------------------------------------------------

void sam6883_device::configure_bank(int bank, UINT8 *memory, UINT32 memory_size, bool is_read_only)
{
	configure_bank(bank, memory, memory_size, is_read_only, read8_delegate(), write8_delegate());
}



//-------------------------------------------------
//  configure_bank - bank configuration
//-------------------------------------------------

void sam6883_device::configure_bank(int bank, read8_delegate rhandler, write8_delegate whandler)
{
	configure_bank(bank, NULL, 0, false, rhandler, whandler);
}



//-------------------------------------------------
//  configure_bank - bank configuration
//-------------------------------------------------

void sam6883_device::configure_bank(int bank, UINT8 *memory, UINT32 memory_size, bool is_read_only, read8_delegate rhandler, write8_delegate whandler)
{
	assert((bank >= 0) && (bank < sizeof(m_banks) / sizeof(m_banks[0])));
	m_banks[bank].m_memory = memory;
	m_banks[bank].m_memory_size = memory_size;
	m_banks[bank].m_memory_read_only = is_read_only;
	m_banks[bank].m_rhandler = rhandler;
	m_banks[bank].m_whandler = whandler;

	/* if we're configuring a bank that never changes, update it now */
	switch(bank)
	{
		case 4:
			update_bank(4, 0xFF00, 0xFF1F, 0x0000);
			break;
		case 5:
			update_bank(5, 0xFF20, 0xFF3F, 0x0000);
			break;
		case 6:
			update_bank(6, 0xFF40, 0xFF5F, 0x0000);
			break;
		case 7:
			update_bank(7, 0xFF60, 0xFFBF, 0x0000);
			break;
		case 2:
			update_bank(2, 0xFFE0, 0xFFFF, 0x1FE0);
			break;
	}
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sam6883_device::device_reset()
{
	m_counter = 0;
	m_counter_xdiv = 0;
	m_counter_ydiv = 0;
	m_sam_state = 0x0000;
	update_state();
}



//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void sam6883_device::device_post_load()
{
	device_t::device_post_load();
	update_state();
}



//-------------------------------------------------
//  update_state
//-------------------------------------------------

void sam6883_device::update_state(void)
{
	update_memory();
	update_cpu_clock();
}



//-------------------------------------------------
//  update_memory
//-------------------------------------------------

void sam6883_device::update_memory(void)
{
	/* Memory size - allowed restricting memory accesses to something less than
     * 32k
     *
     * This was a SAM switch that occupied 4 addresses:
     *
     *      $FFDD   (set)   R1
     *      $FFDC   (clear) R1
     *      $FFDB   (set)   R0
     *      $FFDA   (clear) R0
     *
     * R1:R0 formed the following states:
     *      00  - 4k
     *      01  - 16k
     *      10  - 64k
     *      11  - static RAM (??)
     *
     * If something less than 64k was set, the low RAM would be smaller and
     * mirror the other parts of the RAM
     *
     * TODO:  Find out what "static RAM" is
     * TODO:  This should affect _all_ memory accesses, not just video ram
     * TODO:  Verify that the CoCo 3 ignored this
     */

	/* update $0000-$7FFF */
	update_bank(0, 0x0000, 0x7FFF, m_sam_state & SAM_STATE_P1 ? 0x8000 : 0x0000);

	if (m_sam_state & SAM_STATE_TY)
	{
		update_bank(0, 0x8000, 0xFEFF, 0x8000);
	}
	else
	{
		update_bank(1, 0x8000, 0x9FFF, 0x0000);
		update_bank(2, 0xA000, 0xBFFF, 0x0000);
		update_bank(3, 0xC000, 0xFEFF, 0x0000);
	}
}



//-------------------------------------------------
//  install_memory
//-------------------------------------------------

void sam6883_device::install_memory(offs_t addrstart, offs_t addrend, void *memory, bool is_read_only)
{
	if (addrend >= addrstart)
	{
		if (memory == NULL)
			m_cpu_space->unmap_readwrite(addrstart, addrend);
		else if (is_read_only)
			m_cpu_space->install_rom(addrstart, addrend, memory);
		else
			m_cpu_space->install_ram(addrstart, addrend, memory);
	}
}



//-------------------------------------------------
//  update_bank
//-------------------------------------------------

void sam6883_device::update_bank(int bank, offs_t addrstart, offs_t addrend, offs_t offset)
{
	assert((bank >= 0) && (bank < sizeof(m_banks) / sizeof(m_banks[0])));

	if (m_banks[bank].m_memory != NULL)
	{
		/* this bank is a memory bank */
		install_memory(addrstart, MIN(addrend, addrstart + m_banks[bank].m_memory_size - 1), m_banks[bank].m_memory + m_banks[bank].m_memory_offset + offset, m_banks[bank].m_memory_read_only);
		install_memory(addrstart + m_banks[bank].m_memory_size, addrend, NULL, m_banks[bank].m_memory_read_only);
	}
	else
	{
		/* this bank uses handlers */
		assert(offset == 0);	/* changes to the offset are not supported */
		if (!m_banks[bank].m_rhandler.isnull())
			m_cpu_space->install_read_handler(addrstart, addrend, 0, 0, m_banks[bank].m_rhandler);
		if (!m_banks[bank].m_whandler.isnull())
			m_cpu_space->install_write_handler(addrstart, addrend, 0, 0, m_banks[bank].m_whandler);
	}
}



//-------------------------------------------------
//  update_cpu_clock - adjusts the speed of the CPU
//  clock
//-------------------------------------------------

void sam6883_friend_device::update_cpu_clock(void)
{
	/* The infamous speed up poke.
     *
     * This was a SAM switch that occupied 4 addresses:
     *
     *      $FFD9   (set)   R1
     *      $FFD8   (clear) R1
     *      $FFD7   (set)   R0
     *      $FFD6   (clear) R0
     *
     * R1:R0 formed the following states:
     *      00  - slow          0.89 MHz
     *      01  - dual speed    ???
     *      1x  - fast          1.78 MHz
     *
     * R1 controlled whether the video addressing was speeded up and R0
     * did the same for the CPU.  On pre-CoCo 3 machines, setting R1 caused
     * the screen to display garbage because the M6847 could not display
     * fast enough.
     *
     * TODO:  Make the overclock more accurate.  In dual speed, ROM was a fast
     * access but RAM was not.  I don't know how to simulate this.
     */
	int speed = (m_sam_state & (SAM_STATE_R1|SAM_STATE_R0)) / SAM_STATE_R0;

	/* the line below is weird because we are not strictly emulating the M6809E with emphasis on the 'E' */
	m_cpu->set_clock_scale(speed ? 2 : 1);
}



//-------------------------------------------------
//  set_bank_offset
//-------------------------------------------------

void sam6883_device::set_bank_offset(int bank, offs_t offset)
{
	if (m_banks[bank].m_memory_offset != offset)
	{
		m_banks[bank].m_memory_offset = offset;
		update_memory();
		update_bank(2, 0xFFE0, 0xFFFF, 0x1FE0);
	}
}



//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER( sam6883_device::read )
{
	return 0;
}



//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER( sam6883_device::write )
{
	/* alter the SAM state */
	UINT16 xorval = alter_sam_state(offset);

	/* based on the mask, apply effects */
	if (xorval & (SAM_STATE_TY|SAM_STATE_M1|SAM_STATE_M0|SAM_STATE_P1))
		update_memory();
	if (xorval & (SAM_STATE_R1|SAM_STATE_R0))
		update_cpu_clock();
}



//-------------------------------------------------
//  horizontal_sync
//-------------------------------------------------

void sam6883_device::horizontal_sync(void)
{
	bool carry;

	// When horizontal sync occurs, bits B1-B3 or B1-B4 may be cleared (except in DMA mode).  The catch
	// is that the SAM's counter is a chain of flip-flops.  Clearing the counter can cause carries to
	// occur just as they can when the counter is bumped.
	//
	// This is critical in getting certain semigraphics modes to work correctly.  Guardian uses this
	// mode (see bug #1153).  Special thanks to Ciaran Anscomb and Phill Harvey-Smith for figuring this
	// out
	switch((m_sam_state & (SAM_STATE_V2|SAM_STATE_V1|SAM_STATE_V0)) / SAM_STATE_V0)
	{
		case 0x01:
		case 0x03:
		case 0x05:
			/* these SAM modes clear bits B1-B3 */
			carry = (m_counter & 0x0008) ? true : false;
			m_counter &= ~0x000F;
			if (carry)
				counter_carry_bit3();
			break;

		case 0x00:
		case 0x02:
		case 0x04:
		case 0x06:
			/* clear bits B1-B4 */
			carry = (m_counter & 0x0010) ? true : false;
			m_counter &= ~0x001F;
			if (carry)
				counter_carry_bit4();
			break;

		case 0x07:
			/* DMA mode - do nothing */
			break;

		default:
			fatalerror("Should not get here");
			return;
	}
}



//-------------------------------------------------
//  hs_w
//-------------------------------------------------

WRITE_LINE_MEMBER( sam6883_device::hs_w )
{
	if (state)
	{
		horizontal_sync();
	}
}
