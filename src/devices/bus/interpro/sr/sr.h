// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_INTERPRO_SR_SR_H
#define MAME_BUS_INTERPRO_SR_SR_H

#pragma once

#define MCFG_CBUS_OUT_IRQ0_CB(_devcb) \
	devcb = &downcast<cbus_device &>(*device).set_out_irq0_callback(DEVCB_##_devcb);

#define MCFG_CBUS_OUT_IRQ1_CB(_devcb) \
	devcb = &downcast<cbus_device &>(*device).set_out_irq1_callback(DEVCB_##_devcb);

#define MCFG_CBUS_OUT_IRQ2_CB(_devcb) \
	devcb = &downcast<cbus_device &>(*device).set_out_irq2_callback(DEVCB_##_devcb);

#define MCFG_CBUS_OUT_VBLANK_CB(_devcb) \
	devcb = &downcast<cbus_device &>(*device).set_out_vblank_callback(DEVCB_##_devcb);

#define MCFG_CBUS_SLOT_ADD(_bus_tag, _slot_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_slot_tag, CBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed) \
	downcast<cbus_slot_device &>(*device).set_tags(_bus_tag, _slot_tag);

#define MCFG_CBUS_MEMORY(_tag, _main_spacenum, _io_spacenum) \
	downcast<cbus_device &>(*device).set_memory(_tag, _main_spacenum, _io_spacenum);


#define MCFG_SRX_OUT_IRQ0_CB(_devcb) \
	devcb = &downcast<srx_device &>(*device).set_out_irq0_callback(DEVCB_##_devcb);

#define MCFG_SRX_OUT_IRQ1_CB(_devcb) \
	devcb = &downcast<srx_device &>(*device).set_out_irq1_callback(DEVCB_##_devcb);

#define MCFG_SRX_OUT_IRQ2_CB(_devcb) \
	devcb = &downcast<srx_device &>(*device).set_out_irq2_callback(DEVCB_##_devcb);

#define MCFG_SRX_OUT_VBLANK_CB(_devcb) \
	devcb = &downcast<srx_device &>(*device).set_out_vblank_callback(DEVCB_##_devcb);

#define MCFG_SRX_SLOT_ADD(_bus_tag, _slot_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_slot_tag, SRX_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed) \
	downcast<srx_slot_device &>(*device).set_tags(_bus_tag, _slot_tag);

#define MCFG_SRX_MEMORY(_tag, _main_spacenum, _io_spacenum) \
	downcast<srx_device &>(*device).set_memory(_tag, _main_spacenum, _io_spacenum);


class cbus_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	cbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration
	void set_tags(const char *bus_tag, const char *slot_tag);

protected:
	cbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

	// configuration
	const char *m_bus_tag;
	const char *m_slot_tag;
};

class device_cbus_card_interface;

class cbus_device : public device_t
{
public:
	// construction/destruction
	cbus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration
	template <class Object> devcb_base &set_out_irq0_callback(Object &&cb) { return m_out_irq0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq1_callback(Object &&cb) { return m_out_irq1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq2_callback(Object &&cb) { return m_out_irq2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_vblank_callback(Object &&cb) { return m_out_vblank_cb.set_callback(std::forward<Object>(cb)); }

	void set_memory(const char *const tag, const int main_spacenum, const int io_spacenum);

	static const u32 CBUS_BASE = 0x87000000;
	static const u32 CBUS_SIZE = 0x08000000;
	static const int CBUS_COUNT = 16;

	DECLARE_WRITE_LINE_MEMBER(irq0_w) { m_out_irq0_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(irq1_w) { m_out_irq1_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(irq2_w) { m_out_irq2_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(vblank_w) { m_out_vblank_cb(state); }

	// installation function for card devices
	template <typename T> void install_card(T &device, void (T::*map)(address_map &map))
	{
		// record the device in the next free slot
		m_slot[m_slot_count] = &device;

		// compute slot base address
		offs_t start = CBUS_BASE + m_slot_count * CBUS_SIZE;
		offs_t end = start + (CBUS_SIZE - 1);

		// install the device address map
		m_main_space->install_device(start, end, device, map);
		m_io_space->install_device(start, end, device, map);

		m_slot_count++;
	}

protected:
	cbus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	address_space *m_main_space;
	address_space *m_io_space;

	devcb_write_line m_out_irq0_cb;
	devcb_write_line m_out_irq1_cb;
	devcb_write_line m_out_irq2_cb;
	devcb_write_line m_out_vblank_cb;

private:
	device_cbus_card_interface *m_slot[CBUS_COUNT];
	int m_slot_count;

	const char *m_memory_tag;
	int m_main_spacenum;
	int m_io_spacenum;
};

class device_cbus_card_interface : public device_slot_card_interface
{
public:
	void set_bus_device();

	// inline configuration
	void set_tags(const char *bus_tag, const char *slot_tag) { m_bus_tag = bus_tag; m_slot_tag = slot_tag; }

	DECLARE_WRITE_LINE_MEMBER(irq0) { m_bus->irq0_w(state); }
	DECLARE_WRITE_LINE_MEMBER(irq1) { m_bus->irq1_w(state); }
	DECLARE_WRITE_LINE_MEMBER(irq2) { m_bus->irq2_w(state); }
	DECLARE_WRITE_LINE_MEMBER(vblank) { m_bus->vblank_w(state); }

protected:
	device_cbus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void map(address_map &map) = 0;

	cbus_device  *m_bus;
	const char *m_bus_tag;
	const char *m_slot_tag;
};

class cbus_card_device_base : public device_cbus_card_interface
{
protected:
	cbus_card_device_base(const machine_config &mconfig, device_t &device, const char *idprom_region = "idprom");

public:
	const char *tag() { return device().tag(); }

protected:
	virtual void map(address_map &map) override;

private:
	const char *const m_idprom_region;
};

class srx_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	srx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration
	void set_tags(const char *bus_tag, const char *slot_tag);

protected:
	srx_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

	// configuration
	const char *m_bus_tag;
	const char *m_slot_tag;
};

class device_srx_card_interface;

class srx_device : public device_t
{
public:
	// construction/destruction
	srx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration
	template <class Object> devcb_base &set_out_irq0_callback(Object &&cb) { return m_out_irq0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq1_callback(Object &&cb) { return m_out_irq1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq2_callback(Object &&cb) { return m_out_irq2_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_vblank_callback(Object &&cb) { return m_out_vblank_cb.set_callback(std::forward<Object>(cb)); }

	void set_memory(const char *const tag, const int main_spacenum, const int io_spacenum);

	static const u32 SRX_BASE = 0x8f000000;
	static const u32 SRX_SIZE = 0x8000;
	static const int SRX_COUNT = 32;

	DECLARE_WRITE_LINE_MEMBER(irq0_w) { m_out_irq0_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(irq1_w) { m_out_irq1_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(irq2_w) { m_out_irq2_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(vblank_w) { m_out_vblank_cb(state); }

	// installation function for card devices
	template <typename T> void install_card(T &device, void (T::*map)(address_map &map))
	{
		// record the device in the next free slot
		m_slot[m_slot_count] = &device;

		// compute slot base address
		offs_t start = SRX_BASE + m_slot_count * SRX_SIZE;
		offs_t end = start + (SRX_SIZE - 1);

		// install the device address map
		m_main_space->install_device(start, end, device, map);
		m_io_space->install_device(start, end, device, map);

		m_slot_count++;
	}

	template <typename T> T *get_card()
	{
		for (auto device : m_slot)
			if (dynamic_cast<T *>(device) != nullptr)
				return dynamic_cast<T *>(device);

		return nullptr;
	}

	template <typename T> void install_map(T &device, offs_t start, offs_t end, void (T::*map)(address_map &map))
	{
		// install the device address map
		m_main_space->install_device(start, end, device, map);
		m_io_space->install_device(start, end, device, map);
	}

	address_space *main_space() { return m_main_space; }
	address_space *io_space() { return m_io_space; }

protected:
	srx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	address_space *m_main_space;
	address_space *m_io_space;

	devcb_write_line m_out_irq0_cb;
	devcb_write_line m_out_irq1_cb;
	devcb_write_line m_out_irq2_cb;
	devcb_write_line m_out_vblank_cb;

private:
	device_srx_card_interface *m_slot[SRX_COUNT];
	int m_slot_count;

	const char *m_memory_tag;
	int m_main_spacenum;
	int m_io_spacenum;
};

class device_srx_card_interface : public device_slot_card_interface
{
public:
	void set_bus_device();

	// inline configuration
	void set_tags(const char *bus_tag, const char *slot_tag) { m_bus_tag = bus_tag; m_slot_tag = slot_tag; }

	DECLARE_WRITE_LINE_MEMBER(irq0) { m_bus->irq0_w(state); }
	DECLARE_WRITE_LINE_MEMBER(irq1) { m_bus->irq1_w(state); }
	DECLARE_WRITE_LINE_MEMBER(irq2) { m_bus->irq2_w(state); }
	DECLARE_WRITE_LINE_MEMBER(vblank) { m_bus->vblank_w(state); }

protected:
	device_srx_card_interface(const machine_config &mconfig, device_t &device);

	virtual void map(address_map &map) = 0;

	srx_device  *m_bus;
	const char *m_bus_tag;
	const char *m_slot_tag;
};

class srx_card_device_base : public device_srx_card_interface
{
protected:
	srx_card_device_base(const machine_config &mconfig, device_t &device, const char *idprom_region = "idprom");

public:
	const char *tag() { return device().tag(); }

protected:
	virtual void map(address_map &map) override;

private:
	const char *const m_idprom_region;
};

DECLARE_DEVICE_TYPE(CBUS, cbus_device)
DECLARE_DEVICE_TYPE(CBUS_SLOT, cbus_slot_device)
DECLARE_DEVICE_TYPE(SRX, srx_device)
DECLARE_DEVICE_TYPE(SRX_SLOT, srx_slot_device)

#endif // MAME_BUS_INTERPRO_SR_SR_H
