// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Short Description:

        Systems which run on the SPG243 SoC

        die markings show
        "SunPlus QL8041" ( known as Sunplus SPG240? )
        JAKKS WWE
        Fantastic 4
        Justice League
        Dora the Explorer
        Mattel Classic Sports
        Disney Princess (GKR)
        Wheel of Fortune (GKR)
        (all GameKeyReady units?)

        "SunPlus QL8041C" ( known as Sunplus SPG2?? )
        Clickstart ( see clickstart.cpp instead)
        Wheel of Fortune 2nd Edition


        "SunPlus PA7801" ( known as Sunplus SPG110? ) see spg110.cpp instead
        Classic Arcade Pinball
        EA Sports (NHL95 + Madden 95)

        It is unknown if the following are close to this architecture or not (no dumps yet)

        "SunPlus QU7073-P69A"
        Mortal Kombat

        "Sunplus QL8167"
        Disney Princess (older)
        Go Diego Go


Disney Princess non-GKR is Sunplus QL8167.

    Status:

        Mostly working

    To-Do:

        Proper driver_device inheritance to untangle the mess of members

    Detailed list of bugs:

        All systems:
            Various inaccuracies in samples/envelopes.

        walle:
            Game seems unhappy with NVRAM, clears contents on each boot.
        jak_pooh:
            In the 'Light Tag' minigame (select the rock) you can't move left with the DRC (ok with -nodrc)
            and the game usually softlocks when you find a friend (with or without DRC)

        vii:
            When loading a cart from file manager, sometimes MAME will crash.
            The "MOTOR" option in the diagnostic menu does nothing when selected.
            The "SPEECH IC" option in the diagnostic menu does nothing when selected.
            On 'vii_vc1' & 'vii_vc2' cart, the left-right keys are transposed with the up-down keys.
            - This is not a bug per se, as the games are played with the controller physically rotated 90 degrees.

    Note:
        Cricket, Skateboarder, Skannerz and Football 2 list a 32-bit checksum at the start of ROM.
        This is the byte sum of the file, excluding the first 16 byte (where the checksum is stored)

        Test Modes:
        Justice League : press UP, DOWN, LEFT, BT3 on the JAKKS logo in that order, quickly, to get test menu
        WWE : press UP, BT1, BT2 together during startup logos

        Disney Friends, MS Pacman, WallE, Batman (and some other HotGen GameKKeys) for test mode, hold UP,
        press A, press DOWN during startup

    TODO:
        Work out how to access the hidden TEST menus for all games (most JAKKS games should have one at least)

    Also on this hardware:

        name                        PCB ID      ROM width   TSOP pads   ROM size        SEEPROM         die markings
        Dream Life                  ?           x16         48          not dumped      no              Sunplus

*******************************************************************************/

#include "emu.h"

#include "cpu/unsp/unsp.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "machine/spg2xx.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/jakks_gamekey/slot.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class spg2xx_game_state : public driver_device
{
public:
	spg2xx_game_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_spg(*this, "spg")
		, m_bank(*this, "cartbank")
		, m_io_p1(*this, "P1")
		, m_io_p2(*this, "P2")
		, m_io_p3(*this, "P3")
		, m_i2cmem(*this, "i2cmem")
		, m_nvram(*this, "nvram")
	{ }

	void spg2xx_base(machine_config &config);
	void spg2xx_basep(machine_config &config);
	void jakks(machine_config &config);
	void jakks_i2c(machine_config &config);
	void walle(machine_config &config);
	void wireless60(machine_config &config);
	void rad_skat(machine_config &config);
	void rad_skatp(machine_config &config);
	void rad_sktv(machine_config &config);
	void rad_crik(machine_config &config);
	void non_spg_base(machine_config &config);
	void lexizeus(machine_config &config);

	void init_crc();
	void init_zeus();
	void init_zone40();

	DECLARE_CUSTOM_INPUT_MEMBER(i2c_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void switch_bank(uint32_t bank);

	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ8_MEMBER(eeprom_r);

	DECLARE_READ16_MEMBER(jakks_porta_r);
	DECLARE_WRITE16_MEMBER(wireless60_porta_w);
	DECLARE_WRITE16_MEMBER(wireless60_portb_w);
	DECLARE_READ16_MEMBER(wireless60_porta_r);

	DECLARE_READ16_MEMBER(rad_porta_r);
	DECLARE_READ16_MEMBER(rad_portb_r);
	DECLARE_READ16_MEMBER(rad_portc_r);

	DECLARE_WRITE16_MEMBER(jakks_porta_w);
	DECLARE_WRITE16_MEMBER(jakks_portb_w);

	required_device<unsp_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<spg2xx_device> m_spg;
	optional_memory_bank m_bank;

	DECLARE_READ16_MEMBER(walle_portc_r);
	DECLARE_WRITE16_MEMBER(walle_portc_w);

	virtual void mem_map_4m(address_map &map);
	virtual void mem_map_2m(address_map &map);
	virtual void mem_map_1m(address_map &map);

	uint32_t m_current_bank;

	std::unique_ptr<uint8_t[]> m_serial_eeprom;
	uint8_t m_w60_controller_input;
	uint16_t m_w60_porta_data;

	uint16_t m_walle_portc_data;

	required_ioport m_io_p1;
	optional_ioport m_io_p2;
	optional_ioport m_io_p3;
	optional_device<i2cmem_device> m_i2cmem;
	optional_device<nvram_device> m_nvram;
};

class jakks_gkr_state : public spg2xx_game_state
{
public:
	jakks_gkr_state(const machine_config &mconfig, device_type type, const char *tag)
		: spg2xx_game_state(mconfig, type, tag)
		, m_porta_key_mode(false)
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void jakks_gkr(machine_config &config);
	void jakks_gkr_i2c(machine_config &config);
	void jakks_gkr_1m_i2c(machine_config &config);
	void jakks_gkr_2m_i2c(machine_config &config);
	void jakks_gkr_nk(machine_config &config);
	void jakks_gkr_nk_i2c(machine_config &config);
	void jakks_gkr_dy_i2c(machine_config &config);
	void jakks_gkr_dp_i2c(machine_config &config);
	void jakks_gkr_sw_i2c(machine_config &config);
	void jakks_gkr_nm_i2c(machine_config &config);
	void jakks_gkr_wf_i2c(machine_config &config);
	void jakks_gkr_mv_i2c(machine_config &config);
	void jakks_gkr_wp(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(i2c_gkr_r);

private:
	virtual void machine_start() override;

	DECLARE_WRITE16_MEMBER(gkr_portc_w);
	DECLARE_WRITE16_MEMBER(jakks_porta_key_io_w);
	DECLARE_READ16_MEMBER(jakks_porta_key_io_r);
	bool m_porta_key_mode;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(gamekey_cart);

	required_device<jakks_gamekey_slot_device> m_cart;
	memory_region *m_cart_region;
};

class vii_state : public spg2xx_game_state
{
public:
	vii_state(const machine_config &mconfig, device_type type, const char *tag)
		: spg2xx_game_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_io_motionx(*this, "MOTIONX")
		, m_io_motiony(*this, "MOTIONY")
		, m_io_motionz(*this, "MOTIONZ")
		, m_cart_region(nullptr)
		, m_ctrl_poll_timer(nullptr)
	{ }

	void vii(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	static const device_timer_id TIMER_CTRL_POLL = 0;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_WRITE16_MEMBER(vii_portb_w);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(vii_cart);

	virtual void poll_controls();

	required_device<generic_slot_device> m_cart;
	required_ioport m_io_motionx;
	required_ioport m_io_motiony;
	required_ioport m_io_motionz;
	memory_region *m_cart_region;

	emu_timer *m_ctrl_poll_timer;
	uint8_t m_controller_input[8];
};

class icanguit_state : public spg2xx_game_state
{
public:
	icanguit_state(const machine_config &mconfig, device_type type, const char *tag)
		: spg2xx_game_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void icanguit(machine_config &config);

private:
	virtual void machine_start() override;
	//virtual void machine_reset() override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(icanguit_cart);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};


/*************************
*    Machine Hardware    *
*************************/

void spg2xx_game_state::switch_bank(uint32_t bank)
{
	if (bank != m_current_bank)
	{
		m_current_bank = bank;
		m_bank->set_entry(bank);
		m_maincpu->invalidate_cache();
	}
}

WRITE8_MEMBER(spg2xx_game_state::eeprom_w)
{
	m_serial_eeprom[offset & 0x3ff] = data;
}

READ8_MEMBER(spg2xx_game_state::eeprom_r)
{
	return m_serial_eeprom[offset & 0x3ff];
}

WRITE16_MEMBER(spg2xx_game_state::wireless60_porta_w)
{
	m_w60_porta_data = data & 0xf00;
	switch (m_w60_porta_data & 0x300)
	{
	case 0x300:
		m_w60_controller_input = -1;
		break;

	case 0x200:
		m_w60_controller_input++;
		break;

	default:
		uint16_t temp1 = m_io_p1->read();
		uint16_t temp2 = m_io_p2->read();
		uint16_t temp3 = 1 << m_w60_controller_input;
		if (temp1 & temp3) m_w60_porta_data ^= 0x400;
		if (temp2 & temp3) m_w60_porta_data ^= 0x800;
		break;
	}
}

READ16_MEMBER(spg2xx_game_state::wireless60_porta_r)
{
	return m_w60_porta_data;
}

WRITE16_MEMBER(spg2xx_game_state::wireless60_portb_w)
{
	switch_bank(data & 7);
}

void vii_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CTRL_POLL:
		poll_controls();
		break;
	default:
		logerror("Unknown timer ID: %d\n", id);
		break;
	}
}

WRITE16_MEMBER(vii_state::vii_portb_w)
{
	switch_bank(((data & 0x80) >> 7) | ((data & 0x20) >> 4));
}


CUSTOM_INPUT_MEMBER(spg2xx_game_state::i2c_r)
{
	return m_i2cmem->read_sda();
}

CUSTOM_INPUT_MEMBER(jakks_gkr_state::i2c_gkr_r)
{
	if (m_cart && m_cart->exists())
	{
		return m_cart->read_cart_seeprom();
	}
	else
	{
		return m_i2cmem->read_sda();
	}
}

WRITE16_MEMBER(spg2xx_game_state::walle_portc_w)
{
	m_walle_portc_data = data & mem_mask;
	if (BIT(mem_mask, 1))
		m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		m_i2cmem->write_sda(BIT(data, 0));
}

WRITE16_MEMBER(jakks_gkr_state::gkr_portc_w)
{
	m_walle_portc_data = data & mem_mask;

	if (m_cart && m_cart->exists())
	{
		m_cart->write_cart_seeprom(space,offset,data,mem_mask);
	}
	else
	{
		if (m_i2cmem)
		{
			if (BIT(mem_mask, 1))
				m_i2cmem->write_scl(BIT(data, 1));
			if (BIT(mem_mask, 0))
				m_i2cmem->write_sda(BIT(data, 0));
		}
	}
}

READ16_MEMBER(spg2xx_game_state::jakks_porta_r)
{
	//logerror("%s: jakks_porta_r\n", machine().describe_context());
	return m_io_p1->read();
}

WRITE16_MEMBER(spg2xx_game_state::jakks_porta_w)
{
	//logerror("%s: jakks_porta_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(spg2xx_game_state::jakks_portb_w)
{
	//logerror("%s: jakks_portb_w %04x\n", machine().describe_context(), data);
}

READ16_MEMBER(jakks_gkr_state::jakks_porta_key_io_r)
{
	//logerror("%s: jakks_porta_key_io_r\n", machine().describe_context());
	if (m_porta_key_mode == false)
	{
		return m_io_p1->read();
	}
	else
	{
		/* masks with 0xf, inverts, and combines it with a previous read (when data written to jakks_porta_key_io_w was 0x0000) and expects result to be 0x0000
		   could just expect data written to be returned, but that would be a strange check.
		   all systems seem to respond to the same result, so how is the per-system lock implemented? */
		return (m_io_p1->read() & 0xfff0) | 0x000f;
	}
}

WRITE16_MEMBER(jakks_gkr_state::jakks_porta_key_io_w)
{
	logerror("%s: jakks_porta_key_io_w %04x\n", machine().describe_context(), data);
	// only seen 0xffff and 0x0000 written here.. writes 0xffff before the 2nd part of the port a gamekey check read.
	if (data == 0xffff)
	{
		m_porta_key_mode = true;
	}
	else
	{
		m_porta_key_mode = false;
	}
}

READ16_MEMBER(spg2xx_game_state::rad_porta_r)
{
	uint16_t data = m_io_p1->read();
	logerror("Port A Read: %04x\n", data);
	return data;
}

READ16_MEMBER(spg2xx_game_state::rad_portb_r)
{
	uint16_t data = m_io_p2->read();
	logerror("Port B Read: %04x\n", data);
	return data;
}

READ16_MEMBER(spg2xx_game_state::rad_portc_r)
{
	uint16_t data = m_io_p3->read();
	logerror("Port C Read: %04x\n", data);
	return data;
}

void spg2xx_game_state::mem_map_4m(address_map &map)
{
	map(0x000000, 0x3fffff).bankr("cartbank");
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

void spg2xx_game_state::mem_map_2m(address_map &map)
{
	map(0x000000, 0x1fffff).mirror(0x200000).bankr("cartbank");
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

void spg2xx_game_state::mem_map_1m(address_map &map)
{
	map(0x000000, 0x0fffff).mirror(0x300000).bankr("cartbank");
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

static INPUT_PORTS_START( vii )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("Button A")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Button B")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Button C")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Button D")

	PORT_START("MOTIONX")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_NAME("Motion Control X")

	PORT_START("MOTIONY")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_NAME("Motion Control Y") PORT_PLAYER(2)

	PORT_START("MOTIONZ")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_NAME("Motion Control Z") PORT_PLAYER(3)
INPUT_PORTS_END

static INPUT_PORTS_START( batman )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")
INPUT_PORTS_END

static INPUT_PORTS_START( walle )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, spg2xx_game_state,i2c_r, nullptr)
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_sith_i2c )
	PORT_START("P1")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0xf3df, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, jakks_gkr_state,i2c_gkr_r, nullptr)
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOYX")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("JOYY")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_pooh )
	PORT_START("P1")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Menu / Pause")
	PORT_BIT( 0xf7df, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOYX")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("JOYY")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_nm_i2c )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, jakks_gkr_state,i2c_gkr_r, nullptr)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag? music speed changes in Mappy
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )


	PORT_START("DIALX") // for Pole Position, joystick can be twisted like a dial/wheel (limited?) (check range)
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_wf_i2c )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, jakks_gkr_state,i2c_gkr_r, nullptr)
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* on real unit you can spin the wheel (and must make sure it completes a full circle, or you lose your turn) instead of pressing 'B' for a random spin but where does it map?
	PORT_START("DIALX")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("DIALY")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
	*/
INPUT_PORTS_END

static INPUT_PORTS_START( jak_gkr )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) // this causes WWE to think the unit is a '2nd Controller' and tells you to plug the 1st one in.
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_sdoo_i2c ) // GameKeyReady units had 2 main buttons, later releases reduced that to 1 button (as the internal games don't require 2 and no GameKeys were released)
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED ) // debug input, skips levels!
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED ) // must be low or other inputs don't work?
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, jakks_gkr_state,i2c_gkr_r, nullptr) // is this correct? doesn't seem to work
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( jak_gkr_i2c )
	PORT_INCLUDE(jak_gkr)

	PORT_MODIFY("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, jakks_gkr_state,i2c_gkr_r, nullptr)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_disp_i2c )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Start / Menu / Pause")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, jakks_gkr_state,i2c_gkr_r, nullptr)
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



static INPUT_PORTS_START( wirels60 )
	PORT_START("P1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Start")

	PORT_START("P2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_NAME("Joypad Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_NAME("Joypad Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_NAME("Joypad Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_NAME("Joypad Right")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2) PORT_NAME("A Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(2) PORT_NAME("B Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(2) PORT_NAME("Menu")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(2) PORT_NAME("Start")
INPUT_PORTS_END

static INPUT_PORTS_START( rad_skat )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Full Left")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Full Right")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Slight Left") // you have to use this for the menus (eg trick lists)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Slight Right")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Front")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Back")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	// there only seem to be 3 buttons on the pad part, so presumably all the above are the skateboard, and below are the pad?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("M Button")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("X Button")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("O Button")
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED ) // read but unused?

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_CUSTOM ) // NTSC (1) / PAL (0) flag
INPUT_PORTS_END

static INPUT_PORTS_START( rad_skatp )
	PORT_INCLUDE(rad_skat)

	PORT_MODIFY("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) // NTSC (1) / PAL (0) flag
INPUT_PORTS_END

static INPUT_PORTS_START( rad_sktv )
	/* how does the Scanner connect? probably some serial port with comms protocol, not IO ports?
	   internal test mode shows 'uart' ports (which currently fail)

	   To access internal test hold DOWN and BUTTON1 together on startup until a coloured screen appears.
	   To cycle through the tests again hold DOWN and press BUTTON1 */

	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mattelcs ) // there is a 'secret test mode' that previously got activated before inputs were mapped, might need unused inputs to active?
	PORT_START("P1")
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED ) // must be IP_ACTIVE_LOW or you can't switch to Football properly?
	PORT_DIPNAME( 0x0018, 0x0000, "Game Select Slider" ) // technically not a dipswitch, a 3 position slider, but how best map it?
	PORT_DIPSETTING(      0x0008, "Baseball (Left)" )
	PORT_DIPSETTING(      0x0010, "Basketball (Middle)" )
	PORT_DIPSETTING(      0x0000, "Football (Right)" )
	// no 4th position possible
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_BIT( 0xffa0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_NAME("Joypad Up")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_NAME("Joypad Down")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_NAME("Joypad Left")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_NAME("Sound") // toggles between sound+music, sound only, and no sound
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_NAME("Hike / Pitch")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_NAME("Shoot / Run")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_NAME("Kick / Hit")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* hold 'Console Down' while powering up to get the test menu, including input tests
   the ball (Wired) and bat (IR) are read some other way as they don't seem to appear in the ports. */
static INPUT_PORTS_START( rad_crik )
	PORT_START("P1")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Console Enter") // these are the controls on the base unit
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Console Down")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Console Left")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Console Right")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Console Up")
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( icanguit ) // this has something like 55 buttons, and some strings to map, must be multiplexed somehow?
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "P2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, "P3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Enter?
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch") // presumably power, kils the game
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rad_fb2 ) // controls must be multiplexed somehow, as there's no room for P2 controls otherwise (unless P2 controls were never finished and it was only sold in a single mat version, Radica left useless P2 menu options in the mini Genesis consoles)
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) // 'left'
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // 'up'
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) // 'right'
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // acts a 'motion ball' in menu (this is an analog input from the ball tho? at least in rad_fb in xavix.cpp so this might just be a debug input here)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) // 'p2 right'
	// none of the remaining inputs seem to do anything
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_CUSTOM ) // NTSC (1) / PAL (0) flag
INPUT_PORTS_END


static INPUT_PORTS_START( lexizeus ) // how many buttons does this have?  I accidentally entered a secret test mode before that seemed to indicate 6, but can't get there again
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button 1") // shoot in Tiger Rescue & Deep
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Pause")

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "P2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button 1 Rapid") // same function as button 1 but with rapid toggle on/off
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button 2 Rapid") // same function as button 2 but with rapid toggle on/off
	PORT_DIPNAME( 0x0004, 0x0004, "P3" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button 2") // toggles ball / number view in pool
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

void icanguit_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
		m_bank->set_entry(0);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(icanguit_state, icanguit_cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size < 0x800000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}



void vii_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
		m_bank->set_entry(0);
	}

	m_ctrl_poll_timer = timer_alloc(TIMER_CTRL_POLL);
	m_ctrl_poll_timer->adjust(attotime::never);

	save_item(NAME(m_controller_input));
}

void vii_state::machine_reset()
{
	spg2xx_game_state::machine_reset();

	m_controller_input[0] = 0;
	m_controller_input[4] = 0;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;

	m_ctrl_poll_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}

void spg2xx_game_state::machine_start()
{
	m_bank->configure_entries(0, (memregion("maincpu")->bytes() + 0x7fffff) / 0x800000, memregion("maincpu")->base(), 0x800000);
	m_bank->set_entry(0);

	m_serial_eeprom = std::make_unique<uint8_t[]>(0x400);
	if (m_nvram)
		m_nvram->set_base(&m_serial_eeprom[0], 0x400);

	save_item(NAME(m_current_bank));
	save_item(NAME(m_w60_controller_input));
	save_item(NAME(m_w60_porta_data));
	save_item(NAME(m_walle_portc_data));
}

void spg2xx_game_state::machine_reset()
{
	m_current_bank = 0;

	m_w60_controller_input = -1;
	m_w60_porta_data = 0;
}

void vii_state::poll_controls()
{
	int32_t x = m_io_motionx ? ((int32_t)m_io_motionx->read() - 0x200) : 0;
	int32_t y = m_io_motiony ? ((int32_t)m_io_motiony->read() - 0x200) : 0;
	int32_t z = m_io_motionz ? ((int32_t)m_io_motionz->read() - 0x200) : 0;

	uint8_t old_input[8];
	memcpy(old_input, m_controller_input, 8);

	m_controller_input[0] = m_io_p1->read();
	m_controller_input[1] = (uint8_t)x;
	m_controller_input[2] = (uint8_t)y;
	m_controller_input[3] = (uint8_t)z;
	m_controller_input[4] = 0;
	x = (x >> 8) & 3;
	y = (y >> 8) & 3;
	z = (z >> 8) & 3;
	m_controller_input[5] = (z << 4) | (y << 2) | x;
	m_controller_input[6] = 0xff;
	m_controller_input[7] = 0;

	if (memcmp(old_input, m_controller_input, 8))
	{
		for(int i = 0; i < 8; i++)
			m_spg->uart_rx(m_controller_input[i]);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(vii_state, vii_cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size < 0x800000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

void spg2xx_game_state::spg2xx_base(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
}

void spg2xx_game_state::non_spg_base(machine_config &config)
{
	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);

	spg2xx_base(config);
}

void spg2xx_game_state::spg2xx_basep(machine_config &config)
{
	spg2xx_base(config);

	m_screen->set_refresh_hz(50);
	m_screen->set_size(320, 312);
}

void vii_state::vii(machine_config &config)
{
	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);

	spg2xx_base(config);

	m_spg->portb_out().set(FUNC(vii_state::vii_portb_w));
	m_spg->eeprom_w().set(FUNC(vii_state::eeprom_w));
	m_spg->eeprom_r().set(FUNC(vii_state::eeprom_r));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vii_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(device_image_load_delegate(&vii_state::device_image_load_vii_cart, this));

	SOFTWARE_LIST(config, "vii_cart").set_original("vii");
}

void icanguit_state::icanguit(machine_config &config)
{
	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);

	spg2xx_base(config);

	m_spg->porta_in().set_ioport("P1");
	m_spg->portb_in().set_ioport("P2");
	m_spg->portc_in().set_ioport("P3");

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "icanguit_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(device_image_load_delegate(&icanguit_state::device_image_load_icanguit_cart, this));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "icanguit_cart").set_original("icanguit");
}


void spg2xx_game_state::wireless60(machine_config &config)
{
	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	spg2xx_base(config);

	m_spg->porta_out().set(FUNC(spg2xx_game_state::wireless60_porta_w));
	m_spg->portb_out().set(FUNC(spg2xx_game_state::wireless60_portb_w));
	m_spg->porta_in().set(FUNC(spg2xx_game_state::wireless60_porta_r));
}

void spg2xx_game_state::jakks(machine_config &config)
{
	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	spg2xx_base(config);

	m_spg->porta_in().set(FUNC(spg2xx_game_state::jakks_porta_r));
	m_spg->porta_out().set(FUNC(spg2xx_game_state::jakks_porta_w));
	m_spg->portb_out().set(FUNC(spg2xx_game_state::jakks_portb_w));
}

void spg2xx_game_state::jakks_i2c(machine_config &config)
{
	jakks(config);
	I2CMEM(config, m_i2cmem, 0).set_data_size(0x200);
}

void jakks_gkr_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(JAKKSSLOT_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
		m_bank->set_entry(0);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(jakks_gkr_state, gamekey_cart)
{
	return m_cart->call_load();
}

void jakks_gkr_state::jakks_gkr(machine_config &config)
{
	jakks(config);

	m_spg->porta_in().set(FUNC(jakks_gkr_state::jakks_porta_key_io_r));
	m_spg->porta_out().set(FUNC(jakks_gkr_state::jakks_porta_key_io_w));
	m_spg->portc_in().set_ioport("P3");
	m_spg->portc_out().set(FUNC(jakks_gkr_state::gkr_portc_w));

	m_spg->set_rowscroll_offset(0);

	JAKKS_GAMEKEY_SLOT(config, m_cart, 0, jakks_gamekey, nullptr);
}

void jakks_gkr_state::jakks_gkr_i2c(machine_config &config)
{
	jakks_gkr(config);
	I2CMEM(config, m_i2cmem, 0).set_data_size(0x200);
}


void jakks_gkr_state::jakks_gkr_1m_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
}

void jakks_gkr_state::jakks_gkr_2m_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_2m);
}

void jakks_gkr_state::jakks_gkr_nk(machine_config &config)
{
	jakks_gkr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_nk").set_original("jakks_gamekey_nk");
}

void jakks_gkr_state::jakks_gkr_nk_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_nk").set_original("jakks_gamekey_nk");
}

void jakks_gkr_state::jakks_gkr_dy_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_dy").set_original("jakks_gamekey_dy");
}

void jakks_gkr_state::jakks_gkr_mv_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_mv").set_original("jakks_gamekey_mv");
}


void jakks_gkr_state::jakks_gkr_dp_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_dp").set_original("jakks_gamekey_dp");
}

void jakks_gkr_state::jakks_gkr_sw_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	m_spg->adc_in<0>().set_ioport("JOYX");
	m_spg->adc_in<1>().set_ioport("JOYY");
	SOFTWARE_LIST(config, "jakks_gamekey_sw").set_original("jakks_gamekey_sw");
}

void jakks_gkr_state::jakks_gkr_wp(machine_config &config)
{
	jakks_gkr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	m_spg->adc_in<0>().set_ioport("JOYX");
	m_spg->adc_in<1>().set_ioport("JOYY");
	//SOFTWARE_LIST(config, "jakks_gamekey_wp").set_original("jakks_gamekey_wp"); // NO KEYS RELEASED
}

void jakks_gkr_state::jakks_gkr_nm_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	m_spg->adc_in<0>().set_ioport("DIALX");
	SOFTWARE_LIST(config, "jakks_gamekey_nm").set_original("jakks_gamekey_nm");
}

void jakks_gkr_state::jakks_gkr_wf_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	//m_spg->adc_in<0>().set_ioport("DIALX"); // wheel does not seem to map here
	//m_spg->adc_in<1>().set_ioport("DIALY");
	//SOFTWARE_LIST(config, "jakks_gamekey_wf").set_original("jakks_gamekey_wf"); // no game keys were released
}


void spg2xx_game_state::lexizeus(machine_config &config)
{
	non_spg_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_4m);
	m_spg->porta_in().set_ioport("P1");
	m_spg->portb_in().set_ioport("P2");
	m_spg->portc_in().set_ioport("P3");
}

void spg2xx_game_state::walle(machine_config &config)
{
	jakks_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg2xx_game_state::mem_map_2m);
	m_spg->portc_in().set_ioport("P3");
	m_spg->portc_out().set(FUNC(spg2xx_game_state::walle_portc_w));
}

void spg2xx_game_state::rad_skat(machine_config &config)
{
	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	spg2xx_base(config);

	m_spg->porta_in().set_ioport("P1");
	m_spg->portb_in().set_ioport("P2");
	m_spg->portc_in().set_ioport("P3");
	m_spg->eeprom_w().set(FUNC(spg2xx_game_state::eeprom_w));
	m_spg->eeprom_r().set(FUNC(spg2xx_game_state::eeprom_r));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);
}

void spg2xx_game_state::rad_skatp(machine_config &config)
{
	rad_skat(config);
	m_spg->set_pal(true);
}

void spg2xx_game_state::rad_sktv(machine_config &config)
{
	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	spg2xx_base(config);

	m_spg->porta_in().set(FUNC(spg2xx_game_state::rad_porta_r));
	m_spg->portb_in().set(FUNC(spg2xx_game_state::rad_portb_r));
	m_spg->portc_in().set(FUNC(spg2xx_game_state::rad_portc_r));
	m_spg->eeprom_w().set(FUNC(spg2xx_game_state::eeprom_w));
	m_spg->eeprom_r().set(FUNC(spg2xx_game_state::eeprom_r));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);
}

void spg2xx_game_state::rad_crik(machine_config &config)
{
	SPG28X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	spg2xx_base(config);

	m_spg->porta_in().set_ioport("P1");
	m_spg->portb_in().set_ioport("P2");
	m_spg->portc_in().set_ioport("P3");
	m_spg->eeprom_w().set(FUNC(spg2xx_game_state::eeprom_w));
	m_spg->eeprom_r().set(FUNC(spg2xx_game_state::eeprom_r));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);
}

ROM_START( vii )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vii.bin", 0x0000, 0x2000000, CRC(04627639) SHA1(f883a92d31b53c9a5b0cdb112d07cd793c95fc43))
ROM_END

ROM_START( jak_batm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "batman.bin", 0x000000, 0x400000, CRC(46f848e5) SHA1(5875d57bb3fe0cac5d20e626e4f82a0e5f9bb94c) )
ROM_END

ROM_START( jak_wall )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, BAD_DUMP CRC(bd554cba) SHA1(6cd06a036ab12e7b0e1fd8003db873b0bb783868) )
	// both of these dumps are bad, but in slightly different ways, note the random green pixels around the text (bad data is reported in secret test mode)
	//ROM_LOAD16_WORD_SWAP( "walle.bin", 0x000000, 0x400000, BAD_DUMP CRC(6bc90b16) SHA1(184d72de059057aae7800da510fcf05ed1da9ec9))
ROM_END

ROM_START( jak_wwe )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkswwegkr.bin", 0x000000, 0x200000, CRC(b078a812) SHA1(7d97c0e2171b3fd91b280480c9ffd5651828195a) )
ROM_END

ROM_START( jak_fan4 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksffgkr.bin", 0x000000, 0x200000, CRC(8755a1f7) SHA1(7214da15fe61881da27b81575fbdb54cc0f1d6aa) )
ROM_END

ROM_START( jak_just )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksjlagkr.bin", 0x000000, 0x200000, CRC(182989f0) SHA1(799229c537d6fe629ba9e1e4051d1bb9ca445d44) )
ROM_END

ROM_START( jak_dora )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdoragkr.bin", 0x000000, 0x200000, CRC(bcaa132d) SHA1(3894b980fbc4144731b2a7a94acebb29e30de67c) )
ROM_END

ROM_START( jak_nick )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksnicktoonsgkr.bin", 0x000000, 0x200000, CRC(4dec1656) SHA1(b3002ab15e75068102f4955a3f0c52fb6d5cda56) )
ROM_END

ROM_START( jak_sbfc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksspongebobgkr.bin", 0x000000, 0x200000, CRC(9871303c) SHA1(78bc2687e1514094db8bb875e1117df3fcb3d201) )
ROM_END

ROM_START( jak_dorr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdora2gkr.bin", 0x000000, 0x200000, CRC(6c09bcd9) SHA1(4bcad79658832f319d16b4f63257e127f6862d79) )
ROM_END


ROM_START( jak_spdm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksspidermangkr.bin", 0x000000, 0x200000, CRC(1b2ee700) SHA1(30ea69c489e1238b004f473f972b682e35573138) )
ROM_END

ROM_START( jak_pooh )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspoohgkr.bin", 0x000000, 0x200000, CRC(0d97df55) SHA1(f108621a83c7b2263dd1531d82311627c3a02002) )
ROM_END


ROM_START( jak_wof )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkswheeloffortunegkr.bin", 0x000000, 0x200000, CRC(6a879620) SHA1(95478764a61741569041c2299528f6464651d593) )
ROM_END

ROM_START( jak_disf )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "disneyfriendsgkr.bin", 0x000000, 0x200000, CRC(77bca50b) SHA1(6e0f4fd229ee11eac721b5dbe79cf9002d3dbd64) )
ROM_END

ROM_START( jak_disp )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdisneyprincessgkr.bin", 0x000000, 0x200000, CRC(e26003ce) SHA1(ee15243281df6f09b96185c34582d7091604c954) )
ROM_END

ROM_START( jak_mpac )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksmspacmangkr.bin", 0x000000, 0x100000, CRC(cab40f77) SHA1(30731acc461150d96aafa7a0451cfb1a25264678) )
ROM_END

ROM_START( jak_sdoo )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksscoobydoogkr.bin", 0x000000, 0x400000, CRC(61062ce5) SHA1(9d21767fd855385ef83e4209c429ecd4bf7e5384) )
ROM_END

ROM_START( jak_dbz )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdragonballzgkr.bin", 0x000000, 0x200000, CRC(d52c3b20) SHA1(fd5ce41c143cad9bca3372054f4ff98b52c33874) )
ROM_END

ROM_START( jak_sith )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksstarwarsgkr.bin", 0x000000, 0x200000, CRC(932cde19) SHA1(b88b748c235e9eeeda574e4d5b4077ae9da6fbd0) )
ROM_END


ROM_START( lexizeus )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "lexibook1g900us.bin", 0x0000, 0x800000, CRC(c2370806) SHA1(cbb599c29c09b62b6a9951c724cd9fc496309cf9))
ROM_END

ROM_START( zone40 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "zone40.bin", 0x0000, 0x4000000, CRC(4ba1444f) SHA1(de83046ab93421486668a247972ad6d3cda19440) )
ROM_END

ROM_START( zone60 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "zone60.bin", 0x0000, 0x4000000, CRC(4cb637d1) SHA1(1f97cbdb4299ac0fbafc2a3aa592066cb0727066))
ROM_END

ROM_START( wirels60 )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wirels60.bin", 0x0000, 0x4000000, CRC(b4df8b28) SHA1(00e3da542e4bc14baf4724ad436f66d4c0f65c84))
ROM_END

ROM_START( rad_skat )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "skateboarder.bin", 0x000000, 0x400000, CRC(08b9ab91) SHA1(6665edc4740804956136c68065890925a144626b) )
ROM_END

ROM_START( rad_skatp ) // rom was dumped from the NTSC version, but region comes from an io port, so ROM is probably the same
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "skateboarder.bin", 0x000000, 0x400000, CRC(08b9ab91) SHA1(6665edc4740804956136c68065890925a144626b) )
ROM_END

ROM_START( rad_sktv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "skannerztv.bin", 0x000000, 0x200000, CRC(e92278e3) SHA1(eb6bee5e661128d83784960dfff50379c36bfaeb) )

	/* The external scanner MCU is a Winbond from 2000: SA5641
	   the scanner plays sound effects when scanning, without being connected to the main unit, so a way to dump / emulate
	   this MCU is also needed for complete emulation

	   TODO: find details on MCU so that we know capacity etc. */
ROM_END

ROM_START( rad_fb2 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "football2.bin", 0x000000, 0x400000, CRC(96b4f0d2) SHA1(e91f2ac679fb0c026ffe216eb4ab58802f361a17) )
ROM_END

ROM_START( rad_crik ) // only released in EU?
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "cricket.bin", 0x000000, 0x200000, CRC(6fa0aaa9) SHA1(210d2d4f542181f59127ce2f516d0408dc6de7a8) )
ROM_END


ROM_START( mattelcs )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mattelclassicsports.bin", 0x000000, 0x100000, CRC(e633e7ad) SHA1(bf3e325a930cf645a7e32195939f3c79c6d35dac) )
ROM_END

ROM_START( dreamlif )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dreamlife.bin", 0x000000, 0x800000, CRC(632e0237) SHA1(a8586e8a626d75cf7782f13cfd9f1b938af23d56) )
ROM_END

ROM_START( icanguit )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// no internal ROM, requires a cartridge
ROM_END


/*
Wireless Air 60
(info provided with dump)

System: Wireless Air 60
ROM: Toshiba TC58NVG0S3ETA00
RAM: ESMT M12L128168A

This is a raw NAND flash dump

Interesting Strings:

GPnandnand; (GP is General Plus, which is Sunplus by another name)
GLB_GP-F_5B_USBD_1.0.0
SP_ToneMaker
GLB_GP-FS1_0405L_SPU_1.0.2.3
SPF2ALP

"GPnandnand" as a required signature appears to be referenced right here, in page 19 of a GeneralPlus document;
http://www.lcis.com.tw/paper_store/paper_store/GPL162004A-507A_162005A-707AV10_code_reference-20147131205102.pdf

*/

ROM_START( wlsair60 )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wlsair60.nand", 0x0000, 0x8400000, CRC(eec23b97) SHA1(1bb88290cf54579a5bb51c08a02d793cd4d79f7a) )
ROM_END

/*
Wireless Hunting Video Game System
(info provided with dump)

System: Wireless Hunting Video Game System
Publisher: Hamy / Kids Station Toys Inc
Year: 2011
ROM: FDI MSP55LV100G
RAM: Micron Technology 48LC8M16A2

Games:

Secret Mission
Predator
Delta Force
Toy Land
Dream Forest
Trophy Season
Freedom Force
Be Careful
Net Power
Open Training
Super Archer
Ultimate Frisbee
UFO Shooting
Happy Darts
Balloon Shoot
Avatair
Angry Pirate
Penguin War
Ghost Shooter
Duck Hunt


ROM Board:

Package: SO44
Spacing: 1.27 mm
Width: 16.14 mm
Length: 27.78 mm
Voltage: 3V
Pinout:

          A25  A24
            |  |
      +--------------------------+
A21 --|==   #  # `.__.'        ==|-- A20
A18 --|==                      ==|-- A19
A17 --|==                      ==|-- A8
 A7 --|==                      ==|-- A9
 A6 --|==                  o   ==|-- A10
 A5 --|==  +----------------+  ==|-- A11
 A4 --|==  |                |  ==|-- A12
 A3 --|==  |  MSP55LV100G   |  ==|-- A13
 A2 --|==  |  0834 M02H     |  ==|-- A14
 A1 --|==  |  JAPAN         |  ==|-- A15
 A0 --|==  |                |  ==|-- A16
#CE --|==  |                |  ==|-- A23
GND --|==  |                |  ==|-- A22
#OE --|==  |                |  ==|-- Q15
 Q0 --|==  |                |  ==|-- Q7
 Q8 --|==  |                |  ==|-- Q14
 Q1 --|==  +----------------+  ==|-- Q6
 Q9 --|==                      ==|-- Q13
 Q2 --|==       M55L100G       ==|-- Q5
Q10 --|==                      ==|-- Q12
 Q3 --|==                      ==|-- Q4
Q11 --|==                      ==|-- VCC
      +--------------------------+


The only interesting string in this ROM is SPF2ALP,
which is also found in the Wireless Air 60 ROM.

*/

ROM_START( wrlshunt )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wireless.bin", 0x0000, 0x8000000, CRC(a6ecc20e) SHA1(3645f23ba2bb218e92d4560a8ae29dddbaabf796) )
ROM_END


void spg2xx_game_state::init_crc()
{
	// several games have a byte sum checksum listed at the start of ROM, this little helper function logs what it should match.
	const int length = memregion("maincpu")->bytes();
	const uint8_t* rom = memregion("maincpu")->base();

	uint32_t checksum = 0x00000000;
	// the first 0x10 bytes are where the "chksum:xxxxxxxx " string is listed, so skip over them
	for (int i = 0x10; i < length; i++)
	{
		checksum += rom[i];
	}

	logerror("Calculated Byte Sum of bytes from 0x10 to 0x%08x is %08x)\n", length - 1, checksum);
}

void spg2xx_game_state::init_zeus()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0x8000 / 2; i < size / 2; i++)
	{
		// global 16-bit xor
		ROM[i] = ROM[i] ^ 0x8a1d;

		// 4 single bit conditional xors
		if (ROM[i] & 0x0020)
			ROM[i] ^= 0x0100;

		if (ROM[i] & 0x0040)
			ROM[i] ^= 0x1000;

		if (ROM[i] & 0x4000)
			ROM[i] ^= 0x0001;

		if (ROM[i] & 0x0080)
			ROM[i] ^= 0x0004;

		// global 16-bit bitswap
		ROM[i] = bitswap<16>(ROM[i], 7, 12, 9, 14, 4, 6, 0, 10, 15, 1, 3, 2, 5, 13, 8, 11);
	}
}

void spg2xx_game_state::init_zone40()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = ROM[i] ^ 0xbb88;
	}
	//there is also bitswapping as above, and some kind of address scramble as the vectors are not exactly where expected
}

// year, name, parent, compat, machine, input, class, init, company, fullname, flags

// Jungle Soft TV games
CONS( 2007, vii,      0, 0, vii,        vii,      vii_state,         empty_init, "Jungle Soft / KenSingTon / Siatronics",    "Vii",         MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // motion controls are awkward, but playable for the most part
CONS( 2010, zone60,   0, 0, wireless60, wirels60, spg2xx_game_state, empty_init, "Jungle's Soft / Ultimate Products (HK) Ltd", "Zone 60",     MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2010, wirels60, 0, 0, wireless60, wirels60, spg2xx_game_state, empty_init, "Jungle Soft / Kids Station Toys Inc",      "Wireless 60", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// JAKKS Pacific Inc TV games
CONS( 2004, jak_batm, 0, 0, jakks, batman, spg2xx_game_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd", "The Batman (JAKKS Pacific TV Game)",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2008, jak_wall, 0, 0, walle, walle,  spg2xx_game_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd", "Wall-E (JAKKS Pacific TV Game)",              MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// 'Game-Key Ready' JAKKS games (these can also take per-game specific expansion cartridges, although not all games had them released)
// Some of these were available in versions without Game-Key ports, it is unconfirmed if code was the same unless otherwise stated
// For units released AFTER the GameKey promotion was cancelled it appears the code is the same as the PCB inside is the same, just the external port closed off, earlier units might be different hardware in some cases.
CONS( 2005, jak_wwe,  0, 0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "WWE (JAKKS Pacific TV Game, Game-Key Ready)",            MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // WW (no game-keys released)
CONS( 2005, jak_fan4, 0, 0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse",         "Fantastic Four (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // F4 (no game-keys released)
CONS( 2005, jak_just, 0, 0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Taniko",                  "Justice League (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // DC (no game-keys released)
CONS( 2005, jak_dora, 0, 0, jakks_gkr_nk,     jak_gkr,      jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Dora the Explorer - Nursery Rhyme Adventure (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys (same as Nicktoons & Spongebob) (3 released) - The upper part of this one is pink/purple.
CONS( 2005, jak_dorr, 0, 0, jakks_gkr_nk_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Dora the Explorer - Race to Play Park (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys (same as Nicktoons & Spongebob) (3 released) - The upper part of this one is blue
CONS( 2004, jak_nick, 0, 0, jakks_gkr_nk_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Nicktoons (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys
CONS( 2005, jak_sbfc, 0, 0, jakks_gkr_nk_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "SpongeBob SquarePants - The Fry Cook Games (JAKKS Pacific TV Game, Game-Key Ready) (AUG 18 2005 21:31:56)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys
CONS( 2005, jak_sdoo, 0, 0, jakks_gkr_2m_i2c, jak_sdoo_i2c, jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Jolliford Management",    "Scooby-Doo! and the Mystery of the Castle (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) //  SD (no game-keys released)  (was dumped from a later unit with GameKey port missing, but internal PCB still supported it, code likely the same)
CONS( 2005, jak_disf, 0, 0, jakks_gkr_dy_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "Disney Friends (JAKKS Pacific TV Game, Game-Key Ready) (17 MAY 2005 A)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses DY keys (3 released)
CONS( 2005, jak_disp, 0, 0, jakks_gkr_dp_i2c, jak_disp_i2c, jakks_gkr_state, empty_init, "JAKKS Pacific Inc / 5000ft, Inc",             "Disney Princess (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses DP keys (1 key released)
// There seems to be a second game called 'Disney Princesses' with a 'board game' style front end as well as the minigames, also GKR, see https://www.youtube.com/watch?v=w9p5TI029bQ  The one we have is https://www.youtube.com/watch?v=9ppPKVbpoMs  the physical package seems identical.
CONS( 2005, jak_sith, 0, 0, jakks_gkr_sw_i2c, jak_sith_i2c, jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Griptonite Games",        "Star Wars - Revenge of the Sith (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses SW keys (1 released)
CONS( 2005, jak_dbz,  0, 0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Dragon Ball Z (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // DB (no game-keys released, 1 in development but cancelled)
CONS( 2005, jak_mpac, 0, 0, jakks_gkr_nm_i2c, jak_nm_i2c,   jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man 5-in-1 (Ms. Pac-Man, Pole Position, Galaga, Xevious, Mappy) (JAKKS Pacific TV Game, Game-Key Ready) (07 FEB 2005 A SKU F)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NM (3 keys available [Dig Dug, New Rally-X], [Rally-X, Pac-Man, Bosconian], [Pac-Man, Bosconian])
CONS( 2005, jak_wof,  0, 0, jakks_gkr_wf_i2c, jak_wf_i2c,   jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "Wheel of Fortune (JAKKS Pacific TV Game, Game-Key Ready)",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses WF keys (no game-keys released)  analog wheel not emulated
// There is a 'Second Edition' version of Wheel of Fortune with a Gold case, GameKey port removed, and a '2' over the usual Game Key Ready logo, internals are different too, not Game-Key Ready
CONS( 2004, jak_spdm, 0, 0, jakks_gkr_mv_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse",         "Spider-Man (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) //  MV (1 key available)
CONS( 2005, jak_pooh, 0, 0, jakks_gkr_wp,     jak_pooh,     jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Backbone Entertainment",  "Winnie the Pooh - Piglet's Special Day (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // WP (no game-keys released)

// no keys released for the following, some were in development but cancelled
// Capcom 3-in-1                               CC (no game-keys released)
// Care Bears                                  CB (no game-keys released)

// Some versions of the Shrek - Over the Hedge unit show the GameKey logo on startup (others don't) there is no evidence to suggest it was ever released with a GameKey port tho, and the internal PCB has no place for one on the versions we've seen (which show the logo)

// Radica TV games
CONS( 2006, rad_skat,  0,        0, rad_skat, rad_skat,   spg2xx_game_state, init_crc, "Radica", "Play TV Skateboarder (NTSC)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2006, rad_skatp, rad_skat, 0, rad_skatp,rad_skatp,  spg2xx_game_state, init_crc, "Radica", "Connectv Skateboarder (PAL)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2006, rad_crik,  0,        0, rad_crik, rad_crik,   spg2xx_game_state, init_crc, "Radica", "Connectv Cricket (PAL)",      MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // Version 3.00 20/03/06 is listed in INTERNAL TEST
CONS( 2007, rad_sktv,  0,        0, rad_sktv, rad_sktv,   spg2xx_game_state, init_crc, "Radica", "Skannerz TV",                 MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
CONS( 2007, rad_fb2,   0,        0, rad_skat, rad_fb2,    spg2xx_game_state, init_crc, "Radica", "Play TV Football 2",          MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

// Mattel games
CONS( 2005, mattelcs,  0,        0, rad_skat, mattelcs,   spg2xx_game_state, empty_init, "Mattel", "Mattel Classic Sports",     MACHINE_IMPERFECT_SOUND )

// Hasbro games
CONS( 2007, dreamlif,  0,        0, rad_skat, rad_crik,   spg2xx_game_state, empty_init, "Hasbro", "Dream Life",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

CONS( 2007, icanguit,  0,        0, icanguit, icanguit,   icanguit_state, empty_init, "Mattel / Fisher-Price", "I Can Play Guitar",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// might not fit here.  First 0x8000 bytes are blank (not too uncommon for these) then rest of rom looks like it's probably encrypted at least
// could be later model VT based instead? even after decrypting (simple word xor) the vectors have a different format and are at a different location to the SunPlus titles
CONS( 2009, zone40,    0,       0,        non_spg_base, wirels60, spg2xx_game_state, init_zone40, "Jungle Soft / Ultimate Products (HK) Ltd",          "Zone 40",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// Similar, SPG260?, scrambled
CONS( 200?, lexizeus,    0,       0,        lexizeus, lexizeus, spg2xx_game_state, init_zeus, "Lexibook",          "Zeus IG900 20-in-1 (US?)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )


// valid looking code, but extended periperhal area (twice the size?) makes use of unemulated opcode 0xfe00 ?
CONS( 2011, wrlshunt,  0,       0,        non_spg_base, wirels60, spg2xx_game_state, empty_init, "Hamy / Kids Station Toys Inc",                      "Wireless Hunting Video Game System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// NAND dumps w/ internal bootstrap. Almost certainly do not fit in this driver, as the SPG2xx can only address up to 4Mwords. These are 'GeneralPlus' instead?
CONS( 2010, wlsair60,  0,       0,        non_spg_base, wirels60, spg2xx_game_state, empty_init, "Jungle Soft / Kids Station Toys Inc",               "Wireless Air 60",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
