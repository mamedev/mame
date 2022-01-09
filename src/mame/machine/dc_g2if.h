// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**************************************************************************************************

	Sega Dreamcast G2 System Bus I/F

**************************************************************************************************/

#ifndef MAME_MACHINE_DC_G2IF_H
#define MAME_MACHINE_DC_G2IF_H

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

	void amap(address_map &map);

	void hw_irq_trigger_hs(u32 normal_ist, u32 ext_ist);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_address_space m_host_space;
	devcb_write8 m_int_w;

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

	template <u8 ch> u32 stag_r();
	template <u8 ch> void stag_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 ch> u32 star_r();
	template <u8 ch> void star_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 ch> u32 len_r();
	template <u8 ch> void len_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 ch> u32 dir_r();
	template <u8 ch> void dir_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 ch> u32 tsel_r();
	template <u8 ch> void tsel_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 ch> u32 en_r();
	template <u8 ch> void en_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	
	template <u8 ch> u32 st_r();
	template <u8 ch> void st_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <u8 ch> u32 susp_r();
	template <u8 ch> void susp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	
	void dma_execute(u8 channel);
};


// device type definition
DECLARE_DEVICE_TYPE(DC_G2IF, dc_g2if_device)

#endif // MAME_MACHINE_DC_G2IF_H
