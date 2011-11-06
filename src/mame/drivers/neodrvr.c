/***************************************************************************

    neodrvr.c

    Neo-Geo hardware
    This file contains all game specific overrides


    Title catalogue
    (source: http://neogeomuseum.snkplaymore.co.jp/english/catalogue/index.php)

    In 2010, SNK Playmore, the successor of SNK, released a title catalogue which lists the released
    games (MVS/AES/CD) including their release dates in Japan. It is not 100% complete.
    The included title catalogue is the english one.

    Game Title                                                  Genre           Publisher       Date Released (in Japan)
    =================================================================================================================================
    NAM-1975                                                    3D Action       SNK             MVS Cartridge:1990/04/26
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/09/09
    MAHJONG KYORETSUDEN                                         Mahjong         SNK             MVS Cartridge:1990/04/26
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/09/09
    MAGICIAN LORD                                               Action          ADK             MVS Cartridge:1990/04/26
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/10/31
    BASEBALL STARS PROFESSIONAL                                 Sports          SNK             MVS Cartridge:1990/04/26
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1995/04/21
    TOP PLAYER'S GOLF                                           Sports          SNK             MVS Cartridge:1990/05/23
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/09/09
    NINJA COMBAT                                                Action          ADK             MVS Cartridge:1990/07/24
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/10/31
    RIDING HERO                                                 3D Racing       SNK             MVS Cartridge:1990/07/24
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1995/05/26
    THE SUPER SPY                                               3D Action       SNK             MVS Cartridge:1990/10/08
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/09/09
    CYBER-LIP                                                   Action          SNK             MVS Cartridge:1990/11/07
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1995/04/21
    PUZZLED                                                     Puzzle          SNK             MVS Cartridge:1990/11/20
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/09/09
    LEAGUE BOWLING                                              Sports          SNK             MVS Cartridge:1990/12/10
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/09/09
    GHOST PILOTS                                                Shooter         SNK             MVS Cartridge:1991/01/25
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1995/03/17
    SENGOKU                                                     Action          SNK             MVS Cartridge:1991/02/12
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1995/03/17
    KING OF THE MONSTERS                                        Fighting        SNK             MVS Cartridge:1991/02/25
                                                                                                NEOGEO ROM-cart:1991/07/01
    BLUE'S JOURNEY                                              Action          ADK             MVS Cartridge:1991/03/14
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/10/31
    ALPHA MISSION II                                            Shooter         SNK             MVS Cartridge:1991/03/25
                                                                                                NEOGEO ROM-cart:1991/07/01
                                                                                                NEOGEO CD:1994/09/09
    BURNING FIGHT                                               Action          SNK             MVS Cartridge:1991/05/20
                                                                                                NEOGEO ROM-cart:1991/08/09
                                                                                                NEOGEO CD:1994/09/09
    MINNASAN NO OKAGESAMA DESU                                  Table           Monolith        MVS Cartridge:1991/07/25
                                                                                                NEOGEO ROM-cart:1991/07/21
    CROSSED SWORDS                                              Action          ADK             MVS Cartridge:1991/07/25
                                                                                                NEOGEO ROM-cart:1991/10/01
                                                                                                NEOGEO CD:1994/10/31
    LEGEND OF SUCCESS JOE                                       Action          Wave            MVS Cartridge:1991/07
                                                                                                NEOGEO ROM-cart:1991/08/30
    QUIZ DAISUSA SEN: THE LAST COUNT DOWN                       Quiz            SNK             MVS Cartridge:1991/07
                                                                                                NEOGEO ROM-cart:1991/08/30
    SUPER BASEBALL 2020                                         Sports          SNK             MVS Cartridge:1991/09/20
                                                                                                NEOGEO ROM-cart:1991/10/25
                                                                                                NEOGEO CD:1995/02/25
    ROBO ARMY                                                   Action          SNK             MVS Cartridge:1991/10/30
                                                                                                NEOGEO ROM-cart:1991/12/20
                                                                                                NEOGEO CD:1995/04/21
    THRASH RALLY                                                Racing          ADK             MVS Cartridge:1991/11/08
                                                                                                NEOGEO ROM-cart:1991/12/20
                                                                                                NEOGEO CD:1994/10/31
    EIGHT MAN                                                   Action          SNK             NEOGEO ROM-cart:1991/11/20
    FATAL FURY                                                  Fighting        SNK             MVS Cartridge:1991/11/25
                                                                                                NEOGEO ROM-cart:1991/12/20
                                                                                                NEOGEO CD:1994/09/09
    BAKATONO-SAMA MAHJONG MAN'YUKI                              Mahjong         Monolith        MVS Cartridge:1991/11
                                                                                                NEOGEO ROM-cart:1991/12/13
    THRASH RALLY                                                Racing          ADK             NEOGEO ROM-cart:1991/12/20
    FOOTBALL FRENZY                                             Sports          SNK             MVS Cartridge:1992/01/31
                                                                                                NEOGEO ROM-cart:1992/02/21
                                                                                                NEOGEO CD:1994/09/09
    SOCCER BRAWL                                                Sports          SNK             MVS Cartridge:1992/02/14
                                                                                                NEOGEO ROM-cart:1992/03/13
                                                                                                NEOGEO CD:1995/03/31
    MUTATION NATION                                             Action          SNK             MVS Cartridge:1992/03/16
                                                                                                NEOGEO ROM-cart:1992/04/17
                                                                                                NEOGEO CD:1995/02/25
    LAST RESORT                                                 Shooter         SNK             MVS Cartridge:1992/03/23
                                                                                                NEOGEO ROM-cart:1992/04/24
                                                                                                NEOGEO CD:1994/09/09
    QUIZ MEITANTEI NEO & GEO: QUIZ DAISOUSASEN PART 2           Quiz            SNK             MVS Cartridge:1992/03
                                                                                                NEOGEO ROM-cart:1991/04/24
    BASEBALL STARS 2                                            Sports          SNK             MVS Cartridge:1992/04/15
                                                                                                NEOGEO ROM-cart:1992/04/28
                                                                                                NEOGEO CD:1994/09/09
    NINJA COMMANDO                                              Shooter         ADK             MVS Cartridge:1992/04/30
                                                                                                NEOGEO ROM-cart:1992/05/29
                                                                                                NEOGEO CD:1994/10/31
    KING OF THE MONSTERS 2                                      Fighting        SNK             MVS Cartridge:1992/05/25
                                                                                                NEOGEO ROM-cart:1992/06/19
                                                                                                NEOGEO CD:1994/09/09
    ANDRO DUNOS                                                 Shooter         Visco           MVS Cartridge:1992/06/15
                                                                                                NEOGEO ROM-cart:1992/07/17
    WORLD HEROES                                                Fighting        ADK             MVS Cartridge:1992/07/28
                                                                                                NEOGEO ROM-cart:1992/09/11
                                                                                                NEOGEO CD:1995/03/17
    ART OF FIGHTING                                             Fighting        SNK             MVS Cartridge:1992/09/24
                                                                                                NEOGEO ROM-cart:1992/12/11
                                                                                                NEOGEO CD:1994/09/09
    VIEWPOINT                                                   Shooter         Sammy           MVS Cartridge:1992/11/20
                                                                                                NEOGEO ROM-cart:1992/12/11
                                                                                                NEOGEO CD:1995/02/25
    FATAL FURY 2                                                Fighting        SNK             MVS Cartridge:1992/12/10
                                                                                                NEOGEO ROM-cart:1993/03/05
                                                                                                NEOGEO CD:1994/09/09
    SUPER SIDEKICKS                                             Sports          SNK             MVS Cartridge:1992/12/14
                                                                                                NEOGEO ROM-cart:1993/02/19
                                                                                                NEOGEO CD:1995/03/31
    SENGOKU 2                                                   Action          SNK             MVS Cartridge:1993/02/18
                                                                                                NEOGEO ROM-cart:1993/04/09
                                                                                                NEOGEO CD:1995/03/17
    3 COUNT BOUT                                                Fighting        SNK             MVS Cartridge:1993/03/25
                                                                                                NEOGEO ROM-cart:1993/04/23
                                                                                                NEOGEO CD:1995/04/21
    WORLD HEROES 2                                              Fighting        ADK             MVS Cartridge:1993/04/26
                                                                                                NEOGEO ROM-cart:1993/06/04
                                                                                                NEOGEO CD:1995/04/14
    SAMURAI SHODOWN                                             Fighting        SNK             MVS Cartridge:1993/07/07
                                                                                                NEOGEO ROM-cart:1993/08/11
                                                                                                NEOGEO CD:1994/09/09
    FATAL FURY SPECIAL                                          Fighting        SNK             MVS Cartridge:1993/09/16
                                                                                                NEOGEO ROM-cart:1993/12/22
                                                                                                NEOGEO CD:1994/09/09
    SPINMASTER                                                  Sideview Action Data East       MVS Cartridge:1993/12/16
                                                                                                NEOGEO ROM-cart:1994/02/18
    ART OF FIGHTING 2                                           Fighting        SNK             MVS Cartridge:1994/02/03
                                                                                                NEOGEO ROM-cart:1994/03/11
                                                                                                NEOGEO CD:1994/09/09
    WINDJAMMERS                                                 Sports          Data East       MVS Cartridge:1994/02/17
                                                                                                NEOGEO ROM-cart:1994/04/08
                                                                                                NEOGEO CD:1995/01/20
    KARNOV'S REVENGE                                            Fighting        Data East       MVS Cartridge:1994/03/17
                                                                                                NEOGEO ROM-cart:1994/04/28
                                                                                                NEOGEO CD:1994/12/22
    SUPER SIDEKICKS 2                                           Sports          SNK             MVS Cartridge:1994/04/19
                                                                                                NEOGEO ROM-cart:1994/05/27
                                                                                                NEOGEO CD:1994/09/09
    WORLD HEROES 2 JET                                          Fighting        ADK             MVS Cartridge:1994/04/26
                                                                                                NEOGEO ROM-cart:1994/06/10
                                                                                                NEOGEO CD:1994/11/11
    TOP HUNTER                                                  Action          SNK             MVS Cartridge:1994/05/18
                                                                                                NEOGEO ROM-cart:1994/06/24
                                                                                                NEOGEO CD:1994/09/29
    GURURIN                                                     Puzzle          Face            MVS Cartridge:1994/05/25
    FIGHT FEVER                                                 Fighting        VICCOM          MVS Cartridge:1994/06/28
    JANSHIN DENSETSU: QUEST OF JONGMASTER                       Mahjong         Aicom           MVS Cartridge:1994/06/29
                                                                                                NEOGEO CD:1995/03/31
    AERO FIGHTERS 2                                             Topview Shooter Video System    MVS Cartridge:1994/07/18
                                                                                                NEOGEO ROM-cart:1994/08/26
                                                                                                NEOGEO CD:1994/09/29
    AGGRESSORS OF DARK KOMBAT                                   Fighting        ADK             MVS Cartridge:1994/07/26
                                                                                                NEOGEO ROM-cart:1994/08/26
                                                                                                NEOGEO CD:1995/01/13
    THE KING OF FIGHTERS '94                                    Fighting        SNK             MVS Cartridge:1994/08/25
                                                                                                NEOGEO ROM-cart:1994/10/01
                                                                                                NEOGEO CD:1994/11/02
    ZED BLADE                                                   Shooter         NMK             MVS Cartridge:1994/09/13
    POWER SPIKES II                                             Sports          Video System    MVS Cartridge:1994/10/19
                                                                                                NEOGEO CD:1995/03/18
    SAMURAI SHODOWN II                                          Fighting        SNK             MVS Cartridge:1994/10/28
                                                                                                NEOGEO ROM-cart:1994/12/02
                                                                                                NEOGEO CD:1994/12/15
    STREET HOOP                                                 Sports          Data East       MVS Cartridge:1994/12/08
                                                                                                NEOGEO ROM-cart:1994/12/09
                                                                                                NEOGEO CD:1995/01/20
    PUZZLE BOBBLE                                               Puzzle          TAITO           MVS Cartridge:1994/12/21
                                                                                                NEOGEO CD:1995/05/02
    SUPER VOLLEY '94                                            Sports          TAITO           MVS Cartridge:1994
    BOMBERMAN: PANIC BOMBER                                     Puzzle          Eighting        MVS Cartridge:1995/01/18
    GALAXY FIGHT: UNIVERSAL WARRIORS                            Fighting        Sunsoft         MVS Cartridge:1995/01/24
                                                                                                NEOGEO ROM-cart:1995/02/25
                                                                                                NEOGEO CD:1995/04/21
    QUIZ KING OF FIGHTERS                                       Quiz            Saurus          MVS Cartridge:1995/02/01
                                                                                                NEOGEO ROM-cart:1995/03/10
                                                                                                NEOGEO CD:1995/04/07
    DOUBLE DRAGON                                               Fighting        Technos         MVS Cartridge:1995/03/03
                                                                                                NEOGEO ROM-cart:1995/03/31
                                                                                                NEOGEO CD:1995/06/02
    SUPER SIDEKICKS 3                                           Sports          SNK             MVS Cartridge:1995/03/07
                                                                                                NEOGEO ROM-cart:1995/04/07
                                                                                                NEOGEO CD:1995/06/23
    FATAL FURY 3                                                Fighting        SNK             MVS Cartridge:1995/03/27
                                                                                                NEOGEO ROM-cart:1995/04/21
                                                                                                NEOGEO CD:1995/04/28
    SAVAGE REIGN                                                Fighting        SNK             MVS Cartridge:1995/04/25
                                                                                                NEOGEO ROM-cart:1995/03/10
                                                                                                NEOGEO CD:1995/06/16
    CROSSED SWORDS II                                           Action          ADK             NEOGEO CD:1995/05/02
    WORLD HEROES PERFECT                                        Fighting        ADK             MVS Cartridge:1995/05/25
                                                                                                NEOGEO ROM-cart:1995/06/30
                                                                                                NEOGEO CD:1995/07/21
    FAR EAST OF EDEN: KABUKI KLASH                              Fighting        Hudson Soft     MVS Cartridge:1995/06/20
                                                                                                NEOGEO ROM-cart:1995/07/28
                                                                                                NEOGEO CD:1995/11/24
    THE KING OF FIGHTERS '95                                    Fighting        SNK             MVS Cartridge:1995/07/25
                                                                                                NEOGEO ROM-cart:1995/09/01
                                                                                                NEOGEO CD:1995/09/29
    IDOL MAHJONG FINAL ROMANCE 2                                Mahjong         Video System    NEOGEO CD:1995/08/25
    PULSTAR                                                     Sidevi. Shooter Aicom           MVS Cartridge:1995/08/28
                                                                                                NEOGEO ROM-cart:1995/09/29
                                                                                                NEOGEO CD:1995/10/27
    VOLTAGE FIGHTER GOWCAIZER                                   Fighting        Technos         MVS Cartridge:1995/09/18
                                                                                                NEOGEO ROM-cart:1995/10/20
                                                                                                NEOGEO CD:1995/11/24
    STAKES WINNER                                               Action          Saurus          MVS Cartridge:1995/09/27
                                                                                                NEOGEO ROM-cart:1995/10/27
                                                                                                NEOGEO CD:1996/03/22
    SHOGI NO TATSUJIN - MASTER OF SYOUGI                        Japanese chess  ADK             MVS Cartridge:1995/09/28
                                                                                                NEOGEO ROM-cart:1995/10/13
                                                                                                NEOGEO CD:1995/10/20
    AERO FIGHTERS 3                                             Topview Action  Video System    MVS Cartridge:1995/10/12
                                                                                                NEOGEO ROM-cart:1995/11/17
                                                                                                NEOGEO CD:1995/12/08
    ADK WORLD                                                   Variety         ADK             NEOGEO CD:1995/11/10
    SAMURAI SHODOWN III                                         Fighting        SNK             MVS Cartridge:1995/11/15
                                                                                                NEOGEO ROM-cart:1995/12/01
                                                                                                NEOGEO CD:1995/12/29
    CHIBI MARUKO-CHAN DELUXE QUIZ                               Variety         Takara          MVS Cartridge:1995/11/27
                                                                                                NEOGEO ROM-cart:1996/01/26
    PUZZLE DE PON!                                              Puzzle          Visco           MVS Cartridge:1995/11/28
    REAL BOUT FATAL FURY                                        Fighting        SNK             MVS Cartridge:1995/12/21
                                                                                                NEOGEO ROM-cart:1996/01/26
                                                                                                NEOGEO CD:1996/02/23
    NEO-GEO CD SPECIAL                                          Variety         SNK             NEOGEO CD:1995/12/22
    NEO TURF MASTERS                                            Sports          Nazca           MVS Cartridge:1996/01/29
                                                                                                NEOGEO ROM-cart:1996/03/01
                                                                                                NEOGEO CD:1996/05/03
    ART OF FIGHTING 3                                           Fighting        SNK             MVS Cartridge:1996/03/12
                                                                                                NEOGEO ROM-cart:1996/04/26
                                                                                                NEOGEO CD:1996/06/14
    MAGICAL DROP II                                             Puzzle          Data East       MVS Cartridge:1996/03/21
                                                                                                NEOGEO ROM-cart:1996/04/19
                                                                                                NEOGEO CD:1996/05/24
    OSHIDASHI JIN TRICK                                         Puzzle          ADK             NEOGEO CD:1996/03/22
    NEO DRIFT OUT                                               Racing          Visco           MVS Cartridge:1996/03/28
                                                                                                NEOGEO CD:1996/07/26
    METAL SLUG                                                  Action          Nazca           MVS Cartridge:1996/04/19
                                                                                                NEOGEO ROM-cart:1996/05/24
                                                                                                NEOGEO CD:1996/07/05
    OVER TOP                                                    Racing          ADK             MVS Cartridge:1996/04/26
                                                                                                NEOGEO ROM-cart:1996/06/07
                                                                                                NEOGEO CD:1996/07/26
    NINJA MASTER'S                                              Fighting        ADK             MVS Cartridge:1996/05/27
                                                                                                NEOGEO ROM-cart:1996/06/28
                                                                                                NEOGEO CD:1996/09/27
    RAGNAGARD                                                   Fighting        Saurus          MVS Cartridge:1996/06/13
                                                                                                NEOGEO ROM-cart:1996/07/26
                                                                                                NEOGEO CD:1996/08/23
    FUTSAL                                                      Sports          Saurus          NEOGEO CD:1996/07/19
    THE KING OF FIGHTERS '96                                    Fighting        SNK             MVS Cartridge:1996/07/30
                                                                                                NEOGEO ROM-cart:1996/09/27
                                                                                                NEOGEO CD:1996/10/25
    KIZUNA ENCOUNTER SUPER TAG BATTLE                           Fighting        SNK             MVS Cartridge:1996/09/20
                                                                                                NEOGEO ROM-cart:1996/11/08
    CHOUTETSU BURIKINGA                                         Shooter         Saurus          NEOGEO CD:1996/09/20
    STAKES WINNER 2                                             Real Jockey Act Saurus          MVS Cartridge:1996/09/24
                                                                                                NEOGEO ROM-cart:1996/12/13
    THE ULTIMATE 11                                             Sports          SNK             MVS Cartridge:1996/10/16
                                                                                                NEOGEO ROM-cart:1996/12/20
    SAMURAI SHODOWN IV                                          Fighting        SNK             MVS Cartridge:1996/10/25
                                                                                                NEOGEO ROM-cart:1996/11/29
                                                                                                NEOGEO CD:1996/12/27
    WAKU WAKU 7                                                 Fighting        Sunsoft         MVS Cartridge:1996/11/21
                                                                                                NEOGEO ROM-cart:1996/12/27
    TWINKLE STAR SPRITES                                        Shooter         ADK             MVS Cartridge:1996/11/25
                                                                                                NEOGEO ROM-cart:1997/01/31
                                                                                                NEOGEO CD:1997/02/21
    BREAKERS                                                    Fighting        Visco           MVS Cartridge:1996/12/17
                                                                                                NEOGEO ROM-cart:1997/03/21
                                                                                                NEOGEO CD:1997/04/25
    MONEY IDOL EXCHANGER                                        Puzzle          Face            MVS Cartridge:1997/01/15
    Real Bout FATAL FURY SPECIAL                                Fighting        SNK             MVS Cartridge:1997/01/28
                                                                                                NEOGEO ROM-cart:1997/02/28
                                                                                                NEOGEO CD:1997/03/03
    THE KING OF FIGHTERS '96 NEOGEO COLLECTION                  Variety         SNK             NEOGEO CD:1997/02/14
    MAGICAL DROP III                                            Puzzle          Data East       MVS Cartridge:1997/02/25
                                                                                                NEOGEO ROM-cart:1997/04/25
    NEO BOMBERMAN                                               Action          Hudson Soft     MVS Cartridge:1997/05/01
    NEO MR.DO!                                                  Action          Visco           MVS Cartridge:1997/06/26
    SHINSETSU SAMURAI SHODOWN BUSHIDO RETSUDEN                  Role-playing    SNK             NEOGEO CD:1997/06/27
    THE KING OF FIGHTERS '97                                    Fighting        SNK             MVS Cartridge:1997/07/28
                                                                                                NEOGEO ROM-cart:1997/09/25
                                                                                                NEOGEO CD:1997/10/30
    UCCHAN NANCHAN NO HONO NO CHALLENGER ULTRA DENRYU IRAIRABOU Action          Saurus          MVS Cartridge:1997/08/25
    SHOCK TROOPERS                                              Shooter         Saurus          MVS Cartridge:1997/11/11
    THE LAST BLADE                                              Fighting        SNK             MVS Cartridge:1997/12/05
                                                                                                NEOGEO ROM-cart:1998/01/29
                                                                                                NEOGEO CD:1998/03/26
    BLAZING STAR                                                Shooter         Yumekobo        MVS Cartridge:1998/01/19
                                                                                                NEOGEO ROM-cart:1998/02/26
    METAL SLUG 2                                                Action          SNK             MVS Cartridge:1998/02/23
                                                                                                NEOGEO ROM-cart:1998/04/02
                                                                                                NEOGEO CD:1998/06/25
    REAL BOUT FATAL FURY 2                                      Fighting        SNK             MVS Cartridge:1998/03/20
                                                                                                NEOGEO ROM-cart:1998/04/29
                                                                                                NEOGEO CD:1998/07/23
    NEOGEO CUP '98                                              Sports          SNK             MVS Cartridge:1998/05/28
                                                                                                NEOGEO ROM-cart:1998/07/30
    BREAKERS REVENGE                                            Fighting        Visco           MVS Cartridge:1998/07/03
                                                                                                NEOGEO ROM-cart:
    THE KING OF FIGHTERS '98                                    Fighting        SNK             MVS Cartridge:1998/07/23
                                                                                                NEOGEO ROM-cart:1998/09/23
                                                                                                NEOGEO CD:1998/12/23
    SHOCK TROOPERS 2nd Squad                                    Action Shooter  Saurus          MVS Cartridge:1998/11/06
                                                                                                NEOGEO ROM-cart:1999/06/24
    THE LAST BLADE 2                                            Fighting        SNK             MVS Cartridge:1998/11/25
                                                                                                NEOGEO ROM-cart:1999/01/28
                                                                                                NEOGEO CD:1999/02/27
    FLIP SHOT                                                   Action          Visco           MVS Cartridge:1998/12/08
    METAL SLUG X                                                Action          SNK             MVS Cartridge:1999/03/19
                                                                                                NEOGEO ROM-cart:1999/05/27
    CAPTAIN TOMADAY                                             Shooter         Visco           MVS Cartridge:1999/05/27
    THE KING OF FIGHTERS '99                                    Fighting        SNK             MVS Cartridge:1999/07/22
                                                                                                NEOGEO ROM-cart:1999/09/23
                                                                                                NEOGEO CD:1999/12/02
    PREHISTORIC ISLE 2                                          Shooter         Yumekobo        MVS Cartridge:1999/09/27
    GAROU: MARK OF THE WOLVES                                   Fighting        SNK             MVS Cartridge:1999/11/26
                                                                                                NEOGEO ROM-cart:2000/02/25
    STRIKERS 1945 PLUS                                          Shooter         Psikyo          MVS Cartridge:1999/12/24
    METAL SLUG 3                                                Action Shooter  SNK             MVS Cartridge:2000/03/23
                                                                                                NEOGEO ROM-cart:2000/06/01
    THE KING OF FIGHTERS 2000                                   Fighting        SNK             MVS Cartridge:2000/07/26
                                                                                                NEOGEO ROM-cart:2000/12/21
    NIGHTMARE IN THE DARK                                       Horror Action   Gavaking        MVS Cartridge:2001
    ZUPAPA!                                                     Comical Action  Video System    MVS Cartridge:2001
    SENGOKU 3                                                   Action          SNK PLAYMORE    MVS Cartridge:2001/07/18
                                                                                                NEOGEO ROM-cart:2001/10/25
    THE KING OF FIGHTERS 2001                                   Fighting        SNK PLAYMORE    MVS Cartridge:2001/11/15
                                                                                                NEOGEO ROM-cart:2002/03/14
    METAL SLUG 4                                                Action Shooter  SNK PLAYMORE    MVS Cartridge:2002/03/27
                                                                                                NEOGEO ROM-cart:2002/06/13
    RAGE OF THE DRAGONS                                         Fighting        Evoga           MVS Cartridge:2002/06/06
                                                                                                NEOGEO ROM-cart:2002/09/26
    THE KING OF FIGHTERS 2002                                   Fighting        SNK PLAYMORE    MVS Cartridge:2002/10/10
                                                                                                NEOGEO ROM-cart:2002/12/19
    POWER INSTINCT MATRIMELEE                                   Fighting        ATLUS/NOISE FA. MVS Cartridge:2003/03/20
                                                                                                NEOGEO ROM-cart:2003/05/29
    SNK VS. CAPCOM: SVC CHAOS                                   Fighting        SNK PLAYMORE    MV-0:2003/07/24
                                                                                                NEOGEO ROM-cart:2003/11/13
    SAMURAI SHODOWN V                                           Fighting        SNK P/Yuki Ent  MVS Cartridge:2003/10/10
                                                                                                NEOGEO ROM-cart:2003/12/11
    METAL SLUG 5                                                Action Shooter  SNK PLAYMORE    MV-0:2003/11/14
                                                                                                NEOGEO ROM-cart:2004/02/19
    THE KING OF FIGHTERS 2003                                   Fighting        SNK PLAYMORE    MV-0:2003/12/12
                                                                                                NEOGEO ROM-cart:2004/03/18
    POCHI & NYAA                                                Puzzle          Aiky            MVS Cartridge:2003/12/24
    SAMURAI SHODOWN V SPECIAL                                   Fighting        SNK P/Yuki Ent  MVS Cartridge:2004/04/22
                                                                                                NEOGEO ROM-cart:2004/07/15


    Neo-Geo game PCB infos:
    =======================

    The Neo-Geo games for AES (home) and MVS (arcade) systems are cartridge based.

    Each cartridge consists of two PCBs: CHA and PROG.
    .CHA PCB contains gfx data ('C' - rom), text layer data ('S' - rom) and sound driver ('M' - rom).
    .PROG PCB contains sample data ('V' - rom) and program code ('P' - rom).

    On most PCBs various custom/protection chips can also be found:
    (Custom chip detail information (modified) from: http://wiki.neogeodev.org)

    CHA:
    . NEO-273  (C and S-ROM address latch)
    . NEO-CMC 90G06CF7042 (NEO-273 logic / NEO-ZMC logic / C-ROM decryption / C and S-ROM multiplexer / S-ROM bankswitching)
    . NEO-CMC 90G06CF7050 (NEO-273 logic / NEO-ZMC logic / C-ROM decryption / M-ROM decryption / C and S-ROM multiplexer / S-ROM bankswitching)
    . NEO-ZMC  (Z80 memory controller)
    . NEO-ZMC2 (Z80 memory controller / Tile serializer)
    . PRO-CT0  (C-ROM serializer and multiplexer?; used on early AES-CHA boards)
    . SNK-9201 (C-ROM serializer and multiplexer?; used on early AES-CHA boards)

    PROG:
    . 0103 (QFP144) (Only found on Metal Slug X NEO-MVS PROGEOP board; function unknown)
    . ALTERA   (EPM7128SQC100-15) (P-ROM protection chip used for KOF98 NEO-MVS PROGSF1 board and Metal Slug X NEO-MVS PROGEOP board)
    . NEO-COMA (Microcontroller; used for MULTI PLAY MODE, boards and sets see below)
    . NEO-PCM2 (SNK 1999) (PCM functionality / V-ROM decryption / P-ROM decoding and bankswitching)
    . NEO-PCM2 (PLAYMORE 2002) (PCM functionality / V-ROM decryption / P-ROM decoding and bankswitching)
    . NEO-PVC  (P-ROM decryption and bankswitching)
    . NEO-SMA  (P-ROM decryption and bankswitching / RNG / Storage of 256kb game data)
    . PCM      (ADPCM bus latches / V-ROM multiplexer)
    . PRO-CT0  (On PROG board used for P-ROM protection -> Fatal Fury 2)
    . SNK-9201 (On PROG board used for P-ROM protection -> Fatal Fury 2)


    Known PCBs:
    ============

    MVS CHA:
    -- SNK --
    . NEO-MVS CHA-32
    . NEO-MVS CHA-8M
    . NEO-MVS CHA42G
    . NEO-MVS CHA42G-1
    . NEO-MVS CHA 42G-2
    . NEO-MVS CHA 42G-3
    . NEO-MVS CHA42G-3B
    . NEO-MVS CHA256
    . NEO-MVS CHA256B
    . NEO-MVS CHA512Y
    . NEO-MVS CHAFIO (1999.6.14) - used with NEO-CMC 90G06C7042 or NEO-CMC 90G06C7050
    . MVS CHAFIO REV1.0 (KOF-2001)
    . NEO-MVS CHAFIO (SNK 2002) - MADE IN KOREA
    -- SNKPLAYMORE --
    . NEO-MVS CHAFIO (2003.7.24) - used only with NEO-CMC 90G06C7050

    MVS PROG:
    -- SNK --
    . NEO-MVS PROG-NAM
    . NEO-MVS PROG-HERO
    . NEO-MVS PROG-EP
    . NEO-MVS PROG-8MB
    . NEO-MVS PROGEP8M
    . NEO-MVS PROG8M42
    . NEO-MVS PROG16
    . NEO-MVS PROG42G
    . NEO-MVS PROG42G-COM
    . NEO-MVS PROG42G-1
    . NEO-MVS PROG-G2
    . NEO-MVS PROG 4096
    . NEO-MVS PROG 4096 B
    . NEO-MVS PROGGSC
    . NEO-MVS PROGSM
    . NEO-MVS PROGSS3
    . NEO-MVS PROGTOP
    . NEO-MVS PROGSF1 (1998.6.17)
    . NEO-MVS PROGEOP (1999.2.2)
    . NEO-MVS PROGLBA (1999.4.12) - LBA-SUB (2000.2.24)
    . NEO-MVS PROGBK1 (1994)
    . NEO-MVS PROGBK1 (2001)
    . NEO-MVS PROGBK2 (2000.3.21) - used with NEO-PCM2 (1999 SNK) or NEO-PCM2 (2002 PLAYMORE)
    . MVS PROGBK2 REV1.0 (KOF-2001)
    . NEO-MVS PROGBK2 (SNK 2002) - MADE IN KOREA
    -- SNKPLAYMORE --
    . NEO-MVS PROGBK2R (2003.8.26) - NEO-HYCS (2003.9.29)
    . NEO-MVS PROGBK3R (2003.9.2) - NEO-HYCS (2003.9.29)
    . NEO-MVS PROGBK3S (2003.10.1)
    . NEO-MVS PROGBK2S (2003.10.18)


    AES CHA:
    -- SNK --
    . NEO-AEG CHA-32
    . NEO-AEG CHA-8M
    . NEO-AEG CHA42G
    . NEO-AEG CHA42G-1
    . NEO-AEG CHA42G-2B
    . NEO-AEG CHA42G-3
    . NEO-AEG CHA42G-4
    . NEO-AEG CHA256
    . NEO-AEG CHA256[B]
    . NEO-AEG CHA256RY
    . NEO-AEG CHA512Y
    . NEO-AEG CHAFIO (1999.8.10) - used with NEO-CMC 90G06C7042 or NEO-CMC 90G06C7050
    -- SNKPLAYMORE --
    . NEO-AEG CHAFIO (2003.7.24) - used only with NEO-CMC 90G06C7050

    AES PROG:
    -- SNK --
    . NEO-AEG PROG-NAM
    . NEO-AEG PROG-HERO
    . NEO-AEG PROG-4A
    . NEO-AEG PROG-4B
    . NEO-AEG PROG 8M42
    . NEO-AEG PROG B
    . NEO-AEG PROG16
    . NEO-AEG PROG42G
    . NEO-AEG PROG42G-COM
    . NEO-AEG PROG42G-1
    . NEO-AEG PROG-G2
    . NEO-AEG PROG4096 B
    . NEO-AEG PROGGS
    . NEO-AEG PROGTOP2
    . NEO-AEG PROGLBA (1999.7.6)
    . NEO-AEG PROGRK
    . NEO-AEG PROGRKB
    . NEO-AEG PROGBK1Y
    . NEO-AEG PROGBK1F
    -- PLAYMORE --
    . NEO-AEG PROGBK2 (2002.4.1) - used with NEO-PCM2 (1999 SNK) or NEO-PCM2 (2002 PLAYMORE)
    -- SNKPLAYMORE --
    . NEO-AEG PROGBK3R (2003.8.29) - NEO-HYCS (2003.9.29)
    . NEO-AEG PROGBK3S (2003.10.6)
    . NEO-AEG PROGBK2S (2003.10.16)


    Cartridge colours:
    ==================

    MVS cartridges were produced in different colours.

    Known cartridge colours:
    . Black
    . Blue
    . Green
    . Grey
    . Red
    . Transparent
    . Transparent Blue
    . Transparent Green
    . White
    . Yellow

    The above listed only covers SNK / PLAYMORE / SNKPLAYMORE PCBs. There also exists a
    wide range of 'bootleg' PCBs.


    Neo-Geo game PCB infos by Johnboy



    MVS cart pinout
    ===============

    Kindly submitted by Apollo69 (apollo69@columbus.rr.com)
    =================================================================
                CTRG1                            CTRG2
    =================================================================
         GND = 01A | 01B = GND            GND = 01A | 01B = GND
         GND = 02A | 02B = GND            GND = 02A | 02B = GND
          P0 = 03A | 03B = P1             GND = 03A | 03B = GND
          P2 = 04A | 04B = P3             GND = 04A | 04B = GND
          P4 = 05A | 05B = P5              D0 = 05A | 05B = A1
          P6 = 06A | 06B = P7              D1 = 06A | 06B = A2
          P8 = 07A | 07B = P9              D2 = 07A | 07B = A3
         P10 = 08A | 08B = P11             D3 = 08A | 08B = A4
         P12 = 09A | 09B = P13             D4 = 09A | 09B = A5
         P14 = 10A | 10B = P15             D5 = 10A | 10B = A6
         P16 = 11A | 11B = P17             D6 = 11A | 11B = A7
         P18 = 12A | 12B = P19             D7 = 12A | 12B = A8
         P20 = 13A | 13B = P21             D8 = 13A | 13B = A9
         P22 = 14A | 14B = P23             D9 = 14A | 14B = A10
       PCK1B = 15A | 15B = 24M            D10 = 15A | 15B = A11
       PCK2B = 16A | 16B = 12M            D11 = 16A | 16B = A12
         2H1 = 17A | 17B = 8M             D12 = 17A | 17B = A13
         CA4 = 18A | 18B = RESET          D13 = 18A | 18B = A14
         CR0 = 19A | 19B = CR1            D14 = 19A | 19B = A15
         CR2 = 20A | 20B = CR3            D15 = 20A | 20B = A16
         CR4 = 21A | 21B = CR5            R/W = 21A | 21B = A17
         CR6 = 22A | 22B = CR7             AS = 22A | 22B = A18
         CR8 = 23A | 23B = CR9         ROMOEU = 23A | 23B = A19
        CR10 = 24A | 24B = CR11        ROMOEL = 24A | 24B = 68KCLKB
        CR12 = 25A | 25B = CR13       PORTOEU = 25A | 25B = ROMWAIT
        CR14 = 26A | 26B = CR15       PORTOEL = 26A | 26B = PWAIT0
        CR16 = 27A | 27B = CR17       PORTWEU = 27A | 27B = PWAIT1
        CR18 = 28A | 28B = CR19       PORTWEL = 28A | 28B = PDTACT
         VCC = 29A | 29B = VCC            VCC = 29A | 29B = VCC
         VCC = 30A | 30B = VCC            VCC = 30A | 30B = VCC
         VCC = 31A | 31B = VCC            VCC = 31A | 31B = VCC
         VCC = 32A | 32B = VCC            VCC = 32A | 32B = VCC
        CR20 = 33A | 33B = CR21      PORTADRS = 33A | 33B = 4MB
        CR22 = 34A | 34B = CR23            NC = 34A | 34B = ROMOE
        CR24 = 35A | 35B = CR25            NC = 35A | 35B = RESET
        CR26 = 36A | 36B = CR27            NC = 36A | 36B = NC
        CR28 = 37A | 37B = CR29            NC = 37A | 37B = NC
        CR30 = 38A | 38B = CR31            NC = 38A | 38B = NC
          NC = 39A | 39B = FIX00           NC = 39A | 39B = NC
          NC = 40A | 40B = FIX01           NC = 40A | 40B = NC
          NC = 41A | 41B = FIX02           NC = 41A | 41B = SDPAD0
     SYSTEMB = 42A | 42B = FIX03      SYSTEMB = 42A | 42B = SDPAD1
        SDA0 = 43A | 43B = FIX04        SDPA8 = 43A | 43B = SDPAD2
        SDA1 = 44A | 44B = FIX05        SDPA9 = 44A | 44B = SDPAD3
        SDA2 = 45A | 45B = FIX06       SDPA10 = 45A | 45B = SDPAD4
        SDA3 = 46A | 46B = FIX07       SDPA11 = 46A | 46B = SDPAD5
        SDA4 = 47A | 47B = SDRD0       SDPMPX = 47A | 47B = SDPAD6
        SDA5 = 48A | 48B = SDRD1        SDPOE = 48A | 48B = SDPAD7
        SDA6 = 49A | 49B = SDROM        SDRA8 = 49A | 49B = SDRA00
        SDA7 = 50A | 50B = SDMRD        SDRA9 = 50A | 50B = SDRA01
        SDA8 = 51A | 51B = SDDO        SDRA20 = 51A | 51B = SDRA02
        SDA9 = 52A | 52B = SDD1        SDRA21 = 52A | 52B = SDRA03
       SDA10 = 53A | 53B = SDD2        SDRA22 = 53A | 53B = SDRA04
       SDA11 = 54A | 54B = SDD3        SDRA23 = 54A | 54B = SDRA05
       SDA12 = 55A | 55B = SDD4        SDRMPX = 55A | 55B = SDRA06
       SDA13 = 56A | 56B = SDD5         SDROE = 56A | 56B = SDRA07
       SDA14 = 57A | 57B = SDD6           GND = 57A | 57B = GND
       SDA15 = 58A | 58B = SDD7           GND = 58A | 58B = GND
         GND = 59A | 59B = GND            GND = 59A | 59B = GND
         GND = 60A | 60B = GND            GND = 60A | 60B = GND

    CTRG1 (CHA)  = Contains gfx data ('C' - rom), text layer data ('S' - rom) and sound driver ('M' - rom)
    CTRG2 (PROG) = Contains sample data ('V' - rom) and program code ('P' - rom)

    NOTE: On CTRG2-B, The "A" lines start at "A1". If you trace this on an
    actual cart, you will see that this is actually "A0" (A0 - A18).

    These are from a very hard to read copy of the schematics, so
    I hope that I got the pin names correct.

    Apollo69 10/19/99


****************************************************************************/

	/* Neo-Geo bios */

/*
    These are the known Bios Roms, Set options.bios to the one you want.

    The Universe bios roms are supported because they're now used on enough PCBs
    to be considered 'in active arcade use' rather than just homebrew hacks.
    Some may be missing, there have been multiple CRCs reported for the same
    revision in some cases (the Universe bios has an option for entering / displaying
    a serial number; these should be noted as such if they're added).

    The 'japan-hotel' BIOS is a dump of an MVS which could be found in some japanese
    hotels. it is a custom MVS mobo which uses MVS carts but it hasn't jamma
    connector and it's similar to a console with a coin mechanism, so it's a sort
    of little coin op console installed in hotels.

    The sp-45.sp1 bios is the latest 'ASIA' revision. Japan-j3.bin is the latest 'JAPAN'
    revision. Both of them are also used in the sp-4x.sp1 bios of the Jamma PCB boards.

    The current Neo-Geo MVS system set (SFIX/SM1/000-LO) used is from a NEO-MVH MV1FS board.
    Other boards (MV1xx / MV2x / MV4x /MV6x) other system sets?

    Zoom ROM (LO)    128K   TC531000CP      1x 128Kx8   Zoom look-up table ROM
    Fix ROM (SFIX)   128K   27C1000         1x 128Kx8   Text layer graphics ROM
    Sound ROM (SM1)  128K   27C1000/23C1000 1x 128Kx8   Z80 program ROM

*/

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

#define NEOGEO_BIOS \
	ROM_REGION16_BE( 0x80000, "mainbios", 0 )													\
	ROM_SYSTEM_BIOS( 0, "euro",        "Europe MVS (Ver. 2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "sp-s2.sp1",    0x00000, 0x020000, CRC(9036d879) SHA1(4f5ed7105b7128794654ce82b51723e16e389543) ) /* Europe, 1 Slot, has also been found on 2 Slot and 4 Slot (the old hacks were designed for this one) */ \
	ROM_SYSTEM_BIOS( 1, "euro-s1",     "Europe MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "sp-s.sp1",     0x00000, 0x020000, CRC(c7f2fa45) SHA1(09576ff20b4d6b365e78e6a5698ea450262697cd) ) /* Europe, 4 Slot */ \
	ROM_SYSTEM_BIOS( 2, "us",          "US MVS (Ver. 2?)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "usa_2slt.bin", 0x00000, 0x020000, CRC(e72943de) SHA1(5c6bba07d2ec8ac95776aa3511109f5e1e2e92eb) ) /* US, 2 Slot */ \
	ROM_SYSTEM_BIOS( 3, "us-e",        "US MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "sp-e.sp1",     0x00000, 0x020000, CRC(2723a5b5) SHA1(5dbff7531cf04886cde3ef022fb5ca687573dcb8) ) /* US, 6 Slot (V5?) */ \
	ROM_SYSTEM_BIOS( 4, "asia",        "Asia MVS (Ver. 3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "asia-s3.rom",  0x00000, 0x020000, CRC(91b64be3) SHA1(720a3e20d26818632aedf2c2fd16c54f213543e1) ) /* Asia */ \
	ROM_SYSTEM_BIOS( 5, "japan",       "Japan MVS (Ver. 3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "vs-bios.rom",  0x00000, 0x020000, CRC(f0e8f27d) SHA1(ecf01eda815909f1facec62abf3594eaa8d11075) ) /* Japan, Ver 6 VS Bios */ \
	ROM_SYSTEM_BIOS( 6, "japan-s2",    "Japan MVS (Ver. 2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6, "sp-j2.sp1",    0x00000, 0x020000, CRC(acede59c) SHA1(b6f97acd282fd7e94d9426078a90f059b5e9dd91) ) /* Japan, Older */ \
	ROM_SYSTEM_BIOS( 7, "japan-s1",    "Japan MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7, "sp1.jipan.1024",0x00000, 0x020000, CRC(9fb0abe4) SHA1(18a987ce2229df79a8cf6a84f968f0e42ce4e59d) ) /* Japan, Older */ \
	ROM_SYSTEM_BIOS( 8, "mv1c",        "NEO-MVH MV1C" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8, "sp-45.sp1",0x00000, 0x080000, CRC(03cc9f6a) SHA1(cdf1f49e3ff2bac528c21ed28449cf35b7957dc1) ) /* Latest Asia bios */ \
	ROM_SYSTEM_BIOS( 9, "japan-j3",    "Japan MVS (J3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 9, "japan-j3.bin",0x00000, 0x020000, CRC(dff6d41f) SHA1(e92910e20092577a4523a6b39d578a71d4de7085) ) /* Latest Japan bios; correct chip label unknown */ \
	ROM_SYSTEM_BIOS( 10, "japan-hotel","Custom Japanese Hotel" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 10, "sp-1v1_3db8c.bin",0x00000, 0x020000, CRC(162f0ebe) SHA1(fe1c6dd3dfcf97d960065b1bb46c1e11cb7bf271) ) /* 'rare MVS found in japanese hotels' shows v1.3 in test mode */ \
	ROM_SYSTEM_BIOS( 11, "uni-bios_2_3","Universe Bios (Hack, Ver. 2.3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 11, "uni-bios_2_3.rom",  0x00000, 0x020000, CRC(27664eb5) SHA1(5b02900a3ccf3df168bdcfc98458136fd2b92ac0) ) /* Universe Bios v2.3 (hack) */ \
	ROM_SYSTEM_BIOS( 12, "uni-bios_2_3o","Universe Bios (Hack, Ver. 2.3, older?)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 12, "uni-bios_2_3o.rom",  0x00000, 0x020000, CRC(601720ae) SHA1(1b8a72c720cdb5ee3f1d735bbcf447b09204b8d9) ) /* Universe Bios v2.3 (hack) alt version, withdrawn? */ \
	ROM_SYSTEM_BIOS( 13, "uni-bios_2_2","Universe Bios (Hack, Ver. 2.2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 13, "uni-bios_2_2.rom",  0x00000, 0x020000, CRC(2d50996a) SHA1(5241a4fb0c63b1a23fd1da8efa9c9a9bd3b4279c) ) /* Universe Bios v2.2 (hack) */ \
	ROM_SYSTEM_BIOS( 14, "uni-bios_2_1","Universe Bios (Hack, Ver. 2.1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 14, "uni-bios_2_1.rom",  0x00000, 0x020000, CRC(8dabf76b) SHA1(c23732c4491d966cf0373c65c83c7a4e88f0082c) ) /* Universe Bios v2.1 (hack) */ \
	ROM_SYSTEM_BIOS( 15, "uni-bios_2_0","Universe Bios (Hack, Ver. 2.0)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 15, "uni-bios_2_0.rom",  0x00000, 0x020000, CRC(0c12c2ad) SHA1(37bcd4d30f3892078b46841d895a6eff16dc921e) ) /* Universe Bios v2.0 (hack) */ \
	ROM_SYSTEM_BIOS( 16, "uni-bios_1_3","Universe Bios (Hack, Ver. 1.3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 16, "uni-bios_1_3.rom",  0x00000, 0x020000, CRC(b24b44a0) SHA1(eca8851d30557b97c309a0d9f4a9d20e5b14af4e) ) /* Universe Bios v1.3 (hack) */ \
	ROM_SYSTEM_BIOS( 17, "uni-bios_1_2","Universe Bios (Hack, Ver. 1.2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 17, "uni-bios_1_2.rom",  0x00000, 0x020000, CRC(4fa698e9) SHA1(682e13ec1c42beaa2d04473967840c88fd52c75a) ) /* Universe Bios v1.2 (hack) */ \
	ROM_SYSTEM_BIOS( 18, "uni-bios_1_2o","Universe Bios (Hack, Ver. 1.2, older)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 18, "uni-bios_1_2o.rom", 0x00000, 0x020000, CRC(e19d3ce9) SHA1(af88ef837f44a3af2d7144bb46a37c8512b67770) ) /* Universe Bios v1.2 (hack) alt version */ \
	ROM_SYSTEM_BIOS( 19, "uni-bios_1_1","Universe Bios (Hack, Ver. 1.1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 19, "uni-bios_1_1.rom",  0x00000, 0x020000, CRC(5dda0d84) SHA1(4153d533c02926a2577e49c32657214781ff29b7) ) /* Universe Bios v1.1 (hack) */ \
	ROM_SYSTEM_BIOS( 20, "uni-bios_1_0","Universe Bios (Hack, Ver. 1.0)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 20, "uni-bios_1_0.rom",  0x00000, 0x020000, CRC(0ce453a0) SHA1(3b4c0cd26c176fc6b26c3a2f95143dd478f6abf9) ) /* Universe Bios v1.0 (hack) */ \
//  ROM_SYSTEM_BIOS( 21, "debug",      "Debug MVS (Hack?)" )
//  ROM_LOAD16_WORD_SWAP_BIOS( 21, "neodebug.rom", 0x00000, 0x020000, CRC(698ebb7d) SHA1(081c49aa8cc7dad5939833dc1b18338321ea0a07) ) /* Debug (Development) Bios */
//  ROM_SYSTEM_BIOS( 22, "asia-aes",   "Asia AES" )
//  ROM_LOAD16_WORD_SWAP_BIOS( 22, "neo-epo.sp1", 0x00000, 0x020000, CRC(d27a71f1) SHA1(1b3b22092f30c4d1b2c15f04d1670eb1e9fbea07) ) /* AES Console (Asia?) Bios */
//  ROM_SYSTEM_BIOS( 23, "japan-aes",  "Japan AES" )
//  ROM_LOAD16_WORD_SWAP_BIOS( 23, "neo-po.sp1", 0x00000, 0x020000, CRC(16d0c132) SHA1(4e4a440cae46f3889d20234aebd7f8d5f522e22c) ) /* AES Console (Japan) Bios */

/*  Note you'll have to modify the last for lines of each block to use the extra bios roms,
    they're hacks / homebrew / console bios roms so MAME doesn't list them by default. */

#define NEO_BIOS_AUDIO_64K(name, hash)																			\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x20000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x10000, hash )																\
	ROM_RELOAD(          0x10000, 0x10000 )


#define NEO_BIOS_AUDIO_128K(name, hash) 																		\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x30000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x20000, hash )																\
	ROM_RELOAD(          0x10000, 0x20000 )


#define NEO_BIOS_AUDIO_256K(name, hash) 																		\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x50000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x40000, hash )																\
	ROM_RELOAD(          0x10000, 0x40000 )


#define NEO_BIOS_AUDIO_512K(name, hash) 																		\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x90000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x80000, hash )																\
	ROM_RELOAD(          0x10000, 0x80000 )

#define NEO_BIOS_AUDIO_ENCRYPTED_512K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 	 0x00000, 0x80000, hash ) \

#define NEO_BIOS_AUDIO_ENCRYPTED_256K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 	 0x00000, 0x40000, hash ) \

#define NEO_BIOS_AUDIO_ENCRYPTED_128K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 	 0x00000, 0x20000, hash ) \


#define NO_DELTAT_REGION


#define NEO_SFIX_32K(name, hash)																				\
	ROM_REGION( 0x20000, "fixed", 0 )												\
	ROM_LOAD( name,       0x000000, 0x08000, hash ) 															\
																												\
	ROM_REGION( 0x20000, "fixedbios", 0 )													\
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )		\
																												\
	ROM_Y_ZOOM


#define NEO_SFIX_64K(name, hash)																				\
	ROM_REGION( 0x20000, "fixed", 0 )												\
	ROM_LOAD( name,       0x000000, 0x10000, hash ) 															\
																												\
	ROM_REGION( 0x20000, "fixedbios", 0 )													\
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )		\
																												\
	ROM_Y_ZOOM


#define NEO_SFIX_128K(name, hash)																				\
	ROM_REGION( 0x20000, "fixed", 0 )												\
	ROM_LOAD( name,       0x000000, 0x20000, hash ) 															\
																												\
	ROM_REGION( 0x20000, "fixedbios", 0 )													\
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )		\
																												\
	ROM_Y_ZOOM


#define ROM_Y_ZOOM																								\
	ROM_REGION( 0x20000, "zoomy", 0 )														\
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )


/****************************************************************************/

	/* Official sets */

/*
    About supported sets:

    MVS carts (arcade) were released before the AES carts (home)
    The actual codepath taken depends entirely on the BIOS rom, not the roms in the cartridge, which (with
    a few exceptions) support both codepaths.

    The initial AES releases are therefore later revisions of the game, often with bug fixes over the
    initial MVS releases. It isn't uncommon for later production runs and bootlegs to use these newer sets,
    so all of them are supported in MAME.

    Likewise, because the MVS carts were released first (and were produced in higher numbers and generally
    have a lower cost) it's not uncommon for AES units to operate with converted MVS carts, so, with the
    exception of the sets that specifically lock out the AES mode* these sets are all equally suitable
    for MESS.
    * nitd, kof2001 (initial release has no AES code), and a number of the hacked bootlegs.

    The 'MVS ONLY RELEASE' tagged sets were not officially released for the AES (home) system.
    Information about this can be found at 'The NeoGeo Master List' (unofficial) - http://www.neo-geo.com
    and the official NeoGeo museum - http://neogeomuseum.snkplaymore.co.jp/english/index.php
    Several unofficial 'conversions' of these sets can be found across the internet.
    For completeness sake: Some of these have sets have been released for the CD system.


    M1 (sound driver) rom information:
    . Many 'M1' roms contain mirrored data (64k mirrored or 128k mirrored).
    . Found on several early sets (ID 0001 ~ 0045) and on the last sets (ID 0267 ~ 0272).
    . This caused some confusion and incorrect rom sizes.
    . Minimum 'M1' size is 1mbit, maximum size 4mbit.
    . The remaining 64k 'M1' are marked BAD_DUMP.


    S1 (text layer) rom information:
    . All 'S1' roms found on prom are 1mbit.
    . The remainig 64k 'S1' are marked BAD_DUMP.


    MULTI PLAY MODE:
    The NeoGeo has three games which support MULTI PLAY MODE (Riding Hero / League Bowling / Trash Rally).
    This allows you to 'link' 4 games (MVS) / 2 games (AES) using in game 'Multi-Play' option. To establish
    a link between the carts you have to connect the carts to each other by a communicator cable. The communicatior
    cable is a regular headphone cable with stereo pin jack. It has been reported that you can also 'link' MVS <-> AES.

    All three games use a special PROG board for MULTI PLAY MODE support:
    . Riding Hero    (AES - NEO-AEG PROG-HERO   / MVS NEO-MVS PROG-HERO)
    . League Bowling (AES - NEO-AEG PROG-HERO   / MVS NEO-MVS PROG-HERO)
    . Trash Rally    (AES - NEO-AEG PROG42G-COM / NEO-MVS PROG42G-COM)

    A HD6301V1P MCU on the above boards is used for establishing the 'link'. The MCU has a 4kb internal ROM which
    is not dumped.
    To use the MULTI PLAY MODE on your MVS you have to set the following hardware dips:
    HARD DIP SETTING  4   5   6
    CABINET 1:        OFF OFF ON
    CABINET 2:        OFF ON  ON
    CABINET 3:        ON  OFF ON
    CABINET 4:        ON  ON  ON

*/


/* entry for the base system driver */
ROM_START( neogeo )
	NEOGEO_BIOS

	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x90000, "audiocpu", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_Y_ZOOM

	ROM_REGION( 0x20000, "fixed", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION( 0x1000000, "ymsnd", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "ymsnd.deltat", ROMREGION_ERASEFF )

	ROM_REGION( 0x900000, "sprites", ROMREGION_ERASEFF )
ROM_END




/****************************************
 ID-0236
 . ??M-2360
 NEO-MVS PROGBK1 / NEO-MVS CHA512Y
****************************************/

ROM_START( irrmaze ) /* MVS ONLY RELEASE */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "236-p1.p1", 0x100000, 0x100000, CRC(4c2ff660) SHA1(4a0cbd09044648ff9ec67723729f16d422c34bda) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "236-s1.s1", CRC(5d1ca640) SHA1(40a9668a1742a44597a07ce72273d17119815637) )

	ROM_REGION16_BE( 0x20000, "mainbios", 0 )
	/* special BIOS with trackball support, we only have one Irritating Maze bios and thats asia */
	ROM_LOAD16_WORD_SWAP("236-bios.sp1", 0x00000, 0x020000, CRC(853e6b96) SHA1(de369cb4a7df147b55168fa7aaf0b98c753b735e) )

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "236-m1.m1", 0x00000, 0x20000, CRC(880a1abd) SHA1(905afa157aba700e798243b842792e50729b19a0) ) /* so overwrite it with the real thing */
	ROM_RELOAD( 0x10000, 0x20000 )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "236-v1.v1", 0x000000, 0x200000, CRC(5f89c3b4) SHA1(dc8fd561cf8dfdd41696dcf14ea8d2d0ac4eec4b) )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 )
	ROM_LOAD( "236-v2.v2", 0x000000, 0x100000, CRC(1e843567) SHA1(30d63887b4900571025b3077b9e41099a59c3ad9) )

	ROM_REGION( 0x0800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "236-c1.c1", 0x000000, 0x400000, CRC(c1d47902) SHA1(727001c34f979226fc8f581113ce2aaac4fc0d42) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "236-c2.c2", 0x000001, 0x400000, CRC(e15f972e) SHA1(6a329559c57a67be73a6733513b59e9e6c8d61cc) ) /* Plane 2,3 */
ROM_END

	/* Jamma PCB sets */


/****************************************
 ID-2680
 . MV-0 ????
 NEO-MVH MVOBR 2003.8.4
****************************************/

/* complete redump required */
ROM_START( ms5pcb ) /* Encrypted Set */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "268-p1r.p1", 0x000000, 0x400000, NO_DUMP )
	ROM_LOAD32_WORD_SWAP( "268-p2r.p2", 0x000002, 0x400000, NO_DUMP )

	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "268-m1.m1", 0x00000, 0x80000, CRC(4a5a6e0e) SHA1(df0f660f2465e1db7be5adfcaf5e88ad61a74a42) ) /* mask rom TC534000 */
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	/* Encrypted */

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "268-v1.v1", 0x000000, 0x1000000, CRC(8458afe5) SHA1(62b4c6e7db763e9ff2697bbcdb43dc5a56b48c68) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "268-c1.c1", 0x0000000, 0x1000000, BAD_DUMP CRC(802042e8) SHA1(ff028b65f60f0b51b255a380cc47ec19fdc0c0cf) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "268-c2.c2", 0x0000002, 0x1000000, BAD_DUMP CRC(3b89fb9f) SHA1(cbc0729aae961f683b105ec3e1cda58b3f985abc) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "268-c3.c3", 0x2000000, 0x1000000, BAD_DUMP CRC(0f3f59e3) SHA1(8cc751dc7d4e94864a9ce3346f23b8f011082fcc) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "268-c4.c4", 0x2000002, 0x1000000, BAD_DUMP CRC(3ad8435a) SHA1(b333c8993c9b4c4ea59450ad0a3560e0b28056bc) ) /* Plane 2,3 */
ROM_END

/****************************************
 ID-2690
 . MV-0 ????
 NEO-MVH MVO 2003.6.5
****************************************/

ROM_START( svcpcb ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "269-p1.p1", 0x000000, 0x2000000, CRC(432cfdfc) SHA1(19b40d32188a8bace6d2d570c6cf3d2f1e31e379) )

	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "269-m1.m1", 0x00000, 0x80000, CRC(f6819d00) SHA1(d3bbe09df502464f104e53501708ac6e2c1832c6) ) /* mask rom TC534000 */
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "269-v1.v1", 0x000000, 0x800000, CRC(c659b34c) SHA1(1931e8111ef43946f68699f8707334c96f753a1e) )
	ROM_LOAD( "269-v2.v1", 0x800000, 0x800000, CRC(dd903835) SHA1(e58d38950a7a8697bb22a1cc7a371ae6664ae8f9) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD( "269-c1.c1", 0x0000000, 0x2000000, CRC(1b608f9c) SHA1(4e70ad182da2ca18815bd3936efb04a06ebce01e) ) /* Plane 0,1 */
	ROM_LOAD( "269-c2.c1", 0x2000000, 0x2000000, CRC(5a95f294) SHA1(6123cc7b20b494076185d27c2ffea910e124b195) ) /* Plane 0,1 */
ROM_END

/****************************************
 ID-2690
 . MV-0 ????
 NEO-MVH MVOB 2003.7.9
****************************************/

ROM_START( svcpcba ) /* Encrypted Set, JAMMA PCB */
	/* alt PCB version, this one has the same program roms as the MVS set, and different GFX / Sound rom arrangements */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "269-p1a.p1", 0x000000, 0x400000, CRC(38e2005e) SHA1(1b902905916a30969282f1399a756e32ff069097)  )
	ROM_LOAD32_WORD_SWAP( "269-p2a.p1", 0x000002, 0x400000, CRC(6d13797c) SHA1(3cb71a95cea6b006b44cac0f547df88aec0007b7)  )

	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "269-m1.m1", 0x00000, 0x80000, CRC(f6819d00) SHA1(d3bbe09df502464f104e53501708ac6e2c1832c6) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "269-v1a.v1", 0x000000, 0x1000000, CRC(a6af4753) SHA1(ec4f61a526b707a7faec4653b773beb3bf3a17ba) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "269-c1a.c1", 0x0000000, 0x1000000, CRC(e64d2b0c) SHA1(0714198c400e5c273181e4c6f906b49e35fef75d) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "269-c2a.c2", 0x0000002, 0x1000000, CRC(249089c2) SHA1(1c0ca19e330efe1a74b2d35a1a9a8d61481e16a9) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "269-c3a.c3", 0x2000000, 0x1000000, CRC(d32f2fab) SHA1(273d58cb3c9075075b1ca39a3b247a2cd545fbe7) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "269-c4a.c4", 0x2000002, 0x1000000, CRC(bf77e878) SHA1(e6e76f8eed0d04ee9ad39bf38ce305930b10e2c1) ) /* Plane 2,3 */
ROM_END

/****************************************
 ID-2710
 . MV-0 ????
 NEO-MVH MVOC 2003.11.3
****************************************/

ROM_START( kf2k3pcb ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "271-p1.p1", 0x000000, 0x400000, CRC(b9da070c) SHA1(1a26325af142a4dd221c336061761468598c4634) )
	ROM_LOAD32_WORD_SWAP( "271-p2.p2", 0x000002, 0x400000, CRC(da3118c4) SHA1(582e4f44f03276adecb7b2848d3b96bf6da57f1e) )
	ROM_LOAD16_WORD_SWAP( "271-p3.p3", 0x800000, 0x100000, CRC(5cefd0d2) SHA1(cddc3164629fed4b6f715e12b109ad35d1009355) )

	ROM_REGION( 0x100000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x100000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_LOAD16_WORD_SWAP( "spj.sp1", 0x00000, 0x080000, CRC(148dd727) SHA1(2cf592a16c7157de02a989675d47965f2b3a44dd) ) // encrypted

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "271-m1.m1", 0x00000, 0x80000, CRC(d6bcf2bc) SHA1(df78bc95990eb8e8f3638dde6e1876354df7fe84) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1.v1", 0x000000, 0x1000000, CRC(1d96154b) SHA1(1d4e262b0d30cee79a4edc83bb9706023c736668) )

	NO_DELTAT_REGION

	ROM_REGION( 0x6000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "271-c1.c1", 0x0000000, 0x1000000, CRC(f5ebb327) SHA1(e4f799a54b09adcca13b1b0cf95971a1f4291b61) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c2.c2", 0x0000002, 0x1000000, CRC(2be21620) SHA1(872c658f53bbc558e90f18d5db9cbaa82e748a6a) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "271-c3.c3", 0x2000000, 0x1000000, CRC(ddded4ff) SHA1(ff7b356125bc9e6637b164f5e81b13eabeb8d804) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c4.c4", 0x2000002, 0x1000000, CRC(d85521e6) SHA1(62278fa8690972ed32aca07a4f7f97e7203d9f3a) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "271-c5.c5", 0x4000000, 0x1000000, CRC(18aa3540) SHA1(15e0a8c4e0927b1f7eb9bee8f532acea6818d5eb) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c6.c6", 0x4000002, 0x1000000, CRC(1c40de87) SHA1(8d6425aed43ff6a96c88194e203df6a783286373) ) /* Plane 2,3 */
ROM_END



	/* BrezzaSoft games, licensed? */


/****************************************
 ID-0008 (official?)
 . ???-????
 MVS PROGV (2000.11.17) / NEO-MVS CHAFIO (1999.6.14)
****************************************/

ROM_START( jockeygp ) /* MVS ONLY RELEASE */
	/* Officially licensed? Cart has a holographic 'SNK' sticker applied */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "008-epr.p1", 0x000000, 0x100000, CRC(2fb7f388) SHA1(e3c9b03944b4c10cf5081caaf9c8be1f08c06493) ) /* M27C160 */
	/* P on eprom, correct chip label unknown */
	ROM_FILL( 0x100000, 0x100000, 0xff )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "008-mg1.m1", CRC(d163c690) SHA1(1dfd04d20c5985037f07cd01000d0b04f3a8f4f4) )

	ROM_REGION( 0x0200000, "ymsnd", 0 )
	ROM_LOAD( "008-v1.v1", 0x000000, 0x200000, CRC(443eadba) SHA1(3def3c22f0e276bc4c2fc7ff70ce473c08b0d2df) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "008-c1.c1", 0x0000000, 0x800000, CRC(a9acbf18) SHA1(d55122c70cbe78c2679598dc07863e1d1d1a31df) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "008-c2.c2", 0x0000001, 0x800000, CRC(6289eef9) SHA1(a2ede77bb2468a2e1486d74745a22a5451026039) ) /* Plane 2,3 */
ROM_END

ROM_START( jockeygpa ) /* MVS ONLY RELEASE */
	/* Officially licensed? Cart has a holographic 'SNK' sticker applied */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "008-epr_a.p1", 0x000000, 0x100000, CRC(b8f35532) SHA1(b46c96677f1bfe324b678112e9c614a20c550d51) ) /* M27C800 */
	/* P on eprom, correct chip label unknown */

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "008-mg1.m1", CRC(d163c690) SHA1(1dfd04d20c5985037f07cd01000d0b04f3a8f4f4) )

	ROM_REGION( 0x0200000, "ymsnd", 0 )
	ROM_LOAD( "008-v1.v1", 0x000000, 0x200000, CRC(443eadba) SHA1(3def3c22f0e276bc4c2fc7ff70ce473c08b0d2df) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "008-c1.c1", 0x0000000, 0x800000, CRC(a9acbf18) SHA1(d55122c70cbe78c2679598dc07863e1d1d1a31df) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "008-c2.c2", 0x0000001, 0x800000, CRC(6289eef9) SHA1(a2ede77bb2468a2e1486d74745a22a5451026039) ) /* Plane 2,3 */
ROM_END

/****************************************
 ID-03E7 (incorrect ID-code)
 . ???-????
 MVS PROGV (2000.11.17) / MVS CHAV (2000.10.26)
****************************************/

ROM_START( vliner ) /* MVS ONLY RELEASE */
	/* Officially licensed? Cart has a holographic 'SNK' sticker applied */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "epr.p1", 0x000000, 0x080000, CRC(72a2c043) SHA1(b34bcc10ff33e4465126a6865fe8bf6b6a3d6cee) )
	/* P on eprom, correct chip label unknown */

	NEO_SFIX_128K( "s-1.s1", CRC(972d8c31) SHA1(41f09ef28a3791668ea304c74b8b06c117a50e9a) )

	NEO_BIOS_AUDIO_64K( "m-1.m1", CRC(9b92b7d1) SHA1(2c9b777feb9a8e43fa1bd942aba5afe3b5427d94) )

	ROM_REGION( 0x200000, "ymsnd", ROMREGION_ERASE00 )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "c-1.c1", 0x000000, 0x80000, CRC(5118f7c0) SHA1(b6fb6e9cbb660580d98e00780ebf248c0995145a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "c-2.c2", 0x000001, 0x80000, CRC(efe9b33e) SHA1(910c651aadce9bf59e51c338ceef62287756d2e8) ) /* Plane 2,3 */
ROM_END

ROM_START( vlinero ) /* MVS ONLY RELEASE */
	/* Officially licensed? Cart has a holographic 'SNK' sticker applied */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "epr_54.p1", 0x000000, 0x080000, CRC(172efc18) SHA1(8ca739f8780a9e6fa19ac2c3e931d75871603f58) )
	/* P on eprom, correct chip label unknown */

	NEO_SFIX_128K( "s-1.s1", CRC(972d8c31) SHA1(41f09ef28a3791668ea304c74b8b06c117a50e9a) )

	NEO_BIOS_AUDIO_64K( "m-1.m1", CRC(9b92b7d1) SHA1(2c9b777feb9a8e43fa1bd942aba5afe3b5427d94) )

	ROM_REGION( 0x200000, "ymsnd", ROMREGION_ERASE00 )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "c-1.c1", 0x000000, 0x80000, CRC(5118f7c0) SHA1(b6fb6e9cbb660580d98e00780ebf248c0995145a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "c-2.c2", 0x000001, 0x80000, CRC(efe9b33e) SHA1(910c651aadce9bf59e51c338ceef62287756d2e8) ) /* Plane 2,3 */
ROM_END




/****************************************************************************/

	/* Game specific input definitions */


static INPUT_PORTS_START( svcpcb )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* the rom banking is tied directly to the dipswitch?, or is there a bank write somewhere? */
	PORT_START("HARDDIP")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Region ) ) PORT_DIPLOCATION("HARDDIP:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Asia ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Japan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ms5pcb )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* the rom banking is tied directly to the dipswitch?, or is there a bank write somewhere? */
	PORT_START("HARDDIP")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Region ) ) PORT_DIPLOCATION("HARDDIP:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Asia ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Japan ) )
INPUT_PORTS_END


#if 0 // this needs to be added dynamically somehow
static INPUT_PORTS_START( kog )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* a jumper on the pcb overlays a ROM address, very strange but that's how it works. */
	PORT_START("JUMPER")
	PORT_DIPNAME( 0x0001, 0x0001, "Title Language" ) PORT_DIPLOCATION("CART-JUMPER:1")
	PORT_DIPSETTING(	  0x0001, DEF_STR( English ) )
	PORT_DIPSETTING(	  0x0000, "Non-English" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END
#endif

#if 0 // this needs to be added dynamically somehow
static INPUT_PORTS_START( mjneogeo )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Coin Chutes?" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(	  0x0000, "1?" )
	PORT_DIPSETTING(	  0x0002, "2?" )
	PORT_DIPNAME( 0x0004, 0x0000, "Mahjong Control Panel" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, "COMM Setting (Cabinet No.)" ) PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING(	  0x0018, "1" )
	PORT_DIPSETTING(	  0x0008, "2" )
	PORT_DIPSETTING(	  0x0010, "3" )
	PORT_DIPSETTING(	  0x0000, "4" )
	PORT_DIPNAME( 0x0020, 0x0020, "COMM Setting (Link Enable)" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mahjong_controller_r, NULL)

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	PORT_START("MAHJONG1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MAHJONG2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MAHJONG3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MAHJONG4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END
#endif

static INPUT_PORTS_START( irrmaze )
	PORT_START("IN0")
	STANDARD_DIPS
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(multiplexed_controller_r, (void *)0)

	PORT_START("IN1")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_memcard_status, NULL)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	STANDARD_IN3

	PORT_START("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )  /* this bit is used.. */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter BIOS") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE

	PORT_START("IN0-1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE
INPUT_PORTS_END

#if 0 // this needs to be added dynamically somehow
static INPUT_PORTS_START( popbounc )
	PORT_START("IN0")
	STANDARD_DIPS
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(multiplexed_controller_r, (void *)0)

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(multiplexed_controller_r, (void *)1)

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* Fake inputs read by CUSTOM_INPUT handlers */
	PORT_START("IN0-0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x90, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* note it needs it from 0x80 when using paddle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1-0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL  ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("IN1-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x90, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* note it needs it from 0x80 when using paddle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
INPUT_PORTS_END
#endif

static INPUT_PORTS_START( vliner )
	PORT_START("IN0")
	STANDARD_DIPS
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("View Payout Table/Big")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet/Small")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop/Double Up")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Start/Collect")

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* this bit is used.. */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* this bit is used.. */
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_memcard_status, NULL)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_calendar_status, NULL)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_audio_result, NULL)

	STANDARD_IN4

	PORT_START("IN5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Operator Menu") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Clear Credit")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Hopper Out")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* not sure what any of these bits are */
	PORT_START("IN6")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( jockeygp )
	STANDARD_IN0

	STANDARD_IN1

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* game freezes with this bit enabled */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Next Game") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* game freezes with this bit enabled */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Previous Game") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_memcard_status, NULL)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	STANDARD_IN3

	STANDARD_IN4
INPUT_PORTS_END


/****************************************************************************/

	/* Game specific inits */

static DRIVER_INIT( fatfury2 )
{
	DRIVER_INIT_CALL(neogeo);
	fatfury2_install_protection(machine);
}

static DRIVER_INIT ( kof98 )
{
	DRIVER_INIT_CALL(neogeo);
	kof98_decrypt_68k(machine);
	install_kof98_protection(machine);
}

static DRIVER_INIT( mslugx )
{
	DRIVER_INIT_CALL(neogeo);
	mslugx_install_protection(machine);
}

static DRIVER_INIT( kof99 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	kof99_decrypt_68k(machine);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x00);
	kof99_install_protection(machine);
}

static DRIVER_INIT( kof99k )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x00);
}

static DRIVER_INIT( garou )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	garou_decrypt_68k(machine);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x06);
	garou_install_protection(machine);
}

static DRIVER_INIT( garouo )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	garouo_decrypt_68k(machine);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x06);
	garouo_install_protection(machine);
}

static DRIVER_INIT( garoubl )
{
	DRIVER_INIT_CALL(neogeo);
	neogeo_bootleg_sx_decrypt(machine, 2);
	neogeo_bootleg_cx_decrypt(machine);
}

static DRIVER_INIT( mslug3 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	mslug3_decrypt_68k(machine);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xad);
	mslug3_install_protection(machine);
}

static DRIVER_INIT( mslug3h )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xad);
}

static DRIVER_INIT( mslug3b6 )
{
	DRIVER_INIT_CALL(neogeo);
	neogeo_bootleg_sx_decrypt(machine, 2);
	cmc42_neogeo_gfx_decrypt(machine, 0xad);
}

static DRIVER_INIT( kof2000 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	kof2000_decrypt_68k(machine);
	state->m_fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x00);
	kof2000_install_protection(machine);
}

static DRIVER_INIT( kof2000n )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x00);
}

static DRIVER_INIT( kof2001 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof2000_neogeo_gfx_decrypt(machine, 0x1e);
	neogeo_cmc50_m1_decrypt(machine);
}

static DRIVER_INIT( cthd2003 )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_cthd2003(machine);
	patch_cthd2003(machine);
}

static DRIVER_INIT ( ct2k3sp )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_ct2k3sp(machine);
	patch_cthd2003(machine);
}

static DRIVER_INIT ( ct2k3sa )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_ct2k3sa(machine);
	patch_ct2k3sa(machine);
}

static DRIVER_INIT( mslug4 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1; /* USA violent content screen is wrong -- not a bug, confirmed on real hardware! */
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x31);
	neo_pcm2_snk_1999(machine, 8);
}

static DRIVER_INIT( ms4plus )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x31);
	neo_pcm2_snk_1999(machine, 8);
	neogeo_cmc50_m1_decrypt(machine);
}

static DRIVER_INIT( ganryu )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x07);
}

static DRIVER_INIT( s1945p )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x05);
}

static DRIVER_INIT( preisle2 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x9f);
}

static DRIVER_INIT( bangbead )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xf8);
}

static DRIVER_INIT( nitd )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xff);
}

static DRIVER_INIT( zupapa )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xbd);
}

static DRIVER_INIT( sengoku3 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xfe);
}

static DRIVER_INIT( rotd )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	neo_pcm2_snk_1999(machine, 16);
	state->m_fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x3f);
}

static DRIVER_INIT( kof2002 )
{
	DRIVER_INIT_CALL(neogeo);
	kof2002_decrypt_68k(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kof2002b )
{
	DRIVER_INIT_CALL(neogeo);
	kof2002_decrypt_68k(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	kof2002b_gfx_decrypt(machine, machine.region("sprites")->base(),0x4000000);
	kof2002b_gfx_decrypt(machine, machine.region("fixed")->base(),0x20000);
}

static DRIVER_INIT( kf2k2pls )
{
	DRIVER_INIT_CALL(neogeo);
	kof2002_decrypt_68k(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	cmc50_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kf2k2mp )
{
	DRIVER_INIT_CALL(neogeo);
	kf2k2mp_decrypt(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 2);
	cmc50_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kf2k2mp2 )
{
	DRIVER_INIT_CALL(neogeo);
	kf2k2mp2_px_decrypt(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	cmc50_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kof10th )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kof10th(machine);
	install_kof10th_protection(machine);
}

static DRIVER_INIT( kf10thep )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kf10thep(machine);
}

static DRIVER_INIT( kf2k5uni )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kf2k5uni(machine);
}

static DRIVER_INIT( kof2k4se )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kof2k4se_68k(machine);
}

static DRIVER_INIT( matrim )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	matrim_decrypt_68k(machine);
	neo_pcm2_swap(machine, 1);
	state->m_fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x6a);
}

static DRIVER_INIT( matrimbl )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	matrim_decrypt_68k(machine);
	state->m_fixed_layer_bank_type = 2;
	matrimbl_decrypt(machine);
	neogeo_sfix_decrypt(machine); /* required for text layer */
}

static DRIVER_INIT( pnyaa )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	neo_pcm2_snk_1999(machine, 4);
	state->m_fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x2e);
}

static DRIVER_INIT( mslug5 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	mslug5_decrypt_68k(machine);
	neo_pcm2_swap(machine, 2);
	state->m_fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x19);
	install_pvc_protection(machine);
}

static TIMER_CALLBACK( ms5pcb_bios_timer_callback )
{
	int harddip3 = input_port_read(machine, "HARDDIP") & 1;
	memory_set_bankptr(machine, NEOGEO_BANK_BIOS, machine.region("mainbios")->base() + 0x20000 + harddip3 * 0x20000);
}

static DRIVER_INIT( ms5pcb )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);

	/* start a timer that will check the BIOS select DIP every second */
	machine.scheduler().timer_set(attotime::zero, FUNC(ms5pcb_bios_timer_callback));
	machine.scheduler().timer_pulse(attotime::from_msec(1000), FUNC(ms5pcb_bios_timer_callback));

	mslug5_decrypt_68k(machine);
	svcpcb_gfx_decrypt(machine);
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x19);
	state->m_fixed_layer_bank_type = 2;
	svcpcb_s1data_decrypt(machine);
	neo_pcm2_swap(machine, 2);
	install_pvc_protection(machine);
}

static DRIVER_INIT( ms5plus )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x19);
	neo_pcm2_swap(machine, 2);
	neogeo_bootleg_sx_decrypt(machine, 1);
	state->m_fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	install_ms5plus_protection(machine);
}

static TIMER_CALLBACK( svcpcb_bios_timer_callback )
{
	int harddip3 = input_port_read(machine, "HARDDIP") & 1;
	memory_set_bankptr(machine, NEOGEO_BANK_BIOS, machine.region("mainbios")->base() + 0x20000 + harddip3 * 0x20000);
}

static DRIVER_INIT( svcpcb )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);

	/* start a timer that will check the BIOS select DIP every second */
	machine.scheduler().timer_set(attotime::zero, FUNC(svcpcb_bios_timer_callback));
	machine.scheduler().timer_pulse(attotime::from_msec(1000), FUNC(svcpcb_bios_timer_callback));

	svc_px_decrypt(machine);
	svcpcb_gfx_decrypt(machine);
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x57);
	svcpcb_s1data_decrypt(machine);
	neo_pcm2_swap(machine, 3);
	state->m_fixed_layer_bank_type = 2;
	install_pvc_protection(machine);
}

static DRIVER_INIT( svc )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	svc_px_decrypt(machine);
	neo_pcm2_swap(machine, 3);
	state->m_fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x57);
	install_pvc_protection(machine);
}

static DRIVER_INIT( svcboot )
{
	DRIVER_INIT_CALL(neogeo);
	svcboot_px_decrypt(machine);
	svcboot_cx_decrypt(machine);
	install_pvc_protection(machine);
}

static DRIVER_INIT( svcplus )
{
	DRIVER_INIT_CALL(neogeo);
	svcplus_px_decrypt(machine);
	svcboot_cx_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	svcplus_px_hack(machine);
}

static DRIVER_INIT( svcplusa )
{
	DRIVER_INIT_CALL(neogeo);
	svcplusa_px_decrypt(machine);
	svcboot_cx_decrypt(machine);
	svcplus_px_hack(machine);
}

static DRIVER_INIT( svcsplus )
{
	DRIVER_INIT_CALL(neogeo);
	svcsplus_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 2);
	svcboot_cx_decrypt(machine);
	svcsplus_px_hack(machine);
	install_pvc_protection(machine);
}

static DRIVER_INIT( samsho5 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	samsho5_decrypt_68k(machine);
	neo_pcm2_swap(machine, 4);
	state->m_fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x0f);
}

static DRIVER_INIT( samsho5b )
{
	DRIVER_INIT_CALL(neogeo);
	samsho5b_px_decrypt(machine);
	samsho5b_vx_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	neogeo_bootleg_cx_decrypt(machine);
}

static DRIVER_INIT( kf2k3pcb )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	kf2k3pcb_decrypt_68k(machine);
	kf2k3pcb_gfx_decrypt(machine);
	kof2003biosdecode(machine);
	neogeo_cmc50_m1_decrypt(machine);

	/* extra little swap on the m1 - this must be performed AFTER the m1 decrypt
       or the m1 checksum (used to generate the key) for decrypting the m1 is
       incorrect */
	{
		int i;
		UINT8* rom = machine.region("audiocpu")->base();
		for (i = 0; i < 0x90000; i++)
		{
			rom[i] = BITSWAP8(rom[i], 5, 6, 1, 4, 3, 0, 7, 2);
		}

	}

	kof2000_neogeo_gfx_decrypt(machine, 0x9d);
	kf2k3pcb_decrypt_s1data(machine);
	neo_pcm2_swap(machine, 5);
	state->m_fixed_layer_bank_type = 2;
	install_pvc_protection(machine);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0xc00000, 0xc7ffff, "bios" );  // 512k bios
}

static DRIVER_INIT( kof2003 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	kof2003_decrypt_68k(machine);
	neo_pcm2_swap(machine, 5);
	state->m_fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x9d);
	install_pvc_protection(machine);
}

static DRIVER_INIT( kof2003h )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	kof2003h_decrypt_68k(machine);
	neo_pcm2_swap(machine, 5);
	state->m_fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x9d);
	install_pvc_protection(machine);
}

static DRIVER_INIT( kf2k3bl )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x9d);
	neo_pcm2_swap(machine, 5);
	neogeo_bootleg_sx_decrypt(machine, 1);
	kf2k3bl_install_protection(machine);
}

static DRIVER_INIT( kf2k3pl )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x9d);
	neo_pcm2_swap(machine, 5);
	kf2k3pl_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	kf2k3pl_install_protection(machine);
}

static DRIVER_INIT( kf2k3upl )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x9d);
	neo_pcm2_swap(machine, 5);
	kf2k3upl_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 2);
	kf2k3upl_install_protection(machine);
}

static DRIVER_INIT( samsh5sp )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	samsh5sp_decrypt_68k(machine);
	neo_pcm2_swap(machine, 6);
	state->m_fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x0d);
}

static DRIVER_INIT( jockeygp )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	DRIVER_INIT_CALL(neogeo);
	state->m_fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0xac);

	/* install some extra RAM */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x200000, 0x201fff);

//  machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port(0x280000, 0x280001, "IN5");
//  machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port(0x2c0000, 0x2c0001, "IN6");
}

static DRIVER_INIT( vliner )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x200000, 0x201fff);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port(0x280000, 0x280001, "IN5");
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port(0x2c0000, 0x2c0001, "IN6");

	DRIVER_INIT_CALL(neogeo);
}

static READ16_HANDLER( kog_jumper_r )
{
	return 0xff01;
	//return 0xff00;
}

static DRIVER_INIT( kog )
{
	/* overlay cartridge ROM */
	// we need to add the jumper to the inputs dynamically somehow
	//machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_port(0x0ffffe, 0x0fffff, "JUMPER");
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x0ffffe, 0x0fffff, FUNC(kog_jumper_r));
	
	kog_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	neogeo_bootleg_cx_decrypt(machine);
	DRIVER_INIT_CALL(neogeo);
}

static DRIVER_INIT( lans2004 )
{
	lans2004_decrypt_68k(machine);
	lans2004_vx_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	neogeo_bootleg_cx_decrypt(machine);
	DRIVER_INIT_CALL(neogeo);
}

driver_init_func neo_gameinit;

static DRIVER_INIT( mvs )
{
	DRIVER_INIT_CALL(neogeo);
}


// handle protected carts
void mvs_install_protection(device_image_interface& image)
{
//	neogeo_state *state = image.device().machine().driver_data<neogeo_state>();
	const char *crypt_feature = image.get_feature( "crypt" );

	if(crypt_feature == NULL)
	{
		neo_gameinit = driver_init_neogeo; 
	}
	else
	{
		/* work out which Driver Init decryption function to call based on flag in softlist
		 - note this is called AFTER the regular DRIVER_INIT */

		if(strcmp(crypt_feature,"mslugx_prot") == 0)		{ neo_gameinit = driver_init_mslugx; }
		else if(strcmp(crypt_feature,"fatfury2_prot") == 0) { neo_gameinit = driver_init_fatfury2; }
		else if(strcmp(crypt_feature,"kog_prot") == 0)		{ neo_gameinit = driver_init_kog; }
		else if(strcmp(crypt_feature,"kof98_prot") == 0)	{ neo_gameinit = driver_init_kof98; }
		else if(strcmp(crypt_feature,"kof99_prot") == 0)	{ neo_gameinit = driver_init_kof99; }
		else if(strcmp(crypt_feature,"kof99k_prot") == 0)	{ neo_gameinit = driver_init_kof99k; }
		else if(strcmp(crypt_feature,"garou_prot") == 0)	{ neo_gameinit = driver_init_garou; }
		else if(strcmp(crypt_feature,"garouo_prot") == 0)	{ neo_gameinit = driver_init_garouo; }
		else if(strcmp(crypt_feature,"garoubl_prot") == 0)	{ neo_gameinit = driver_init_garoubl; }
		else if(strcmp(crypt_feature,"mslug3_prot") == 0)	{ neo_gameinit = driver_init_mslug3; }
		else if(strcmp(crypt_feature,"mslug3h_prot") == 0)	{ neo_gameinit = driver_init_mslug3h; }
		else if(strcmp(crypt_feature,"mslug3b6_prot") == 0)	{ neo_gameinit = driver_init_mslug3b6; }
		else if(strcmp(crypt_feature,"kof2000_prot") == 0)	{ neo_gameinit = driver_init_kof2000; }
		else if(strcmp(crypt_feature,"kof2000n_prot") == 0)	{ neo_gameinit = driver_init_kof2000n; }
		else if(strcmp(crypt_feature,"sengoku3_prot") == 0)	{ neo_gameinit = driver_init_sengoku3; }
		else if(strcmp(crypt_feature,"zupapa_prot") == 0)	{ neo_gameinit = driver_init_zupapa; }
		else if(strcmp(crypt_feature,"kof2001_prot") == 0)	{ neo_gameinit = driver_init_kof2001; }
		else if(strcmp(crypt_feature,"cthd2003_prot") == 0)	{ neo_gameinit = driver_init_cthd2003; }
		else if(strcmp(crypt_feature,"ct2k3sp_prot") == 0)	{ neo_gameinit = driver_init_ct2k3sp; }
		else if(strcmp(crypt_feature,"ct2k3sa_prot") == 0)	{ neo_gameinit = driver_init_ct2k3sa; }
		else if(strcmp(crypt_feature,"kof2002_prot") == 0)	{ neo_gameinit = driver_init_kof2002; }
		else if(strcmp(crypt_feature,"kof2002b_prot") == 0)	{ neo_gameinit = driver_init_kof2002b; }
		else if(strcmp(crypt_feature,"kf2k2pls_prot") == 0)	{ neo_gameinit = driver_init_kf2k2pls; }
		else if(strcmp(crypt_feature,"kf2k2mp_prot") == 0)	{ neo_gameinit = driver_init_kf2k2mp; }
		else if(strcmp(crypt_feature,"kf2k2mp2_prot") == 0)	{ neo_gameinit = driver_init_kf2k2mp2; }
		else if(strcmp(crypt_feature,"kof10th_prot") == 0)	{ neo_gameinit = driver_init_kof10th; }
		else if(strcmp(crypt_feature,"kf10thep_prot") == 0)	{ neo_gameinit = driver_init_kf10thep; }
		else if(strcmp(crypt_feature,"kf2k5uni_prot") == 0)	{ neo_gameinit = driver_init_kf2k5uni; }
		else if(strcmp(crypt_feature,"kof2k4se_prot") == 0)	{ neo_gameinit = driver_init_kof2k4se; }
		else if(strcmp(crypt_feature,"mslug5_prot") == 0)	{ neo_gameinit = driver_init_mslug5; }
		else if(strcmp(crypt_feature,"ms5plus_prot") == 0)	{ neo_gameinit = driver_init_ms5plus; }
		else if(strcmp(crypt_feature,"svc_prot") == 0)		{ neo_gameinit = driver_init_svc; }
		else if(strcmp(crypt_feature,"svcboot_prot") == 0)	{ neo_gameinit = driver_init_svcboot; }
		else if(strcmp(crypt_feature,"svcplus_prot") == 0)	{ neo_gameinit = driver_init_svcplus; }
		else if(strcmp(crypt_feature,"svcplusa_prot") == 0)	{ neo_gameinit = driver_init_svcplusa; }
		else if(strcmp(crypt_feature,"svcsplus_prot") == 0)	{ neo_gameinit = driver_init_svcsplus; }
		else if(strcmp(crypt_feature,"samsho5_prot") == 0)	{ neo_gameinit = driver_init_samsho5; }
		else if(strcmp(crypt_feature,"samsho5b_prot") == 0)	{ neo_gameinit = driver_init_samsho5b; }
		else if(strcmp(crypt_feature,"kof2003_prot") == 0)	{ neo_gameinit = driver_init_kof2003; }
		else if(strcmp(crypt_feature,"kof2003h_prot") == 0)	{ neo_gameinit = driver_init_kof2003h; }
		else if(strcmp(crypt_feature,"kf2k3bl_prot") == 0)	{ neo_gameinit = driver_init_kf2k3bl; }
		else if(strcmp(crypt_feature,"kf2k3pl_prot") == 0)	{ neo_gameinit = driver_init_kf2k3pl; }
		else if(strcmp(crypt_feature,"kf2k3upl_prot") == 0)	{ neo_gameinit = driver_init_kf2k3upl; }
		else if(strcmp(crypt_feature,"samsh5sp_prot") == 0)	{ neo_gameinit = driver_init_samsh5sp; }
		else if(strcmp(crypt_feature,"preisle2_prot") == 0)	{ neo_gameinit = driver_init_preisle2; }
		else if(strcmp(crypt_feature,"nitd_prot") == 0)		{ neo_gameinit = driver_init_nitd; }
		else if(strcmp(crypt_feature,"s1945p_prot") == 0)	{ neo_gameinit = driver_init_s1945p; }
		else if(strcmp(crypt_feature,"lans2004_prot") == 0)	{ neo_gameinit = driver_init_lans2004; }
		else if(strcmp(crypt_feature,"pnyaa_prot") == 0)	{ neo_gameinit = driver_init_pnyaa; }
		else if(strcmp(crypt_feature,"ganryu_prot") == 0)	{ neo_gameinit = driver_init_ganryu; }
		else if(strcmp(crypt_feature,"bangbead_prot") == 0)	{ neo_gameinit = driver_init_bangbead; }
		else if(strcmp(crypt_feature,"mslug4_prot") == 0)	{ neo_gameinit = driver_init_mslug4; }
		else if(strcmp(crypt_feature,"ms4plus_prot") == 0)	{ neo_gameinit = driver_init_ms4plus; }
		else if(strcmp(crypt_feature,"rotd_prot") == 0)		{ neo_gameinit = driver_init_rotd; }
		else if(strcmp(crypt_feature,"matrim_prot") == 0)	{ neo_gameinit = driver_init_matrim; }
		else if(strcmp(crypt_feature,"matrimbl_prot") == 0)	{ neo_gameinit = driver_init_matrimbl; }
		else { fatalerror("unknown crypt type\n"); }
	}

	if (neo_gameinit) neo_gameinit(image.device().machine());
}


/****************************************************************************/

/*    YEAR  NAME       PARENT    COMPAT		MACHINE   INPUT     INIT     */
CONS( 1990, neogeo,    0,        0,			mvs,	  neogeo,   mvs,    "SNK", "Neo-Geo", GAME_IS_BIOS_ROOT | GAME_SUPPORTS_SAVE )

/* This requires a special bios so is handled here, not in the softlist */
GAME( 1997, irrmaze,   neogeo,   neogeo,   irrmaze,  neogeo,   ROT0, "SNK / Saurus", "The Irritating Maze / Ultra Denryu Iraira Bou", GAME_SUPPORTS_SAVE )

/* These are standalone PCBs and don't belong in the softlist */
GAME( 2003, ms5pcb,    0,        neogeo,   ms5pcb,   ms5pcb,   ROT0, "SNK Playmore", "Metal Slug 5 (JAMMA PCB)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 2003, svcpcb,    0,        neogeo,   svcpcb,   svcpcb,   ROT0, "SNK Playmore", "SNK vs. Capcom - SVC Chaos (JAMMA PCB, set 1)", GAME_SUPPORTS_SAVE )
GAME( 2003, svcpcba,   svcpcb,   neogeo,   svcpcb,   svcpcb,   ROT0, "SNK Playmore", "SNK vs. Capcom - SVC Chaos (JAMMA PCB, set 2)" , GAME_SUPPORTS_SAVE )
GAME( 2003, kf2k3pcb,  0,        neogeo,   neogeo,   kf2k3pcb, ROT0, "SNK Playmore", "The King of Fighters 2003 (Japan, JAMMA PCB)", GAME_SUPPORTS_SAVE )

/* These expect very specific extra hardware despite being carts, and don't really belong in the softlist */
GAME( 2001, jockeygp,  neogeo,   neogeo,   jockeygp, jockeygp, ROT0, "Sun Amusement / BrezzaSoft", "Jockey Grand Prix (set 1)", GAME_SUPPORTS_SAVE )
GAME( 2001, jockeygpa, jockeygp, neogeo,   jockeygp, jockeygp, ROT0, "Sun Amusement / BrezzaSoft", "Jockey Grand Prix (set 2)", GAME_SUPPORTS_SAVE )
GAME( 2001, vliner,    neogeo,   neogeo,   vliner,   vliner,   ROT0, "Dyna / BrezzaSoft", "V-Liner (set 1)", GAME_SUPPORTS_SAVE )
GAME( 2001, vlinero,   vliner,   neogeo,   vliner,   vliner,   ROT0, "Dyna / BrezzaSoft", "V-Liner (set 2)", GAME_SUPPORTS_SAVE )


