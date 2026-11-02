// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Acclaim PinkEye JPEG texture decoder (Magic the Gathering: Armageddon)
    Emulation by R. Belmont

    PinkEye is a PCI daughterboard where a Zoran ZR36120 bridges PCI to a
    "GuestBus" holding an FPGA (guests 0 and 1), a ZR36050 JPEG codec (guest 2)
    and a ZR36016 colour space converter (guest 3).  The FPGA reads the
    JPEGs from flash and each texture's mask from the key ROM.

    The game queues 12-byte commands, one per texture, for the ZR36120's
    code transfer engine to hand to the FPGA, and the ZR36120 writes the
    decoded 16-bit texels to host memory from its video top address on.

    TODO:
    - decoding takes no time
    - the ZR36120 is only emulated as far as the game uses it

***************************************************************************/

#include "emu.h"
#include "magictg_pinkeye.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(ACCLAIM_PINKEYE, acclaim_pinkeye_device, "acclaim_pinkeye", "Acclaim PinkEye JPEG texture decoder")


acclaim_pinkeye_device::acclaim_pinkeye_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, ACCLAIM_PINKEYE, tag, owner, clock)
	, m_zr36050(*this, "zr36050")
	, m_flash(*this, "jpeg")
	, m_key(*this, "key")
	, m_alpha(*this, "alpha", 256 * 256, ENDIANNESS_LITTLE)
	, m_irq_cb(*this)
	, m_done_timer(nullptr)
	, m_regs{}
	, m_po(0)
	, m_fpga{}
	, m_flash_ptr(0)
	, m_zr050_addr(0)
	, m_zr016_regs{}
	, m_zr016_iaddr(0)
	, m_zr016_iregs{}
{
	// the ZR36120 is a multimedia video controller
	set_ids(0x11de6120, 0x02, 0x040000, 0x00000000);
}

void acclaim_pinkeye_device::device_add_mconfig(machine_config &config)
{
	ZR36050(config, m_zr36050, 29'500'000);
	m_zr36050->code_read().set(FUNC(acclaim_pinkeye_device::code_r));
}

void acclaim_pinkeye_device::device_start()
{
	pci_device::device_start();
	add_map(0x1000, M_MEM, FUNC(acclaim_pinkeye_device::regs_map));
	intr_pin = 1;

	m_done_timer = timer_alloc(FUNC(acclaim_pinkeye_device::transfer_done), this);

	// only used while a texture is decoded, so not saved
	m_mask = make_unique_clear<uint8_t []>(256 * 256);

	save_item(NAME(m_regs));
	save_item(NAME(m_po));
	save_item(NAME(m_fpga));
	save_item(NAME(m_flash_ptr));
	save_item(NAME(m_zr050_addr));
	save_item(NAME(m_zr016_regs));
	save_item(NAME(m_zr016_iaddr));
	save_item(NAME(m_zr016_iregs));
}

void acclaim_pinkeye_device::device_reset()
{
	pci_device::device_reset();

	m_regs[0x00 / 4] = (1 << 10) | 0x3ff;
	m_regs[0x04 / 4] = (1 << 10) | 0x3ff;
	m_regs[0x08 / 4] = (1 << 25) | (2 << 3) | 1;
	m_regs[0x0c / 4] = 0xfffffffc;
	m_regs[0x10 / 4] = 0xfffffffc;
	m_regs[0x14 / 4] = 0x00000000;
	m_regs[0x18 / 4] = (7 << 25) | (0xf0 << 12) | 0x3ff;
	m_regs[0x1c / 4] = 0xfffffffc;
	m_regs[0x20 / 4] = 0xfffffffc;
	m_regs[0x24 / 4] = 0x000000ff;
	m_regs[0x28 / 4] = 0x000000ff;
	m_regs[0x2c / 4] = 0xf0000000;
	m_regs[0x30 / 4] = 0xfffffffc;
	m_regs[0x34 / 4] = (1 << 29) | (1 << 28) | (3 << 12) | (1 << 8) | (6 << 1);
	m_regs[0x38 / 4] = 0x00000000;
	m_regs[0x3c / 4] = 0x00000000;
	m_regs[0x40 / 4] = 0x00000000;
	m_regs[0x44 / 4] = 0x00000003;
	m_regs[0x48 / 4] = 0x00000000;
	m_po = 0;

	m_done_timer->adjust(attotime::never);
	update_irq();
}

void acclaim_pinkeye_device::regs_map(address_map &map)
{
	map(0x000, 0xfff).rw(FUNC(acclaim_pinkeye_device::regs_r), FUNC(acclaim_pinkeye_device::regs_w));
}

uint32_t acclaim_pinkeye_device::regs_r(offs_t offset)
{
	if (offset < std::size(m_regs))
	{
		return m_regs[offset];
	}
	else if (offset == 0x204 / 4)
	{
		return m_po;
	}

	if (!machine().side_effects_disabled())
	{
		logerror("%s: ZR36120 read %03x\n", machine().describe_context(), offset * 4);
	}
	return 0;
}

void acclaim_pinkeye_device::regs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0x204 / 4)
	{
		// PostOffice: accesses complete at once, so never pending or timed out
		uint32_t po = m_po;
		COMBINE_DATA(&po);
		m_po = po & 0x00ff00ff;
		if (BIT(po, 23))
		{
			guest_w(BIT(po, 20, 2), BIT(po, 16, 3), po & 0xff);
		}
		else
		{
			m_po = (m_po & ~0xff) | guest_r(BIT(po, 20, 2), BIT(po, 16, 3));
		}
		return;
	}
	else if (offset >= std::size(m_regs))
	{
		logerror("%s: ZR36120 write %03x = %08x & %08x\n", machine().describe_context(), offset * 4, data, mem_mask);
		return;
	}

	switch (offset)
	{
		case 0x34 / 4:
			COMBINE_DATA(&m_regs[offset]);
			if (ACCESSING_BITS_0_7 && BIT(data, 7))
			{
				code_transfer();
			}
			break;

		case 0x3c / 4:
			// interrupt status bits are cleared by writing 1
			m_regs[offset] &= ~(data & mem_mask & 0x70000000);
			update_irq();
			break;

		case 0x40 / 4:
			COMBINE_DATA(&m_regs[offset]);
			update_irq();
			break;

		default:
			COMBINE_DATA(&m_regs[offset]);
			break;
	}
}

uint8_t acclaim_pinkeye_device::guest_r(int guest, int reg)
{
	switch (guest)
	{
		case 0:
			return m_fpga[0][reg];

		case 1:
			if (reg == 3)
			{
				// flash data port
				return m_flash[m_flash_ptr++ & 0x1ffffff];
			}
			return m_fpga[1][reg];

		case 2:
			if (reg < 4)
			{
				return m_zr36050->read((reg << 8) | m_zr050_addr);
			}
			return m_zr050_addr;

		case 3:
			// the ZR36016 only decodes two address lines
			reg &= 3;
			if (reg == 3)
			{
				return m_zr016_iregs[m_zr016_iaddr & 0x0f];
			}
			else if (reg == 2)
			{
				return m_zr016_iaddr;
			}
			return m_zr016_regs[reg];
	}
	return 0xff;
}

void acclaim_pinkeye_device::guest_w(int guest, int reg, uint8_t data)
{
	switch (guest)
	{
		case 0:
			m_fpga[0][reg] = data;
			break;

		case 1:
			if (reg == 3)
			{
				logerror("%s: flash write %07x = %02x\n", machine().describe_context(), m_flash_ptr, data);
				m_flash_ptr++;
				break;
			}
			m_fpga[1][reg] = data;
			if (reg < 3)
			{
				m_flash_ptr = (BIT(m_fpga[0][6], 6) << 24) | (m_fpga[1][2] << 16) | (m_fpga[1][1] << 8) | m_fpga[1][0];
			}
			break;

		case 2:
			if (reg < 4)
			{
				m_zr36050->write((reg << 8) | m_zr050_addr, data);
			}
			else if (reg == 4)
			{
				m_zr050_addr = data;
			}
			break;

		case 3:
			reg &= 3;
			if (reg == 3)
			{
				m_zr016_iregs[m_zr016_iaddr & 0x0f] = data;
			}
			else if (reg == 2)
			{
				m_zr016_iaddr = data;
			}
			else
			{
				m_zr016_regs[reg] = data;
			}
			break;
	}
}

void acclaim_pinkeye_device::code_transfer()
{
	// FPGA register for each command byte; byte 1 bit 6 marks the last command of a run
	static constexpr int FPGA_REG[12][2] = {
			{ 0, 4 }, { 0, 2 }, { 0, 1 }, { 0, 7 }, { 1, 1 }, { 1, 0 },
			{ 0, 6 }, { 0, 5 }, { -1, 0 }, { 1, 6 }, { 0, 0 }, { 1, 2 } };

	// the game passes KSEG0 addresses, and how the board decodes the segment bits is unknown
	address_space &space = *get_pci_busmaster_space();
	offs_t source = m_regs[0x30 / 4] & 0x1fffffff;
	offs_t dest = m_regs[0x0c / 4] & 0x1fffffff;
	for (int block = 0; block < 64; block++, source += 12)
	{
		uint8_t command[12];
		for (int i = 0; i < 12; i++)
		{
			command[i] = space.read_byte(source + i);
			if (FPGA_REG[i][0] >= 0)
			{
				m_fpga[FPGA_REG[i][0]][FPGA_REG[i][1]] = command[i];
			}
		}

		dest = decode(dest);
		if (BIT(command[1], 6))
		{
			break;
		}
	}

	m_done_timer->adjust(attotime::from_usec(500));
}

offs_t acclaim_pinkeye_device::decode(offs_t dest)
{
	m_flash_ptr = (BIT(m_fpga[0][6], 6) << 24) | (m_fpga[1][2] << 16) | (m_fpga[1][1] << 8) | m_fpga[1][0];
	const uint8_t format = m_fpga[0][7];
	const uint8_t mode = m_fpga[0][2] & 0x3f;

	m_zr36050->write(0x000, 0x01);

	const uint32_t vfeh = m_regs[0x00 / 4];
	const int width = std::max(int(BIT(vfeh, 0, 10)) - int(BIT(vfeh, 10, 10)) + 1, 0);
	const int height = std::min<int>(m_zr36050->image_height(), 256);
	const int key_w = std::min<int>(m_zr36050->image_width(), 256);
	const int src_w = std::min(key_w, width);

	// an alpha pass leaves the luma in the FPGA's buffer for the following colour pass
	if (mode == 0x23)
	{
		for (int y = 0; y < height; y++)
		{
			const uint8_t *const src = m_zr36050->image_row(y);
			for (int x = 0; x < 256; x++)
			{
				m_alpha[y * 256 + x] = (x < src_w) ? src[x * 3] : 0;
			}
		}
		return dest;
	}

	// the key covers the JPEG image; a colour pass also keeps the luma from its alpha pass
	decode_key((BIT(m_fpga[0][6], 0, 6) << 16) | (m_fpga[0][5] << 8) | m_fpga[0][4], key_w, height);

	address_space &space = *get_pci_busmaster_space();
	for (int y = 0; y < height; y++)
	{
		const uint8_t *const src = m_zr36050->image_row(y);
		for (int x = 0; x < width; x += 2)
		{
			uint32_t texels = 0;
			for (int i = 0; i < 2; i++)
			{
				if ((x + i) >= src_w)
				{
					continue;
				}

				const uint8_t *const pixel = src + (x + i) * 3;
				const rgb_t color = zr36050_device::ycbcr_to_rgb(pixel[0], pixel[1], pixel[2]);
				uint8_t alpha = m_mask[y * key_w + x + i];
				if (mode == 0x1f)
				{
					alpha &= m_alpha[y * 256 + x + i];
				}

				uint16_t texel;
				if ((format == 6) && (mode != 0x1f))
				{
					// RGB 565
					texel = ((color.r() >> 3) << 11) | ((color.g() >> 2) << 5) | (color.b() >> 3);
				}
				else if (format == 5)
				{
					// ARGB 1555
					texel = (BIT(alpha, 7) << 15) | ((color.r() >> 3) << 10) | ((color.g() >> 3) << 5) | (color.b() >> 3);
				}
				else
				{
					// ARGB 4444
					texel = ((alpha >> 4) << 12) | ((color.r() >> 4) << 8) | (color.g() & 0xf0) | (color.b() >> 4);
				}
				texels |= uint32_t(texel) << (16 * i);
			}

			space.write_dword(dest + (y * width + x) * 2, texels);
		}
	}
	return dest + width * height * 2;
}

void acclaim_pinkeye_device::decode_key(offs_t address, int width, int height)
{
	// alternating transparent and opaque runs, transparent first: n zero bits and a one, then 3(n+1)
	// bits of run length - (n+1); 00001 1xx is two pixels, toggling first if xx is 0; 8 zeros end it
	const uint32_t count = std::min(width * height, 256 * 256);
	uint32_t bitpos = address << 3;
	const auto next_bit =
			[this, &bitpos] () -> int
			{
				const int bit = BIT(m_key[(bitpos >> 3) & 0x3fffff], ~bitpos & 7);
				bitpos++;
				return bit;
			};

	uint32_t pixel = 0;
	uint8_t state = 0x00;
	while (pixel < count)
	{
		int zeros = 0;
		while ((zeros < 8) && !next_bit())
		{
			zeros++;
		}

		if (zeros > 4)
		{
			if (zeros < 8)
			{
				logerror("unknown key code at %06x\n", bitpos >> 3);
			}
			break;
		}

		const int bits = (zeros < 4) ? (3 * (zeros + 1)) : 3;
		uint32_t value = 0;
		for (int i = 0; i < bits; i++)
		{
			value = (value << 1) | next_bit();
		}

		uint32_t run;
		if (zeros < 4)
		{
			run = value + zeros + 1;
		}
		else
		{
			if (!BIT(value, 0, 2))
			{
				state ^= 0xff;
			}
			run = 2;
		}

		const uint32_t end = std::min(pixel + run, count);
		std::fill(m_mask.get() + pixel, m_mask.get() + end, state);
		pixel = end;
		if (zeros < 4)
		{
			state ^= 0xff;
		}
	}
	std::fill(m_mask.get() + pixel, m_mask.get() + count, 0x00);
}

uint8_t acclaim_pinkeye_device::code_r()
{
	return m_flash[m_flash_ptr++ & 0x1ffffff];
}

TIMER_CALLBACK_MEMBER(acclaim_pinkeye_device::transfer_done)
{
	// the FPGA signals the end of a frame on GuestBus IRQ 1
	m_regs[0x3c / 4] |= 0x40000000;
	update_irq();
}

void acclaim_pinkeye_device::update_irq()
{
	const uint32_t icr = m_regs[0x40 / 4];
	m_irq_cb(BIT(icr, 24) && (m_regs[0x3c / 4] & icr & 0x70000000));
}
