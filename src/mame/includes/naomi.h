/*

naomi.h -> NAOMI includes

*/

enum {
	JVSBD_DEFAULT = 0,
	JVSBD_ADSTICK,
	JVSBD_LIGHTGUN,
	JVSBD_MAHJONG,
	JVSBD_KEYBOARD
};
class naomi_state : public dc_state
{
	public:
		naomi_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag),
		pvr2_texture_ram(*this, "textureram2"),
		pvr2_framebuffer_ram(*this, "frameram2"),
		elan_ram(*this, "elan_ram") { }

	/* Naomi 2 specific (To be moved) */
	optional_shared_ptr<UINT64> pvr2_texture_ram;
	optional_shared_ptr<UINT64> pvr2_framebuffer_ram;
	optional_shared_ptr<UINT64> elan_ram;
	
	DECLARE_WRITE_LINE_MEMBER(aica_irq);
	DECLARE_MACHINE_RESET(naomi);
	DECLARE_DRIVER_INIT(atomiswave);
	DECLARE_DRIVER_INIT(naomigd);
	DECLARE_DRIVER_INIT(ggxx);
	DECLARE_DRIVER_INIT(ggxxrl);
	DECLARE_DRIVER_INIT(ggxxsla);
	DECLARE_DRIVER_INIT(naomi2);
	DECLARE_DRIVER_INIT(naomi);
	DECLARE_DRIVER_INIT(naomigd_mp);
	DECLARE_DRIVER_INIT(sfz3ugd);
	DECLARE_DRIVER_INIT(hotd2);
	DECLARE_DRIVER_INIT(qmegamis);
	DECLARE_DRIVER_INIT(gram2000);
	DECLARE_DRIVER_INIT(kick4csh);
	DECLARE_DRIVER_INIT(vf4evoct);
	DECLARE_DRIVER_INIT(naomi_mp);
	DECLARE_DRIVER_INIT(mvsc2);	
	
};
/*----------- defined in machine/gdcrypt.c -----------*/

extern void naomi_game_decrypt(running_machine& machine, UINT64 key, UINT8* region, int length);

/*----------- defined in machine/naomi.c -----------*/

extern int jvsboard_type;
extern UINT16 actel_id;

void naomi_g1_irq(running_machine &machine);
