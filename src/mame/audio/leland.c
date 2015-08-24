// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cinemat/Leland driver

    Leland sound hardware
    driver by Aaron Giles and Paul Leaman

    -------------------------------------------------------------------

    1st generation sound hardware was controlled by the master Z80.
    It drove an AY-8910/AY-8912 pair for music. It also had two DACs
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
#include "cpu/z80/z80.h"
#include "includes/leland.h"

#define LOG_COMM 0
#define LOG_EXTERN 0

/*************************************
 *
 *  2nd-4th generation sound
 *
 *************************************/

WRITE_LINE_MEMBER(leland_80186_sound_device::pit0_2_w)
{
	set_clock_line(2, state);
}

WRITE_LINE_MEMBER(leland_80186_sound_device::pit1_0_w)
{
	set_clock_line(3, state);
}

WRITE_LINE_MEMBER(leland_80186_sound_device::pit1_1_w)
{
	set_clock_line(4, state);
}

WRITE_LINE_MEMBER(leland_80186_sound_device::pit1_2_w)
{
	set_clock_line(5, state);
}

WRITE_LINE_MEMBER(leland_80186_sound_device::pit2_0_w)
{
	set_clock_line(5, state);
}

WRITE_LINE_MEMBER(leland_80186_sound_device::i80186_tmr0_w)
{
	set_clock_line(6, state);
}

WRITE_LINE_MEMBER(leland_80186_sound_device::i80186_tmr1_w)
{
	if (state)
	{
		if (m_ext_active && (m_ext_start < m_ext_stop))
		{
			m_dac4->write_signed8(m_ext_base[m_ext_start]);
			m_ext_start++;
		}
	}
	set_clock_line(7, state);
}

static MACHINE_CONFIG_FRAGMENT( leland_80186_sound )
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac3", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac4", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac5", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac6", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac7", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.00)

	MCFG_DEVICE_ADD("pit0", PIT8254, 0)
	MCFG_PIT8253_CLK0(4000000)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq0_w))
	MCFG_PIT8253_CLK1(4000000)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq1_w))
	MCFG_PIT8253_CLK2(4000000)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(leland_80186_sound_device, pit0_2_w))

	MCFG_DEVICE_ADD("pit1", PIT8254, 0)
	MCFG_PIT8253_CLK0(4000000)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(leland_80186_sound_device, pit1_0_w))
	MCFG_PIT8253_CLK1(4000000)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(leland_80186_sound_device, pit1_1_w))
	MCFG_PIT8253_CLK2(4000000)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(leland_80186_sound_device, pit1_2_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( redline_80186_sound )
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac3", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac4", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac5", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac6", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac7", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac8", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)

	MCFG_DEVICE_ADD("pit0", PIT8254, 0)
	MCFG_PIT8253_CLK0(7000000)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq0_w))
	MCFG_PIT8253_CLK1(7000000)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq1_w))
	MCFG_PIT8253_CLK2(7000000)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(leland_80186_sound_device, pit0_2_w))

	MCFG_DEVICE_ADD("pit1", PIT8254, 0)
	MCFG_PIT8253_CLK0(7000000)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(leland_80186_sound_device, pit1_0_w))
	MCFG_PIT8253_CLK1(7000000)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(leland_80186_sound_device, pit1_1_w))
	MCFG_PIT8253_CLK2(7000000)

	MCFG_DEVICE_ADD("pit2", PIT8254, 0)
	MCFG_PIT8253_CLK0(7000000)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(leland_80186_sound_device, pit1_2_w))
	MCFG_PIT8253_CLK1(7000000)
	MCFG_PIT8253_CLK2(7000000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( ataxx_80186_sound )
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac3", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac4", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac7", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.00)

	MCFG_DEVICE_ADD("pit0", PIT8254, 0)
	MCFG_PIT8253_CLK0(4000000)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq0_w))
	MCFG_PIT8253_CLK1(4000000)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq1_w))
	MCFG_PIT8253_CLK2(4000000)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(leland_80186_sound_device, pit0_2_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( wsf_80186_sound )
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac3", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac4", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.40)
	MCFG_SOUND_ADD("dac7", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 1.00)

	/* sound hardware */
	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_SOUND_ROUTE(0, "speaker", 0.40)
	MCFG_SOUND_ROUTE(1, "speaker", 0.40)

	MCFG_DEVICE_ADD("pit0", PIT8254, 0)
	MCFG_PIT8253_CLK0(4000000)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq0_w))
	MCFG_PIT8253_CLK1(4000000)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE(":audiocpu", i80186_cpu_device, drq1_w))
	MCFG_PIT8253_CLK2(4000000)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(leland_80186_sound_device, pit0_2_w))
MACHINE_CONFIG_END

machine_config_constructor leland_80186_sound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( leland_80186_sound );
}

machine_config_constructor redline_80186_sound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( redline_80186_sound );
}

machine_config_constructor ataxx_80186_sound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ataxx_80186_sound );
}

machine_config_constructor wsf_80186_sound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wsf_80186_sound );
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
	m_ext_start = 0;
	m_ext_stop = 0;
	m_ext_active = 0;
	m_ext_base = NULL;

	m_audiocpu = downcast<i80186_cpu_device *>(machine().device("audiocpu"));

	/* determine which sound hardware is installed */
	if (m_type == TYPE_WSF)
		m_ext_base = machine().root_device().memregion("dac")->base();
}

void leland_80186_sound_device::device_reset()
{
	m_last_control = 0xf8;
	m_clock_active = 0;
	m_clock_tick = 0;
	m_ext_start = 0;
	m_ext_stop = 0;
	m_ext_active = 0;
}

const device_type LELAND_80186 = &device_creator<leland_80186_sound_device>;

leland_80186_sound_device::leland_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LELAND_80186, "80186 DAC (Leland)", tag, owner, clock, "leland_80186_sound", __FILE__),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_dac3(*this, "dac3"),
		m_dac4(*this, "dac4"),
		m_dac5(*this, "dac5"),
		m_dac6(*this, "dac6"),
		m_dac7(*this, "dac7"),
		m_dac8(*this, "dac8"),
		m_pit0(*this, "pit0"),
		m_pit1(*this, "pit1"),
		m_pit2(*this, "pit2"),
		m_ymsnd(*this, "ymsnd")
{
	m_type = TYPE_LELAND;
}

leland_80186_sound_device::leland_80186_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_dac3(*this, "dac3"),
		m_dac4(*this, "dac4"),
		m_dac5(*this, "dac5"),
		m_dac6(*this, "dac6"),
		m_dac7(*this, "dac7"),
		m_dac8(*this, "dac8"),
		m_pit0(*this, "pit0"),
		m_pit1(*this, "pit1"),
		m_pit2(*this, "pit2"),
		m_ymsnd(*this, "ymsnd")
{
}

const device_type REDLINE_80186 = &device_creator<redline_80186_sound_device>;

redline_80186_sound_device::redline_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: leland_80186_sound_device(mconfig, REDLINE_80186, "80186 DAC (Redline Racer)", tag, owner, clock, "redline_80186_sound", __FILE__)
{
	m_type = TYPE_REDLINE;
}

const device_type ATAXX_80186 = &device_creator<ataxx_80186_sound_device>;

ataxx_80186_sound_device::ataxx_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: leland_80186_sound_device(mconfig, REDLINE_80186, "80186 DAC (Ataxx)", tag, owner, clock, "ataxx_80186_sound", __FILE__)
{
	m_type = TYPE_ATAXX;
}

const device_type WSF_80186 = &device_creator<wsf_80186_sound_device>;

wsf_80186_sound_device::wsf_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: leland_80186_sound_device(mconfig, REDLINE_80186, "80186 DAC (WSF)", tag, owner, clock, "wsf_80186_sound", __FILE__)
{
	m_type = TYPE_WSF;
}

WRITE16_MEMBER(leland_80186_sound_device::peripheral_ctrl)
{
	switch (offset)
	{
		case 2:
			m_peripheral = data;
			break;

		case 4:
		{
			UINT32 temp = (m_peripheral & 0xffc0) << 4;
			if (data & 0x0040)
			{
				m_audiocpu->device_t::memory().space(AS_PROGRAM).install_readwrite_handler(temp, temp + 0x2ff, read16_delegate(FUNC(leland_80186_sound_device::peripheral_r), this), write16_delegate(FUNC(leland_80186_sound_device::peripheral_w), this));
			}
			else
			{
				temp &= 0xffff;
				m_audiocpu->device_t::memory().space(AS_IO).install_readwrite_handler(temp, temp + 0x2ff, read16_delegate(FUNC(leland_80186_sound_device::peripheral_r), this), write16_delegate(FUNC(leland_80186_sound_device::peripheral_w), this));
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

WRITE8_MEMBER( leland_80186_sound_device::leland_80186_control_w )
{
	/* see if anything changed */
	int diff = (m_last_control ^ data) & 0xf8;
	if (diff == 0)
		return;
	m_last_control = data;

	if (LOG_COMM)
	{
		logerror("%04X:80186 control = %02X", m_audiocpu->device_t::safe_pc(), data);
		if (!(data & 0x80)) logerror("  /RESET");
		if (!(data & 0x40)) logerror("  ZNMI");
		if (!(data & 0x20)) logerror("  INT0");
		if (!(data & 0x10)) logerror("  /TEST");
		if (!(data & 0x08)) logerror("  INT1");
		logerror("\n");
	}

	/* /RESET */
	m_audiocpu->device_t::execute().set_input_line(INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
	m_audiocpu->device_t::execute().set_input_line(INPUT_LINE_TEST, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	/* /NMI */
/*  If the master CPU doesn't get a response by the time it's ready to send
    the next command, it uses an NMI to force the issue; unfortunately, this
    seems to really screw up the sound system. It turns out it's better to
    just wait for the original interrupt to occur naturally */
/*  space.machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_NMI, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);*/

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

void leland_80186_sound_device::command_lo_sync(void *ptr, int param)
{
	if (LOG_COMM) logerror("%s:Write sound command latch lo = %02X\n", machine().describe_context(), param);
	m_sound_command = (m_sound_command & 0xff00) | param;
}


WRITE8_MEMBER( leland_80186_sound_device::leland_80186_command_lo_w )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(leland_80186_sound_device::command_lo_sync), this), data);
}


WRITE8_MEMBER( leland_80186_sound_device::leland_80186_command_hi_w )
{
	if (LOG_COMM) logerror("%04X:Write sound command latch hi = %02X\n", m_audiocpu->device_t::safe_pc(), data);
	m_sound_command = (m_sound_command & 0x00ff) | (data << 8);
}


READ16_MEMBER( leland_80186_sound_device::main_to_sound_comm_r )
{
	if (LOG_COMM) logerror("%05X:Read sound command latch = %02X\n", m_audiocpu->device_t::safe_pc(), m_sound_command);
	return m_sound_command;
}




/*************************************
 *
 *  Sound response handling
 *
 *************************************/

void leland_80186_sound_device::delayed_response_r(void *ptr, int param)
{
	cpu_device *master = machine().device<cpu_device>("master");
	int checkpc = param;
	int pc = master->pc();
	int oldaf = master->state_int(Z80_AF);

	/* This is pretty cheesy, but necessary. Since the CPUs run in round-robin order,
	   synchronizing on the write to this register from the slave side does nothing.
	   In order to make sure the master CPU get the real response, we synchronize on
	   the read. However, the value we returned the first time around may not be
	   accurate, so after the system has synced up, we go back into the master CPUs
	   state and put the proper value into the A register. */
	if (pc == checkpc)
	{
		if (LOG_COMM) logerror("(Updated sound response latch to %02X)\n", m_sound_response);

		oldaf = (oldaf & 0x00ff) | (m_sound_response << 8);
		master->set_state_int(Z80_AF, oldaf);
	}
	else if(LOG_COMM)
		logerror("ERROR: delayed_response_r - current PC = %04X, checkPC = %04X\n", pc, checkpc);
}


READ8_MEMBER( leland_80186_sound_device::leland_80186_response_r )
{
	cpu_device *master = machine().device<cpu_device>("master");
	offs_t pc = master->device_t::safe_pcbase();

	if (LOG_COMM) logerror("%04X:Read sound response latch = %02X\n", pc, m_sound_response);

	/* synchronize the response */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(leland_80186_sound_device::delayed_response_r), this), pc + 2);
	return m_sound_response;
}


WRITE16_MEMBER( leland_80186_sound_device::sound_to_main_comm_w )
{
	if (LOG_COMM) logerror("%05X:Write sound response latch = %02X\n", m_audiocpu->device_t::safe_pc(), data);
	m_sound_response = data;
}



/*************************************
 *
 *  Low-level DAC I/O
 *
 *************************************/

WRITE16_MEMBER( leland_80186_sound_device::dac_w )
{
	int which = offset & 7;

	/* handle value changes */
	if (ACCESSING_BITS_0_7)
	{
		switch(which)
		{
			case 0:
				m_dac1->write_signed8(data & 0xff);
				break;
			case 1:
				m_dac2->write_signed8(data & 0xff);
				break;
			case 2:
				m_dac3->write_signed8(data & 0xff);
				break;
			case 3:
				m_dac4->write_signed8(data & 0xff);
				break;
			case 4:
				m_dac5->write_signed8(data & 0xff);
				break;
			case 5:
				m_dac6->write_signed8(data & 0xff);
				break;
			case 6:
				m_dac7->write_signed8(data & 0xff);
				break;
			case 7:
				m_dac8->write_signed8(data & 0xff);
				break;
		}
		m_clock_active &= ~(1<<which);
	}

	/* handle volume changes */
	if (ACCESSING_BITS_8_15)
		switch(which)
		{
			case 0:
				m_dac1->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
			case 1:
				m_dac2->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
			case 2:
				m_dac3->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
			case 3:
				m_dac4->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
			case 4:
				m_dac5->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
			case 5:
				m_dac6->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
			case 6:
				m_dac7->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
			case 7:
				m_dac8->set_output_gain(ALL_OUTPUTS, (data >> 8)/255.0f);
				break;
		}

}


WRITE16_MEMBER( redline_80186_sound_device::redline_dac_w )
{
	dac_w(space, (offset >> 8) & 7, (data & 0xff) | (offset << 8), 0xffff);
}

WRITE16_MEMBER( leland_80186_sound_device::ataxx_dac_control )
{
	/* handle common offsets */
	switch (offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			if (ACCESSING_BITS_0_7)
				dac_w(space, offset, data, 0x00ff);
			return;

		case 0x03:
			if(ACCESSING_BITS_0_7)
			{
				m_dac1->set_output_gain(ALL_OUTPUTS, (((data & 7) * 0x49) >> 1) / 255.0f);
				m_dac2->set_output_gain(ALL_OUTPUTS, ((((data >> 3) & 7) * 0x49) >> 1) / 255.0f);
				m_dac3->set_output_gain(ALL_OUTPUTS, (((data >> 6) & 3) * 0x55) / 255.0f);
			}
			return;

		case 0x21:
			if (ACCESSING_BITS_0_7)
				dac_w(space, 1, data, mem_mask);
			return;

		default:
			break;
	}

	/* if we have a YM2151 (and an external DAC), handle those offsets */
	if (m_type == TYPE_WSF)
	{
		switch (offset)
		{
			case 0x04:
				m_ext_active = 1;
				if (LOG_EXTERN) logerror("External DAC active\n");
				return;

			case 0x05:
				m_ext_active = 0;
				if (LOG_EXTERN) logerror("External DAC inactive\n");
				return;

			case 0x06:
				m_ext_start >>= 4;
				COMBINE_DATA(&m_ext_start);
				m_ext_start <<= 4;
				if (LOG_EXTERN) logerror("External DAC start = %05X\n", m_ext_start);
				return;

			case 0x07:
				m_ext_stop >>= 4;
				COMBINE_DATA(&m_ext_stop);
				m_ext_stop <<= 4;
				if (LOG_EXTERN) logerror("External DAC stop = %05X\n", m_ext_stop);
				return;

			default:
				break;
		}
	}
	logerror("%05X:Unexpected peripheral write %d/%02X = %02X\n", m_audiocpu->device_t::safe_pc(), 5, offset, data);
}



/*************************************
 *
 *  Peripheral chip dispatcher
 *
 *************************************/

READ16_MEMBER( leland_80186_sound_device::peripheral_r )
{
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 0:
			/* we have to return 0 periodically so that they handle interrupts */
			//if ((++m_clock_tick & 7) == 0)
			//  return 0;

			/* if we've filled up all the active channels, we can give this CPU a reset */
			/* until the next interrupt */
			if (m_type != TYPE_REDLINE)
				return ((m_clock_active >> 1) & 0x3e);
			else
				return ((m_clock_active << 1) & 0x7e);

		case 1:
			return main_to_sound_comm_r(space, offset, mem_mask);

		case 2:
			if (mem_mask != 0xff00)
				return m_pit0->read(space, offset & 3);
			break;

		case 3:
			if (m_type <= TYPE_REDLINE)
			{
				if (mem_mask != 0xff00)
					return m_pit1->read(space, offset & 3);
			}
			else if (m_type == TYPE_WSF)
				return m_ymsnd->read(space, offset);
			break;

		case 4:
			if (m_type == TYPE_REDLINE)
			{
				if (mem_mask != 0xff00)
					return m_pit2->read(space, offset & 3);
			}
			else
				logerror("%05X:Unexpected peripheral read %d/%02X\n", m_audiocpu->device_t::safe_pc(), select, offset*2);
			break;

		default:
			logerror("%05X:Unexpected peripheral read %d/%02X\n", m_audiocpu->device_t::safe_pc(), select, offset*2);
			break;
	}
	return 0xffff;
}


WRITE16_MEMBER( leland_80186_sound_device::peripheral_w )
{
	int select = offset / 0x40;
	offset &= 0x3f;

	switch (select)
	{
		case 1:
			sound_to_main_comm_w(space, offset, data, mem_mask);
			break;

		case 2:
			if (mem_mask != 0xff00)
				m_pit0->write(space, offset & 3, data);
			break;

		case 3:
			if (m_type <= TYPE_REDLINE)
			{
				if (mem_mask != 0xff00)
					m_pit1->write(space, offset & 3, data);
			}
			else if(m_type == TYPE_WSF)
				m_ymsnd->write(space, offset, data);
			break;

		case 4:
			if (m_type == TYPE_REDLINE)
			{
				if (mem_mask != 0xff00)
					m_pit2->write(space, offset & 3, data);
			}
			else if (mem_mask == 0xffff)
			{
				m_dac7->write_signed16(data << 6);
				m_clock_active &= ~(1<<6);
			}
			break;

		case 5: /* Ataxx/WSF/Indy Heat only */
			if (m_type > TYPE_REDLINE)
				ataxx_dac_control(space, offset, data, mem_mask);
			break;

		default:
			logerror("%05X:Unexpected peripheral write %d/%02X = %02X\n", m_audiocpu->device_t::safe_pc(), select, offset, data);
			break;
	}
}



/*************************************
 *
 *  Game-specific handlers
 *
 *************************************/

WRITE8_MEMBER( leland_80186_sound_device::ataxx_80186_control_w )
{
	/* compute the bit-shuffled variants of the bits and then write them */
	int modified =  ((data & 0x01) << 7) |
					((data & 0x02) << 5) |
					((data & 0x04) << 3) |
					((data & 0x08) << 1);
	leland_80186_control_w(space, offset, modified);
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

ADDRESS_MAP_START( leland_80186_map_program, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x00000, 0x03fff) AM_MIRROR(0x1c000) AM_RAM
	AM_RANGE(0x20000, 0xfffff) AM_ROM
ADDRESS_MAP_END

ADDRESS_MAP_START( ataxx_80186_map_io, AS_IO, 16, driver_device )
ADDRESS_MAP_END

ADDRESS_MAP_START( redline_80186_map_io, AS_IO, 16, driver_device )
	AM_RANGE(0x0000, 0xffff) AM_DEVWRITE("custom", redline_80186_sound_device, redline_dac_w)
ADDRESS_MAP_END


ADDRESS_MAP_START( leland_80186_map_io, AS_IO, 16, driver_device )
	AM_RANGE(0x0000, 0xffff) AM_DEVWRITE("custom", leland_80186_sound_device, dac_w)
ADDRESS_MAP_END


/************************************************************************

Memory configurations:

    Redline Racer:
        FFDF7:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFDF7:80186 lower chip select = 00FC        -> 00000-00FFF, 4k long
        FFDF7:80186 peripheral chip select = 013C   -> 01000, 01080, 01100, 01180, 01200, 01280, 01300
        FFDF7:80186 middle chip select = 81FC       -> 80000-C0000, 64k chunks, 256k total
        FFDF7:80186 middle P chip select = A0FC

    Quarterback, Team Quarterback, AAFB, Super Offroad, Track Pack, Pigout, Viper:
        FFDFA:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFDFA:80186 peripheral chip select = 203C   -> 20000, 20080, 20100, 20180, 20200, 20280, 20300
        FFDFA:80186 middle chip select = 01FC       -> 00000-7FFFF, 128k chunks, 512k total
        FFDFA:80186 middle P chip select = C0FC

    Ataxx, Indy Heat, World Soccer Finals:
        FFD9D:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFD9D:80186 peripheral chip select = 043C   -> 04000, 04080, 04100, 04180, 04200, 04280, 04300
        FFD9D:80186 middle chip select = 01FC       -> 00000-7FFFF, 128k chunks, 512k total
        FFD9D:80186 middle P chip select = C0BC

************************************************************************/
