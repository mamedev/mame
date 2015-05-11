// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cococart.h

    CoCo/Dragon cartridge management

*********************************************************************/

#ifndef __COCOCART_H__
#define __COCOCART_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* TIMER_POOL: Must be power of two */
#define TIMER_POOL         2

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* output lines on the CoCo cartridge slot */
enum cococart_line
{
	COCOCART_LINE_CART,             /* connects to PIA1 CB1 */
	COCOCART_LINE_NMI,              /* connects to NMI line on CPU */
	COCOCART_LINE_HALT,             /* connects to HALT line on CPU */
	COCOCART_LINE_SOUND_ENABLE      /* sound enable */
};

/* since we have a special value "Q" - we have to use a special enum here */
enum cococart_line_value
{
	COCOCART_LINE_VALUE_CLEAR,
	COCOCART_LINE_VALUE_ASSERT,
	COCOCART_LINE_VALUE_Q
};

struct coco_cartridge_line
{
	emu_timer                   *timer[TIMER_POOL];
	int                         timer_index;
	int                         delay;
	cococart_line_value         value;
	int                         line;
	int                         q_count;
	devcb_write_line        *callback;
};

// ======================> cococart_base_update_delegate

// direct region update handler
typedef delegate<void (UINT8 *)> cococart_base_update_delegate;

#define MCFG_COCO_CARTRIDGE_CART_CB(_devcb) \
	devcb = &cococart_slot_device::static_set_cart_callback(*device, DEVCB_##_devcb);

#define MCFG_COCO_CARTRIDGE_NMI_CB(_devcb) \
	devcb = &cococart_slot_device::static_set_nmi_callback(*device, DEVCB_##_devcb);

#define MCFG_COCO_CARTRIDGE_HALT_CB(_devcb) \
	devcb = &cococart_slot_device::static_set_halt_callback(*device, DEVCB_##_devcb);


// ======================> cococart_slot_device
class device_cococart_interface;

class cococart_slot_device : public device_t,
								public device_slot_interface,
								public device_image_interface
{
public:
	// construction/destruction
	cococart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &static_set_cart_callback(device_t &device, _Object object)  { return downcast<cococart_slot_device &>(device).m_cart_callback.set_callback(object); }
	template<class _Object> static devcb_base &static_set_nmi_callback(device_t &device, _Object object)  { return downcast<cococart_slot_device &>(device).m_nmi_callback.set_callback(object); }
	template<class _Object> static devcb_base &static_set_halt_callback(device_t &device, _Object object)  { return downcast<cococart_slot_device &>(device).m_halt_callback.set_callback(object); }

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "coco_cart"; }
	virtual const char *file_extensions() const { return "ccc,rom"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	// reading and writing to $FF40-$FF7F
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	// sets a cartridge line
	void cart_set_line(cococart_line line, cococart_line_value value);

	// hack to support twiddling the Q line
	void twiddle_q_lines();

	// cart base
	UINT8* get_cart_base();
	void set_cart_base_update(cococart_base_update_delegate update);

private:
	enum
	{
		TIMER_CART,
		TIMER_NMI,
		TIMER_HALT
	};

	// configuration
	coco_cartridge_line         m_cart_line;
	coco_cartridge_line         m_nmi_line;
	coco_cartridge_line         m_halt_line;
public:
	devcb_write_line        m_cart_callback;
	devcb_write_line            m_nmi_callback;
	devcb_write_line            m_halt_callback;
private:
	// cartridge
	device_cococart_interface   *m_cart;

	// methods
	void set_line(const char *line_name, coco_cartridge_line &line, cococart_line_value value);
	void set_line_timer(coco_cartridge_line &line, cococart_line_value value);
	void twiddle_line_if_q(coco_cartridge_line &line);
};

// device type definition
extern const device_type COCOCART_SLOT;

// ======================> device_cococart_interface

class device_cococart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_cococart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_cococart_interface();

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

	virtual UINT8* get_cart_base();
	void set_cart_base_update(cococart_base_update_delegate update);

protected:
	void cart_base_changed(void);

private:
	cococart_base_update_delegate m_update;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_COCO_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, COCOCART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_COCO_CARTRIDGE_REMOVE(_tag)        \
	MCFG_DEVICE_REMOVE(_tag)

#endif /* __COCOCART_H__ */
