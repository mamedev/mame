// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_framestore.cpp
    DPB-7000/1 - Framestore Card

	TODO:
	- Code is currently a more or less direct translation of the board
	  schematic. It is highly inefficient, but accurate. An equally-
	  accurate, but faster, version can be made once better understanding
	  of the overall DPB-7000 system is had.

***************************************************************************/

#include "emu.h"
#include "dpb_framestore.h"


/*****************************************************************************/

DEFINE_DEVICE_TYPE(DPB7000_FRAMESTORE, dpb7000_framestore_card_device, "dpb_framestore", "Quantel DPB-7000 Framestore Card")


//-------------------------------------------------
//  dpb7000_framestore_card_device - constructor
//-------------------------------------------------

dpb7000_framestore_card_device::dpb7000_framestore_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DPB7000_FRAMESTORE, tag, owner, clock)
	, m_vopen(false)
	, m_copen(false)
	, m_csel(false)
	, m_rck(false)
	, m_cck(false)
	, m_opstr(false)
	, m_opwa(false)
	, m_opwb(false)
	, m_opw(0)
	, m_opra(false)
	, m_oprb(false)
	, m_opr(0)
	, m_ras_in(false)
	, m_cas(false)
	, m_write(false)
	, m_ipen(false)
	, m_whp(false)
	, m_lumen(false)
	, m_bdsel(false)
	, m_ipsel(0)
	, m_cav(0)
	, m_front_pal_base(nullptr)
	, m_front_pal_addr(0)
	, m_front_pal_out(0)
	, m_back_pal_base(nullptr)
	, m_back_pal_addr(0)
	, m_back_pal_out(0)
	, m_a(0)
	, m_ra(0)
	, m_wa(0)
	, m_cdata_front(0)
	, m_cdata_back(0)
	, m_cdata(0)
	, m_vdata_front(0)
	, m_vdata_back(0)
	, m_vdata(0)
	, m_cdata_out(*this)
	, m_vdata_out(*this)
	, m_cbusy_out(*this)
	, m_back_pal(*this, "back_pal")
	, m_front_pal(*this, "front_pal")
{
}

ROM_START( dpb7000_framestore )
	ROM_REGION(0x400, "back_pal", 0)
	ROM_LOAD("pb-02f-01748a-aba.bin", 0x000, 0x400, CRC(24b31494) SHA1(f9185a00e5470ec95d234a76c15acbf33cfb285d))

	ROM_REGION(0x400, "front_pal", 0)
	ROM_LOAD("pb-02f-01748a-bba.bin", 0x000, 0x400, CRC(8f06b632) SHA1(233b841c3957a6df229f3a693f9288cb8feec58c))
ROM_END

const tiny_rom_entry *dpb7000_framestore_card_device::device_rom_region() const
{
	return ROM_NAME( dpb7000_framestore );
}

void dpb7000_framestore_card_device::device_start()
{
	save_item(NAME(m_vopen));
	save_item(NAME(m_copen));
	save_item(NAME(m_csel));
	save_item(NAME(m_rck));
	save_item(NAME(m_cck));

	save_item(NAME(m_openx));
	save_item(NAME(m_opstr));
	save_item(NAME(m_opwa));
	save_item(NAME(m_opwb));
	save_item(NAME(m_opw));
	save_item(NAME(m_opra));
	save_item(NAME(m_oprb));
	save_item(NAME(m_opr));

	save_item(NAME(m_ras_in));
	save_pointer(NAME(m_ras), 10);
	save_item(NAME(m_cas));
	save_item(NAME(m_write));
	save_item(NAME(m_ipen));
	save_item(NAME(m_whp));
	save_item(NAME(m_lumen));
	save_item(NAME(m_bdsel));
	save_item(NAME(m_ipsel));

	save_pointer(NAME(m_ipenx), 10);
	save_pointer(NAME(m_rav), 10);
	save_item(NAME(m_cav));

	save_item(NAME(m_front_pal_addr));
	save_item(NAME(m_front_pal_out));
	save_item(NAME(m_back_pal_addr));
	save_item(NAME(m_back_pal_out));

	save_item(NAME(m_a));
	for (int i = 0; i < 10; i++)
	{
		save_item(NAME(m_d_in[i][0]), i);
		save_item(NAME(m_d_in[i][1]), i);
		save_item(NAME(m_d_out[i][0]), i);
		save_item(NAME(m_d_out[i][1]), i);
		save_item(NAME(m_d_out[i][2]), i);
		save_item(NAME(m_d_out[i][3]), i);
	}
	save_item(NAME(m_ra));
	save_item(NAME(m_wa));
	save_item(NAME(m_cdata_front));
	save_item(NAME(m_cdata_back));
	save_item(NAME(m_cdata));
	save_item(NAME(m_vdata_front));
	save_item(NAME(m_vdata_back));
	save_item(NAME(m_vdata));

	for (int i = 0; i < 10; i++)
	{
		m_stripes[i] = make_unique_clear<uint8_t[]>(0x10000);
		save_pointer(NAME(m_stripes[i]), 0x10000, i);
	}

	m_cdata_out.resolve_safe();
	m_vdata_out.resolve_safe();
	m_cbusy_out.resolve_safe();

	m_front_pal_base = m_front_pal->base();
	m_back_pal_base = m_back_pal->base();
}

void dpb7000_framestore_card_device::device_reset()
{
	m_vopen = false;
	m_copen = false;
	m_csel = false;
	m_rck = false;
	m_cck = false;

	memset(m_openx, 0, 5);
	m_opstr = false;
	m_opwa = false;
	m_opwb = false;
	m_opw = 0;
	m_opra = false;
	m_oprb = false;
	m_opr = 0;

	m_ras_in = false;
	memset(m_ras, 0, 10);
	m_cas = false;
	m_write = false;
	m_ipen = false;
	m_whp = false;
	m_lumen = false;
	m_bdsel = false;
	m_ipsel = 0;

	memset(m_ipenx, 0, 10);
	memset(m_rav, 0, 10);
	m_cav = 0;

	m_front_pal_addr = 0;
	m_front_pal_out = 0;
	m_back_pal_addr = 0;
	m_back_pal_out = 0;

	m_a = 0;
	m_ra = 0;
	m_wa = 0;
	m_cdata_front = 0;
	m_cdata_back = 0;
	m_cdata = 0;
	m_vdata_front = 0;
	m_vdata_back = 0;
	m_vdata = 0;

	for (int i = 0; i < 10; i++)
	{
		memset(&m_stripes[i][0], 0, 0x10000);
	}
}

void dpb7000_framestore_card_device::update_vdata_latches(const uint8_t stripe)
{
	for (int i = 0; i < 5; i++)
	{
		if (m_openx[i])
		{
			m_vdata_front = m_d_out[ i << 1     ][m_opr];
			m_vdata_back  = m_d_out[(i << 1) | 1][m_opr];
		}
	}
}

void dpb7000_framestore_card_device::update_cdata_latches(const uint8_t stripe)
{
	for (int i = 0; i < 5; i++)
	{
		if (m_openx[i])
		{
			m_cdata_front = m_d_out[ i << 1     ][m_opr];
			m_cdata_back  = m_d_out[(i << 1) | 1][m_opr];
		}
	}
}

void dpb7000_framestore_card_device::update_cdata_out()
{
	if (m_copen)
	{
		m_cdata_out(m_csel ? m_cdata_back : m_cdata_front);
	}
}

void dpb7000_framestore_card_device::update_vdata_out()
{
	if (m_vopen)
	{
		m_vdata_out(BIT(m_ra, 0) ? m_vdata_back : m_vdata_front);
	}
}

void dpb7000_framestore_card_device::update_pals()
{
	const uint8_t old_front_addr = m_front_pal_addr;
	const uint8_t old_back_addr = m_back_pal_addr;
	const uint8_t old_front_ras = m_front_pal_out & 0x1f;
	const uint8_t old_back_ras = m_back_pal_out & 0x1f;

	do
	{
		m_front_pal_out = m_front_pal_base[m_front_pal_addr];
		m_back_pal_out = m_back_pal_base[m_back_pal_addr];

		m_front_pal_addr &= 0x0ff;
		m_front_pal_addr |= BIT(m_back_pal_out, 6) ? 0x100 : 0x000;
		m_front_pal_addr |= BIT(m_back_pal_out, 5) ? 0x200 : 0x000;
		m_back_pal_addr  &= 0x0ff;
		m_back_pal_addr  |= BIT(m_front_pal_out, 6) ? 0x100 : 0x000;
		m_back_pal_addr  |= BIT(m_front_pal_out, 5) ? 0x200 : 0x000;
	} while (old_front_addr != m_front_pal_addr || old_back_addr != m_back_pal_addr);

	if (old_front_ras != (m_front_pal_out & 0x1f) || old_back_ras != (m_back_pal_out & 0x1f))
	{
		m_ras[0] = BIT(m_front_pal_out, 4);
		m_ras[2] = BIT(m_front_pal_out, 3);
		m_ras[4] = BIT(m_front_pal_out, 2);
		m_ras[6] = BIT(m_front_pal_out, 1);
		m_ras[8] = BIT(m_front_pal_out, 0);
		m_ras[1] = BIT(m_back_pal_out, 4);
		m_ras[3] = BIT(m_back_pal_out, 3);
		m_ras[5] = BIT(m_back_pal_out, 2);
		m_ras[7] = BIT(m_back_pal_out, 1);
		m_ras[9] = BIT(m_back_pal_out, 0);

		for (int i = 0; i < 10; i++)
		{
			if (!m_ras[i])
			{
				m_rav[i] = m_a;
			}
		}
	}
}

void dpb7000_framestore_card_device::vopen_w(int state)
{
	const bool old = m_vopen;
	m_vopen = (bool)state;
	if (!old && m_vopen)
	{
		update_vdata_out();
	}
}

void dpb7000_framestore_card_device::copen_w(int state)
{
	const bool old = m_copen;
	m_copen = (bool)state;
	if (!old && m_copen)
	{
		update_cdata_out();
	}
}

void dpb7000_framestore_card_device::csel_w(int state)
{
	const bool old = m_csel;
	m_csel = (bool)state;
	if (old != m_csel)
	{
		update_cdata_out();
	}
}

void dpb7000_framestore_card_device::rck_w(int state)
{
	const bool old = m_rck;
	m_rck = (bool)state;
	if (old != m_rck)
	{
		update_vdata_latches(m_ra & 0xe);
	}
}

void dpb7000_framestore_card_device::cck_w(int state)
{
	const bool old = m_cck;
	m_cck = (bool)state;
	if (old != m_cck)
	{
		update_cdata_latches(m_ra & 0xe);
	}
}


void dpb7000_framestore_card_device::opstr_w(int state)
{
	m_opstr = (bool)state;
}

void dpb7000_framestore_card_device::opwa_w(int state)
{
	m_opw &= 2;
	m_opw |= state;
}

void dpb7000_framestore_card_device::opwb_w(int state)
{
	m_opw &= 1;
	m_opw |= state << 1;
}

void dpb7000_framestore_card_device::opra_w(int state)
{
	m_opr &= 2;
	m_opr |= state;
}

void dpb7000_framestore_card_device::oprb_w(int state)
{
	m_opr &= 1;
	m_opr |= state << 1;
}


void dpb7000_framestore_card_device::ras_w(int state)
{
	const bool old = m_ras_in;
	m_ras_in = (bool)state;
	if (old != m_ras_in)
	{
		m_front_pal_addr &= ~0x001;
		m_back_pal_addr &= ~0x001;
		const uint16_t value = m_ras_in ? 0x001 : 0x000;
		m_front_pal_addr |= value;
		m_back_pal_addr |= value;

		update_pals();
	}
}

void dpb7000_framestore_card_device::cas_w(int state)
{
	const bool old = m_cas;
	m_cas = (bool)state;
	if (old && !m_cas)
	{
		m_cav = m_a;

		for (int i = 0; i < 10; i++)
		{
			if (m_ipenx[i])
			{
				m_stripes[i][(m_cav << 8) | m_rav[i]] = m_d_in[i][m_ipsel];
			}
			if (!m_opstr)
			{
				m_d_out[i][m_opw] = m_stripes[i][(m_cav << 8) | m_rav[i]];
			}
		}
	}
}

void dpb7000_framestore_card_device::write_w(int state)
{
	const bool old = m_write;
	m_write = (bool)state;
	if (old != m_write)
	{
		m_front_pal_addr &= ~0x002;
		m_back_pal_addr &= ~0x002;
		const uint16_t value = m_write ? 0x000 : 0x002;
		m_front_pal_addr |= value;
		m_back_pal_addr |= value;

		update_pals();
	}
}

void dpb7000_framestore_card_device::ipen_w(int state)
{
	const bool old = m_ipen;
	m_ipen = (bool)state;
	if (old != m_ipen)
	{
		m_front_pal_addr &= ~0x008;
		m_back_pal_addr &= ~0x008;
		const uint16_t value = m_ipen ? 0x008 : 0x000;
		m_front_pal_addr |= value;
		m_back_pal_addr |= value;

		update_pals();
	}
}

void dpb7000_framestore_card_device::whp_w(int state)
{
	const bool old = m_whp;
	m_whp = (bool)state;
	if (old != m_whp)
	{
		m_front_pal_addr &= ~0x004;
		m_back_pal_addr &= ~0x004;
		const uint16_t value = m_whp ? 0x004 : 0x000;
		m_front_pal_addr |= value;
		m_back_pal_addr |= value;

		update_pals();
	}
}

void dpb7000_framestore_card_device::lumen_w(int state)
{
	const bool old = m_lumen;
	m_lumen = (bool)state;
	if (old != m_lumen)
	{
		m_front_pal_addr &= ~0x080;
		m_back_pal_addr &= ~0x080;
		const uint16_t value = m_lumen ? 0x080 : 0x000;
		m_front_pal_addr |= value;
		m_back_pal_addr |= value;

		update_pals();
	}
}

void dpb7000_framestore_card_device::bdsel_w(int state)
{
	const bool old = m_bdsel;
	m_bdsel = (bool)state;
	if (old != m_bdsel)
	{
		m_front_pal_addr &= ~0x040;
		m_back_pal_addr &= ~0x040;
		const uint16_t value = m_bdsel ? 0x040 : 0x000;
		m_front_pal_addr |= value;
		m_back_pal_addr |= value;

		update_pals();
	}
}

void dpb7000_framestore_card_device::ipsel_w(int state)
{
	m_ipsel = state;
}


void dpb7000_framestore_card_device::a_w(uint8_t data)
{
	m_a = data;
}

void dpb7000_framestore_card_device::d_w(uint8_t data)
{
	for (int i = 0; i < 10; i++)
	{
		if (m_ipenx[i])
		{
			m_d_in[i][1 - m_ipsel] = data;
		}
	}
}

void dpb7000_framestore_card_device::ra_w(uint8_t data)
{
	m_openx[m_ra >> 1] = false;
	m_ra = data & 0xf;
	m_openx[m_ra >> 1] = true;
}

void dpb7000_framestore_card_device::wa_w(uint8_t data)
{
	const uint8_t old = m_wa;
	m_wa = data & 0xf;
	if (old != m_wa)
	{
		m_ipenx[old] = false;
		m_ipenx[m_wa] = true;

		m_back_pal_addr  &= ~0x030;
		m_back_pal_addr  |= BIT(m_wa, 3) ? 0x010 : 0x000;
		m_back_pal_addr  |= BIT(m_wa, 2) ? 0x020 : 0x000;
		m_front_pal_addr &= ~0x030;
		m_front_pal_addr |= BIT(m_wa, 1) ? 0x010 : 0x000;
		m_front_pal_addr |= BIT(m_wa, 0) ? 0x020 : 0x000;

		update_pals();
	}
}

