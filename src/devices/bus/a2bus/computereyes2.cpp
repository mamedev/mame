// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    computereyes2.c

    Implemention of the Digital Vision ComputerEyes/2 card.

*********************************************************************/

#include "emu.h"
#include "computereyes2.h"

#include "imagedev/picture.h"

#include "bitmap.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_computereyes2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_computereyes2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_computereyes2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	required_device<picture_image_device> m_picture;
	int m_x, m_y, m_cer0, m_cer1, m_cer2;
	u8 m_a2_bitmap[280*193];
	u8 m_threshold;
	bool m_bActive;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_computereyes2_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_computereyes2_device::a2bus_computereyes2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_picture(*this, "srcimg")
{
}

a2bus_computereyes2_device::a2bus_computereyes2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_computereyes2_device(mconfig, A2BUS_COMPUTEREYES2, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_computereyes2_device::device_start()
{
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_cer0));
	save_item(NAME(m_cer1));
	save_item(NAME(m_cer2));
	save_item(NAME(m_threshold));
	save_item(NAME(m_bActive));
	save_item(NAME(m_a2_bitmap));
}

void a2bus_computereyes2_device::device_reset()
{
	m_x = m_y = 0;
	m_cer0 = m_cer2 = 0;
	m_cer1 = 0x80;  // interface disabled
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space

    CER0 read bits
    01 = capture data bit (read)
    10 = sync (read)
    20 = test bit (used in detection) (read)

   CER0 write bits
    0F = brightness (write)
    40 = monitor relay (write)
    80 = turn on test bit (write)

   CER1 write bits
    0F = contrast (guessing bits 0..3?)
    80 = interface on (bit 7)

   CER2 write bits
   (any) resets the threshold for a new pixel
-------------------------------------------------*/

uint8_t a2bus_computereyes2_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			if (m_cer0 & 0x80)
			{
				return m_cer0 | 0x20;
			}
			else
			{
				u8 ret = (m_cer0 & 0xc0) | 0x10;
				u64 video_ticks = machine().time().as_ticks(1021800);
				int frame_time = video_ticks % (65 * 262);  // 65 clocks per scanline, 262 scanlines per frame
				int h = frame_time % 65;
				int v = frame_time / 65;

				// generate sync bit
				if (v > 224)
				{
					// vsync
					ret &= ~0x10;
				}
				else if (h > 49)
				{
					// hsync
					ret &= ~0x10;
				}

				// is the interface enabled?  (Active low, 0 = enabled)
				if (!(m_cer1 & 0x80))
				{
					{
						if (m_bActive)
						{
							//printf("read at X=%d Y=%d CX=%d CY=%d thr=%02x\n", h, v, m_x, m_y, m_threshold);
							ret |= (m_a2_bitmap[m_x + ((v-2) * 280)] > m_threshold) ? 0 : 1;
							if (m_threshold > 0x20)
							{
								m_threshold -= 0x20;
							}
							else
							{
								m_y++;
								if (m_y >= 192)
								{
									if (m_x < 280)
									{
										m_x++;
									}
									else
									{
										m_x = 0;
									}

									m_bActive = false;
								}
							}
						}
					/*  else
					    {
					        printf("SYNC at X=%d Y=%d CX=%d CY=%d thr=%02x\n", h, v, m_x, m_y, m_threshold);
					    }*/
					}
				}
				return ret;
			}
			break;

		case 1: return m_cer1;
		case 2: return m_cer2;
	}

	return 0;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_computereyes2_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_cer0 = data & 0xcf;
			break;

		case 1:
			if (!(data & 0x80))
			{
				m_bActive = false;
			}
			m_y = 0;

			if (!(data & 0x80) && (m_cer1 & 0x80))
			{
				m_x = m_y = 0;
				std::fill_n(m_a2_bitmap, 280*193, 0);

				const bitmap_argb32 &bitmap = m_picture->get_bitmap();
				if (bitmap.valid())
				{
					// convert arbitrary sized ARGB32 image to a 188x193 image with 256 levels of grayscale
					double stepx = (double)bitmap.width() / 188.0;
					double stepy = (double)bitmap.height() / 193.0;

					for (int y = 0; y < 193; y++)
					{
						for (int x = 0; x < 280; x++)
						{
							u32 pixel = bitmap.pix(int((double)y * stepy), int((double)x * stepx));
							double mono = ((0.2126 * double(((pixel>>16) & 0xff) / 255.0)) +
								   (0.7152 * double(((pixel>>8) & 0xff) / 255.0)) +
								   (0.0722 * double((pixel& 0xff) / 255.0)));
							m_a2_bitmap[(y*280)+x] = u8(mono * 255.0);
						}
					}
				}
			}
			m_cer1 = data;
			break;

		case 2: // written to initialize a sample
			m_cer2 = data;
			m_threshold = 0xe0;
			if (!m_bActive)
			{
				m_y = 0;
			}
			m_bActive = true;
			break;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_COMPUTEREYES2, device_a2bus_card_interface, a2bus_computereyes2_device, "a2ceyes2", "Digital Vision ComputerEyes/2")
