// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    kc.h

    KC85_2/3/4/5 expansion slot emulation

*********************************************************************/

#ifndef __KCEXP_H__
#define __KCEXP_H__

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> device_kcexp_interface

class device_kcexp_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_kcexp_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_kcexp_interface();

	// reading and writing
	virtual UINT8 module_id_r() { return 0xff; }
	virtual void control_w(UINT8 data) { }
	virtual void read(offs_t offset, UINT8 &data) { }
	virtual void write(offs_t offset, UINT8 data) { }
	virtual void io_read(offs_t offset, UINT8 &data) { }
	virtual void io_write(offs_t offset, UINT8 data) { }
	virtual UINT8* get_cart_base() { return nullptr; }
	virtual DECLARE_WRITE_LINE_MEMBER( mei_w ) { };
};

// ======================> kcexp_slot_device

class kcexp_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	kcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	kcexp_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~kcexp_slot_device();

	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<kcexp_slot_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_nmi_callback(device_t &device, _Object object) { return downcast<kcexp_slot_device &>(device).m_out_nmi_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_halt_callback(device_t &device, _Object object) { return downcast<kcexp_slot_device &>(device).m_out_halt_cb.set_callback(object); }

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void static_set_next_slot(device_t &device, const char *next_module_tag);

	// reading and writing
	virtual UINT8 module_id_r();
	virtual void control_w(UINT8 data);
	virtual void read(offs_t offset, UINT8 &data);
	virtual void write(offs_t offset, UINT8 data);
	virtual void io_read(offs_t offset, UINT8 &data);
	virtual void io_write(offs_t offset, UINT8 data);
	virtual DECLARE_WRITE_LINE_MEMBER( mei_w );
	virtual DECLARE_WRITE_LINE_MEMBER( meo_w );

	devcb_write_line                m_out_irq_cb;
	devcb_write_line                m_out_nmi_cb;
	devcb_write_line                m_out_halt_cb;

	device_kcexp_interface*     m_cart;

	const char*                 m_next_slot_tag;
	kcexp_slot_device*          m_next_slot;
};

// ======================> kccart_slot_device

class kccart_slot_device : public kcexp_slot_device,
							public device_image_interface
{
public:
	// construction/destruction
	kccart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~kccart_slot_device();

	// device-level overrides
	virtual void device_config_complete() override;

	// image-level overrides
	virtual bool call_load() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "kc_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result) override;
};

// device type definition
extern const device_type KCEXP_SLOT;
extern const device_type KCCART_SLOT;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_KCEXP_SLOT_OUT_IRQ_CB(_devcb) \
	devcb = &kcexp_slot_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_KCEXP_SLOT_OUT_NMI_CB(_devcb) \
	devcb = &kcexp_slot_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_KCEXP_SLOT_OUT_HALT_CB(_devcb) \
	devcb = &kcexp_slot_device::set_out_halt_callback(*device, DEVCB_##_devcb);

#define MCFG_KCEXP_SLOT_NEXT_SLOT(_next_slot_tag) \
	kcexp_slot_device::static_set_next_slot(*device, _next_slot_tag);


#define MCFG_KCCART_SLOT_OUT_IRQ_CB(_devcb) \
	devcb = &kccart_slot_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_KCCART_SLOT_OUT_NMI_CB(_devcb) \
	devcb = &kccart_slot_device::set_out_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_KCCART_SLOT_OUT_HALT_CB(_devcb) \
	devcb = &kccart_slot_device::set_out_halt_callback(*device, DEVCB_##_devcb);

#define MCFG_KCCART_SLOT_NEXT_SLOT(_next_slot_tag) \
	kccart_slot_device::static_set_next_slot(*device, _next_slot_tag);

// #define MCFG_KC85_EXPANSION_ADD(_tag,_next_slot_tag,_config,_slot_intf,_def_slot)
//  MCFG_DEVICE_ADD(_tag, KCEXP_SLOT, 0)
//  MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

// #define MCFG_KC85_CARTRIDGE_ADD(_tag,_next_slot_tag,_config,_slot_intf,_def_slot)
//  MCFG_DEVICE_ADD(_tag, KCCART_SLOT, 0)
//  MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#endif /* __KCEXP_H__ */
