// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**************************************************************************************************

    Sega Dreamcast G2 System Bus I/F

**************************************************************************************************/

#ifndef MAME_SEGA_DC_G2IF_H
#define MAME_SEGA_DC_G2IF_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dc_g2if_device : public device_t
{
public:
	// construction/destruction
	dc_g2if_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> void set_host_space(T &&tag, int index) { m_host_space.set_tag(std::forward<T>(tag), index); }
	auto int_cb() { return m_int_w.bind(); }
	auto error_ia_cb() { return m_error_ia_w.bind(); }
	auto error_ov_cb() { return m_error_ov_w.bind(); }

	void amap(address_map &map) ATTR_COLD;

	void hw_irq_trigger_hs(u32 normal_ist, u32 ext_ist);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	//virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(dma_end_tick);

private:
	required_address_space m_host_space;
	devcb_write8 m_int_w;
	devcb_write8 m_error_ia_w;
	devcb_write8 m_error_ov_w;

	struct {
		u32 g2_addr;
		u32 root_addr;
		u32 len;
		u32 size;
		bool mode;
		bool dir;
		bool enable;
		bool in_progress;
		bool start;
		u8 tsel;
		bool hw_trigger;
		emu_timer *end_timer;
	} m_dma[4];

	struct {
		u32 bottom_addr;
		u32 top_addr;
	} m_g2apro;

	template <u8 Channel> void channel_map(address_map &map) ATTR_COLD;

	template <u8 Channel> u32 stag_r();
	template <u8 Channel> void stag_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 Channel> u32 star_r();
	template <u8 Channel> void star_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 Channel> u32 len_r();
	template <u8 Channel> void len_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 Channel> u32 dir_r();
	template <u8 Channel> void dir_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 Channel> u32 tsel_r();
	template <u8 Channel> void tsel_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 Channel> u32 en_r();
	template <u8 Channel> void en_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 Channel> u32 st_r();
	template <u8 Channel> void st_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 Channel> u32 susp_r();
	template <u8 Channel> void susp_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void dma_execute(u8 channel);
	bool root_address_check(u32 offset);
	bool g2_address_check(u32 offset);
	bool root_overflow_check(u32 offset, u8 channel);

	u32 g2id_r();
	void g2apro_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};


// device type definition
DECLARE_DEVICE_TYPE(DC_G2IF, dc_g2if_device)

#endif // MAME_SEGA_DC_G2IF_H
