/***************************************************************************

    Seibu Sound System v1.02, designed 1986 by Seibu Kaihatsu

    The Seibu sound system comprises of a Z80A, a YM3812, a YM3931*, and
    an Oki MSM6205.  As well as sound the Z80 can controls coins and pass
    data to the main cpu.  There are a few little quirks that make it
    worthwhile emulating in a seperate file:

    * The YM3812 generates interrupt RST10, by asserting the interrupt line,
    and placing 0xd7 on the data bus.

    * The main cpu generates interrupt RST18, by asserting the interrupt line,
    and placing 0xdf on the data bus.

    A problem can occur if both the YM3812 and the main cpu try to assert
    the interrupt line at the same time.  The effect in the old Mame
    emulation would be for sound to stop playing - this is because a RST18
    cancelled out a RST10, and if a single RST10 is dropped sound stops
    as the YM3812 timer is not reset.  The problem occurs because even
    if both interrupts happen at the same time, there can only be one value
    on the data bus.  Obviously the real hardware must have some circuit
    to prevent this.  It is emulated by user timers to control the z80
    interrupt vector.

    * The YM3931 is the main/sub cpu interface, similar to Konami's K054986A
      or Taito's TC0140SYT.  It also provides the Z80 memory map and
      interrupt control.  It's not a Yamaha chip :-)

    Emulation by Bryan McPhail, mish@tendril.co.uk
    ADPCM by R. Belmont and Jarek Burczynski

***************************************************************************/

#include "driver.h"
#include "streams.h"
#include "deprecat.h"
#include "audio/seibu.h"
#include "sound/3812intf.h"
#include "sound/2151intf.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"


/*
    Games using encrypted sound cpu:

    Air Raid         1987   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Cabal            1988   "Michel/Seibu    sound 11/04/88"
    Dead Angle       1988?  "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Dynamite Duke    1989   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Toki             1989   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden (alt)     1990   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."

    raiden and the decrypted raidena are not identical, there are vast sections of different data.
    However, there are a few different bytes in the middle of identical data, suggesting a possible
    error in the decryption scheme: they all require an additional XOR with 0x20 and are located at
    similar addresses.
    00002422: 03 23
    000024A1: 00 20
    000024A2: 09 29
    00002822: 48 68
    000028A1: 06 26
    00002A21: 17 37
    00002A22: 00 20
    00002AA1: 12 32
    00002C21: 02 22
    00002CA1: 02 22
    00002CA2: 17 37
*/


static UINT8 decrypt_data(int a,int src)
{
	if ( BIT(a,9)  &  BIT(a,8))             src ^= 0x80;
	if ( BIT(a,11) &  BIT(a,4) &  BIT(a,1)) src ^= 0x40;
	if ( BIT(a,11) & ~BIT(a,8) &  BIT(a,1)) src ^= 0x04;
	if ( BIT(a,13) & ~BIT(a,6) &  BIT(a,4)) src ^= 0x02;
	if (~BIT(a,11) &  BIT(a,9) &  BIT(a,2)) src ^= 0x01;

	if (BIT(a,13) &  BIT(a,4)) src = BITSWAP8(src,7,6,5,4,3,2,0,1);
	if (BIT(a, 8) &  BIT(a,4)) src = BITSWAP8(src,7,6,5,4,2,3,1,0);

	return src;
}

static UINT8 decrypt_opcode(int a,int src)
{
	if ( BIT(a,9)  &  BIT(a,8))             src ^= 0x80;
	if ( BIT(a,11) &  BIT(a,4) &  BIT(a,1)) src ^= 0x40;
	if (~BIT(a,13) & BIT(a,12))             src ^= 0x20;
	if (~BIT(a,6)  &  BIT(a,1))             src ^= 0x10;
	if (~BIT(a,12) &  BIT(a,2))             src ^= 0x08;
	if ( BIT(a,11) & ~BIT(a,8) &  BIT(a,1)) src ^= 0x04;
	if ( BIT(a,13) & ~BIT(a,6) &  BIT(a,4)) src ^= 0x02;
	if (~BIT(a,11) &  BIT(a,9) &  BIT(a,2)) src ^= 0x01;

	if (BIT(a,13) &  BIT(a,4)) src = BITSWAP8(src,7,6,5,4,3,2,0,1);
	if (BIT(a, 8) &  BIT(a,4)) src = BITSWAP8(src,7,6,5,4,2,3,1,0);
	if (BIT(a,12) &  BIT(a,9)) src = BITSWAP8(src,7,6,4,5,3,2,1,0);
	if (BIT(a,11) & ~BIT(a,6)) src = BITSWAP8(src,6,7,5,4,3,2,1,0);

	return src;
}

void seibu_sound_decrypt(int cpu_region,int length)
{
	UINT8 *decrypt = auto_malloc(length);
	UINT8 *rom = memory_region(cpu_region);
	int i;

	memory_set_decrypted_region(cpu_region - REGION_CPU1, 0x0000, (length < 0x10000) ? (length - 1) : 0x1fff, decrypt);

	for (i = 0;i < length;i++)
	{
		UINT8 src = rom[i];

		rom[i]      = decrypt_data(i,src);
		decrypt[i]  = decrypt_opcode(i,src);
	}

	if (length > 0x10000)
		memory_configure_bank_decrypted(1, 0, (length - 0x10000) / 0x8000, decrypt + 0x10000, 0x8000);
}


/***************************************************************************/

/*
    Handlers for early Seibu/Tad games with dual channel ADPCM
*/

static struct seibu_adpcm_state
{
	struct adpcm_state adpcm;
	sound_stream *stream;
	UINT32 current, end;
	UINT8 nibble;
	UINT8 playing;
	UINT8 allocated;
	UINT8 *base;
} seibu_adpcm[2];

static void seibu_adpcm_callback(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	struct seibu_adpcm_state *state = param;
	stream_sample_t *dest = outputs[0];

	while (state->playing && samples > 0)
	{
		int val = (state->base[state->current] >> state->nibble) & 15;

		state->nibble ^= 4;
		if (state->nibble == 4)
		{
			state->current++;
			if (state->current >= state->end)
				state->playing = 0;
		}

		*dest++ = clock_adpcm(&state->adpcm, val);
		samples--;
	}
	while (samples > 0)
	{
		*dest++ = 0;
		samples--;
	}
}

static void *seibu_adpcm_start(int clock, const struct CustomSound_interface *config)
{
	int i;

	for (i = 0; i < 2; i++)
		if (!seibu_adpcm[i].allocated)
		{
			struct seibu_adpcm_state *state = &seibu_adpcm[i];
			state->allocated = 1;
			state->playing = 0;
			state->stream = stream_create(0, 1, clock, state, seibu_adpcm_callback);
			state->base = memory_region(REGION_SOUND1);
			reset_adpcm(&state->adpcm);
			return state;
		}
	return NULL;
}

static void seibu_adpcm_stop(void *token)
{
	struct seibu_adpcm_state *state = token;
	state->allocated = 0;
}

// "decrypt" is a bit flowery here, as it's probably just line-swapping to
// simplify PCB layout/routing rather than intentional protection, but it
// still fits, especially since the Z80s for all these games are truly encrypted.

void seibu_adpcm_decrypt(int region)
{
	UINT8 *ROM = memory_region(region);
	int i;

	for (i = 0; i < memory_region_length(region); i++)
	{
		ROM[i] = BITSWAP8(ROM[i], 7, 5, 3, 1, 6, 4, 2, 0);
	}
}

WRITE8_HANDLER( seibu_adpcm_adr_1_w )
{
	if (seibu_adpcm[0].stream)
		stream_update(seibu_adpcm[0].stream);
	if (offset)
	{
		seibu_adpcm[0].end = data<<8;
	}
	else
	{
		seibu_adpcm[0].current = data<<8;
		seibu_adpcm[0].nibble = 4;
	}
}

WRITE8_HANDLER( seibu_adpcm_ctl_1_w )
{
	// sequence is 00 02 01 each time.
	if (seibu_adpcm[0].stream)
		stream_update(seibu_adpcm[0].stream);
	switch (data)
	{
		case 0:
			seibu_adpcm[0].playing = 0;
			break;
		case 2:
			break;
		case 1:
			seibu_adpcm[0].playing = 1;
			break;

	}
}

WRITE8_HANDLER( seibu_adpcm_adr_2_w )
{
	if (seibu_adpcm[1].stream)
		stream_update(seibu_adpcm[1].stream);
	if (offset)
	{
		seibu_adpcm[1].end = (data<<8) + 0x10000;
	}
	else
	{
		seibu_adpcm[1].current = (data<<8) + 0x10000;
		seibu_adpcm[1].nibble = 4;
	}
}

WRITE8_HANDLER( seibu_adpcm_ctl_2_w )
{
	// sequence is 00 02 01 each time.
	if (seibu_adpcm[1].stream)
		stream_update(seibu_adpcm[1].stream);
	switch (data)
	{
		case 0:
			seibu_adpcm[1].playing = 0;
			break;
		case 2:
			break;
		case 1:
			seibu_adpcm[1].playing = 1;
			break;

	}
}

/***************************************************************************/

static int sound_cpu;

enum
{
	VECTOR_INIT,
	RST10_ASSERT,
	RST10_CLEAR,
	RST18_ASSERT,
	RST18_CLEAR
};

static void update_irq_lines(running_machine *machine, int param)
{
	static int irq1,irq2;

	switch(param)
	{
		case VECTOR_INIT:
			irq1 = irq2 = 0xff;
			break;

		case RST10_ASSERT:
			irq1 = 0xd7;
			break;

		case RST10_CLEAR:
			irq1 = 0xff;
			break;

		case RST18_ASSERT:
			irq2 = 0xdf;
			break;

		case RST18_CLEAR:
			irq2 = 0xff;
			break;
	}

	if ((irq1 & irq2) == 0xff)	/* no IRQs pending */
		cpunum_set_input_line(machine, sound_cpu,0,CLEAR_LINE);
	else	/* IRQ pending */
		cpunum_set_input_line_and_vector(machine, sound_cpu,0,ASSERT_LINE,irq1 & irq2);
}

WRITE8_HANDLER( seibu_irq_clear_w )
{
	update_irq_lines(Machine, VECTOR_INIT);
}

WRITE8_HANDLER( seibu_rst10_ack_w )
{
	/* Unused for now */
}

WRITE8_HANDLER( seibu_rst18_ack_w )
{
	update_irq_lines(Machine, RST18_CLEAR);
}

void seibu_ym3812_irqhandler(int linestate)
{
	update_irq_lines(Machine, linestate ? RST10_ASSERT : RST10_CLEAR);
}

void seibu_ym2151_irqhandler(int linestate)
{
	update_irq_lines(Machine, linestate ? RST10_ASSERT : RST10_CLEAR);
}

void seibu_ym2203_irqhandler(int linestate)
{
	update_irq_lines(Machine, linestate ? RST10_ASSERT : RST10_CLEAR);
}

/***************************************************************************/

/* Use this if the sound cpu is cpu 1 */
MACHINE_RESET( seibu_sound_1 )
{
	int romlength = memory_region_length(REGION_CPU2);
	UINT8 *rom = memory_region(REGION_CPU2);

	sound_cpu=1;
	update_irq_lines(machine, VECTOR_INIT);
	if (romlength > 0x10000)
		memory_configure_bank(1, 0, (romlength - 0x10000) / 0x8000, rom + 0x10000, 0x8000);
}

/* Use this if the sound cpu is cpu 2 */
MACHINE_RESET( seibu_sound_2 )
{
	int romlength = memory_region_length(REGION_CPU3);
	UINT8 *rom = memory_region(REGION_CPU3);

	sound_cpu=2;
	update_irq_lines(machine, VECTOR_INIT);
	if (romlength > 0x10000)
		memory_configure_bank(1, 0, (romlength - 0x10000) / 0x8000, rom + 0x10000, 0x8000);
}

/***************************************************************************/

static UINT8 main2sub[2],sub2main[2];
static int main2sub_pending,sub2main_pending;

WRITE8_HANDLER( seibu_bank_w )
{
	memory_set_bank(1, data & 1);
}

WRITE8_HANDLER( seibu_coin_w )
{
	coin_counter_w(0,data & 1);
	coin_counter_w(1,data & 2);
}

READ8_HANDLER( seibu_soundlatch_r )
{
	return main2sub[offset];
}

READ8_HANDLER( seibu_main_data_pending_r )
{
	return sub2main_pending ? 1 : 0;
}

WRITE8_HANDLER( seibu_main_data_w )
{
	sub2main[offset] = data;
}

static WRITE8_HANDLER( seibu_pending_w )
{
	/* just a guess */
	main2sub_pending = 0;
	sub2main_pending = 1;
}

READ16_HANDLER( seibu_main_word_r )
{
	//logerror("%06x: seibu_main_word_r(%x)\n",activecpu_get_pc(),offset);
	switch (offset)
	{
		case 2:
		case 3:
			return sub2main[offset-2];
		case 5:
			return main2sub_pending ? 1 : 0;
		default:
			//logerror("%06x: seibu_main_word_r(%x)\n",activecpu_get_pc(),offset);
			return 0xffff;
	}
}

WRITE16_HANDLER( seibu_main_word_w )
{
	//logerror("%06x: seibu_main_word_w(%x,%02x)\n",activecpu_get_pc(),offset,data);
	if (ACCESSING_BITS_0_7)
	{
		switch (offset)
		{
			case 0:
			case 1:
				main2sub[offset] = data;
				break;
			case 4:
				update_irq_lines(Machine, RST18_ASSERT);
				break;
			case 6:
				/* just a guess */
				sub2main_pending = 0;
				main2sub_pending = 1;
				break;
			default:
				//logerror("%06x: seibu_main_word_w(%x,%02x)\n",activecpu_get_pc(),offset,data);
				break;
		}
	}
}

READ8_HANDLER( seibu_main_v30_r )
{
	return seibu_main_word_r(machine,offset/2,0) >> (8 * (offset & 1));
}

WRITE8_HANDLER( seibu_main_v30_w )
{
	seibu_main_word_w(machine,offset/2,data << (8 * (offset & 1)),0xff00 >> (8 * (offset & 1)));
}

WRITE16_HANDLER( seibu_main_mustb_w )
{
	main2sub[0] = data&0xff;
	main2sub[1] = data>>8;

//  logerror("seibu_main_mustb_w: %x -> %x %x\n", data, main2sub[0], main2sub[1]);

	update_irq_lines(Machine, RST18_ASSERT);
}

/***************************************************************************/

const struct YM3812interface seibu_ym3812_interface =
{
	seibu_ym3812_irqhandler
};

const struct CustomSound_interface seibu_adpcm_interface =
{
	seibu_adpcm_start,
	seibu_adpcm_stop
};

const struct YM2151interface seibu_ym2151_interface =
{
	seibu_ym2151_irqhandler
};

const struct YM2203interface seibu_ym2203_interface =
{
	0,0,0,0,seibu_ym2203_irqhandler
};

/***************************************************************************/

ADDRESS_MAP_START( seibu_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(seibu_pending_w)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(seibu_irq_clear_w)
	AM_RANGE(0x4002, 0x4002) AM_WRITE(seibu_rst10_ack_w)
	AM_RANGE(0x4003, 0x4003) AM_WRITE(seibu_rst18_ack_w)
	AM_RANGE(0x4007, 0x4007) AM_WRITE(seibu_bank_w)
	AM_RANGE(0x4008, 0x4008) AM_READWRITE(YM3812_status_port_0_r, YM3812_control_port_0_w)
	AM_RANGE(0x4009, 0x4009) AM_WRITE(YM3812_write_port_0_w)
	AM_RANGE(0x4010, 0x4011) AM_READ(seibu_soundlatch_r)
	AM_RANGE(0x4012, 0x4012) AM_READ(seibu_main_data_pending_r)
	AM_RANGE(0x4013, 0x4013) AM_READ_PORT("COIN")
	AM_RANGE(0x4018, 0x4019) AM_WRITE(seibu_main_data_w)
	AM_RANGE(0x401b, 0x401b) AM_WRITE(seibu_coin_w)
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK(1)
ADDRESS_MAP_END


ADDRESS_MAP_START( seibu2_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(seibu_pending_w)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(seibu_irq_clear_w)
	AM_RANGE(0x4002, 0x4002) AM_WRITE(seibu_rst10_ack_w)
	AM_RANGE(0x4003, 0x4003) AM_WRITE(seibu_rst18_ack_w)
	AM_RANGE(0x4007, 0x4007) AM_WRITE(seibu_bank_w)
	AM_RANGE(0x4008, 0x4008) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x4009, 0x4009) AM_READWRITE(YM2151_status_port_0_r, YM2151_data_port_0_w)
	AM_RANGE(0x4010, 0x4011) AM_READ(seibu_soundlatch_r)
	AM_RANGE(0x4012, 0x4012) AM_READ(seibu_main_data_pending_r)
	AM_RANGE(0x4013, 0x4013) AM_READ_PORT("COIN")
	AM_RANGE(0x4018, 0x4019) AM_WRITE(seibu_main_data_w)
	AM_RANGE(0x401b, 0x401b) AM_WRITE(seibu_coin_w)
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK(1)
ADDRESS_MAP_END



ADDRESS_MAP_START( seibu3_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(seibu_pending_w)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(seibu_irq_clear_w)
	AM_RANGE(0x4002, 0x4002) AM_WRITE(seibu_rst10_ack_w)
	AM_RANGE(0x4003, 0x4003) AM_WRITE(seibu_rst18_ack_w)
	AM_RANGE(0x4007, 0x4007) AM_WRITE(seibu_bank_w)
	AM_RANGE(0x4008, 0x4008) AM_READWRITE(YM2203_status_port_0_r, YM2203_control_port_0_w)
	AM_RANGE(0x4009, 0x4009) AM_READWRITE(YM2203_read_port_0_r, YM2203_write_port_0_w)
	AM_RANGE(0x4010, 0x4011) AM_READ(seibu_soundlatch_r)
	AM_RANGE(0x4012, 0x4012) AM_READ(seibu_main_data_pending_r)
	AM_RANGE(0x4013, 0x4013) AM_READ_PORT("COIN")
	AM_RANGE(0x4018, 0x4019) AM_WRITE(seibu_main_data_w)
	AM_RANGE(0x401b, 0x401b) AM_WRITE(seibu_coin_w)
	AM_RANGE(0x6008, 0x6008) AM_READWRITE(YM2203_status_port_1_r, YM2203_control_port_1_w)
	AM_RANGE(0x6009, 0x6009) AM_READWRITE(YM2203_read_port_1_r, YM2203_write_port_1_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK(1)
ADDRESS_MAP_END

ADDRESS_MAP_START( seibu3_adpcm_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(seibu_pending_w)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(seibu_irq_clear_w)
	AM_RANGE(0x4002, 0x4002) AM_WRITE(seibu_rst10_ack_w)
	AM_RANGE(0x4003, 0x4003) AM_WRITE(seibu_rst18_ack_w)
	AM_RANGE(0x4005, 0x4006) AM_WRITE(seibu_adpcm_adr_1_w)
	AM_RANGE(0x4007, 0x4007) AM_WRITE(seibu_bank_w)
	AM_RANGE(0x4008, 0x4008) AM_READWRITE(YM2203_status_port_0_r, YM2203_control_port_0_w)
	AM_RANGE(0x4009, 0x4009) AM_READWRITE(YM2203_read_port_0_r, YM2203_write_port_0_w)
	AM_RANGE(0x4010, 0x4011) AM_READ(seibu_soundlatch_r)
	AM_RANGE(0x4012, 0x4012) AM_READ(seibu_main_data_pending_r)
	AM_RANGE(0x4013, 0x4013) AM_READ_PORT("COIN")
	AM_RANGE(0x4018, 0x4019) AM_WRITE(seibu_main_data_w)
	AM_RANGE(0x401a, 0x401a) AM_WRITE(seibu_adpcm_ctl_1_w)
	AM_RANGE(0x401b, 0x401b) AM_WRITE(seibu_coin_w)
	AM_RANGE(0x6005, 0x6006) AM_WRITE(seibu_adpcm_adr_2_w)
	AM_RANGE(0x6008, 0x6008) AM_READWRITE(YM2203_status_port_1_r, YM2203_control_port_1_w)
	AM_RANGE(0x6009, 0x6009) AM_READWRITE(YM2203_read_port_1_r, YM2203_write_port_1_w)
	AM_RANGE(0x601a, 0x601a) AM_WRITE(seibu_adpcm_ctl_2_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK(1)
ADDRESS_MAP_END
