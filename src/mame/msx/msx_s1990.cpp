// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_s1990.h"


#define LOG_CPU (1U << 1)
#define LOG_PCM (1U << 2)
//#define VERBOSE (LOG_CPU | LOG_PCM)
#include "logmacro.h"

/***************************************************************************

    ASCII S1990 MSX-Engine

TODO:
- Injection of wait states when accessing VDP when Z80 is active.
- Injection of wait states when accessing VDP when R800 is active.
- Injection of wait state to align 7MHz R800 bus with 3.5MHz MSX bus.
- Unknown what the other internal registers do

Internal registers:
- 0x00
- 0x01
- 0x02
- 0x03
- 0x04
- 0x05 - Read only?
  Read
    7------- Unknown
    -6------ 1 - Firmware enabled
    --543210 Unknown
- 0x06 - R/W - CPU Control
  Write
    76------ Unknown
    --5----- 0 - R800 active, 1 - Z80 active
    ---43210 Unknown
- 0x07
- 0x08
- 0x09
- 0x0a
- 0x0b
- 0x0c
- 0x0d
- 0x0e - Byte before last byte written - read only?
- 0x0f - Last byte written - read only?

***************************************************************************/


DEFINE_DEVICE_TYPE(MSX_S1990, msx_s1990_device, "msx_s1990", "MSX-Engine S1990")


msx_s1990_device::msx_s1990_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_S1990, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_firmware_switch_cb(*this, 0)
	, m_pause_led_cb(*this)
	, m_r800_led_cb(*this)
	, m_dac_write_cb(*this)
	, m_sample_hold_cb(*this)
	, m_select_cb(*this)
	, m_filter_cb(*this)
	, m_muting_cb(*this)
	, m_comp_cb(*this, 0)
	, m_z80(*this, finder_base::DUMMY_TAG)
	, m_r800(*this, finder_base::DUMMY_TAG)
	, m_reg_index(0)
	, m_last_counter_reset(attotime::zero)
	, m_last_pcm_write_ticks(0)
	, m_pcm_control(0)
	, m_pcm_data(0x80)
	, m_dac_timer(nullptr)
	, m_z80_halt_enabled(false)
{
}


device_memory_interface::space_config_vector msx_s1990_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}


void msx_s1990_device::device_start()
{
	space(AS_PROGRAM).specific(m_data);
	space(AS_IO).specific(m_io);

	save_item(NAME(m_regs));
	save_item(NAME(m_reg_index));
	save_item(NAME(m_last_counter_reset));
	save_item(NAME(m_last_pcm_write_ticks));
	save_item(NAME(m_pcm_control));
	save_item(NAME(m_pcm_data));

	m_dac_timer = timer_alloc(FUNC(msx_s1990_device::dac_write), this);
}


void msx_s1990_device::device_reset()
{
	m_regs[6] = 0x20; // Z80 active
	m_last_counter_reset = attotime::zero;
	m_last_pcm_write_ticks = 0;
	m_z80->resume(SUSPEND_REASON_HALT);
	m_r800->suspend(SUSPEND_REASON_HALT, true);
}


INPUT_CHANGED_MEMBER(msx_s1990_device::pause_callback)
{
	if (m_z80_halt_enabled && BIT(m_regs[0x06], 5))
		m_z80->set_input_line(Z80_INPUT_LINE_WAIT, BIT(newval, 0) ? ASSERT_LINE : CLEAR_LINE);
}


void msx_s1990_device::pause_w(u8 data)
{
	m_pause_led_cb(BIT(data, 0));
	m_z80_halt_enabled = BIT(data, 1);
	m_r800_led_cb(BIT(data, 7));
}


void msx_s1990_device::reg_index_write(u8 data)
{
	m_reg_index = data;
}


u8 msx_s1990_device::regs_read()
{
	if ((m_reg_index & 0x0f) == 0x05)
		return (m_firmware_switch_cb()) ? 0x40 : 0x00;
	return m_regs[m_reg_index & 0x0f];
}


void msx_s1990_device::regs_write(u8 data)
{
	if ((m_reg_index & 0x0f) == 0x06)
	{
		if (BIT(data, 5))
		{
			LOGMASKED(LOG_CPU, "Enable Z80, disable R800\n");
			m_z80->resume(SUSPEND_REASON_HALT);
			m_r800->suspend(SUSPEND_REASON_HALT, true);
		}
		else
		{
			LOGMASKED(LOG_CPU, "Disable Z80, enable R800\n");
			m_z80->suspend(SUSPEND_REASON_HALT, true);
			m_r800->resume(SUSPEND_REASON_HALT);
		}
	}

	m_regs[m_reg_index & 0x0f] = data;
}


void msx_s1990_device::mem_write(offs_t offset, u8 data)
{
	// TODO Clock syncing when in R800 mode
	m_regs[0x0e] = m_regs[0x0f];
	m_regs[0x0f] = data;
	m_data.write_byte(offset, data);
}


u8 msx_s1990_device::mem_read(offs_t offset)
{
	// TODO Clock syncing when in R800 mode
	return m_data.read_byte(offset);
}


void msx_s1990_device::io_write(offs_t offset, u8 data)
{
	// TODO Clock syncing/extra wait cycles
	// TODO Injection of wait states when accessing VDP
	m_io.write_byte(offset, data);
}


u8 msx_s1990_device::io_read(offs_t offset)
{
	// TODO Clock syncing/extra wait cycles
	return m_io.read_byte(offset);
}


void msx_s1990_device::counter_write(u8 data)
{
	// TODO: Round to last counter tick?
	m_last_counter_reset = machine().time();
}


u8 msx_s1990_device::counter_read(offs_t offset)
{
	u64 counter = attotime_to_clocks(machine().time() - m_last_counter_reset) / 28;
	return (counter >> (8 * BIT(offset, 0))) & 0xff;
}


TIMER_CALLBACK_MEMBER(msx_s1990_device::dac_write)
{
	m_dac_write_cb(m_pcm_data);
}


void msx_s1990_device::pmdac(u8 data)
{
	m_pcm_data = data;
	if (!BIT(m_pcm_control, 1))
	{
		// Round to last counter tick and schedule next dac write?
		m_last_pcm_write_ticks = machine().time().as_ticks(PCM_FREQUENCY);
		m_dac_timer->adjust(attotime::from_ticks(m_last_pcm_write_ticks + 1, PCM_FREQUENCY));
	}
}


u8 msx_s1990_device::pmcnt()
{
	return (machine().time().as_ticks(PCM_FREQUENCY) - m_last_pcm_write_ticks) & 0x03;
}


void msx_s1990_device::pmcntl(u8 data)
{
	// 7------- 0
	// -6------ 0
	// --5----- 0
	// ---4---- SMPL
	// ----3--- SEL
	// -----2-- FILT
	// ------1- MUTE
	// -------0 ADDA ??

	LOGMASKED(LOG_PCM, "pmcntl: %02x\n", data);
	m_pcm_control = data & 0x1f;
	m_sample_hold_cb(BIT(m_pcm_control, 4));
	m_select_cb(BIT(m_pcm_control, 3));
	m_filter_cb(BIT(m_pcm_control, 2));
	m_muting_cb(BIT(m_pcm_control, 1));
}


u8 msx_s1990_device::pmstat()
{
	// 7------- COMP
	// -6------ 0
	// --5----- 0
	// ---4---- SMPL
	// ----3--- SEL
	// -----2-- FILT
	// ------1- MUTE
	// -------0 BUFF ??

	return (m_pcm_control & 0x1f) | (m_comp_cb() ? 0x80 : 0x00);
}
