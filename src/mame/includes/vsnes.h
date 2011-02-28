class vsnes_state : public driver_device
{
public:
	vsnes_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *work_ram;
	UINT8 *work_ram_1;
	int coin;
	int do_vrom_bank;
	int input_latch[4];
	int sound_fix;
	UINT8 last_bank;
	UINT8* vram;
	UINT8* vrom[2];
	UINT8* nt_ram[2];
	UINT8* nt_page[2][4];
	UINT32 vrom_size[2];
	int vrom_banks;
	int zapstore;
	int old_bank;
	int drmario_shiftreg;
	int drmario_shiftcount;
	int size16k;
	int switchlow;
	int vrom4k;
	int MMC3_cmd;
	int MMC3_prg_bank[4];
	int MMC3_chr_bank[6];
	int MMC3_prg_mask;
	int IRQ_enable;
	int IRQ_count;
	int IRQ_count_latch;
	int VSindex;
	int supxevs_prot_index;
	int security_counter;
	int ret;
};


/*----------- defined in video/vsnes.c -----------*/

VIDEO_START( vsnes );
PALETTE_INIT( vsnes );
SCREEN_UPDATE( vsnes );
VIDEO_START( vsdual );
SCREEN_UPDATE( vsnes_bottom );
PALETTE_INIT( vsdual );

extern const ppu2c0x_interface vsnes_ppu_interface_1;
extern const ppu2c0x_interface vsnes_ppu_interface_2;


/*----------- defined in machine/vsnes.c -----------*/

MACHINE_RESET( vsnes );
MACHINE_RESET( vsdual );
MACHINE_START( vsnes );
MACHINE_START( vsdual );

DRIVER_INIT( vsnormal );
DRIVER_INIT( vsgun );
DRIVER_INIT( vskonami );
DRIVER_INIT( vsvram );
DRIVER_INIT( drmario );
DRIVER_INIT( rbibb );
DRIVER_INIT( tkoboxng );
DRIVER_INIT( MMC3 );
DRIVER_INIT( platoon );
DRIVER_INIT( supxevs );
DRIVER_INIT( bnglngby );
DRIVER_INIT( vsgshoe );
DRIVER_INIT( vsfdf );
DRIVER_INIT( vsdual );

READ8_HANDLER( vsnes_in0_r );
READ8_HANDLER( vsnes_in1_r );
READ8_HANDLER( vsnes_in0_1_r );
READ8_HANDLER( vsnes_in1_1_r );
WRITE8_HANDLER( vsnes_in0_w );
WRITE8_HANDLER( vsnes_in0_1_w );
