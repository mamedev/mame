// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Duel Station's 32 by 32 optical card imager. The parallel setup bus,
    frame/pixel strobes and held analogue pixel feed the console's ADC; card
    calibration, accumulation and recognition remain in the original ROM.

    Still images represent an upright, tightly cropped card front. With no
    card, the sensor sees four black registration marks on the white cover
    described in US7963851B2. Boot calibration requires an empty slot.
    GPIO1.5 enables the illumination during exposure; pixels remain held when
    the console turns the LEDs off for readout.
    Scan timing uses representative intervals of the console reference clock,
    not measured sensor timing.

    Card images: https://dm.takaratomy.co.jp/card (use the complete upright
    front, not an artwork crop).

    Scanner Input defaults to the image sensor. The optional named-card
    settings are a hack that synthesizes pixels from the ROM's recognition
    signatures, allowing testing and use without scans of the full card
    catalogue. Loaded card pictures use the image-sensor path instead.
    The calibration setting supplies synthetic cover marks; cards marked
    Ver.1 require the expansion cartridge.

    Suspected BTANB, not confirmed on hardware: the BIOS matcher at 00:f135
    triggers multiplication at 7ff4 before selecting signed mode at 7ff2.
    Another ROM task can leave unsigned mode selected, inflating a negative
    difference's square and causing intermittent misreads (seen with DM01-048;
    rescanning the same image can succeed). The ver1 matcher selects signed
    mode before triggering multiplication.

    TODO:
    - Identify the fitted sensor and verify its clock, exposure and gain.
    - Verify the inferred card field, spectral response and cover-mark dimensions.

***************************************************************************/

#include "emu.h"
#include "xavix_io_duelmast_scanner.h"

#include "imagedev/picture.h"

#include <algorithm>
#include <cmath>

DEFINE_DEVICE_TYPE(XAVIX_DUELMAST_SCANNER, xavix_duelmast_scanner_device, "xavix_duelmast_scanner", "XaviX Duel Station Card Imager")

static INPUT_PORTS_START( xavix_duelmast_scanner )
	PORT_START("INPUT")
	PORT_CONFNAME( 0x01ff, 0x0000, "Scanner Input" )
	PORT_CONFSETTING( 0x0000, "Image Sensor" )
	PORT_CONFSETTING( 0x0001, "Calibration Pattern" )
	PORT_CONFSETTING( 0x00fc, "Aeris, Flight Elemental / \u98db\u7fd4\u306e\u7cbe\u970a\u30a2\u30ea\u30a8\u30b9 (Ver.1)" ) // DM04-006 飛翔の精霊アリエス
	PORT_CONFSETTING( 0x00f7, "Alcadeias, Lord of Spirits / \u8056\u970a\u738b\u30a2\u30eb\u30ab\u30c7\u30a3\u30a2\u30b9 (Ver.1)" ) // DM04-001 聖霊王アルカディアス
	PORT_CONFSETTING( 0x00c1, "Alek, Solidity Enforcer / \u5805\u7262\u306e\u4f1d\u9053\u5e2b\u30a2\u30ec\u30fc\u30af (Ver.1)" ) // DM03-007 堅牢の伝道師アレーク
	PORT_CONFSETTING( 0x00de, "Aless, the Oracle / \u4e88\u8a00\u8005\u30a2\u30ec\u30b9 (Ver.1)" ) // DM03-036 予言者アレス
	PORT_CONFSETTING( 0x011a, "Amber Grass / \u7425\u73c0\u8349 (Ver.1)" ) // DM04-036 琥珀草
	PORT_CONFSETTING( 0x007e, "Amber Piercer / \u30a2\u30f3\u30d0\u30fc\u30fb\u30d4\u30a2\u30b9 (Ver.1)" ) // DM02-014 アンバー・ピアス
	PORT_CONFSETTING( 0x0167, "Ambush Scorpion / \u30a2\u30f3\u30d6\u30c3\u30b7\u30e5\u30fb\u30b9\u30b3\u30fc\u30d4\u30aa\u30f3 (Ver.1)" ) // DM05-053 アンブッシュ・スコーピオン
	PORT_CONFSETTING( 0x016d, "Amnis, Holy Elemental / \u8056\u6e05\u306e\u7cbe\u970a\u30a2\u30e0\u30cb\u30b9 (Ver.1)" ) // PROMOY1-015 聖清の精霊アムニス
	PORT_CONFSETTING( 0x0108, "Ancient Giant / \u3044\u306b\u3057\u3048\u306e\u8d85\u4eba (Ver.1)" ) // DM04-018 いにしえの超人
	PORT_CONFSETTING( 0x00e3, "Angler Cluster / \u30a2\u30f3\u30b0\u30e9\u30fc\u30fb\u30af\u30e9\u30b9\u30bf\u30fc (Ver.1)" ) // DM03-041 アングラー・クラスター
	PORT_CONFSETTING( 0x00ad, "Aqua Bouncer / \u30a2\u30af\u30a2\u30fb\u30d0\u30a6\u30f3\u30b6\u30fc (Ver.1)" ) // DM02-010 アクア・バウンザー
	PORT_CONFSETTING( 0x00c3, "Aqua Deformer / \u30a2\u30af\u30a2\u30fb\u30c7\u30d5\u30a9\u30fc\u30de\u30fc (Ver.1)" ) // DM03-009 アクア・デフォーマー
	PORT_CONFSETTING( 0x0026, "Aqua Guard / \u30a2\u30af\u30a2\u30fb\u30ac\u30fc\u30c9" ) // DM01-084 アクア・ガード
	PORT_CONFSETTING( 0x003c, "Aqua Hulcus / \u30a2\u30af\u30a2\u30fb\u30cf\u30eb\u30ab\u30b9" ) // DM01-080 アクア・ハルカス
	PORT_CONFSETTING( 0x0120, "Aqua Jolter / \u30a2\u30af\u30a2\u30fb\u30c1\u30e3\u30fc\u30b8\u30e3\u30fc (Ver.1)" ) // DM04-042 アクア・チャージャー
	PORT_CONFSETTING( 0x0025, "Aqua Knight / \u30a2\u30af\u30a2\u30fb\u30ca\u30a4\u30c8" ) // DM01-017 アクア・ナイト
	PORT_CONFSETTING( 0x00b2, "Aqua Shooter / \u30a2\u30af\u30a2\u30fb\u30b7\u30e5\u30fc\u30bf\u30fc (Ver.1)" ) // DM02-042 アクア・シューター
	PORT_CONFSETTING( 0x0065, "Aqua Sniper / \u30a2\u30af\u30a2\u30fb\u30b9\u30ca\u30a4\u30d1\u30fc" ) // DM01-S03 アクア・スナイパー
	PORT_CONFSETTING( 0x000a, "Aqua Soldier / \u30a2\u30af\u30a2\u30fb\u30bd\u30eb\u30b8\u30e3\u30fc" ) // DM01-050 アクア・ソルジャー
	PORT_CONFSETTING( 0x014a, "Aqua Surfer / \u30a2\u30af\u30a2\u30fb\u30b5\u30fc\u30d5\u30a1\u30fc (Ver.1)" ) // DM05-024 アクア・サーファー
	PORT_CONFSETTING( 0x000b, "Aqua Vehicle / \u30a2\u30af\u30a2\u30fb\u30d3\u30fc\u30af\u30eb" ) // DM01-083 アクア・ビークル
	PORT_CONFSETTING( 0x0100, "Aquan / \u30a2\u30af\u30a2\u30f3 (Ver.1)" ) // DM04-010 アクアン
	PORT_CONFSETTING( 0x0086, "Armored Blaster Valdios / \u6a5f\u795e\u88c5\u7532\u30f4\u30a1\u30eb\u30c7\u30a3\u30aa\u30b9 (Ver.1)" ) // DM02-S04 機神装甲ヴァルディオス
	PORT_CONFSETTING( 0x008a, "Armored Cannon Balbaro / \u6a5f\u795e\u88c5\u7532\u30f4\u30a1\u30eb\u30d0\u30ed\u30b9 (Ver.1)" ) // DM02-030 機神装甲ヴァルバロス
	PORT_CONFSETTING( 0x0069, "Armored Walker Urherion / \u8d70\u795e\u5175\u30a6\u30eb\u30d8\u30ea\u30aa\u30f3" ) // DM01-061 走神兵ウルヘリオン
	PORT_CONFSETTING( 0x00d9, "Armored Warrior Quelos / \u7532\u795e\u5175\u30af\u30a8\u30ed\u30b9 (Ver.1)" ) // DM03-031 甲神兵クエロス
	PORT_CONFSETTING( 0x0007, "Artisan Picora / \u8077\u4eba\u30d4\u30b3\u30e9" ) // DM01-099 職人ピコラ
	PORT_CONFSETTING( 0x00f8, "Astral Reef / \u30a2\u30b9\u30c8\u30e9\u30eb\u30fb\u30ea\u30fc\u30d5 (Ver.1)" ) // DM04-002 アストラル・リーフ
	PORT_CONFSETTING( 0x0044, "Astrocomet Dragon / \u30e1\u30c6\u30aa\u30fb\u30c9\u30e9\u30b4\u30f3" ) // DM01-S08 メテオ・ドラゴン
	PORT_CONFSETTING( 0x005b, "Aura Blast / \u30aa\u30fc\u30e9\u30fb\u30d6\u30e9\u30b9\u30bf\u30fc" ) // DM01-040 オーラ・ブラスター
	PORT_CONFSETTING( 0x00ce, "Aurora of Reversal / \u9006\u8ee2\u306e\u30aa\u30fc\u30ed\u30e9 (Ver.1)" ) // DM03-020 逆転のオーロラ
	PORT_CONFSETTING( 0x0132, "Avalanche Giant / \u795e\u62f3\u306e\u8d85\u4eba (Ver.1)" ) // DM05-S05 神拳の超人
	PORT_CONFSETTING( 0x00eb, "Baby Zoppe / \u30c1\u30c3\u30c1\u30fb\u30db\u30c3\u30d4\u30fc (Ver.1)" ) // DM03-049 チッチ・ホッピー
	PORT_CONFSETTING( 0x00f4, "Ballom, Master of Death / \u60aa\u9b54\u795e\u30d0\u30ed\u30e0 (Ver.1)" ) // DM04-S03 悪魔神バロム
	PORT_CONFSETTING( 0x0154, "Balloonshroom Q / \u30d5\u30a6\u30bb\u30f3\u30a2\u30ef\u30c0\u30b1β (Ver.1)" ) // DM05-034 フウセンアワダケβ
	PORT_CONFSETTING( 0x0148, "Ballus, Dogfight Enforcer Q / \u6226\u7a7a\u306e\u4f1d\u9053\u58eb\u30d0\u30eb\u30b9β (Ver.1)" ) // DM05-022 戦空の伝道士バルスβ
	PORT_CONFSETTING( 0x00d6, "Baraga, Blade of Gloom / \u90aa\u5263\u30d0\u30e9\u30ac (Ver.1)" ) // DM03-028 邪剣バラガ
	PORT_CONFSETTING( 0x009d, "Barkwhip, the Smasher / \u5927\u52c7\u8005\u300c\u5927\u5730\u306e\u731b\u653b\u300d (Ver.1)" ) // DM02-033 大勇者「大地の猛攻」
	PORT_CONFSETTING( 0x0131, "Billion-Degree Dragon / \u30b0\u30e9\u30c7\u30a3\u30a2\u30f3\u30fb\u30ec\u30c3\u30c9\u30fb\u30c9\u30e9\u30b4\u30f3 (Ver.1)" ) // DM05-S04 グラディアン・レッド・ドラゴン
	PORT_CONFSETTING( 0x0019, "Black Feather, Shadow of Rage / \u6012\u308a\u306e\u5f71\u30d6\u30e9\u30c3\u30af\u30fb\u30d5\u30a7\u30b6\u30fc" ) // DM01-092 怒りの影ブラック・フェザー
	PORT_CONFSETTING( 0x0136, "Bladerush Skyterror Q / \u30d6\u30ec\u30a4\u30c9\u30e9\u30c3\u30b7\u30e5\u30fb\u30ef\u30a4\u30d0\u30fc\u30f3δ (Ver.1)" ) // DM05-004 ブレイドラッシュ・ワイバーンδ
	PORT_CONFSETTING( 0x0129, "Blasto, Explosive Soldier / \u7206\u88c2\u5175\u30c0\u30fc\u30af\u30fb\u30d6\u30e9\u30b9\u30bf\u30fc (Ver.1)" ) // DM04-051 爆裂兵ダーク・ブラスター
	PORT_CONFSETTING( 0x00cb, "Blaze Cannon / \u30d6\u30ec\u30a4\u30ba\u30fb\u30ad\u30e3\u30ce\u30f3 (Ver.1)" ) // DM03-017 ブレイズ・キャノン
	PORT_CONFSETTING( 0x0165, "Blazosaur Q / \u30d6\u30ec\u30a4\u30ba\u30b6\u30a6\u30eb\u30b9α (Ver.1)" ) // DM05-051 ブレイズザウルスα
	PORT_CONFSETTING( 0x0145, "Bloodwing Mantis / \u30d6\u30e9\u30c3\u30c9\u30a6\u30a4\u30f3\u30b0\u30fb\u30de\u30f3\u30c6\u30a3\u30b9 (Ver.1)" ) // DM05-019 ブラッドウイング・マンティス
	PORT_CONFSETTING( 0x001d, "Bloody Squito / \u30d6\u30e9\u30c3\u30c7\u30a3\u30fb\u30a4\u30e4\u30ea\u30f3\u30b0" ) // DM01-089 ブラッディ・イヤリング
	PORT_CONFSETTING( 0x0150, "Bolgash Dragon / \u30dc\u30eb\u30ac\u30c3\u30b7\u30e5\u30fb\u30c9\u30e9\u30b4\u30f3 (Ver.1)" ) // DM05-030 ボルガッシュ・ドラゴン
	PORT_CONFSETTING( 0x0005, "Bolshack Dragon / \u30dc\u30eb\u30b7\u30e3\u30c3\u30af\u30fb\u30c9\u30e9\u30b4\u30f3" ) // DM01-008 ボルシャック・ドラゴン
	PORT_CONFSETTING( 0x00c9, "Boltail Dragon / \u30dc\u30eb\u30c6\u30fc\u30eb\u30fb\u30c9\u30e9\u30b4\u30f3 (Ver.1)" ) // DM03-015 ボルテール・ドラゴン
	PORT_CONFSETTING( 0x0087, "Bolzard Dragon / \u30dc\u30eb\u30b6\u30fc\u30c9\u30fb\u30c9\u30e9\u30b4\u30f3 (Ver.1)" ) // DM02-004 ボルザード・ドラゴン
	PORT_CONFSETTING( 0x0151, "Bombat, General of Speed / \u731b\u7206\u8ecd\u66f9\u30dc\u30f3\u30d0\u30c3\u30c8 (Ver.1)" ) // DM05-031 猛爆軍曹ボンバット
	PORT_CONFSETTING( 0x00b5, "Bombersaur / \u30dc\u30de\u30fc\u30b6\u30a6\u30eb\u30b9 (Ver.1)" ) // DM02-016 ボマーザウルス
	PORT_CONFSETTING( 0x0068, "Bone Assassin, the Ripper / \u5207\u308a\u88c2\u304f\u8005\u30dc\u30fc\u30f3\u30fb\u30a2\u30b5\u30b7\u30f3" ) // DM01-088 切り裂く者ボーン・アサシン
	PORT_CONFSETTING( 0x00e8, "Bone Piercer / \u30dc\u30fc\u30f3\u30fb\u30d4\u30a2\u30fc\u30b9 (Ver.1)" ) // DM03-046 ボーン・ピアース
	PORT_CONFSETTING( 0x003e, "Bone Spider / \u6355\u3089\u3048\u308b\u8005\u30dc\u30fc\u30f3\u30fb\u30b9\u30d1\u30a4\u30c0\u30fc" ) // DM01-056 捕らえる者ボーン・スパイダー
	PORT_CONFSETTING( 0x00d1, "Boomerang Comet / \u30e9\u30a4\u30c8\u30fb\u30d6\u30fc\u30e1\u30e9\u30f3 (Ver.1)" ) // DM03-023 ライト・ブーメラン
	PORT_CONFSETTING( 0x0006, "Brawler Zyler / \u55a7\u5629\u5c4b\u30bf\u30a4\u30e9\u30fc" ) // DM01-098 喧嘩屋タイラー
	PORT_CONFSETTING( 0x006a, "Bronze-Arm Tribe / \u9752\u9285\u306e\u93a7" ) // DM01-106 青銅の鎧
	PORT_CONFSETTING( 0x0146, "Brutal Charge / \u30d3\u30fc\u30b9\u30c8\u30fb\u30c1\u30e3\u30fc\u30b8 (Ver.1)" ) // DM05-020 ビースト・チャージ
	PORT_CONFSETTING( 0x001f, "Burning Mane / \u708e\u306e\u305f\u3066\u304c\u307f" ) // DM01-108 炎のたてがみ
	PORT_CONFSETTING( 0x005a, "Burning Power / \u30d0\u30fc\u30cb\u30f3\u30b0\u30fb\u30d1\u30ef\u30fc" ) // DM01-102 バーニング・パワー
	PORT_CONFSETTING( 0x008c, "Burst Shot / \u30d0\u30fc\u30b9\u30c8\u30fb\u30b7\u30e7\u30c3\u30c8 (Ver.1)" ) // DM02-032 バースト・ショット
	PORT_CONFSETTING( 0x0158, "Calgo, Vizier of Rainclouds / \u4e71\u96f2\u306e\u4f7f\u5f92\u30ab\u30eb\u30b4 (Ver.1)" ) // DM05-038 乱雲の使徒カルゴ
	PORT_CONFSETTING( 0x004b, "Candy Drop / \u30ad\u30e3\u30f3\u30c7\u30a3\u30fb\u30c9\u30ed\u30c3\u30d7" ) // DM01-081 キャンディ・ドロップ
	PORT_CONFSETTING( 0x012b, "Cannon Shell / \u30b7\u30a7\u30eb\u30fb\u30ad\u30e3\u30ce\u30f3 (Ver.1)" ) // DM04-053 シェル・キャノン
	PORT_CONFSETTING( 0x0142, "Cannoneer Bargon / \u7832\u795e\u5175\u30d0\u30b4\u30fc\u30f3 (Ver.1)" ) // DM05-016 砲神兵バゴーン
	PORT_CONFSETTING( 0x0143, "Cataclysmic Eruption / \u7adc\u8108\u5674\u706b (Ver.1)" ) // DM05-017 竜脈噴火
	PORT_CONFSETTING( 0x008d, "Cavalry General Curatops / \u9a0e\u5175\u7dcf\u9577\u30ad\u30e5\u30e9\u30c8\u30d7\u30b9 (Ver.1)" ) // DM02-031 騎兵総長キュラトプス
	PORT_CONFSETTING( 0x0104, "Chains of Sacrifice / \u3044\u3051\u306b\u3048\u306e\u9396 (Ver.1)" ) // DM04-014 いけにえの鎖
	PORT_CONFSETTING( 0x00b7, "Chaos Fish / \u6df7\u6c8c\u9b5a (Ver.1)" ) // DM03-S02 混沌魚
	PORT_CONFSETTING( 0x0071, "Chaos Strike / \u30ab\u30aa\u30b9\u30fb\u30b9\u30c8\u30e9\u30a4\u30af" ) // DM01-034 カオス・ストライク
	PORT_CONFSETTING( 0x007a, "Chaos Worm / \u9b54\u7363\u866b\u30ab\u30aa\u30b9\u30fb\u30ef\u30fc\u30e0 (Ver.1)" ) // DM02-027 魔獣虫カオス・ワーム
	PORT_CONFSETTING( 0x0105, "Chaotic Skyterror / \u30ab\u30aa\u30c6\u30a3\u30c3\u30af\u30fb\u30ef\u30a4\u30d0\u30fc\u30f3 (Ver.1)" ) // DM04-015 カオティック・ワイバーン
	PORT_CONFSETTING( 0x0062, "Chilias, the Oracle / \u4e88\u8a00\u8005\u30ad\u30ea\u30a2\u30b9" ) // DM01-014 予言者キリアス
	PORT_CONFSETTING( 0x0122, "Clone Factory / \u30af\u30ed\u30fc\u30f3\u30fb\u30d5\u30a1\u30af\u30c8\u30ea\u30fc (Ver.1)" ) // DM04-044 クローン・ファクトリー
	PORT_CONFSETTING( 0x0057, "Coiling Vines / \u304b\u3089\u307f\u30ab\u30ba\u30e9" ) // DM01-067 からみカズラ
	PORT_CONFSETTING( 0x00b3, "Corile / \u30b3\u30fc\u30e9\u30a4\u30eb (Ver.1)" ) // DM02-040 コーライル
	PORT_CONFSETTING( 0x0074, "Creeping Plague / \u30d6\u30e9\u30c3\u30af\u30fb\u30b9\u30ec\u30a4\u30e4\u30fc" ) // DM01-028 ブラック・スレイヤー
	PORT_CONFSETTING( 0x0070, "Crimson Hammer / \u30af\u30ea\u30e0\u30be\u30f3\u30fb\u30cf\u30f3\u30de\u30fc" ) // DM01-101 クリムゾン・ハンマー
	PORT_CONFSETTING( 0x0081, "Critical Blade / \u30af\u30ea\u30c6\u30a3\u30ab\u30eb\u30fb\u30d6\u30ec\u30fc\u30c9 (Ver.1)" ) // DM02-047 クリティカル・ブレード
	PORT_CONFSETTING( 0x0169, "Crow Winger / \u5de8\u7ffc\u306e\u722a (Ver.1)" ) // DM05-055 巨翼の爪
	PORT_CONFSETTING( 0x00a9, "Crystal Lancer / \u30af\u30ea\u30b9\u30bf\u30eb\u30fb\u30e9\u30f3\u30b5\u30fc (Ver.1)" ) // DM02-024 クリスタル・ランサー
	PORT_CONFSETTING( 0x000d, "Crystal Memory / \u30af\u30ea\u30b9\u30bf\u30eb\u30fb\u30e1\u30e2\u30ea\u30fc" ) // DM01-022 クリスタル・メモリー
	PORT_CONFSETTING( 0x00ae, "Crystal Paladin / \u30af\u30ea\u30b9\u30bf\u30eb\u30fb\u30d1\u30e9\u30c7\u30a3\u30f3 (Ver.1)" ) // DM02-S02 クリスタル・パラディン
	PORT_CONFSETTING( 0x0079, "Cyber Brain / \u30b5\u30a4\u30d0\u30fc\u30fb\u30d6\u30ec\u30a4\u30f3" ) // DM01-052 サイバー・ブレイン
	PORT_CONFSETTING( 0x0152, "Cyclone Panic / \u30b5\u30a4\u30af\u30ed\u30f3\u30fb\u30d1\u30cb\u30c3\u30af (Ver.1)" ) // DM05-032 サイクロン・パニック
	PORT_CONFSETTING( 0x0049, "Dark Clown / \u30c0\u30fc\u30af\u30fb\u30af\u30e9\u30a6\u30f3" ) // DM01-026 ダーク・クラウン
	PORT_CONFSETTING( 0x0067, "Dark Raven, Shadow of Grief / \u5606\u304d\u306e\u5f71\u30c0\u30fc\u30af\u30fb\u30ec\u30a4\u30d6\u30f3" ) // DM01-055 嘆きの影ダーク・レイブン
	PORT_CONFSETTING( 0x0073, "Dark Reversal / \u30c0\u30fc\u30af\u30fb\u30ea\u30d0\u30fc\u30b9" ) // DM01-058 ダーク・リバース
	PORT_CONFSETTING( 0x0084, "Dark Titan Maginn / \u6697\u9ed2\u5de8\u5175\u30de\u30ae\u30f3 (Ver.1)" ) // DM02-003 暗黒巨兵マギン
	PORT_CONFSETTING( 0x0127, "Darkpact / \u60aa\u9b54\u306e\u5951\u7d04 (Ver.1)" ) // DM04-049 悪魔の契約
	PORT_CONFSETTING( 0x00cc, "Dawn Giant / \u591c\u660e\u3051\u306e\u8d85\u4eba (Ver.1)" ) // DM03-018 夜明けの超人
	PORT_CONFSETTING( 0x0003, "Deadly Fighter Braid Claw / \u51f6\u6226\u58eb\u30d6\u30ec\u30a4\u30ba\u30fb\u30af\u30ed\u30fc" ) // DM01-100 凶戦士ブレイズ・クロー
	PORT_CONFSETTING( 0x0130, "Death Cruzer, the Annihilator / \u6bba\u622e\u306e\u7f85\u5239\u30c7\u30b9\u30fb\u30af\u30eb\u30fc\u30b6\u30fc (Ver.1)" ) // DM05-S03 殺戮の羅刹デス・クルーザー
	PORT_CONFSETTING( 0x005c, "Death Smoke / \u30c7\u30b9\u30fb\u30b9\u30e2\u30fc\u30af" ) // DM01-093 デス・スモーク
	PORT_CONFSETTING( 0x0038, "Deathblade Beetle / \u30c7\u30b9\u30d6\u30ec\u30fc\u30c9\u30fb\u30d3\u30fc\u30c8\u30eb" ) // DM01-S10 デスブレード・ビートル
	PORT_CONFSETTING( 0x0047, "Deathliger, Lion of Chaos / \u6df7\u6c8c\u306e\u7345\u5b50\u30c7\u30b9\u30e9\u30a4\u30ac\u30fc" ) // DM01-S05 混沌の獅子デスライガー
	PORT_CONFSETTING( 0x00b0, "Deep Operation / \u30c7\u30a3\u30fc\u30d7\u30fb\u30aa\u30da\u30ec\u30fc\u30b7\u30e7\u30f3 (Ver.1)" ) // DM02-026 ディープ・オペレーション
	PORT_CONFSETTING( 0x0119, "Dew Mushroom / \u30ab\u30b9\u30df\u30c0\u30b1 (Ver.1)" ) // DM04-035 カスミダケ
	PORT_CONFSETTING( 0x0043, "Dia Nork, Moonlight Guardian / \u6708\u5149\u306e\u5b88\u8b77\u8005\u30c7\u30a3\u30a2\u30fb\u30ce\u30fc\u30af" ) // DM01-013 月光の守護者ディア・ノーク
	PORT_CONFSETTING( 0x009c, "Diamond Cutter / \u30c0\u30a4\u30e4\u30e2\u30f3\u30c9\u30fb\u30ab\u30c3\u30bf\u30fc (Ver.1)" ) // DM02-023 ダイヤモンド・カッター
	PORT_CONFSETTING( 0x006c, "Dimension Gate / \u30c7\u30a3\u30e1\u30f3\u30b8\u30e7\u30f3\u30fb\u30b2\u30fc\u30c8" ) // DM01-110 ディメンジョン・ゲート
	PORT_CONFSETTING( 0x016a, "Divine Machine Armor Valborg / \u6a5f\u795e\u88c5\u7532\u30f4\u30a1\u30eb\u30dc\u30fc\u30b0 (Ver.1)" ) // PROMOY1-005 機神装甲ヴァルボーグ
	PORT_CONFSETTING( 0x013d, "Divine Riptide / \u795e\u3005\u306e\u9006\u6d41 (Ver.1)" ) // DM05-011 神々の逆流
	PORT_CONFSETTING( 0x00fa, "Doboulgyser, Giant Rock Beast / \u8d85\u5de8\u5ca9\u7363\u30c9\u30dc\u30eb\u30ac\u30a4\u30b6\u30fc (Ver.1)" ) // DM04-004 超巨岩獣ドボルガイザー
	PORT_CONFSETTING( 0x0088, "Dogarn, the Marauder / \u7a81\u795e\u5175\u30c9\u30ac\u30fc\u30f3 (Ver.1)" ) // DM02-017 突神兵ドガーン
	PORT_CONFSETTING( 0x0039, "Dome Shell / \u30b7\u30a7\u30eb\u30fb\u30c9\u30fc\u30e0" ) // DM01-068 シェル・ドーム
	PORT_CONFSETTING( 0x0004, "Draglide / \u30c9\u30e9\u30b0\u30e9\u30a4\u30c9" ) // DM01-030 ドラグライド
	PORT_CONFSETTING( 0x00ba, "Earthstomp Giant / \u5929\u7a7a\u306e\u8d85\u4eba (Ver.1)" ) // DM03-S05 天空の超人
	PORT_CONFSETTING( 0x00d7, "Eldritch Poison / \u9b54\u6d41\u6bd2 (Ver.1)" ) // DM03-029 魔流毒
	PORT_CONFSETTING( 0x009f, "Elf-X / \u5ff5\u4ecf\u30a8\u30eb\u30d5\u30a3\u30f3 (Ver.1)" ) // DM02-020 念仏エルフィン
	PORT_CONFSETTING( 0x00e4, "Emeral / \u30a8\u30e1\u30e9\u30eb (Ver.1)" ) // DM03-042 エメラル
	PORT_CONFSETTING( 0x0056, "Emerald Grass / \u78a7\u7389\u8349" ) // DM01-074 碧玉草
	PORT_CONFSETTING( 0x0155, "Enchanted Soil / \u518d\u8a95\u306e\u68ee (Ver.1)" ) // DM05-035 再誕の森
	PORT_CONFSETTING( 0x008e, "Engineer Kipo / \u6280\u5e2b\u30d4\u30fc\u30dd (Ver.1)" ) // DM02-050 技師ピーポ
	PORT_CONFSETTING( 0x00a7, "Essence Elf / \u30e9\u30d6\u30fb\u30a8\u30eb\u30d5\u30a3\u30f3 (Ver.1)" ) // DM02-054 ラブ・エルフィン
	PORT_CONFSETTING( 0x0094, "Ethel, Star Sea Elemental / \u661f\u6d77\u306e\u7cbe\u970a\u30a8\u30fc\u30c6\u30eb (Ver.1)" ) // DM02-001 星海の精霊エーテル
	PORT_CONFSETTING( 0x012c, "Exploding Cactus / \u7206\u767a\u30b5\u30dc\u30c6\u30f3 (Ver.1)" ) // DM04-054 爆発サボテン
	PORT_CONFSETTING( 0x00ea, "Explosive Dude Joe / \u7206\u708e\u91ce\u90ce\u30b8\u30e7\u30fc (Ver.1)" ) // DM03-048 爆炎野郎ジョー
	PORT_CONFSETTING( 0x0023, "Explosive Fighter Ucarn / \u7206\u52c7\u58eb\u30e6\u30fc\u30ab\u30fc\u30f3" ) // DM01-029 爆勇士ユーカーン
	PORT_CONFSETTING( 0x0063, "Faerie Child / \u30d5\u30a7\u30a2\u30ea\u30fc\u30fb\u30c1\u30e3\u30a4\u30eb\u30c9" ) // DM01-049 フェアリー・チャイルド
	PORT_CONFSETTING( 0x0051, "Fatal Attacker Horvath / \u4e00\u6483\u5fc5\u6bba\u306e\u30db\u30fc\u30d0\u30b9" ) // DM01-096 一撃必殺のホーバス
	PORT_CONFSETTING( 0x001e, "Fear Fang / \u767d\u9280\u306e\u7259" ) // DM01-103 白銀の牙
	PORT_CONFSETTING( 0x00a2, "Fighter Dual Fang / \u5927\u52c7\u8005\u300c\u3075\u305f\u3064\u7259\u300d (Ver.1)" ) // DM02-S05 大勇者「ふたつ牙」
	PORT_CONFSETTING( 0x003a, "Fire Sweeper Burning Hellion / \u6383\u8a0e\u5175\u30d0\u30fc\u30cb\u30f3\u30b0\u30fb\u30d8\u30eb" ) // DM01-095 掃討兵バーニング・ヘル
	PORT_CONFSETTING( 0x00ca, "Flametropus / \u30d5\u30ec\u30e0\u30c8\u30ed\u30d7\u30b9 (Ver.1)" ) // DM03-016 フレムトロプス
	PORT_CONFSETTING( 0x00d4, "Flood Valve / \u30cf\u30a4\u30d1\u30fc\u30fb\u30a6\u30a7\u30fc\u30d6 (Ver.1)" ) // DM03-026 ハイパー・ウェーブ
	PORT_CONFSETTING( 0x0099, "Fonch, the Oracle / \u9810\u8a00\u8005\u30d5\u30a3\u30f3\u30c1 (Ver.1)" ) // DM02-037 預言者フィンチ
	PORT_CONFSETTING( 0x0031, "Forest Hornet / \u30d5\u30a9\u30ec\u30b9\u30c8\u30fb\u30db\u30fc\u30cd\u30c3\u30c8" ) // DM01-066 フォレスト・ホーネット
	PORT_CONFSETTING( 0x00a0, "Fortress Shell / \u30b7\u30a7\u30eb\u30fb\u30d5\u30a9\u30fc\u30c8\u30ec\u30b9 (Ver.1)" ) // DM02-018 シェル・フォートレス
	PORT_CONFSETTING( 0x0012, "Frei, Vizier of Air / \u5927\u6c17\u306e\u4f7f\u5f92\u30d5\u30ec\u30a4" ) // DM01-043 大気の使徒フレイ
	PORT_CONFSETTING( 0x010c, "Fu Reil, Seeker of Storms / \u66b4\u98a8\u306e\u6c42\u9053\u8005\u30d5\u30fb\u30ec\u30a4\u30eb (Ver.1)" ) // DM04-022 暴風の求道者フ・レイル
	PORT_CONFSETTING( 0x00fe, "Full Defensor / \u30d5\u30eb\u30fb\u30c7\u30a3\u30d5\u30a7\u30f3\u30b5\u30fc (Ver.1)" ) // DM04-008 フル・ディフェンサー
	PORT_CONFSETTING( 0x00f5, "Galklife Dragon / \u30ac\u30eb\u30af\u30e9\u30a4\u30d5\u30fb\u30c9\u30e9\u30b4\u30f3 (Ver.1)" ) // DM04-S04 ガルクライフ・ドラゴン
	PORT_CONFSETTING( 0x0156, "Gallia Zohl, Iron Guardian Q / \u9244\u58c1\u306e\u5b88\u8b77\u8005\u30ac\u30ea\u30a2\u30fb\u30be\u30fc\u30ebα (Ver.1)" ) // DM05-036 鉄壁の守護者ガリア・ゾールα
	PORT_CONFSETTING( 0x008f, "Galsaur / \u30ac\u30eb\u30b6\u30a6\u30eb\u30b9 (Ver.1)" ) // DM02-048 ガルザウルス
	PORT_CONFSETTING( 0x00c6, "Gamil, Knight of Hatred / \u618e\u60aa\u306e\u9a0e\u58eb\u30ac\u30df\u30eb (Ver.1)" ) // DM03-012 憎悪の騎士ガミル
	PORT_CONFSETTING( 0x00b9, "Garkago Dragon / \u30ac\u30eb\u30ab\u30fc\u30b4\u30fb\u30c9\u30e9\u30b4\u30f3 (Ver.1)" ) // DM03-S04 ガルカーゴ・ドラゴン
	PORT_CONFSETTING( 0x0033, "Gatling Skyterror / \u30ac\u30c8\u30ea\u30f3\u30b0\u30fb\u30ef\u30a4\u30d0\u30fc\u30f3" ) // DM01-007 ガトリング・ワイバーン
	PORT_CONFSETTING( 0x007d, "General Dark Fiend / \u9b54\u5c06\u30c0\u30fc\u30af\u30fb\u30d5\u30ea\u30fc\u30c9 (Ver.1)" ) // DM02-012 魔将ダーク・フリード
	PORT_CONFSETTING( 0x00c8, "Ghastly Drain / \u30c7\u30d3\u30eb\u30fb\u30c9\u30ec\u30fc\u30f3 (Ver.1)" ) // DM03-014 デビル・ドレーン
	PORT_CONFSETTING( 0x004d, "Ghost Touch / \u30b4\u30fc\u30b9\u30c8\u30fb\u30bf\u30c3\u30c1" ) // DM01-094 ゴースト・タッチ
	PORT_CONFSETTING( 0x0036, "Gigaberos / \u30ae\u30ac\u30d9\u30ed\u30b9" ) // DM01-024 ギガベロス
	PORT_CONFSETTING( 0x0114, "Gigabolver / \u30ae\u30ac\u30dc\u30eb\u30d0 (Ver.1)" ) // DM04-030 ギガボルバ
	PORT_CONFSETTING( 0x001a, "Gigagiele / \u30ae\u30ac\u30b8\u30fc\u30eb" ) // DM01-025 ギガジール
	PORT_CONFSETTING( 0x014d, "Gigakail / \u30ae\u30ac\u30b2\u30a4\u30eb (Ver.1)" ) // DM05-027 ギガゲイル
	PORT_CONFSETTING( 0x015e, "Gigaling Q / \u30ae\u30ac\u30ea\u30f3\u30b0α (Ver.1)" ) // DM05-044 ギガリングα
	PORT_CONFSETTING( 0x00bf, "Gigamantis / \u5927\u6606\u866b\u30ae\u30ac\u30de\u30f3\u30c6\u30a3\u30b9 (Ver.1)" ) // DM03-005 大昆虫ギガマンティス
	PORT_CONFSETTING( 0x0041, "Gigargon / \u30ae\u30ac\u30eb\u30b4\u30f3" ) // DM01-006 ギガルゴン
	PORT_CONFSETTING( 0x007b, "Gigastand / \u30ae\u30ac\u30b9\u30bf\u30f3\u30c9 (Ver.1)" ) // DM02-044 ギガスタンド
	PORT_CONFSETTING( 0x015f, "Gigazoul / \u30ae\u30ac\u30be\u30a6\u30eb (Ver.1)" ) // DM05-045 ギガゾウル
	PORT_CONFSETTING( 0x016c, "Giliam, the Tormentor / \u90aa\u7344\u30ae\u30ea\u30a2\u30e0 (Ver.1)" ) // PROMOY1-014 邪獄ギリアム
	PORT_CONFSETTING( 0x00b8, "Giriel, Ghastly Warrior / \u51f6\u95d8\u306e\u9b54\u4eba\u30ae\u30ea\u30a8\u30eb (Ver.1)" ) // DM03-S03 凶闘の魔人ギリエル
	PORT_CONFSETTING( 0x013a, "Glory Snow / \u30b0\u30ed\u30fc\u30ea\u30fc\u30fb\u30b9\u30ce\u30fc (Ver.1)" ) // DM05-008 グローリー・スノー
	PORT_CONFSETTING( 0x002b, "Golden Wing Striker / \u9ec4\u91d1\u306e\u7ffc" ) // DM01-104 黄金の翼
	PORT_CONFSETTING( 0x0034, "Gran Gure, Space Guardian / \u5929\u7a7a\u306e\u5b88\u8b77\u8005\u30b0\u30e9\u30f3\u30fb\u30ae\u30e5\u30fc\u30ec" ) // DM01-001 天空の守護者グラン・ギューレ
	PORT_CONFSETTING( 0x0080, "Gray Balloon, Shadow of Greed / \u8caa\u6b32\u306e\u5f71\u30b0\u30ec\u30a4\u30fb\u30d0\u30eb\u30fc\u30f3 (Ver.1)" ) // DM02-028 貪欲の影グレイ・バルーン
	PORT_CONFSETTING( 0x0102, "Gregoria, Princess of War / \u90aa\u5983\u30b0\u30ec\u30b4\u30ea\u30a2 (Ver.1)" ) // DM04-012 邪妃グレゴリア
	PORT_CONFSETTING( 0x0123, "Gregorian Worm / \u66b4\u98df\u866b\u30b0\u30ec\u30b4\u30ea\u30a2\u30fb\u30ef\u30fc\u30e0 (Ver.1)" ) // DM04-045 暴食虫グレゴリア・ワーム
	PORT_CONFSETTING( 0x011c, "Gulan Rias, Speed Guardian / \u795e\u901f\u306e\u5b88\u8b77\u8005\u30b0\u30e9\u30f3\u30fb\u30ea\u30a8\u30b9 (Ver.1)" ) // DM04-038 神速の守護者グラン・リエス
	PORT_CONFSETTING( 0x00d5, "Hang Worm, Fetid Larva / \u8150\u5375\u866b\u30cf\u30f3\u30b0\u30ef\u30fc\u30e0 (Ver.1)" ) // DM03-027 腐卵虫ハングワーム
	PORT_CONFSETTING( 0x0046, "Hanusa, Radiance Elemental / \u5149\u8f2a\u306e\u7cbe\u970a\u30b7\u30e3\u30a6\u30ca" ) // DM01-S01 光輪の精霊シャウナ
	PORT_CONFSETTING( 0x0060, "Holy Awe / \u30db\u30fc\u30ea\u30fc\u30fb\u30b9\u30d1\u30fc\u30af" ) // DM01-015 ホーリー・スパーク
	PORT_CONFSETTING( 0x014e, "Horned Mutant / \u89d2\u7537 (Ver.1)" ) // DM05-028 角男
	PORT_CONFSETTING( 0x0082, "Horrid Worm / \u5375\u80de\u866b\u30bc\u30ea\u30fc\u30fb\u30ef\u30fc\u30e0 (Ver.1)" ) // DM02-045 卵胞虫ゼリー・ワーム
	PORT_CONFSETTING( 0x0110, "Hunter Cluster / \u30cf\u30f3\u30bf\u30fc\u30fb\u30af\u30e9\u30b9\u30bf\u30fc (Ver.1)" ) // DM04-026 ハンター・クラスター
	PORT_CONFSETTING( 0x002e, "Hunter Fish / \u72e9\u731f\u9b5a" ) // DM01-082 狩猟魚
	PORT_CONFSETTING( 0x0101, "Hydro Hurricane / \u30cf\u30a4\u30c9\u30ed\u30fb\u30cf\u30ea\u30b1\u30fc\u30f3 (Ver.1)" ) // DM04-011 ハイドロ・ハリケーン
	PORT_CONFSETTING( 0x00b1, "Hypersquid Walter / \u30a6\u30a9\u30eb\u30bf (Ver.1)" ) // DM02-011 ウォルタ
	PORT_CONFSETTING( 0x0028, "Iere, Vizier of Bullets / \u5f3e\u4e38\u306e\u4f7f\u5f92\u30a4\u30fc\u30ec" ) // DM01-072 弾丸の使徒イーレ
	PORT_CONFSETTING( 0x002c, "Illusionary Merfolk / \u30df\u30e9\u30fc\u30b8\u30e5\u30fb\u30de\u30fc\u30e1\u30a4\u30c9" ) // DM01-047 ミラージュ・マーメイド
	PORT_CONFSETTING( 0x0050, "Immortal Baron, Vorg / \u4e0d\u6b7b\u8eab\u7537\u7235\u30dc\u30fc\u30b0" ) // DM01-097 不死身男爵ボーグ
	PORT_CONFSETTING( 0x0010, "Iocant, the Oracle / \u4e88\u8a00\u8005\u30ab\u30c6\u30a3\u30ce" ) // DM01-045 予言者カティノ
	PORT_CONFSETTING( 0x00bd, "Jack Viper, Shadow of Doom / \u7d76\u671b\u306e\u9b54\u9ed2\u30b8\u30e3\u30c3\u30af\u30d0\u30a4\u30d1\u30fc (Ver.1)" ) // DM03-003 絶望の魔黒ジャックバイパー
	PORT_CONFSETTING( 0x0161, "Jewel Spider / \u30b9\u30d1\u30a4\u30c0\u30fc\u30fb\u30b8\u30e5\u30a8\u30eb (Ver.1)" ) // DM05-047 スパイダー・ジュエル
	PORT_CONFSETTING( 0x012a, "Kamikaze, Chainsaw Warrior / \u53cc\u795e\u5175\u30ab\u30df\u30ab\u30bc (Ver.1)" ) // DM04-052 双神兵カミカゼ
	PORT_CONFSETTING( 0x011f, "Keeper of the Sunlit Abyss / \u30c1\u30a2\u30d5\u30eb\u30fb\u30a2\u30d3\u30b9 (Ver.1)" ) // DM04-041 チアフル・アビス
	PORT_CONFSETTING( 0x00f3, "King Aquakamui / \u30ad\u30f3\u30b0\u30fb\u30a2\u30af\u30a2\u30ab\u30e0\u30a4 (Ver.1)" ) // DM04-S02 キング・アクアカムイ
	PORT_CONFSETTING( 0x0066, "King Coral / \u30ad\u30f3\u30b0\u30fb\u30af\u30e9\u30fc\u30b1\u30f3" ) // DM01-051 キング・クラーケン
	PORT_CONFSETTING( 0x0045, "King Depthcon / \u30ad\u30f3\u30b0\u30fb\u30aa\u30ea\u30aa\u30f3" ) // DM01-S04 キング・オリオン
	PORT_CONFSETTING( 0x0134, "King Mazelan / \u30ad\u30f3\u30b0\u30fb\u30de\u30bc\u30e9\u30f3 (Ver.1)" ) // DM05-002 キング・マゼラン
	PORT_CONFSETTING( 0x00ab, "King Nautilus / \u30ad\u30f3\u30b0\u30fb\u30ce\u30fc\u30c1\u30e9\u30b9 (Ver.1)" ) // DM02-009 キング・ノーチラス
	PORT_CONFSETTING( 0x00c4, "King Neptas / \u30ad\u30f3\u30b0\u30fb\u30cd\u30d7\u30bf\u30b9 (Ver.1)" ) // DM03-010 キング・ネプタス
	PORT_CONFSETTING( 0x00d2, "King Ponitas / \u30ad\u30f3\u30b0\u30fb\u30dc\u30cb\u30fc\u30bf\u30b9 (Ver.1)" ) // DM03-024 キング・ボニータス
	PORT_CONFSETTING( 0x000f, "King Ripped-Hide / \u30ad\u30f3\u30b0\u30fb\u30dd\u30bb\u30a4\u30c9\u30f3" ) // DM01-003 キング・ポセイドン
	PORT_CONFSETTING( 0x012f, "King Tsunami / \u30ad\u30f3\u30b0\u30fb\u30a2\u30c8\u30e9\u30f3\u30c6\u30a3\u30b9 (Ver.1)" ) // DM05-S02 キング・アトランティス
	PORT_CONFSETTING( 0x0164, "Kip Chippotto / \u30dd\u30c3\u30dd\u30fb\u30c1\u30e3\u30c3\u30d4\u30fc (Ver.1)" ) // DM05-050 ポッポ・チャッピー
	PORT_CONFSETTING( 0x011b, "Kolon, the Oracle / \u4e88\u8a00\u8005\u30b3\u30ed\u30f3 (Ver.1)" ) // DM04-037 予言者コロン
	PORT_CONFSETTING( 0x0157, "Kulus, Soulshine Enforcer / \u5149\u9b42\u306e\u4f1d\u9053\u58eb\u30af\u30eb\u30b9 (Ver.1)" ) // DM05-037 光魂の伝道士クルス
	PORT_CONFSETTING( 0x0133, "La Byle, Seeker of the Winds / \u98a8\u6483\u306e\u6c42\u9053\u8005\u30e9\u30fb\u30d0\u30a4\u30eb (Ver.1)" ) // DM05-001 風撃の求道者ラ・バイル
	PORT_CONFSETTING( 0x0147, "La Guile, Seeker of Skyfire / \u96f7\u6483\u306e\u6c42\u9053\u8005\u30e9\u30fb\u30ac\u30a4\u30eb (Ver.1)" ) // DM05-021 雷撃の求道者ラ・ガイル
	PORT_CONFSETTING( 0x0013, "La Ura Giga, Sky Guardian / \u84bc\u5929\u306e\u5b88\u8b77\u8005\u30e9\u30fb\u30a6\u30e9\u30fb\u30ae\u30ac" ) // DM01-076 蒼天の守護者ラ・ウラ・ギガ
	PORT_CONFSETTING( 0x0091, "Ladia Bale, the Inspirational / \u5b88\u8b77\u8056\u5929\u30e9\u30c7\u30a3\u30a2\u30fb\u30d0\u30fc\u30ec (Ver.1)" ) // DM02-S01 守護聖天ラディア・バーレ
	PORT_CONFSETTING( 0x0095, "Laguna, Lightning Enforcer / \u9583\u5149\u306e\u4f1d\u9053\u5e2b\u30e9\u30b0\u30ca (Ver.1)" ) // DM02-007 閃光の伝道師ラグナ
	PORT_CONFSETTING( 0x0014, "Lah, Purification Enforcer / \u7c9b\u6e05\u306e\u4f1d\u9053\u5e2b\u30e9\u30fc" ) // DM01-012 粛清の伝道師ラー
	PORT_CONFSETTING( 0x0096, "Larba Geer, the Immaculate / \u5b88\u8b77\u8056\u5929\u30e9\u30eb\u30d0\u30fb\u30ae\u30a2 (Ver.1)" ) // DM02-021 守護聖天ラルバ・ギア
	PORT_CONFSETTING( 0x0077, "Laser Wing / \u30ec\u30fc\u30b6\u30fc\u30fb\u30a6\u30a4\u30f3\u30b0" ) // DM01-016 レーザー・ウイング
	PORT_CONFSETTING( 0x0159, "Le Quist, the Oracle / \u4e88\u8a00\u8005\u30ec\u30af\u30a4\u30eb (Ver.1)" ) // DM05-039 予言者レクイル
	PORT_CONFSETTING( 0x009e, "Leaping Tornado Horn / \u8df3\u8e8d\u3059\u308b\u30c8\u30eb\u30cd\u30fc\u30c9\u30fb\u30db\u30fc\u30f3 (Ver.1)" ) // DM02-052 跳躍するトルネード・ホーン
	PORT_CONFSETTING( 0x00bc, "Legendary Bynor / \u30ec\u30b8\u30a7\u30f3\u30c0\u30ea\u30fc\u30fb\u30d0\u30a4\u30ed\u30f3 (Ver.1)" ) // DM03-002 レジェンダリー・バイロン
	PORT_CONFSETTING( 0x00e0, "Lena, Vizier of Brilliance / \u9583\u5149\u306e\u4f7f\u5f92\u30ec\u30fc\u30ca (Ver.1)" ) // DM03-038 閃光の使徒レーナ
	PORT_CONFSETTING( 0x00e5, "Liquid Scope / \u30ea\u30ad\u30c3\u30c9\u30fb\u30b9\u30b3\u30fc\u30d7 (Ver.1)" ) // DM03-043 リキッド・スコープ
	PORT_CONFSETTING( 0x0113, "Locomotiver / \u6c7d\u8eca\u7537 (Ver.1)" ) // DM04-029 汽車男
	PORT_CONFSETTING( 0x0098, "Logic Cube / \u30ed\u30b8\u30c3\u30af\u30fb\u30ad\u30e5\u30fc\u30d6 (Ver.1)" ) // DM02-039 ロジック・キューブ
	PORT_CONFSETTING( 0x00c2, "Logic Sphere / \u30d7\u30ed\u30c6\u30af\u30c8\u30fb\u30ad\u30e5\u30fc\u30d6 (Ver.1)" ) // DM03-008 プロテクト・キューブ
	PORT_CONFSETTING( 0x0011, "Lok, Vizier of Hunting / \u8ffd\u8de1\u306e\u4f7f\u5f92\u30ed\u30fc\u30af" ) // DM01-042 追跡の使徒ローク
	PORT_CONFSETTING( 0x0085, "Lost Soul / \u30ed\u30b9\u30c8\u30fb\u30bd\u30a6\u30eb (Ver.1)" ) // DM02-029 ロスト・ソウル
	PORT_CONFSETTING( 0x015a, "Lurking Eel / \u30d0\u30a6\u30f3\u30b5\u30fc\u30fb\u30a4\u30fc\u30eb (Ver.1)" ) // DM05-040 バウンサー・イール
	PORT_CONFSETTING( 0x006f, "Magma Gazer / \u30de\u30b0\u30de\u30fb\u30b2\u30a4\u30b6\u30fc" ) // DM01-033 マグマ・ゲイザー
	PORT_CONFSETTING( 0x0106, "Magmarex / \u30de\u30b0\u30de\u30c6\u30a3\u30e9\u30ce\u30b9 (Ver.1)" ) // DM04-016 マグマティラノス
	PORT_CONFSETTING( 0x0097, "Magris, Vizier of Magnetism / \u78c1\u529b\u306e\u4f7f\u5f92\u30de\u30b0\u30ea\u30b9 (Ver.1)" ) // DM02-036 磁力の使徒マグリス
	PORT_CONFSETTING( 0x00a4, "Mana Crisis / \u30de\u30ca\u30fb\u30af\u30e9\u30a4\u30b7\u30b9 (Ver.1)" ) // DM02-035 マナ・クライシス
	PORT_CONFSETTING( 0x00dd, "Mana Nexus / \u6df1\u7dd1\u306e\u9b54\u65b9\u9663 (Ver.1)" ) // DM03-035 深緑の魔方陣
	PORT_CONFSETTING( 0x0121, "Marine Flower / \u30de\u30ea\u30f3\u30fb\u30d5\u30e9\u30ef\u30fc (Ver.1)" ) // DM04-043 マリン・フラワー
	PORT_CONFSETTING( 0x0083, "Marrow Ooze, the Twister / \u306d\u3058\u308c\u308b\u8005\u30dc\u30fc\u30f3\u30fb\u30b9\u30e9\u30a4\u30e0 (Ver.1)" ) // DM02-046 ねじれる者ボーン・スライム
	PORT_CONFSETTING( 0x003f, "Masked Horror, Shadow of Scorn / \u5632\u308a\u306e\u5f71\u30de\u30b9\u30af\u30c9\u30fb\u30db\u30e9\u30fc" ) // DM01-054 嘲りの影マスクド・ホラー
	PORT_CONFSETTING( 0x00db, "Masked Pomegranate / \u5947\u9762\u3056\u304f\u308d (Ver.1)" ) // DM03-033 奇面ざくろ
	PORT_CONFSETTING( 0x0107, "Mega Detonator / \u30e1\u30ac\u30fb\u30d6\u30e9\u30b9\u30bf\u30fc (Ver.1)" ) // DM04-017 メガ・ブラスター
	PORT_CONFSETTING( 0x0089, "Metalwing Skyterror / \u30e1\u30bf\u30eb\u30a6\u30a3\u30f3\u30b0\u30fb\u30ef\u30a4\u30d0\u30fc\u30f3 (Ver.1)" ) // DM02-015 メタルウィング・ワイバーン
	PORT_CONFSETTING( 0x0054, "Meteosaur / \u30e1\u30c6\u30aa\u30b6\u30a6\u30eb\u30b9" ) // DM01-060 メテオザウルス
	PORT_CONFSETTING( 0x00b6, "Miar, Comet Elemental / \u6d41\u661f\u306e\u7cbe\u970a\u30df\u30fc\u30a2 (Ver.1)" ) // DM03-S01 流星の精霊ミーア
	PORT_CONFSETTING( 0x0027, "Miele, Vizier of Lightning / \u96f7\u5149\u306e\u4f7f\u5f92\u30df\u30fc\u30eb" ) // DM01-073 雷光の使徒ミール
	PORT_CONFSETTING( 0x006b, "Mighty Shouter / \u7121\u6575\u306e\u5486\u54ee" ) // DM01-105 無敵の咆哮
	PORT_CONFSETTING( 0x010d, "Milieus, the Daystretcher / \u7262\u9ed2\u306e\u4f1d\u9053\u5e2b\u30df\u30ea\u30a8\u30b9 (Ver.1)" ) // DM04-023 牢黒の伝道師ミリエス
	PORT_CONFSETTING( 0x008b, "Mini Titan Gett / \u5c0f\u3055\u306a\u52c7\u8005\u30b2\u30c3\u30c8 (Ver.1)" ) // DM02-049 小さな勇者ゲット
	PORT_CONFSETTING( 0x014c, "Miracle Quest / \u30df\u30e9\u30af\u30eb\u30fb\u30b5\u30fc\u30c1\u30e3\u30fc (Ver.1)" ) // DM05-026 ミラクル・サーチャー
	PORT_CONFSETTING( 0x0116, "Missile Boy / \u7206\u5f3e\u5c0f\u50e7\u30df\u30b5\u30a4\u30eb\u30dc\u30fc\u30a4 (Ver.1)" ) // DM04-032 爆弾小僧ミサイルボーイ
	PORT_CONFSETTING( 0x010e, "Mist Rias, Sonic Guardian / \u96f7\u9cf4\u306e\u5b88\u8b77\u8005\u30df\u30b9\u30c8\u30fb\u30ea\u30a8\u30b9 (Ver.1)" ) // DM04-024 雷鳴の守護者ミスト・リエス
	PORT_CONFSETTING( 0x0112, "Mongrel Man / \u5c51\u7537 (Ver.1)" ) // DM04-028 屑男
	PORT_CONFSETTING( 0x0153, "Moon Horn / \u6708\u6d74\u3059\u308b\u30e0\u30fc\u30f3\u30fb\u30db\u30fc\u30f3 (Ver.1)" ) // DM05-033 月浴するムーン・ホーン
	PORT_CONFSETTING( 0x0075, "Moonlight Flash / \u30e0\u30fc\u30f3\u30e9\u30a4\u30c8\u30fb\u30d5\u30e9\u30c3\u30b7\u30e5" ) // DM01-046 ムーンライト・フラッシュ
	PORT_CONFSETTING( 0x00e6, "Mudman / \u6ce5\u7537 (Ver.1)" ) // DM03-044 泥男
	PORT_CONFSETTING( 0x00d8, "Muramasa, Duke of Blades / \u5207\u65ad\u4f2f\u7235\u30e0\u30e9\u30de\u30b5 (Ver.1)" ) // DM03-030 切断伯爵ムラマサ
	PORT_CONFSETTING( 0x0109, "Mystic Inscription / \u30df\u30b9\u30c6\u30ea\u30fc\u30fb\u30d6\u30ec\u30b9 (Ver.1)" ) // DM04-019 ミステリー・ブレス
	PORT_CONFSETTING( 0x005e, "Natural Snare / \u30ca\u30c1\u30e5\u30e9\u30eb\u30fb\u30c8\u30e9\u30c3\u30d7" ) // DM01-039 ナチュラル・トラップ
	PORT_CONFSETTING( 0x004e, "Night Master, Shadow of Decay / \u6ec5\u3073\u306e\u5f71\u30ca\u30a4\u30c8\u30fb\u30de\u30b9\u30bf\u30fc" ) // DM01-023 滅びの影ナイト・マスター
	PORT_CONFSETTING( 0x00f6, "Niofa, Horned Protector / \u8b77\u308a\u306e\u89d2\u30d5\u30a3\u30aa\u30ca (Ver.1)" ) // DM04-S05 護りの角フィオナ
	PORT_CONFSETTING( 0x0144, "Nocturnal Giant / \u54c0\u54ed\u306e\u8d85\u4eba (Ver.1)" ) // DM05-018 哀哭の超人
	PORT_CONFSETTING( 0x0053, "Nomad Hero Gigio / \u653e\u6d6a\u306e\u52c7\u8005\u30b8\u30fc\u30b8\u30e7" ) // DM01-031 放浪の勇者ジージョ
	PORT_CONFSETTING( 0x0137, "Obsidian Scarab / \u30aa\u30d6\u30b7\u30c7\u30a3\u30a2\u30f3\u30fb\u30d3\u30fc\u30c8\u30eb (Ver.1)" ) // DM05-005 オブシディアン・ビートル
	PORT_CONFSETTING( 0x016e, "Olgate, Nightmare Samurai / \u9b54\u523b\u306e\u9a0e\u58eb\u30aa\u30eb\u30b2\u30a4\u30c8 (Ver.1)" ) // DMC04-S01 魔刻の騎士オルゲイト
	PORT_CONFSETTING( 0x0024, "Onslaughter Triceps / \u5f37\u8972\u5175\u30ae\u30ac\u30f3\u30c8\u30b9" ) // DM01-063 強襲兵ギガントス
	PORT_CONFSETTING( 0x00fd, "Ouks, Vizier of Restoration / \u4fee\u5fa9\u306e\u4f7f\u5f92\u30aa\u30fc\u30af\u30b9 (Ver.1)" ) // DM04-007 修復の使徒オークス
	PORT_CONFSETTING( 0x006d, "Pangaea's Song / \u30ac\u30a4\u30a2\u30ba\u30fb\u30bd\u30f3\u30b0" ) // DM01-070 ガイアズ・ソング
	PORT_CONFSETTING( 0x009a, "Phal Eega, Dawn Guardian / \u6681\u306e\u5b88\u8b77\u8005\u30d5\u30a1\u30eb\u30fb\u30a4\u30fc\u30ac (Ver.1)" ) // DM02-006 暁の守護者ファル・イーガ
	PORT_CONFSETTING( 0x002d, "Phantom Fish / \u30d5\u30a1\u30f3\u30c8\u30e0\u30fb\u30d5\u30a3\u30c3\u30b7\u30e5" ) // DM01-079 ファントム・フィッシュ
	PORT_CONFSETTING( 0x0103, "Photocide, Lord of the Wastes / \u8352\u5ec3\u306e\u5de8\u738b\u30b8\u30a7\u30ce\u30b5\u30a4\u30c9 (Ver.1)" ) // DM04-013 荒廃の巨王ジェノサイド
	PORT_CONFSETTING( 0x0128, "Pippie Kuppie / \u30d4\u30c3\u30d4\u30fb\u30af\u30c3\u30d4\u30fc (Ver.1)" ) // DM04-050 ピッピ・クッピー
	PORT_CONFSETTING( 0x00aa, "Plasma Chaser / \u30d7\u30e9\u30ba\u30de\u30fb\u30c1\u30a7\u30a4\u30b5\u30fc (Ver.1)" ) // DM02-002 プラズマ・チェイサー
	PORT_CONFSETTING( 0x007f, "Poison Worm / \u6bd2\u7159\u866b\u30dd\u30a4\u30ba\u30f3\u30fb\u30ef\u30fc\u30e0 (Ver.1)" ) // DM02-013 毒煙虫ポイズン・ワーム
	PORT_CONFSETTING( 0x0030, "Poisonous Dahlia / \u6bd2\u5410\u304d\u30c0\u30ea\u30a2" ) // DM01-065 毒吐きダリア
	PORT_CONFSETTING( 0x0022, "Poisonous Mushroom / \u30b7\u30d3\u30ec\u30a2\u30b7\u30c0\u30b1" ) // DM01-069 シビレアシダケ
	PORT_CONFSETTING( 0x013c, "Pokolul / \u30dd\u30b3\u30eb\u30eb (Ver.1)" ) // DM05-010 ポコルル
	PORT_CONFSETTING( 0x00dc, "Pouch Shell / \u30b7\u30a7\u30eb\u30fb\u30dd\u30fc\u30c1 (Ver.1)" ) // DM03-034 シェル・ポーチ
	PORT_CONFSETTING( 0x00cd, "Psyshroom / \u30de\u30dc\u30ed\u30b7\u30ad\u30ea\u30c0\u30b1 (Ver.1)" ) // DM03-019 マボロシキリダケ
	PORT_CONFSETTING( 0x0126, "Purple Piercer / \u30d1\u30fc\u30d7\u30eb\u30fb\u30d4\u30a2\u30b9 (Ver.1)" ) // DM04-048 パープル・ピアス
	PORT_CONFSETTING( 0x00cf, "Ra Vu, Seeker of Lightning / \u96f7\u96fb\u306e\u6c42\u9053\u8005\u30e9\u30fb\u30d3\u30e5\u30fc (Ver.1)" ) // DM03-021 雷電の求道者ラ・ビュー
	PORT_CONFSETTING( 0x00ee, "Raging Dash-Horn / \u6fc0\u6602\u3059\u308b\u30c0\u30c3\u30b7\u30e5\u30fb\u30db\u30fc\u30f3 (Ver.1)" ) // DM03-052 激昂するダッシュ・ホーン
	PORT_CONFSETTING( 0x00a8, "Rainbow Stone / \u30ec\u30a4\u30f3\u30dc\u30fc\u30fb\u30b9\u30c8\u30fc\u30f3 (Ver.1)" ) // DM02-055 レインボー・ストーン
	PORT_CONFSETTING( 0x0061, "Rayla, Truth Enforcer / \u771f\u7406\u306e\u4f1d\u9053\u5e2b\u30ec\u30a4\u30fc\u30e9" ) // DM01-002 真理の伝道師レイーラ
	PORT_CONFSETTING( 0x00c0, "Raza Vega, Thunder Guardian / \u96f7\u96f2\u306e\u5b88\u8b77\u8005\u30e9\u30fc\u30b6\u30fb\u30d9\u30ac (Ver.1)" ) // DM03-006 雷雲の守護者ラーザ・ベガ
	PORT_CONFSETTING( 0x010b, "Re Bil, Seeker of Archery / \u8056\u5f13\u306e\u6c42\u9053\u8005\u30ec\u30fb\u30d3\u30fc\u30eb (Ver.1)" ) // DM04-021 聖弓の求道者レ・ビール
	PORT_CONFSETTING( 0x00b4, "Recon Operation / \u30de\u30a4\u30f3\u30c9\u30fb\u30b5\u30fc\u30c1 (Ver.1)" ) // DM02-043 マインド・サーチ
	PORT_CONFSETTING( 0x0020, "Red-Eye Scorpion / \u30ec\u30c3\u30c9\u30a2\u30a4\u30fb\u30b9\u30b3\u30fc\u30d4\u30aa\u30f3" ) // DM01-037 レッドアイ・スコーピオン
	PORT_CONFSETTING( 0x009b, "Reso Pacos, Clear Sky Guardian / \u6674\u5929\u306e\u5b88\u8b77\u8005\u30ec\u30be\u30fb\u30d1\u30b3\u30b9 (Ver.1)" ) // DM02-038 晴天の守護者レゾ・パコス
	PORT_CONFSETTING( 0x0052, "Reusol, the Oracle / \u4e88\u8a00\u8005\u30ea\u30e5\u30be\u30eb" ) // DM01-075 予言者リュゾル
	PORT_CONFSETTING( 0x0064, "Revolver Fish / \u30ea\u30dc\u30eb\u30d0\u30fc\u30fb\u30d5\u30a3\u30c3\u30b7\u30e5" ) // DM01-048 リボルバー・フィッシュ
	PORT_CONFSETTING( 0x0163, "Rikabu, the Dismantler / \u89e3\u4f53\u5c4b\u30d4\u30fc\u30ab\u30d7 (Ver.1)" ) // DM05-049 解体屋ピーカプ
	PORT_CONFSETTING( 0x00f2, "Rimuel, Cloudbreak Elemental / \u5f57\u661f\u306e\u7cbe\u970a\u30ea\u30e0\u30a8\u30eb (Ver.1)" ) // DM04-S01 彗星の精霊リムエル
	PORT_CONFSETTING( 0x00f1, "Roar of the Earth / \u5927\u5730\u306e\u5486\u54ee (Ver.1)" ) // DM03-055 大地の咆哮
	PORT_CONFSETTING( 0x0048, "Roaring Great-Horn / \u5486\u54ee\u3059\u308b\u30b0\u30ec\u30fc\u30c8\u30fb\u30db\u30fc\u30f3" ) // DM01-S09 咆哮するグレート・ホーン
	PORT_CONFSETTING( 0x0032, "Rothus, the Traveler / \u9283\u795e\u5175\u30c7\u30a3\u30aa\u30e9\u30a4\u30aa\u30b9" ) // DM01-032 銃神兵ディオライオス
	PORT_CONFSETTING( 0x0016, "Ruby Grass / \u7d05\u7389\u8349" ) // DM01-044 紅玉草
	PORT_CONFSETTING( 0x0090, "Rumble Gate / \u30af\u30a8\u30a4\u30af\u30fb\u30b2\u30fc\u30c8 (Ver.1)" ) // DM02-051 クエイク・ゲート
	PORT_CONFSETTING( 0x00a1, "Rumbling Terahorn / \u9cf4\u52d5\u3059\u308b\u30ae\u30ac\u30fb\u30db\u30fc\u30f3 (Ver.1)" ) // DM02-019 鳴動するギガ・ホーン
	PORT_CONFSETTING( 0x011d, "Sarius, Vizier of Suppression / \u93ae\u5727\u306e\u4f7f\u5f92\u30b5\u30ea\u30a8\u30b9 (Ver.1)" ) // DM04-039 鎮圧の使徒サリエス
	PORT_CONFSETTING( 0x000c, "Saucer-Head Shark / \u30bd\u30fc\u30b5\u30fc\u30d8\u30c3\u30c9\u30fb\u30b7\u30e3\u30fc\u30af" ) // DM01-018 ソーサーヘッド・シャーク
	PORT_CONFSETTING( 0x003b, "Scarlet Skyterror / \u30af\u30ea\u30e0\u30be\u30f3\u30fb\u30ef\u30a4\u30d0\u30fc\u30f3" ) // DM01-S07 クリムゾン・ワイバーン
	PORT_CONFSETTING( 0x014f, "Scheming Hands / \u7b56\u7565\u306e\u624b (Ver.1)" ) // DM05-029 策略の手
	PORT_CONFSETTING( 0x00ac, "Scissor Eye / \u30b7\u30b6\u30fc\u30fb\u30a2\u30a4 (Ver.1)" ) // DM02-041 シザー・アイ
	PORT_CONFSETTING( 0x0166, "Scissor Scarab / \u30b7\u30b6\u30fc\u30ba\u30fb\u30d3\u30fc\u30c8\u30eb (Ver.1)" ) // DM05-052 シザーズ・ビートル
	PORT_CONFSETTING( 0x00c7, "Scratchclaw / \u7259\u7537 (Ver.1)" ) // DM03-013 牙男
	PORT_CONFSETTING( 0x010f, "Screaming Sunburst / \u30a8\u30f3\u30b8\u30a7\u30eb\u30fb\u30bd\u30f3\u30b0 (Ver.1)" ) // DM04-025 エンジェル・ソング
	PORT_CONFSETTING( 0x013b, "Sea Slug / \u30b7\u30fc\u30fb\u30b9\u30e9\u30c3\u30b0 (Ver.1)" ) // DM05-009 シー・スラッグ
	PORT_CONFSETTING( 0x0042, "Seamine / \u30b7\u30fc\u30de\u30a4\u30f3" ) // DM01-004 シーマイン
	PORT_CONFSETTING( 0x00da, "Searing Wave / \u707c\u71b1\u6ce2 (Ver.1)" ) // DM03-032 灼熱波
	PORT_CONFSETTING( 0x0055, "Senatine Jade Tree / \u7fe1\u7fe0\u6a39" ) // DM01-071 翡翠樹
	PORT_CONFSETTING( 0x0124, "Shadow Moon, Cursed Shade / \u546a\u3044\u306e\u5f71\u30b7\u30e3\u30c9\u30a6\u30fb\u30e0\u30fc\u30f3 (Ver.1)" ) // DM04-046 呪いの影シャドウ・ムーン
	PORT_CONFSETTING( 0x00e2, "Shtra / \u30b7\u30e5\u30c8\u30e9 (Ver.1)" ) // DM03-040 シュトラ
	PORT_CONFSETTING( 0x00bb, "Sieg Balicula, the Intense / \u8056\u5929\u4f7f\u30b8\u30fc\u30af\u30fb\u30d0\u30ea\u30ad\u30e5\u30fc\u30e9 (Ver.1)" ) // DM03-001 聖天使ジーク・バリキューラ
	PORT_CONFSETTING( 0x00a5, "Silver Axe / \u9280\u306e\u6226\u65a7 (Ver.1)" ) // DM02-053 銀の戦斧
	PORT_CONFSETTING( 0x00a3, "Silver Fist / \u9280\u306e\u62f3 (Ver.1)" ) // DM02-034 銀の拳
	PORT_CONFSETTING( 0x0135, "Sinister General Damudo / \u51a5\u5c06\u30c0\u30e0\u30c9 (Ver.1)" ) // DM05-003 冥将ダムド
	PORT_CONFSETTING( 0x0017, "Skeleton Soldier, the Defiled / \u6c5a\u308c\u305f\u8005\u30b9\u30b1\u30eb\u30c8\u30f3\u30bd\u30eb\u30b8\u30e3\u30fc" ) // DM01-087 汚れた者スケルトンソルジャー
	PORT_CONFSETTING( 0x0125, "Skeleton Thief, the Revealer / \u95c7\u3092\u3042\u3070\u304f\u8005\u30b9\u30b1\u30eb\u30c8\u30f3\u30fb\u30b7\u30fc\u30d5 (Ver.1)" ) // DM04-047 闇をあばく者スケルトン・シーフ
	PORT_CONFSETTING( 0x013f, "Skullsweeper Q / \u30c0\u30fc\u30af\u30fb\u30c6\u30a3\u30a2\u30e9γ (Ver.1)" ) // DM05-013 ダーク・ティアラγ
	PORT_CONFSETTING( 0x0140, "Slime Veil / \u30d5\u30a1\u30f3\u30c8\u30e0\u30fb\u30d9\u30fc\u30eb (Ver.1)" ) // DM05-014 ファントム・ベール
	PORT_CONFSETTING( 0x0168, "Smash Horn Q / \u5a01\u5687\u3059\u308b\u30b9\u30de\u30c3\u30b7\u30e5\u30fb\u30db\u30fc\u30f3α (Ver.1)" ) // DM05-054 威嚇するスマッシュ・ホーンα
	PORT_CONFSETTING( 0x00ff, "Smile Angler / \u30b9\u30de\u30a4\u30eb\u30fb\u30a2\u30f3\u30b0\u30e9\u30fc (Ver.1)" ) // DM04-009 スマイル・アングラー
	PORT_CONFSETTING( 0x00e9, "Snake Attack / \u30b9\u30cd\u30fc\u30af\u30a2\u30bf\u30c3\u30af (Ver.1)" ) // DM03-047 スネークアタック
	PORT_CONFSETTING( 0x00ec, "Snip Striker Bullraizer / \u5947\u8972\u5175\u30d6\u30eb\u30ec\u30a4\u30b6\u30fc (Ver.1)" ) // DM03-050 奇襲兵ブルレイザー
	PORT_CONFSETTING( 0x00f0, "Sniper Mosquito / \u30b9\u30ca\u30a4\u30d7\u30fb\u30e2\u30b9\u30ad\u30fc\u30c8 (Ver.1)" ) // DM03-054 スナイプ・モスキート
	PORT_CONFSETTING( 0x0139, "Snork La, Shrine Guardian / \u795e\u9b42\u306e\u5b88\u8b77\u8005\u30b7\u30e5\u30ce\u30fc\u30af\u30fb\u30e9\u30fc (Ver.1)" ) // DM05-007 神魂の守護者シュノーク・ラー
	PORT_CONFSETTING( 0x005f, "Solar Ray / \u30bd\u30fc\u30e9\u30fc\u30fb\u30ec\u30a4" ) // DM01-078 ソーラー・レイ
	PORT_CONFSETTING( 0x015d, "Solidskin Fish / \u5ca9\u808c\u9b5a (Ver.1)" ) // DM05-043 岩肌魚
	PORT_CONFSETTING( 0x0076, "Sonic Wing / \u30bd\u30cb\u30c3\u30af\u30fb\u30a6\u30a4\u30f3\u30b0" ) // DM01-077 ソニック・ウイング
	PORT_CONFSETTING( 0x0115, "Soul Gulp / \u30bd\u30a6\u30eb\u30fb\u30a4\u30fc\u30bf\u30fc (Ver.1)" ) // DM04-031 ソウル・イーター
	PORT_CONFSETTING( 0x00d0, "Sparkle Flower / \u7dda\u5149\u82b1 (Ver.1)" ) // DM03-022 線光花
	PORT_CONFSETTING( 0x015b, "Spikestrike Ichthys Q / \u6d41\u661f\u9b5aα (Ver.1)" ) // DM05-041 流星魚α
	PORT_CONFSETTING( 0x0058, "Spiral Gate / \u30b9\u30d1\u30a4\u30e9\u30eb\u30fb\u30b2\u30fc\u30c8" ) // DM01-086 スパイラル・ゲート
	PORT_CONFSETTING( 0x0092, "Spiral Grass / \u7adc\u5dfb\u8349 (Ver.1)" ) // DM02-008 竜巻草
	PORT_CONFSETTING( 0x014b, "Split-Head Hydroturtle Q / \u30c4\u30a4\u30f3\u30d8\u30c3\u30c9\u30fb\u30bf\u30fc\u30c8\u30ebβ (Ver.1)" ) // DM05-025 ツインヘッド・タートルβ
	PORT_CONFSETTING( 0x00af, "Stained Glass / \u30b9\u30c6\u30f3\u30c9\u30b0\u30e9\u30b9 (Ver.1)" ) // DM02-025 ステンドグラス
	PORT_CONFSETTING( 0x0021, "Stampeding Longhorn / \u66b4\u8d70\u3059\u308b\u30ed\u30f3\u30b0\u30fb\u30db\u30fc\u30f3" ) // DM01-038 暴走するロング・ホーン
	PORT_CONFSETTING( 0x002a, "Steel Smasher / \u92fc\u9244\u306e\u939a" ) // DM01-107 鋼鉄の鎚
	PORT_CONFSETTING( 0x015c, "Steel-Turret Cluster / \u30b9\u30c1\u30fc\u30eb\u30a2\u30fc\u30e0\u30fb\u30af\u30e9\u30b9\u30bf\u30fc (Ver.1)" ) // DM05-042 スチールアーム・クラスター
	PORT_CONFSETTING( 0x00d3, "Stinger Ball / \u30b9\u30c6\u30a3\u30f3\u30ac\u30fc\u30fb\u30dc\u30fc\u30eb (Ver.1)" ) // DM03-025 スティンガー・ボール
	PORT_CONFSETTING( 0x0035, "Stinger Worm / \u51f6\u98df\u866b\u30b9\u30c6\u30a3\u30f3\u30ac\u30fc\u30ef\u30fc\u30e0" ) // DM01-057 凶食虫スティンガーワーム
	PORT_CONFSETTING( 0x0008, "Stonesaur / \u30b9\u30c8\u30fc\u30f3\u30b6\u30a6\u30eb\u30b9" ) // DM01-059 ストーンザウルス
	PORT_CONFSETTING( 0x0162, "Storm Javelin Wyvern / \u30b9\u30c8\u30fc\u30e0\u30b8\u30e3\u30d9\u30ea\u30f3\u30fb\u30ef\u30a4\u30d0\u30fc\u30f3 (Ver.1)" ) // DM05-048 ストームジャベリン・ワイバーン
	PORT_CONFSETTING( 0x004a, "Storm Shell / \u30b7\u30a7\u30eb\u30fb\u30b9\u30c8\u30fc\u30e0" ) // DM01-035 シェル・ストーム
	PORT_CONFSETTING( 0x00c5, "Streaming Shaper / \u30b9\u30c8\u30ea\u30fc\u30df\u30f3\u30b0\u30fb\u30b7\u30a7\u30a4\u30d1\u30fc (Ver.1)" ) // DM03-011 ストリーミング・シェイパー
	PORT_CONFSETTING( 0x00e1, "Sundrop Armor / \u30db\u30fc\u30ea\u30fc\u30fb\u30e1\u30fc\u30eb (Ver.1)" ) // DM03-039 ホーリー・メール
	PORT_CONFSETTING( 0x0002, "Super Explosive Volcanodon / \u8d85\u7832\u624b\u30dc\u30eb\u30ab\u30ce\u30c9\u30f3" ) // DM01-062 超砲手ボルカノドン
	PORT_CONFSETTING( 0x00fb, "Supporting Tulip / \u5fdc\u63f4\u30c1\u30e5\u30fc\u30ea\u30c3\u30d7 (Ver.1)" ) // DM04-005 応援チューリップ
	PORT_CONFSETTING( 0x001c, "Swamp Worm / \u8150\u98df\u866b\u30b9\u30ef\u30f3\u30d7\u30ef\u30fc\u30e0" ) // DM01-053 腐食虫スワンプワーム
	PORT_CONFSETTING( 0x00ef, "Sword Butterfly / \u30bd\u30fc\u30c9\u30fb\u30d0\u30bf\u30d5\u30e9\u30a4 (Ver.1)" ) // DM03-053 ソード・バタフライ
	PORT_CONFSETTING( 0x010a, "Sword of Benevolent Life / \u30d6\u30ec\u30b9\u30fb\u30bd\u30fc\u30c9 (Ver.1)" ) // DM04-020 ブレス・ソード
	PORT_CONFSETTING( 0x0117, "Sword of Malevolent Death / \u30c7\u30fc\u30e2\u30f3\u30fb\u30bd\u30fc\u30c9 (Ver.1)" ) // DM04-033 デーモン・ソード
	PORT_CONFSETTING( 0x0138, "Syforce, Aurora Elemental / \u5149\u7ffc\u306e\u7cbe\u970a\u30b5\u30a4\u30d5\u30a9\u30b9 (Ver.1)" ) // DM05-006 光翼の精霊サイフォス
	PORT_CONFSETTING( 0x012e, "Syrius, Firmament Elemental / \u5929\u6d77\u306e\u7cbe\u970a\u30b7\u30ea\u30a6\u30b9 (Ver.1)" ) // DM05-S01 天海の精霊シリウス
	PORT_CONFSETTING( 0x002f, "Szubs Kin, Twilight Guardian / \u9ec4\u660f\u306e\u5b88\u8b77\u8005\u30b7\u30fc\u30d6\u30b9\u30fb\u30ad\u30fc\u30f3" ) // DM01-011 黄昏の守護者シーブス・キーン
	PORT_CONFSETTING( 0x0078, "Teleportation / \u30c6\u30ec\u30dd\u30fc\u30c6\u30fc\u30b7\u30e7\u30f3" ) // DM01-021 テレポーテーション
	PORT_CONFSETTING( 0x0072, "Terror Pit / \u30c7\u30fc\u30e2\u30f3\u30fb\u30cf\u30f3\u30c9" ) // DM01-027 デーモン・ハンド
	PORT_CONFSETTING( 0x0037, "Thorny Mandra / \u30c8\u30b2\u523a\u3057\u30de\u30f3\u30c9\u30e9" ) // DM01-010 トゲ刺しマンドラ
	PORT_CONFSETTING( 0x0118, "Three-Eyed Dragonfly / \u30b9\u30ea\u30fc\u30a2\u30a4\u30ba\u30fb\u30c9\u30e9\u30b4\u30f3\u30d5\u30e9\u30a4 (Ver.1)" ) // DM04-034 スリーアイズ・ドラゴンフライ
	PORT_CONFSETTING( 0x0149, "Thunder Net / \u30b5\u30f3\u30c0\u30fc\u30fb\u30cd\u30c3\u30c8 (Ver.1)" ) // DM05-023 サンダー・ネット
	PORT_CONFSETTING( 0x003d, "Toel, Vizier of Hope / \u5e0c\u671b\u306e\u4f7f\u5f92\u30c8\u30fc\u30eb" ) // DM01-041 希望の使徒トール
	PORT_CONFSETTING( 0x012d, "Torcon / \u5b64\u9ad8\u306e\u9858 (Ver.1)" ) // DM04-055 孤高の願
	PORT_CONFSETTING( 0x005d, "Tornado Flame / \u30c8\u30eb\u30cd\u30fc\u30c9\u30fb\u30d5\u30ec\u30fc\u30e0" ) // DM01-064 トルネード・フレーム
	PORT_CONFSETTING( 0x0040, "Tower Shell / \u30b7\u30a7\u30eb\u30fb\u30bf\u30ef\u30fc" ) // DM01-009 シェル・タワー
	PORT_CONFSETTING( 0x004f, "Tri-horn Shepherd / \u7fa4\u308c\u3092\u5b88\u308b\u30c8\u30e9\u30a4\u30fb\u30db\u30fc\u30f3" ) // DM01-036 群れを守るトライ・ホーン
	PORT_CONFSETTING( 0x0009, "Tropico / \u30c8\u30ed\u30d4\u30b3" ) // DM01-019 トロピコ
	PORT_CONFSETTING( 0x00f9, "Trox, General of Destruction / \u51f6\u9aa8\u306e\u90aa\u5c06\u30af\u30a8\u30a4\u30af\u30b9 (Ver.1)" ) // DM04-003 凶骨の邪将クエイクス
	PORT_CONFSETTING( 0x0141, "Twin-Cannon Skyterror / \u30c4\u30a4\u30f3\u30ad\u30e3\u30ce\u30f3\u30fb\u30ef\u30a4\u30d0\u30fc\u30f3 (Ver.1)" ) // DM05-015 ツインキャノン・ワイバーン
	PORT_CONFSETTING( 0x00be, "Überdragon Jabaha / \u8d85\u7adc\u30b8\u30e3\u30d0\u30cf (Ver.1)" ) // DM03-004 超竜ジャバハ
	PORT_CONFSETTING( 0x016b, "Überdragon Zaschack / \u8d85\u7adc\u30b6\u30b7\u30e3\u30c3\u30af (Ver.1)" ) // PROMOY1-011 超竜ザシャック
	PORT_CONFSETTING( 0x006e, "Ultimate Force / \u30a2\u30eb\u30c6\u30a3\u30e1\u30c3\u30c8\u30fb\u30d5\u30a9\u30fc\u30b9" ) // DM01-109 アルティメット・フォース
	PORT_CONFSETTING( 0x007c, "Ultracide Worm / \u9b54\u7ffc\u866b\u30b8\u30a7\u30ce\u30b5\u30a4\u30c9\u30fb\u30ef\u30fc\u30e0 (Ver.1)" ) // DM02-S03 魔翼虫ジェノサイド・ワーム
	PORT_CONFSETTING( 0x000e, "Unicorn Fish / \u4e00\u89d2\u9b5a" ) // DM01-020 一角魚
	PORT_CONFSETTING( 0x00df, "Ur Pale, Seeker of Sunlight / \u967d\u5149\u306e\u6c42\u9053\u8005\u30eb\u30fb\u30d1\u30fc\u30ec (Ver.1)" ) // DM03-037 陽光の求道者ル・パーレ
	PORT_CONFSETTING( 0x0015, "Urth, Purifying Elemental / \u6d44\u5316\u306e\u7cbe\u970a\u30a6\u30eb\u30b9" ) // DM01-S02 浄化の精霊ウルス
	PORT_CONFSETTING( 0x004c, "Vampire Silphy / \u5996\u59eb\u30b7\u30eb\u30d5\u30a3" ) // DM01-005 妖姫シルフィ
	PORT_CONFSETTING( 0x013e, "Vashuna, Sword Dancer / \u5263\u821e\u306e\u4fee\u7f85\u30f4\u30a1\u30b7\u30e5\u30ca (Ver.1)" ) // DM05-012 剣舞の修羅ヴァシュナ
	PORT_CONFSETTING( 0x016f, "Velyrika Dragon / \u30d0\u30eb\u30ad\u30ea\u30fc\u30fb\u30c9\u30e9\u30b4\u30f3 (Ver.1)" ) // DMC03-S02 バルキリー・ドラゴン
	PORT_CONFSETTING( 0x0059, "Virtual Tripwire / \u30a8\u30ec\u30e1\u30f3\u30bf\u30eb\u30fb\u30c8\u30e9\u30c3\u30d7" ) // DM01-085 エレメンタル・トラップ
	PORT_CONFSETTING( 0x00ed, "Volcanic Arrows / \u30dc\u30eb\u30ab\u30cb\u30c3\u30af\u30fb\u30a2\u30ed\u30fc (Ver.1)" ) // DM03-051 ボルカニック・アロー
	PORT_CONFSETTING( 0x0111, "Volcano Smog, Deceptive Shade / \u507d\u308a\u306e\u5f71\u30d8\u30eb\u30fb\u30b9\u30e2\u30fc\u30af (Ver.1)" ) // DM04-027 偽りの影ヘル・スモーク
	PORT_CONFSETTING( 0x00e7, "Wailing Shadow Belbetphlo / \u5606\u304d\u306e\u5f71\u30d9\u30eb\u30d9\u30c3\u30c8\u30d5\u30ed\u30fc (Ver.1)" ) // DM03-045 嘆きの影ベルベットフロー
	PORT_CONFSETTING( 0x0018, "Wandering Braineater / \u3055\u307e\u3088\u3046\u8005\u30d6\u30ec\u30a4\u30f3\u30fb\u30a4\u30fc\u30bf\u30fc" ) // DM01-091 さまよう者ブレイン・イーター
	PORT_CONFSETTING( 0x011e, "Whisking Whirlwind / \u30e9\u30a4\u30c8\u30fb\u30c7\u30a3\u30d5\u30a7\u30f3\u30b9 (Ver.1)" ) // DM04-040 ライト・ディフェンス
	PORT_CONFSETTING( 0x0160, "Wisp Howler, Shadow of Tears / \u9b54\u5e7b\u306e\u5f71\u30a8\u30bf\u30fc\u30ca\u30eb\u30fb\u30af\u30e9\u30a4 (Ver.1)" ) // DM05-046 魔幻の影エターナル・クライ
	PORT_CONFSETTING( 0x0029, "Writhing Bone Ghoul / \u3046\u3054\u3081\u304f\u8005\u30dc\u30fc\u30f3\u30fb\u30b0\u30fc\u30eb" ) // DM01-090 うごめく者ボーン・グール
	PORT_CONFSETTING( 0x0093, "Wyn, the Oracle / \u4e88\u8a00\u8005\u30a6\u30a3\u30f3 (Ver.1)" ) // DM02-022 予言者ウィン
	PORT_CONFSETTING( 0x00a6, "Xeno Mantis / \u30bc\u30ce\u30fb\u30de\u30f3\u30c6\u30a3\u30b9 (Ver.1)" ) // DM02-005 ゼノ・マンティス
	PORT_CONFSETTING( 0x001b, "Zagaan, Knight of Darkness / \u6697\u9ed2\u306e\u9a0e\u58eb\u30b6\u30ac\u30fc\u30f3" ) // DM01-S06 暗黒の騎士ザガーン
INPUT_PORTS_END


xavix_duelmast_scanner_device::xavix_duelmast_scanner_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_analog_image_sensor_device_base(mconfig, XAVIX_DUELMAST_SCANNER, tag, owner, clock, 64)
	, m_picture(*this, "picture")
	, m_input(*this, "INPUT")
	, m_card_data_cb(*this, 0)
	, m_expanded_cards_cb(*this, 0)
{
}

void xavix_duelmast_scanner_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}

ioport_constructor xavix_duelmast_scanner_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_duelmast_scanner);
}

void xavix_duelmast_scanner_device::device_start()
{
	xavix_analog_image_sensor_device_base::device_start();
	for (unsigned level = 0; level < m_source_response.size(); ++level)
		m_source_response[level] = std::pow(double(level) / 255, 2.2);
}

void xavix_duelmast_scanner_device::capture_image()
{
	// The slot is enclosed; without illumination the analogue dark level is approximated as zero.
	if (!illuminated())
	{
		m_image.fill(0);
		return;
	}

	unsigned const selection = m_input->read();
	if (selection > 1 && capture_manual_card(selection - 1))
		return;

	bitmap_argb32 const &picture = m_picture->get_bitmap();
	if (selection || !picture.valid())
	{
		// The ROM specifies these mark centres; their optical size and brightness are representative.
		m_image.fill(160);
		for (auto const &centre : COVER_MARK_CENTRES)
		{
			for (int y = centre[1] - 1; y <= centre[1] + 1; ++y)
			{
				for (int x = centre[0] - 1; x <= centre[0] + 1; ++x)
					m_image[y * IMAGE_WIDTH + x] = 40;
			}
		}
		return;
	}

	// The source gamma, red-weighted colour response and ADC levels are fitted approximations, not measured optics.
	double const left = CARD_FIELD[0] * picture.width();
	double const top = CARD_FIELD[1] * picture.height();
	double const pixel_width = CARD_FIELD[2] * picture.width() / IMAGE_WIDTH;
	double const pixel_height = CARD_FIELD[3] * picture.height() / IMAGE_HEIGHT;
	for (unsigned y = 0; y < IMAGE_HEIGHT; ++y)
	{
		double const y0 = top + y * pixel_height;
		double const y1 = top + (y + 1) * pixel_height;
		for (unsigned x = 0; x < IMAGE_WIDTH; ++x)
		{
			double const x0 = left + x * pixel_width;
			double const x1 = left + (x + 1) * pixel_width;
			double light = 0;
			for (int source_y = int(std::floor(y0)); source_y < int(std::ceil(y1)); ++source_y)
			{
				double const height = std::min(y1, double(source_y + 1)) - std::max(y0, double(source_y));
				for (int source_x = int(std::floor(x0)); source_x < int(std::ceil(x1)); ++source_x)
				{
					double const width = std::min(x1, double(source_x + 1)) - std::max(x0, double(source_x));
					rgb_t const colour(picture.pix(source_y, source_x));
					light += (0.85 * m_source_response[colour.r()] + 0.15 * m_source_response[colour.g()]) * width * height;
				}
			}
			m_image[y * IMAGE_WIDTH + x] = 48 + std::floor(18 * light / (pixel_width * pixel_height) + 0.75);
		}
	}
}

bool xavix_duelmast_scanner_device::capture_manual_card(unsigned card)
{
	bool const expanded = m_expanded_cards_cb();
	if (!card || card > (expanded ? 366 : 120))
		return false;

	// HACK: Reconstruct pixels from the ROM's recognition signatures for optional named-card selection.
	// The ROM still acquires the pixels and performs recognition through the normal sensor interface.
	std::array<int, 64> signature;
	for (unsigned pixel = 0; pixel < signature.size(); ++pixel)
	{
		offs_t const offset = card * 64 + pixel;
		signature[pixel] = s8(m_card_data_cb(offset));
	}

	if (!expanded)
	{
		m_image.fill(48);
		m_image[0] = 1;
		for (unsigned y = 0; y < 24; ++y)
		{
			for (unsigned x = 0; x < 24; ++x)
				m_image[(y + 3) * IMAGE_WIDTH + x + 4] = std::clamp(int(std::floor(32 + 0.5 * signature[(y / 3) * 8 + x / 3] + 0.5)), 17, 55);
		}
	}
	else
	{
		// Compensate for the cartridge's overlapping filter before expanding its seven-by-seven signature.
		std::array<double, 64> sharpened{};
		for (int y = 0; y < 7; ++y)
		{
			for (int x = 0; x < 7; ++x)
			{
				sharpened[y * 8 + x] = signature[y * 8 + x] + 0.5 * (4 * signature[y * 8 + x]
						- signature[y * 8 + std::max(0, x - 1)] - signature[y * 8 + std::min(6, x + 1)]
						- signature[std::max(0, y - 1) * 8 + x] - signature[std::min(6, y + 1) * 8 + x]);
			}
		}

		m_image.fill(128);
		for (unsigned y = 4; y < 28; ++y)
		{
			for (unsigned x = 5; x < 29; ++x)
			{
				double const xf = std::clamp((double(x) - 7.5) / 3, 0.0, 6.0);
				double const yf = std::clamp((double(y) - 6.5) / 3, 0.0, 6.0);
				unsigned const xa = unsigned(xf);
				unsigned const ya = unsigned(yf);
				unsigned const xb = std::min(6U, xa + 1);
				unsigned const yb = std::min(6U, ya + 1);
				double const dx = xf - xa;
				double const dy = yf - ya;
				double const value = (1 - dx) * (1 - dy) * sharpened[ya * 8 + xa] + dx * (1 - dy) * sharpened[ya * 8 + xb]
						+ (1 - dx) * dy * sharpened[yb * 8 + xa] + dx * dy * sharpened[yb * 8 + xb];
				m_image[y * IMAGE_WIDTH + x] = std::clamp(int(std::floor(144 + value + 0.5)), 48, 192);
			}
		}
	}
	return true;
}
