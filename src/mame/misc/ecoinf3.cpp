// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Electrocoin Pyramid HW type */

// this seems to not like our Z180 timers much? (or wants a 10ms interrupt externally?)
// also quite a few of the reads / writes are fall-through from Z180 internal reads/writes

// assuming this is like the other hardware EC produced the IO devices should probably
// be several 8255s on 4-byte boundaries

// what is the sound hardware on this one? (should there be sound roms, or does the main CPU drive it directly?)

// 2 of the sets contain program scrambled roms (where the last 0x2000 bytes match between games) why, badly dumped?

#include "emu.h"

#include "awpvid.h" // drawing reels

#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "machine/steppers.h" // stepper motor
#include "sound/sn76496.h"

#include "speaker.h"

#include "ecoinf3.lh"

#include <algorithm>


namespace {

class ecoinf3_state : public driver_device
{
public:
	ecoinf3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_reels(*this, "reel%u", 0U),
		m_lamp_outputs(*this, "lamp%u", 0U),
		m_vfd_outputs(*this, "vfd%u", 0U)
	{
	}

	void init_ecoinf3();
	void init_ecoinf3_swap();
	void ecoinf3_pyramid(machine_config &config);

private:
	virtual void machine_start() override
	{
		m_lamp_outputs.resolve();
		m_vfd_outputs.resolve();

		save_item(NAME(m_lamps));
		save_item(NAME(m_chars));
		save_item(NAME(m_strobe_addr));
		save_item(NAME(m_strobe_amount));
		save_item(NAME(m_optic_pattern));
		save_item(NAME(m_percent_mux));
	}

	template <unsigned N> void reel_optic_cb(int state) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }

	uint8_t ppi8255_intf_a_read_a() { int ret = 0x00; logerror("%04x - ppi8255_intf_a_read_a %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_a_read_b()
	{
		int ret = ioport("IN1")->read();
		logerror("%04x - ppi8255_intf_a_(used)read_b %02x\n", m_maincpu->pcbase(), ret);
		return ret;
	}

	uint8_t ppi8255_intf_a_read_c()
	{
		int ret = ioport("IN5")->read();
		logerror("%04x - ppi8255_intf_a_(used)read_c %02x\n", m_maincpu->pcbase(), ret);
		return ret;
	}

	uint8_t ppi8255_intf_b_read_a() { int ret = 0x00; logerror("%04x - ppi8255_intf_b_read_a %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_b_read_b() { int ret = 0x00; logerror("%04x - ppi8255_intf_b_read_b %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_b_read_c() { int ret = 0x00; logerror("%04x - ppi8255_intf_b_read_c %02x\n", m_maincpu->pcbase(), ret); return ret; }

	uint8_t ppi8255_intf_c_read_a() { int ret = 0x00; logerror("%04x - ppi8255_intf_c_(used)read_a %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_c_read_b()
	{
		int ret = ioport("IN2")->read();
		logerror("%04x - ppi8255_intf_c_(used)read_b %02x (COINS+TEST)\n", m_maincpu->pcbase(), ret);
		return ret;
	} // changing to 00 gives coin tamper

	uint8_t ppi8255_intf_c_read_c()
	{
		int ret = ioport("IN6")->read();
		logerror("%04x - ppi8255_intf_c_(used)read_c %02x\n", m_maincpu->pcbase(), ret);
		return ret;
	}

	uint8_t ppi8255_intf_d_read_a() { int ret = 0x00; logerror("%04x - ppi8255_intf_d_read_a %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_d_read_b()
	{
		int ret = ioport("IN7")->read();
		logerror("%04x - ppi8255_intf_d_(used)read_b %02x\n", m_maincpu->pcbase(), ret);
		return ret;
	}

	uint8_t ppi8255_intf_d_read_c()
	{
		// guess, what are the bottom 4 bits, if anything?

		int ret = m_optic_pattern | (ioport("IN0")->read() & 0xf);

		// | 0x80 = reel 4 fault
		// | 0x40 = reel 3 fault
		// | 0x20 = reel 2 fault
		// | 0x10 = reel 1 fault

		logerror("%04x - ppi8255_intf_d_(used)read_c %02x (Reel Optics)\n", m_maincpu->pcbase(), ret);

		return ret;


	}

	uint8_t ppi8255_intf_e_read_a() { int ret = 0x00; logerror("%04x - ppi8255_intf_e_read_a %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_e_read_b()
	{   // changing gives no % key error in sphinx

		int ret;

		if (m_percent_mux==1)
		{
			ret = ioport("PERKEY")->read();
			logerror("%04x - ppi8255_intf_e_(used)read_b (PER KEY) %02x\n", m_maincpu->pcbase(), ret);
		}
		else if (m_percent_mux==0x80)
		{
			ret = ioport("BUTTONS")->read();
			logerror("%04x - ppi8255_intf_e_(used)read_b (BUTTONS?) %02x\n", m_maincpu->pcbase(), ret);
		}
		else
		{
			ret = 0x00;
			logerror("%04x - ppi8255_intf_e_(used)read_b (UNK MUX %02x) %02x\n", m_maincpu->pcbase(), m_percent_mux, ret);
		}
		return ret;

	}

	uint8_t ppi8255_intf_e_read_c() { int ret = 0x00; logerror("%04x - ppi8255_intf_e_read_c %02x\n", m_maincpu->pcbase(), ret); return ret; }

	uint8_t ppi8255_intf_f_read_a()
	{
		int ret = ioport("IN4")->read();
		logerror("%04x - ppi8255_intf_f_(used)read_a %02x\n", m_maincpu->pcbase(), ret);
		return ret;
	}

	uint8_t ppi8255_intf_f_read_b() { int ret = 0x00; logerror("%04x - ppi8255_intf_f_read_b %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_f_read_c() { int ret = 0x00; logerror("%04x - ppi8255_intf_f_read_c %02x\n", m_maincpu->pcbase(), ret); return ret; }

	uint8_t ppi8255_intf_g_read_a() { int ret = 0x00; logerror("%04x - ppi8255_intf_g_read_a %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_g_read_b() { int ret = 0x00; logerror("%04x - ppi8255_intf_g_read_b %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_g_read_c() { int ret = 0x00; logerror("%04x - ppi8255_intf_g_read_c %02x\n", m_maincpu->pcbase(), ret); return ret; }

	uint8_t ppi8255_intf_h_read_a() { int ret = 0x00; logerror("%04x - ppi8255_intf_h_read_a %02x\n", m_maincpu->pcbase(), ret); return ret; }
	uint8_t ppi8255_intf_h_read_b()
	{
		int ret = ioport("IN5")->read();
		logerror("%04x - ppi8255_intf_h_(used)read_b %02x\n", m_maincpu->pcbase(), ret);
		return ret;
	}
	uint8_t ppi8255_intf_h_read_c() { int ret = 0x00; logerror("%04x - ppi8255_intf_h_read_c %02x\n", m_maincpu->pcbase(), ret); return ret; }

	void update_lamps()
	{
		for (int i=0; i<16; i++)
		{
			for (int bit=0; bit<16; bit++)
				m_lamp_outputs[(i << 4) | bit] = BIT(m_lamps[i], 15 - bit);
		}

	}

	void ppi8255_intf_a_write_a_strobedat0(uint8_t data)
	{
	//  logerror("%04x - ppi8255_intf_a_(used)write_a %02x (STROBEDAT?)\n", m_maincpu->pcbase(), data);
		if (m_strobe_amount)
		{
			m_lamps[m_strobe_addr] = (m_lamps[m_strobe_addr] & 0xff00) | (data & 0x00ff);
			m_strobe_amount--;
		}
	}

	void ppi8255_intf_a_write_b_strobedat1(uint8_t data)
	{
	//  logerror("%04x - ppi8255_intf_a_(used)write_b %02x (STROBEDAT?)\n", m_maincpu->pcbase(), data);
		if (m_strobe_amount)
		{
			m_lamps[m_strobe_addr] = (m_lamps[m_strobe_addr] & 0x00ff) | (data << 8);
			m_strobe_amount--;
		}
	}
	void ppi8255_intf_a_write_c_strobe(uint8_t data)
	{
		if (data>=0xf0)
		{
		//  logerror("%04x - ppi8255_intf_a_(used)write_c %02x (STROBE?)\n", m_maincpu->pcbase(), data);
			m_strobe_addr = data & 0xf;

			// hack, it writes values for the lamps, then writes 0x00 afterwards, probably giving the bulbs power, then removing the power
			// before switching the strobe to the next line?
			m_strobe_amount = 2;

			update_lamps();
		}
		else logerror("%04x - ppi8255_intf_a_(used)write_c %02x (UNUSUAL?)\n", m_maincpu->pcbase(), data);
	}

	void ppi8255_intf_b_write_a(uint8_t data) { logerror("%04x - ppi8255_intf_b_(used)write_a %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_b_write_b(uint8_t data) { logerror("%04x - ppi8255_intf_b_(used)write_b %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_b_write_c(uint8_t data) { logerror("%04x - ppi8255_intf_b_(used)write_c %02x\n", m_maincpu->pcbase(), data); }

	void ppi8255_intf_c_write_a(uint8_t data) { logerror("%04x - ppi8255_intf_c_(used)write_a %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_c_write_b(uint8_t data) { logerror("%04x - ppi8255_intf_c_(used)write_b %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_c_write_c(uint8_t data) { logerror("%04x - ppi8255_intf_c_(used)write_c %02x\n", m_maincpu->pcbase(), data); }

	void ppi8255_intf_d_write_a_reel01(uint8_t data)
	{
//      logerror("%04x - ppi8255_intf_d_(used)write_a %02x\n", m_maincpu->pcbase(), data);
		m_reels[0]->update( data    &0x0f);
		m_reels[1]->update((data>>4)&0x0f);

		awp_draw_reel(machine(),"reel1", *m_reels[0]);
		awp_draw_reel(machine(),"reel2", *m_reels[1]);
	}

	void ppi8255_intf_d_write_b_reel23(uint8_t data)
	{
//      logerror("%04x - ppi8255_intf_d_(used)write_b %02x\n", m_maincpu->pcbase(), data);

		m_reels[2]->update( data    &0x0f);
		m_reels[3]->update((data>>4)&0x0f);

		awp_draw_reel(machine(),"reel3", *m_reels[2]);
		awp_draw_reel(machine(),"reel4", *m_reels[3]);
	}

	void ppi8255_intf_d_write_c(uint8_t data) { logerror("%04x - ppi8255_intf_d_(used)write_c %02x\n", m_maincpu->pcbase(), data);}

	void ppi8255_intf_e_write_a_alpha_display(uint8_t data);
	void ppi8255_intf_e_write_b(uint8_t data) { logerror("%04x - ppi8255_intf_e_write_b %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_e_write_c(uint8_t data)
	{
		m_percent_mux = data;

		logerror("%04x - ppi8255_intf_e_write_c %02x (INPUT MUX?)\n", m_maincpu->pcbase(), data);
	}

	void ppi8255_intf_f_write_a(uint8_t data) { logerror("%04x - ppi8255_intf_f_write_a %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_f_write_b(uint8_t data) { logerror("%04x - ppi8255_intf_f_write_b %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_f_write_c(uint8_t data) { logerror("%04x - ppi8255_intf_f_write_c %02x\n", m_maincpu->pcbase(), data); }

	void ppi8255_intf_g_write_a(uint8_t data) { logerror("%04x - ppi8255_intf_g_write_a %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_g_write_b(uint8_t data) { logerror("%04x - ppi8255_intf_g_write_b %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_g_write_c(uint8_t data) { logerror("%04x - ppi8255_intf_g_write_c %02x\n", m_maincpu->pcbase(), data); }

	void ppi8255_intf_h_write_a(uint8_t data) { logerror("%04x - ppi8255_intf_h_(used)write_a %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_h_write_b(uint8_t data) { logerror("%04x - ppi8255_intf_h_(used)write_b %02x\n", m_maincpu->pcbase(), data); }
	void ppi8255_intf_h_write_c(uint8_t data) { logerror("%04x - ppi8255_intf_h_(used)write_c %02x\n", m_maincpu->pcbase(), data); }

	void pyramid_memmap(address_map &map) ATTR_COLD;
	void pyramid_portmap(address_map &map) ATTR_COLD;

	required_device<z180_device> m_maincpu;
	required_device_array<stepper_device, 4> m_reels;
	output_finder<16 * 16> m_lamp_outputs;
	output_finder<14> m_vfd_outputs;

	uint16_t m_lamps[16];
	uint16_t m_chars[14];

	int m_strobe_addr = 0;
	int m_strobe_amount = 0;
	int m_optic_pattern = 0;

	int m_percent_mux = 0;
};


// this is a copy of roc10937charset for now, I don't know what chip we're meant be using here
// it is some kind of 14 digit, 16 seg display tho
static const uint16_t ecoin_charset[]=
{            // FEDC BA98 7654 3210
	0x507F, // 0101 0000 0111 1111 @.
	0x44CF, // 0100 0100 1100 1111 A.
	0x153F, // 0001 0101 0011 1111 B.
	0x00F3, // 0000 0000 1111 0011 C.
	0x113F, // 0001 0001 0011 1111 D.
	0x40F3, // 0100 0000 1111 0011 E.
	0x40C3, // 0100 0000 1100 0011 F.
	0x04FB, // 0000 0100 1111 1011 G.
	0x44CC, // 0100 0100 1100 1100 H.
	0x1133, // 0001 0001 0011 0011 I.
	0x007C, // 0000 0000 0111 1100 J.
	0x4AC0, // 0100 1010 1100 0000 K.
	0x00F0, // 0000 0000 1111 0000 L.
	0x82CC, // 1000 0010 1100 1100 M.
	0x88CC, // 1000 1000 1100 1100 N.
	0x00FF, // 0000 0000 1111 1111 O.
	0x44C7, // 0100 0100 1100 0111 P.
	0x08FF, // 0000 1000 1111 1111 Q.
	0x4CC7, // 0100 1100 1100 0111 R.
	0x44BB, // 0100 0100 1011 1011 S.
	0x1103, // 0001 0001 0000 0011 T.
	0x00FC, // 0000 0000 1111 1100 U.
	0x22C0, // 0010 0010 1100 0000 V.
	0x28CC, // 0010 1000 1100 1100 W.
	0xAA00, // 1010 1010 0000 0000 X.
	0x9200, // 1001 0010 0000 0000 Y.
	0x2233, // 0010 0010 0011 0011 Z.
	0x00E1, // 0000 0000 1110 0001 [.
	0x8800, // 1000 1000 0000 0000 \.
	0x001E, // 0000 0000 0001 1110 ].
	0x2800, // 0010 1000 0000 0000 ^.
	0x0030, // 0000 0000 0011 0000 _.
	0x0000, // 0000 0000 0000 0000 dummy.
	0x8121, // 1000 0001 0010 0001 !.
	0x0180, // 0000 0001 1000 0000 ".
	0x553C, // 0101 0101 0011 1100 #.
	0x55BB, // 0101 0101 1011 1011 $.
	0x7799, // 0111 0111 1001 1001 %.
	0xC979, // 1100 1001 0111 1001 &.
	0x0200, // 0000 0010 0000 0000 '.
	0x0A00, // 0000 1010 0000 0000 (.
	0xA050, // 1010 0000 0000 0000 ).
	0xFF00, // 1111 1111 0000 0000 *.
	0x5500, // 0101 0101 0000 0000 +.
	0x0000, // 0000 0000 0000 0000 ;. (Set separately)
	0x4400, // 0100 0100 0000 0000 --.
	0x0000, // 0000 0000 0000 0000 . .(Set separately)
	0x2200, // 0010 0010 0000 0000 /.
	0x22FF, // 0010 0010 1111 1111 0.
	0x1100, // 0001 0001 0000 0000 1.
	0x4477, // 0100 0100 0111 0111 2.
	0x443F, // 0100 0100 0011 1111 3.
	0x448C, // 0100 0100 1000 1100 4.
	0x44BB, // 0100 0100 1011 1011 5.
	0x44FB, // 0100 0100 1111 1011 6.
	0x000F, // 0000 0000 0000 1111 7.
	0x44FF, // 0100 0100 1111 1111 8.
	0x44BF, // 0100 0100 1011 1111 9.
	0x0021, // 0000 0000 0010 0001 -
			//                     -.
	0x2001, // 0010 0000 0000 0001 -
			//                     /.
	0x2430, // 0010 0100 0011 0000 <.
	0x4430, // 0100 0100 0011 0000 =.
	0x8830, // 1000 1000 0011 0000 >.
	0x1407, // 0001 0100 0000 0111 ?.
};

static uint32_t set_display(uint32_t segin)
{
	return bitswap<32>(segin, 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,11,9,15,13,12,8,10,14,7,6,5,4,3,2,1,0);
}

// is the 2 digit bank display part of this, or multiplexed elsewhere
void ecoinf3_state::ppi8255_intf_e_write_a_alpha_display(uint8_t data)
{
	if ((data>=0x20) && (data<0x5b))  logerror("%04x - ppi8255_intf_e_write_a %02x (alpha) '%c'\n", m_maincpu->pcbase(), data, data);
	else logerror("%04x - ppi8255_intf_e_write_a %02x (alpha)\n", m_maincpu->pcbase(), data);

	static uint8_t send_buffer = 0;
	static int count = 0;
	// writes the 'PYRAMID' string from RAM (copied from ROM) here...
	// along with port 40/41/42 accesses
	// also error messages? (well it looks like it should, but code is strange and skips them) I guess it's a debug port or the vfd?
	// watch ram around e3e0

	// Pyramid - Writes PYRAMID V6, and 10MS INIT ERROR
	// Labyrinth - Same behavior as Pyramid
	// Secret Castle - Same behavior as Pyramid

	// Sphinx - Writes "No % Key"  -- depends on port 0x51, writes "SPHINX  V- 1" if it's happy with that .. after that you get COIN TAMPER,  a count down with COINS TRIM and a reboot
	// Pennies from Heaven - same behavior as Sphinx

	// typically writes a letter (sometimes twice) then 0x00, usually twice

	if (data==0x00)
	{
		if (send_buffer!=0x00)
		{
			if ((send_buffer>=0x20) && (send_buffer<0x5b))
			{
				if (count<14)
				{
					int chr = send_buffer & 0x3f;

					if (chr>0 && chr<0x3f)
						m_chars[count] =  ecoin_charset[chr];

				}
				//printf("%c", send_buffer);
			}
			else
			{
				// control characters?
				//printf(" (%02x) ", send_buffer);
				count = -1;
			}

			count++;
			if (count%14 == 0)
			{
				//printf("\n");
				count = 0;

			}


			send_buffer = 0x00;
		}
	}
	else
	{
		send_buffer = data;
	}

	std::transform(std::begin(m_chars), std::end(m_chars), std::begin(m_vfd_outputs), set_display);
}

void ecoinf3_state::pyramid_memmap(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xffff).ram();
}

void ecoinf3_state::pyramid_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).ram(); // z180 internal area!

	map(0x40, 0x43).rw("ppi8255_a", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x44, 0x47).rw("ppi8255_b", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x48, 0x4b).rw("ppi8255_c", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x4c, 0x4f).rw("ppi8255_d", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x50, 0x53).rw("ppi8255_e", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x54, 0x57).rw("ppi8255_f", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x58, 0x5b).rw("ppi8255_g", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5c, 0x5f).rw("ppi8255_h", FUNC(i8255_device::read), FUNC(i8255_device::write));
	// frequently accesses DB after 5B, mirror? bug?
	map(0xDB, 0xDB).w("sn1", FUNC(sn76489_device::write));  // no idea what the sound chip is, this sounds terrible


}




static INPUT_PORTS_START( ecoinf3 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN0:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN0:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN0:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN1:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN1:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN1:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN1:18" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN1:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN1:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN1:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_DIPNAME( 0x20, 0x20, "IN2:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN2:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN2:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("PERKEY")
	PORT_DIPNAME( 0x0f, 0x00, "% Key?" )
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_DIPSETTING(    0x01, "0x01" )
	PORT_DIPSETTING(    0x02, "0x02" )
	PORT_DIPSETTING(    0x03, "0x03" )
	PORT_DIPSETTING(    0x04, "0x04" )
	PORT_DIPSETTING(    0x05, "0x05" )
	PORT_DIPSETTING(    0x06, "0x06" )
	PORT_DIPSETTING(    0x07, "0x07" )
	PORT_DIPSETTING(    0x08, "0x08" )
	PORT_DIPSETTING(    0x09, "0x09" )
	PORT_DIPSETTING(    0x0a, "0x0a" )
	PORT_DIPSETTING(    0x0b, "0x0b" )
	PORT_DIPSETTING(    0x0c, "0x0c" )
	PORT_DIPSETTING(    0x0d, "0x0d" )
	PORT_DIPSETTING(    0x0e, "0x0e" )
	PORT_DIPSETTING(    0x0f, "None" )
	PORT_DIPNAME( 0x10, 0x10, "PER_KEY:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "PER_KEY:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "PER_KEY:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "PER_KEY:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Nudge / Hold 3") PORT_CODE(KEYCODE_D)
	PORT_DIPNAME( 0x02, 0x02, "BT:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "BT:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Cancel Holds") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Nudge / Hold 1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Nudge / Hold 2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) // ?? advances through test items, spins the reels

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) // causes various spins etc. (but also causes the whole thing to freak out?)
	PORT_DIPNAME( 0x02, 0x02, "IN4:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN4:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN4:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN4:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN4:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN4:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN4:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN5:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN5:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN5:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Meter Connection (leave on)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN5:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN5:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN5:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, "IN6:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN6:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN6:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN6:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN6:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN6:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN6:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN6:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN7")
	PORT_DIPNAME( 0x01, 0x01, "IN7:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN7:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN7:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN7:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN7:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN7:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN7:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN7:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


void ecoinf3_state::ecoinf3_pyramid(machine_config &config)
{
	/* basic machine hardware */
	Z80180(config, m_maincpu, 8000000); // certainly not a plain z80 at least, invalid opcodes for that
	m_maincpu->set_addrmap(AS_PROGRAM, &ecoinf3_state::pyramid_memmap);
	m_maincpu->set_addrmap(AS_IO, &ecoinf3_state::pyramid_portmap);

	config.set_default_layout(layout_ecoinf3);

	SPEAKER(config, "mono").front_center();

	SN76489(config, "sn1", 4000000).add_route(ALL_OUTPUTS, "mono", 0.30); // no idea what the sound chip is, this sounds terrible

	i8255_device &ppia(I8255(config, "ppi8255_a"));
	ppia.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_a_read_a));
	ppia.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_a_write_a_strobedat0));
	ppia.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_a_read_b));
	ppia.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_a_write_b_strobedat1));
	ppia.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_a_read_c));
	ppia.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_a_write_c_strobe));

	i8255_device &ppib(I8255(config, "ppi8255_b"));
	ppib.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_b_read_a));
	ppib.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_b_write_a));
	ppib.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_b_read_b));
	ppib.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_b_write_b));
	ppib.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_b_read_c));
	ppib.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_b_write_c));

	i8255_device &ppic(I8255(config, "ppi8255_c"));
	ppic.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_c_read_a));
	ppic.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_c_write_a));
	ppic.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_c_read_b));
	ppic.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_c_write_b));
	ppic.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_c_read_c));
	ppic.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_c_write_c));

	i8255_device &ppid(I8255(config, "ppi8255_d"));
	ppid.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_d_read_a));
	ppid.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_d_write_a_reel01));
	ppid.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_d_read_b));
	ppid.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_d_write_b_reel23));
	ppid.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_d_read_c));
	ppid.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_d_write_c));

	i8255_device &ppie(I8255(config, "ppi8255_e"));
	ppie.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_e_read_a));
	ppie.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_e_write_a_alpha_display));    // alpha display characters
	ppie.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_e_read_b));
	ppie.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_e_write_b));  // not written at an appropriate time for it to be a 'send' address for the text
	ppie.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_e_read_c));
	ppie.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_e_write_c));  // not written at an appropriate time for it to be a 'send' address for the text

	i8255_device &ppif(I8255(config, "ppi8255_f"));
	ppif.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_f_read_a));
	ppif.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_f_write_a));
	ppif.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_f_read_b));
	ppif.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_f_write_b));
	ppif.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_f_read_c));
	ppif.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_f_write_c));

	i8255_device &ppig(I8255(config, "ppi8255_g"));
	ppig.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_g_read_a));
	ppig.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_g_write_a));
	ppig.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_g_read_b));
	ppig.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_g_write_b));
	ppig.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_g_read_c));
	ppig.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_g_write_c));

	i8255_device &ppih(I8255(config, "ppi8255_h"));
	ppih.in_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_h_read_a));
	ppih.out_pa_callback().set(FUNC(ecoinf3_state::ppi8255_intf_h_write_a));
	ppih.in_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_h_read_b));
	ppih.out_pb_callback().set(FUNC(ecoinf3_state::ppi8255_intf_h_write_b));
	ppih.in_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_h_read_c));
	ppih.out_pc_callback().set(FUNC(ecoinf3_state::ppi8255_intf_h_write_c));

	REEL(config, m_reels[0], ECOIN_200STEP_REEL, 12, 24, 0x09, 7, 200*2);
	m_reels[0]->optic_handler().set(FUNC(ecoinf3_state::reel_optic_cb<0>));
	REEL(config, m_reels[1], ECOIN_200STEP_REEL, 12, 24, 0x09, 7, 200*2);
	m_reels[1]->optic_handler().set(FUNC(ecoinf3_state::reel_optic_cb<1>));
	REEL(config, m_reels[2], ECOIN_200STEP_REEL, 12, 24, 0x09, 7, 200*2);
	m_reels[2]->optic_handler().set(FUNC(ecoinf3_state::reel_optic_cb<2>));
	REEL(config, m_reels[3], ECOIN_200STEP_REEL, 12, 24, 0x09, 7, 200*2);
	m_reels[3]->optic_handler().set(FUNC(ecoinf3_state::reel_optic_cb<3>));
}


/********************************************************************************************************************
 ROMs for PYRAMID Hw Type
********************************************************************************************************************/

ROM_START( ec_pyram )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* the last 0x2000 bytes (unmapped?) are the same as on the ec_sphin set */
	ROM_LOAD( "pyramid.bin", 0x0000, 0x010000, CRC(370a6d2c) SHA1(ea4f899adeca734529b19ba8de0e371841982c20) )
ROM_END

ROM_START( ec_pyrama )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "pyramid 5p 3.bin", 0x0000, 0x010000, CRC(06a047d8) SHA1(4a1a15f1ab9defd3a0c5f2d333beae0daa16c6a4) )
ROM_END

ROM_START( ec_sphin ) /* the last 0x2000 bytes (unmapped?) are the same as on the ec_pyram set */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "spnx5p", 0x0000, 0x010000, CRC(b4b49259) SHA1(a26172b659b739564b25dcc0f3f31f131a144d52) )
ROM_END

ROM_START( ec_sphina )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "spx10cv2.bin", 0x0000, 0x00e000, CRC(e2bf11a0) SHA1(f267385dbb06b2be8bcad7ae5e5804f5bb467f6d) )
ROM_END

ROM_START( ec_sphinb )
	ROM_REGION( 0x200000, "maincpu", 0 ) // z80 ROMS but truncated, other sets just seem to contain garbage at the end tho, so probably OK
	ROM_LOAD( "sphinx8c.bin", 0x0000, 0x00e000, CRC(f8e110fc) SHA1(4f55b5de87151f9127b84ffcf7f6f2e3ce34469f) )
ROM_END

ROM_START( ec_penni )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "pfh_8c.bin", 0x0000, 0x010000, CRC(282a42d8) SHA1(f985d238c72577e755090ce0f04dcc7850af6f3b) )
ROM_END

ROM_START( ec_pennia )
	ROM_REGION( 0x200000, "maincpu", 0 ) // z80 ROMS but truncated, other sets just seem to contain garbage at the end tho, so probably OK
	ROM_LOAD( "pfh_v6.bin", 0x0000, 0x00e000, CRC(febb3fce) SHA1(f8df085a563405ea5adcd15a4162a7ba56bcfad7) )
ROM_END


ROM_START( ec_stair )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD("sth5.4",      0x0000, 0x010000, CRC(879c8dcb) SHA1(358d9bb567da4b7913434d29dcd8a81c51c5fe2e) )
ROM_END

ROM_START( ec_staira )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD("sth5.8c", 0x0000, 0x010000, CRC(7ce6b760) SHA1(c828689481d7e187c504dd072bd6714222888d33) )
ROM_END



ROM_START( ec_laby ) // no header info with these
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "lab1v8.bin", 0x0000, 0x008000, CRC(16f0eeac) SHA1(9e28a6ae9176f730234dd8a7a8e50bad2904b611) )
	ROM_LOAD( "lab2v8.bin", 0x8000, 0x008000, CRC(14d7c58b) SHA1(e6b19523d96c9c1f39b743f8c52791465ab79637) )
ROM_END

ROM_START( ec_labya ) // no header info with these
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "laby10", 0x0000, 0x010000, CRC(a8b58fc3) SHA1(16e940b04fa85ff85a29197b4e45c8a39f5cad19) )
ROM_END

ROM_START( ec_secrt )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "scastle1.bin", 0x0000, 0x010000, CRC(e6abb596) SHA1(35518c46f1ddf1d3a85af13e4ba8bee07e804f64) )
ROM_END

void ecoinf3_state::init_ecoinf3()
{
}

void ecoinf3_state::init_ecoinf3_swap()
{
	// not all sets have this, are they just badly dumped?
	uint8_t table[] =
	{
		0x48, 0x4c, 0x49, 0x4d, 0x40, 0x44, 0x41, 0x45,     0x68, 0x78, 0x60, 0x70, 0x6a, 0x7a, 0x62, 0x72,
		0x08, 0x0c, 0x09, 0x0d, 0x00, 0x04, 0x01, 0x05,     0x6c, 0x7c, 0x64, 0x74, 0x6e, 0x7e, 0x66, 0x76,
		0x58, 0x5c, 0x59, 0x5d, 0x50, 0x54, 0x51, 0x55,     0x28, 0x38, 0x20, 0x30, 0x2a, 0x3a, 0x22, 0x32,
		0x18, 0x1c, 0x19, 0x1d, 0x10, 0x14, 0x11, 0x15,     0x2c, 0x3c, 0x24, 0x34, 0x2e, 0x3e, 0x26, 0x36,
		0x56, 0x52, 0x57, 0x53, 0x5e, 0x5a, 0x5f, 0x5b,     0x75, 0x65, 0x7d, 0x6d, 0x77, 0x67, 0x7f ,0x6f,
		0x16, 0x12, 0x17, 0x13, 0x1e, 0x1a, 0x1f, 0x1b,     0x71, 0x61, 0x79, 0x69, 0x73, 0x63, 0x7b, 0x6b,
		0x46, 0x42, 0x47, 0x43, 0x4e, 0x4a, 0x4f, 0x4b,     0x35, 0x25, 0x3d, 0x2d, 0x37, 0x27, 0x3f ,0x2f,
		0x06, 0x02, 0x07, 0x03, 0x0e, 0x0a, 0x0f, 0x0b,     0x31, 0x21, 0x39, 0x29, 0x33, 0x23, 0x3b, 0x2b,
	};

	auto buffer = std::make_unique<uint8_t[]>(0x10000);
	uint8_t *rom = memregion( "maincpu" )->base();
	for (int i = 0; i < 0x10000; i++)
	{
		buffer[i] = rom[(i&0xff80)|table[i&0x7f]];
	}

	memcpy(rom,buffer.get(),0x10000);

}

} // anonymous namespace


// another hw type (similar to stuff in ecoinf2.c) (watchdog on port 58?)
GAME( 19??, ec_pyram,  0,        ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3_swap, ROT0, "Electrocoin", "Pyramid (v1) (Electrocoin)",             MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_pyrama, ec_pyram, ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Pyramid (v6) (Electrocoin)",             MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_sphin,  0,        ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3_swap, ROT0, "Electrocoin", "Sphinx (v2) (Electrocoin) (set 1)",      MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_sphina, ec_sphin, ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Sphinx (v2) (Electrocoin) (set 2)",      MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_sphinb, ec_sphin, ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Sphinx (v1) (Electrocoin)",              MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_penni,  0,        ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Pennies From Heaven (v1) (Electrocoin)", MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_pennia, ec_penni, ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Pennies From Heaven (v6) (Electrocoin)", MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_stair,  0,        ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Stairway To Heaven (v11) (Electrocoin)", MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_staira, ec_stair, ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Stairway To Heaven (v1) (Electrocoin)",  MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_laby,   0,        ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Labyrinth (v8) (Electrocoin)",           MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_labya,  ec_laby,  ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Labyrinth (v10) (Electrocoin)",          MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_secrt,  0,        ecoinf3_pyramid, ecoinf3, ecoinf3_state, init_ecoinf3,      ROT0, "Electrocoin", "Secret Castle (v1) (Electrocoin)",       MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
