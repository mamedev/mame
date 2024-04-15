// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Philip Bennett, hap
/*****************************************************************************

  Fujitsu MB87078 6-bit, 4-channel electronic volume controller emulator

  An excerpt from the datasheet about the chip functionality:
  "A digital signal input controls gain every 0.5 dB step from 0dB to -32dB.
  - Gain variable range: 0 dB to -32 dB by 0.5dB or -infinity
  - Gain variable range is expanded to connect two channels serially (0 dB to -64 dB)
  - Each channel gain can be set respectively
  - Test function is provided (to confirm internal data)
  - Data is initialized by reset signal (all channels are set to 0dB)
  - Logic I/O is TTL comatible"

  There are 6 digital data input/output pins and DSEL pin that selects
  the group (there are two) of internal registers to be read/written.

  Group 0 is 6-bit gain latch
  Group 1 is 5-bit control latch (2-bits are channel select and 3-bits are volume control)

  Digital I/O Setting:
  /TC   DSEL    D0      D1      D2      D3      D4      D5      I/O MODES (when /TC==H ->write)
  H     H       DSC1    DSC2    EN      C0      C32     X       Input mode
  H     L       GD0     GD1     GD2     GD3     GD4     GD5     (set)
  L     H       DSC1    DSC2    EN      C0      C32     L       Output mode
  L     L       GD0     GD1     GD2     GD3     GD4     GD5     (check)

  Channel Setting:
  DSC2  DSC1    CHANNEL
  L      L         0
  L      H         1
  H      L         2
  H      H         3

  Electrical Volume Setting:
                 DATA*                  GAIN
  GD5 GD4 GD3 GD2 GD1 GD0  EN  C0  C32  (dB)
   1   1   1   1   1   1   1   0   0     0
   1   1   1   1   1   0   1   0   0    -0.5
   1   1   1   1   0   1   1   0   0    -1
   1   1   1   1   0   0   1   0   0    -1.5
   1   1   1   0   1   1   1   0   0    -2
  [..........................................]
   0   0   0   0   0   1   1   0   0    -31
   0   0   0   0   0   0   1   0   0    -31.5
   X   X   X   X   X   X   1   X   1    -32
   X   X   X   X   X   X   1   1   0     0
   X   X   X   X   X   X   0   X   X    -infinity

   X=don't care
   * When reset, DATA is set to 0 dB (code 111111 100)


  MB87078 pins and assigned interface variables/functions

                   /[ 1] D0        /TC [24]
                  | [ 2] D1        /WR [23]
          data_w()| [ 3] D2        /CE [22]
          data_r()| [ 4] D3       DSEL [21]-data_w()/data_r() (offset)
          (data)  | [ 5] D4     /RESET [20]
                   \[ 6] D5        /PD [19]
                    [ 7] DGND      VDD [18]
                    [ 8] AGND  1/2 VDD [17]
                    [ 9] AIN0    AOUT3 [16]
                    [10] AOUT0    AIN3 [15]
                    [11] AIN1    AOUT2 [14]
                    [12] AOUT1    AIN2 [13]

*****************************************************************************/

#include "emu.h"
#include "mb87078.h"


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

DEFINE_DEVICE_TYPE(MB87078, mb87078_device, "mb87078", "MB87078 Volume Controller")

mb87078_device::mb87078_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MB87078, tag, owner, clock),
	m_gain_changed_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb87078_device::device_start()
{
	m_data = 0;
	m_control = 0;

	// invalidate gain index
	for (int i = 0; i < 4; i++)
		m_gain_index[i] = 66;

	// output volume table, 0dB to -32dB in steps of -0.5dB
	for (int i = 0; i < (64+1); i++)
		m_lut_gains[i] = pow(10.0, (-0.5 * i) / 20.0);
	m_lut_gains[65] = 0.0; // -infinity
	m_lut_gains[66] = m_lut_gains[0];

	// register for savestates
	save_item(NAME(m_gain_index));
	save_item(NAME(m_channel_latch));
	save_item(NAME(m_data));
	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb87078_device::device_reset()
{
	// all channels enabled, and set at 0dB
	for (int i = 0; i < 4; i++)
		m_channel_latch[i] = 0x7f;

	gain_recalc();
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

void mb87078_device::gain_recalc()
{
	for (int i = 0; i < 4; i++)
	{
		int gain_index;

		// EN = 0: -infinity dB
		if (~m_channel_latch[i] & 0x40)
			gain_index = 65;

		// C32 = 1: -32dB
		else if (m_channel_latch[i] & 0x100)
			gain_index = 64;

		// C0 = 1: 0dB
		else if (m_channel_latch[i] & 0x80)
			gain_index = 0;

		else
			gain_index = ~m_channel_latch[i] & 0x3f;

		// callback on change
		if (gain_index != m_gain_index[i])
		{
			m_gain_index[i] = gain_index;
			m_gain_changed_cb((offs_t)i, gain_percent_r(i));
		}
	}
}

void mb87078_device::data_w(offs_t offset, u8 data)
{
	if (offset & 1)
		m_control = data & 0x1f;
	else
	{
		m_data = data & 0x3f;

		// set channel volume gain
		m_channel_latch[m_control & 3] = (m_control << 4 & 0x1c0) | m_data;
		gain_recalc();
	}
}

u8 mb87078_device::data_r(offs_t offset)
{
	return (offset & 1) ? m_control : m_data;
}
