// license:BSD-3-Clause
// copyright-holders:Mark McDougall, R. Belmont
/***************************************************************************

  namcond1.h

  Common functions & declarations for the Namco ND-1 driver

***************************************************************************/

#include "video/ygv608.h"
#include "machine/nvram.h"

class namcond1_state : public driver_device
{
public:
	namcond1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_ygv608(*this, "ygv608"),
		m_zpr1(*this, "zpr1"),
		m_zpr2(*this, "zpr2"),
		m_shared_ram(*this, "shared_ram") { }

	void abcheck(machine_config &config);
	void namcond1(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<ygv608_device> m_ygv608;
	optional_device<nvram_device> m_zpr1, m_zpr2;

	required_shared_ptr<uint16_t> m_shared_ram;

	uint8_t m_h8_irq5_enabled;
	int m_p8;

	DECLARE_READ16_MEMBER(mcu_p7_read);
	DECLARE_READ16_MEMBER(mcu_pa_read);
	DECLARE_WRITE16_MEMBER(mcu_pa_write);
	DECLARE_READ16_MEMBER(cuskey_r);
	DECLARE_WRITE16_MEMBER(cuskey_w);
	DECLARE_READ16_MEMBER(printer_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER( vblank_irq_w );
	DECLARE_WRITE_LINE_MEMBER( raster_irq_w );
	INTERRUPT_GEN_MEMBER(mcu_interrupt);
	void abcheck_map(address_map &map);
	void namcond1_map(address_map &map);
	void nd1h8iomap(address_map &map);
	void nd1h8rwmap(address_map &map);
};
