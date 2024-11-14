// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Digital Vision ComputerEyes (original gameport version)

*********************************************************************/

#include "emu.h"
#include "bus/a2gameio/computereyes.h"

#include "imagedev/picture.h"

#include "bitmap.h"


namespace {

// ======================> apple2_compeyes_device

class apple2_compeyes_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_compeyes_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_a2gameio_interface overrides
	virtual int sw0_r() override;
	virtual int sw1_r() override;
	virtual void an0_w(int state) override;
	virtual void an1_w(int state) override;
	virtual void an2_w(int state) override;
	virtual void an3_w(int state) override;

private:
	required_device<picture_image_device> m_picture;
	int m_x, m_y, m_an1, m_an2, m_an3, m_level;
	u8 m_a2_bitmap[280*192];
};

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

apple2_compeyes_device::apple2_compeyes_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE2_COMPUTEREYES, tag, owner, clock)
	, device_a2gameio_interface(mconfig, *this)
	, m_picture(*this, "srcimg")
{
}

void apple2_compeyes_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}

void apple2_compeyes_device::device_start()
{
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_an1));
	save_item(NAME(m_an2));
	save_item(NAME(m_an3));
	save_item(NAME(m_level));
}

void apple2_compeyes_device::device_reset()
{
	m_x = m_y = m_an1 = m_an2 = m_an3 = m_level = 0;
}

int apple2_compeyes_device::sw0_r()
{
	return 0;
}

int apple2_compeyes_device::sw1_r()
{
	// to avoid triggering the self-test on //e, return only 0
	// for the first 2 seconds of emulation.
	if (machine().time().seconds() < 2)
	{
		return 0;
	}

	int res = m_a2_bitmap[(m_y*280)+m_x] > m_level ? 1 : 0;
//  if (m_a2_bitmap[(m_y*280)+m_x] != 0)
//      printf("Read pixel at (%d, %d) = %d (pix %d, level %d)\n", m_x, m_y, res, m_a2_bitmap[(m_y*280)+m_x], m_level);
	m_y++;
	if (m_y >= 192)
	{
		if (m_x < 279)
		{
			m_x++;
		}
		m_y = 0;
	}
	return res;
}

void apple2_compeyes_device::an0_w(int state)
{
	m_x =  m_y = 0;

	std::fill_n(m_a2_bitmap, 280*192, 0);

	const bitmap_argb32 &bitmap = m_picture->get_bitmap();
	if (bitmap.valid())
	{
		// convert arbitrary sized ARGB32 image to a 280x192 image with 256 levels of grayscale
		double stepx = (double)bitmap.width() / 280.0;
		double stepy = (double)bitmap.height() / 192.0;

		for (int y = 0; y < 192; y++)
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

void apple2_compeyes_device::an1_w(int state)
{
	m_an1 = state;
	m_level = (128 * m_an2) + (64 * m_an1) + (32 * m_an3);
}

void apple2_compeyes_device::an2_w(int state)
{
	m_an2 = state;
	m_level = (128 * m_an2) + (64 * m_an1) + (32 * m_an3);
}

void apple2_compeyes_device::an3_w(int state)
{
	m_an3 = state;
	m_level = (128 * m_an2) + (64 * m_an1) + (32 * m_an3);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_COMPUTEREYES, device_a2gameio_interface, apple2_compeyes_device, "a2ceyes", "Digital Vision ComputerEyes")
