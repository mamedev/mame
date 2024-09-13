// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Paul Leaman
/***************************************************************************

    Cinemat/Leland driver

    Leland sound hardware
    driver by Aaron Giles and Paul Leaman

    -------------------------------------------------------------------

    1st generation sound hardware was controlled by the master Z80.
    It drove either an AY-8910/AY-8912 for music. It also had two DACs
    that were driven by the video refresh. At the end of each scanline
    there are 8-bit DAC samples that can be enabled via the output
    ports on the AY-8910. The DACs run at a fixed frequency of 15.3kHz,
    since they are clocked once each scanline.

    -------------------------------------------------------------------

    2nd generation sound hardware was used in Redline Racer. It
    consisted of an 80186 microcontroller driving 8 8-bit DACs. The
    frequency of the DACs were controlled by one of 3 Intel 8254
    programmable interval timers (PITs):

        DAC number  Clock source
        ----------  -----------------
            0       8254 PIT 1 output 0
            1       8254 PIT 1 output 1
            2       8254 PIT 1 output 2
            3       8254 PIT 2 output 0
            4       8254 PIT 2 output 1
            5-7     8254 PIT 3 output 0

    The clock outputs for each DAC can be read, and are polled to
    determine when data should be updated on the chips. The 80186's
    two DMA channels are generally used to drive the first two DACs,
    with the remaining 6 DACs being fed manually via polling.

    -------------------------------------------------------------------

    3rd generation sound hardware appeared in the football games
    (Quarterback, AAFB) and the later games up through Pigout. This
    variant is closely based on the Redline Racer sound system, but
    they took out two of the DACs and replaced them with a higher
    resolution (10-bit) DAC. The driving clocks have been rearranged
    a bit, and the number of PITs reduced from 3 to 2:

        DAC number  Clock source
        ----------  -----------------
            0       8254 PIT 1 output 0
            1       8254 PIT 1 output 1
            2       8254 PIT 1 output 2
            3       8254 PIT 2 output 0
            4       8254 PIT 2 output 1
            5       8254 PIT 2 output 2
            10-bit  80186 timer 0

    Like the 2nd generation board, the first two DACs are driven via
    the DMA channels, and the remaining 5 DACs are polled.

    -------------------------------------------------------------------

    4th generation sound hardware showed up in Ataxx, Indy Heat, and
    World Soccer Finals. For this variant, they removed one more PIT
    and 3 of the 8-bit DACs, and added a YM2151 music chip and an
    externally-fed 8-bit DAC.

        DAC number  Clock source
        ----------  -----------------
            0       8254 PIT 1 output 0
            1       8254 PIT 1 output 1
            2       8254 PIT 1 output 2
            10-bit  80186 timer 0
            ext     80186 timer 1

    The externally driven DACs have registers for a start/stop address
    and triggers to control the clocking.

***************************************************************************/

#include "emu.h"
#include "leland_a.h"

#include "cpu/z80/z80.h"
#include "speaker.h"

#define LOG_WARN   (1U << 1)
#define LOG_COMM   (1U << 2)
#define LOG_EXTERN (1U << 3)

#define VERBOSE    LOG_WARN
#include "logmacro.h"

/*************************************
 *
 *  2nd-4th generation sound
 *
 *************************************/

void leland_80186_sound_device::pit0_2_w(int state)
{
	set_clock_line(2, state);
}

void leland_80186_sound_device::pit1_0_w(int state)
{
	set_clock_line(3, state);
}

void leland_80186_sound_device::pit1_1_w(int state)
{
	set_clock_line(4, state);
}

void leland_80186_sound_device::pit1_2_w(int state)
{
	set_clock_line(5, state);
}

void leland_80186_sound_device::i80186_tmr0_w(int state)
{
	set_clock_line(6, state);
}

void leland_80186_sound_device::i80186_tmr1_w(int state)
{
	if (m_ext_base != nullptr)
	{
		if (state)
		{
			if (m_ext_active && (m_ext_start < m_ext_stop))
			{
				m_dac[3]->write(m_ext_base[m_ext_start]);
				m_ext_start++;
			}
		}
	}
	set_clock_line(3, state);
}

void leland_80186_sound_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_audiocpu, 16_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &leland_80186_sound_device::leland_80186_map_program);
	m_audiocpu->set_addrmap(AS_IO, &leland_80186_sound_device::leland_80186_map_io);
	m_audiocpu->chip_select_callback().set(FUNC(leland_80186_sound_device::peripheral_ctrl));
	m_audiocpu->tmrout0_handler().set(FUNC(leland_80186_sound_device::i80186_tmr0_w));

	SPEAKER(config, "speaker").front_center();
	for (int i = 0; i < 6; i++)
	{
		AD7524(config, m_dac[i], 0).add_route(ALL_OUTPUTS, "speaker", 0.2); // 74hc374.u31..6 + ad7524.u46..51
		DAC_8BIT_BINARY_WEIGHTED(config, m_dacvol[i], 0).set_output_range(0, 1); // 74hc374.u17..22 + rX2-rX9 (24k,12k,6.2k,3k,1.5k,750,360,160) where X is 0..5
		m_dacvol[i]->add_route(0, m_dac[i], 1.0, DAC_INPUT_RANGE_HI);
		m_dacvol[i]->add_route(0, m_dac[i], -1.0, DAC_INPUT_RANGE_LO);
	}
	AD7533(config, "dac9", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // ad7533.u64

	PIT8254(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(4000000);
	m_pit[0]->out_handler<0>().set(m_audiocpu, FUNC(i80186_cpu_device::drq0_w));
	m_pit[0]->set_clk<1>(4000000);
	m_pit[0]->out_handler<1>().set(m_audiocpu, FUNC(i80186_cpu_device::drq1_w));
	m_pit[0]->set_clk<2>(4000000);
	m_pit[0]->out_handler<2>().set(FUNC(leland_80186_sound_device::pit0_2_w));

	PIT8254(config, m_pit[1], 0);
	m_pit[1]->set_clk<0>(4000000);
	m_pit[1]->out_handler<0>().set(FUNC(leland_80186_sound_device::pit1_0_w));
	m_pit[1]->set_clk<1>(4000000);
	m_pit[1]->out_handler<1>().set(FUNC(leland_80186_sound_device::pit1_1_w));
	m_pit[1]->set_clk<2>(4000000);
	m_pit[1]->out_handler<2>().set(FUNC(leland_80186_sound_device::pit1_2_w));

	GENERIC_LATCH_16(config, m_soundlatch);
}

void redline_80186_sound_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_audiocpu, 16_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &redline_80186_sound_device::leland_80186_map_program);
	m_audiocpu->set_addrmap(AS_IO, &redline_80186_sound_device::redline_80186_map_io);
	m_audiocpu->chip_select_callback().set(FUNC(leland_80186_sound_device::peripheral_ctrl));

	SPEAKER(config, "speaker").front_center();
	for (int i = 0; i < 8; i++)
	{
		AD7524(config, m_dac[i], 0).add_route(ALL_OUTPUTS, "speaker", 0.2); // unknown DAC
		DAC_8BIT_BINARY_WEIGHTED(config, m_dacvol[i], 0).set_output_range(0, 1);
		m_dacvol[i]->add_route(0, m_dac[i], 1.0, DAC_INPUT_RANGE_HI);
		m_dacvol[i]->add_route(0, m_dac[i], -1.0, DAC_INPUT_RANGE_LO); // unknown DAC
	}

	PIT8254(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(7000000);
	m_pit[0]->out_handler<0>().set(m_audiocpu, FUNC(i80186_cpu_device::drq0_w));
	m_pit[0]->set_clk<1>(7000000);
	m_pit[0]->out_handler<1>().set(m_audiocpu, FUNC(i80186_cpu_device::drq1_w));
	m_pit[0]->set_clk<2>(7000000);
	m_pit[0]->out_handler<2>().set(FUNC(leland_80186_sound_device::pit0_2_w));

	PIT8254(config, m_pit[1], 0);
	m_pit[1]->set_clk<0>(7000000);
	m_pit[1]->out_handler<0>().set(FUNC(leland_80186_sound_device::pit1_0_w));
	m_pit[1]->set_clk<1>(7000000);
	m_pit[1]->out_handler<1>().set(FUNC(leland_80186_sound_device::pit1_1_w));
	m_pit[1]->set_clk<2>(7000000);

	PIT8254(config, m_pit[2], 0);
	m_pit[2]->set_clk<0>(7000000);
	m_pit[2]->out_handler<0>().set(FUNC(leland_80186_sound_device::pit1_2_w));
	m_pit[2]->set_clk<1>(7000000);
	m_pit[2]->set_clk<2>(7000000);

	GENERIC_LATCH_16(config, m_soundlatch);
}

void ataxx_80186_sound_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_audiocpu, 16_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ataxx_80186_sound_device::leland_80186_map_program);
	m_audiocpu->set_addrmap(AS_IO, &ataxx_80186_sound_device::ataxx_80186_map_io);
	m_audiocpu->chip_select_callback().set(FUNC(leland_80186_sound_device::peripheral_ctrl));
	m_audiocpu->tmrout0_handler().set(FUNC(leland_80186_sound_device::i80186_tmr0_w));

	SPEAKER(config, "speaker").front_center();
	for (int i = 0; i < 4; i++)
	{
		AD7524(config, m_dac[i], 0).add_route(ALL_OUTPUTS, "speaker", 0.2); // unknown DAC
		DAC_8BIT_BINARY_WEIGHTED(config, m_dacvol[i], 0).set_output_range(0, 1); // unknown DAC
		m_dacvol[i]->add_route(0, m_dac[i], 1.0, DAC_INPUT_RANGE_HI);
		m_dacvol[i]->add_route(0, m_dac[i], -1.0, DAC_INPUT_RANGE_LO);
	}
	AD7533(config, "dac9", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC

	PIT8254(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(4000000);
	m_pit[0]->out_handler<0>().set(m_audiocpu, FUNC(i80186_cpu_device::drq0_w));
	m_pit[0]->set_clk<1>(4000000);
	m_pit[0]->out_handler<1>().set(m_audiocpu, FUNC(i80186_cpu_device::drq1_w));
	m_pit[0]->set_clk<2>(4000000);
	m_pit[0]->out_handler<2>().set(FUNC(leland_80186_sound_device::pit0_2_w));

	GENERIC_LATCH_16(config, m_soundlatch);
}

void wsf_80186_sound_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_audiocpu, 16_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &wsf_80186_sound_device::leland_80186_map_program);
	m_audiocpu->set_addrmap(AS_IO, &wsf_80186_sound_device::ataxx_80186_map_io);
	m_audiocpu->chip_select_callback().set(FUNC(leland_80186_sound_device::peripheral_ctrl));
	m_audiocpu->tmrout0_handler().set(FUNC(leland_80186_sound_device::i80186_tmr0_w));
	m_audiocpu->tmrout1_handler().set(FUNC(leland_80186_sound_device::i80186_tmr1_w));

	SPEAKER(config, "speaker").front_center();
	for (int i = 0; i < 4; i++)
	{
		AD7524(config, m_dac[i], 0).add_route(ALL_OUTPUTS, "speaker", 0.2); // unknown DAC
		DAC_8BIT_BINARY_WEIGHTED(config, m_dacvol[i], 0).set_output_range(0, 1); // unknown DAC
		m_dacvol[i]->add_route(0, m_dac[i], 1.0, DAC_INPUT_RANGE_HI);
		m_dacvol[i]->add_route(0, m_dac[i], -1.0, DAC_INPUT_RANGE_LO); // unknown DAC
	}
	AD7533(config, "dac9", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC

	/* sound hardware */
	YM2151(config, m_ymsnd, 4000000);
	m_ymsnd->add_route(0, "speaker", 0.40);
	m_ymsnd->add_route(1, "speaker", 0.40);

	PIT8254(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(4000000);
	m_pit[0]->out_handler<0>().set(m_audiocpu, FUNC(i80186_cpu_device::drq0_w));
	m_pit[0]->set_clk<1>(4000000);
	m_pit[0]->out_handler<1>().set(m_audiocpu, FUNC(i80186_cpu_device::drq1_w));
	m_pit[0]->set_clk<2>(4000000);
	m_pit[0]->out_handler<2>().set(FUNC(leland_80186_sound_device::pit0_2_w));

	GENERIC_LATCH_16(config, m_soundlatch);
}


void leland_80186_sound_device::leland_80186_map_program(address_map &map)
{
	map(0x00000, 0x03fff).mirror(0x1c000).ram();
	map(0x20000, 0xfffff).rom().region("audiocpu", 0x20000);
}

void leland_80186_sound_device::ataxx_80186_map_io(address_map &map)
{
}

void redline_80186_sound_device::redline_80186_map_io(address_map &map)
{
	map(0x0000, 0xffff).w(FUNC(redline_80186_sound_device::redline_dac_w));
}


void leland_80186_sound_device::leland_80186_map_io(address_map &map)
{
	map(0x0000, 0xffff).w(FUNC(leland_80186_sound_device::dac_w));
}

/*************************************
 *
 *  Sound initialization
 *
 *************************************/

void leland_80186_sound_device::device_start()
{
	// register for savestates
	save_item(NAME(m_peripheral));
	save_item(NAME(m_last_control));
	save_item(NAME(m_clock_active));
	save_item(NAME(m_clock_tick));
	save_item(NAME(m_sound_command));
	save_item(NAME(m_sound_response));
	save_item(NAME(m_response_sync));
	save_item(NAME(m_ext_start));
	save_item(NAME(m_ext_stop));
	save_item(NAME(m_ext_active));

	// zerofill
	m_peripheral = 0;
	m_last_control = 0;
	m_clock_active = 0;
	m_clock_tick = 0;
	m_sound_command = 0;
	m_sound_response = 0;
	m_response_sync = false;
	m_ext_start = 0;
	m_ext_stop = 0;
	m_ext_active = 0;
}

void leland_80186_sound_device::device_reset()
{
	m_last_control = 0xf8;
	m_clock_active = 0;
	m_clock_tick = 0;
	m_response_sync = false;
	m_ext_start = 0;
	m_ext_stop = 0;
	m_ext_active = 0;

	if (m_type == TYPE_WSF)
		m_dacvol[3]->write(0xff);  //TODO: determine how to set this if at all
}

DEFINE_DEVICE_TYPE(LELAND_80186, leland_80186_sound_device, "leland_80186_sound", "80186 DAC (Leland)")

leland_80186_sound_device::leland_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: leland_80186_sound_device(mconfig, LELAND_80186, tag, owner, clock)
{
	m_type = TYPE_LELAND;
}

leland_80186_sound_device::leland_80186_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_soundlatch(*this, "soundlatch")
	, m_dac(*this, "dac%u", 1U)
	, m_dac9(*this, "dac9")
	, m_dacvol(*this, "dac%uvol", 1U)
	, m_pit(*this, "pit%u", 0U)
	, m_audiocpu(*this, "audiocpu")
	, m_ymsnd(*this, "ymsnd")
	, m_master(*this, finder_base::DUMMY_TAG)
	, m_ext_base(*this, "ext")
{
}

DEFINE_DEVICE_TYPE(REDLINE_80186, redline_80186_sound_device, "redline_80186_sound", "80186 DAC (Redline Racer)")

redline_80186_sound_device::redline_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: leland_80186_sound_device(mconfig, REDLINE_80186, tag, owner, clock)
{
	m_type = TYPE_REDLINE;
}

DEFINE_DEVICE_TYPE(ATAXX_80186, ataxx_80186_sound_device, "ataxx_80186_sound", "80186 DAC (Ataxx)")

ataxx_80186_sound_device::ataxx_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: leland_80186_sound_device(mconfig, ATAXX_80186, tag, owner, clock)
{
	m_type = TYPE_ATAXX;
}

DEFINE_DEVICE_TYPE(WSF_80186, wsf_80186_sound_device, "wsf_80186_sound", "80186 DAC (WSF)")

wsf_80186_sound_device::wsf_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: leland_80186_sound_device(mconfig, WSF_80186, tag, owner, clock)
{
	m_type = TYPE_WSF;
}

void leland_80186_sound_device::peripheral_ctrl(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 2:
			m_peripheral = data;
			break;

		case 4:
		{
			u32 temp = (m_peripheral & 0xffc0) << 4;
			if (data & 0x0040)
			{
				m_audiocpu->space(AS_PROGRAM).install_readwrite_handler(temp, temp + 0x2ff, read16s_delegate(*this, FUNC(leland_80186_sound_device::peripheral_r)), write16s_delegate(*this, FUNC(leland_80186_sound_device::peripheral_w)));
			}
			else
			{
				temp &= 0xffff;
				m_audiocpu->space(AS_IO).install_readwrite_handler(temp, temp + 0x2ff, read16s_delegate(*this, FUNC(leland_80186_sound_device::peripheral_r)), write16s_delegate(*this, FUNC(leland_80186_sound_device::peripheral_w)));
			}
			break;
		}

		default:
			break;
	}
}



/*************************************
 *
 *  External 80186 control
 *
 *************************************/

void leland_80186_sound_device::leland_80186_control_w(u8 data)
{
	/* see if anything changed */
	int diff = (m_last_control ^ data) & 0xf8;
	if (diff == 0)
		return;
	m_last_control = data;

	LOGMASKED(LOG_COMM, "%s:80186 control = %02X%s%s%s%s%s\n",
			machine().describe_context(), data,
			(data & 0x80) ? "" : "  /RESET",
			(data & 0x40) ? "" : "  ZNMI",
			(data & 0x20) ? "" : "  INT0",
			(data & 0x10) ? "" : "  /TEST",
			(data & 0x08) ? "" : "  INT1");

	/* /RESET */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_TEST, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	/* /NMI */
	/*  If the master CPU doesn't get a response by the time it's ready to send
	    the next command, it uses an NMI to force the issue; unfortunately, this
	    seems to really screw up the sound system. It turns out it's better to
	    just wait for the original interrupt to occur naturally */
	//m_audiocpu->set_input_line(INPUT_LINE_NMI, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

	/* INT0 */
	m_audiocpu->int0_w(data & 0x20);
	/* INT1 */
	m_audiocpu->int1_w(data & 0x08);
	/* handle reset here */
	if ((diff & 0x80) && (data & 0x80))
		reset();
}



/*************************************
 *
 *  Sound command handling
 *
 *************************************/

void leland_80186_sound_device::command_lo_w(u8 data)
{
	LOGMASKED(LOG_COMM, "%s:Write sound command latch lo = %02X\n", machine().describe_context(), data);
	m_sound_command = (m_sound_command & 0xff00) | data;
	m_soundlatch->write(m_sound_command);
}


void leland_80186_sound_device::command_hi_w(u8 data)
{
	LOGMASKED(LOG_COMM, "%s:Write sound command latch hi = %02X\n", machine().describe_context(), data);
	m_sound_command = (m_sound_command & 0x00ff) | (data << 8);
	m_soundlatch->write(m_sound_command);
}



/*************************************
 *
 *  Sound response handling
 *
 *************************************/

u8 leland_80186_sound_device::response_r()
{
	if (!machine().side_effects_disabled())
	{
		/* This is pretty cheesy, but necessary. Since the CPUs run in round-robin order,
		   synchronizing on the write to this register from the slave side does nothing.
		   The usual trick with briefly setting perfect quantum on master CPU side write
		   is also ineffective. In order to make sure the master CPU gets the real response,
		   we force a synchronize on the read like this. */
		if (!m_response_sync)
		{
			machine().scheduler().synchronize();
			m_master->defer_access();
		}
		else
		{
			LOGMASKED(LOG_COMM, "%s:Read sound response latch = %02X\n", machine().describe_context(), m_sound_response);
		}

		m_response_sync = !m_response_sync;
	}

	return m_sound_response;
}



/*************************************
 *
 *  Low-level DAC I/O
 *
 *************************************/

void leland_80186_sound_device::dac_w(offs_t offset, u16 data, u16 mem_mask)
{
	int dac = offset & 7;

	/* handle value changes */
	if (ACCESSING_BITS_0_7)
	{
		if ((offset & 0x60) == 0x40)
			m_audiocpu->drq0_w(CLEAR_LINE);
		else if ((offset & 0x60) == 0x60)
			m_audiocpu->drq1_w(CLEAR_LINE);

		m_dac[dac]->write(data & 0xff);

		set_clock_line(dac, 0);
	}

	/* handle volume changes */
	if (ACCESSING_BITS_8_15)
	{
		m_dacvol[dac]->write(data >> 8);
	}
}


void redline_80186_sound_device::redline_dac_w(offs_t offset, u16 data)
{
	data = (data & 0xff) | (offset << 8);
	offset = ((offset >> 8) & 7) | ((offset & 0x2000) ? 0x40 : 0) | ((offset & 0x800) ? 0x20 : 0);
	dac_w(offset, data, 0xffff);
}

void leland_80186_sound_device::ataxx_dac_control(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* handle common offsets */
		switch (offset & 0x1f)
		{
		case 0x00:
			dac_w(0x40, data, 0x00ff);
			return;
		case 0x01:
			dac_w(0x61, data, 0x00ff);
			return;
		case 0x02:
			dac_w(2, data, 0x00ff);
			return;
		case 0x03:
			m_dacvol[0]->write((data & 7) << 5);
			m_dacvol[1]->write(((data >> 3) & 7) << 5);
			m_dacvol[2]->write(((data >> 6) & 3) << 6);
			return;
		}
	}

	/* if we have a YM2151 (and an external DAC), handle those offsets */
	switch (m_type)
	{
	case TYPE_WSF:
		switch (offset)
		{
		case 0x04:
			m_ext_active = 1;
			LOGMASKED(LOG_EXTERN, "External DAC active\n");
			return;
		case 0x05:
			m_ext_active = 0;
			LOGMASKED(LOG_EXTERN, "External DAC inactive\n");
			return;
		case 0x06:
			m_ext_start >>= 4;
			COMBINE_DATA(&m_ext_start);
			m_ext_start <<= 4;
			LOGMASKED(LOG_EXTERN, "External DAC start = %05X\n", m_ext_start);
			return;
		case 0x07:
			m_ext_stop >>= 4;
			COMBINE_DATA(&m_ext_stop);
			m_ext_stop <<= 4;
			LOGMASKED(LOG_EXTERN, "External DAC stop = %05X\n", m_ext_stop);
			return;
		}
		break;
	}

	LOGMASKED(LOG_WARN, "%s:Unexpected peripheral write %d/%02X = %02X\n", machine().describe_context(), 5, offset, data);
}



/*************************************
 *
 *  Peripheral chip dispatcher
 *
 *************************************/

u16 leland_80186_sound_device::peripheral_r(offs_t offset, u16 mem_mask)
{
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 0:
			/* we have to return 0 periodically so that they handle interrupts */
			//if ((++m_clock_tick & 7) == 0)
			//  return 0;

			/* if we've filled up all the active channels, we can give this CPU a rest */
			/* until the next interrupt */
			if (m_type != TYPE_REDLINE)
				return ((m_clock_active >> 1) & 0x3e);
			else
				return ((m_clock_active << 1) & 0x7e);

		case 1:
			LOGMASKED(LOG_COMM, "%s:Read sound command latch = %02X\n", machine().describe_context(), m_soundlatch->read());
			return m_soundlatch->read();

		case 2:
			if (ACCESSING_BITS_0_7)
				return m_pit[0]->read(offset & 3);
			break;

		case 3:
			if (m_type <= TYPE_REDLINE)
			{
				if (ACCESSING_BITS_0_7)
					return m_pit[1]->read(offset & 3);
			}
			else if (m_type == TYPE_WSF)
				return m_ymsnd->read(offset);
			break;

		case 4:
			if (m_type == TYPE_REDLINE)
			{
				if (ACCESSING_BITS_0_7)
					return m_pit[2]->read(offset & 3);
			}
			else
				LOGMASKED(LOG_WARN, "%s:Unexpected peripheral read %d/%02X\n", machine().describe_context(), select, offset*2);
			break;

		default:
			LOGMASKED(LOG_WARN, "%s:Unexpected peripheral read %d/%02X\n", machine().describe_context(), select, offset*2);
			break;
	}
	return 0xffff;
}


void leland_80186_sound_device::peripheral_w(offs_t offset, u16 data, u16 mem_mask)
{
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 1:
			LOGMASKED(LOG_COMM, "%s:Write sound response latch = %02X\n", machine().describe_context(), data);
			m_sound_response = data;
			break;

		case 2:
			if (ACCESSING_BITS_0_7)
				m_pit[0]->write(offset & 3, data);
			break;

		case 3:
			if (m_type <= TYPE_REDLINE)
			{
				if (ACCESSING_BITS_0_7)
					m_pit[1]->write(offset & 3, data);
			}
			else if(m_type == TYPE_WSF)
				m_ymsnd->write(offset, data);
			break;

		case 4:
			if (m_type == TYPE_REDLINE)
			{
				if (ACCESSING_BITS_0_7)
					m_pit[2]->write(offset & 3, data);
			}
			else if (mem_mask == 0xffff)
			{
				m_dac9->write(data);
				set_clock_line(6, 0);
			}
			break;

		case 5: /* Ataxx/WSF/Indy Heat only */
			if (m_type > TYPE_REDLINE)
				ataxx_dac_control(offset, data, mem_mask);
			break;

		default:
			LOGMASKED(LOG_WARN, "%s:Unexpected peripheral write %d/%02X = %02X\n", machine().describe_context(), select, offset, data);
			break;
	}
}



/*************************************
 *
 *  Game-specific handlers
 *
 *************************************/

void leland_80186_sound_device::ataxx_80186_control_w(u8 data)
{
	/* compute the bit-shuffled variants of the bits and then write them */
	int modified =  ((data & 0x01) << 7) |
					((data & 0x02) << 5) |
					((data & 0x04) << 3) |
					((data & 0x08) << 1);
	leland_80186_control_w(modified);
}
