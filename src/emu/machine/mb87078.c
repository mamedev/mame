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
  MB87078_data_w()| [ 3] D2        /CE [22]
  MB87078_data_r()| [ 4] D3       DSEL [21]-MB87078_data_w()/data_r() parameter
                  | [ 5] D4     /RESET [20]-MB87078_reset_comp_w()
                   \[ 6] D5        /PD [19]
                    [ 7] DGND      VDD [18]
                    [ 8] AGND  1/2 VDD [17]
                    [ 9] AIN0    AOUT3 [16]
                    [10] AOUT0    AIN3 [15]
                    [11] AIN1    AOUT2 [14]
                    [12] AOUT1    AIN2 [13]


 *****************************************************************************/

#include "emu.h"
#include "machine/mb87078.h"

struct mb87078_state
{
	int          gain[4];       /* gain index 0-63,64,65 */
	int          channel_latch; /* current channel */
	UINT8        latch[2][4];   /* 6bit+3bit 4 data latches */
	UINT8        reset_comp;

	mb87078_gain_changed_cb   gain_changed_cb;
};


static const float mb87078_gain_decibel[66] = {
	0.0, -0.5, -1.0, -1.5, -2.0, -2.5, -3.0, -3.5,
	-4.0, -4.5, -5.0, -5.5, -6.0, -6.5, -7.0, -7.5,
	-8.0, -8.5, -9.0, -9.5,-10.0,-10.5,-11.0,-11.5,
	-12.0,-12.5,-13.0,-13.5,-14.0,-14.5,-15.0,-15.5,
	-16.0,-16.5,-17.0,-17.5,-18.0,-18.5,-19.0,-19.5,
	-20.0,-20.5,-21.0,-21.5,-22.0,-22.5,-23.0,-23.5,
	-24.0,-24.5,-25.0,-25.5,-26.0,-26.5,-27.0,-27.5,
	-28.0,-28.5,-29.0,-29.5,-30.0,-30.5,-31.0,-31.5,
	-32.0, -256.0
	};

static const int mb87078_gain_percent[66] = {
	100,94,89,84,79,74,70,66,
	63,59,56,53,50,47,44,42,
	39,37,35,33,31,29,28,26,
	25,23,22,21,19,18,17,16,
	15,14,14,13,12,11,11,10,
	10, 9, 8, 8, 7, 7, 7, 6,
		6, 5, 5, 5, 5, 4, 4, 4,
		3, 3, 3, 3, 3, 2, 2, 2,
	2, 0
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE mb87078_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == MB87078);

	return (mb87078_state *)downcast<mb87078_device *>(device)->token();
}

INLINE const mb87078_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == MB87078));
	return (const mb87078_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

#define GAIN_MAX_INDEX 64
#define GAIN_INFINITY_INDEX 65


static int calc_gain_index( int data0, int data1 )
{
//data 0: GD0-GD5
//data 1: 1  2  4  8  16
//        c1 c2 EN C0 C32

	if (!(data1 & 0x04))
	{
		return GAIN_INFINITY_INDEX;
	}
	else
	{
		if (data1 & 0x10)
		{
			return GAIN_MAX_INDEX;
		}
		else
		{
			if (data1 & 0x08)
			{
				return 0;
			}
			else
			{
				return (data0 ^ 0x3f);
			}
		}
	}
}


static void gain_recalc( device_t *device )
{
	mb87078_state *mb87078 = get_safe_token(device);
	int i;

	for (i = 0; i < 4; i++)
	{
		int old_index = mb87078->gain[i];
		mb87078->gain[i] = calc_gain_index(mb87078->latch[0][i], mb87078->latch[1][i]);
		if (old_index != mb87078->gain[i])
			mb87078->gain_changed_cb(device->machine(), i, mb87078_gain_percent[mb87078->gain[i]]);
	}
}



void mb87078_data_w( device_t *device, int data, int dsel )
{
	mb87078_state *mb87078 = get_safe_token(device);

	if (mb87078->reset_comp == 0)
		return;

	if (dsel == 0)  /* gd0 - gd5 */
	{
		mb87078->latch[0][mb87078->channel_latch] = data & 0x3f;
	}
	else        /* dcs1, dsc2, en, c0, c32, X */
	{
		mb87078->channel_latch = data & 3;
		mb87078->latch[1][mb87078->channel_latch] = data & 0x1f; //always zero bit 5
	}
	gain_recalc(device);
}


float mb87078_gain_decibel_r( device_t *device, int channel )
{
	mb87078_state *mb87078 = get_safe_token(device);
	return mb87078_gain_decibel[mb87078->gain[channel]];
}


int mb87078_gain_percent_r( device_t *device, int channel )
{
	mb87078_state *mb87078 = get_safe_token(device);
	return mb87078_gain_percent[mb87078->gain[channel]];
}

void mb87078_reset_comp_w( device_t *device, int level )
{
	mb87078_state *mb87078 = get_safe_token(device);

	mb87078->reset_comp = level;

	/*this seems to be true, according to the datasheets*/
	if (level == 0)
	{
		mb87078->latch[0][0] = 0x3f;
		mb87078->latch[0][1] = 0x3f;
		mb87078->latch[0][2] = 0x3f;
		mb87078->latch[0][3] = 0x3f;

		mb87078->latch[1][0] = 0x0 | 0x4;
		mb87078->latch[1][1] = 0x1 | 0x4;
		mb87078->latch[1][2] = 0x2 | 0x4;
		mb87078->latch[1][3] = 0x3 | 0x4;
	}

	gain_recalc(device);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( mb87078 )
{
	mb87078_state *mb87078 = get_safe_token(device);
	const mb87078_interface *intf = get_interface(device);

	mb87078->gain_changed_cb = intf->gain_changed_cb;

	device->save_item(NAME(mb87078->channel_latch));
	device->save_item(NAME(mb87078->reset_comp));
	device->save_item(NAME(mb87078->latch[0]));
	device->save_item(NAME(mb87078->latch[1]));
	device->save_item(NAME(mb87078->gain));
}

static DEVICE_RESET( mb87078 )
{
	mb87078_state *mb87078 = get_safe_token(device);

	mb87078->channel_latch = 0;

	/* reset chip */
	mb87078_reset_comp_w(device, 0);
	mb87078_reset_comp_w(device, 1);
}

const device_type MB87078 = &device_creator<mb87078_device>;

mb87078_device::mb87078_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB87078, "Fujitsu MB87078", tag, owner, clock)
{
	m_token = global_alloc_clear(mb87078_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mb87078_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb87078_device::device_start()
{
	DEVICE_START_NAME( mb87078 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb87078_device::device_reset()
{
	DEVICE_RESET_NAME( mb87078 )(this);
}
