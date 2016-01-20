// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarigen.c

    General functions for Atari games.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/2151intf.h"
#include "sound/2413intf.h"
#include "sound/tms5220.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "video/atarimo.h"
#include "atarigen.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SOUND_TIMER_RATE            attotime::from_usec(5)
#define SOUND_TIMER_BOOST           attotime::from_usec(1000)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline const atarigen_screen_timer *get_screen_timer(screen_device &screen)
{
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();
	int i;

	// find the index of the timer that matches the screen
	for (i = 0; i < ARRAY_LENGTH(state->m_screen_timer); i++)
		if (state->m_screen_timer[i].screen == &screen)
			return &state->m_screen_timer[i];

	fatalerror("Unexpected: no atarivc_eof_update_timer for screen '%s'\n", screen.tag().c_str());
	return nullptr;
}



//**************************************************************************
//  SOUND COMMUNICATIONS DEVICE
//**************************************************************************

// device type definition
const device_type ATARI_SOUND_COMM = &device_creator<atari_sound_comm_device>;

//-------------------------------------------------
//  atari_sound_comm_device - constructor
//-------------------------------------------------

atari_sound_comm_device::atari_sound_comm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARI_SOUND_COMM, "Atari Sound Communications", tag, owner, clock, "atarscom", __FILE__),
		m_main_int_cb(*this),
		m_sound_cpu(nullptr),
		m_main_to_sound_ready(false),
		m_sound_to_main_ready(false),
		m_main_to_sound_data(0),
		m_sound_to_main_data(0),
		m_timed_int(0),
		m_ym2151_int(0)
{
}


//-------------------------------------------------
//  static_set_sound_cpu: Set the tag of the
//  sound CPU
//-------------------------------------------------

void atari_sound_comm_device::static_set_sound_cpu(device_t &device, const char *cputag)
{
	downcast<atari_sound_comm_device &>(device).m_sound_cpu_tag = cputag;
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_sound_comm_device::device_start()
{
	// find the sound CPU
	if (m_sound_cpu_tag == nullptr)
		throw emu_fatalerror("No sound CPU specified!");
	m_sound_cpu = siblingdevice<m6502_device>(m_sound_cpu_tag);
	if (m_sound_cpu == nullptr)
		throw emu_fatalerror("Sound CPU '%s' not found!", m_sound_cpu_tag);

	// resolve callbacks
	m_main_int_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_main_to_sound_ready));
	save_item(NAME(m_sound_to_main_ready));
	save_item(NAME(m_main_to_sound_data));
	save_item(NAME(m_sound_to_main_data));
	save_item(NAME(m_timed_int));
	save_item(NAME(m_ym2151_int));
}


//-------------------------------------------------
//  device_reset: Handle a device reset by
//  clearing the interrupt lines and states
//-------------------------------------------------

void atari_sound_comm_device::device_reset()
{
	// reset the internal interrupts states
	m_timed_int = m_ym2151_int = 0;

	// reset the sound I/O states
	m_main_to_sound_data = m_sound_to_main_data = 0;
	m_main_to_sound_ready = m_sound_to_main_ready = false;
}


//-------------------------------------------------
//  device_timer: Handle device-specific timer
//  calbacks
//-------------------------------------------------

void atari_sound_comm_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SOUND_RESET:
			delayed_sound_reset(param);
			break;

		case TID_SOUND_WRITE:
			delayed_sound_write(param);
			break;

		case TID_6502_WRITE:
			delayed_6502_write(param);
			break;
	}
}


//-------------------------------------------------
//  sound_irq_gen: Generates an IRQ signal to the
//  6502 sound processor.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atari_sound_comm_device::sound_irq_gen)
{
	m_timed_int = 1;
	update_sound_irq();
}


//-------------------------------------------------
//  sound_irq_ack_r: Resets the IRQ signal to the
//  6502 sound processor. Both reads and writes
//  can be used.
//-------------------------------------------------

READ8_MEMBER(atari_sound_comm_device::sound_irq_ack_r)
{
	m_timed_int = 0;
	update_sound_irq();
	return 0;
}

WRITE8_MEMBER(atari_sound_comm_device::sound_irq_ack_w)
{
	m_timed_int = 0;
	update_sound_irq();
}


//-------------------------------------------------
//  atarigen_ym2151_irq_gen: Sets the state of the
//  YM2151's IRQ line.
//-------------------------------------------------

WRITE_LINE_MEMBER(atari_sound_comm_device::ym2151_irq_gen)
{
	m_ym2151_int = state;
	update_sound_irq();
}


//-------------------------------------------------
//  sound_reset_w: Write handler which resets the
//  sound CPU in response.
//-------------------------------------------------

WRITE16_MEMBER(atari_sound_comm_device::sound_reset_w)
{
	synchronize(TID_SOUND_RESET);
}


//-------------------------------------------------
//  main_command_w: Handles communication from the main CPU
//  to the sound CPU. Two versions are provided, one with the
//  data byte in the low 8 bits, and one with the data byte in
//  the upper 8 bits.
//-------------------------------------------------

WRITE8_MEMBER(atari_sound_comm_device::main_command_w)
{
	synchronize(TID_SOUND_WRITE, data);
}


//-------------------------------------------------
//  main_response_r: Handles reading data communicated from the
//  sound CPU to the main CPU. Two versions are provided, one
//  with the data byte in the low 8 bits, and one with the data
//  byte in the upper 8 bits.
//-------------------------------------------------

READ8_MEMBER(atari_sound_comm_device::main_response_r)
{
	m_sound_to_main_ready = false;
	m_main_int_cb(CLEAR_LINE);
	return m_sound_to_main_data;
}


//-------------------------------------------------
//  sound_response_w: Handles communication from the
//  sound CPU to the main CPU.
//-------------------------------------------------

WRITE8_MEMBER(atari_sound_comm_device::sound_response_w)
{
	synchronize(TID_6502_WRITE, data);
}


//-------------------------------------------------
//  sound_command_r: Handles reading data
//  communicated from the main CPU to the sound
//  CPU.
//-------------------------------------------------

READ8_MEMBER(atari_sound_comm_device::sound_command_r)
{
	m_main_to_sound_ready = false;
	m_sound_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_main_to_sound_data;
}


//-------------------------------------------------
//  update_sound_irq: Called whenever the IRQ state
//  changes. An interrupt is generated if either
//  sound_irq_gen() was called, or if the YM2151
//  generated an interrupt via the
//  ym2151_irq_gen() callback.
//-------------------------------------------------

void atari_sound_comm_device::update_sound_irq()
{
	if (m_timed_int || m_ym2151_int)
		m_sound_cpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	else
		m_sound_cpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}


//-------------------------------------------------
//  delayed_sound_reset: Synchronizes the sound
//  reset command between the two CPUs.
//-------------------------------------------------

void atari_sound_comm_device::delayed_sound_reset(int param)
{
	// unhalt and reset the sound CPU
	if (param == 0)
	{
		m_sound_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_sound_cpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}

	// reset the sound write state
	m_sound_to_main_ready = false;
	m_main_int_cb(CLEAR_LINE);

	// allocate a high frequency timer until a response is generated
	// the main CPU is *very* sensistive to the timing of the response
	machine().scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


//-------------------------------------------------
//  delayed_sound_write: Synchronizes a data write
//  from the main CPU to the sound CPU.
//-------------------------------------------------

void atari_sound_comm_device::delayed_sound_write(int data)
{
	// warn if we missed something
	if (m_main_to_sound_ready)
		logerror("Missed command from 68010\n");

	// set up the states and signal an NMI to the sound CPU
	m_main_to_sound_data = data;
	m_main_to_sound_ready = true;
	m_sound_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// allocate a high frequency timer until a response is generated
	// the main CPU is *very* sensistive to the timing of the response
	machine().scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


//-------------------------------------------------
//  delayed_6502_write: Synchronizes a data write
//  from the sound CPU to the main CPU.
//-------------------------------------------------

void atari_sound_comm_device::delayed_6502_write(int data)
{
	// warn if we missed something
	if (m_sound_to_main_ready)
		logerror("Missed result from 6502\n");

	// set up the states and signal the sound interrupt to the main CPU
	m_sound_to_main_data = data;
	m_sound_to_main_ready = true;
	m_main_int_cb(ASSERT_LINE);
}



//**************************************************************************
//  VAD VIDEO CONTROLLER DEVICE
//**************************************************************************

// device type definition
const device_type ATARI_VAD = &device_creator<atari_vad_device>;

//-------------------------------------------------
//  atari_vad_device - constructor
//-------------------------------------------------

atari_vad_device::atari_vad_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARI_VAD, "Atari VAD", tag, owner, clock, "atarivad", __FILE__),
		device_video_interface(mconfig, *this),
		m_scanline_int_cb(*this),
		m_alpha_tilemap(*this, "alpha"),
		m_playfield_tilemap(*this, "playfield"),
		m_playfield2_tilemap(*this, "playfield2"),
		m_mob(*this, "mob"),
		m_eof_data(*this, "eof"),
		m_scanline_int_timer(nullptr),
		m_tilerow_update_timer(nullptr),
		m_eof_timer(nullptr),
		m_palette_bank(0),
		m_pf0_xscroll_raw(0),
		m_pf0_yscroll(0),
		m_pf1_xscroll_raw(0),
		m_pf1_yscroll(0),
		m_mo_xscroll(0),
		m_mo_yscroll(0)
{
}


//-------------------------------------------------
//  control_write: Does the bulk of the word for an I/O
//  write.
//-------------------------------------------------

WRITE16_MEMBER(atari_vad_device::control_write)
{
	UINT16 newword = m_control[offset];
	COMBINE_DATA(&newword);
	internal_control_write(offset, newword);
}


//-------------------------------------------------
//  control_read: Handles an I/O read from the video controller.
//-------------------------------------------------

READ16_MEMBER(atari_vad_device::control_read)
{
	logerror("vc_r(%02X)\n", offset);

	// a read from offset 0 returns the current scanline
	// also sets bit 0x4000 if we're in VBLANK
	if (offset == 0)
	{
		int result = m_screen->vpos();
		if (result > 255)
			result = 255;
		if (result > m_screen->visible_area().max_y)
			result |= 0x4000;
		return result;
	}
	else
		return m_control[offset];
}


//-------------------------------------------------
//  alpha_w: Generic write handler for alpha RAM.
//-------------------------------------------------

WRITE16_MEMBER(atari_vad_device::alpha_w)
{
	m_alpha_tilemap->write(space, offset, data, mem_mask);
}


//-------------------------------------------------
//  playfield_upper_w: Generic write handler for
//  upper word of split playfield RAM.
//-------------------------------------------------

WRITE16_MEMBER(atari_vad_device::playfield_upper_w)
{
	m_playfield_tilemap->write_ext(space, offset, data, mem_mask);
	if (m_playfield2_tilemap != nullptr)
		m_playfield2_tilemap->write_ext(space, offset, data, mem_mask);
}


//-------------------------------------------------
//  playfield_latched_lsb_w: Generic write handler for
//  lower word of playfield RAM with a latch in the LSB of the
//  upper word.
//-------------------------------------------------

WRITE16_MEMBER(atari_vad_device::playfield_latched_lsb_w)
{
	m_playfield_tilemap->write(space, offset, data, mem_mask);
	if ((m_control[0x0a] & 0x80) != 0)
		m_playfield_tilemap->write_ext(space, offset, m_control[0x1d], UINT16(0x00ff));
}


//-------------------------------------------------
//  playfield_latched_msb_w: Generic write handler for
//  lower word of playfield RAM with a latch in the MSB of the
//  upper word.
//-------------------------------------------------

WRITE16_MEMBER(atari_vad_device::playfield_latched_msb_w)
{
	m_playfield_tilemap->write(space, offset, data, mem_mask);
	if ((m_control[0x0a] & 0x80) != 0)
		m_playfield_tilemap->write_ext(space, offset, m_control[0x1c], UINT16(0xff00));
}


//-------------------------------------------------
//  playfield2_latched_msb_w: Generic write handler for
//  lower word of second playfield RAM with a latch in the MSB
//  of the upper word.
//-------------------------------------------------

WRITE16_MEMBER(atari_vad_device::playfield2_latched_msb_w)
{
	m_playfield2_tilemap->write(space, offset, data, mem_mask);
	if ((m_control[0x0a] & 0x80) != 0)
		m_playfield2_tilemap->write_ext(space, offset, m_control[0x1c], UINT16(0xff00));
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_vad_device::device_start()
{
	// verify configuration
	if (m_playfield_tilemap == nullptr)
		throw emu_fatalerror("Playfield tilemap not found!");
	if (m_eof_data == nullptr)
		throw emu_fatalerror("EOF data not found!");

	// resolve callbacks
	m_scanline_int_cb.resolve_safe();

	// allocate timers
	m_scanline_int_timer = timer_alloc(TID_SCANLINE_INT);
	m_tilerow_update_timer = timer_alloc(TID_TILEROW_UPDATE);
	m_eof_timer = timer_alloc(TID_EOF);

	// register for save states
	save_item(NAME(m_palette_bank));          // which palette bank is enabled
	save_item(NAME(m_pf0_xscroll_raw));       // playfield 1 xscroll raw value
	save_item(NAME(m_pf0_yscroll));           // playfield 1 yscroll
	save_item(NAME(m_pf1_xscroll_raw));       // playfield 2 xscroll raw value
	save_item(NAME(m_pf1_yscroll));           // playfield 2 yscroll
	save_item(NAME(m_mo_xscroll));            // sprite xscroll
	save_item(NAME(m_mo_yscroll));            // sprite xscroll
}


//-------------------------------------------------
//  device_reset: Handle a device reset by
//  clearing the interrupt lines and states
//-------------------------------------------------

void atari_vad_device::device_reset()
{
	// share extended memory between the two tilemaps
	if (m_playfield2_tilemap != nullptr)
		m_playfield2_tilemap->extmem().set(m_playfield_tilemap->extmem());

	// reset the state
	m_palette_bank = 0;
	m_pf0_xscroll_raw = m_pf1_xscroll_raw = 0;
	m_pf0_yscroll = m_pf1_yscroll = 0;
	m_mo_xscroll = m_mo_yscroll = 0;
	memset(m_control, 0, sizeof(m_control));

	// start the timers
	m_tilerow_update_timer->adjust(m_screen->time_until_pos(0));
	m_eof_timer->adjust(m_screen->time_until_pos(0));
}


//-------------------------------------------------
//  device_timer: Handle device-specific timer
//  calbacks
//-------------------------------------------------

void atari_vad_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SCANLINE_INT:
			m_scanline_int_cb(ASSERT_LINE);
			break;

		case TID_TILEROW_UPDATE:
			update_tilerow(timer, param);
			break;

		case TID_EOF:
			eof_update(timer);
			break;
	}
}


//-------------------------------------------------
//  internal_control_write: Handle writes to the
//  control registers and EOF updates
//-------------------------------------------------

void atari_vad_device::internal_control_write(offs_t offset, UINT16 newword)
{
	// switch off the offset
	UINT16 oldword = m_control[offset];
	m_control[offset] = newword;
	switch (offset)
	{
		//
		//  VAD register map:
		//
		//      00 = HDW_ENABLE
		//              A000 = enable other VAD params
		//              E000 = enable other VAD params but disable update from ALPHA RAM
		//      01 = HDW_VSY_LD (standard = 0x795)
		//              0FF8 = V_SY_S
		//              0007 = V_SY_E
		//      02 = HDW_VBL_LD (standard = 0xAEF)
		//              FE00 = VLAST
		//              01FF = V_BL_S
		//      03 = HDW_VINT
		//      04 = HDW_HSY_LD (standard = 0x5EEF)
		//              FC00 = H_SY_E/2
		//              03FF = H_SY_S
		//      05 = HDW_HBL_LD (relief = 0x72AC, batman/thunderj = 0xBEB1, shuuz/offtwall = 0xCEB1)
		//              F000 = LB_CLR
		//              0800 = H_BL_E/16
		//              0400 = H_BL_E/1
		//              03FF = H_BL_S
		//      06 = HDW_SLIP_LD (relief/batman/shuuz = 0x6F05, shuuz/offtwall = 0x0F05)
		//              7F00 = SLIP_S
		//              0080 = SLIP_E_SE1
		//              007F = SLIP_E
		//      07 = HDW_APDMA_LD (relief/batman/shuuz = 0xBE84, shuuz = 0xB90E, offtwall = 0xB90D)
		//              C000 = OPFPIC
		//              3800 = PF_HRESET/16
		//              0780 = PFAT
		//              0070 = AL_HRESET (depending on ALHSIZ)
		//              000F = ALPHA_DMA
		//      08 = HDW_PMBASE_LD (relief/batman/shuuz = 0x0519, shuuz/offtwall = 0x49B4)
		//              E000 = P1BASE/0x2000
		//              1C00 = P2BASE/0x2000
		//              0380 = PABASE/0x2000
		//              0040 = PMASK
		//              003E = MOBASE/0x800
		//              0001 = MOMASK
		//      09 = HDW_ALBASE_LD (relief/batman/shuuz = 0x061F, shuuz/offtwall = 0x017F)
		//              0400 = OPMINUS1
		//              03C0 = ALBASE/0x1000
		//              003F = SLIPBASE/0x80
		//      0A = HDW_OPT_LD (relief/batman/shuuz = 0x4410, shuuz = 0x0250, offtwall = 0x0A10)
		//              8000 = OPFHSIZ - subtract 1 from PF2 attribute horizontal stamp address
		//              4000 = OPIUM - playfield 2 enable
		//              2000 = OLS_EN - linescroll enable
		//              1000 = OSLPINK - split links enable
		//              0800 = OSLORAM - slow MO DMA cycles
		//              0400 = O_DCDMA - enable double time color RAM DMA cycles
		//              0200 = O_DVDMA - enable double time video RAM DMA cycles
		//              0100 = OADROEN - tristate address bus
		//              0080 = OAS_EN - enable autostore of playfield attributes
		//              0040 = OCRDWE - enable 8-bit color RAM
		//              0020 = OVRDTACK - wait for VRAM DMA on writes
		//              0010 = OCRDTACK - wait for CRAM DMA on writes
		//              0008 = OALHSIZ - alpha horiz stamp size (0=8)
		//              0004 = OPFHSIZ - PF horiz stamp size (0=8)
		//              0002 = OPFVSIZ - PF vertical stamp size (0=8)
		//              0001 = OMOVSIZ - MO vertical stamp size (0=8)
		//      10 = HDW_MOCON (MOB chip MOB code)
		//      11 = HDW_PCON (MOB chip PF code)
		//      12 = HDW_GRCON (MOB chip GR code)
		//      13 = MO_hscroll (9)
		//      14 = PF1_hscroll (A)
		//      15 = PF2_hscroll (B)
		//      16 = MO_vscroll (D)
		//      17 = PF1_vscroll (E)
		//      18 = PF2_vscroll (F)
		//
		//  3efffe = reset to wrong configuration?
		//

		case 0:
//if (oldword != newword) printf("Word 0 = %04X\n", newword);
			break;

		// set the scanline interrupt here
		case 0x03:
			if (oldword != newword || !m_scanline_int_timer->enabled())
				m_scanline_int_timer->adjust(m_screen->time_until_pos(newword & 0x1ff));
			break;

		// latch enable
		case 0x0a:
			// check for palette banking
			if (m_palette_bank != (((newword & 0x0400) >> 10) ^ 1))
			{
				m_screen->update_partial(m_screen->vpos());
				m_palette_bank = ((newword & 0x0400) >> 10) ^ 1;
			}
//if ((oldword & ~0x0080) != (newword & ~0x0080)) printf("Latch control = %04X\n", newword);
			break;

		// indexed parameters
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			update_parameter(newword);
			break;

		// scanline IRQ ack here
		case 0x1e:
			m_scanline_int_cb(CLEAR_LINE);
			break;

		// log anything else
		default:
			if (oldword != newword)
				logerror("vc_w(%02X, %04X) ** [prev=%04X]\n", offset, newword, oldword);
			break;
	}
}


//-------------------------------------------------
//  update_pf_xscrolls: Update the playfield
//  scroll values.
//-------------------------------------------------

inline void atari_vad_device::update_pf_xscrolls()
{
	m_playfield_tilemap->set_scrollx(0, m_pf0_xscroll_raw + ((m_pf1_xscroll_raw) & 7));
	if (m_playfield2_tilemap != nullptr)
		m_playfield2_tilemap->set_scrollx(0, m_pf1_xscroll_raw + 4);
}


//-------------------------------------------------
//  update_parameter: Update parameters, shared
//  between end-of-frame, tilerow updates, and
//  direct control writes.
//-------------------------------------------------

void atari_vad_device::update_parameter(UINT16 newword)
{
	switch (newword & 15)
	{
		case 9:
			m_mo_xscroll = (newword >> 7) & 0x1ff;
			if (m_mob != nullptr)
				m_mob->set_xscroll(m_mo_xscroll);
			break;

		case 10:
			m_pf1_xscroll_raw = (newword >> 7) & 0x1ff;
			update_pf_xscrolls();
			break;

		case 11:
			m_pf0_xscroll_raw = (newword >> 7) & 0x1ff;
			update_pf_xscrolls();
			break;

		case 13:
			m_mo_yscroll = (newword >> 7) & 0x1ff;
			if (m_mob != nullptr)
				m_mob->set_yscroll(m_mo_yscroll);
			break;

		case 14:
			m_pf1_yscroll = (newword >> 7) & 0x1ff;
			if (m_playfield2_tilemap != nullptr)
				m_playfield2_tilemap->set_scrolly(0, m_pf1_yscroll);
			break;

		case 15:
			m_pf0_yscroll = (newword >> 7) & 0x1ff;
			m_playfield_tilemap->set_scrolly(0, m_pf0_yscroll);
			break;
	}
}


//-------------------------------------------------
//  update_tilerow: Fetch parameters stored at
//  the end of the current tilerow, which affect
//  rowscrolling.
//-------------------------------------------------

void atari_vad_device::update_tilerow(emu_timer &timer, int scanline)
{
	// skip if out of bounds, or not enabled
	if (scanline <= m_screen->visible_area().max_y && (m_control[0x0a] & 0x2000) != 0 && m_alpha_tilemap != nullptr)
	{
		// iterate over non-visible alpha tiles in this row
		int offset = scanline / 8 * 64 + 48 + 2 * (scanline % 8);
		int data0 = m_alpha_tilemap->basemem_read(offset++);
		int data1 = m_alpha_tilemap->basemem_read(offset++);

		// force an update if we have data
		if (scanline > 0 && ((data0 | data1) & 15) != 0)
			m_screen->update_partial(scanline - 1);

		// write the data
		if ((data0 & 15) != 0)
			update_parameter(data0);
		if ((data1 & 15) != 0)
			update_parameter(data1);
	}

	// update the timer to go off at the start of the next row
	scanline += ((m_control[0x0a] & 0x2000) != 0) ? 1 : 8;
	if (scanline >= m_screen->height())
		scanline = 0;
	timer.adjust(m_screen->time_until_pos(scanline), scanline);
}


//-------------------------------------------------
//  eof_update: Callback that slurps up data and
//  feeds it into the video controller registers
//  every refresh.
//-------------------------------------------------

void atari_vad_device::eof_update(emu_timer &timer)
{
	// echo all the commands to the video controller
	for (int i = 0; i < 0x1c; i++)
		if (m_eof_data[i] != 0)
			internal_control_write(i, m_eof_data[i]);

	// update the scroll positions
/*  atarimo_set_xscroll(0, m_mo_xscroll);
    atarimo_set_yscroll(0, m_mo_yscroll);

    update_pf_xscrolls();

    m_playfield_tilemap->set_scrolly(0, m_pf0_yscroll);
    if (m_playfield2_tilemap != NULL)
        m_playfield2_tilemap->set_scrolly(0, m_pf1_yscroll);*/
	timer.adjust(m_screen->time_until_pos(0));

	// use this for debugging the video controller values
#if 0
	if (machine().input().code_pressed(KEYCODE_8))
	{
		static FILE *out;
		if (!out) out = fopen("scroll.log", "w");
		if (out)
		{
			for (i = 0; i < 64; i++)
				fprintf(out, "%04X ", data[i]);
			fprintf(out, "\n");
		}
	}
#endif
}



//**************************************************************************
//  EEPROM INTERFACE DEVICE
//**************************************************************************

// device type definition
const device_type ATARI_EEPROM_2804 = &device_creator<atari_eeprom_2804_device>;
const device_type ATARI_EEPROM_2816 = &device_creator<atari_eeprom_2816_device>;

//-------------------------------------------------
//  atari_eeprom_device - constructor
//-------------------------------------------------

atari_eeprom_device::atari_eeprom_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source)
	: device_t(mconfig, devtype, name, tag, owner, 0, shortname, source),
		m_eeprom(*this, "eeprom"),
		m_unlocked(false)
{
}


//-------------------------------------------------
//  unlock_read/unlock_write - unlock read/write
//  handlers
//-------------------------------------------------

READ8_MEMBER(atari_eeprom_device::unlock_read) { m_unlocked = true; return space.unmap(); }
WRITE8_MEMBER(atari_eeprom_device::unlock_write) { m_unlocked = true; }
READ16_MEMBER(atari_eeprom_device::unlock_read) { m_unlocked = true; return space.unmap(); }
WRITE16_MEMBER(atari_eeprom_device::unlock_write) { m_unlocked = true; }
READ32_MEMBER(atari_eeprom_device::unlock_read) { m_unlocked = true; return space.unmap(); }
WRITE32_MEMBER(atari_eeprom_device::unlock_write) { m_unlocked = true; }


//-------------------------------------------------
//  read/write - data read/write handlers
//-------------------------------------------------

READ8_MEMBER(atari_eeprom_device::read)
{
	return m_eeprom->read(space, offset);
}

WRITE8_MEMBER(atari_eeprom_device::write)
{
	if (m_unlocked)
		m_eeprom->write(space, offset, data, mem_mask);
	else
		logerror("%s: Attemptedt to write to EEPROM while not unlocked\n", machine().describe_context());
	m_unlocked = false;
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_eeprom_device::device_start()
{
	// register for save states
	save_item(NAME(m_unlocked));
}


//-------------------------------------------------
//  device_reset: Handle a device reset by
//  clearing the interrupt lines and states
//-------------------------------------------------

void atari_eeprom_device::device_reset()
{
	// reset unlocked state
	m_unlocked = false;
}


//-------------------------------------------------
//  atari_eeprom_2804_device - constructor
//-------------------------------------------------

atari_eeprom_2804_device::atari_eeprom_2804_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: atari_eeprom_device(mconfig, ATARI_EEPROM_2804, "Atari EEPROM Interface (2804)", tag, owner, "atari2804", __FILE__)
{
}


//-------------------------------------------------
//  device_mconfig_additions - return machine
//  config fragment
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT(atari_eeprom_2804_config)
	MCFG_EEPROM_2804_ADD("eeprom")
MACHINE_CONFIG_END

machine_config_constructor atari_eeprom_2804_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(atari_eeprom_2804_config);
}


//-------------------------------------------------
//  atari_eeprom_2816_device - constructor
//-------------------------------------------------

atari_eeprom_2816_device::atari_eeprom_2816_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: atari_eeprom_device(mconfig, ATARI_EEPROM_2816, "Atari EEPROM Interface (2816)", tag, owner, "atari2816", __FILE__)
{
}


//-------------------------------------------------
//  device_mconfig_additions - return machine
//  config fragment
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT(atari_eeprom_2816_config)
	MCFG_EEPROM_2816_ADD("eeprom")
MACHINE_CONFIG_END

machine_config_constructor atari_eeprom_2816_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(atari_eeprom_2816_config);
}



/***************************************************************************
    OVERALL INIT
***************************************************************************/

atarigen_state::atarigen_state(const machine_config &mconfig, device_type type, std::string tag)
	: driver_device(mconfig, type, tag),
		m_earom(*this, "earom"),
		m_earom_data(0),
		m_earom_control(0),
		m_scanline_int_state(0),
		m_sound_int_state(0),
		m_video_int_state(0),
		m_xscroll(*this, "xscroll"),
		m_yscroll(*this, "yscroll"),
		m_slapstic_num(0),
		m_slapstic(nullptr),
		m_slapstic_bank(0),
		m_slapstic_last_pc(0),
		m_slapstic_last_address(0),
		m_slapstic_base(0),
		m_slapstic_mirror(0),
		m_scanlines_per_callback(0),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_soundcomm(*this, "soundcomm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_slapstic_device(*this, ":slapstic")
{
}

void atarigen_state::machine_start()
{
	screen_device *screen;
	int i;

	// allocate timers for all screens
	screen_device_iterator iter(*this);
	assert(iter.count() <= ARRAY_LENGTH(m_screen_timer));
	for (i = 0, screen = iter.first(); screen != nullptr; i++, screen = iter.next())
	{
		m_screen_timer[i].screen = screen;
		m_screen_timer[i].scanline_interrupt_timer = timer_alloc(TID_SCANLINE_INTERRUPT, (void *)screen);
		m_screen_timer[i].scanline_timer = timer_alloc(TID_SCANLINE_TIMER, (void *)screen);
	}

	save_item(NAME(m_scanline_int_state));
	save_item(NAME(m_sound_int_state));
	save_item(NAME(m_video_int_state));

	save_item(NAME(m_slapstic_num));
	save_item(NAME(m_slapstic_bank));
	save_item(NAME(m_slapstic_last_pc));
	save_item(NAME(m_slapstic_last_address));

	save_item(NAME(m_scanlines_per_callback));

	save_item(NAME(m_earom_data));
	save_item(NAME(m_earom_control));
}


void atarigen_state::machine_reset()
{
	// reset the interrupt states
	m_video_int_state = m_sound_int_state = m_scanline_int_state = 0;

	// reset the control latch on the EAROM, if present
	if (m_earom != nullptr)
		m_earom->set_control(0, 1, 1, 0, 0);

	// reset the slapstic
	if (m_slapstic_num != 0)
	{
		if (!m_slapstic_device)
			fatalerror("Slapstic device is missing?\n");

		m_slapstic_device->slapstic_reset();
		slapstic_update_bank(m_slapstic_device->slapstic_bank());
	}
}


void atarigen_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SCANLINE_INTERRUPT:
		{
			scanline_int_gen(*m_maincpu);
			screen_device *screen = reinterpret_cast<screen_device *>(ptr);
			timer.adjust(screen->frame_period());
			break;
		}

		case TID_SCANLINE_TIMER:
			scanline_timer(timer, *reinterpret_cast<screen_device *>(ptr), param);
			break;

		// unhalt the CPU that was passed as a pointer
		case TID_UNHALT_CPU:
			reinterpret_cast<device_t *>(ptr)->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			break;
	}
}


void atarigen_state::scanline_update(screen_device &screen, int scanline)
{
}


/***************************************************************************
    INTERRUPT HANDLING
***************************************************************************/

//-------------------------------------------------
//  scanline_int_set: Sets the scanline when the next
//  scanline interrupt should be generated.
//-------------------------------------------------

void atarigen_state::scanline_int_set(screen_device &screen, int scanline)
{
	get_screen_timer(screen)->scanline_interrupt_timer->adjust(screen.time_until_pos(scanline));
}


//-------------------------------------------------
//  sound_int_write_line: Standard write line
//  callback for the scanline interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER(atarigen_state::scanline_int_write_line)
{
	m_scanline_int_state = state;
	update_interrupts();
}


//-------------------------------------------------
//  scanline_int_gen: Standard interrupt routine
//  which sets the scanline interrupt state.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atarigen_state::scanline_int_gen)
{
	m_scanline_int_state = 1;
	update_interrupts();
}


//-------------------------------------------------
//  scanline_int_ack_w: Resets the state of the
//  scanline interrupt.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::scanline_int_ack_w)
{
	m_scanline_int_state = 0;
	update_interrupts();
}


//-------------------------------------------------
//  sound_int_write_line: Standard write line
//  callback for the sound interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER(atarigen_state::sound_int_write_line)
{
	m_sound_int_state = state;
	update_interrupts();
}


//-------------------------------------------------
//  sound_int_gen: Standard interrupt routine which
//  sets the sound interrupt state.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atarigen_state::sound_int_gen)
{
	m_sound_int_state = 1;
	update_interrupts();
}


//-------------------------------------------------
//  sound_int_ack_w: Resets the state of the sound
//  interrupt.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::sound_int_ack_w)
{
	m_sound_int_state = 0;
	update_interrupts();
}


//-------------------------------------------------
//  video_int_gen: Standard interrupt routine which
//  sets the video interrupt state.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atarigen_state::video_int_gen)
{
	m_video_int_state = 1;
	update_interrupts();
}


//-------------------------------------------------
//  video_int_ack_w: Resets the state of the video
//  interrupt.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::video_int_ack_w)
{
	m_video_int_state = 0;
	update_interrupts();
}



/***************************************************************************
    SLAPSTIC HANDLING
***************************************************************************/

inline void atarigen_state::slapstic_update_bank(int bank)
{
	// if the bank has changed, copy the memory; Pit Fighter needs this
	if (bank != m_slapstic_bank)
	{
		// bank 0 comes from the copy we made earlier
		if (bank == 0)
			memcpy(m_slapstic, &m_slapstic_bank0[0], 0x2000);
		else
			memcpy(m_slapstic, &m_slapstic[bank * 0x1000], 0x2000);

		// remember the current bank
		m_slapstic_bank = bank;
	}
}


void atarigen_state::device_post_load()
{
	if (m_slapstic_num != 0)
	{
		if (!m_slapstic_device)
		fatalerror("Slapstic device is missing?\n");

		slapstic_update_bank(m_slapstic_device->slapstic_bank());
	}
}


DIRECT_UPDATE_MEMBER(atarigen_state::slapstic_setdirect)
{
	// if we jump to an address in the slapstic region, tweak the slapstic
	// at that address and return ~0; this will cause us to be called on
	// subsequent fetches as well
	address &= ~m_slapstic_mirror;
	if (address >= m_slapstic_base && address < m_slapstic_base + 0x8000)
	{
		offs_t pc = direct.space().device().safe_pcbase();
		if (pc != m_slapstic_last_pc || address != m_slapstic_last_address)
		{
			m_slapstic_last_pc = pc;
			m_slapstic_last_address = address;
			slapstic_r(direct.space(), (address >> 1) & 0x3fff, 0xffff);
		}
		return ~0;
	}
	return address;
}



//-------------------------------------------------
//  slapstic_configure: Installs memory handlers for the
//  slapstic and sets the chip number.
//-------------------------------------------------

void atarigen_state::slapstic_configure(cpu_device &device, offs_t base, offs_t mirror, int chipnum)
{
	// reset in case we have no state
	m_slapstic_num = chipnum;
	m_slapstic = nullptr;

	// if we have a chip, install it
	if (chipnum != 0)
	{
		if (!m_slapstic_device)
			fatalerror("Slapstic device is missing\n");

		// initialize the slapstic
		m_slapstic_device->slapstic_init(machine(), chipnum);

		// install the memory handlers
		address_space &program = device.space(AS_PROGRAM);
		m_slapstic = program.install_readwrite_handler(base, base + 0x7fff, 0, mirror, read16_delegate(FUNC(atarigen_state::slapstic_r), this), write16_delegate(FUNC(atarigen_state::slapstic_w), this));
		program.set_direct_update_handler(direct_update_delegate(FUNC(atarigen_state::slapstic_setdirect), this));

		// allocate memory for a copy of bank 0
		m_slapstic_bank0.resize(0x2000);
		memcpy(&m_slapstic_bank0[0], m_slapstic, 0x2000);

		// ensure we recopy memory for the bank
		m_slapstic_bank = 0xff;

		// install an opcode base handler if we are a 68000 or variant
		m_slapstic_base = base;
		m_slapstic_mirror = mirror;
	}
}


//-------------------------------------------------
//  slapstic_w: Assuming that the slapstic sits in
//  ROM memory space, we just simply tweak the slapstic at this
//  address and do nothing more.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::slapstic_w)
{
	if (!m_slapstic_device)
		fatalerror("Slapstic device is missing?\n");

	slapstic_update_bank(m_slapstic_device->slapstic_tweak(space, offset));
}


//-------------------------------------------------
//  slapstic_r: Tweaks the slapstic at the appropriate
//  address and then reads a word from the underlying memory.
//-------------------------------------------------

READ16_MEMBER(atarigen_state::slapstic_r)
{
	if (!m_slapstic_device)
		fatalerror("Slapstic device is missing?\n");

	// fetch the result from the current bank first
	int result = m_slapstic[offset & 0xfff];

	// then determine the new one
	slapstic_update_bank(m_slapstic_device->slapstic_tweak(space, offset));
	return result;
}



/***************************************************************************
    SOUND HELPERS
***************************************************************************/

//-------------------------------------------------
//  set_volume_by_type: Scans for a particular
//  sound chip and changes the volume on all
//  channels associated with it.
//-------------------------------------------------

void atarigen_state::set_volume_by_type(int volume, device_type type)
{
	sound_interface_iterator iter(*this);
	for (device_sound_interface *sound = iter.first(); sound != nullptr; sound = iter.next())
		if (sound->device().type() == type)
			sound->set_output_gain(ALL_OUTPUTS, volume / 100.0);
}


//-------------------------------------------------
//  set_XXXXX_volume: Sets the volume for a given
//  type of chip.
//-------------------------------------------------

void atarigen_state::set_ym2151_volume(int volume)
{
	set_volume_by_type(volume, YM2151);
}

void atarigen_state::set_ym2413_volume(int volume)
{
	set_volume_by_type(volume, YM2413);
}

void atarigen_state::set_pokey_volume(int volume)
{
	set_volume_by_type(volume, POKEY);
}

void atarigen_state::set_tms5220_volume(int volume)
{
	set_volume_by_type(volume, TMS5220);
}

void atarigen_state::set_oki6295_volume(int volume)
{
	set_volume_by_type(volume, OKIM6295);
}



/***************************************************************************
    SCANLINE TIMING
***************************************************************************/

//-------------------------------------------------
//  scanline_timer_reset: Sets up the scanline timer.
//-------------------------------------------------

void atarigen_state::scanline_timer_reset(screen_device &screen, int frequency)
{
	// set the scanline callback
	m_scanlines_per_callback = frequency;

	// set a timer to go off at scanline 0
	if (frequency != 0)
		get_screen_timer(screen)->scanline_timer->adjust(screen.time_until_pos(0));
}


//-------------------------------------------------
//  scanline_timer: Called once every n scanlines
//  to generate the periodic callback to the main
//  system.
//-------------------------------------------------

void atarigen_state::scanline_timer(emu_timer &timer, screen_device &screen, int scanline)
{
	// callback
	scanline_update(screen, scanline);

	// generate another
	scanline += m_scanlines_per_callback;
	if (scanline >= screen.height())
		scanline = 0;
	timer.adjust(screen.time_until_pos(scanline), scanline);
}




/***************************************************************************
    VIDEO HELPERS
***************************************************************************/

//-------------------------------------------------
//  halt_until_hblank_0: Halts CPU 0 until the
//  next HBLANK.
//-------------------------------------------------

void atarigen_state::halt_until_hblank_0(device_t &device, screen_device &screen)
{
	// halt the CPU until the next HBLANK
	int hpos = screen.hpos();
	int width = screen.width();
	int hblank = width * 9 / 10;

	// if we're in hblank, set up for the next one
	if (hpos >= hblank)
		hblank += width;

	// halt and set a timer to wake up
	device.execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	timer_set(screen.scan_period() * (hblank - hpos) / width, TID_UNHALT_CPU, 0, (void *)&device);
}


/***************************************************************************
    MISC HELPERS
***************************************************************************/

//-------------------------------------------------
//  blend_gfx: Takes two GFXElements and blends their
//  data together to form one. Then frees the second.
//-------------------------------------------------

void atarigen_state::blend_gfx(int gfx0, int gfx1, int mask0, int mask1)
{
	gfx_element *gx0 = m_gfxdecode->gfx(gfx0);
	gfx_element *gx1 = m_gfxdecode->gfx(gfx1);
	UINT8 *srcdata, *dest;
	int c, x, y;

	// allocate memory for the assembled data
	srcdata = auto_alloc_array(machine(), UINT8, gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	dest = srcdata;
	for (c = 0; c < gx0->elements(); c++)
	{
		const UINT8 *c0base = gx0->get_data(c);
		const UINT8 *c1base = gx1->get_data(c);

		// loop over height
		for (y = 0; y < gx0->height(); y++)
		{
			const UINT8 *c0 = c0base;
			const UINT8 *c1 = c1base;

			for (x = 0; x < gx0->width(); x++)
				*dest++ = (*c0++ & mask0) | (*c1++ & mask1);
			c0base += gx0->rowbytes();
			c1base += gx1->rowbytes();
		}
	}

//  int newdepth = gx0->depth() * gx1->depth();
	int granularity = gx0->granularity();
	gx0->set_raw_layout(srcdata, gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(granularity);

	// free the second graphics element
	m_gfxdecode->set_gfx(gfx1, nullptr);
}



//**************************************************************************
//  VECTOR AND EARLY RASTER EAROM INTERFACE
//**************************************************************************

READ8_MEMBER( atarigen_state::earom_r )
{
	// return data latched from previous clock
	return m_earom->data();
}


WRITE8_MEMBER( atarigen_state::earom_w )
{
	// remember the value written
	m_earom_data = data;

	// output latch only enabled if control bit 2 is set
	if (m_earom_control & 4)
		m_earom->set_data(m_earom_data);

	// always latch the address
	m_earom->set_address(offset);
}


WRITE8_MEMBER( atarigen_state::earom_control_w )
{
	// remember the control state
	m_earom_control = data;

	// ensure ouput data is put on data lines prior to updating controls
	if (m_earom_control & 4)
		m_earom->set_data(m_earom_data);

	// set the control lines; /CS2 is always held low
	m_earom->set_control(data & 8, 1, ~data & 4, data & 2, data & 1);
}
