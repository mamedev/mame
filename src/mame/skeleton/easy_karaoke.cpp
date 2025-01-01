// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    IVL Technologies Karaoke systems

    KaraokeTV Star (c) IVL Technologies

    licensed out as

    KaraokeStation (Japan) (c) Bandai
    Easy Karaoke Groove Station (UK) (c) Easy Karaoke
    KaraokeMicro Star (France) (c) Lexibook

    -------------------------------------

    Easy Karaoke uses

    Clarity 4.3 ARM
    SVI1186
    NV0165  0317
    Sound Vision Inc.

    an overview for 4.1 and 4.2 can be found at
    http://web.archive.org/web/20031212120255fw_/http://www.soundvisioninc.com/OEMProducts/C4datasheet072401.pdf
    Amusingly this datasheet advertises 'MAME Game emulation' as one of the capabilities despite the chip
    clocking in at only 72Mhz

    Support chip is

    IVL
    Technologies
    ICS0253R1.0
    UA1068ABK-RD
    0327 A01491F

    RAM chip is

    IC42S16400-7T

    ROM is

    IVL
    Technologies
    ICS0303-B
    (c)1985-1986
    3415BAI THAI

    --------------

    Cartridges contain:

    1x MX 29LV040TC-90 (Flash ROM)

    1x HC573A

    1x ICSI IC89LV52A-24PQ (80C52 MCU with 8KBytes Flash memory, can be read protected)

    presumably manages a serial protocol to send data to the main unit


    -----------------

    Lexibook's KaraokeMicro Star version uses

    Clarity 4.1 ARM
    SV11180
    NV0093  0246
    Sound Vision Inc.

    ------------------

    KaraokeTV Star also uses the Clarity 4.1 but with "JVR043  0225" numbering

    Packaging also shows 'On-Key Karaoke' logo on box, maybe this is the original US / Canada product name?

    "Karaoke TV Star" appears to be a US product using this technology - advertises 50 built in songs, but
    also a downloadable service.  It has a 2002 date on the box / product.  Another version offers 35 songs

    ------------------

    For units which allow downloadable songs the only difference in the bootloader ROMs is a couple of bytes
    at the end.  This may be used as security, to tie the downloads stored in the flash ROM to the units as
    the format of the downloaded data in the flash ROMs appears to be different in each case, starting with
    different unique bytes after the header information for each unit we've seen. (maybe encryption?)
    The bootloader ROM has a unique ID on a sticker in each case too (could be a bytesum, haven't checked)


*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class ivl_karaoke_state : public driver_device
{
public:
	ivl_karaoke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void ivl_karaoke_base(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint32_t a000004_r();

	void arm_map(address_map &map) ATTR_COLD;
};

class easy_karaoke_cartslot_state : public ivl_karaoke_state
{
public:
	easy_karaoke_cartslot_state(const machine_config &mconfig, device_type type, const char *tag)
		: ivl_karaoke_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void easy_karaoke(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};

uint32_t ivl_karaoke_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ivl_karaoke_state::machine_start()
{

}

void easy_karaoke_cartslot_state::machine_start()
{
	ivl_karaoke_state::machine_start();

	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void ivl_karaoke_state::machine_reset()
{
	m_maincpu->set_state_int(arm7_cpu_device::ARM7_R15, 0x04000000);
}

DEVICE_IMAGE_LOAD_MEMBER(easy_karaoke_cartslot_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( ivl_karaoke )
INPUT_PORTS_END

uint32_t ivl_karaoke_state::a000004_r()
{
	return machine().rand();
}

void ivl_karaoke_state::arm_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram();
	map(0x04000000, 0x047fffff).rom().region("maincpu", 0);
	map(0x0a000004, 0x0a000007).r(FUNC(ivl_karaoke_state::a000004_r));
}


void ivl_karaoke_state::ivl_karaoke_base(machine_config &config)
{
	ARM9(config, m_maincpu, 72000000); // ARM 720 core
	m_maincpu->set_addrmap(AS_PROGRAM, &ivl_karaoke_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(ivl_karaoke_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void easy_karaoke_cartslot_state::easy_karaoke(machine_config &config)
{
	ivl_karaoke_base(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "easy_karaoke_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(easy_karaoke_cartslot_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("easy_karaoke_cart");
}

/*
The 'karatvst' set has the following 50 songs built in, there don't appear to be any downloaded songs in this NAND dump

ABC                                         Jackson 5
All I Have To Give                          Backstreet Boys
Always On My Mind                           Elvis Presley
America The Beautiful                       Standard
Baby Love                                   Supremes
...Baby One More Time                       Britney Spears
Born To Make You Happy                      Britney Spears
Brick House                                 The Commodores
Bye Bye Bye                                 N'Sync
Dancing Queen                               Abba
Don't Let Me Get Me                         pink
Drive (For Daddy Gene)                      Alan Jackson
Fallin'                                     Alicia Keys
Girlfriend                                  N'Sync
Goodbye Earl                                Dixie Chicks
Hit 'Em Up Style                            Blu Cantrell
I Believe I Can Fly                         R. Kelly
I Heard It Through the Grapevine            Marvin Gaye
I Should Be Sleeping                        Emerson Drive
I Wanna Know                                Joe
I Want It That Way                          Backstreet Boys
If You're Gone                              Matchbox 20
It's A Great Day To Be Alive                Travis Tritt
Lady Marmalade                              Christina Aguilera
Love Shack                                  B52's
Me And Bobby McGee                          Kris Kristofferson
My Girl                                     Temptations
My Guy                                      Mary Wells
New York New York                           Frank Sinatra
No More Drama                               Mary J. Blige
One Fine Day                                Chiffons
Oops...I Did It Again                       Britney Spears
Over The Rainbow                            Judy Garland
Overprotected                               Britney Spears
Stand By Your Man                           Tammy Wynette
Star Spangled Banner                        Public Domain
Stop In The Name Of Love                    Supremes
Stronger                                    Britney Spears
Super Freak                                 Rick James
Superman                                    Five For Fighting
That's The Way (I Like It)                  KC And The Sunshine Band
The Greatest Love Of All                    Whitney Houston
The Loco-Motion                             Kylie Minogue
The One                                     Backstreet Boys
There Is No Arizona                         Jamie O'Neal
We Wish You A Merry Xmas                    Traditional
What's Going On                             Marvin Gaye
Wild Thing                                  The Troggs
Wrapped Around                              Brad Paisley
You Can't Hurry Love                        The Supremes

*/
ROM_START( karatvst )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "karaoke37vf010.bin", 0x000000, 0x20000, CRC(9d3020e4) SHA1(bce5d42ecff88b310a43599c9e47cba920c6b6e1) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "karaoketc58v64bft.bin", 0x000000, 0x840000, CRC(8cf42f20) SHA1(fae09ab08035e8c894fe00dcb23746ea78391d7f) )
ROM_END

/*
The 'karatvsta' set has the following 25 songs built in, there don't appear to be any downloaded songs in this NAND dump
These are a subset of the songs in the 50 song unit, and while the box advertises 35 songs, 10 of those are via a download voucher

Always On My Mind                           Elvis Presley (R)
Brick House                                 Commodores, The
Dancing Queen                               Abba
Don't Let Me Get Me                         Pink
Drive (For Daddy Gene)                      Alan Jackson
Fallin'                                     Alicia Keys
Goodbye Earl                                Dixie Chicks
Heard It Through The Grapevine              Marvin Gaye
I Should Be Sleeping                        Emerson Drive
Lady Marmalade                              Christina Aguilera
Love Shack                                  B52's
Me And Bobby Mcgee                          Janis Joplin
My Girl                                     Temptations
My Guy                                      Mary Wells
New York New York                           Frank Sinatra
No More Drama                               Mary J.Blige
Over The Rainbow                            Judy Garland
Stand By Your Man                           Tammy Wynette
That's The Way (I Like It)                  KC And The Sunshine Band
The Greatest Love Of All                    Whitney Houston
The Loco-Motion                             Kylie Minogue
There Is No Arizona                         Jamie O'neal
What's Going On                             Marvin Gaye
Wild Thing                                  Troggs
Wrapped Around                              Brad Paisley
*/

ROM_START( karatvsta )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "tvstarkaraoke_sst37vf010_bfc5.bin", 0x000000, 0x20000, CRC(fbac56e9) SHA1(bae393ce86de108b6ddc603770863ced885280b6) ) // bootloader, only final bytes differ from karatvsta, serial number?

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tvstarkaraoke_tc58v64aft_98e6a59a.bin", 0x000000, 0x840000, CRC(207f2c6b) SHA1(a8853b792ba23bdead913c8d5ad0c75c39a48c99) )
ROM_END

/*

ABC                                         Jackson 5
All I Have To Give                          Backstreet Boys
Always On My Mind                           Elvis Presley
Amazing Grace                               Traditional
America The Beautiful                       Standard
Auld Lang Syne                              Traditional
...Baby One More Time                       Britney Spears
Brick House                                 The Commodores
Bye Bye Bye                                 N'Sync
Dancing Queen                               Abba
 Danny Boy                                   Traditional
Don't Let Me Get Me                         pink
Drive (For Daddy Gene)                      Alan Jackson
Fallin'                                     Alicia Keys
Girlfriend                                  N'Sync
Goodbye Earl                                Dixie Chicks
Hit 'Em Up Style                            Blu Cantrell
I Believe I Can Fly                         R. Kelly
I Heard It Through the Grapevine            Marvin Gaye
I Wanna Know                                Joe
I Want It That Way                          Backstreet Boys
It's A Great Day To Be Alive                Travis Tritt
Jingle Bells                                Traditional
Lady Marmalade                              Christina Aguilera
Love Shack                                  B52's
Me And Bobby McGee                          Kris Kristofferson
My Girl                                     Temptations
My Guy                                      Mary Wells
New York New York                           Frank Sinatra
No More Drama                               Mary J. Blige
O Sole Mio                                  Traditional
One Fine Day                                Chiffons
Oops...I Did It Again                       Britney Spears
Over The Rainbow                            Judy Garland
Overprotected                               Britney Spears
Stand By Your Man                           Tammy Wynette
Star Spangled Banner                        Public Domain
Stop In The Name Of Love                    Supremes
Stronger                                    Britney Spears
Superman                                    Five For Fighting
Swing Low, Sweet Chariot                    Traditional
That's The Way (I Like It)                  KC And The Sunshine Band
The Greatest Love Of All                    Whitney Houston
The One                                     Backstreet Boys
There Is No Arizona                         Jamie O'Neal
We Wish You A Merry Xmas                    Traditional
What's Going On                             Marvin Gaye
When The Saints Go Marching In              Traditional
Wild Thing                                  The Troggs
Wrapped Around                              Brad Paisley

-- downloaded songs?

All The Things You Are                      Frank Sinatra
How Great Thou Art                          Popular
(Let Me Be Your) Teddy Bear                 Elvis Presley
All I Have To Do Is Dream                   Everly Brothers
Ave Maria                                   Christmas
Blue Hawaii                                 Elvis Presley
Blue Velvet                                 Bobby Vinton
Breaking Up Is Hard To Do                   Neil Sedaka
Calendar Girl                               Neil Sedaka
Can't Help Falling In Love                  Elvis Presley
Don't                                       Elvis Presley
Everybody Loves Somebody                    Dean Martin
For Me And My Gal                           Popular
Frosty The Snowman                          Christmas
Georgia On My Mind                          Ray Charles
Give My Regards To Broadway                 Popular
Good Luck Charm                             Elvis Presley
It's Now Or Never                           Elvis Presley
Jingle Bells                                Christmas
Love Me Tender                              Elvis Presley
Moon River                                  Andy Williams
O Holy Night (Cantique De Noel)             Christmas
Put Your Head On My Shoulder                Paul Anka
Rudolph The Red Nosed Reindeer              Christmas
Surfin' U.S.A.                              Beach Boys
That's Life                                 Frank Sinatra
The Beat Goes On                            Sonny & Cher
Twinkle, Twinkle, Little Star               Children
Unchained Melody                            Righteous Brothers
What A Wonderful World                      Louis Armstrong
White Christmas                             Bing Crosby
Yesterday Once More                         Carpenters
zz Free Test Song                           For Testing Download
Crazy                                       Leann Rimes
Nobody Does It Better                       Carly Simon
As Time Goes By                             Frank Sinatra
Smoke Gets In Your Eyes                     Platters
Could I Have This Dance                     Anne Murray
The Christmas Song                          Nat King Cole
I Only Have Eyes For You                    Frank Sinatra
Oh Pretty Woman                             Roy Orbison
When I Fall In Love                         Clive Griffin
Live And Let Die                            Guns N' Roses
You're Sixteen                              Ringo Starr
Puff (The Magic Dragon)                     Peter, Paul & Mary
Crazy                                       Patsy Cline
Hopelessly Devoted to You                   Olivia Newton-John
I Can't Stop Loving You                     Ray Charles
On The Road Again                           Willie Nelson
You Light Up My Life                        Leann Rimes
Happy Birthday Sweet Sixteen                Neil Sedaka
Save The Last Dance For Me                  Drifters
You're Nobody Till Somebody Loves You       Dean Martin
Only You                                    Platters

*/

ROM_START( mks4001 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "memorex_sst37vf010.bin", 0x000000, 0x20000, CRC(e2eb86f1) SHA1(846734c1e193b268e24b8493334639a4311a8053) ) // bootloader, only final bytes differ from karatvsta, serial number?

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and user downloads)
	ROM_LOAD( "memorex_tc58v64bft.bin", 0x000000, 0x840000, CRC(980589e8) SHA1(e3f8d65ba92e7dbdba356a8c565aef9b0e800347) )
ROM_END

/*

the bkarast set contains the following 68 songs, the box indicates there are '3 trial songs' so most of these are probably downloaded
to the unit?

長崎は今日も雨だった                      内山田洋とクール・ファイブ
ＴＳＵＮＡＭＩ                           サザンオールスターズ
北酒場                                   細川たかし
未来へ                                   Kiroro
らいおんハート                            SMAP
ザ☆ピ～ス！                              モーニング娘。
君をのせて                                天空の城ラピュタより
となりのトトロ                           となりのトトロ　井上あずみ
おどるポンポコリン                          Ｂ．Ｂ．クィーンズ
ついて来るかい                              小林旭
あの娘が泣いてる波止場                      三橋美智也
君といつまでも                              加山雄三
いい日旅立ち                              山口百恵
矢切の渡し                                細川たかし
恋の町札幌                               石原裕次郎
またあえる日まで                             ゆ　ず
また逢う日まで                              尾崎紀世彦
中の島ブルース                             内山田洋とクール・ファイブ
燃えてヒーロー                              キャプテン翼　沖田浩之・小粥よう子
アカシアの雨がやむとき                        西田佐知子
霧の摩周湖                               布施明
圭子の夢は夜ひらく                          藤圭子
きよしのズンドコ節                           氷川きよし
人生いろいろ                              島倉千代子
真赤な太陽                               美空ひばり＆ジャッキー吉川とブルー・コメッツ
別れの一本杉                             春日八郎
天城越え                                 石川さゆり
夜霧よ今夜も有難う                         石原裕次郎
命くれない                                 瀬川瑛子
千曲川                                  五木ひろし
川の流れのように                           美空ひばり
北国の春                                千昌夫
瀬戸の花嫁                              小柳ルミ子
愛燦燦＜あいさんさん＞                     美空ひばり
一円玉の旅がらす                          晴山さおり
北の宿から                               都はるみ
孫                                      大泉逸郎
だんな様                                 三船和子
越冬つばめ                               森昌子
津軽海峡・冬景色                         石川さゆり
君は心の妻だから                          鶴岡雅義と東京ロマンチカ
おふくろさん                               森進一
祝い酒                                  坂本冬美
さざんかの宿                              大川栄策
昔の名前で出ています                       小林旭
DAN DAN 心魅かれてく                      FIELD OF VIEW
君こそわが命                              水原弘
くちなしの花                               渡哲也
おもいで酒                                小林幸子
ソーラン節                                北海道民謡
星は何でも知っている                        平尾昌晃
嵐を呼ぶ男                               石原裕次郎
昴－すばる－                             谷村新司
骨まで愛して                             城卓矢
女のみち                                宮史郎とぴんからトリオ
襟裳岬                                 森進一
さそり座の女                             美川憲一
笑って許して                             和田アキ子
長崎から船に乗って                        五木ひろし
星のフラメンコ                             西郷輝彦
今日でお別れ                            菅原洋一
どうにもとまらない                          山本リンダ
いつか逢う日を夢みて                      増位山太志郎・長沢薫
男と女のラブゲーム                         日野美歌／葵司朗
別れても好きな人                         ロス・インディオス＆シルヴィア
もしかして－ＰＡＲＴⅡ                       小林幸子＋美樹克彦
ウナ・セラ・ディ東京                        ザ・ピーナッツ
涙くんさよなら                            坂本九


*/

ROM_START( bkarast )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "sst37vf010.u9", 0x000000, 0x20000, CRC(a7c69fbb) SHA1(28ef698e63e76d9461b71649e4ee9c8f252f82e2) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_with_spare.u8", 0x000000, 0x840000, CRC(7770996a) SHA1(cb6df756f88c1f5ff4bd202e7758586c03aff00e) )
ROM_END

ROM_START( bkarasta )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "1ed041.u9", 0x000000, 0x20000, CRC(b6a9c84b) SHA1(062b44dfbbf1dcbc40aa86ab0836fc19c6bddcd5) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_with_spare.u8", 0x000000, 0x840000, CRC(50ff920d) SHA1(421850df09d7f407291de31102ebcdfc69da1872) )
ROM_END

ROM_START( bkarastb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "1f2aea.u9", 0x000000, 0x20000, CRC(4001da51) SHA1(3b3a7cc08d8a30b6b8a422c108d71cb34b0c0b3a) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_with_spare.u8", 0x000000, 0x840000, CRC(a7e8ae4f) SHA1(69cc81c574a47c9b1e8c6570b6d3f1144ec99168) )
ROM_END

ROM_START( bkarastc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "1f260f.u9", 0x000000, 0x20000, CRC(a8ba923f) SHA1(79295b4508b02d9db2dd38b785693146c5be34f2) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_with_spare.u8", 0x000000, 0x840000, CRC(34cafa82) SHA1(4c3970a996c06398d11ada88fa412e0e66ebf509) )
ROM_END

ROM_START( bkarastd )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "1f2589.u9", 0x000000, 0x20000, CRC(62cec581) SHA1(c7737be096805565db53ab668089296efb4a8984) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_with_spare.u8", 0x000000, 0x840000, CRC(491578a4) SHA1(e20be74daa83fce5dcd5d90822de73f98b7d25d5) )
ROM_END

ROM_START( bkaraste )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "1f4808.u9", 0x000000, 0x20000, CRC(77244cac) SHA1(17b8db2d94a0f83a175d6ad8ce8ffd31ebb8500b) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_with_spare.u8", 0x000000, 0x840000, CRC(6b3985f3) SHA1(8dda8ece164d6cbad99f6f6cfde5a714c7271e1f) )
ROM_END

ROM_START( bkarastf )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "200cc8.u9", 0x000000, 0x20000, CRC(2e24ad8a) SHA1(64e4b7756c1a8818f8c445286f63113f6b5c61c6) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_with_spare.u8", 0x000000, 0x840000, CRC(4d300ac8) SHA1(afec80f68265e1cd81de6024be4b744acf9a2d3c) )
ROM_END


/*
The 'easykara' set has the following 10 songs built in.

One Step Closer                             S Club Juniors
S Club Party                                S Club 7
Automatic High                              S Club Juniors
Don't Stop Movin'                           S Club 7
Get the Party Started                       Pink
Feel                                        Robbie Williams
Complicated                                 Avril Lavigne
One love                                    Blue
If you're not the one                       Daniel Bedingfield
Sound of the Underground                    Girls Aloud

*/
ROM_START( easykara )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ics0303-b.bin", 0x000000, 0x400000, CRC(43d86ae8) SHA1(219dcbf72b92d1b7e00f78f237194ab47dc08f1b) )
ROM_END

/*
The 'karams' set has the following 50 songs built in.

...Baby One More Time                       Britney Spears
Alexandrie Alexandra                        Claude François
All By Myself                               Celine Dion
Allumer le feu                              Johnny Hallyday
Alors regarde                               Patrick Bruel
Always On My Mind                           Elvis Presley (R)
Au soleil                                   Jennifer
Auprès de ma blonde                         Enfant
Believe                                     Cher
Bye Bye Bye                                 'N Sync
Can't Get You Out Of My Head                Kylie Minogue
Cette année là                              Claude François
Don't Let Me Get Me                         Pink
Déshabillez-moi                             Juliette Greco
Elle te rend dingue                         Nuttea
Embrasse-moi idiot                          Forban
Fallin'                                     Alicia Keys
Fame                                        Irene Cara
Femmes je vous aime                         Julien Clerc
Frère Jacques                               Enfant
I Love Rock and Roll                        Joan Jett
I Will Survive                              Gloria Gaynor
I'll Be There                               Mariah Carey
Il était un petit navire                    Enfant
It's Raining Men                            Geri Halliwell
Juste quelqu'un de bien                     Enzo Enzo
La Bohème                                   Charles Aznavour
La Cucaracha                                Standard
La Marseillaise                             Popular
La musique                                  Star Academy 1
Lady Marmalade                              Christina Aguilera
Laissons entrer le soleil                   A la recherche de la Nouvelle Star
Le bon roi Dagobert                         Enfant
Le pénitencier                              Johnny Halliday
London Bridge                               Children
Magnolias forever                           Claude François
My Girl                                     Temptations
New York New York                           Frank Sinatra
Noir c'est noir                             Johnny Hallyday
Oops!...I Did It Again                      Britney Spears
Pour le plaisir                             Herbert Léonard
Qui est l'exemple?                          Rohf
Silent Night                                Christmas
That's The Way (I Like It)                  KC And The Sunshine Band
That's The Way It Is                        Celine Dion
The Loco-Motion                             Kylie Minogue
Toute seule                                 Lorie
Vieille canaille                            Gainsbourg
We Wish You A Merry Christmas               Standard
When The Saints Go Marchin' In              Louis Armstrong

*/

ROM_START( karams )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ics0300-a.u9", 0x000000, 0x800000, CRC(32a7a429) SHA1(ed219bc9201b45f67c5e7dbe3fb3db70823c59f0) )
ROM_END

/*
The 'karamsg' set has the following 42 songs built in.

40 παλικάρια
Alouette                                    Enfant
Always On My Mind                           Elvis Presley (R)
Discogirl                                   Σάκης Ρουβάς
Guantanamera                                Popular
Kumbaya                                     Popular
La Cucaracha                                Standard
Oh, My Darling Clementine                   Children
Silent Night                                Christmas
The Greatest Love Of All                    Whitney Houston
We Wish You A Merry Christmas               Standard
Έλα να με τελειώσεις                        Αντώνης Ρέμος
Ένας αϊτός
Απόψε                                       Αντώνης Ρέμος
Αρχιμηνιά κι αρχιχρονι
Αχ κορίτσι μου                              Γιάννης Πλούταρχος
Αχ κουνελάκι
Βγαίνει η βαρκούλα
Γιάννη μου το μαντήλι σ
Δεν έχω τάσεις αυτοκτον Άννα Βίσση
Εχω τόσα να σου πω                          One
Ιτιά ιτιά
Καλήν εσπέραν άρχοντες
Κόφτην Ελένη την ελιά
Μια βοσκοπούλα αγάπησα
Μια ωραία πεταλούδα
Μου παρήγγειλε τ'αηδόνι
Παιδιά της Σαμαρίνας
Παπάκι πάει στην ποταμι
Παραδοσιακό Ηπείρου
Περνά περνά η μέλισσα
Σήμερα γάμος γίνεται
Σαμιώτισσα
Στου Μανώλη την ταβέρνα
Το φεγγάρι Νατάσσα Θεοδωρίδου
Το φεγγάρι κάνει κύκλο
Τρίγωνα κάλαντα
Χαρωπά τα δυό μου χέρια
Χιόνια στο καμπαναριό
Ω έλατο
Ο Μενούσης
Ομολογώ                                     Βαλάντης

*/
ROM_START( karamsg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ics0302-a.u9", 0x000000, 0x800000, CRC(9aaeb556) SHA1(c1f9142c15a15c73f249fad229a65580b7a3ad27) )
ROM_END


/*
The 'dks7000c' set has the following 25 songs built in.

A Spoonful of Sugar                         Mary Poppins
A Whole New World                           Disney
Bare Necessities                            The Jungle Book
Be Our Guest                                Beauty and the Beast
Bibbidi-Bobbidi-Boo                         Disney
Can You Feel The Love Tonight               The Lion King
Chim Chim Cheree                            Disney
Circle of Life                              The Lion King
Colors of the Wind                          Pocahontas
Cruella De Vil                              101 Dalmations
Hakuna Matata                               The Lion King
I Just Can't Wait To Be King                The Lion King
I Wan'na Be Like You (The Monkey Song)      The Jungle Book
I Won't Say (I'm In Love)                   Hercules
I'll Make A Man Out Of You                  Mulan
If I Didn't Have You                        Monsters Inc.
It's A Small World                          New York World's Fair
Kiss The Girl                               The Little Mermaid
Part Of Your World                          The Little Mermaid
Supercalifragilisticexpialidocious          Disney
Yo Ho! (A Pirate's Life For Me)             Pirates of the Caribbean
You Can Fly! You Can Fly! You Can Fly!      Peter Pan
You'll Be In My Heart                       Tarzan
You've Got A Friend In Me                   Toy Story
Zip-A-Dee-Do-Dah                            Song of the South

*/
ROM_START( dks7000c )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dks7000c.bin", 0x000000, 0x400000, CRC(1c03e59e) SHA1(617d13a9b353fb648f10e136bc07c496a424b953) )
ROM_END

/*
The 'dks7000p' set has the following 20 songs built in.

A Dream Is a Wish Your Heart Makes          Cinderella
A Whole New World                           Disney
Be Our Guest                                Beauty and the Beast
Bibbidi-Bobbidi-Boo                         Disney
Chim Chim Cheree                            Disney
Circle of Life                              The Lion King
Colors of the Wind                          Pocahontas
Forget About Love                           The Return of Jafar
Gaston                                      Beauty and the Beast
I Just Can't Wait To Be King                The Lion King
I Won't Say (I'm In Love)                   Hercules
I'll Make A Man Out Of You                  Mulan
Just Around The River Bend                  Pocahontas
Kiss The Girl                               The Little Mermaid
Once Upon A Dream                           Sleeping Beauty
Out There                                   The Huntchback of Notre Dame
Poor Unfortunate Souls                      The Little Mermaid
Reflection                                  Mulan
Supercalifragilisticexpialidocious          Disney
Under The Sea                               Disney

*/
ROM_START( dks7000p )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dks7000p.bin", 0x000000, 0x400000, CRC(8ec210fb) SHA1(56337e99f3be57af02e5f7f6e340c3f22deb566d) )
ROM_END


} // anonymous namespace

// This is the original US release, there's no cartridge slot, but it has a NAND Flash inside, and in addition to 50 built-in songs, advertises
// use of a (now defunct) www.onkeysongs.com service for downloading additional songs to the microphone via bundled PC software.
CONS( 2002, karatvst,      0,              0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies", "KaraokeTV Star (US, with 50 songs)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2002, karatvsta,     karatvst,       0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies", "KaraokeTV Star (US, with 25 songs, 'FREE 35 Hit Songs / $35 value' packaging)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 25 songs on unit, download code for 10 songs

// The "Memorex Star Singer Karaoke / MKS4001" is also made by IVL and boasts 50 built in songs, the casing is different too.
CONS( 2002, mks4001,       0,              0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Memorex license)", "Star Singer Karaoke (MKS4001)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 50 songs built in, appears to have around 54 downloads, including a test download

// Bandai's Japanese release also lacks a cartridge slot, relying on downloads for additional songs. It also comes with a CD containing the PC-side software.  The external microphone design differs slightly.
CONS( 2002, bkarast,       0,              0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station (Japan, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2002, bkarasta,      bkarast,        0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station (Japan, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2002, bkarastb,      bkarast,        0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station (Japan, set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2002, bkarastc,      bkarast,        0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station (Japan, set 4)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2002, bkarastd,      bkarast,        0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station (Japan, set 5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2002, bkaraste,      bkarast,        0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station (Japan, set 6)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
// this set has no downloaded data (or session data) suggesting it hasn't been used, the headers on the 3 songs that are included are different from the above sets however, so these themed units are likely different from factory
CONS( 2002, bkarastf,      bkarast,        0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station 'For Girls' (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// The European releases take cartridges rather than relying on a download service
CONS( 2004, easykara,      0,              0,      easy_karaoke, ivl_karaoke, easy_karaoke_cartslot_state, empty_init, "IVL Technologies (Easy Karaoke license)", "Easy Karaoke Groove Station (UK)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2003, karams,        0,              0,      easy_karaoke, ivl_karaoke, easy_karaoke_cartslot_state, empty_init, "IVL Technologies (Lexibook license)",             "KaraokeMicro Star (France)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2003, karamsg,       0,              0,      easy_karaoke, ivl_karaoke, easy_karaoke_cartslot_state, empty_init, "IVL Technologies (Lexibook / Imago license)",     "Karaoke Microphone Pro / KaraokeMicro Star (Greece)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // KaraokeMicro Star branding is used on-screen, Karaoke Microphone Pro on the box

CONS( 2003, dks7000c,      0,              0,      easy_karaoke, ivl_karaoke, easy_karaoke_cartslot_state, empty_init, "IVL Technologies (Disney / Memcorp Inc license)", "Disney Classic Handheld Karaoke Player (DKS7000-C)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2003, dks7000p,      0,              0,      easy_karaoke, ivl_karaoke, easy_karaoke_cartslot_state, empty_init, "IVL Technologies (Disney / Memcorp Inc license)", "Disney Princess Handheld Karaoke Player (DKS7000-P)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
