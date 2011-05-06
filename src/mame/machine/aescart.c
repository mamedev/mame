/**********************************************************************

    aescart.c

    Neo-Geo AES cartridge management
    R. Belmont, 2009

    Based on ti99cart.c by Michael Zapf

    We leave the support in place for multiple slots in case MAME
    uses this at some future point for MVS multi-cart.

*********************************************************************/
#include "emu.h"
#include "aescart.h"
#include "imagedev/cartslot.h"
#include "imagedev/multcart.h"
#include "includes/neogeo.h"

typedef int assmfct(running_machine &machine, device_t *);

enum
{
	AESCART_FCT_ASSM = DEVINFO_FCT_DEVICE_SPECIFIC,
	AESCART_FCT_DISASSM
};

struct _aes_multicart_t
{
	/* Reserves space for all cartridges. This is also used in the legacy
       cartridge system, but only for slot 0. */
	aescartridge_t cartridge[AES_NUMBER_OF_CARTRIDGE_SLOTS];

	/* Determines which slot is currently active. This value is changed when there
    are accesses to other GROM base addresses. */
	int active_slot;

	/* Used in order to enforce a special slot. This value is retrieved
       from the dipswitch setting. A value of -1 means automatic, that is,
       the grom base switch is used. Values 0 .. max refer to the
       respective slot. */
	int fixed_slot;

	/* Holds the highest index of a cartridge being plugged in plus one.
       If we only have one cartridge inserted, we don't want to get a
       selection option, so we just mirror the memory contents. */
	int next_free_slot;

	/* Counts the number of slots which currently contain legacy format
       cartridge images. */
	int legacy_slots;

	/* Counts the number of slots which currently contain new format
       cartridge images. */
	int multi_slots;
};
typedef struct _aes_multicart_t aes_multicart_t;

#define AUTO -1

/* Access to the pcb. Contained in the token of the pcb instance. */
struct _aes_pcb_t
{
	/* Link up to the cartridge structure which contains this pcb. */
	aescartridge_t *cartridge;

	/* Function to assemble this cartridge. */
	assmfct	*assemble;

	/* Function to disassemble this cartridge. */
	assmfct	*disassemble;
};
typedef struct _aes_pcb_t aes_pcb_t;

INLINE aes_multicart_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == AES_MULTICART);

	return (aes_multicart_t *)downcast<legacy_device_base *>(device)->token();
}

INLINE aes_pcb_t *get_safe_pcb_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == AES_CARTRIDGE_PCB_NONE || device->type() == AES_CARTRIDGE_PCB_STD);

	return (aes_pcb_t *)downcast<legacy_device_base *>(device)->token();
}

INLINE cartslot_t *get_safe_cartslot_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CARTSLOT);

	return (cartslot_t *)downcast<legacy_device_base *>(device)->token();
}

/*
    Find the index of the cartridge name. We assume the format
    <name><number>, i.e. the number is the longest string from the right
    which can be interpreted as a number.
*/
static int get_index_from_tagname(device_t *image)
{
	const char *tag = image->tag();
	int maxlen = strlen(tag);
	int i;
	for (i=maxlen-1; i >=0; i--)
		if (tag[i] < 48 || tag[i] > 57) break;

	return atoi(tag+i+1);
}

/*
    Common routine to assemble cartridges from resources.
*/
static aescartridge_t *assemble_common(running_machine &machine, device_t *cartslot)
{
	/* Pointer to the cartridge structure. */
	aescartridge_t *cartridge;
	device_t *cartsys = cartslot->owner();
	aes_multicart_t *cartslots = get_safe_token(cartsys);
	UINT8 *socketcont, *romrgn;
	int reslength, i, blockofs;
	char sprname1[16], sprname2[16];

	int slotnumber = get_index_from_tagname(cartslot)-1;
	assert(slotnumber>=0 && slotnumber<AES_NUMBER_OF_CARTRIDGE_SLOTS);

	/* There is a cartridge in this slot, check the maximum slot number. */
	if (cartslots->next_free_slot <= slotnumber)
	{
		cartslots->next_free_slot = slotnumber+1;
	}
	cartridge = &cartslots->cartridge[slotnumber];

	// check for up to 4 program ROMs
	romrgn = (UINT8 *)machine.region("maincpu")->base();
	blockofs = 0;
	for (i = 0; i < 4; i++)
	{
		sprintf(sprname1, "p%d", i+1);

		socketcont = (UINT8*)cartslot_get_socket(cartslot, sprname1);
		reslength = cartslot_get_resource_length(cartslot, sprname1);

		if (socketcont != NULL)
		{
			memcpy(romrgn+blockofs, socketcont, reslength);
			blockofs += reslength;
		}
	}

	// 1 m1 ROM
	socketcont = (UINT8*)cartslot_get_socket(cartslot, "m1");
	reslength = cartslot_get_resource_length(cartslot, "m1");
	if (socketcont != NULL)
	{
		romrgn = (UINT8 *)machine.region("audiocpu")->base();

		memcpy(romrgn, socketcont, reslength);
		// mirror (how does this really work?)
		memcpy(romrgn+0x10000, socketcont, reslength);
	}

	// up to 8 YM sample ROMs
	romrgn = (UINT8 *)machine.region("ymsnd")->base();
	blockofs = 0;
	for (i = 0; i < 8; i++)
	{
		sprintf(sprname1, "v1%d", i+1);

		socketcont = (UINT8*)cartslot_get_socket(cartslot, sprname1);
		reslength = cartslot_get_resource_length(cartslot, sprname1);

		if (socketcont != NULL)
		{
			memcpy(romrgn+blockofs, socketcont, reslength);
			blockofs += reslength;
		}
	}

	// up to 8 YM delta-T sample ROMs
	romrgn = (UINT8 *)machine.region("ymsnd.deltat")->base();
	blockofs = 0;
	for (i = 0; i < 8; i++)
	{
		sprintf(sprname1, "v2%d", i+1);

		socketcont = (UINT8*)cartslot_get_socket(cartslot, sprname1);
		reslength = cartslot_get_resource_length(cartslot, sprname1);

		if (socketcont != NULL)
		{
			memcpy(romrgn+blockofs, socketcont, reslength);
			blockofs += reslength;
		}
	}

	// 1 s1 ROM
	socketcont = (UINT8*)cartslot_get_socket(cartslot, "s1");
	reslength = cartslot_get_resource_length(cartslot, "s1");
	if (socketcont != NULL)
	{
		romrgn = (UINT8 *)machine.region("fixed")->base();

		memcpy(romrgn, socketcont, reslength);
	}

	// up to 8 sprite ROMs in byte-interleaved pairs
	romrgn = (UINT8 *)machine.region("sprites")->base();
	blockofs = 0;
	for (i = 0; i < 8; i+=2)
	{
		UINT8 *spr1, *spr2;
		int j;

		sprintf(sprname1, "c%d", i+1);
		sprintf(sprname2, "c%d", i+2);

		spr1 = (UINT8*)cartslot_get_socket(cartslot, sprname1);
		spr2 = (UINT8*)cartslot_get_socket(cartslot, sprname2);
		reslength = cartslot_get_resource_length(cartslot, sprname1);

		if ((spr1) && (spr2))
		{
			for (j = 0; j < reslength; j++)
			{
				romrgn[blockofs + (j*2)] = spr1[j];
				romrgn[blockofs + (j*2) + 1] = spr2[j];
			}

			blockofs += reslength*2;
		}
	}

	return cartridge;
}

static void set_pointers(device_t *pcb, int index)
{
	device_t *cartsys = pcb->owner()->owner();
	aes_multicart_t *cartslots = get_safe_token(cartsys);
	aes_pcb_t *pcb_def = get_safe_pcb_token(pcb);

	pcb_def->assemble = (assmfct *)downcast<const legacy_cart_slot_device_base *>(pcb)->get_legacy_fct(AESCART_FCT_ASSM);
	pcb_def->disassemble = (assmfct *)downcast<const legacy_cart_slot_device_base *>(pcb)->get_legacy_fct(AESCART_FCT_DISASSM);

	pcb_def->cartridge = &cartslots->cartridge[index];
	pcb_def->cartridge->pcb = pcb;
}

/*****************************************************************************
  Cartridge type: None
    This PCB device is just a pseudo device; the legacy mode is handled
    by dedicated functions.
******************************************************************************/
static DEVICE_START(aes_pcb_none)
{
	/* device is aes_cartslot:cartridge:pcb */
//  printf("DEVICE_START(aes_pcb_none), tag of device=%s\n", device->tag());
	set_pointers(device, get_index_from_tagname(device->owner())-1);
}

/*****************************************************************************
  Cartridge type: Standard
    Most cartridges are built in this type. This includes word-swapped program,
    z80 program, YM samples, delta-T samples, sprites, and sfix.
******************************************************************************/

static DEVICE_START(aes_pcb_std)
{
	/* device is aes_cartslot:cartridge:pcb */
//  printf("DEVICE_START(aes_pcb_std), tag of device=%s\n", device->tag());
	set_pointers(device, get_index_from_tagname(device->owner())-1);
}

/*
    The standard cartridge assemble routine. We just call the common
    function here.
*/
static int assemble_std(running_machine &machine, device_t *image)
{
//  aescartridge_t *cart;
//  printf("assemble_std, %s\n", image->tag);
	/*cart = */assemble_common(machine, image);

	return IMAGE_INIT_PASS;
}

/*
    Removes pointers and restores the state before plugging in the
    cartridge.
    The pointer to the location after the last cartridge is adjusted.
    As it seems, we can use the same function for the disassembling of all
    cartridge types.
*/
static int disassemble_std(device_t *image)
{
//  int slotnumber;
//  int i;
//  aescartridge_t *cart;
//  device_t *cartsys = image->owner();
//  aes_multicart_t *cartslots = (aes_multicart_t *)cartsys->token;

//  slotnumber = get_index_from_tagname(image)-1;
//  printf("Disassemble cartridge %d\n", slotnumber);
#if 0
	/* Search the highest remaining cartridge. */
	cartslots->next_free_slot = 0;
	for (i=AES_NUMBER_OF_CARTRIDGE_SLOTS-1; i >= 0; i--)
	{
		if (i != slotnumber)
		{
			if (0) //!slot_is_empty(cartsys, i))    (for future use?)
			{
				cartslots->next_free_slot = i+1;
//              printf("Setting new next_free_slot to %d\n", cartslots->next_free_slot);
				break;
			}
		}
	}
#endif
	/* Do we have RAM? If so, swap the bytes (undo the BIG_ENDIANIZE) */
//  cart = &cartslots->cartridge[slotnumber];

//  clear_slot(cartsys, slotnumber);

	return IMAGE_INIT_PASS;
}

/*****************************************************************************
  Device metadata
******************************************************************************/

static DEVICE_GET_INFO(aes_cart_common)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:
			info->i = sizeof(aes_pcb_t);
			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:
			info->i = 0;
			break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_START:
			info->start = DEVICE_START_NAME(aes_pcb_std);
			break;
		case DEVINFO_FCT_STOP:
			/* Nothing */
			break;
		case DEVINFO_FCT_RESET:
			/* Nothing */
			break;

		case AESCART_FCT_ASSM:
			info->f = (genf *) assemble_std;
			break;

		case AESCART_FCT_DISASSM:
			info->f = (genf *) disassemble_std;
			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:
			strcpy(info->s, "AES standard cartridge pcb");
			break;
		case DEVINFO_STR_FAMILY:
			strcpy(info->s, "AES cartridge pcb");
			break;
		case DEVINFO_STR_VERSION:
			strcpy(info->s, "1.0");
			break;
		case DEVINFO_STR_SOURCE_FILE:
			strcpy(info->s, __FILE__);
			break;
		case DEVINFO_STR_CREDITS:
			/* Nothing */
			break;
	}
}

DEVICE_GET_INFO(aes_cartridge_pcb_none)
{
	switch(state)
	{
		case DEVINFO_FCT_START:
			info->start = DEVICE_START_NAME(aes_pcb_none);
			break;

		case DEVINFO_STR_NAME:
			strcpy(info->s, "AES empty cartridge");
			break;

		default:
			DEVICE_GET_INFO_CALL(aes_cart_common);
			break;
	}
}

DEVICE_GET_INFO(aes_cartridge_pcb_std)
{
	DEVICE_GET_INFO_CALL(aes_cart_common);
}

/*****************************************************************************
  The cartridge handling of the multi-cartridge system.
  Every cartridge contains a PCB device. The memory handlers delegate the calls
  to the respective handlers of the cartridges.
******************************************************************************/
/*
    Initialize a cartridge. Each cartridge contains a PCB device.
*/
static DEVICE_START( aes_cartridge )
{
	cartslot_t *cart = get_safe_cartslot_token(device);

	/* find the PCB device */
	cart->pcb_device = device->subdevice(TAG_PCB);
}

// handle protected carts
static void install_protection(device_image_interface& image)
{
	neogeo_state *state = image.device().machine().driver_data<neogeo_state>();
	const char *crypt_feature = image.get_feature( "crypt" );

	if(crypt_feature == NULL)
		return;

	if(strcmp(crypt_feature,"fatfury2_prot") == 0)
	{
		fatfury2_install_protection(image.device().machine());
		logerror("Installed Fatal Fury 2 protection\n");
	}
	if(strcmp(crypt_feature,"kof99_crypt") == 0)
	{
		kof99_decrypt_68k(image.device().machine());
		state->m_fixed_layer_bank_type = 1;
		kof99_neogeo_gfx_decrypt(image.device().machine(), 0x00);
		kof99_install_protection(image.device().machine());
		logerror("Decrypted KOF99 code and graphics.\n");
	}
	if(strcmp(crypt_feature,"mslug3_crypt") == 0)
	{
		state->m_fixed_layer_bank_type = 1;
		kof99_neogeo_gfx_decrypt(image.device().machine(), 0xad);
		logerror("Decrypted Metal Slug 3 graphics\n");
	}
	if(strcmp(crypt_feature,"matrim_crypt") == 0)
	{
		matrim_decrypt_68k(image.device().machine());
		neo_pcm2_swap(image.device().machine(), 1);
		state->m_fixed_layer_bank_type = 2;
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x6a);
		logerror("Decrypted Matrimelee code, sound and graphics\n");
	}
	if(strcmp(crypt_feature,"svc_crypt") == 0)
	{
		svc_px_decrypt(image.device().machine());
		neo_pcm2_swap(image.device().machine(), 3);
		state->m_fixed_layer_bank_type = 2;
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x57);
		install_pvc_protection(image.device().machine());
		logerror("Decrypted SvC code, sound and graphics.\n");
	}
	if(strcmp(crypt_feature,"samsho5_crypt") == 0)
	{
		samsho5_decrypt_68k(image.device().machine());
		neo_pcm2_swap(image.device().machine(), 4);
		state->m_fixed_layer_bank_type = 1;
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x0f);
		logerror("Decrypted Samurai Shodown V code, sound and graphics.\n");
	}
	if(strcmp(crypt_feature,"kof2000_crypt") == 0)
	{
		kof2000_decrypt_68k(image.device().machine());
		state->m_fixed_layer_bank_type = 2;
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x00);
		kof2000_install_protection(image.device().machine());
		logerror("Decrypted KOF2000 code, sound and graphics.\n");
	}
	if(strcmp(crypt_feature,"kof2001_crypt") == 0)
	{
		state->m_fixed_layer_bank_type = 1;
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x1e);
		neogeo_cmc50_m1_decrypt(image.device().machine());
		logerror("Decrypted KOF2001 code and graphics.\n");
	}
	if(strcmp(crypt_feature,"kof2002_crypt") == 0)
	{
		kof2002_decrypt_68k(image.device().machine());
		neo_pcm2_swap(image.device().machine(), 0);
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0xec);
		logerror("Decrypted KOF2002 code, sound and graphics.\n");
	}
	if(strcmp(crypt_feature,"mslug4_crypt") == 0)
	{
		state->m_fixed_layer_bank_type = 1; /* USA violent content screen is wrong -- not a bug, confirmed on real hardware! */
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x31);
		neo_pcm2_snk_1999(image.device().machine(), 8);
		logerror("Decrypted Metal Slug 4 code, sound and graphics.\n");
	}
	if(strcmp(crypt_feature,"mslug5_crypt") == 0)
	{
		mslug5_decrypt_68k(image.device().machine());
		neo_pcm2_swap(image.device().machine(), 2);
		state->m_fixed_layer_bank_type = 1;
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x19);
		install_pvc_protection(image.device().machine());
		logerror("Decrypted Metal Slug 5 code and graphics, and installed protection routines.\n");
	}
	if(strcmp(crypt_feature,"kof2003_crypt") == 0)
	{
		kof2003h_decrypt_68k(image.device().machine());
		neo_pcm2_swap(image.device().machine(), 5);
		state->m_fixed_layer_bank_type = 2;
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x9d);
		install_pvc_protection(image.device().machine());
		logerror("Decrypted KOF2003 code and graphicss, and installed protection routines.\n");
	}
	if(strcmp(crypt_feature,"samsho5s_crypt") == 0)
	{
		samsh5sp_decrypt_68k(image.device().machine());
		neo_pcm2_swap(image.device().machine(), 6);
		state->m_fixed_layer_bank_type = 1;
		neogeo_cmc50_m1_decrypt(image.device().machine());
		kof2000_neogeo_gfx_decrypt(image.device().machine(), 0x0d);
	}
}

/*
    Load the cartridge image files. Apart from reading, we set pointers
    to the image files so that during runtime we do not need search
    operations.
*/
static DEVICE_IMAGE_LOAD( aes_cartridge )
{
	device_t *pcbdev = cartslot_get_pcb(image);
	aes_pcb_t *pcb;
	cartslot_t *cart;
	multicart_open_error me;
	UINT32 size;
	device_t* ym = image.device().machine().device("ymsnd");

	// first check software list
	if(image.software_entry() != NULL)
	{
		// create memory regions
		size = image.get_software_region_length("maincpu");
		image.device().machine().region_free("maincpu");
		image.device().machine().region_alloc("maincpu",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().region("maincpu")->base(),image.get_software_region("maincpu"),size);
		size = image.get_software_region_length("fixed");
		image.device().machine().region_free("fixed");
		image.device().machine().region_alloc("fixed",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().region("fixed")->base(),image.get_software_region("fixed"),size);
		size = image.get_software_region_length("audiocpu");
		image.device().machine().region_free("audiocpu");
		image.device().machine().region_alloc("audiocpu",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().region("audiocpu")->base(),image.get_software_region("audiocpu"),size);
		size = image.get_software_region_length("ymsnd");
		image.device().machine().region_free("ymsnd");
		image.device().machine().region_alloc("ymsnd",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().region("ymsnd")->base(),image.get_software_region("ymsnd"),size);
		if(image.get_software_region("ymsnd.deltat") != NULL)
		{
			size = image.get_software_region_length("ymsnd.deltat");
			image.device().machine().region_free("ymsnd.deltat");
			image.device().machine().region_alloc("ymsnd.deltat",size,1, ENDIANNESS_LITTLE);
			memcpy(image.device().machine().region("ymsnd.deltat")->base(),image.get_software_region("ymsnd.deltat"),size);
		}
		else
			image.device().machine().region_free("ymsnd.deltat");  // removing the region will fix sound glitches in non-Delta-T games
		ym->reset();
		size = image.get_software_region_length("sprites");
		image.device().machine().region_free("sprites");
		image.device().machine().region_alloc("sprites",size,1, ENDIANNESS_LITTLE);
		memcpy(image.device().machine().region("sprites")->base(),image.get_software_region("sprites"),size);
		if(image.get_software_region("audiocrypt") != NULL)  // encrypted Z80 code
		{
			size = image.get_software_region_length("audiocrypt");
			image.device().machine().region_alloc("audiocrypt",size,1, ENDIANNESS_LITTLE);
			memcpy(image.device().machine().region("audiocrypt")->base(),image.get_software_region("audiocrypt"),size);
		}

		// setup cartridge ROM area
		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x000080,0x0fffff,"cart_rom");
		memory_set_bankptr(image.device().machine(),"cart_rom",&image.device().machine().region("maincpu")->base()[0x80]);

		// handle possible protection
		install_protection(image);

		return IMAGE_INIT_PASS;
	}

	if (pcbdev == NULL)
		fatalerror("Error loading multicart: no pcb found.");

	/* If we are here, we have a multicart. */
	pcb = get_safe_pcb_token(pcbdev);
	cart = get_safe_cartslot_token(&image.device());

	/* try opening this as a multicart */
	/* This line requires that cartslot_t be included in cartslot.h,
    otherwise one cannot make use of multicart handling within such a
    custom LOAD function. */
	me = multicart_open(image.device().machine().options(), image.filename(), image.device().machine().system().name, MULTICART_FLAGS_LOAD_RESOURCES, &cart->mc);

	/* Now that we have loaded the image files, let the PCB put them all
    together. This means we put the images in a structure which allows
    for a quick access by the memory handlers. Every PCB defines an
    own assembly method. */
    if (me != MCERR_NONE)
		fatalerror("Error loading multicart: %s", multicart_error_text(me));

	return pcb->assemble(pcbdev->machine(), image);
}

/*
    This is called when the cartridge is unplugged (or the emulator is
    stopped).
*/
static DEVICE_IMAGE_UNLOAD( aes_cartridge )
{
	device_t *pcbdev;

	if (downcast<legacy_device_base *>(&image.device())->token() == NULL)
	{
		/* This means something went wrong during the pcb
           identification (e.g. one of the cartridge files was not
           found). We do not need to (and cannot) unload
           the cartridge. */
		return;
	}
	pcbdev = cartslot_get_pcb(image);

	if (pcbdev != NULL)
	{
		aes_pcb_t *pcb = get_safe_pcb_token(pcbdev);
		cartslot_t *cart = get_safe_cartslot_token(&image.device());

		//  printf("unload\n");
		if (cart->mc != NULL)
		{
			/* Remove pointers and de-big-endianize RAM contents. */
			pcb->disassemble(pcbdev->machine(), image);

			/* Close the multicart; all RAM resources will be
               written to disk */
			multicart_close(pcbdev->machine().options(), cart->mc);
			cart->mc = NULL;
		}
//      else
//          fatalerror("Lost pointer to multicart in cartridge. Report bug.");
	}
}

/*****************************************************************************
  The overall multi-cartridge slot system. It contains instances of
  cartridges which contain PCB devices. The memory handlers delegate the calls
  to the respective handlers of the cartridges.

  Note that the term "multi-cartridge system" and "multicart" are not the same:
  A "multicart" may contain multiple resources, organized on a PCB. The multi-
  cart system may thus host multiple multicarts.

  Actually, the name of the device should be changed (however, the device name
  length is limited)
******************************************************************************/
/*
    Instantiation of a multicart system for the Neo Geo AES.
*/
static DEVICE_START(aes_multicart)
{
	int i;
//  printf("DEVICE_START(aes_multicart)\n");
	aes_multicart_t *cartslots = get_safe_token(device);

	/* Save this in the shortcut; we don't want to look for it each time
       that we have a memory access. And currently we do not plan for
       multiple multicart instances. */
	cartslots->active_slot = 0;
	cartslots->next_free_slot = 0;

	for (i=0; i < AES_NUMBER_OF_CARTRIDGE_SLOTS; i++)
	{
		cartslots->cartridge[i].pcb = NULL;
	}

	cartslots->multi_slots = 0;

	/* The cartslot system is initialized now. The cartridges themselves
       need to check whether their parts are available. */
}

static DEVICE_STOP(aes_multicart)
{
//  printf("DEVICE_STOP(aes_multicart)\n");
}

static MACHINE_CONFIG_FRAGMENT(aes_multicart)
	MCFG_CARTSLOT_ADD("cartridge1")
	MCFG_CARTSLOT_EXTENSION_LIST("rpk,bin")
	MCFG_CARTSLOT_PCBTYPE(0, "none", AES_CARTRIDGE_PCB_NONE)
	MCFG_CARTSLOT_PCBTYPE(1, "standard", AES_CARTRIDGE_PCB_STD)

	MCFG_CARTSLOT_START(aes_cartridge)
	MCFG_CARTSLOT_LOAD(aes_cartridge)
	MCFG_CARTSLOT_UNLOAD(aes_cartridge)
	MCFG_CARTSLOT_INTERFACE("aes_cart")
	MCFG_CARTSLOT_MANDATORY
MACHINE_CONFIG_END


DEVICE_GET_INFO(aes_multicart)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data --- */
		case DEVINFO_PTR_MACHINE_CONFIG:
			info->machine_config = MACHINE_CONFIG_NAME(aes_multicart); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:
			strcpy(info->s, "AES Cartridge handler");
			break;

		case DEVINFO_INT_TOKEN_BYTES: /* private storage, automatically allocated */
			info->i = sizeof(aes_multicart_t);
			break;

		case DEVINFO_STR_FAMILY:
			strcpy(info->s, "Cartridge slot");
			break;
		case DEVINFO_STR_VERSION:
			strcpy(info->s, "1.0");
			break;
		case DEVINFO_STR_SOURCE_FILE:
			strcpy(info->s, __FILE__);
			break;
		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_START:
			info->start = DEVICE_START_NAME(aes_multicart);
			break;
		case DEVINFO_FCT_STOP:
			info->stop = DEVICE_STOP_NAME(aes_multicart);
			break;
		case DEVINFO_FCT_RESET:
			/* Nothing */
			break;
	}
}

DEFINE_LEGACY_CART_SLOT_DEVICE(AES_MULTICART, aes_multicart);
DEFINE_LEGACY_CART_SLOT_DEVICE(AES_CARTRIDGE_PCB_NONE, aes_cartridge_pcb_none);
DEFINE_LEGACY_CART_SLOT_DEVICE(AES_CARTRIDGE_PCB_STD, aes_cartridge_pcb_std);
