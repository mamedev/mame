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
#include "video/v9938.h"

extern const device_type GENEVE_MOUSE;
extern const device_type GENEVE_KEYBOARD;
extern const device_type GENEVE_MAPPER;

/*****************************************************************************/

class geneve_mouse_device : public device_t
{
public:
	geneve_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void poll();
	line_state	left_button();	// left button is not connected to the V9938 but to a TMS9901 pin

protected:
	void device_start();
	void device_reset();
	ioport_constructor device_input_ports() const;

private:
	v9938_device*	m_v9938;
	int				m_last_mx;
	int				m_last_my;
};

#define MCFG_GENEVE_MOUSE_ADD(_tag )	\
	MCFG_DEVICE_ADD(_tag, GENEVE_MOUSE, 0)

/*****************************************************************************/

struct geneve_keyboard_config 
{
	devcb_write_line	interrupt;
};

#define GENEVE_KEYBOARD_CONFIG(name) \
	const geneve_keyboard_config(name) =


#define KEYQUEUESIZE 256
#define MAXKEYMSGLENGTH 10
#define KEYAUTOREPEATDELAY 30
#define KEYAUTOREPEATRATE 6

class geneve_keyboard_device : public device_t
{
public:
	geneve_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_WRITE_LINE_MEMBER( reset_line );
	DECLARE_WRITE_LINE_MEMBER( send_scancodes );
	DECLARE_WRITE_LINE_MEMBER( clock_control );
	UINT8 get_recent_key();

protected:
	void						device_start();
	void						device_reset();
	void						device_config_complete();
	ioport_constructor			device_input_ports() const;
	devcb_resolved_write_line	m_interrupt;	// Keyboard interrupt to console
	void						device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	void	post_in_key_queue(int keycode);
	void	signal_when_key_available();
	void	poll();

	bool	m_key_reset;
	int		m_key_queue_length;
	UINT8	m_key_queue[KEYQUEUESIZE];
	int 	m_key_queue_head;
	bool	m_key_in_buffer;
	UINT32	m_key_state_save[4];
	bool	m_key_numlock_state;

	int 	m_key_ctrl_state;
	int 	m_key_alt_state;
	int 	m_key_real_shift_state;

	bool	m_key_fake_shift_state;
	bool	m_key_fake_unshift_state;

	int 	m_key_autorepeat_key;
	int 	m_key_autorepeat_timer;

	bool	m_keep_keybuf;
	bool	m_keyboard_clock;

	emu_timer*		m_timer;
};

#define MCFG_GENEVE_KEYBOARD_ADD(_tag, _intf )	\
	MCFG_DEVICE_ADD(_tag, GENEVE_KEYBOARD, 0)	\
	MCFG_DEVICE_CONFIG(_intf)

/*****************************************************************************/

struct geneve_mapper_config 
{
	devcb_write_line	ready;
};

class geneve_mapper_device : public device_t
{
public:
	geneve_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	inline void set_geneve_mode(bool geneve) { m_geneve_mode = geneve; }
	inline void set_direct_mode(bool direct) { m_direct_mode = direct; }
	inline void set_cartridge_size(int size) { m_cartridge_size = size; }
	inline void set_cartridge_writable(int base, bool write)
	{
		if (base==0x6000)  m_cartridge6_writable = write;
		else  m_cartridge7_writable = write;
	}
	inline void set_video_waitstates(bool wait) { m_video_waitstates = wait; }
	inline void set_extra_waitstates(bool wait) { m_extra_waitstates = wait; }

	void do_wait(int min);

	DECLARE_READ8_MEMBER( readm );
	DECLARE_WRITE8_MEMBER( writem );

	DECLARE_INPUT_CHANGED_MEMBER( gm_changed );

	void clock_in(int state);

protected:
	void	device_start();
	void	device_reset();

private:
	// GROM simulation
	bool	m_gromwaddr_LSB;
	bool	m_gromraddr_LSB;
	int		m_grom_address;
	DECLARE_READ8_MEMBER( read_grom );
	DECLARE_WRITE8_MEMBER( write_grom );

	// wait states
	bool		m_video_waiting;
	bool		m_video_waitstates;
	bool		m_extra_waitstates;

	// Mapper function
	bool	m_geneve_mode;
	bool	m_direct_mode;
	int		m_cartridge_size;
	bool	m_cartridge_secondpage;
	bool	m_cartridge6_writable;
	bool	m_cartridge7_writable;
	int		m_map[8];

	// Genmod modifications
	bool	m_turbo;
	bool	m_genmod;
	bool	m_timode;

	int		m_sram_mask;
	int		m_sram_val;

	// Ready line to the CPU
	devcb_resolved_write_line m_ready;

	// Counter for the wait states.
	int   m_waitcount;

	// Devices
	device_t*				m_clock;
	geneve_keyboard_device*	m_keyboard;
	bus8z_device*			m_video;
	bus8z_device*			m_peribox;
	bus8z_device*			m_sound;
	UINT8*					m_eprom;
	UINT8*					m_sram;
	UINT8*					m_dram;
};

#define GENEVE_MAPPER_CONFIG(name) \
	const geneve_mapper_config(name) =

#define MCFG_GENEVE_MAPPER_ADD(_tag, _conf )	\
	MCFG_DEVICE_ADD(_tag, GENEVE_MAPPER, 0)	\
	MCFG_DEVICE_CONFIG( _conf )

#endif
