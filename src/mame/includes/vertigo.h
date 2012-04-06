/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

/*************************************
 *
 *  Typedefs
 *
 *************************************/

#define MC_LENGTH 512

typedef struct _am2901
{
	UINT32 ram[16];	  /* internal ram */
	UINT32 d;		  /* direct data D input */
	UINT32 q;		  /* Q register */
	UINT32 f;		  /* F ALU result */
	UINT32 y;		  /* Y output */
} am2901;

class vector_generator
{
public:
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	void set_machine(running_machine &machine) { m_machine = &machine; }

	UINT32 sreg;	  /* shift register */
	UINT32 l1;		  /* latch 1 adder operand only */
	UINT32 l2;		  /* latch 2 adder operand only */
	UINT32 c_v;		  /* vertical position counter */
	UINT32 c_h;		  /* horizontal position counter */
	UINT32 c_l;		  /* length counter */
	UINT32 adder_s;	  /* slope generator result and B input */
	UINT32 adder_a;	  /* slope generator A input */
	UINT32 color;	  /* color */
	UINT32 intensity; /* intensity */
	UINT32 brez;	  /* h/v-counters enable */
	UINT32 vfin;	  /* drawing yes/no */
	UINT32 hud1;	  /* h-counter up or down (stored in L1) */
	UINT32 hud2;	  /* h-counter up or down (stored in L2) */
	UINT32 vud1;	  /* v-counter up or down (stored in L1) */
	UINT32 vud2;	  /* v-counter up or down (stored in L2) */
	UINT32 hc1;		  /* use h- or v-counter in L1 mode */
	UINT32 ven;       /* vector intensity enable */

private:
	running_machine *m_machine;
};

typedef struct _microcode
{
	UINT32 x;
	UINT32 a;
	UINT32 b;
	UINT32 inst;
	UINT32 dest;
	UINT32 cn;
	UINT32 mreq;
	UINT32 rsel;
	UINT32 rwrite;
	UINT32 of;
	UINT32 iif;
	UINT32 oa;
	UINT32 jpos;
	UINT32 jmp;
	UINT32 jcon;
	UINT32 ma;
} microcode;

typedef struct _vproc
{
	UINT16 sram[64]; /* external sram */
	UINT16 ramlatch; /* latch between 2901 and sram */
	UINT16 rom_adr;	 /* vector ROM/RAM address latch */
	UINT32 pc;		 /* program counter */
	UINT32 ret;		 /* return address */

} vproc;


class vertigo_state : public driver_device
{
public:
	vertigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vectorram;
	device_t *m_ttl74148;
	device_t *m_custom;
	attotime m_irq4_time;
	UINT8 m_irq_state;
	UINT8 m_adc_result;
	vproc m_vs;
	am2901 m_bsp;
	vector_generator m_vgen;
	UINT16 *m_vectorrom;
	microcode m_mc[MC_LENGTH];
	DECLARE_READ16_MEMBER(vertigo_io_convert);
	DECLARE_READ16_MEMBER(vertigo_io_adc);
	DECLARE_READ16_MEMBER(vertigo_coin_r);
	DECLARE_WRITE16_MEMBER(vertigo_wsot_w);
	DECLARE_WRITE16_MEMBER(vertigo_audio_w);
	DECLARE_READ16_MEMBER(vertigo_sio_r);
	DECLARE_WRITE16_MEMBER(vertigo_motor_w);
};


/*----------- defined in machine/vertigo.c -----------*/

void vertigo_update_irq(device_t *device);

extern const struct pit8253_config vertigo_pit8254_config;


INTERRUPT_GEN( vertigo_interrupt );
MACHINE_START( vertigo );
MACHINE_RESET( vertigo );

/*----------- defined in video/vertigo.c -----------*/

void vertigo_vproc_init(running_machine &machine);
void vertigo_vproc_reset(running_machine &machine);
void vertigo_vproc(running_machine &machine, int cycles, int irq4);

