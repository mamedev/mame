// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**********************************************************************

                            <B>          <A>
                                +------+
                            GND |[ 01 ]| GND
                             V1 |[ 02 ]| V1
                             V2 |[ 03 ]| V2
                          DB001 |[ 04 ]| AB001
                          DB011 |[ 05 ]| AB011
                          DB021 |[ 06 ]| AB021
                          DB031 |[ 07 ]| AB031
                          DB041 |[ 08 ]| AB041
                          DB051 |[ 09 ]| AB051
                          DB061 |[ 10 ]| AB061
                            GND |[ 11 ]| GND
                          DB071 |[ 12 ]| AB071
                          DB081 |[ 13 ]| AB081
                          DB091 |[ 14 ]| AB091
                          DB101 |[ 15 ]| AB101
                          DB111 |[ 16 ]| AB111
                          DB121 |[ 17 ]| AB121
                          DB131 |[ 18 ]| AB131
                          DB141 |[ 19 ]| AB141
                          DB151 |[ 20 ]| AB151
                            GND |[ 21 ]| GND
                          +12 V |[ 22 ]| AB161
                          +12 V |[ 23 ]| AB171
            "INT0"         IR31 |[ 24 ]| AB181
            "INT1"         IR51 |[ 25 ]| AB191
            "INT2"         IR61 |[ 26 ]| AB201
            "INT3"         IR91 |[ 27 ]| AB211
            "INT4"  IR101/IR111 |[ 28 ]| AB221
            "INT5"        IR121 |[ 29 ]| AB231
            "INT6"        IR131 |[ 30 ]| INT0
                            GND |[ 31 ]| GND
                          -12 V |[ 32 ]| IOCHK0
                          -12 V |[ 33 ]| IOR0
                         RESET0 |[ 34 ]| IOW0
                         DACK00 |[ 35 ]| MRC0
                  DACK30/DACK20 |[ 36 ]| MWC0
                          DRQ00 |[ 37 ]| S00        (INTA0)
                    DRQ30/DRQ20 |[ 38 ]| S10        (NOWAIT0)
                          WORD0 |[ 39 ]| S20        (SALE1)
            (EXHRQ10)   CPKILL0 |[ 40 ]| LOCK       (MACS0)
                            GND |[ 41 ]| GND
            (EXHLA10)     RQGT0 |[ 42 ]| CPUENB10
                         DMATC0 |[ 43 ]| RFSH0
                           NMI0 |[ 44 ]| BHE0
                           MWE0 |[ 45 ]| IORDY1
            (EXHLA20)    HLDA00 |[ 46 ]| SCLK1
            (EXHRQ20)     HRQ00 |[ 47 ]| S18CLK1    = 307.2 kHz
            (SUBSRQ1)   DMAHLD0 |[ 48 ]| POWER0
                           +5 V |[ 49 ]| +5 V
                           +5 V |[ 50 ]| +5 V
                                +------+
                            <B>          <A>

**********************************************************************/

#ifndef MAME_BUS_PC98_CBUS_SLOT_H
#define MAME_BUS_PC98_CBUS_SLOT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_pc98_cbus_slot_interface;

class pc98_cbus_root_device : public device_t,
							  public device_memory_interface
{
public:
	pc98_cbus_root_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u16 mem_r(offs_t offset, u16 mem_mask = ~0);
	void mem_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 mem_slot_r(offs_t offset, u16 mem_mask = ~0);
	void mem_slot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 io_r(offs_t offset, u16 mem_mask = ~0);
	void io_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void remap(int space_id, offs_t start, offs_t end);

	template<typename T> void install_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(class address_map &map), uint64_t unitmask = ~u64(0))
	{
		space(AS_IO).install_device(addrstart, addrend, device, map, unitmask);
	}

	void add_slot(const char *tag);
	void add_slot(device_slot_interface *slot);

	// from C-bus to host
	template<std::size_t Line> auto int_cb() { return m_int_cb[Line].bind(); }
	template<std::size_t Line> auto drq_cb() { return m_drq_cb[Line].bind(); }

	u8 dack_r(int line);
	void dack_w(int line, u8 data);
	void eop_w(int line, int state);

	// from card to C-Bus
	void int_w(int Line, int state) { m_int_cb[Line](state); }
	void drq_w(int Line, int state) { m_drq_cb[Line](state); }

	void set_dma_channel(u8 channel, device_pc98_cbus_slot_interface *dev, bool do_eop);

protected:
	virtual space_config_vector memory_space_config() const override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;

	std::forward_list<device_slot_interface *> m_slot_list;
private:
	address_space_config m_space_mem_config;
	address_space_config m_space_io_config;

	devcb_write_line::array<8> m_int_cb;
	devcb_write_line::array<4> m_drq_cb;

	device_pc98_cbus_slot_interface *m_dma_device[8];
	bool                             m_dma_eop[8];

};


class device_pc98_cbus_slot_interface : public device_interface
{
	friend class pc98_cbus_root_device;

public:
	virtual ~device_pc98_cbus_slot_interface();

	virtual void remap(int space_id, offs_t start, offs_t end) {}
	virtual u8 dack_r(int line);
	virtual void dack_w(int line, u8 data);
	virtual void eop_w(int state);

	void set_bus(pc98_cbus_root_device *cbus_device) { m_bus = cbus_device; }

	device_pc98_cbus_slot_interface(const machine_config &mconfig, device_t &device);

	pc98_cbus_root_device     *m_bus;
};

class pc98_cbus_slot_device : public device_t
							, public device_slot_interface
							, public device_pc98_cbus_slot_interface
{
public:
	template <typename T, typename U>
	pc98_cbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cbus_tag, U &&opts, const char *dflt, bool fixed = false)
		: pc98_cbus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		m_bus.set_tag(std::forward<T>(cbus_tag));
	}
	pc98_cbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pc98_cbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<pc98_cbus_root_device> m_bus;
};


DECLARE_DEVICE_TYPE(PC98_CBUS_ROOT, pc98_cbus_root_device)
DECLARE_DEVICE_TYPE(PC98_CBUS_SLOT, pc98_cbus_slot_device)

#endif // MAME_BUS_PC98_CBUS_SLOT_H
