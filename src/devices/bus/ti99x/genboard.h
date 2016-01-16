// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Geneve main board components.
    See genboard.c for documentation.

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/
#ifndef __GENBOARD__
#define __GENBOARD__

#include "emu.h"
#include "ti99defs.h"
#include "machine/mm58274c.h"
#include "video/v9938.h"
#include "cpu/tms9900/tms9995.h"
#include "machine/at29x.h"

extern const device_type GENEVE_MOUSE;
extern const device_type GENEVE_KEYBOARD;
extern const device_type GENEVE_MAPPER;

/*****************************************************************************/

class geneve_mouse_device : public device_t
{
public:
	geneve_mouse_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	void poll();
	line_state  left_button();  // left button is not connected to the V9938 but to a TMS9901 pin

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

private:
	v9938_device*   m_v9938;
	int             m_last_mx;
	int             m_last_my;
};

#define MCFG_GENEVE_MOUSE_ADD(_tag )    \
	MCFG_DEVICE_ADD(_tag, GENEVE_MOUSE, 0)

/*****************************************************************************/

#define KEYQUEUESIZE 256
#define MAXKEYMSGLENGTH 10
#define KEYAUTOREPEATDELAY 30
#define KEYAUTOREPEATRATE 6

class geneve_keyboard_device : public device_t
{
public:
	geneve_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	DECLARE_WRITE_LINE_MEMBER( reset_line );
	DECLARE_WRITE_LINE_MEMBER( send_scancodes );
	DECLARE_WRITE_LINE_MEMBER( clock_control );
	UINT8 get_recent_key();

	template<class _Object> static devcb_base &static_set_int_callback(device_t &device, _Object object) { return downcast<geneve_keyboard_device &>(device).m_interrupt.set_callback(object); }

protected:
	void               device_start() override;
	void               device_reset() override;
	ioport_constructor device_input_ports() const override;
	devcb_write_line  m_interrupt;    // Keyboard interrupt to console
	void               device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void    post_in_key_queue(int keycode);
	void    signal_when_key_available();
	void    poll();

	bool    m_key_reset;
	int     m_key_queue_length;
	UINT8   m_key_queue[KEYQUEUESIZE];
	int     m_key_queue_head;
	bool    m_key_in_buffer;
	UINT32  m_key_state_save[4];
	bool    m_key_numlock_state;

	int     m_key_ctrl_state;
	int     m_key_alt_state;
	int     m_key_real_shift_state;

	bool    m_key_fake_shift_state;
	bool    m_key_fake_unshift_state;

	int     m_key_autorepeat_key;
	int     m_key_autorepeat_timer;

	bool    m_keep_keybuf;
	bool    m_keyboard_clock;

	emu_timer*      m_timer;
};

#define MCFG_GENEVE_KBINT_HANDLER( _intcallb ) \
	devcb = &geneve_keyboard_device::static_set_int_callback( *device, DEVCB_##_intcallb );

/*****************************************************************************/

class geneve_mapper_device : public device_t
{
public:
	geneve_mapper_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	void set_geneve_mode(bool geneve);
	void set_direct_mode(bool direct);
	void set_cartridge_size(int size);
	void set_cartridge_writable(int base, bool write);
	void set_video_waitstates(bool wait);
	void set_extra_waitstates(bool wait);

	DECLARE_READ8_MEMBER( readm );
	DECLARE_WRITE8_MEMBER( writem );
	DECLARE_SETOFFSET_MEMBER( setoffset );

	DECLARE_INPUT_CHANGED_MEMBER( settings_changed );

	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( dbin_in );

	// PFM support
	DECLARE_WRITE_LINE_MEMBER( pfm_select_lsb );
	DECLARE_WRITE_LINE_MEMBER( pfm_select_msb );
	DECLARE_WRITE_LINE_MEMBER( pfm_output_enable );

	template<class _Object> static devcb_base &static_set_ready_callback(device_t &device, _Object object) {  return downcast<geneve_mapper_device &>(device).m_ready.set_callback(object); }

protected:
	void    device_start() override;
	void    device_reset() override;

private:
	// GROM simulation
	bool    m_gromwaddr_LSB;
	bool    m_gromraddr_LSB;
	int     m_grom_address;
	DECLARE_READ8_MEMBER( read_grom );
	DECLARE_WRITE8_MEMBER( write_grom );

	// wait states
	void        set_wait(int min);
	void        set_ext_wait(int min);
	bool        m_video_waitstates;
	bool        m_extra_waitstates;
	bool        m_ready_asserted;

	bool        m_read_mode;

	bool        m_debug_no_ws;

	// Mapper function
	typedef struct
	{
		int     function;
		offs_t  offset;
		offs_t  physaddr;
	} decdata;

	bool    m_geneve_mode;
	bool    m_direct_mode;
	int     m_cartridge_size;
	bool    m_cartridge_secondpage;
	bool    m_cartridge6_writable;
	bool    m_cartridge7_writable;
	int     m_map[8];

	void    decode(address_space& space, offs_t offset, bool read_mode, decdata* dec);
	decdata m_decoded;

	// Genmod modifications
	bool    m_turbo;
	bool    m_genmod;
	bool    m_timode;

	// PFM mod (0 = none, 1 = AT29C040, 2 = AT29C040A)
	DECLARE_READ8_MEMBER( read_from_pfm );
	DECLARE_WRITE8_MEMBER( write_to_pfm );
	void    set_boot_rom(int selection);
	int     m_pfm_mode;
	int     m_pfm_bank;
	bool    m_pfm_output_enable;

	// SRAM access
	int     m_sram_mask;
	int     m_sram_val;

	// Ready line to the CPU
	devcb_write_line m_ready;

	// Counter for the wait states.
	int     m_waitcount;
	int     m_ext_waitcount;

	// Devices
	mm58274c_device*        m_clock;
	tms9995_device*         m_cpu;
	at29c040_device*         m_pfm512;
	at29c040a_device*        m_pfm512a;

	geneve_keyboard_device* m_keyboard;
	bus8z_device*           m_video;
	bus8z_device*           m_peribox;
	bus8z_device*           m_sound;
	UINT8*                  m_eprom;
	UINT8*                  m_sram;
	UINT8*                  m_dram;
};

#define MCFG_GENEVE_READY_HANDLER( _intcallb ) \
	devcb = &geneve_mapper_device::static_set_ready_callback( *device, DEVCB_##_intcallb );

#endif
