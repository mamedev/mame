/***************************************************************************

    MPU-401 MIDI device interface

    TODO:
    - skeleton, doesn't do anything

***************************************************************************/

#include "emu.h"
#include "isa_mpu401.h"
#include "machine/pic8259.h"

/*
DIP-SWs
1-2-3-4
         0x200
      1  0x210
    1    0x220
...
1   1 1  0x330 (default)
...
1 1 1 1  0x370

5-6-7-8
1        irq2 (default)
  1      irq3
    1    irq5
      1  irq7
*/

READ8_MEMBER( isa8_mpu401_device::mpu401_r )
{
	UINT8 res;

	if(offset == 0) // data
	{
		res = 0xff;
	}
	else // status
	{
		res = 0x3f | 0x80; // bit 7 queue empty (DSR), bit 6 DRR (Data Receive Ready?)
	}

	return res;
}

WRITE8_MEMBER( isa8_mpu401_device::mpu401_w )
{
	if(offset == 0) // data
	{
		printf("%02x %02x\n",offset,data);
	}
	else // command
	{
		printf("%02x %02x\n",offset,data);

		switch(data)
		{
			case 0xff: // reset
				//m_isa->irq2_w(1);
				break;
		}
	}

}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_MPU401 = &device_creator<isa8_mpu401_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_adlib_device - constructor
//-------------------------------------------------

isa8_mpu401_device::isa8_mpu401_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, ISA8_MPU401, "Roland MPU-401 Sound Card", tag, owner, clock),
		device_isa8_card_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_mpu401_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x330, 0x0331, 0, 0, read8_delegate(FUNC(isa8_mpu401_device::mpu401_r), this), write8_delegate(FUNC(isa8_mpu401_device::mpu401_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_mpu401_device::device_reset()
{
}
