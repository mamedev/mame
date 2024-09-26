// license:LGPL-2.1+
// copyright-holders:R. Belmont, Brad Martin
/*****************************************************************************
 *
 * Nintendo/Sony S-SMP emulation
 *
 ****************************************************************************/

#ifndef MAME_MACHINE_S_SMP_H
#define MAME_MACHINE_S_SMP_H

#include "cpu/spc700/spc700.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

class s_smp_device : public spc700_device
{
public:
	s_smp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto dsp_io_read_callback() { return m_dsp_io_r_cb.bind(); }
	auto dsp_io_write_callback() { return m_dsp_io_w_cb.bind(); }

	u8 spc_port_out_r(offs_t offset);
	void spc_port_in_w(offs_t offset, u8 data);

protected:
	tiny_rom_entry const *device_rom_region() const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	TIMER_CALLBACK_MEMBER(update_timers);

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_dcache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	inline u8 data_read_byte(offs_t a)
	{
		/* IPL ROM enabled */
		if (a >= 0xffc0 && m_ctrl & 0x80)
			return m_ipl_region[a & 0x3f];

		return m_dcache.read_byte(a);
	}
	inline void data_write_byte(offs_t a, u8 d) { m_data.write_byte(a, d); }

	required_region_ptr<u8> m_ipl_region;     /* SPC top 64 bytes */

	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);

	/* timers */
	emu_timer             *m_tick_timer;
	bool                  m_timer_enabled[3];
	u16                   m_counter[3];
	u8                    m_subcounter[3];
	inline void update_timer_tick(u8 which);

	/* IO ports */
	u8                    m_port_in[4];         /* SPC input ports */
	u8                    m_port_out[4];        /* SPC output ports */

	u16                   m_TnDIV[3]; /**< Timer N Divider */

	// registers
	u8                    m_test;
	u8                    m_ctrl;
	u8                    m_counter_reg[3];

	devcb_read8           m_dsp_io_r_cb;
	devcb_write8          m_dsp_io_w_cb;

	void internal_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(S_SMP, s_smp_device)


#endif // MAME_MACHINE_S_SMP_H
