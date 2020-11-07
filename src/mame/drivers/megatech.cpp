// license:BSD-3-Clause
// copyright-holders:David Haywood, Barry Rodewald
/* Sega MegaTech

About MegaTech:

Megatech games are identical to their Genesis/SMS equivalents, however the Megatech cartridges contain
a BIOS rom with the game instructions.  The last part number of the bios ROM is the cart/game ID code.

The instruction rom appears to map at 0x300000 in the cart space.

In Megatech games your coins buy you time to play the game, how you perform in the game does not
matter, you can die and start a new game providing you still have time, likewise you can be playing
well and run out of time if you fail to insert more coins.  This is the same method Nintendo used
with their Playchoice 10 system.

The BIOS screen is based around SMS hardware, with an additional Z80 and SMS VDP chip not present on
a standard Genesis.

SMS games run on Megatech in the Genesis's SMS compatibility mode, where the Genesis Z80 becomes the
main CPU and the Genesis VDP acts in a mode mimicking the behavior of the SMS VDP. A pin on the carts
determines which mode the game runs in.

Additions will only be made to this driver if proof that the dumped set are original roms with original
Sega part numbers is given..


Sega Mega Tech Cartridges (Readme by Guru)
-------------------------

These are cart-based games for use with Sega Mega Tech hardware. There are 6 known types of carts. All carts
are very simple, almost exactly the same as Mega Play carts. They contain just 2 or 3 ROMs.
PCB 171-6215A has locations for 2 ROMs and is dated 1991. PCB 171-6215A is also used in Mega Play!
PCB 171-5782 has locations for 2 ROMs and is dated 1989.
PCB 171-5869A has locations for 3 ROMs and is dated 1989.
PCB 171-5834 has locations for 3 ROMs and is dated 1989.
PCB 171-5783 has locations for 2 ROMs and is dated 1989.
PCB 171-5784 has locations for 2 ROMs and is dated 1989. It also contains a custom Sega IC 315-5235

                                                                           |------------------------------- ROMs --------------------------------|
                                                                           |                                                                     |
Game                       PCB #       Sticker on PCB    Sticker on cart     IC1                          IC2                      IC3
-------------------------------------------------------------------------------------------------------------------------------------------------
Altered Beast              171-5782    837-6963-01       610-0239-01         MPR-12538F     (834200A)     EPR-12368-01   (27C256)  n/a
Space Harrier II           171-5782    837-6963-02       610-0239-02         MPR-11934      (834200)      EPR-12368-02   (27256)   n/a
Super Thunder Blade                                      610-0239-03
Great Golf                                               610-0239-04
Afterburner                                              610-0239-05
Out Run                    171-5783    837-6963-06       610-0239-06         MPR-11078      (Mask)        EPR-12368-06   (27256)   n/a
Alien Syndrome             171-5783    837-6963-07       610-0239-07         MPR-11194      (232011)      EPR-12368-07   (27256)   n/a
Shinobi                                                  610-0239-08
Fantasy Zone                                             610-0239-09
Afterburner                171-5784    837-6963-10       610-0239-10         315-5235       (custom)      MPR-11271-T    (834000)  EPR-12368-10 (27256)
Great Football             171-5783    837-6963-19       610-0239-19         MPR-10576F     (831000)      EPR-12368-19   (27256)   n/a
World Championship Soccer  171-5782    837-6963-21       610-0239-21         MPR-12607B     (uPD23C4000)  EPR-12368-21   (27256)   n/a
Tetris                     171-5834    837-6963-22       610-0239-22         MPR-12356F     (831000)      MPR-12357F     (831000)  EPR-12368-22 (27256)
Ghouls & Ghosts            171-5869A   -                 610-0239-23         MPR-12605      (40 pins)     MPR-12606      (40 pins) EPR-12368-23 (27256)
Super Hang On              171-5782    837-6963-24       610-0239-24         MPR-12640      (234000)      EPR-12368-24   (27256)   n/a
Forgotten Worlds           171-5782    837-6963-26       610-0239-26         MPR-12672-H    (Mask)        EPR-12368-26   (27256)   n/a
The Revenge Of Shinobi     171-5782    837-6963-28       610-0239-28         MPR-12675 S44  (uPD23C4000)  EPR-12368-28   (27C256)  n/a
Arnold Palmer Tour Golf    171-5782    837-6963-31       610-0239-31         MPR-12645F     (834200A)     EPR-12368-31   (27256)   n/a
Super Real Basket Ball     171-5782    837-6963-32       610-0239-32         MPR-12904F     (838200A)     EPR-12368-32   (27256)   n/a
Tommy Lasorda Baseball     171-5782    837-6963-35       610-0239-35         MPR-12706F     (834200A)     EPR-12368-35   (27256)   n/a
ESWAT                      171-5782    837-6963-38       610-0239-38         MPR-13192-H    (uPD23C4000)  EPR-12368-38   (27256)   n/a
Moonwalker                 171-5782    837-6963-40       610-0239-40         MPR-13285A S61 (uPD23C4000)  EPR-12368-40   (27256)   n/a
Shadow Dancer              171-5782    837-6963-43       610-0239-43         MPR-13571-S    (uPD23C4000)  EPR-12368-43   (27256)   n/a
Wrestle War                171-5782    837-6963-48       610-0239-48         MPR-14025-F    (834200A)     EPR-12368-48   (27256)   n/a
Bonanza Bros.              171-5782    837-6963-49       610-0239-49         MPR-13905A-F   (834200A)     EPR-12368-49   (27256)   n/a
Streets of Rage            171-5782    837-6963-51       610-0239-51         MPR-14125-SM   (uPD23C4000)  EPR-12368-51   (27C256)  n/a
Sonic The Hedgehog         171-5782    837-6963-52       610-0239-52         MPR-13913-F    (834200A)     EPR-12368-52   (27C256)  n/a
Spider-Man                 171-5782    837-6963-54       610-0239-54         MPR-14027-SM   (uPD23C4000)  EPR-12368-54   (27C256)  n/a
California Games           171-5834    837-6963-55-01    610-0239-55         EPR-14494      (27C020)      EPR-14495      (27C020)  EPR-12368-55 (27C256)
Mario Lemeux Hockey        171-5782    837-6963-59       610-0239-59         MPR-14376-H    (234000)      EPR-12368-59   (27256)   n/a
Turbo Outrun               171-5782    837-6963-61       610-0239-61         MPR-14674      (uPD23C4000)  EPR-12368-61   (27256)   n/a
Sonic Hedgehog 2           171-6215A   837-6963-62       610-0239-62         MPR-15000A-F   (838200)      EPR-12368-62   (27256)   n/a

*/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/cxd1095.h"
#include "rendlay.h"

#include "includes/megadriv.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist.h"

#define MASTER_CLOCK        53693100



namespace {

class mtech_state : public md_base_state
{
public:
	mtech_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
		m_vdp1(*this, "vdp1"),
		m_cart1(*this, "mt_slot1"),
		m_cart2(*this, "mt_slot2"),
		m_cart3(*this, "mt_slot3"),
		m_cart4(*this, "mt_slot4"),
		m_cart5(*this, "mt_slot5"),
		m_cart6(*this, "mt_slot6"),
		m_cart7(*this, "mt_slot7"),
		m_cart8(*this, "mt_slot8"),
		m_bioscpu(*this, "mtbios"),
		m_region_maincpu(*this, "maincpu")
	{ }

	void megatech_multislot(machine_config &config);
	void megatech_fixedslot(machine_config &config);

	void init_mt_crt();
	void init_mt_slot();

protected:
	virtual void machine_reset() override;

private:

	void megatech(machine_config &config);

	void cart_select_w(uint8_t data);
	uint8_t bios_portc_r();
	uint8_t bios_porte_r();
	void bios_portd_w(uint8_t data);
	void bios_porte_w(uint8_t data);
	uint8_t read_68k_banked_data(offs_t offset);
	void write_68k_banked_data(offs_t offset, uint8_t data);
	void mt_z80_bank_w(uint8_t data);
	uint8_t banked_ram_r(offs_t offset);
	void banked_ram_w(offs_t offset, uint8_t data);
	void bios_port_ctrl_w(uint8_t data);
	uint8_t bios_joypad_r(offs_t offset);
	void bios_port_7f_w(uint8_t data);
	uint8_t vdp1_count_r(offs_t offset);
	u8 sms_count_r(offs_t offset);
	uint8_t sms_ioport_dc_r();
	uint8_t sms_ioport_dd_r();
	void mt_sms_standard_rom_bank_w(address_space &space, offs_t offset, uint8_t data);


	image_init_result load_cart(device_image_interface &image, generic_slot_device *slot, int gameno);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart1 ) { return load_cart(image, m_cart1, 0); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart2 ) { return load_cart(image, m_cart2, 1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart3 ) { return load_cart(image, m_cart3, 2); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart4 ) { return load_cart(image, m_cart4, 3); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart5 ) { return load_cart(image, m_cart5, 4); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart6 ) { return load_cart(image, m_cart6, 5); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart7 ) { return load_cart(image, m_cart7, 6); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mt_cart8 ) { return load_cart(image, m_cart8, 7); }

	uint32_t screen_update_main(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_menu(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_main);

	void megatech_bios_map(address_map &map);
	void megatech_bios_portmap(address_map &map);

	uint8_t m_mt_cart_select_reg;
	uint32_t m_bios_port_ctrl;
	int m_current_machine_is_sms; // is the current game SMS based (running on genesis z80, in VDP compatibility mode)
	uint32_t m_bios_ctrl_inputs;
	int m_mt_bank_addr;

	int m_cart_is_genesis[8];

	void set_genz80_as_md();
	void set_genz80_as_sms();

	void switch_cart(int gameno);

	std::unique_ptr<uint8_t[]> m_banked_ram;
	std::unique_ptr<uint8_t[]> sms_mainram;
	std::unique_ptr<uint8_t[]> sms_rom;

	required_device<sega315_5124_device> m_vdp1;
	required_device<generic_slot_device> m_cart1;
	optional_device<generic_slot_device> m_cart2;
	optional_device<generic_slot_device> m_cart3;
	optional_device<generic_slot_device> m_cart4;
	optional_device<generic_slot_device> m_cart5;
	optional_device<generic_slot_device> m_cart6;
	optional_device<generic_slot_device> m_cart7;
	optional_device<generic_slot_device> m_cart8;
	required_device<cpu_device>          m_bioscpu;
	required_memory_region               m_region_maincpu;

	memory_region *m_cart_reg[8];
};



/* not currently used */
static INPUT_PORTS_START( megatech ) /* Genesis Input Ports */
	PORT_INCLUDE(megadriv)

	PORT_START("BIOS_IN0") // port 6
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Select") PORT_CODE(KEYCODE_0)
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
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("BIOS_IN1") // port 6
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_COIN1 )  // a few coin inputs here
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service coin") PORT_CODE(KEYCODE_9)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Enter") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("BIOS_DSW0")
	PORT_DIPNAME( 0x02, 0x02, "Coin slot 3" )
	PORT_DIPSETTING(    0x00, "Inhibit" )
	PORT_DIPSETTING(    0x02, "Accept" )
	PORT_DIPNAME( 0x01, 0x01, "Coin slot 4" )
	PORT_DIPSETTING(    0x00, "Inhibit" )
	PORT_DIPSETTING(    0x01, "Accept" )
	PORT_DIPNAME( 0x1c, 0x1c, "Coin slot 3/4 value" )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 credits" )
	PORT_DIPNAME( 0xe0, 0x60, "Coin slot 2 value" )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Inhibit" )

	PORT_START("BIOS_DSW1")
	PORT_DIPNAME( 0x0f, 0x01, "Coin Slot 1 value" )
	PORT_DIPSETTING(    0x00, "Inhibit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x0a, "1 coin/10 credits" )
	PORT_DIPSETTING(    0x0b, "1 coin/11 credits" )
	PORT_DIPSETTING(    0x0c, "1 coin/12 credits" )
	PORT_DIPSETTING(    0x0d, "1 coin/13 credits" )
	PORT_DIPSETTING(    0x0e, "1 coin/14 credits" )
	PORT_DIPSETTING(    0x0f, "1 coin/15 credits" )
	PORT_DIPNAME( 0xf0, 0xa0, "Time per credit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, "7:30" )
	PORT_DIPSETTING(    0x20, "7:00" )
	PORT_DIPSETTING(    0x30, "6:30" )
	PORT_DIPSETTING(    0x40, "6:00" )
	PORT_DIPSETTING(    0x50, "5:30" )
	PORT_DIPSETTING(    0x60, "5:00" )
	PORT_DIPSETTING(    0x70, "4:30" )
	PORT_DIPSETTING(    0x80, "4:00" )
	PORT_DIPSETTING(    0x90, "3:30" )
	PORT_DIPSETTING(    0xa0, "3:00" )
	PORT_DIPSETTING(    0xb0, "2:30" )
	PORT_DIPSETTING(    0xc0, "2:00" )
	PORT_DIPSETTING(    0xd0, "1:30" )
	PORT_DIPSETTING(    0xe0, "1:00" )
	PORT_DIPSETTING(    0xf0, "0:30" )


	PORT_START("BIOS_J1")
	PORT_DIPNAME( 0x0001, 0x0001, "5" )
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
INPUT_PORTS_END

/* MEGATECH specific */

u8 mtech_state::sms_count_r(offs_t offset)
{
	if (offset & 0x01)
		return m_vdp->hcount_read();
	else
		return m_vdp->vcount_read();
}

uint8_t mtech_state::sms_ioport_dc_r()
{
	/* 2009-05 FP: would it be worth to give separate inputs to SMS? SMS has only 2 keys A,B (which are B,C on megadrive) */
	/* bit 4: TL-A; bit 5: TR-A */
	return (machine().root_device().ioport("PAD1")->read() & 0x3f) | ((machine().root_device().ioport("PAD2")->read() & 0x03) << 6);
}

uint8_t mtech_state::sms_ioport_dd_r()
{
	/* 2009-05 FP: would it be worth to give separate inputs to SMS? SMS has only 2 keys A,B (which are B,C on megadrive) */
	/* bit 2: TL-B; bit 3: TR-B; bit 4: RESET; bit 5: unused; bit 6: TH-A; bit 7: TH-B*/
	return ((machine().root_device().ioport("PAD2")->read() & 0x3c) >> 2) | 0x10;
}


void mtech_state::mt_sms_standard_rom_bank_w(address_space &space, offs_t offset, uint8_t data)
{
	int bank = data & 0x1f;
	//logerror("bank w %02x %02x\n", offset, data);

	sms_mainram[0x1ffc + offset] = data;
	switch (offset)
	{
		case 0:
			logerror("bank w %02x %02x\n", offset, data);
			space.install_rom(0x0000, 0xbfff, sms_rom.get());
			space.unmap_write(0x0000, 0xbfff);
			//printf("bank ram??\n");
			break;
		case 1:
			memcpy(sms_rom.get()+0x0000, m_region_maincpu->base()+bank*0x4000, 0x4000);
			break;
		case 2:
			memcpy(sms_rom.get()+0x4000, m_region_maincpu->base()+bank*0x4000, 0x4000);
			break;
		case 3:
			memcpy(sms_rom.get()+0x8000, m_region_maincpu->base()+bank*0x4000, 0x4000);
			break;

	}
}

void mtech_state::set_genz80_as_sms()
{
	address_space &prg = m_z80snd->space(AS_PROGRAM);
	address_space &io = m_z80snd->space(AS_IO);

	// main ram area
	sms_mainram = std::make_unique<uint8_t[]>(0x2000);
	prg.install_ram(0xc000, 0xdfff, 0x2000, sms_mainram.get());
	memset(sms_mainram.get(), 0x00, 0x2000);

	// fixed rom bank area
	sms_rom = std::make_unique<uint8_t[]>(0xc000);
	prg.install_rom(0x0000, 0xbfff, sms_rom.get());

	memcpy(sms_rom.get(), m_region_maincpu->base(), 0xc000);

	prg.install_write_handler(0xfffc, 0xffff, write8m_delegate(*this, FUNC(mtech_state::mt_sms_standard_rom_bank_w)));

	// ports
	io.install_read_handler      (0x40, 0x41, 0, 0x3e, 0, read8sm_delegate(*this, FUNC(mtech_state::sms_count_r)));
	io.install_write_handler     (0x40, 0x41, 0, 0x3e, 0, write8smo_delegate(*m_vdp, FUNC(sega315_5124_device::psg_w)));
	io.install_readwrite_handler (0x80, 0x80, 0, 0x3e, 0, read8smo_delegate(*m_vdp, FUNC(sega315_5124_device::data_read)), write8smo_delegate(*m_vdp, FUNC(sega315_5124_device::data_write)));
	io.install_readwrite_handler (0x81, 0x81, 0, 0x3e, 0, read8smo_delegate(*m_vdp, FUNC(sega315_5124_device::control_read)), write8smo_delegate(*m_vdp, FUNC(sega315_5124_device::control_write)));

	io.install_read_handler      (0x10, 0x10, read8smo_delegate(*this, FUNC(mtech_state::sms_ioport_dd_r))); // super tetris

	io.install_read_handler      (0xdc, 0xdc, read8smo_delegate(*this, FUNC(mtech_state::sms_ioport_dc_r)));
	io.install_read_handler      (0xdd, 0xdd, read8smo_delegate(*this, FUNC(mtech_state::sms_ioport_dd_r)));
	io.install_read_handler      (0xde, 0xde, read8smo_delegate(*this, FUNC(mtech_state::sms_ioport_dd_r)));
	io.install_read_handler      (0xdf, 0xdf, read8smo_delegate(*this, FUNC(mtech_state::sms_ioport_dd_r))); // adams family
}


/* sets the megadrive z80 to it's normal ports / map */
void mtech_state::set_genz80_as_md()
{
	address_space &prg = m_z80snd->space(AS_PROGRAM);

	prg.install_ram(0x0000, 0x1fff, m_genz80.z80_prgram.get());

	prg.install_readwrite_handler(0x4000, 0x4003, read8sm_delegate(*m_ymsnd, FUNC(ym2612_device::read)), write8sm_delegate(*m_ymsnd, FUNC(ym2612_device::write)));
	prg.install_write_handler    (0x6000, 0x6000, write8smo_delegate(*this, FUNC(mtech_state::megadriv_z80_z80_bank_w)));
	prg.install_write_handler    (0x6001, 0x6001, write8smo_delegate(*this, FUNC(mtech_state::megadriv_z80_z80_bank_w)));
	prg.install_read_handler     (0x6100, 0x7eff, read8smo_delegate(*this, FUNC(mtech_state::megadriv_z80_unmapped_read)));
	prg.install_readwrite_handler(0x7f00, 0x7fff, read8sm_delegate(*this, FUNC(mtech_state::megadriv_z80_vdp_read)), write8sm_delegate(*this, FUNC(mtech_state::megadriv_z80_vdp_write)));
	prg.install_readwrite_handler(0x8000, 0xffff, read8sm_delegate(*this, FUNC(mtech_state::z80_read_68k_banked_data)), write8sm_delegate(*this, FUNC(mtech_state::z80_write_68k_banked_data)));
}


void mtech_state::switch_cart(int gameno)
{
	logerror("select game %d\n", gameno + 1);

	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_z80snd->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	//m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	//m_z80snd->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_ymsnd->reset();

	megadriv_stop_scanline_timer();// stop the scanline timer for the genesis vdp... it can be restarted in video eof when needed
	m_vdp->reset();

	/* if the regions exist we're fine */
	if (m_cart_reg[gameno])
	{
		memcpy(m_region_maincpu->base(), m_cart_reg[gameno]->base(), 0x400000);

		if (!m_cart_is_genesis[gameno])
		{
			logerror("enabling SMS Z80\n");
			m_current_machine_is_sms = 1;
			set_genz80_as_sms();
			//m_z80snd->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_z80snd->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}
		else
		{
			logerror("disabling SMS Z80\n");
			m_current_machine_is_sms = 0;
			set_genz80_as_md();
			m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			//m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		}
	}
	else    /* else, no cart.. */
	{
		memset(memregion("mtbios")->base() + 0x8000, 0x00, 0x8000);
		memset(m_region_maincpu->base(), 0x00, 0x400000);
	}
}

void mtech_state::cart_select_w(uint8_t data)
{
	/* seems to write the slot number..
	  but it stores something in (banked?) ram
	  because it always seems to show the
	  same instructions ... */
	m_mt_cart_select_reg = data;
	switch_cart(m_mt_cart_select_reg);
}


uint8_t mtech_state::bios_portc_r()
{
	return 0;
}

uint8_t mtech_state::bios_porte_r()
{
	return 0;
}

void mtech_state::bios_portd_w(uint8_t data)
{
	output().set_value("Alarm_sound", BIT(data, 7));
	m_bios_ctrl_inputs = data & 0x04;  // Genesis/SMS input ports disable bit
}

void mtech_state::bios_porte_w(uint8_t data)
{
	output().set_value("Flash_screen", BIT(data, 1));
}

/* this sets 0x300000 which may indicate that the 68k can see the instruction rom
   there, this limiting the max game rom capacity to 3meg. */

uint8_t mtech_state::read_68k_banked_data(offs_t offset)
{
	address_space &space68k = m_maincpu->space();
	uint8_t ret = space68k.read_byte(m_mt_bank_addr + offset);
	return ret;
}

void mtech_state::write_68k_banked_data(offs_t offset, uint8_t data)
{
	address_space &space68k = m_maincpu->space();
	space68k.write_byte(m_mt_bank_addr + offset,data);
}

void mtech_state::mt_z80_bank_w(uint8_t data)
{
	m_mt_bank_addr = ((m_mt_bank_addr >> 1) | (data << 23)) & 0xff8000;
}

uint8_t mtech_state::banked_ram_r(offs_t offset)
{
	return m_banked_ram[offset + 0x1000 * (m_mt_cart_select_reg & 0x07)];
}

void mtech_state::banked_ram_w(offs_t offset, uint8_t data)
{
	m_banked_ram[offset + 0x1000 * (m_mt_cart_select_reg & 0x07)] = data;
}



void mtech_state::megatech_bios_map(address_map &map)
{
	map(0x0000, 0x2fff).rom(); // from bios rom (0x0000-0x2fff populated in ROM)
	map(0x3000, 0x3fff).rw(FUNC(mtech_state::banked_ram_r), FUNC(mtech_state::banked_ram_w)); // copies instruction data here at startup, must be banked
	map(0x4000, 0x5fff).ram(); // plain ram?
	map(0x6000, 0x6000).w(FUNC(mtech_state::mt_z80_bank_w));
	map(0x6400, 0x6407).rw("io1", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	map(0x6800, 0x6807).rw("io2", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	map(0x7000, 0x77ff).rom(); // from bios rom (0x7000-0x77ff populated in ROM)
	//map(0x7800, 0x7fff).ram(); // ?
	map(0x8000, 0x9fff).rw(FUNC(mtech_state::read_68k_banked_data), FUNC(mtech_state::write_68k_banked_data)); // window into 68k address space, reads instr rom and writes to reset banks on z80 carts?
}


void mtech_state::bios_port_ctrl_w(uint8_t data)
{
	m_bios_port_ctrl = data;
}

/* the test mode accesses the joypad/stick inputs like this */
uint8_t mtech_state::bios_joypad_r(offs_t offset)
{
	uint8_t retdata = 0;

	if (m_bios_port_ctrl == 0x55)
	{
		/* A keys */
		retdata = ((m_io_pad_3b[0]->read() & 0x40) >> 2) | ((m_io_pad_3b[1]->read() & 0x40) >> 4) | 0xeb;
	}
	else
	{
		if (offset == 0)
			retdata = (m_io_pad_3b[0]->read() & 0x3f) | ((m_io_pad_3b[1]->read() & 0x03) << 6);
		else
			retdata = ((m_io_pad_3b[1]->read() & 0x3c) >> 2) | 0xf0;

	}
	return retdata;
}

void mtech_state::bios_port_7f_w(uint8_t data)
{
//  popmessage("CPU #3: I/O port 0x7F write, data %02x", data);
}


uint8_t mtech_state::vdp1_count_r(offs_t offset)
{
	if (offset & 0x01)
		return m_vdp1->hcount_read();
	else
		return m_vdp1->vcount_read();
}

void mtech_state::megatech_bios_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x3f, 0x3f).w(FUNC(mtech_state::bios_port_ctrl_w));
	map(0x7f, 0x7f).w(FUNC(mtech_state::bios_port_7f_w)); // PSG?

	map(0x40, 0x41).mirror(0x3e).r(FUNC(mtech_state::vdp1_count_r));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp1, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp1, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));

	map(0xdc, 0xdd).r(FUNC(mtech_state::bios_joypad_r));  // player inputs
}



void mtech_state::init_mt_slot()
{
	m_banked_ram = std::make_unique<uint8_t[]>(0x1000*8);

	init_megadriv();

	// this gets set in DEVICE_IMAGE_LOAD
	memset(m_cart_is_genesis, 0, sizeof(m_cart_is_genesis));
}

void mtech_state::init_mt_crt()
{
	uint8_t* pin = memregion("sms_pin")->base();
	init_mt_slot();

	m_cart_is_genesis[0] = !pin[0] ? 1 : 0;
}


uint32_t mtech_state::screen_update_main(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// if we're running an sms game then use the SMS update.. maybe this should be moved to the megadrive emulation core as compatibility mode is a feature of the chip
	if (!m_current_machine_is_sms)
		screen_update_megadriv(screen, bitmap, cliprect);
	else
	{
		m_vdp->screen_update(screen, bitmap, cliprect);
#if 0
		// when launching megatech + both sms and megadrive games, the following would be needed...
		for (int y = 0; y < 224; y++)
		{
			uint32_t* lineptr = &bitmap.pix(y);
			uint32_t* srcptr =  &m_vdp->get_bitmap().pix(y + sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT);

			for (int x = 0; x < sega315_5124_device::WIDTH; x++)
				lineptr[x] = srcptr[x];
		}
#endif
	}
	return 0;
}

WRITE_LINE_MEMBER(mtech_state::screen_vblank_main)
{
	if (!m_current_machine_is_sms)
		screen_vblank_megadriv(state);
}

void mtech_state::machine_reset()
{
	m_mt_bank_addr = 0;
	md_base_state::machine_reset();

	for (int i = 0; i < 8; i++)
		m_cart_reg[i] = nullptr;

	if (m_cart1->get_rom_size() > 0)
		m_cart_reg[0] = memregion(std::string(m_cart1->tag()) + GENERIC_ROM_REGION_TAG);
	else
		m_cart_reg[0] = memregion(":mt_slot1:cart");
	if (m_cart2)
		m_cart_reg[1] = memregion(std::string(m_cart2->tag()) + GENERIC_ROM_REGION_TAG);
	if (m_cart3)
		m_cart_reg[2] = memregion(std::string(m_cart3->tag()) + GENERIC_ROM_REGION_TAG);
	if (m_cart4)
		m_cart_reg[3] = memregion(std::string(m_cart4->tag()) + GENERIC_ROM_REGION_TAG);
	if (m_cart5)
		m_cart_reg[4] = memregion(std::string(m_cart5->tag()) + GENERIC_ROM_REGION_TAG);
	if (m_cart6)
		m_cart_reg[5] = memregion(std::string(m_cart6->tag()) + GENERIC_ROM_REGION_TAG);
	if (m_cart7)
		m_cart_reg[6] = memregion(std::string(m_cart7->tag()) + GENERIC_ROM_REGION_TAG);
	if (m_cart8)
		m_cart_reg[7] = memregion(std::string(m_cart8->tag()) + GENERIC_ROM_REGION_TAG);

	switch_cart(0);
}

uint32_t mtech_state::screen_update_menu(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vdp1->screen_update(screen, bitmap, cliprect);
	return 0;
}


void mtech_state::megatech(machine_config &config)
{
	/* basic machine hardware */
	md_ntsc(config);

	/* Megatech has an extra SMS based bios *and* an additional screen */
	Z80(config, m_bioscpu, MASTER_CLOCK / 15); /* ?? */
	m_bioscpu->set_addrmap(AS_PROGRAM, &mtech_state::megatech_bios_map);
	m_bioscpu->set_addrmap(AS_IO, &mtech_state::megatech_bios_portmap);

	cxd1095_device &io1(CXD1095(config, "io1"));
	io1.in_porta_cb().set_ioport("BIOS_DSW0");
	io1.in_portb_cb().set_ioport("BIOS_DSW1");
	io1.out_porte_cb().set(FUNC(mtech_state::cart_select_w));

	cxd1095_device &io2(CXD1095(config, "io2"));
	io2.in_porta_cb().set_ioport("BIOS_IN0");
	io2.in_portb_cb().set_ioport("BIOS_IN1");
	io2.in_portc_cb().set(FUNC(mtech_state::bios_portc_r));
	io2.out_portd_cb().set(FUNC(mtech_state::bios_portd_w));
	io2.in_porte_cb().set(FUNC(mtech_state::bios_porte_r));
	io2.out_porte_cb().set(FUNC(mtech_state::bios_porte_w));

	config.set_default_layout(layout_dualhovu);

	screen_device &screen(*subdevice<screen_device>("megadriv"));
	screen.set_raw(XTAL(10'738'635)/2,
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	screen.set_screen_update(FUNC(mtech_state::screen_update_main));
	screen.screen_vblank().set(FUNC(mtech_state::screen_vblank_main));

	m_vdp->n_int().set_inputline(m_z80snd, 0);

	screen_device &menu(SCREEN(config, "menu", SCREEN_TYPE_RASTER));
	// check frq
	menu.set_raw(XTAL(10'738'635)/2,
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	menu.set_screen_update(FUNC(mtech_state::screen_update_menu));

	SEGA315_5246(config, m_vdp1, MASTER_CLOCK / 5); /* ?? */
	m_vdp1->set_screen("menu");
	m_vdp1->set_is_pal(false);
	m_vdp1->n_int().set_inputline(m_bioscpu, 0);
	m_vdp1->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_vdp1->add_route(ALL_OUTPUTS, "rspeaker", 0.25);
}


image_init_result mtech_state::load_cart(device_image_interface &image, generic_slot_device *slot, int gameno)
{
	uint8_t *ROM;
	const char  *pcb_name;
	uint32_t size = slot->common_get_size("rom");

	if (!image.loaded_through_softlist())
		return image_init_result::FAIL;

	slot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	ROM = slot->get_rom_base();
	memcpy(ROM, image.get_software_region("rom"), size);

	if ((pcb_name = image.get_feature("pcb_type")) == nullptr)
		return image_init_result::FAIL;
	else
	{
		if (!core_stricmp("genesis", pcb_name))
		{
			osd_printf_debug("cart%d is genesis\n", gameno + 1);
			m_cart_is_genesis[gameno] = 1;
		}
		else if (!core_stricmp("sms", pcb_name))
		{
			osd_printf_debug("cart%d is sms\n", gameno + 1);
			m_cart_is_genesis[gameno] = 0;
		}
		else
			osd_printf_debug("cart%d is invalid\n", gameno + 1);
	}

	return image_init_result::PASS;
}

#define MEGATECH_CARTSLOT(_tag, _load) \
	GENERIC_CARTSLOT(config, _tag, generic_plain_slot, "megatech_cart").set_device_load(FUNC(mtech_state::_load))

void mtech_state::megatech_multislot(machine_config &config)
{
	megatech(config);

	// add cart slots
	MEGATECH_CARTSLOT("mt_slot1", mt_cart1);
	MEGATECH_CARTSLOT("mt_slot2", mt_cart2);
	MEGATECH_CARTSLOT("mt_slot3", mt_cart3);
	MEGATECH_CARTSLOT("mt_slot4", mt_cart4);
	MEGATECH_CARTSLOT("mt_slot5", mt_cart5);
	MEGATECH_CARTSLOT("mt_slot6", mt_cart6);
	MEGATECH_CARTSLOT("mt_slot7", mt_cart7);
	MEGATECH_CARTSLOT("mt_slot8", mt_cart8);

	SOFTWARE_LIST(config, "cart_list").set_original("megatech");
}


void mtech_state::megatech_fixedslot(machine_config &config)
{
	megatech(config);

	// add cart slots
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "mt_slot1", generic_plain_slot, "megatech_cart"));
	cartslot.set_device_load(FUNC(mtech_state::mt_cart1));
	cartslot.set_user_loadable(false);
}


/* MegaTech Games - Genesis & sms! Games with a timer */

#define MEGATECH_BIOS \
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x10000, "mtbios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "ver1", "Ver 1" ) \
	ROMX_LOAD( "epr-12664.20",  0x000000, 0x8000, CRC(f71e9526) SHA1(1c7887541d02c41426992d17f8e3db9e03975953), ROM_BIOS(0)) \
	ROM_SYSTEM_BIOS( 1, "ver0a", "Ver 0 Rev A" ) \
	ROMX_LOAD( "epr-12263a.20", 0x000000, 0x8000, CRC(07c3f423) SHA1(50c28bbc2d4349c820d988ae3f20aae3f808545f), ROM_BIOS(1)) \
	ROM_SYSTEM_BIOS( 2, "ver0b", "Ver 0 Rev B" ) \
	ROMX_LOAD( "epr-12263b.20", 0x000000, 0x8000, CRC(ca26c87a) SHA1(987a18bede6e54cd73c4434426eb6c302a37cdc5), ROM_BIOS(2)) \
	ROM_SYSTEM_BIOS( 3, "ver0aa","Ver 0 Rev B (alt?)" ) \
	ROMX_LOAD( "epr-12604a.20", 0x000000, 0x8000, CRC(884e4aa5) SHA1(c9008c431a937c084fb475273093ca0b434b5f47), ROM_BIOS(3))


/* no games */
ROM_START( megatech )
	MEGATECH_BIOS
ROM_END


/* Game 01 - Altered Beast (Genesis) */
ROM_START( mt_beast ) /* Altered Beast */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12538.ic1", 0x000000, 0x080000, CRC(3bea3dce) SHA1(ec72e4fde191dedeb3f148f132603ed3c23f0f86) )
	ROM_LOAD16_BYTE( "epr-12368-01.ic2", 0x300001, 0x08000, CRC(40cb0088) SHA1(e1711532c29f395a35a1cb34d789015881b5a1ed) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 02 - Space Harrier 2 */
ROM_START( mt_shar2 ) /* Space Harrier 2 */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11934.ic1", 0x000000, 0x080000, CRC(932daa09) SHA1(a2d7a76f3604c6227d43229908bfbd02b0ef5fd9) )
	ROM_LOAD16_BYTE( "epr-12368-02.ic2", 0x300001, 0x08000, CRC(c129c66c) SHA1(e7c0c97db9df9eb04e2f9ff561b64305219b8f1f) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 03 - Super Thunder Blade */
ROM_START( mt_stbld ) /* Super Thunder Blade */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11996f.ic1", 0x000000, 0x080000,  CRC(9355c34e) SHA1(26ff91c2921408673c644b0b1c8931d98524bf63) )
	ROM_LOAD16_BYTE( "epr-12368-03.ic2", 0x300001, 0x08000,  CRC(1ba4ac5d) SHA1(9bde57d70189d159ebdc537a9026001abfd0deae) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 04 - Great Golf (SMS) */
/* Also known to have the ID# MPR-11128 instead of MPR-11129F, same contents */
ROM_START( mt_ggolf ) /* Great Golf */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11129f.ic1", 0x000000, 0x020000, CRC(c6611c84) SHA1(eab0eed872dd26b13bcf0b2dd74fcbbc078812c9) )
	ROM_LOAD16_BYTE( "epr-12368-04.ic2", 0x300001, 0x08000, CRC(62e5579b) SHA1(e1f531be5c40a1216d4192baeda9352384444410) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 05 - Great Soccer (SMS) - bad dump */
ROM_START( mt_gsocr ) /* Great Soccer */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp10747f.ic1", 0x000000, 0x020000, BAD_DUMP CRC(9cf53703) SHA1(c6b4d1de56bd5bf067ec7fc80449c07686d01337) )
	ROM_LOAD16_BYTE( "epr-12368-05.ic2", 0x300001, 0x08000, CRC(bab91fcc) SHA1(a160c9d34b253e93ac54fdcef33f95f44d8fa90c) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 06 - Out Run (SMS) */
ROM_START( mt_orun ) /* Out Run */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-11078.ic1", 0x000000, 0x040000, CRC(5589d8d2) SHA1(4f9b61b24f0d9fee0448cdbbe8fc05411dbb1102) )
	ROM_LOAD16_BYTE( "epr-12368-06.ic2", 0x300001, 0x08000, CRC(c7c74429) SHA1(22ee261a653e10d66e0d6703c988bb7f236a7571) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 07 - Alien Syndrome (SMS) */
ROM_START( mt_asyn ) /* Alien Syndrome */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-11194.ic1", 0x000000, 0x040000, CRC(4cc11df9) SHA1(5d786476b275de34efb95f576dd556cf4b335a83) )
	ROM_LOAD16_BYTE( "epr-12368-07.ic2", 0x300001, 0x08000, CRC(14f4a17b) SHA1(0fc010ac95762534892f1ae16986dbf1c25399d3) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 08 - Shinobi (SMS) */
ROM_START( mt_shnbi ) /* Shinobi */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11706.ic1", 0x000000, 0x040000, CRC(0c6fac4e) SHA1(7c0778c055dc9c2b0aae1d166dbdb4734e55b9d1) )
	ROM_LOAD16_BYTE( "epr-12368-08.ic2", 0x300001, 0x08000, CRC(103a0459) SHA1(d803ddf7926b83785e8503c985b8c78e7ccb5dac) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 09 - Fantasy Zone (SMS) */
// note, dump was bad, but the good (uniquely identifiable) parts matched the 'fantasy zone (world) (v1.2).bin' SMS rom
// so I'm using that until it gets verified.
ROM_START( mt_fz ) /* Fantasy Zone */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-10118.ic1", 0x000000, 0x020000, CRC(65d7e4e0) SHA1(0278cd120dc3a7707eda9314c46c7f27f9e8fdda) )
	ROM_LOAD16_BYTE( "epr-12368-09.bin", 0x300001, 0x08000, CRC(373d2a70) SHA1(c39dd1003d71a417b12a359126bfef64c7a2fd00) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END



/* Game 10 - Afterburner (SMS) */
ROM_START( mt_aftrb ) /* Afterburner */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11271.ic1", 0x000000, 0x080000, CRC(1c951f8e) SHA1(51531df038783c84640a0cab93122e0b59e3b69a) )
	ROM_LOAD16_BYTE( "epr-12368-10.ic2", 0x300001, 0x08000, CRC(2a7cb590) SHA1(2236963bddc89ca9045b530259cc7b5ccf889eaf) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 11 - Thunder Force II */
ROM_START( mt_tfor2 ) /* Thunder Force II */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12559.ic1", 0x000000, 0x080000, CRC(b093bee3) SHA1(0bf6194c3d228425f8cf1903ed70d8da1b027b6a) )
	ROM_LOAD16_BYTE( "epr-12368-11.ic2", 0x300001, 0x08000, CRC(f4f27e8d) SHA1(ae1a2823deb416c53838115966f1833d5dac72d4) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 13 - Astro Warrior (SMS) */
ROM_START( mt_astro ) /* Astro Warrior */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ep13817.ic2", 0x000000, 0x20000, CRC(299cbb74) SHA1(901697a3535ad70190647f34ad5b30b695d54542) )
	ROM_LOAD16_BYTE( "epr-12368-13.ic1", 0x300001, 0x08000,  CRC(4038cbd1) SHA1(696bc1efce45d9f0052b2cf0332a232687c8d6ab) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 19 - Great Football (SMS) */
ROM_START( mt_gfoot ) /* Great Football */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-10576f.ic1", 0x000000, 0x020000, CRC(2055825f) SHA1(a768f44ce7e50083ffe8c4b5e3ac93ceb7bd3266) )
	ROM_LOAD16_BYTE( "epr-12368-19.ic2", 0x300001, 0x08000, CRC(e27cb37a) SHA1(2b6259957e86d033a5689fd716a9efcfeff7d5ba) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 20 - Last Battle */
ROM_START( mt_lastb ) /* Last Battle */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12578f.ic1", 0x000000, 0x080000, CRC(531191a0) SHA1(f6bc26e975c01a3e10ab4033e4c5f494627a1e2f) )
	ROM_LOAD16_BYTE( "epr-12368-20.ic2", 0x300001, 0x08000, CRC(e1a71c91) SHA1(c250da18660d8aea86eb2abace41ba46130dabc8) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 21 - World Championship Soccer (Genesis) */
ROM_START( mt_wcsoc ) /* World Championship Soccer */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12607b.ic1", 0x000000, 0x080000, CRC(bc591b30) SHA1(55e8577171c0933eee53af1dabd0f4c6462d5fc8) )
	ROM_LOAD16_BYTE( "epr-12368-21.ic2", 0x300001, 0x08000, CRC(028ee46b) SHA1(cd8f81d66e5ae62107eb20e0ca5db4b66d4b2987) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 22 - Tetris */
ROM_START( mt_tetri ) /* Tetris */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "mpr-12356f.ic1", 0x000001, 0x020000, CRC(1e71c1a0) SHA1(44b2312792e49d46d71e0417a7f022e5ffddbbfe) )
	ROM_LOAD16_BYTE( "mpr-12357f.ic2", 0x000000, 0x020000, CRC(d52ca49c) SHA1(a9159892eee2c0cf28ebfcfa99f81f80781851c6) )
	ROM_LOAD16_BYTE( "epr-12368-22.ic3", 0x300001, 0x08000, CRC(1c1b6468) SHA1(568a38f4186167486e39ab4aa2c1ceffd0b81156) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 23 - Ghouls and Ghosts (Genesis) */
ROM_START( mt_gng ) /* Ghouls and Ghosts */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12605.ic1", 0x000000, 0x020000, CRC(1066c6ab) SHA1(c30e4442732bdb38c96d780542f8550a94d127b0) )
	ROM_LOAD16_WORD_SWAP( "mpr12606.ic2", 0x080000, 0x020000, CRC(d0be7777) SHA1(a44b2a3d427f6973b5c1a3dcd8d1776366acb9f7) )
	ROM_CONTINUE(0x020000,0x60000)
	ROM_LOAD16_BYTE( "epr-12368-23.ic3", 0x300001, 0x08000, CRC(7ee58546) SHA1(ad5bb0934475eacdc5e354f67c96fe0d2512d33b) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 24 - Super Hang-On (Genesis) */
ROM_START( mt_shang ) /* Super Hang-On */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-12640.ic1", 0x000000, 0x080000, CRC(2fe2cf62) SHA1(4728bcc847deb38b16338cbd0154837cd4a07b7d) )
	ROM_LOAD16_BYTE( "epr-12368-24.ic2", 0x300001, 0x08000, CRC(6c2db7e3) SHA1(8de0a10ed9185c9e98f17784811a79d3ce8c4c03) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 25 - Golden Axe (Genesis) */
ROM_START( mt_gaxe ) /* Golden Axe */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "12806.ic1", 0x000000, 0x080000, CRC(43456820) SHA1(2f7f1fcd979969ac99426f11ab99999a5494a121) )
	ROM_LOAD16_BYTE( "epr-12368-25.ic2", 0x300001, 0x08000, CRC(1f07ed28) SHA1(9d54192f4c6c1f8a51c38a835c1dd1e4e3e8279e) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 26 - Forgotten Worlds */
/* why is this pre-swapped like a console dump?? */
ROM_START( mt_fwrld ) /* Forgotten Worlds */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD( "mpr-12672-h.ic1", 0x000000, 0x080000, CRC(d0ee6434) SHA1(8b9a37c206c332ef23dc71f09ec40e1a92b1f83a) )
	ROM_LOAD16_BYTE( "epr-12368-26.ic2", 0x300001, 0x08000, CRC(4623b573) SHA1(29df4a5c5de66cd9cb7519e4f30000f7dddc2138) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 27 - Mystic Defender */
ROM_START( mt_mystd ) /* Mystic Defender */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12707.1", 0x000000, 0x080000, CRC(4f2c513d) SHA1(f9bb548b3688170fe18bb3f1b5b54182354143cf) )
	ROM_LOAD16_BYTE( "epr-12368-27.ic2", 0x300001, 0x08000, CRC(caf46f78) SHA1(a9659e86a6a223646338cd8f29c346866e4406c7) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 28 - The Revenge of Shinobi */
ROM_START( mt_revsh ) /* The Revenge Of Shinobi */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12675.ic1", 0x000000, 0x080000, CRC(672a1d4d) SHA1(5fd0af14c8f2cf8ceab1ae61a5a19276d861289a) )
	ROM_LOAD16_BYTE( "epr-12368-28.ic2", 0x300001, 0x08000, CRC(0d30bede) SHA1(73a090d84b78a570e02fb54a33666dcada52849b) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 29 - Parlour Games (SMS) */
ROM_START( mt_parlg ) /* Parlour Games */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11404.ic1", 0x000000, 0x020000, CRC(e030e66c) SHA1(06664daf208f07cb00b603b12eccfc3f01213a17) )
	ROM_LOAD16_BYTE( "epr-12368-29.ic2", 0x300001, 0x08000, CRC(534151e8) SHA1(219238d90c1d3ac07ff64c9a2098b490fff68f04) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 31 - Arnold Palmer Tournament Golf */
ROM_START( mt_tgolf ) /* Arnold Palmer Tournament Golf */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-12645f.ic1", 0x000000, 0x080000, CRC(c07ef8d2) SHA1(9d111fdc7bb92d52bfa048cd134aa488b4f475ef) )
	ROM_LOAD16_BYTE( "epr-12368-31.ic2", 0x300001, 0x08000, CRC(30af7e4a) SHA1(baf91d527393dc90aba9371abcb1e690bcc83c7e) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 32 - Super Real Basketball */
/* why is this pre-swapped like a console dump?? */
ROM_START( mt_srbb ) /* Super Real Basketball */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD( "mpr-12904f.ic1", 0x000000, 0x080000, CRC(4346e11a) SHA1(c86725780027ef9783cb7884c8770cc030b0cd0d) )
	ROM_LOAD16_BYTE( "epr-12368-32.ic2", 0x300001, 0x08000, CRC(f70adcbe) SHA1(d4412a7cd59fe282a1c6619aa1051a2a2e00e1aa) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 35 - Tommy Lasorda Baseball */
ROM_START( mt_tlbba ) /* Tommy Lasorda Baseball */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12706.ic1", 0x000000, 0x080000, CRC(8901214f) SHA1(f5ec166be1cf9b86623b9d7a78ec903b899da32a) )
	ROM_LOAD16_BYTE( "epr-12368-35.ic2", 0x300001, 0x08000, CRC(67bbe482) SHA1(6fc283b22e68befabb44b2cc61a7f82a71d6f029) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 36 - Columns */
ROM_START( mt_cols ) /* Columns */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13193-t.ic1", 0x000000, 0x080000, CRC(8c770e2f) SHA1(02a3626025c511250a3f8fb3176eebccc646cda9) )
	ROM_LOAD16_BYTE( "epr-12368-36.ic3",   0x300001, 0x008000,  CRC(a4b29bac) SHA1(c9be866ac96243897d09612fe17562e0481f66e3) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 38 - ESWAT */
ROM_START( mt_eswat ) /* ESWAT */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13192-h.ic1", 0x000000, 0x080000, CRC(82f458ef) SHA1(58444b783312def71ecffc4ad021b72a609685cb) )
	ROM_LOAD16_BYTE( "epr-12368-38.ic2", 0x300001, 0x08000, CRC(43c5529b) SHA1(104f85adea6da1612c0aa96d553efcaa387d7aaf) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 39 - Super Monaco Grand Prix (Genesis) */
ROM_START( mt_smgp ) /* Super Monaco Grand Prix */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "13250.ic1", 0x000000, 0x080000, CRC(189b885f) SHA1(31c06ffcb48b1604989a94e584261457de4f1f46) )
	ROM_LOAD16_BYTE( "epr-12368-39.ic2", 0x300001, 0x08000, CRC(64b3ce25) SHA1(83a9f2432d146a712b037f96f261742f7dc810bb) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 40 - Moon Walker */
ROM_START( mt_mwalk ) /* Moon Walker */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13285a.ic1", 0x000000, 0x080000, CRC(189516e4) SHA1(2a79e07da2e831832b8d448cae87a833c85e67c9) )
	ROM_LOAD16_BYTE( "epr-12368-40.ic2", 0x300001, 0x08000, CRC(0482378c) SHA1(734772f3ddb5ff82b76c3514d18a464b2bce8381) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 41 - Crackdown */
ROM_START( mt_crack ) /* Crackdown */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13578a-s.ic1", 0x000000, 0x080000, CRC(23f19893) SHA1(09aca793871e2246af4dc24925bc1eda8ff34446) )
	ROM_LOAD16_BYTE( "epr-12368-41.ic2", 0x300001, 0x08000, CRC(3014acec) SHA1(07953e9ae5c23fc7e7d08993b215f4dfa88aa5d7) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 43 - Shadow Dancer */
ROM_START( mt_shado ) /* Shadow Dancer */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-13571-s.ic1", 0x000000, 0x080000, CRC(56a29310) SHA1(55836177e4a1e2deb68408976b29d0282cf661a9) )
	ROM_LOAD16_BYTE( "epr-12368-43.ic2", 0x300001, 0x08000, CRC(1116cbc7) SHA1(ba6dd21ceadeedf730b71b67acbd20d9067114f3) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 44 - Arrow Flash */
ROM_START( mt_arrow ) /* Arrow Flash */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr13396h.ic1", 0x000000, 0x080000, CRC(091226e3) SHA1(cb15c6277314f3c4a86b5ae5823f72811d5d269d) )
	ROM_LOAD16_BYTE( "epr-12368-44.ic2", 0x300001, 0x08000, CRC(e653065d) SHA1(96b014fc4df8eb2188ac94ed0a778d974fe6dcad) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 47 - Alien Storm */
ROM_START( mt_astrm ) /* Alien Storm */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13941.ic1", 0x000000, 0x080000, CRC(d71b3ee6) SHA1(05f272dad243d132d517c303388248dc4c0482ed) )
	ROM_LOAD16_BYTE( "epr-12368-47.ic2", 0x300001, 0x08000, CRC(31fb683d) SHA1(e356da020bbf817b97fb10c27f75cf5931edf4fc) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 48 - Wrestle War */
ROM_START( mt_wwar ) /* Wrestle War */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-14025-f.ic1", 0x000000, 0x080000, CRC(26e899fe) SHA1(6d28e154ae2e4196097a2aa96c5acd5dfe7e3d2b) )
	ROM_LOAD16_BYTE( "epr-12368-48.ic2", 0x300001, 0x08000, CRC(25817bc2) SHA1(ba1bbb952aff12fb4d3ecfb10d82c54128439395) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 49 - Bonanza Bros. */
// original dump of mpr-13905a.ic1 had a size of 0x100000, which is a double dump of the correct size 0x080000
// the IC is a Fujitsu MB834200A MaskROM by 4Mb (512KB) in a DIP40-600mil package
ROM_START( mt_bbros ) /* Bonanza Bros. */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-13905a.ic1", 0x000000, 0x080000, CRC(6d617940) SHA1(11d5ff1c2db79632f6dea2edf97f56af2149cea4) )
	ROM_LOAD16_BYTE( "epr-12368-49.ic2", 0x300001, 0x08000, CRC(c5101da2) SHA1(636f30043e2e9291e193ef9a2ead2e97a0bf7380) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 51 - Streets of Rage */
ROM_START( mt_srage ) /* Streets of Rage */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-14125-s.ic1", 0x000000, 0x080000, CRC(db4ac746) SHA1(c7cc24e2329f279574513fa32bbf79f72f75aeea) )
	ROM_LOAD16_BYTE( "epr-12368-51.ic2", 0x300001, 0x08000, CRC(49b7d6f4) SHA1(96e69851c92715e7daf35b184cf374147a8d2880) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 52 - Sonic The Hedgehog (Genesis) */
ROM_START( mt_sonic ) /* Sonic The Hedgehog */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13913.ic1", 0x000000, 0x080000, CRC(480b4b5c) SHA1(ab1dc1f738e3b2d0898a314b123fa71182bf572e) )
	ROM_LOAD16_BYTE( "epr-12368-52.ic2", 0x300001, 0x8000,  CRC(6a69d20c) SHA1(e483b39ff6eca37dc192dc296d004049e220554a) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


ROM_START( mt_sonia ) /* Sonic (alt)*/
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13933.ic1", 0x000000, 0x080000, CRC(13775004) SHA1(5decfd35944a2d0e7b996b9a4a12b616a309fd5e) )
	ROM_LOAD16_BYTE( "epr-12368-52.ic2", 0x300001, 0x8000,  CRC(6a69d20c) SHA1(e483b39ff6eca37dc192dc296d004049e220554a) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 53 - Fire Shark */
	/* alt version with these roms exists, but the content is the same */
	/* (6a221fd6) ep14706.ic1             mp14341.ic1  [even]     IDENTICAL */
	/* (09fa48af) ep14707.ic2             mp14341.ic1  [odd]      IDENTICAL */

ROM_START( mt_fshrk ) /* Fire Shark */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14341.ic1", 0x000000, 0x080000, CRC(04d65ebc) SHA1(24338aecdc52b6f416548be722ca475c83dbae96) )
	ROM_LOAD16_BYTE( "epr-12368-53.ic2", 0x300001, 0x08000,  CRC(4fa61044) SHA1(7810deea221c10b0b2f5233443d81f4f1998ee58) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 54 - Spiderman */
ROM_START( mt_spman ) /* Spiderman */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14027-sm.ic1", 0x000000, 0x080000, CRC(e2c08a06) SHA1(39e592eafd47e2aa6edbb4845d44750057bff890) )
	ROM_LOAD16_BYTE( "epr-12368-54.ic2", 0x300001, 0x08000,  CRC(30b68988) SHA1(04eeb0fad732a791b6bc0c0846306d567573649f) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 55 - California Games */
ROM_START( mt_calga ) /* California Games */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "epr-14494.ic1", 0x000001, 0x040000, CRC(cbe58b1b) SHA1(ea067fc08e644c993f8d13731425c9296c1a2a75) )
	ROM_LOAD16_BYTE( "epr-14495.ic2", 0x000000, 0x040000, CRC(cb956f4f) SHA1(3574c496b79aefdec7d02975490ebe3bb373bc60) )
	ROM_LOAD16_BYTE( "epr-12368-55.ic3", 0x300001, 0x08000, CRC(6f7dd8f5) SHA1(a6cb1aa8c3635738dd9e4d3e0d729d089fd9b599) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 57 - Golden Axe 2 (Genesis) */
ROM_START( mt_gaxe2 ) /* Golden Axe 2 */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14272.ic1", 0x000000, 0x080000, CRC(d4784cae) SHA1(b6c286027d06fd850016a2a1ee1f1aeea080c3bb) )
	ROM_LOAD16_BYTE( "epr-12368-57.ic2", 0x300001, 0x08000, CRC(dc9b4433) SHA1(efd3a598569010cdc4bf38ecbf9ed1b4e14ffe36) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 58 - Sports Talk Football */
ROM_START( mt_stf ) /* Sports Talk Football */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14356a-f.ic1", 0x000000, 0x100000, CRC(20cf32f6) SHA1(752314346a7a98b3808b3814609e024dc0a4108c) )
	ROM_LOAD16_BYTE( "epr-12368-58.ic2", 0x300001, 0x08000, CRC(dce2708e) SHA1(fcebb1899ee11468f6bda705899f074e7de9d723) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 59 - Mario Lemieux Hockey */
ROM_START( mt_mlh ) /* Mario Lemieux Hockey */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-14376-h.ic1", 0x000000, 0x80000, CRC(aa9be87e) SHA1(dceed94eaeb30e534f6953a4bc25ff37673b1e6b) )
	ROM_LOAD16_BYTE( "epr-12368-59.ic2", 0x300001, 0x08000, CRC(6d47b438) SHA1(0a145f6438e4e55c957ae559663c37662b685246) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 60 - Kid Chameleon */
ROM_START( mt_kcham ) /* Kid Chameleon */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14557.ic1", 0x000000, 0x100000, CRC(e1a889a4) SHA1(a2768eacafc47d371e5276f0cce4b12b6041337a) )
	ROM_LOAD16_BYTE( "epr-12368-60.ic2", 0x300001, 0x08000, CRC(a8e4af18) SHA1(dfa49f6ec4047718f33dba1180f6204dbaff884c) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 61 - Turbo Outrun */
// original dump of mpr-14674.ic1 had CRC(c2b9a802) SHA1(108cc844c944125f9d271a2f2db094301294e8c2)
// with the byte at offset 3 being F6 instead of Fe, this seems like a bad dump when compared to the Genesis rom which
// has been verified on multiple carts, chances are the ROM had developed a fault.
ROM_START( mt_tout ) /* Turbo Outrun */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-14674.ic1", 0x000000, 0x080000, CRC(453712a2) SHA1(5d2c8430a9a14aac7f19c22617539b0503ab92cd) )
	ROM_LOAD16_BYTE( "epr-12368-61.ic2", 0x300001, 0x08000, CRC(4aa0b2a2) SHA1(bce03f88d6cfd02683d51c28058f6229fda13b49) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 62 - Sonic The Hedgehog 2 */
ROM_START( mt_soni2 ) /* Sonic The Hedgehog 2 */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "mt_slot1:cart", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp15000a-f.ic1", 0x000000, 0x100000, CRC(679ebb49) SHA1(557482064677702454562f753460993067ef9e16) )
	ROM_LOAD16_BYTE( "epr-12368-62.ic2", 0x300001, 0x08000, CRC(14a8566f) SHA1(d1d14162144bf068ddd19e9736477ff98fb43f9e) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END

} // Anonymous namespace



/* nn */ /* nn is part of the instruction rom name, should there be a game for each number? */
/* -- */ CONS( 1989, megatech, 0, 0,     megatech_multislot, megatech, mtech_state, init_mt_slot,      "Sega",                  "Mega-Tech", MACHINE_IS_BIOS_ROOT )
/* 01 */ GAME( 1988, mt_beast, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Altered Beast (Mega-Tech)", MACHINE_NOT_WORKING )
/* 02 */ GAME( 1988, mt_shar2, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Space Harrier II (Mega-Tech)", MACHINE_NOT_WORKING )
/* 03 */ GAME( 1988, mt_stbld, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Super Thunder Blade (Mega-Tech)", MACHINE_NOT_WORKING )
/* 04 */ GAME( 1987, mt_ggolf, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Great Golf (Mega-Tech, SMS based)", MACHINE_NOT_WORKING ) /* sms! */
/* 05 */ GAME( 198?, mt_gsocr, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Great Soccer (Mega-Tech, SMS based)", MACHINE_NOT_WORKING ) /* sms! also bad */
/* 06 */ GAME( 1987, mt_orun,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Out Run (Mega-Tech, SMS based)", MACHINE_NOT_WORKING ) /* sms! */
/* 07 */ GAME( 1987, mt_asyn,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Alien Syndrome (Mega-Tech, SMS based)", MACHINE_NOT_WORKING ) /* sms! */
/* 08 */ GAME( 1987, mt_shnbi, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Shinobi (Mega-Tech, SMS based)", MACHINE_NOT_WORKING) /* sms */
/* 09 */ GAME( 1987, mt_fz,    megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Fantasy Zone (Mega-Tech, SMS based)", MACHINE_NOT_WORKING) /* sms */
/* 10 */ GAME( 1987, mt_aftrb, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "After Burner (Mega-Tech, SMS based)", MACHINE_NOT_WORKING) /* sms */
/* 11 */ GAME( 1989, mt_tfor2, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Techno Soft / Sega",    "Thunder Force II MD (Mega-Tech)", MACHINE_NOT_WORKING )
/* 12 */ // unknown
/* 13 */ GAME( 1986, mt_astro, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Astro Warrior (Mega-Tech, SMS based)", MACHINE_NOT_WORKING ) /* sms! */
/* 14 */ // unknown
/* 15 */ // unknown
/* 16 */ // unknown
/* 17 */ // unknown
/* 18 */ // Kung Fu Kid (sms)
/* 19 */ GAME( 1987, mt_gfoot, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Great Football (Mega-Tech, SMS based)", MACHINE_NOT_WORKING ) /* sms! */
/* 20 */ GAME( 1989, mt_lastb, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Last Battle (Mega-Tech)", MACHINE_NOT_WORKING )
/* 21 */ GAME( 1989, mt_wcsoc, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "World Championship Soccer (Mega-Tech)", MACHINE_NOT_WORKING )
/* 22 */ GAME( 1989, mt_tetri, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Tetris (Mega-Tech)", MACHINE_NOT_WORKING )
/* 23 */ GAME( 1989, mt_gng,   megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Capcom / Sega",         "Ghouls'n Ghosts (Mega-Tech)", MACHINE_NOT_WORKING )
/* 24 */ GAME( 1989, mt_shang, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Super Hang-On (Mega-Tech)", MACHINE_NOT_WORKING )
/* 25 */ GAME( 1989, mt_gaxe,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Golden Axe (Mega-Tech)", MACHINE_NOT_WORKING )
/* 26 */ GAME( 1989, mt_fwrld, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Forgotten Worlds (Mega-Tech)", MACHINE_NOT_WORKING )
/* 27 */ GAME( 1989, mt_mystd, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Mystic Defender (Mega-Tech)", MACHINE_NOT_WORKING )
/* 28 */ GAME( 1989, mt_revsh, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "The Revenge of Shinobi (Mega-Tech)", MACHINE_NOT_WORKING )
/* 29 */ GAME( 1987, mt_parlg, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Parlour Games (Mega-Tech, SMS based)", MACHINE_NOT_WORKING ) /* sms! */
/* 30 */ // unknown
/* 31 */ GAME( 1989, mt_tgolf, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Arnold Palmer Tournament Golf (Mega-Tech)", MACHINE_NOT_WORKING )
/* 32 */ GAME( 1989, mt_srbb,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Super Real Basketball (Mega-Tech)", MACHINE_NOT_WORKING )
/* 33 */ // unknown
/* 34 */ // unknown
/* 35 */ GAME( 1989, mt_tlbba, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Tommy Lasorda Baseball (Mega-Tech)", MACHINE_NOT_WORKING )
/* 36 */ GAME( 1990, mt_cols,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Columns (Mega-Tech)", MACHINE_NOT_WORKING )
/* 37 */ // unknown
/* 38 */ GAME( 1990, mt_eswat, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Cyber Police ESWAT: Enhanced Special Weapons and Tactics (Mega-Tech)", MACHINE_NOT_WORKING )
/* 39 */ GAME( 1990, mt_smgp,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Super Monaco GP (Mega-Tech)", MACHINE_NOT_WORKING )
/* 40 */ GAME( 1990, mt_mwalk, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Michael Jackson's Moonwalker (Mega-Tech)", MACHINE_NOT_WORKING )
/* 41 */ GAME( 1990, mt_crack, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Crack Down (Mega-Tech)", MACHINE_NOT_WORKING )
/* 42 */ // unknown
/* 43 */ GAME( 1990, mt_shado, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Shadow Dancer (Mega-Tech)", MACHINE_NOT_WORKING )
/* 44 */ GAME( 1990, mt_arrow, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Arrow Flash (Mega-Tech)", MACHINE_NOT_WORKING )
/* 45 */ // unknown
/* 46 */ // unknown
/* 47 */ GAME( 1990, mt_astrm, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Alien Storm (Mega-Tech)", MACHINE_NOT_WORKING )
/* 48 */ GAME( 1991, mt_wwar,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Wrestle War (Mega-Tech)", MACHINE_NOT_WORKING ) /* Copyright 1989, 1991 Sega */
/* 49 */ GAME( 1991, mt_bbros, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Bonanza Bros. (Mega-Tech)", MACHINE_NOT_WORKING )
/* 50 */ // unknown
/* 51 */ GAME( 1991, mt_srage, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Streets of Rage (Mega-Tech)", MACHINE_NOT_WORKING )
/* 52 */ GAME( 1991, mt_sonic, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Sonic The Hedgehog (Mega-Tech, set 1)", MACHINE_NOT_WORKING )
/*    */ GAME( 1991, mt_sonia, mt_sonic, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Sonic The Hedgehog (Mega-Tech, set 2)", MACHINE_NOT_WORKING )
/* 53 */ GAME( 1990, mt_fshrk, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Toaplan / Sega",        "Fire Shark (Mega-Tech)", MACHINE_NOT_WORKING )
/* 54 */ GAME( 1991, mt_spman, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega / Marvel",         "Spider-Man vs The Kingpin (Mega-Tech)", MACHINE_NOT_WORKING )
/* 55 */ GAME( 1991, mt_calga, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "California Games (Mega-Tech)", MACHINE_NOT_WORKING )
/* 56 */ // unknown
/* 57 */ GAME( 1991, mt_gaxe2, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Golden Axe II (Mega-Tech)", MACHINE_NOT_WORKING )
/* 58 */ GAME( 1991, mt_stf,   megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Joe Montana II: Sports Talk Football (Mega-Tech)", MACHINE_NOT_WORKING )
/* 59 */ GAME( 1991, mt_mlh,   megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Mario Lemieux Hockey (Mega-Tech)", MACHINE_NOT_WORKING )
/* 60 */ GAME( 1992, mt_kcham, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Kid Chameleon (Mega-Tech)", MACHINE_NOT_WORKING )
/* 61 */ GAME( 1992, mt_tout,  megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Turbo Outrun (Mega-Tech)", MACHINE_NOT_WORKING )
/* 62 */ GAME( 1992, mt_soni2, megatech, megatech_fixedslot, megatech, mtech_state, init_mt_crt, ROT0, "Sega",                  "Sonic The Hedgehog 2 (Mega-Tech)", MACHINE_NOT_WORKING )

/* Games seen in auction (#122011114579), but no confirmed number
- Action Fighter
- Enduro Racer

Games seen in auction (#122011114579) known not to be original but manufactured/bootlegged on actual megatech carts.
The labels are noticeably different than expected.  Be careful if thinking of obtaining!
- After Burner II (GEN)
- Castle of Illusion Starring Mickey Mouse (GEN)
- Double Dragon (SMS)
- Kung Fu Kid (SMS)
- Quackshot Starring Donald Duck (GEN)
- Wonderboy (SMS)

more? */
