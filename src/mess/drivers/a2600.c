/***************************************************************************

  Atari VCS 2600 driver

***************************************************************************/

#include "emu.h"
#include "machine/6532riot.h"
#include "cpu/m6502/m6502.h"
#include "sound/wave.h"
#include "sound/tiaintf.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "formats/a26_cas.h"
#include "video/tia.h"
#include "hashfile.h"

struct df_t {
	UINT8	top;
	UINT8	bottom;
	UINT8	low;
	UINT8	high;
	UINT8	flag;
	UINT8	music_mode;		/* Only used by data fetchers 5,6, and 7 */
	UINT8	osc_clk;		/* Only used by data fetchers 5,6, and 7 */
};

struct dpc_t
{
	df_t df[8];
	UINT8	movamt;
	UINT8	latch_62;
	UINT8	latch_64;
	UINT8	dlc;
	UINT8	shift_reg;
	emu_timer	*oscillator;
};


class a2600_state : public driver_device
{
public:
	a2600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_riot_ram(*this, "riot_ram"){ }

	dpc_t m_dpc;
	memory_region* m_extra_RAM;
	UINT8* m_bank_base[5];
	UINT8* m_ram_base;
	required_shared_ptr<UINT8> m_riot_ram;
	UINT8 m_banking_mode;
	UINT8 m_keypad_left_column;
	UINT8 m_keypad_right_column;
	unsigned m_cart_size;
	unsigned m_number_banks;
	unsigned m_current_bank;
	unsigned m_current_reset_bank_counter;
	unsigned m_mode3E_ram_enabled;
	UINT8 m_modeSS_byte;
	UINT32 m_modeSS_byte_started;
	unsigned m_modeSS_write_delay;
	unsigned m_modeSS_write_enabled;
	unsigned m_modeSS_high_ram_enabled;
	unsigned m_modeSS_diff_adjust;
	unsigned m_FVlocked;
	UINT16 m_current_screen_height;
	int m_FETimer;

	direct_update_delegate m_FE_old_opbase_handler;

	DECLARE_DIRECT_UPDATE_MEMBER(modeF6_opbase);
	DECLARE_DIRECT_UPDATE_MEMBER(modeSS_opbase);
	DECLARE_DIRECT_UPDATE_MEMBER(modeDPC_opbase_handler);
	DECLARE_DIRECT_UPDATE_MEMBER(modeFE_opbase_handler);
	DECLARE_READ8_MEMBER(modeF8_switch_r);
	DECLARE_READ8_MEMBER(modeFA_switch_r);
	DECLARE_READ8_MEMBER(modeF6_switch_r);
	DECLARE_READ8_MEMBER(modeF4_switch_r);
	DECLARE_READ8_MEMBER(modeE0_switch_r);
	DECLARE_READ8_MEMBER(modeE7_switch_r);
	DECLARE_READ8_MEMBER(modeE7_RAM_switch_r);
	DECLARE_READ8_MEMBER(modeUA_switch_r);
	DECLARE_READ8_MEMBER(modeDC_switch_r);
	DECLARE_READ8_MEMBER(modeFV_switch_r);
	DECLARE_READ8_MEMBER(modeJVP_switch_r);
	DECLARE_WRITE8_MEMBER(modeF8_switch_w);
	DECLARE_WRITE8_MEMBER(modeFA_switch_w);
	DECLARE_WRITE8_MEMBER(modeF6_switch_w);
	DECLARE_WRITE8_MEMBER(modeF4_switch_w);
	DECLARE_WRITE8_MEMBER(modeE0_switch_w);
	DECLARE_WRITE8_MEMBER(modeE7_switch_w);
	DECLARE_WRITE8_MEMBER(modeE7_RAM_switch_w);
	DECLARE_WRITE8_MEMBER(mode3F_switch_w);
	DECLARE_WRITE8_MEMBER(modeUA_switch_w);
	DECLARE_WRITE8_MEMBER(modeDC_switch_w);
	DECLARE_WRITE8_MEMBER(mode3E_switch_w);
	DECLARE_WRITE8_MEMBER(mode3E_RAM_switch_w);
	DECLARE_WRITE8_MEMBER(mode3E_RAM_w);
	DECLARE_WRITE8_MEMBER(modeFV_switch_w);
	DECLARE_WRITE8_MEMBER(modeJVP_switch_w);
	DECLARE_READ8_MEMBER(modeSS_r);
	DECLARE_READ8_MEMBER(modeDPC_r);
	DECLARE_WRITE8_MEMBER(modeDPC_w);
	DECLARE_READ8_MEMBER(modeFE_switch_r);
	DECLARE_WRITE8_MEMBER(modeFE_switch_w);
	DECLARE_READ8_MEMBER(current_bank_r);
	DECLARE_READ16_MEMBER(a2600_read_input_port);
	DECLARE_READ8_MEMBER(a2600_get_databus_contents);
	DECLARE_WRITE16_MEMBER(a2600_tia_vsync_callback);
	DECLARE_WRITE16_MEMBER(a2600_tia_vsync_callback_pal);
	void modeDPC_check_flag(UINT8 data_fetcher);
	void modeDPC_decrement_counter(UINT8 data_fetcher);
	virtual void machine_reset();
	DECLARE_MACHINE_START(a2600);
	DECLARE_MACHINE_START(a2600p);
};



#define CART machine.root_device().memregion("user1")->base()
#define CART_MEMBER machine().root_device().memregion("user1")->base()

#define MASTER_CLOCK_NTSC	3579545
#define MASTER_CLOCK_PAL	3546894
#define CATEGORY_SELECT		16

/* Some defines for the naming of the controller ports and the controllers */
#define	STR_LEFT_CONTROLLER		"Left Controller"
#define STR_RIGHT_CONTROLLER	"Right Controller"
#define STR_PADDLES				"Paddles"
#define STR_DRIVING				"Driving"
#define STR_KEYPAD				"Keypad"
#define STR_LIGHTGUN			"Lightgun"
#define STR_BOOSTERGRIP			"Booster Grip"
#define STR_CX22TRAKBALL		"CX-22 Trak-Ball"
#define STR_CX80TRAKBALL		"CX-80 Trak-Ball (TB Mode) / AtariST Mouse"
#define STR_AMIGAMOUSE			"Amiga Mouse"
#define STR_KIDVID				"KidVid Voice Module"

enum
{
	mode2K,
	mode4K,
	modeF8,
	modeFA,
	modeF6,
	modeF4,
	modeFE,
	modeE0,
	mode3F,
	modeUA,
	modeE7,
	modeDC,
	modeCV,
	mode3E,
	modeSS,
	modeFV,
	modeDPC,
	mode32in1,
	modeJVP
};

static const UINT16 supported_screen_heights[4] = { 262, 312, 328, 342 };

static int detect_modeDC(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,numfound = 0;
	// signature is also in 'video reflex'.. maybe figure out that controller port someday...
	static const unsigned char signature[3] = { 0x8d, 0xf0, 0xff };
	if (state->m_cart_size == 0x10000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature,sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modef6(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i, numfound = 0;
	static const unsigned char signature[3] = { 0x8d, 0xf6, 0xff };
	if (state->m_cart_size == 0x4000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature, sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_mode3E(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	// this one is a little hacky.. looks for STY $3e, which is unique to
	// 'not boulderdash', but is the only example i have (cow)
	// Would have used STA $3e, but 'Alien' and 'Star Raiders' do that for unknown reasons

	int i,numfound = 0;
	static const unsigned char signature[3] = { 0x84, 0x3e, 0x9d };
	if (state->m_cart_size == 0x0800 || state->m_cart_size == 0x1000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature,sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeSS(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,numfound = 0;
	static const unsigned char signature[5] = { 0xbd, 0xe5, 0xff, 0x95, 0x81 };
	if (state->m_cart_size == 0x0800 || state->m_cart_size == 0x1000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature,sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeFE(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,j,numfound = 0;
	static const unsigned char signatures[][5] =  {
									{ 0x20, 0x00, 0xd0, 0xc6, 0xc5 },
									{ 0x20, 0xc3, 0xf8, 0xa5, 0x82 },
									{ 0xd0, 0xfb, 0x20, 0x73, 0xfe },
									{ 0x20, 0x00, 0xf0, 0x84, 0xd6 }
	};
	if (state->m_cart_size == 0x2000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
		{
			for (j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeE0(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,j,numfound = 0;
	static const unsigned char signatures[][3] =  {
									{ 0x8d, 0xe0, 0x1f },
									{ 0x8d, 0xe0, 0x5f },
									{ 0x8d, 0xe9, 0xff },
									{ 0xad, 0xe9, 0xff },
									{ 0xad, 0xed, 0xff },
									{ 0xad, 0xf3, 0xbf }
	};
	if (state->m_cart_size == 0x2000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
		{
			for (j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeCV(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,j,numfound = 0;
	static const unsigned char signatures[][3] = {
									{ 0x9d, 0xff, 0xf3 },
									{ 0x99, 0x00, 0xf4 }
	};
	if (state->m_cart_size == 0x0800 || state->m_cart_size == 0x1000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
		{
			for (j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeFV(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,j,numfound = 0;
	static const unsigned char signatures[][3] = {
									{ 0x2c, 0xd0, 0xff }
	};
	if (state->m_cart_size == 0x2000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
		{
			for (j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
		state->m_FVlocked = 0;
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeJVP(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,j,numfound = 0;
	static const unsigned char signatures[][4] = {
									{ 0x2c, 0xc0, 0xef, 0x60 },
									{ 0x8d, 0xa0, 0x0f, 0xf0 }
	};
	if (state->m_cart_size == 0x4000 || state->m_cart_size == 0x2000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
		{
			for (j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeE7(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,j,numfound = 0;
	static const unsigned char signatures[][3] = {
									{ 0xad, 0xe5, 0xff },
									{ 0x8d, 0xe7, 0xff }
	};
	if (state->m_cart_size == 0x2000 || state->m_cart_size == 0x4000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
		{
			for (j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_modeUA(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,numfound = 0;
	static const unsigned char signature[3] = { 0x8d, 0x40, 0x02 };
	if (state->m_cart_size == 0x2000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature,sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound) return 1;
	return 0;
}

static int detect_8K_mode3F(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,numfound = 0;
	static const unsigned char signature1[4] = { 0xa9, 0x01, 0x85, 0x3f };
	static const unsigned char signature2[4] = { 0xa9, 0x02, 0x85, 0x3f };
	// have to look for two signatures because 'not boulderdash' gives false positive otherwise
	if (state->m_cart_size == 0x2000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - sizeof signature1; i++)
		{
			if (!memcmp(&cart[i], signature1,sizeof signature1))
			{
				numfound |= 0x01;
			}
			if (!memcmp(&cart[i], signature2,sizeof signature2))
			{
				numfound |= 0x02;
			}
		}
	}
	if (numfound == 0x03) return 1;
	return 0;
}

static int detect_32K_mode3F(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,numfound = 0;
	static const unsigned char signature[4] = { 0xa9, 0x0e, 0x85, 0x3f };
	if (state->m_cart_size >= 0x8000)
	{
		UINT8 *cart = CART;
		for (i = 0; i < state->m_cart_size - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature,sizeof signature))
			{
				numfound++;
			}
		}
	}
	if (numfound > 1) return 1;
	return 0;
}

static int detect_super_chip(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i,j;
	UINT8 *cart = CART;
	static const unsigned char signatures[][5] = {
									{ 0xa2, 0x7f, 0x9d, 0x00, 0xf0 }, // dig dug
									{ 0xae, 0xf6, 0xff, 0x4c, 0x00 } // off the wall
	};

	if (state->m_cart_size == 0x4000)
	{
		for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
		{
			for (j = 0; j < (sizeof signatures/sizeof signatures[0]); j++)
			{
				if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0]))
				{
					return 1;
				}
			}
		}
	}
	for (i = 0x1000; i < state->m_cart_size; i += 0x1000)
	{
		if (memcmp(cart, cart + i, 0x100))
		{
			return 0;
		}
	}
	/* Check the reset vector does not point into the super chip RAM area */
	i = ( cart[0x0FFD] << 8 ) | cart[0x0FFC];
	if ( ( i & 0x0FFF ) < 0x0100 )
	{
		return 0;
	}
	return 1;
}


static DEVICE_START( a2600_cart )
{
	a2600_state *state = device->machine().driver_data<a2600_state>();
	state->m_banking_mode = 0xff;
}


static DEVICE_IMAGE_LOAD( a2600_cart )
{
	a2600_state *state = image.device().machine().driver_data<a2600_state>();
	running_machine &machine = image.device().machine();
	UINT8 *cart = CART;

	if (image.software_entry() == NULL)
		state->m_cart_size = image.length();
	else
		state->m_cart_size = image.get_software_region_length("rom");

	switch (state->m_cart_size)
	{
	case 0x00800:
	case 0x01000:
	case 0x02000:
	case 0x028ff:
	case 0x02900:
	case 0x03000:
	case 0x04000:
	case 0x08000:
	case 0x10000:
	case 0x80000:
		break;

	default:
		image.seterror(IMAGE_ERROR_UNSUPPORTED, "Invalid rom file size" );
		return 1; /* unsupported image format */
	}

	state->m_current_bank = 0;

	if (image.software_entry() == NULL)
		image.fread(cart, state->m_cart_size);
	else
		memcpy(cart, image.get_software_region("rom"), state->m_cart_size);

	if (!(state->m_cart_size == 0x4000 && detect_modef6(image.device().machine())))
	{
		while (state->m_cart_size > 0x00800)
		{
			if (!memcmp(cart, &cart[state->m_cart_size/2],state->m_cart_size/2)) state->m_cart_size /= 2;
			else break;
		}
	}

	return 0;
}


static int next_bank(a2600_state *state)
{
	return state->m_current_bank = (state->m_current_bank + 1) % 16;
}


static void modeF8_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x1000 * offset;
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void modeFA_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x1000 * offset;
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void modeF6_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x1000 * offset;
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void modeF4_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x1000 * offset;
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void mode3F_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x800 * (data & (state->m_number_banks - 1));
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void modeUA_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + (offset >> 6) * 0x1000;
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void modeE0_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int bank = 1 + (offset >> 3);
	char bank_name[10];
	sprintf(bank_name,"bank%d",bank);
	state->m_bank_base[bank] = CART + 0x400 * (offset & 7);
	state->membank(bank_name)->set_base(state->m_bank_base[bank]);
}
static void modeE7_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x800 * offset;
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void modeE7_RAM_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->membank("bank9")->set_base(state->m_extra_RAM->base() + (4 + offset) * 256 );
}
static void modeDC_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x1000 * next_bank(state);
	state->membank("bank1")->set_base(state->m_bank_base[1]);
}
static void mode3E_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_bank_base[1] = CART + 0x800 * (data & (state->m_number_banks - 1));
	state->membank("bank1")->set_base(state->m_bank_base[1]);
	state->m_mode3E_ram_enabled = 0;
}
static void mode3E_RAM_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	state->m_ram_base = state->m_extra_RAM->base() + 0x200 * ( data & 0x3F );
	state->membank("bank1")->set_base(state->m_ram_base );
	state->m_mode3E_ram_enabled = 1;
}
static void modeFV_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	//printf("ModeFV %04x\n",offset);
	if (!state->m_FVlocked && ( machine.device("maincpu")->safe_pc() & 0x1F00 ) == 0x1F00 )
	{
		state->m_FVlocked = 1;
		state->m_current_bank = state->m_current_bank ^ 0x01;
		state->m_bank_base[1] = CART + 0x1000 * state->m_current_bank;
		state->membank("bank1")->set_base(state->m_bank_base[1]);
	}
}
static void modeJVP_switch(running_machine &machine, UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	switch( offset )
	{
	case 0x00:
	case 0x20:
		state->m_current_bank ^= 1;
		break;
	default:
		printf("%04X: write to unknown mapper address %02X\n", machine.device("maincpu")->safe_pc(), 0xfa0 + offset );
		break;
	}
	state->m_bank_base[1] = CART + 0x1000 * state->m_current_bank;
	state->membank("bank1")->set_base(state->m_bank_base[1] );
}


/* These read handlers will return the byte from the new bank */
READ8_MEMBER(a2600_state::modeF8_switch_r)
{
	modeF8_switch(machine(), offset, 0);
	return m_bank_base[1][0xff8 + offset];
}

READ8_MEMBER(a2600_state::modeFA_switch_r)
{
	modeFA_switch(machine(), offset, 0);
	return m_bank_base[1][0xff8 + offset];
}

READ8_MEMBER(a2600_state::modeF6_switch_r)
{
	modeF6_switch(machine(), offset, 0);
	return m_bank_base[1][0xff6 + offset];
}

READ8_MEMBER(a2600_state::modeF4_switch_r)
{
	modeF4_switch(machine(), offset, 0);
	return m_bank_base[1][0xff4 + offset];
}

READ8_MEMBER(a2600_state::modeE0_switch_r)
{
	modeE0_switch(machine(), offset, 0);
	return m_bank_base[4][0x3e0 + offset];
}

READ8_MEMBER(a2600_state::modeE7_switch_r)
{
	modeE7_switch(machine(), offset, 0);
	return m_bank_base[1][0xfe0 + offset];
}

READ8_MEMBER(a2600_state::modeE7_RAM_switch_r)
{
	modeE7_RAM_switch(machine(), offset, 0);
	return 0;
}

READ8_MEMBER(a2600_state::modeUA_switch_r)
{
	modeUA_switch(machine(), offset, 0);
	return 0;
}

READ8_MEMBER(a2600_state::modeDC_switch_r)
{
	modeDC_switch(machine(), offset, 0);
	return m_bank_base[1][0xff0 + offset];
}

READ8_MEMBER(a2600_state::modeFV_switch_r)
{
	modeFV_switch(machine(), offset, 0);
	return m_bank_base[1][0xfd0 + offset];
}

READ8_MEMBER(a2600_state::modeJVP_switch_r)
{
	modeJVP_switch(machine(), offset, 0);
	return m_riot_ram[ 0x20 + offset ];
}


WRITE8_MEMBER(a2600_state::modeF8_switch_w){ modeF8_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeFA_switch_w){ modeFA_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeF6_switch_w){ modeF6_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeF4_switch_w){ modeF4_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeE0_switch_w){ modeE0_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeE7_switch_w){ modeE7_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeE7_RAM_switch_w){ modeE7_RAM_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::mode3F_switch_w){ mode3F_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeUA_switch_w){ modeUA_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeDC_switch_w){ modeDC_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::mode3E_switch_w){ mode3E_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::mode3E_RAM_switch_w){ mode3E_RAM_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::mode3E_RAM_w)
{
	if ( m_mode3E_ram_enabled )
	{
		m_ram_base[offset] = data;
	}
}
WRITE8_MEMBER(a2600_state::modeFV_switch_w){ modeFV_switch(machine(), offset, data); }
WRITE8_MEMBER(a2600_state::modeJVP_switch_w)
{
	modeJVP_switch(machine(), offset, data); m_riot_ram[ 0x20 + offset ] = data;
}


DIRECT_UPDATE_MEMBER(a2600_state::modeF6_opbase)
{
	if ( ( address & 0x1FFF ) >= 0x1FF6 && ( address & 0x1FFF ) <= 0x1FF9 )
	{
		modeF6_switch_w(*machine().device("maincpu")->memory().space(AS_PROGRAM), ( address & 0x1FFF ) - 0x1FF6, 0 );
	}
	return address;
}

DIRECT_UPDATE_MEMBER(a2600_state::modeSS_opbase)
{
	if ( address & 0x1000 )
	{
		if ( address & 0x800 )
		{
			direct.explicit_configure(( address & 0xf800 ), ( address & 0xf800 ) | 0x7ff, 0x7ff, m_bank_base[2]);
		}
		else
		{
			direct.explicit_configure(( address & 0xf800 ), ( address & 0xf800 ) | 0x7ff, 0x7ff, m_bank_base[1]);
		}
		return ~0;
	}
	return address;
}

READ8_MEMBER(a2600_state::modeSS_r)
{
	UINT8 data = ( offset & 0x800 ) ? m_bank_base[2][offset & 0x7FF] : m_bank_base[1][offset];

	//logerror("%04X: read from modeSS area offset = %04X\n", machine().device("maincpu")->safe_pc(), offset);
	/* Check for control register "write" */
	if ( offset == 0xFF8 )
	{
		//logerror("%04X: write to modeSS control register data = %02X\n", machine().device("maincpu")->safe_pc(), m_modeSS_byte);
		m_modeSS_write_enabled = m_modeSS_byte & 0x02;
		m_modeSS_write_delay = m_modeSS_byte >> 5;
		switch ( m_modeSS_byte & 0x1C )
		{
		case 0x00:
			m_bank_base[1] = m_extra_RAM->base() + 2 * 0x800;
			m_bank_base[2] = ( m_modeSS_byte & 0x01 ) ? memregion("maincpu")->base() + 0x1800 : memregion("user1")->base();
			m_modeSS_high_ram_enabled = 0;
			break;
		case 0x04:
			m_bank_base[1] = m_extra_RAM->base();
			m_bank_base[2] = ( m_modeSS_byte & 0x01 ) ? memregion("maincpu")->base() + 0x1800 : memregion("user1")->base();
			m_modeSS_high_ram_enabled = 0;
			break;
		case 0x08:
			m_bank_base[1] = m_extra_RAM->base() + 2 * 0x800;
			m_bank_base[2] = m_extra_RAM->base();
			m_modeSS_high_ram_enabled = 1;
			break;
		case 0x0C:
			m_bank_base[1] = m_extra_RAM->base();
			m_bank_base[2] = m_extra_RAM->base() + 2 * 0x800;
			m_modeSS_high_ram_enabled = 1;
			break;
		case 0x10:
			m_bank_base[1] = m_extra_RAM->base() + 2 * 0x800;
			m_bank_base[2] = ( m_modeSS_byte & 0x01 ) ? memregion("maincpu")->base() + 0x1800 : memregion("user1")->base();
			m_modeSS_high_ram_enabled = 0;
			break;
		case 0x14:
			m_bank_base[1] = m_extra_RAM->base() + 0x800;
			m_bank_base[2] = ( m_modeSS_byte & 0x01 ) ? memregion("maincpu")->base() + 0x1800 : memregion("user1")->base();
			m_modeSS_high_ram_enabled = 0;
			break;
		case 0x18:
			m_bank_base[1] = m_extra_RAM->base() + 2 * 0x800;
			m_bank_base[2] = m_extra_RAM->base() + 0x800;
			m_modeSS_high_ram_enabled = 1;
			break;
		case 0x1C:
			m_bank_base[1] = m_extra_RAM->base() + 0x800;
			m_bank_base[2] = m_extra_RAM->base() + 2 * 0x800;
			m_modeSS_high_ram_enabled = 1;
			break;
		}
		membank("bank1")->set_base(m_bank_base[1] );
		membank("bank2")->set_base(m_bank_base[2] );
	}
	else if ( offset == 0xFF9 )
	{
		/* Cassette port read */
		double tap_val = machine().device<cassette_image_device>(CASSETTE_TAG)->input();
		//logerror("%04X: Cassette port read, tap_val = %f\n", machine().device("maincpu")->safe_pc(), tap_val);
		if ( tap_val < 0 )
		{
			data = 0x00;
		}
		else
		{
			data = 0x01;
		}
	}
	else
	{
		/* Possible RAM write */
		if ( m_modeSS_write_enabled )
		{
			int diff = machine().device<cpu_device>("maincpu")->total_cycles() - m_modeSS_byte_started;
			//logerror("%04X: offset = %04X, %d\n", machine().device("maincpu")->safe_pc(), offset, diff);
			if ( diff - m_modeSS_diff_adjust == 5 )
			{
				//logerror("%04X: RAM write offset = %04X, data = %02X\n", machine().device("maincpu")->safe_pc(), offset, m_modeSS_byte );
				if ( offset & 0x800 )
				{
					if ( m_modeSS_high_ram_enabled )
					{
						m_bank_base[2][offset & 0x7FF] = m_modeSS_byte;
						data = m_modeSS_byte;
					}
				}
				else
				{
					m_bank_base[1][offset] = m_modeSS_byte;
					data = m_modeSS_byte;
				}
			}
			else if ( offset < 0x0100 )
			{
				m_modeSS_byte = offset;
				m_modeSS_byte_started = machine().device<cpu_device>("maincpu")->total_cycles();
			}
			/* Check for dummy read from same address */
			if ( diff == 2 )
			{
				m_modeSS_diff_adjust = 1;
			}
			else
			{
				m_modeSS_diff_adjust = 0;
			}
		}
		else if ( offset < 0x0100 )
		{
			m_modeSS_byte = offset;
			m_modeSS_byte_started = machine().device<cpu_device>("maincpu")->total_cycles();
		}
	}
	/* Because the mame core caches opcode data and doesn't perform reads like normal */
	/* we have to put in this little hack here to get Suicide Mission to work. */
	if ( offset != 0xFF8 && ( machine().device("maincpu")->safe_pc() & 0x1FFF ) == 0x1FF8 )
	{
		modeSS_r( space, 0xFF8 );
	}
	return data;
}

void a2600_state::modeDPC_check_flag(UINT8 data_fetcher)
{
	/* Set flag when low counter equals top */
	if ( m_dpc.df[data_fetcher].low == m_dpc.df[data_fetcher].top )
	{
		m_dpc.df[data_fetcher].flag = 1;
	}
	/* Reset flag when low counter equals bottom */
	if ( m_dpc.df[data_fetcher].low == m_dpc.df[data_fetcher].bottom )
	{
		m_dpc.df[data_fetcher].flag = 0;
	}
}

void a2600_state::modeDPC_decrement_counter(UINT8 data_fetcher)
{
	m_dpc.df[data_fetcher].low -= 1;
	if ( m_dpc.df[data_fetcher].low == 0xFF )
	{
		m_dpc.df[data_fetcher].high -= 1;
		if ( data_fetcher > 4 && m_dpc.df[data_fetcher].music_mode )
		{
			m_dpc.df[data_fetcher].low = m_dpc.df[data_fetcher].top;
		}
	}

	modeDPC_check_flag(data_fetcher );
}

static TIMER_CALLBACK(modeDPC_timer_callback)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int data_fetcher;
	for( data_fetcher = 5; data_fetcher < 8; data_fetcher++ )
	{
		if ( state->m_dpc.df[data_fetcher].osc_clk )
		{
			state->modeDPC_decrement_counter(data_fetcher );
		}
	}
}

DIRECT_UPDATE_MEMBER(a2600_state::modeDPC_opbase_handler)
{
	UINT8	new_bit;
	new_bit = ( m_dpc.shift_reg & 0x80 ) ^ ( ( m_dpc.shift_reg & 0x20 ) << 2 );
	new_bit = new_bit ^ ( ( ( m_dpc.shift_reg & 0x10 ) << 3 ) ^ ( ( m_dpc.shift_reg & 0x08 ) << 4 ) );
	new_bit = new_bit ^ 0x80;
	m_dpc.shift_reg = new_bit | ( m_dpc.shift_reg >> 1 );
	return address;
}

READ8_MEMBER(a2600_state::modeDPC_r)
{
	static const UINT8 dpc_amplitude[8] = { 0x00, 0x04, 0x05, 0x09, 0x06, 0x0A, 0x0B, 0x0F };
	UINT8	data_fetcher = offset & 0x07;
	UINT8	data = 0xFF;

	logerror("%04X: Read from DPC offset $%02X\n", machine().device("maincpu")->safe_pc(), offset);
	if ( offset < 0x08 )
	{
		switch( offset & 0x06 )
		{
		case 0x00:		/* Random number generator */
		case 0x02:
			return m_dpc.shift_reg;
		case 0x04:		/* Sound value, MOVAMT value AND'd with Draw Line Carry; with Draw Line Add */
			m_dpc.latch_62 = m_dpc.latch_64;
		case 0x06:		/* Sound value, MOVAMT value AND'd with Draw Line Carry; without Draw Line Add */
			m_dpc.latch_64 = m_dpc.latch_62 + m_dpc.df[4].top;
			m_dpc.dlc = ( m_dpc.latch_62 + m_dpc.df[4].top > 0xFF ) ? 1 : 0;
			data = 0;
			if ( m_dpc.df[5].music_mode && m_dpc.df[5].flag )
			{
				data |= 0x01;
			}
			if ( m_dpc.df[6].music_mode && m_dpc.df[6].flag )
			{
				data |= 0x02;
			}
			if ( m_dpc.df[7].music_mode && m_dpc.df[7].flag )
			{
				data |= 0x04;
			}
			return ( m_dpc.dlc ? m_dpc.movamt & 0xF0 : 0 ) | dpc_amplitude[data];
		}
	}
	else
	{
		UINT8	display_data = memregion("user1")->base()[0x2000 + ( ~ ( ( m_dpc.df[data_fetcher].low | ( m_dpc.df[data_fetcher].high << 8 ) ) ) & 0x7FF ) ];

		switch( offset & 0x38 )
		{
		case 0x08:			/* display data */
			data = display_data;
			break;
		case 0x10:			/* display data AND'd w/flag */
			data = m_dpc.df[data_fetcher].flag ? display_data : 0x00;
			break;
		case 0x18:			/* display data AND'd w/flag, nibbles swapped */
			data = m_dpc.df[data_fetcher].flag ? BITSWAP8(display_data,3,2,1,0,7,6,5,4) : 0x00;
			break;
		case 0x20:			/* display data AND'd w/flag, byte reversed */
			data = m_dpc.df[data_fetcher].flag ? BITSWAP8(display_data,0,1,2,3,4,5,6,7) : 0x00;
			break;
		case 0x28:			/* display data AND'd w/flag, rotated right */
			data = m_dpc.df[data_fetcher].flag ? ( display_data >> 1 ) : 0x00;
			break;
		case 0x30:			/* display data AND'd w/flag, rotated left */
			data = m_dpc.df[data_fetcher].flag ? ( display_data << 1 ) : 0x00;
			break;
		case 0x38:			/* flag */
			data = m_dpc.df[data_fetcher].flag ? 0xFF : 0x00;
			break;
		}

		if ( data_fetcher < 5 || ! m_dpc.df[data_fetcher].osc_clk )
		{
			modeDPC_decrement_counter(data_fetcher );
		}
	}
	return data;
}

WRITE8_MEMBER(a2600_state::modeDPC_w)
{
	UINT8	data_fetcher = offset & 0x07;

	switch( offset & 0x38 )
	{
	case 0x00:			/* Top count */
		m_dpc.df[data_fetcher].top = data;
		m_dpc.df[data_fetcher].flag = 0;
		modeDPC_check_flag(data_fetcher );
		break;
	case 0x08:			/* Bottom count */
		m_dpc.df[data_fetcher].bottom = data;
		modeDPC_check_flag(data_fetcher );
		break;
	case 0x10:			/* Counter low */
		m_dpc.df[data_fetcher].low = data;
		if ( data_fetcher == 4 )
		{
			m_dpc.latch_64 = data;
		}
		if ( data_fetcher > 4 && m_dpc.df[data_fetcher].music_mode )
		{
			m_dpc.df[data_fetcher].low = m_dpc.df[data_fetcher].top;
		}
		modeDPC_check_flag(data_fetcher );
		break;
	case 0x18:			/* Counter high */
		m_dpc.df[data_fetcher].high = data;
		m_dpc.df[data_fetcher].music_mode = data & 0x10;
		m_dpc.df[data_fetcher].osc_clk = data & 0x20;
		if ( data_fetcher > 4 && m_dpc.df[data_fetcher].music_mode && m_dpc.df[data_fetcher].low == 0xFF )
		{
			m_dpc.df[data_fetcher].low = m_dpc.df[data_fetcher].top;
			modeDPC_check_flag(data_fetcher );
		}
		break;
	case 0x20:			/* Draw line movement value / MOVAMT */
		m_dpc.movamt = data;
		break;
	case 0x28:			/* Not used */
		logerror("%04X: Write to unused DPC register $%02X, data $%02X\n", machine().device("maincpu")->safe_pc(), offset, data);
		break;
	case 0x30:			/* Random number generator reset */
		m_dpc.shift_reg = 0;
		break;
	case 0x38:			/* Not used */
		logerror("%04X: Write to unused DPC register $%02X, data $%02X\n", machine().device("maincpu")->safe_pc(), offset, data);
		break;
	}
}

/*

There seems to be a kind of lag between the writing to address 0x1FE and the
Activision switcher springing into action. It waits for the next byte to arrive
on the data bus, which is the new PCH in the case of a JSR, and the PCH of the
stored PC on the stack in the case of an RTS.

depending on last byte & 0x20 -> 0x00 -> switch to bank #1
                              -> 0x20 -> switch to bank #0

 */


DIRECT_UPDATE_MEMBER(a2600_state::modeFE_opbase_handler)
{
	if ( ! m_FETimer )
	{
		/* Still cheating a bit here by looking bit 13 of the address..., but the high byte of the
           cpu should be the last byte that was on the data bus and so should determine the bank
           we should switch in. */
		m_bank_base[1] = memregion("user1")->base() + 0x1000 * ( ( address & 0x2000 ) ? 0 : 1 );
		membank("bank1")->set_base(m_bank_base[1] );
		/* and restore old opbase handler */
		machine().device("maincpu")->memory().space(AS_PROGRAM)->set_direct_update_handler(m_FE_old_opbase_handler);
	}
	else
	{
		/* Wait for one memory access to have passed (reading of new PCH either from code or from stack) */
		m_FETimer--;
	}
	return address;
}

static void modeFE_switch(running_machine &machine,UINT16 offset, UINT8 data)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	address_space& space = *machine.device("maincpu")->memory().space(AS_PROGRAM);
	/* Retrieve last byte read by the cpu (for this mapping scheme this
       should be the last byte that was on the data bus
    */
	state->m_FETimer = 1;
	state->m_FE_old_opbase_handler = space.set_direct_update_handler(direct_update_delegate(FUNC(a2600_state::modeFE_opbase_handler), state));
}

READ8_MEMBER(a2600_state::modeFE_switch_r)
{
	modeFE_switch(machine(),offset, 0 );
	return space.read_byte(0xFE );
}

WRITE8_MEMBER(a2600_state::modeFE_switch_w)
{
	space.write_byte(0xFE, data );
	modeFE_switch(machine(),offset, 0 );
}

READ8_MEMBER(a2600_state::current_bank_r)
{
	return m_current_bank;
}

static ADDRESS_MAP_START(a2600_mem, AS_PROGRAM, 8, a2600_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x007F) AM_MIRROR(0x0F00) AM_DEVREADWRITE("tia_video", tia_video_device, read, write)
	AM_RANGE(0x0080, 0x00FF) AM_MIRROR(0x0D00) AM_RAM AM_SHARE("riot_ram")
	AM_RANGE(0x0280, 0x029F) AM_MIRROR(0x0D00) AM_DEVREADWRITE_LEGACY("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x1FFF)                   AM_ROMBANK("bank1")
ADDRESS_MAP_END

static WRITE8_DEVICE_HANDLER(switch_A_w)
{
	a2600_state *state = device->machine().driver_data<a2600_state>();
	running_machine &machine = device->machine();

	/* Left controller port */
	if ( machine.root_device().ioport("CONTROLLERS")->read() / CATEGORY_SELECT == 0x03 )
	{
		state->m_keypad_left_column = data / 16;
	}

	/* Right controller port */
	switch( machine.root_device().ioport("CONTROLLERS")->read() % CATEGORY_SELECT )
	{
	case 0x03:	/* Keypad */
		state->m_keypad_right_column = data & 0x0F;
		break;
	case 0x0a:	/* KidVid voice module */
		machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(( data & 0x02 ) ? (cassette_state)CASSETTE_MOTOR_DISABLED : (cassette_state)(CASSETTE_MOTOR_ENABLED | CASSETTE_PLAY), (cassette_state)CASSETTE_MOTOR_DISABLED );
		break;
	}
}

static READ8_DEVICE_HANDLER( switch_A_r )
{
	static const UINT8 driving_lookup[4] = { 0x00, 0x02, 0x03, 0x01 };
	running_machine &machine = device->machine();
	UINT8 val = 0;

	/* Left controller port PINs 1-4 ( 4321 ) */
	switch( machine.root_device().ioport("CONTROLLERS")->read() / CATEGORY_SELECT )
	{
	case 0x00:  /* Joystick */
	case 0x05:	/* Joystick w/Boostergrip */
		val |= machine.root_device().ioport("SWA_JOY")->read() & 0xF0;
		break;
	case 0x01:  /* Paddle */
		val |= machine.root_device().ioport("SWA_PAD")->read() & 0xF0;
		break;
	case 0x02:	/* Driving */
		val |= 0xC0;
		val |= ( driving_lookup[ ( machine.root_device().ioport("WHEEL_L")->read() & 0x18 ) >> 3 ] << 4 );
		break;
	case 0x06:	/* Trakball CX-22 */
	case 0x07:	/* Trakball CX-80 / ST mouse */
	case 0x09:	/* Amiga mouse */
	default:
		val |= 0xF0;
		break;
	}

	/* Right controller port PINs 1-4 ( 4321 ) */
	switch( machine.root_device().ioport("CONTROLLERS")->read() % CATEGORY_SELECT )
	{
	case 0x00:	/* Joystick */
	case 0x05:	/* Joystick w/Boostergrip */
		val |= machine.root_device().ioport("SWA_JOY")->read() & 0x0F;
		break;
	case 0x01:	/* Paddle */
		val |= machine.root_device().ioport("SWA_PAD")->read() & 0x0F;
		break;
	case 0x02:	/* Driving */
		val |= 0x0C;
		val |= ( driving_lookup[ ( machine.root_device().ioport("WHEEL_R")->read() & 0x18 ) >> 3 ] );
		break;
	case 0x06:	/* Trakball CX-22 */
	case 0x07:	/* Trakball CX-80 / ST mouse */
	case 0x09:	/* Amiga mouse */
	default:
		val |= 0x0F;
		break;
	}

	return val;
}

static WRITE8_DEVICE_HANDLER(switch_B_w )
{
}

static WRITE_LINE_DEVICE_HANDLER( irq_callback )
{
}

static READ8_DEVICE_HANDLER( riot_input_port_8_r )
{
	return device->machine().root_device().ioport("SWB")->read();
}

static const riot6532_interface r6532_interface =
{
	DEVCB_HANDLER(switch_A_r),
	DEVCB_HANDLER(riot_input_port_8_r),
	DEVCB_HANDLER(switch_A_w),
	DEVCB_HANDLER(switch_B_w),
	DEVCB_LINE(irq_callback)
};


static void install_banks(running_machine &machine, int count, unsigned init)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	int i;
	UINT8 *cart = CART;

	for (i = 0; i < count; i++)
	{
		static const char *const handler[] =
		{
			"bank1",
			"bank2",
			"bank3",
			"bank4",
		};

		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(
			0x1000 + (i + 0) * 0x1000 / count - 0,
			0x1000 + (i + 1) * 0x1000 / count - 1, handler[i]);

		state->m_bank_base[i + 1] = cart + init;
		state->membank(handler[i])->set_base(state->m_bank_base[i + 1]);
	}
}

READ16_MEMBER(a2600_state::a2600_read_input_port)
{
	int i;

	switch( offset )
	{
	case 0:	/* Left controller port PIN 5 */
		switch ( ioport("CONTROLLERS")->read() / CATEGORY_SELECT )
		{
		case 0x00:	/* Joystick */
			return TIA_INPUT_PORT_ALWAYS_OFF;
		case 0x01:	/* Paddle */
			return ioport("PADDLE1")->read();
		case 0x03:	/* Keypad */
			for ( i = 0; i < 4; i++ )
			{
				if ( ! ( ( m_keypad_left_column >> i ) & 0x01 ) )
				{
					if ( ( ioport("KEYPAD_L")->read() >> 3*i ) & 0x01 )
					{
						return TIA_INPUT_PORT_ALWAYS_ON;
					}
					else
					{
						return TIA_INPUT_PORT_ALWAYS_OFF;
					}
				}
			}
			return TIA_INPUT_PORT_ALWAYS_ON;
		case 0x05:	/* Boostergrip joystick */
			return ( ioport("BUTTONS_L")->read() & 0x40 ) ? TIA_INPUT_PORT_ALWAYS_OFF : TIA_INPUT_PORT_ALWAYS_ON;
		default:
			return TIA_INPUT_PORT_ALWAYS_OFF;
		}
		break;
	case 1:	/* Right controller port PIN 5 */
		switch ( ioport("CONTROLLERS")->read() / CATEGORY_SELECT )
		{
		case 0x00:	/* Joystick */
			return TIA_INPUT_PORT_ALWAYS_OFF;
		case 0x01:	/* Paddle */
			return ioport("PADDLE3")->read();
		case 0x03:	/* Keypad */
			for ( i = 0; i < 4; i++ )
			{
				if ( ! ( ( m_keypad_left_column >> i ) & 0x01 ) )
				{
					if ( ( ioport("KEYPAD_L")->read() >> 3*i ) & 0x02 )
					{
						return TIA_INPUT_PORT_ALWAYS_ON;
					}
					else
					{
						return TIA_INPUT_PORT_ALWAYS_OFF;
					}
				}
			}
			return TIA_INPUT_PORT_ALWAYS_ON;
		case 0x05:	/* Joystick w/Boostergrip */
			return ( ioport("BUTTONS_L")->read() & 0x20 ) ? TIA_INPUT_PORT_ALWAYS_OFF : TIA_INPUT_PORT_ALWAYS_ON;
		default:
			return TIA_INPUT_PORT_ALWAYS_OFF;
		}
		break;
	case 2:	/* Left controller port PIN 9 */
		switch ( ioport("CONTROLLERS")->read() % CATEGORY_SELECT )
		{
		case 0x00:	/* Joystick */
			return TIA_INPUT_PORT_ALWAYS_OFF;
		case 0x01:	/* Paddle */
			return ioport("PADDLE2")->read();
		case 0x03:	/* Keypad */
			for ( i = 0; i < 4; i++ )
			{
				if ( ! ( ( m_keypad_right_column >> i ) & 0x01 ) )
				{
					if ( ( ioport("KEYPAD_R")->read() >> 3*i ) & 0x01 )
					{
						return TIA_INPUT_PORT_ALWAYS_ON;
					}
					else
					{
						return TIA_INPUT_PORT_ALWAYS_OFF;
					}
				}
			}
			return TIA_INPUT_PORT_ALWAYS_ON;
		case 0x05:	/* Joystick w/Boostergrip */
			return ( ioport("BUTTONS_R")->read() & 0x40 ) ? TIA_INPUT_PORT_ALWAYS_OFF : TIA_INPUT_PORT_ALWAYS_ON;
		default:
			return TIA_INPUT_PORT_ALWAYS_OFF;
		}
		break;
	case 3:	/* Right controller port PIN 9 */
		switch ( ioport("CONTROLLERS")->read() % CATEGORY_SELECT )
		{
		case 0x00:	/* Joystick */
			return TIA_INPUT_PORT_ALWAYS_OFF;
		case 0x01:	/* Paddle */
			return ioport("PADDLE4")->read();
		case 0x03:	/* Keypad */
			for ( i = 0; i < 4; i++ )
			{
				if ( ! ( ( m_keypad_right_column >> i ) & 0x01 ) )
				{
					if ( ( ioport("KEYPAD_R")->read() >> 3*i ) & 0x02 )
					{
						return TIA_INPUT_PORT_ALWAYS_ON;
					}
					else
					{
						return TIA_INPUT_PORT_ALWAYS_OFF;
					}
				}
			}
			return TIA_INPUT_PORT_ALWAYS_ON;
		case 0x05:	/* Joystick w/Boostergrip */
			return ( ioport("BUTTONS_R")->read() & 0x20 ) ? TIA_INPUT_PORT_ALWAYS_OFF : TIA_INPUT_PORT_ALWAYS_ON;
		default:
			return TIA_INPUT_PORT_ALWAYS_OFF;
		}
		break;
	case 4:	/* Left controller port PIN 6 */
		switch ( ioport("CONTROLLERS")->read() / CATEGORY_SELECT )
		{
		case 0x00:	/* Joystick */
		case 0x05:	/* Joystick w/Boostergrip */
			return ioport("BUTTONS_L")->read();
		case 0x01:	/* Paddle */
			return 0xff;
		case 0x02:	/* Driving */
			return ioport("BUTTONS_L")->read() << 3;
		case 0x03:	/* Keypad */
			for ( i = 0; i < 4; i++ )
			{
				if ( ! ( ( m_keypad_left_column >> i ) & 0x01 ) )
				{
					if ( ( ioport("KEYPAD_L")->read() >> 3*i ) & 0x04 )
					{
						return 0xff;
					}
					else
					{
						return 0x00;
					}
				}
			}
			return 0xff;
		case 0x06:	/* Trakball CX-22 */
			return ioport("BUTTONS_L")->read() << 4;
		default:
			return 0xff;
		}
		break;
	case 5:	/* Right controller port PIN 6 */
		switch ( ioport("CONTROLLERS")->read() % CATEGORY_SELECT )
		{
		case 0x00:	/* Joystick */
		case 0x05:	/* Joystick w/Boostergrip */
			return ioport("BUTTONS_R")->read();
		case 0x01:	/* Paddle */
			return 0xff;
		case 0x02:	/* Driving */
			return ioport("BUTTONS_R")->read() << 3;
		case 0x03:	/* Keypad */
			for ( i = 0; i < 4; i++ )
			{
				if ( ! ( ( m_keypad_right_column >> i ) & 0x01 ) )
				{
					if ( ( ioport("KEYPAD_R")->read() >> 3*i ) & 0x04 )
					{
						return 0xff;
					}
					else
					{
						return 0x00;
					}
				}
			}
			return 0xff;
		case 0x06:	/* Trakball CX-22 */
			return ioport("BUTTONS_R")->read() << 4;
		default:
			return 0xff;
		}
	}
	return TIA_INPUT_PORT_ALWAYS_OFF;
}

/* There are a few games that do an LDA ($80-$FF),Y instruction.
   The contents off the databus then depend on whatever was read
   from the RAM. To do this really properly the 6502 core would
   need to keep track of the last databus contents so we can query
   that. For now this is a quick hack to determine that value anyway.
   Examples:
   Q-Bert's Qubes (NTSC,F6) at 0x1594
   Berzerk at 0xF093.
*/
READ8_MEMBER(a2600_state::a2600_get_databus_contents)
{
	UINT16	last_address, prev_address;
	UINT8	last_byte, prev_byte;

	last_address = machine().device("maincpu")->safe_pc() - 1;
	if ( ! ( last_address & 0x1080 ) )
	{
		return offset;
	}
	last_byte = space.read_byte(last_address );
	if ( last_byte < 0x80 || last_byte == 0xFF )
	{
		return last_byte;
	}
	prev_address = last_address - 1;
	if ( ! ( prev_address & 0x1080 ) )
	{
		return last_byte;
	}
	prev_byte = space.read_byte(prev_address );
	if ( prev_byte == 0xB1 )
	{	/* LDA (XX),Y */
		return space.read_byte(last_byte + 1 );
	}
	return last_byte;
}

#if 0
static const rectangle visarea[4] = {
	{ 26, 26 + 160 + 16, 24, 24 + 192 + 31 },	/* 262 */
	{ 26, 26 + 160 + 16, 32, 32 + 228 + 31 },	/* 312 */
	{ 26, 26 + 160 + 16, 45, 45 + 240 + 31 },	/* 328 */
	{ 26, 26 + 160 + 16, 48, 48 + 240 + 31 }	/* 342 */
};
#endif

WRITE16_MEMBER(a2600_state::a2600_tia_vsync_callback)
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              machine.primary_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_NTSC ) * 228 * m_current_screen_height );
			}
		}
	}
}

WRITE16_MEMBER(a2600_state::a2600_tia_vsync_callback_pal)
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              machine.primary_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_PAL ) * 228 * m_current_screen_height );
			}
		}
	}
}

static const tia_interface a2600_tia_interface =
{
	DEVCB_DRIVER_MEMBER16(a2600_state, a2600_read_input_port),
	DEVCB_DRIVER_MEMBER(a2600_state, a2600_get_databus_contents),
	DEVCB_DRIVER_MEMBER16(a2600_state, a2600_tia_vsync_callback)
};

static const tia_interface a2600_tia_interface_pal =
{
	DEVCB_DRIVER_MEMBER16(a2600_state, a2600_read_input_port),
	DEVCB_DRIVER_MEMBER(a2600_state, a2600_get_databus_contents),
	DEVCB_DRIVER_MEMBER16(a2600_state, a2600_tia_vsync_callback_pal)
};


static void common_init(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
	screen_device *screen = machine.first_screen();
	state->m_current_screen_height = screen->height();
	state->m_extra_RAM = machine.memory().region_alloc("user2", 0x8600, 1, ENDIANNESS_LITTLE);
	memset( state->m_riot_ram, 0x00, 0x80 );
	state->m_current_reset_bank_counter = 0xFF;
	state->m_dpc.oscillator = machine.scheduler().timer_alloc(FUNC(modeDPC_timer_callback));
}

MACHINE_START_MEMBER(a2600_state,a2600)
{
	common_init(machine());
}

MACHINE_START_MEMBER(a2600_state,a2600p)
{
	common_init(machine());
}

#ifdef UNUSED_FUNCTIONS
// try to detect 2600 controller setup. returns 32bits with left/right controller info
static unsigned long detect_2600controllers(running_machine &machine)
{
	a2600_state *state = machine.driver_data<a2600_state>();
#define JOYS 0x001
#define PADD 0x002
#define KEYP 0x004
#define LGUN 0x008
#define INDY 0x010
#define BOOS 0x020
#define KVID 0x040
#define CMTE 0x080
#define MLNK 0x100
#define AMSE 0x200
#define CX22 0x400
#define CX80 0x800

	unsigned int left,right;
	int i,j,foundkeypad = 0;
	UINT8 *cart;
	static const unsigned char signatures[][5] =  {
		{ 0x55, 0xa5, 0x3c, 0x29, 0}, // star raiders
		{ 0xf9, 0xff, 0xa5, 0x80, 1}, // sentinel
		{ 0x81, 0x02, 0xe8, 0x86, 1}, // shooting arcade
		{ 0x02, 0xa9, 0xec, 0x8d, 1}, // guntest4 tester
		{ 0x85, 0x2c, 0x85, 0xa7, 2}, // INDY 500
		{ 0xa1, 0x8d, 0x9d, 0x02, 2}, // omega race INDY
		{ 0x65, 0x72, 0x44, 0x43, 2}, // Sprintmaster INDY
		{ 0x89, 0x8a, 0x99, 0xaa, 3}, // omega race
		{ 0x9a, 0x8e, 0x81, 0x02, 4},
		{ 0xdd, 0x8d, 0x80, 0x02, 4},
		{ 0x85, 0x8e, 0x81, 0x02, 4},
		{ 0x8d, 0x81, 0x02, 0xe6, 4},
		{ 0xff, 0x8d, 0x81, 0x02, 4},
		{ 0xa9, 0x03, 0x8d, 0x81, 5},
		{ 0xa9, 0x73, 0x8d, 0x80, 6},
		//                                  { 0x82, 0x02, 0x85, 0x8f, 7}, // Mind Maze (really Mind Link??)
		{ 0xa9, 0x30, 0x8d, 0x80, 7}, // Bionic Breakthrough
		{ 0x02, 0x8e, 0x81, 0x02, 7}, // Telepathy
		{ 0x41, 0x6d, 0x69, 0x67, 9}, // Missile Command Amiga Mouse
		{ 0x43, 0x58, 0x2d, 0x32, 10}, // Missile Command CX22 TrackBall
		{ 0x43, 0x58, 0x2d, 0x38, 11}, // Missile Command CX80 TrackBall
		{ 0x4e, 0xa8, 0xa4, 0xa2, 12}, // Omega Race for Joystick ONLY
		{ 0xa6, 0xef, 0xb5, 0x38, 8} // Warlords.. paddles ONLY
	};
	// start with this.. if anyone finds a game that does NOT work with both controllers enabled
	// it can be fixed here with a new signature (note that the Coleco Gemini has this setup also)
	left = JOYS+PADD; right = JOYS+PADD;
	// default for bad dumps and roms too large to have special controllers
	if ((state->m_cart_size > 0x4000) || (state->m_cart_size & 0x7ff)) return (left << 16) + right;

	cart = CART;
	for (i = 0; i < state->m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
	{
		for (j = 0; j < (sizeof signatures/sizeof signatures[0]); j++)
		{
			if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0] - 1))
			{
				int k = signatures[j][4];
				if (k == 0) return (JOYS << 16) + KEYP;
				if (k == 1) return (LGUN << 16);
				if (k == 2) return (INDY << 16) + INDY;
				if (k == 3) return (BOOS << 16) + BOOS;
				if (k == 5) return (JOYS << 16) + KVID;
				if (k == 6) return (CMTE << 16) + CMTE;
				if (k == 7) return (MLNK << 16) + MLNK;
				if (k == 8) return (PADD << 16) + PADD;
				if (k == 9) return (AMSE << 16) + AMSE;
				if (k == 10) return (CX22 << 16) + CX22;
				if (k == 11) return (CX80 << 16) + CX80;
				if (k == 12) return (JOYS << 16) + JOYS;
				if (k == 4) foundkeypad = 1;
			}
		}
	}
	if (foundkeypad) return (KEYP << 16) + KEYP;
	return (left << 16) + right;
}

static void set_category_value( running_machine &machine, const char* cat, const char *cat_selection )
{
	/* NPW 22-May-2008 - FIXME */
#if 0
	input_port_entry	*cat_in = NULL;
	input_port_entry	*in;

	for( in = machine.input_ports; in->type != IPT_END; in++ )
	{
		if ( in->type == IPT_CATEGORY_NAME && ! mame_stricmp( cat, input_port_name(in) ) )
		{
			cat_in = in;
		}
		if ( cat_in && in->type == IPT_CATEGORY_SETTING && ! mame_stricmp( cat_selection, input_port_name(in) ) )
		{
			cat_in->default_value = in->default_value;
			return;
		}
	}
#endif
}

static void set_controller( running_machine &machine, const char *controller, unsigned int selection )
{
	/* Defaulting to only joystick when joysstick and paddle are set for now... */
	if ( selection == JOYS + PADD )
		selection = JOYS;

	switch( selection )
	{
	case JOYS:	set_category_value( machine, controller, "Joystick" ); break;
	case PADD:	set_category_value( machine, controller, STR_PADDLES ); break;
	case KEYP:	set_category_value( machine, controller, STR_KEYPAD ); break;
	case LGUN:	set_category_value( machine, controller, STR_LIGHTGUN ); break;
	case INDY:	set_category_value( machine, controller, STR_DRIVING ); break;
	case BOOS:	set_category_value( machine, controller, STR_BOOSTERGRIP ); break;
	case KVID:	set_category_value( machine, controller, STR_KIDVID ); break;
	case CMTE:	break;
	case MLNK:	break;
	case AMSE:	set_category_value( machine, controller, STR_AMIGAMOUSE ); break;
	case CX22:	set_category_value( machine, controller, STR_CX22TRAKBALL ); break;
	case CX80:	set_category_value( machine, controller, STR_CX80TRAKBALL ); break;
	}
}
#endif

void a2600_state::machine_reset()
{
	address_space& space = *machine().device("maincpu")->memory().space(AS_PROGRAM);
	int chip = 0xFF;
	static const unsigned char snowwhite[] = { 0x10, 0xd0, 0xff, 0xff }; // Snow White Proto

	m_current_reset_bank_counter++;

#ifdef UNUSED_FUNCTIONS
	/* auto-detect special controllers */
	unsigned long controltemp;
	controltemp = detect_2600controllers(machine());
	set_controller( machine(), STR_LEFT_CONTROLLER, controltemp >> 16 );
	set_controller( machine(), STR_RIGHT_CONTROLLER, controltemp & 0xFFFF );
#endif

	/* auto-detect bank mode */
	if (m_banking_mode == 0xff) if (detect_modeDC(machine())) m_banking_mode = modeDC;
	if (m_banking_mode == 0xff) if (detect_mode3E(machine())) m_banking_mode = mode3E;
	if (m_banking_mode == 0xff) if (detect_modeFE(machine())) m_banking_mode = modeFE;
	if (m_banking_mode == 0xff) if (detect_modeSS(machine())) m_banking_mode = modeSS;
	if (m_banking_mode == 0xff) if (detect_modeE0(machine())) m_banking_mode = modeE0;
	if (m_banking_mode == 0xff) if (detect_modeCV(machine())) m_banking_mode = modeCV;
	if (m_banking_mode == 0xff) if (detect_modeFV(machine())) m_banking_mode = modeFV;
	if (m_banking_mode == 0xff) if (detect_modeJVP(machine())) m_banking_mode = modeJVP;
	if (m_banking_mode == 0xff) if (detect_modeUA(machine())) m_banking_mode = modeUA;
	if (m_banking_mode == 0xff) if (detect_8K_mode3F(machine())) m_banking_mode = mode3F;
	if (m_banking_mode == 0xff) if (detect_32K_mode3F(machine())) m_banking_mode = mode3F;
	if (m_banking_mode == 0xff) if (detect_modeE7(machine())) m_banking_mode = modeE7;

	if (m_banking_mode == 0xff)
	{
		switch (m_cart_size)
		{
		case 0x800:
			m_banking_mode = mode2K;
			break;
		case 0x1000:
			m_banking_mode = mode4K;
			break;
		case 0x2000:
			m_banking_mode = modeF8;
			break;
		case 0x28FF:
		case 0x2900:
			m_banking_mode = modeDPC;
			break;
		case 0x3000:
			m_banking_mode = modeFA;
			break;
		case 0x4000:
			m_banking_mode = modeF6;
			break;
		case 0x8000:
			m_banking_mode = modeF4;
			break;
		case 0x10000:
			m_banking_mode = mode32in1;
			break;
		case 0x80000:
			m_banking_mode = mode3F;
			break;
		}
	}

	/* auto-detect super chip */

	chip = 0;

	if (m_cart_size == 0x2000 || m_cart_size == 0x4000 || m_cart_size == 0x8000)
	{
		chip = detect_super_chip(machine());
	}

	/* Super chip games:
       dig dig, crystal castles, millipede, stargate, defender ii, jr. Pac Man,
       desert falcon, dark chambers, super football, sprintmaster, fatal run,
       off the wall, shooting arcade, secret quest, radar lock, save mary, klax
    */

	/* set up ROM banks */

	switch (m_banking_mode)
	{
	case mode2K:
		install_banks(machine(), 2, 0x0000);
		break;

	case mode4K:
		install_banks(machine(), 1, 0x0000);
		break;

	case modeF8:
		if (!memcmp(&CART_MEMBER[0x1ffc],snowwhite,sizeof(snowwhite)))
		{
			install_banks(machine(), 1, 0x0000);
		}
		else
		{
			install_banks(machine(), 1, 0x1000);
		}
		break;

	case modeFA:
		install_banks(machine(), 1, 0x2000);
		break;

	case modeF6:
		install_banks(machine(), 1, 0x0000);
		break;

	case modeF4:
		install_banks(machine(), 1, 0x7000);
		break;

	case modeFE:
		install_banks(machine(), 1, 0x0000);
		break;

	case modeE0:
		install_banks(machine(), 4, 0x1c00);
		break;

	case mode3F:
		install_banks(machine(), 2, m_cart_size - 0x800);
		m_number_banks = m_cart_size / 0x800;
		break;

	case modeUA:
		install_banks(machine(), 1, 0x1000);
		break;

	case modeE7:
		install_banks(machine(), 2, 0x3800);
		break;

	case modeDC:
		install_banks(machine(), 1, 0x1000 * m_current_bank);
		break;

	case modeCV:
		install_banks(machine(), 2, 0x0000);
		break;

	case mode3E:
		install_banks(machine(), 2, m_cart_size - 0x800);
		m_number_banks = m_cart_size / 0x800;
		m_mode3E_ram_enabled = 0;
		break;

	case modeSS:
		install_banks(machine(), 2, 0x0000);
		break;

	case modeFV:
		install_banks(machine(), 1, 0x0000);
		m_current_bank = 0;
		break;

	case modeDPC:
		install_banks(machine(), 1, 0x0000);
		break;

	case mode32in1:
		install_banks(machine(), 2, 0x0000);
		m_current_reset_bank_counter = m_current_reset_bank_counter & 0x1F;
		break;

	case modeJVP:
		m_current_reset_bank_counter = m_current_reset_bank_counter & 1;
		if ( m_cart_size == 0x2000 )
			m_current_reset_bank_counter = 0;
		m_current_bank = m_current_reset_bank_counter * 2;
		install_banks(machine(), 1, 0x1000 * m_current_bank);
		break;
	}

	/* set up bank counter */

	if (m_banking_mode == modeDC)
	{
		space.install_read_handler(0x1fec, 0x1fec, read8_delegate(FUNC(a2600_state::current_bank_r),this));
	}

	/* set up bank switch registers */

	switch (m_banking_mode)
	{
	case modeF8:
		space.install_write_handler(0x1ff8, 0x1ff9, write8_delegate(FUNC(a2600_state::modeF8_switch_w),this));
		space.install_read_handler(0x1ff8, 0x1ff9, read8_delegate(FUNC(a2600_state::modeF8_switch_r),this));
		break;

	case modeFA:
		space.install_write_handler(0x1ff8, 0x1ffa, write8_delegate(FUNC(a2600_state::modeFA_switch_w),this));
		space.install_read_handler(0x1ff8, 0x1ffa, read8_delegate(FUNC(a2600_state::modeFA_switch_r),this));
		break;

	case modeF6:
		space.install_write_handler(0x1ff6, 0x1ff9, write8_delegate(FUNC(a2600_state::modeF6_switch_w),this));
		space.install_read_handler(0x1ff6, 0x1ff9, read8_delegate(FUNC(a2600_state::modeF6_switch_r),this));
		space.set_direct_update_handler(direct_update_delegate(FUNC(a2600_state::modeF6_opbase), this));
		break;

	case modeF4:
		space.install_write_handler(0x1ff4, 0x1ffb, write8_delegate(FUNC(a2600_state::modeF4_switch_w),this));
		space.install_read_handler(0x1ff4, 0x1ffb, read8_delegate(FUNC(a2600_state::modeF4_switch_r),this));
		break;

	case modeE0:
		space.install_write_handler(0x1fe0, 0x1ff8, write8_delegate(FUNC(a2600_state::modeE0_switch_w),this));
		space.install_read_handler(0x1fe0, 0x1ff8, read8_delegate(FUNC(a2600_state::modeE0_switch_r),this));
		break;

	case mode3F:
		space.install_write_handler(0x00, 0x3f, write8_delegate(FUNC(a2600_state::mode3F_switch_w),this));
		break;

	case modeUA:
		space.install_write_handler(0x200, 0x27f, write8_delegate(FUNC(a2600_state::modeUA_switch_w),this));
		space.install_read_handler(0x200, 0x27f, read8_delegate(FUNC(a2600_state::modeUA_switch_r),this));
		break;

	case modeE7:
		space.install_write_handler(0x1fe0, 0x1fe7, write8_delegate(FUNC(a2600_state::modeE7_switch_w),this));
		space.install_read_handler(0x1fe0, 0x1fe7, read8_delegate(FUNC(a2600_state::modeE7_switch_r),this));
		space.install_write_handler(0x1fe8, 0x1feb, write8_delegate(FUNC(a2600_state::modeE7_RAM_switch_w),this));
		space.install_read_handler(0x1fe8, 0x1feb, read8_delegate(FUNC(a2600_state::modeE7_RAM_switch_r),this));
		space.install_readwrite_bank(0x1800, 0x18ff, "bank9");
		membank("bank9")->set_base(m_extra_RAM->base() + 4 * 256 );
		break;

	case modeDC:
		space.install_write_handler(0x1ff0, 0x1ff0, write8_delegate(FUNC(a2600_state::modeDC_switch_w),this));
		space.install_read_handler(0x1ff0, 0x1ff0, read8_delegate(FUNC(a2600_state::modeDC_switch_r),this));
		break;

	case modeFE:
		space.install_write_handler(0x01fe, 0x01fe, write8_delegate(FUNC(a2600_state::modeFE_switch_w),this));
		space.install_read_handler(0x01fe, 0x01fe, read8_delegate(FUNC(a2600_state::modeFE_switch_r),this));
		break;

	case mode3E:
		space.install_write_handler(0x3e, 0x3e, write8_delegate(FUNC(a2600_state::mode3E_RAM_switch_w),this));
		space.install_write_handler(0x3f, 0x3f, write8_delegate(FUNC(a2600_state::mode3E_switch_w),this));
		space.install_write_handler(0x1400, 0x15ff, write8_delegate(FUNC(a2600_state::mode3E_RAM_w),this));
		break;

	case modeSS:
		space.install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(a2600_state::modeSS_r),this));
		m_bank_base[1] = m_extra_RAM->base() + 2 * 0x800;
		m_bank_base[2] = CART_MEMBER;
		membank("bank1")->set_base(m_bank_base[1] );
		membank("bank2")->set_base(m_bank_base[2] );
		m_modeSS_write_enabled = 0;
		m_modeSS_byte_started = 0;
		space.set_direct_update_handler(direct_update_delegate(FUNC(a2600_state::modeSS_opbase), this));
		/* The Supercharger has no motor control so just enable it */
		machine().device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MOTOR_DISABLED );
		break;

	case modeFV:
		space.install_write_handler(0x1fd0, 0x1fd0, write8_delegate(FUNC(a2600_state::modeFV_switch_w),this));
		space.install_read_handler(0x1fd0, 0x1fd0, read8_delegate(FUNC(a2600_state::modeFV_switch_r),this));
		break;

	case modeDPC:
		space.install_read_handler(0x1000, 0x103f, read8_delegate(FUNC(a2600_state::modeDPC_r),this));
		space.install_write_handler(0x1040, 0x107f, write8_delegate(FUNC(a2600_state::modeDPC_w),this));
		space.install_write_handler(0x1ff8, 0x1ff9, write8_delegate(FUNC(a2600_state::modeF8_switch_w),this));
		space.install_read_handler(0x1ff8, 0x1ff9, read8_delegate(FUNC(a2600_state::modeF8_switch_r),this));
		space.set_direct_update_handler(direct_update_delegate(FUNC(a2600_state::modeDPC_opbase_handler), this));
		{
			int	data_fetcher;
			for( data_fetcher = 0; data_fetcher < 8; data_fetcher++ )
			{
				m_dpc.df[data_fetcher].osc_clk = 0;
				m_dpc.df[data_fetcher].flag = 0;
				m_dpc.df[data_fetcher].music_mode = 0;
			}
		}
		m_dpc.oscillator->adjust(attotime::from_hz(42000), 0, attotime::from_hz(42000));
		break;

	case mode32in1:
		membank("bank1")->set_base(CART_MEMBER + m_current_reset_bank_counter * 0x800 );
		membank("bank2")->set_base(CART_MEMBER + m_current_reset_bank_counter * 0x800 );
		break;

	case modeJVP:
		space.install_read_handler(0x0FA0, 0x0FC0, read8_delegate(FUNC(a2600_state::modeJVP_switch_r),this));
		space.install_write_handler(0x0FA0, 0x0FC0, write8_delegate(FUNC(a2600_state::modeJVP_switch_w),this));
		break;
	}

	/* set up extra RAM */

	if (m_banking_mode == modeFA)
	{
		space.install_write_bank(0x1000, 0x10ff, "bank9");
		space.install_read_bank(0x1100, 0x11ff, "bank9");

		membank("bank9")->set_base(m_extra_RAM->base());
	}

	if (m_banking_mode == modeCV)
	{
		space.install_write_bank(0x1400, 0x17ff, "bank9");
		space.install_read_bank(0x1000, 0x13ff, "bank9");

		membank("bank9")->set_base(m_extra_RAM->base());
	}

	if (chip)
	{
		space.install_write_bank(0x1000, 0x107f, "bank9");
		space.install_read_bank(0x1080, 0x10ff, "bank9");

		membank("bank9")->set_base(m_extra_RAM->base());
	}

	/* Banks may have changed, reset the cpu so it uses the correct reset vector */
	machine().device("maincpu")->reset();
}


static INPUT_PORTS_START( a2600 )

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_PLAYER(1) PORT_REVERSE PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x10)
	PORT_START("PADDLE3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_PLAYER(3) PORT_REVERSE PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x10)
	PORT_START("PADDLE2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_PLAYER(2) PORT_REVERSE PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x01)
	PORT_START("PADDLE4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_PLAYER(4) PORT_REVERSE PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x01)

	PORT_START("BUTTONS_L") /* left port button(s) */
//  PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x50)
//  PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x50)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x50)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x20)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x00)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x00)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x00)

	PORT_START("BUTTONS_R") /* right port button(s) */
//  PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x05)
//  PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x05)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x05)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x02)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x00)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x00)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x00)

	PORT_START("SWA_JOY") /* SWCHA joystick */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x00)
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x00)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x00)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x00)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x00)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x00)
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x00)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x00)

	PORT_START("SWA_PAD") /* SWCHA paddles */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x01)
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x01)
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x10)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x10)

	PORT_START("SWB") /* SWCHB */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Select Game")
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_DIPSETTING(    0x08, "Color" )
	PORT_DIPSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )

	PORT_START("CONTROLLERS")
	PORT_CONFNAME( 0xf0, 0x00, STR_LEFT_CONTROLLER )
	PORT_CONFSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_CONFSETTING(    0x10, STR_PADDLES )
	PORT_CONFSETTING(    0x20, STR_DRIVING )
	PORT_CONFSETTING(    0x30, STR_KEYPAD )
	//PORT_CONFSETTING(    0x40, STR_LIGHTGUN )
	PORT_CONFSETTING(    0x50, STR_BOOSTERGRIP )
	//PORT_CONFSETTING(    0x60, STR_CX22TRAKBALL )
	//PORT_CONFSETTING(    0x70, STR_CX80TRAKBALL )
	//PORT_CONFSETTING(    0x80, "CX-80 Trak-Ball (JS Mode)" )
	//PORT_CONFSETTING(    0x90, STR_AMIGAMOUSE )
	PORT_CONFNAME( 0x0f, 0x00, STR_RIGHT_CONTROLLER )
	PORT_CONFSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_CONFSETTING(    0x01, STR_PADDLES )
	PORT_CONFSETTING(    0x02, STR_DRIVING )
	PORT_CONFSETTING(    0x03, STR_KEYPAD )
	//PORT_CONFSETTING(    0x04, STR_LIGHTGUN )
	PORT_CONFSETTING(    0x05, STR_BOOSTERGRIP )
	//PORT_CONFSETTING(    0x06, STR_CX22TRAKBALL )
	//PORT_CONFSETTING(    0x07, STR_CX80TRAKBALL )
	//PORT_CONFSETTING(    0x08, "CX-80 Trak-Ball (JS Mode)" )
	//PORT_CONFSETTING(    0x09, STR_AMIGAMOUSE )
	PORT_CONFSETTING(    0x0a, STR_KIDVID )
	//PORT_CONFSETTING(    0x0b, "Save Key" )

	PORT_START("KEYPAD_L")	/* left keypad */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 1") PORT_CODE(KEYCODE_7_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 2") PORT_CODE(KEYCODE_8_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 3") PORT_CODE(KEYCODE_9_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 4") PORT_CODE(KEYCODE_4_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 5") PORT_CODE(KEYCODE_5_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 6") PORT_CODE(KEYCODE_6_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 7") PORT_CODE(KEYCODE_1_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 8") PORT_CODE(KEYCODE_2_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 9") PORT_CODE(KEYCODE_3_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left *") PORT_CODE(KEYCODE_0_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left 0") PORT_CODE(KEYCODE_DEL_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("left #") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x30)

	PORT_START("KEYPAD_R")  /* right keypad */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 1") PORT_CODE(KEYCODE_5) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 2") PORT_CODE(KEYCODE_6) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 3") PORT_CODE(KEYCODE_7) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 4") PORT_CODE(KEYCODE_R) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 5") PORT_CODE(KEYCODE_T) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 6") PORT_CODE(KEYCODE_Y) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 7") PORT_CODE(KEYCODE_F) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 8") PORT_CODE(KEYCODE_G) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 9") PORT_CODE(KEYCODE_H) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right *") PORT_CODE(KEYCODE_V) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right 0") PORT_CODE(KEYCODE_B) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("right #") PORT_CODE(KEYCODE_N) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x03)

	PORT_START("WHEEL_L")	/* left driving controller */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x20)

	PORT_START("WHEEL_R")	/* right driving controller */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x02)

	PORT_START("GUNX_L")	/* left light gun X */
//  PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR( X, 1.0, 0.0, 0 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x40)

	PORT_START("GUNY_L")	/* left light gun Y */
//  PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR( Y, 1.0, 0.0, 0 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x40)

	PORT_START("GUNX_R")	/* right light gun X */
//  PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR( X, 1.0, 0.0, 0 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x04)

	PORT_START("GUNY_R")	/* right light gun Y */
//  PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR( Y, 1.0, 0.0, 0 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x04)

	PORT_START("TRAKX_L")	/* left trak ball X */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x50)

	PORT_START("TRAKY_L")	/* left trak ball Y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_CONDITION("CONTROLLERS", 0xf0, EQUALS, 0x50)

	PORT_START("TRAKX_R")	/* right trak ball X */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x05)

	PORT_START("TRAKY_R")	/* right trak ball Y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(40) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_CONDITION("CONTROLLERS", 0x0f, EQUALS, 0x05)

INPUT_PORTS_END


static const cassette_interface a2600_cassette_interface =
{
	a26_cassette_formats,
	NULL,
	(cassette_state)(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	"a2600_cass",
	NULL
};

static MACHINE_CONFIG_FRAGMENT(a2600_cartslot)
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,a26")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_START(a2600_cart)
	MCFG_CARTSLOT_LOAD(a2600_cart)
	MCFG_CARTSLOT_INTERFACE("a2600_cart")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a2600")
	MCFG_SOFTWARE_LIST_ADD("cass_list","a2600_cass")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( a2600, a2600_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK_NTSC / 3)	/* actually M6507 */
	MCFG_CPU_PROGRAM_MAP(a2600_mem)

	MCFG_MACHINE_START_OVERRIDE(a2600_state,a2600)

	/* video hardware */
	MCFG_TIA_VIDEO_ADD("tia_video", a2600_tia_interface)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( MASTER_CLOCK_NTSC, 228, 26, 26 + 160 + 16, 262, 24 , 24 + 192 + 31 )
	MCFG_SCREEN_UPDATE_DEVICE("tia_video", tia_video_device, screen_update)

	MCFG_PALETTE_LENGTH( TIA_PALETTE_LENGTH )
	MCFG_PALETTE_INIT(tia_NTSC)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tia", TIA, MASTER_CLOCK_NTSC/114)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_RIOT6532_ADD("riot", MASTER_CLOCK_NTSC / 3, r6532_interface)
	MCFG_FRAGMENT_ADD(a2600_cartslot)
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "NTSC")
	MCFG_CASSETTE_ADD( CASSETTE_TAG, a2600_cassette_interface )
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( a2600p, a2600_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK_PAL / 3)    /* actually M6507 */
	MCFG_CPU_PROGRAM_MAP(a2600_mem)

	MCFG_MACHINE_START_OVERRIDE(a2600_state,a2600p)

	/* video hardware */
	MCFG_TIA_VIDEO_ADD("tia_video", a2600_tia_interface_pal)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( MASTER_CLOCK_PAL, 228, 26, 26 + 160 + 16, 312, 32, 32 + 228 + 31 )
	MCFG_SCREEN_UPDATE_DEVICE("tia_video", tia_video_device, screen_update)

	MCFG_PALETTE_LENGTH( TIA_PALETTE_LENGTH )
	MCFG_PALETTE_INIT(tia_PAL)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tia", TIA, MASTER_CLOCK_PAL/114)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_RIOT6532_ADD("riot", MASTER_CLOCK_PAL / 3, r6532_interface)
	MCFG_FRAGMENT_ADD(a2600_cartslot)
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "PAL")
	MCFG_CASSETTE_ADD( CASSETTE_TAG, a2600_cassette_interface )
MACHINE_CONFIG_END


ROM_START( a2600 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_FILL( 0x0000, 0x2000, 0xFF )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_FILL( 0x00000, 0x80000, 0xFF )
ROM_END

#define rom_a2600p rom_a2600

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY     FULLNAME */
CONS( 1977,	a2600,	0,		0,		a2600,	a2600, driver_device,	0,		"Atari",	"Atari 2600 (NTSC)" , 0)
CONS( 1978,	a2600p,	a2600,	0,		a2600p,	a2600, driver_device,	0,		"Atari",    "Atari 2600 (PAL)",   0)
