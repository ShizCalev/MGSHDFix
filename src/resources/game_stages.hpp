#pragma once



struct Stage
{
    const char* sStageId;
    const char* sGameMode;
    const char* sRichPresenceName;
};

//X(BOSS,      "boss",      "Menu", "Boss Survival (Transition)") \

#define MGS2_STAGE_LIST \
    /* Menus                 */ \
    X(INIT,      "init",      "Menu", "Game Initialization") \
    X(N_TITLE,   "n_title",   "Menu", "Title Screen") \
    X(SELECT,    "select",    "Menu", "Developer Menu") \
    X(MSELECT,   "mselect",   "Menu", "VR Missions Menu") \
    X(SSELECT,   "sselect",   "Menu", "Snake Tales Select / Credits") \
    X(TALES,     "tales",     "Menu", "Snake Tales Menu") \
    \
    /* Tanker (Playable)     */ \
    X(W00A, "w00a", "Tanker", "Aft Deck") \
    X(W00B, "w00b", "Tanker", "Deck A - Port") \
    X(W00C, "w00c", "Tanker", "Navigational Deck, Wing") \
    X(W01A, "w01a", "Tanker", "Deck-A Crew's Quarters") \
    X(W01B, "w01b", "Tanker", "Deck-B Crew's Quarters") \
    X(W01C, "w01c", "Tanker", "Deck-C Crew's Quarters") \
    X(W01D, "w01d", "Tanker", "Deck-D Crew's Quarters") \
    X(W01E, "w01e", "Tanker", "Deck-E, The Bridge") \
    X(W01F, "w01f", "Tanker", "Deck-A Crew's Lounge") \
    X(W02A, "w02a", "Tanker", "Engine Room") \
    X(W03A, "w03a", "Tanker", "Deck-2, Port") \
    X(W03B, "w03b", "Tanker", "Deck-2, Starboard") \
    X(W04A, "w04a", "Tanker", "Hold No.1") \
    X(W04B, "w04b", "Tanker", "Hold No.2") \
    X(W04C, "w04c", "Tanker", "Hold No.3") \
    \
    /* Tanker (Cutscenes)    */ \
    X(D00T,   "d00t",   "Tanker", "George Washington Bridge") /*Tanker Opening*/ \
    X(D01T,   "d01t",   "Tanker", "Navigational Deck, Wing") /*Russian Invasion*/\
    X(D04T,   "d04t",   "Tanker", "Identifying Choppers") \
    X(D05T,   "d05t",   "Tanker", "Olga Cutscene") \
    X(D10T,   "d10t",   "Tanker", "Cutscene d10t") \
    X(D11T,   "d11t",   "Tanker", "Cutscene d11t") \
    X(D12T,   "d12t",   "Tanker", "Cutscene d12t") \
    X(D12T3,  "d12t3",  "Tanker", "Cutscene d12t3") \
    X(D12T4,  "d12t4",  "Tanker", "Cutscene d12t4") \
    X(D13T,   "d13t",   "Tanker", "Cutscene d13t (NG+ Rewards)") \
    X(D14T,   "d14t",   "Tanker", "Cutscene d14t") \
    \
    /* Plant (Playable)      */ \
    X(W11A, "w11a", "Plant", "Strut A Deep Sea Dock") \
    X(W11B, "w11b", "Plant", "Strut A Deep Sea Dock (Bomb Disposal)") \
    X(W11C, "w11c", "Plant", "Strut A Deep Sea Dock (Fortune)") \
    X(W12A, "w12a", "Plant", "Strut A Roof") \
    X(W12B, "w12b", "Plant", "Strut A Pump Room") \
    X(W12C, "w12c", "Plant", "Strut A Roof (Bomb)") \
    X(W13A, "w13a", "Plant", "AB Connecting Bridge") \
    X(W13B, "w13b", "Plant", "AB Connecting Bridge (Sensor B)") \
    X(W14A, "w14a", "Plant", "Strut B Transformer Room") \
    X(W15A, "w15a", "Plant", "BC Connecting Bridge") \
    X(W15B, "w15b", "Plant", "BC Connecting Bridge (After Stillman)") \
    X(W16A, "w16a", "Plant", "Strut C Dining Hall") \
    X(W16B, "w16b", "Plant", "Strut C Dining Hall (Post Stillman)") \
    X(W17A, "w17a", "Plant", "CD Connecting Bridge") \
    X(W18A, "w18a", "Plant", "Strut D Sediment Pool") \
    X(W19A, "w19a", "Plant", "DE Connecting Bridge") \
    X(W20A, "w20a", "Plant", "Strut E Parcel Room") \
    X(W20B, "w20b", "Plant", "Strut E Heliport") \
    X(W20C, "w20c", "Plant", "Strut E Heliport (Bomb)") \
    X(W20D, "w20d", "Plant", "Strut E Heliport (Post Ninja)") \
    X(W21A, "w21a", "Plant", "EF Connecting Bridge") \
    X(W21B, "w21b", "Plant", "EF Connecting Bridge 2") \
    X(W22A, "w22a", "Plant", "Strut F Warehouse") \
    X(W23A, "w23a", "Plant", "FA Connecting Bridge") \
    X(W23B, "w23b", "Plant", "FA Connecting Bridge (Post Shell 2)") \
    X(W24A, "w24a", "Plant", "Shell 1 Core, 1F") \
    X(W24B, "w24b", "Plant", "Shell 1 Core, B1") \
    X(W24C, "w24c", "Plant", "Shell 1 Core, B1 Hall") \
    X(W24D, "w24d", "Plant", "Shell 1 Core, B2 Computer Room") \
    X(W24E, "w24e", "Plant", "Shell 1 Core, B1 Hall") \
    X(W25A, "w25a", "Plant", "Shell 1-2 Connecting Bridge") \
    X(W25B, "w25b", "Plant", "Shell 1-2 Connecting Bridge - LG Connecting Bridge") \
    X(W25C, "w25c", "Plant", "Strut L Perimeter - KL Connecting Bridge") \
    X(W25D, "w25d", "Plant", "KL Connecting Bridge") \
    X(W28A, "w28a", "Plant", "Strut L Sewage Treatment Facility") \
    X(W31A, "w31a", "Plant", "Shell 2 Core, 1F Air Purification Room") \
    X(W31B, "w31b", "Plant", "Shell 2 Core, B1 Filtration Chamber No.1") \
    X(W31C, "w31c", "Plant", "Shell 2 Core, B1 Filtration Chamber No.2") \
    X(W31D, "w31d", "Plant", "Shell 2 Core, 1F Air Purification Room (With Emma)") \
    X(W31F, "w31f", "Plant", "Shell 2 Core, B1 Filtration Chamber No.2") \
    X(W32A, "w32a", "Plant", "Strut L Oil Fence") \
    X(W32B, "w32b", "Plant", "Strut L Oil Fence (Vamp)") \
    X(W41A, "w41a", "Plant", "Arsenal Gear - Stomach") \
    X(W42A, "w42a", "Plant", "Arsenal Gear - Jejunum") \
    X(W43A, "w43a", "Plant", "Arsenal Gear - Ascending Colon") \
    X(W44A, "w44a", "Plant", "Arsenal Gear - Ileum") \
    X(W45A, "w45a", "Plant", "Arsenal Gear - Sigmoid Colon") \
    X(W46A, "w46a", "Plant", "Arsenal Gear - Rectum") \
    X(W51A, "w51a", "Plant", "Arsenal Gear") \
    X(W61A, "w61a", "Plant", "Federal Hall") \
    \
    /* Plant (Cutscenes)     */ \
    X(MUSEUM,   "museum",   "Plant", "Briefing") \
    X(WEBDEMO,  "webdemo",  "Plant", "Web Demo") \
    X(WMOVIE,   "wmovie",   "Plant", "Full-Screen Movie") \
    X(ENDING,   "ending",   "Plant", "Results Screen") \
    X(D001P01,  "d001p01",  "Plant", "Plant Opening") \
    X(D001P02,  "d001p02",  "Plant", "Sea Dock Cutscene") \
    X(D005P01,  "d005p01",  "Plant", "Raiden On Elevator") \
    X(D005P03,  "d005p03",  "Plant", "Strut A Roof Cutscene") \
    X(D010P01,  "d010p01",  "Plant", "Meeting Vamp") \
    X(D012P01,  "d012p01",  "Plant", "Meeting Fortune") \
    X(D014P01,  "d014p01",  "Plant", "Stillman Cutscene") \
    X(D021P01,  "d021p01",  "Plant", "Fatman and Ninja") \
    X(D036P03,  "d036p03",  "Plant", "Hostage Cutscene") \
    X(D036P05,  "d036p05",  "Plant", "Shell 1 Cutscene") \
    X(D045P01,  "d045p01",  "Plant", "Cutscene d045p01") \
    X(D046P01,  "d046p01",  "Plant", "Cutscene d046p01") \
    X(D053P01,  "d053p01",  "Plant", "Cutscene d053p01") \
    X(D055P01,  "d055p01",  "Plant", "Cutscene d055p01") \
    X(D063P01,  "d063p01",  "Plant", "Cutscene d063p01") \
    X(D065P02,  "d065p02",  "Plant", "Cutscene d065p02") \
    X(D070P01,  "d070p01",  "Plant", "Cutscene d070p01") \
    X(D070P09,  "d070p09",  "Plant", "Cutscene d070p09") \
    X(D070PX9,  "d070px9",  "Plant", "Cutscene d070px9") \
    X(D078P01,  "d078p01",  "Plant", "Cutscene d078p01") \
    X(D080P01,  "d080p01",  "Plant", "Cutscene d080p01") \
    X(D080P06,  "d080p06",  "Plant", "Cutscene d080p06") \
    X(D080P07,  "d080p07",  "Plant", "Cutscene d080p07") \
    X(D080P08,  "d080p08",  "Plant", "Cutscene d080p08") \
    X(D082P01,  "d082p01",  "Plant", "Cutscene d082p01") \
    \
    /* Alternate Tanker maps */ \
    X(A00A, "a00a", "Alternate", "Aft Deck") \
    X(A00B, "a00b", "Alternate", "Deck A - Port") \
    X(A00C, "a00c", "Alternate", "Navigational Deck, Wing (Unused Alt)") \
    X(A01A, "a01a", "Alternate", "Deck-A Crew's Quarters") \
    X(A01B, "a01b", "Alternate", "Deck-B Crew's Quarters") \
    X(A01C, "a01c", "Alternate", "Deck-C Crew's Quarters") \
    X(A01D, "a01d", "Alternate", "Deck-D Crew's Quarters") \
    X(A01E, "a01e", "Alternate", "Deck-E, The Bridge") \
    X(A01F, "a01f", "Alternate", "Deck-A Crew's Lounge") \
    X(A02A, "a02a", "Alternate", "Engine Room") \
    X(A02B, "a02b", "Alternate", "Engine Room") \
    X(A03A, "a03a", "Alternate", "Deck-2, Port") \
    X(A03B, "a03b", "Alternate", "Deck-2, Starboard") \
    X(A04A, "a04a", "Alternate", "Hold No.1") \
    X(A04B, "a04b", "Alternate", "Hold No.2") \
    X(A04C, "a04c", "Alternate", "Hold No.3") \
    /* Alternate Plant (Bomb Disposal / Eliminate / Hold-Up / Photograph) */ \
    X(A20C, "a20c", "Alternate", "Strut E Heliport (Bomb)") \
    X(A24F, "a24f", "Alternate", "Shell 1 Core, B1 Hall") \
    X(A24G, "a24g", "Alternate", "Shell 1 Core, B2 Computer Room") \
    X(A31B, "a31b", "Alternate", "Shell 2 Core, B1 Filtration Chamber No.1") \
    X(A41A, "a41a", "Alternate", "Arsenal Gear - Stomach") \
    X(A41B, "a41b", "Alternate", "Arsenal Gear - Stomach") \
    X(A42A, "a42a", "Alternate", "Arsenal Gear - Jejunum") \
    X(A43A, "a43a", "Alternate", "Arsenal Gear - Ascending Colon") \
    X(A45A, "a45a", "Alternate", "Arsenal Gear - Sigmoid Colon") \
    \
    /* Snake Tales (shared) */ \
    X(A12B, "a12b", "Snake Tales", "Strut A Pump Room (A Wrongdoing / Dead Man Whispers)") \
    X(A13A, "a13a", "Snake Tales", "AB Connecting Bridge (Dead Man Whispers / External Gazer)") \
    X(A13B, "a13b", "Snake Tales", "AB Connecting Bridge (Sensor B) (Dead Man Whispers / External Gazer)") \
    X(A14A, "a14a", "Snake Tales", "Strut B Transformer Room (Big Shell Evil / Dead Man Whispers / External Gazer)") \
    X(A15A, "a15a", "Snake Tales", "BC Connecting Bridge (Big Shell Evil / Dead Man Whispers / External Gazer)") \
    X(A15B, "a15b", "Snake Tales", "BC Connecting Bridge (After Stillman) (Big Shell Evil / Dead Man Whispers / External Gazer)") \
    X(A16A, "a16a", "Snake Tales", "Strut C Dining Hall (Big Shell Evil / Dead Man Whispers / External Gazer)") \
    X(A17A, "a17a", "Snake Tales", "CD Connecting Bridge (Big Shell Evil / Dead Man Whispers / External Gazer)") \
    X(A18A, "a18a", "Snake Tales", "Strut D Sediment Pool (Big Shell Evil / Dead Man Whispers)") \
    X(A19A, "a19a", "Snake Tales", "DE Connecting Bridge (Big Shell Evil / Dead Man Whispers)") \
    X(A20A, "a20a", "Snake Tales", "Strut E Parcel Room (A Wrongdoing / Big Shell Evil / Dead Man Whispers)") \
    X(A21A, "a21a", "Snake Tales", "EF Connecting Bridge (A Wrongdoing / Big Shell Evil)") \
    X(A21B, "a21b", "Snake Tales", "EF Connecting Bridge 2 (A Wrongdoing / Big Shell Evil)") \
    X(A22A, "a22a", "Snake Tales", "Strut F Warehouse (A Wrongdoing / Big Shell Evil / Dead Man Whispers)") \
    X(A23A, "a23a", "Snake Tales", "FA Connecting Bridge (A Wrongdoing / Dead Man Whispers)") \
    X(A23B, "a23b", "Snake Tales", "FA Connecting Bridge (A Wrongdoing / Dead Man Whispers)") \
    \
    /* Snake Tales (A Wrongdoing) */ \
    X(A12A, "a12a", "Snake Tales", "Strut A Roof (A Wrongdoing)") \
    X(A24A, "a24a", "Snake Tales", "Shell 1 Core, 1F (A Wrongdoing)") \
    X(A24B, "a24b", "Snake Tales", "Shell 1 Core, B1 (A Wrongdoing)") \
    X(A24C, "a24c", "Snake Tales", "Shell 1 Core, B1 Hall (A Wrongdoing)") \
    X(A24D, "a24d", "Snake Tales", "Shell 1 Core, B2 Computer Room (A Wrongdoing)") \
    \
    /* Snake Tales (Big Shell Evil) */ \
    X(A20B, "a20b", "Snake Tales", "Strut E Heliport (Big Shell Evil)") \
    X(A20E, "a20e", "Snake Tales", "Strut E Heliport (Big Shell Evil)") \
    X(A22B, "a22b", "Snake Tales", "Strut F Warehouse (Big Shell Evil)") \
    X(A25A, "a25a", "Snake Tales", "Shell 1-2 Connecting Bridge (Big Shell Evil)") \
    \
    /* Snake Tales (Dead Man Whispers) */ \
    X(A14B, "a14b", "Snake Tales", "Strut B Transformer Room (Dead Man Whispers)") \
    X(A25D, "a25d", "Snake Tales", "KL Connecting Bridge (Dead Man Whispers)") \
    X(A28A, "a28a", "Snake Tales", "Strut L Sewage Treatment Facility (Dead Man Whispers)") \
    X(A31A, "a31a", "Snake Tales", "Shell 2 Core, 1F Air Purification Room (Dead Man Whispers)") \
    X(A31C, "a31c", "Snake Tales", "Shell 2 Core, B1 Filtration Chamber No.2 (Dead Man Whispers)") \
    \
    /* Snake Tales (External Gazer) */ \
    X(A13C, "a13c", "Snake Tales", "AB Connecting Bridge (External Gazer)") \
    X(A46A, "a46a", "Snake Tales", "Arsenal Gear - Rectum (External Gazer)") \
    X(A61A, "a61a", "Snake Tales", "Federal Hall (External Gazer)") \
    X(TA00A,  "ta00a",  "Snake Tales", "Aft Deck (External Gazer)") \
    X(TA01A,  "ta01a",  "Snake Tales", "Deck-A Crew's Quarters (External Gazer)") \
    X(TA01B,  "ta01b",  "Snake Tales", "Deck-B Crew's Quarters (External Gazer)") \
    X(TA01C,  "ta01c",  "Snake Tales", "Deck-C Crew's Quarters (External Gazer)") \
    X(TA01D,  "ta01d",  "Snake Tales", "Deck-D Crew's Quarters (External Gazer)") \
    X(TA01E,  "ta01e",  "Snake Tales", "Deck-E, The Bridge (External Gazer)") \
    X(TA01F,  "ta01f",  "Snake Tales", "Deck-A Crew's Lounge (External Gazer)") \
    X(TA02A,  "ta02a",  "Snake Tales", "Engine Room (External Gazer)") \
    X(TA12A,  "ta12a",  "Snake Tales", "Strut A Roof (External Gazer)") \
    X(TA20A,  "ta20a",  "Snake Tales", "Strut E Heliport (External Gazer)") \
    X(TA22A,  "ta22a",  "Snake Tales", "Philanthropy's Hideout (External Gazer)") \
    X(TA24A,  "ta24a",  "Snake Tales", "Shell 1 Core, 1F (External Gazer)") \
    X(TA31A,  "ta31a",  "Snake Tales", "Shell 2 Core, 1F Air Purification Room (External Gazer)") \
    X(TA42A,  "ta42a",  "Snake Tales", "Arsenal Gear - Jejunum (External Gazer)") \
    X(TSP03A, "tsp03a", "Snake Tales", "VR: Variety - Level 3 (External Gazer)") \
    X(TVS03A, "tvs03a", "Snake Tales", "VR: Sneaking - Level 3 (External Gazer)") \
    X(TVS05A, "tvs05a", "Snake Tales", "VR: Sneaking - Level 5 (Eliminate All) (External Gazer)") \
    X(TVS06A, "tvs06a", "Snake Tales", "VR: Sneaking - Level 6 (External Gazer)") \
    X(TVS08A, "tvs08a", "Snake Tales", "VR: Sneaking - Level 8 (External Gazer)") \
    X(TWP03A, "twp03a", "Snake Tales", "VR: Weapons - SOCOM - Level 3 (External Gazer)") \
    X(TWP34A, "twp34a", "Snake Tales", "VR: Weapons - Claymore - Level 4 (External Gazer)") \
    X(TWP43A, "twp43a", "Snake Tales", "VR: Weapons - PSG1 - Level 3 (External Gazer)") \
    \
    /* VR: Sneaking          */ \
    X(VS01A, "vs01a", "VR: Sneaking", "Level 1") \
    X(VS02A, "vs02a", "VR: Sneaking", "Level 2") \
    X(VS03A, "vs03a", "VR: Sneaking", "Level 3") \
    X(VS04A, "vs04a", "VR: Sneaking", "Level 4") \
    X(VS05A, "vs05a", "VR: Sneaking", "Level 5") \
    X(VS06A, "vs06a", "VR: Sneaking", "Level 6") \
    X(VS07A, "vs07a", "VR: Sneaking", "Level 7") \
    X(VS08A, "vs08a", "VR: Sneaking", "Level 8") \
    X(VS09A, "vs09a", "VR: Sneaking", "Level 9") \
    X(VS10A, "vs10a", "VR: Sneaking", "Level 10") \
    \
    /* VR: Variety           */ \
    X(SP01A, "sp01a", "VR: Variety", "Athletic Course (Stage 1 & 2)") \
    X(SP02A, "sp02a", "VR: Variety", "Athletic Course (Stage 3)") \
    X(SP03A, "sp03a", "VR: Variety", "Mob Survival (Stage 7)") \
    X(SP06A, "sp06a", "VR: Variety", "Sniping Stage (Stage 4)") \
    X(SP07A, "sp07a", "VR: Variety", "Escape Stage (Stage 6)") \
    X(SP08A, "sp08a", "VR: Variety", "Dark Stage (Stage 5)") \
    \
    /* VR: First-Person      */ \
    X(SP21A, "sp21a", "VR: First-Person", "Level 1") \
    X(SP22A, "sp22a", "VR: First-Person", "Level 2 & 3") \
    X(SP24A, "sp24a", "VR: First-Person", "Level 4 (Dark Stage)") \
    X(SP25A, "sp25a", "VR: First-Person", "Level 5 (Bomb Disposal)") \
    \
    /* VR: Streaking         */ \
    X(ST01A, "st01a", "VR: Streaking", "Level 1") \
    X(ST02A, "st02a", "VR: Streaking", "Level 2") \
    X(ST03A, "st03a", "VR: Streaking", "Level 3") \
    X(ST04A, "st04a", "VR: Streaking", "Level 4") \
    X(ST05A, "st05a", "VR: Streaking", "Level 5") \
    \
    /* VR: Weapons           */ \
    X(WP01A, "wp01a", "VR: Weapons - SOCOM",   "Level 1") \
    X(WP02A, "wp02a", "VR: Weapons - SOCOM",   "Level 2") \
    X(WP03A, "wp03a", "VR: Weapons - SOCOM",   "Level 3") \
    X(WP04A, "wp04a", "VR: Weapons - SOCOM",   "Level 4") \
    X(WP05A, "wp05a", "VR: Weapons - SOCOM",   "Level 5") \
    X(WP11A, "wp11a", "VR: Weapons - M4",      "Level 1") \
    X(WP12A, "wp12a", "VR: Weapons - M4",      "Level 2") \
    X(WP13A, "wp13a", "VR: Weapons - M4",      "Level 3") \
    X(WP14A, "wp14a", "VR: Weapons - M4",      "Level 4") \
    X(WP15A, "wp15a", "VR: Weapons - M4",      "Level 5") \
    X(WP21A, "wp21a", "VR: Weapons - Claymore","Level 1") \
    X(WP22A, "wp22a", "VR: Weapons - Claymore","Level 2") \
    X(WP23A, "wp23a", "VR: Weapons - Claymore","Level 3") \
    X(WP24A, "wp24a", "VR: Weapons - Claymore","Level 4") \
    X(WP25A, "wp25a", "VR: Weapons - Claymore","Level 5") \
    X(WP31A, "wp31a", "VR: Weapons - Grenade", "Level 1") \
    X(WP32A, "wp32a", "VR: Weapons - Grenade", "Level 2") \
    X(WP33A, "wp33a", "VR: Weapons - Grenade", "Level 3") \
    X(WP34A, "wp34a", "VR: Weapons - Grenade", "Level 4") \
    X(WP35A, "wp35a", "VR: Weapons - Grenade", "Level 5") \
    X(WP41A, "wp41a", "VR: Weapons - PSG1",    "Level 1") \
    X(WP42A, "wp42a", "VR: Weapons - PSG1",    "Level 2") \
    X(WP43A, "wp43a", "VR: Weapons - PSG1",    "Level 3") \
    X(WP44A, "wp44a", "VR: Weapons - PSG1",    "Level 4") \
    X(WP45A, "wp45a", "VR: Weapons - PSG1",    "Level 5") \
    X(WP51A, "wp51a", "VR: Weapons - Stinger", "Level 1") \
    X(WP52A, "wp52a", "VR: Weapons - Stinger", "Level 2") \
    X(WP53A, "wp53a", "VR: Weapons - Stinger", "Level 3") \
    X(WP54A, "wp54a", "VR: Weapons - Stinger", "Level 4") \
    X(WP55A, "wp55a", "VR: Weapons - Stinger", "Level 5") \
    X(WP61A, "wp61a", "VR: Weapons - Nikita",  "Level 1") \
    X(WP62A, "wp62a", "VR: Weapons - Nikita",  "Level 2") \
    X(WP63A, "wp63a", "VR: Weapons - Nikita",  "Level 3") \
    X(WP64A, "wp64a", "VR: Weapons - Nikita",  "Level 4") \
    X(WP65A, "wp65a", "VR: Weapons - Nikita",  "Level 5") \
    X(WP71A, "wp71a", "VR: Weapons - No Weapon", "Level 1") \
    X(WP72A, "wp72a", "VR: Weapons - No Weapon", "Level 2") \
    X(WP73A, "wp73a", "VR: Weapons - No Weapon", "Level 3") \
    X(WP74A, "wp74a", "VR: Weapons - No Weapon", "Level 4") \
    X(WP75A, "wp75a", "VR: Weapons - No Weapon", "Level 5")

namespace MGS2Stages
{
#define X(name, id, mode, disp) inline constexpr const char* name = id;
    MGS2_STAGE_LIST
#undef X
}


#define MGS3_STAGE_LIST \
    /* Menus / Intro / Ending */ \
    X(TITLE,   "title",   "Menu", "Title Screen / Main Menu") \
    X(THEATER, "theater", "Menu", "Demo Theater") \
    X(ENDING,  "ending",  "Menu", "Credits") \
    X(SELECT,  "select",  "Menu", "Developer Menu") \
    X(NET,     "net",     "Menu", "Network Mode") \
    \
    \
    /* Virtuous Mission (VM) and early Snake Eater mirrors */ \
    X(V000A_0, "v000a_0", "Virtuous Mission", "Virtuous Mission Intro") \
    X(V000A_1, "v000a_1", "Virtuous Mission", "Virtuous Mission Intro") \
    X(V001A, "v001a", "Virtuous Mission", "Dremuchji South") \
    X(V001A_0, "v001a_0", "Virtuous Mission", "Dremuchji South") \
    X(V003A, "v003a", "Virtuous Mission", "Dremuchji Swampland") \
    X(V004A, "v004a", "Virtuous Mission", "Dremuchji North") \
    X(V004A_0, "v004a_0", "Virtuous Mission", "Dremuchji North") \
    X(V005A, "v005a", "Virtuous Mission", "Dolinovodno Rope Bridge") \
    X(V005A_0, "v005a_0", "Virtuous Mission", "Dolinovodno Rope Bridge") \
    X(V005A_1, "v005a_1", "Virtuous Mission", "Dolinovodno Rope Bridge") \
    X(V005A_2, "v005a_2", "Virtuous Mission", "Dolinovodno Rope Bridge") \
    X(V006A, "v006a", "Virtuous Mission", "Rassvet") \
    X(V006A_0, "v006a_0", "Virtuous Mission", "Rassvet") \
    X(V006B, "v006b", "Virtuous Mission", "Rassvet") \
    X(V007A, "v007a", "Virtuous Mission", "Dolinovodno Riverbank") \
    X(V007A_0, "v007a_0", "Virtuous Mission", "Dolinovodno Riverbank") \
    X(V008A_0, "v008a_0", "Virtuous Mission", "Virtuous Mission Ending") \
    \
    X(S000A_0, "s000a_0", "Snake Eater", "Snake Eater Intro") \
    X(S000A_1, "s000a_1", "Snake Eater", "Snake Eater Intro") \
    X(S001A, "s001a", "Snake Eater", "Dremuchji South") \
    X(S002A, "s002a", "Snake Eater", "Dremuchji East") \
    X(S003A, "s003a", "Snake Eater", "Dremuchji Swampland") \
    X(S004A, "s004a", "Snake Eater", "Dremuchji North") \
    X(S004A_0, "s004a_0", "Snake Eater", "Dremuchji North") \
    X(S005A, "s005a", "Snake Eater", "Dolinovodno Rope Bridge") \
    X(S006A, "s006a", "Snake Eater", "Rassvet") \
    X(S006A_0, "s006a_0", "Snake Eater", "Rassvet") \
    X(S006B, "s006b", "Snake Eater", "Rassvet") \
    X(S012A, "s012a", "Snake Eater", "Chyornyj Prud (Swamp)") \
    X(S021A, "s021a", "Snake Eater", "Bolshaya Past South") \
    X(S022A, "s022a", "Snake Eater", "Bolshaya Past North") \
    X(S023A, "s023a", "Snake Eater", "Bolshaya Past Crevice (Ocelot Battle)") \
    X(S023A_0, "s023a_0", "Snake Eater", "Bolshaya Past Crevice (Ocelot Battle)") \
    \
    /* Chyornaya Peschera (The Pain) */ \
    X(S031A, "s031a", "Snake Eater", "Chyornaya Peschera Cave Branch") \
    X(S032A, "s032a", "Snake Eater", "Chyornaya Peschera Cave") \
    X(S032B, "s032b", "Snake Eater", "Chyornaya Peschera Cave (The Pain Boss)") \
    X(S032B_0, "s032b_0", "Snake Eater", "Chyornaya Peschera Cave (The Pain Boss)") \
    X(S033A, "s033a", "Snake Eater", "Chyornaya Peschera Cave Entrance") \
    \
    /* Ponizovje (water / armory / warehouse) */ \
    X(S041A, "s041a", "Snake Eater", "Ponizovje South (Watercraft)") \
    X(S042A, "s042a", "Snake Eater", "Ponizovje Armory (SVD)") \
    X(S043A, "s043a", "Snake Eater", "Ponizovje Warehouse (Exterior)")/*(Kill The End Early)*/\
    X(S043A_0, "s043a_0", "Snake Eater", "Ponizovje Warehouse (Exterior)") \
    X(S044A, "s044a", "Snake Eater", "Ponizovje Warehouse (Interior)") \
    X(S044A_0, "s044a_0", "Snake Eater", "Ponizovje Warehouse (Interior)") \
    \
    /* Graniny Gorki (The Fear; labs) */ \
    X(KYLE_OP, "kyle_op", "Snake Eater", "Opening Snake Eater Video") \
    X(S051A, "s051a", "Snake Eater", "Graniny Gorki South (The Fear)") \
    X(S051B, "s051b", "Snake Eater", "Graniny Gorki South (The Fear)") \
    X(S051B_0, "s051b_0", "Snake Eater", "Graniny Gorki South (The Fear)") \
    X(S052A, "s052a", "Snake Eater", "Graniny Gorki Lab Exterior: Outside Walls") \
    X(S052B, "s052b", "Snake Eater", "Graniny Gorki Lab Exterior: Inside Walls") \
    X(S053A, "s053a", "Snake Eater", "Graniny Gorki Lab 1F/2F") \
    X(S054A_0, "s054a_0", "Snake Eater", "Graniny Gorki Lab Interior") \
    X(S055A, "s055a", "Snake Eater", "Graniny Gorki Lab B1 (Prison Cells)") \
    X(S056A, "s056a", "Snake Eater", "Graniny Gorki Lab B1 (Granin Basement)") \
    \
    /* Svyatogornyj / Sokrovenno (The End / Ocelot Unit) */ \
    X(S045A, "s045a", "Snake Eater", "Svyatogornyj South") \
    X(S061A, "s061a", "Snake Eater", "Svyatogornyj West") \
    X(S062A, "s062a", "Snake Eater", "Svyatogornyj East (M63 House)") \
    X(S063A, "s063a", "Snake Eater", "Sokrovenno South (The End)") \
    X(S063A_0, "s063a_0", "Snake Eater", "Sokrovenno South (The End)") \
    X(S063B, "s063b", "Snake Eater", "Sokrovenno South (Ocelot Unit)") \
    X(S064A, "s064a", "Snake Eater", "Sokrovenno West (The End, River)") \
    X(S064A_0, "s064a_0", "Snake Eater", "Sokrovenno West (The End, River)") \
    X(S064B, "s064b", "Snake Eater", "Sokrovenno West (Ocelot Unit, River)") \
    X(S065A, "s065a", "Snake Eater", "Sokrovenno North (The End's Death)") \
    X(S065A_0, "s065a_0", "Snake Eater", "Sokrovenno North (The End's Death)") \
    X(S065B, "s065b", "Snake Eater", "Sokrovenno North (To Krasnogorje Tunnel)") \
    X(S066A, "s066a", "Snake Eater", "Krasnogorje Tunnel (Ladder)") \
    \
    /* Krasnogorje Mountain approach */ \
    X(S071A, "s071a", "Snake Eater", "Krasnogorje Mountain Base") \
    X(S072A, "s072a", "Snake Eater", "Krasnogorje Mountainside (Hovercraft)") \
    X(S072B, "s072b", "Snake Eater", "Krasnogorje Mountainside (Hind)") \
    X(S073A, "s073a", "Snake Eater", "Krasnogorje Mountaintop (Before Eva)") \
    X(S073B, "s073b", "Snake Eater", "Krasnogorje Mountaintop (After Eva)") \
    X(S074A, "s074a", "Snake Eater", "Krasnogorje Mountaintop Ruins (Eva Cutscene)") \
    X(S074A_0, "s074a_0", "Snake Eater", "Krasnogorje Mountaintop Ruins (Eva Cutscene)") \
    X(S075A, "s075a", "Snake Eater", "Krasnogorje Mountaintop: Behind Ruins") \
    X(S075A_0, "s075a_0", "Snake Eater", "Krasnogorje Mountaintop: Behind Ruins") \
    \
    /* Groznyj Grad - all names explicitly prefixed */ \
    X(S081A, "s081a", "Snake Eater", "Groznyj Grad Underground Tunnel (The Fury)") \
    X(S081A_0, "s081a_0", "Snake Eater", "Groznyj Grad Underground Tunnel (The Fury)") \
    X(S091A, "s091a", "Snake Eater", "Groznyj Grad Southwest") \
    X(S091A_0, "s091a_0", "Snake Eater", "Groznyj Grad Southwest") \
    X(S091B, "s091b", "Snake Eater", "Groznyj Grad Southwest (During Escape)") \
    X(S091C, "s091c", "Snake Eater", "Groznyj Grad Southwest (Return)") \
    X(S092A, "s092a", "Snake Eater", "Groznyj Grad Northwest") \
    X(S092A_0, "s092a_0", "Snake Eater", "Groznyj Grad Northwest") \
    X(S092B, "s092b", "Snake Eater", "Groznyj Grad Northwest (During Escape)") \
    X(S092C, "s092c", "Snake Eater", "Groznyj Grad Northwest (Return)") \
    X(S093A, "s093a", "Snake Eater", "Groznyj Grad Northeast") \
    X(S093A_0, "s093a_0", "Snake Eater", "Groznyj Grad Northeast") \
    X(S093B, "s093b", "Snake Eater", "Groznyj Grad Northeast (During Escape)") \
    X(S093C, "s093c", "Snake Eater", "Groznyj Grad Northeast (Return)") \
    X(S094A, "s094a", "Snake Eater", "Groznyj Grad Southeast") \
    X(S094B, "s094b", "Snake Eater", "Groznyj Grad Southeast (During Escape)") \
    X(S094C, "s094c", "Snake Eater", "Groznyj Grad Southeast (Return)") \
    X(S101A, "s101a", "Snake Eater", "Groznyj Grad Weapons Lab: East Wing") \
    X(S101A_0, "s101a_0", "Snake Eater", "Groznyj Grad Weapons Lab: East Wing") \
    X(S101B, "s101b", "Snake Eater", "Groznyj Grad Weapons Lab: East Wing (C3 Mission)") \
    X(S111A, "s111a", "Snake Eater", "Groznyj Grad Weapons Lab: West Wing Corridor") \
    X(S111A_0, "s111a_0", "Snake Eater", "Groznyj Grad Weapons Lab: West Wing Corridor") \
    X(S112A, "s112a", "Snake Eater", "Groznyj Grad Torture Room") \
    X(S112A_0, "s112a_0", "Snake Eater", "Groznyj Grad Torture Room") \
    X(S113A, "s113a", "Snake Eater", "Groznyj Grad Sewers") \
    X(S121A, "s121a", "Snake Eater", "Groznyj Grad Weapons Lab: Main Wing") \
    X(S121A_0, "s121a_0", "Snake Eater", "Groznyj Grad Weapons Lab: Main Wing") \
    X(S121A_1, "s121a_1", "Snake Eater", "Groznyj Grad Weapons Lab: Main Wing") \
    X(S122A, "s122a", "Snake Eater", "Groznyj Grad Weapons Lab: Main Wing B1 (Volgin)") \
    X(S131A_0, "s131a_0", "Snake Eater", "Groznyj Grad (Post-Volgin)") \
    \
    /* Sorrow / Tikhogornyj */ \
    X(S141A, "s141a", "Snake Eater", "The Sorrow (Boss Battle)") \
    X(S141A_0, "s141a_0", "Snake Eater", "The Sorrow (Boss Battle)") \
    X(S151A, "s151a", "Snake Eater", "Tikhogornyj") \
    X(S151A_0, "s151a_0", "Snake Eater", "Tikhogornyj") \
    X(S152A, "s152a", "Snake Eater", "Tikhogornyj: Behind Waterfall") \
    X(S152A_0, "s152a_0", "Snake Eater", "Tikhogornyj: Behind Waterfall") \
    \
    /* Bike chase / Shagohod / Bridge */ \
    X(S161A, "s161a", "Snake Eater", "Groznyj Grad (Bike Chase 1)") \
    X(S161A_0, "s161a_0", "Snake Eater", "Groznyj Grad (Bike Chase 1)") \
    X(S161A_1, "s161a_1", "Snake Eater", "Groznyj Grad (Bike Chase 1)") \
    X(S161A_2, "s161a_2", "Snake Eater", "Groznyj Grad (Bike Chase 1)") \
    X(S162A, "s162a", "Snake Eater", "Groznyj Grad Runway South (Bike Chase 2)") \
    X(S162A_0, "s162a_0", "Snake Eater", "Groznyj Grad Runway South (Bike Chase 2)") \
    X(S163A, "s163a", "Snake Eater", "Groznyj Grad Runway (Bike Chase 3)") \
    X(S163A_0, "s163a_0", "Snake Eater", "Groznyj Grad Runway (Bike Chase 3)") \
    X(S163B, "s163b", "Snake Eater", "Groznyj Grad Runway (Shagohod Fight)") \
    X(S164A_0, "s164a_0", "Snake Eater", "Groznyj Grad Runway (Shagohod Fight)") \
    X(S171A, "s171a", "Snake Eater", "Groznyj Grad Rail Bridge (C3)") \
    X(S171A_0, "s171a_0", "Snake Eater", "Groznyj Grad Rail Bridge (C3)") \
    X(S171B, "s171b", "Snake Eater", "Groznyj Grad Rail Bridge (Shagohod Battle)") \
    X(S171B_0, "s171b_0", "Snake Eater", "Groznyj Grad Rail Bridge (Shagohod Battle)") \
    X(S181A, "s181a", "Snake Eater", "Groznyj Grad Rail Bridge North (Escape)") \
    X(S182A, "s182a", "Snake Eater", "Lazorevo South (Bike Chase)") \
    X(S183A, "s183a", "Snake Eater", "Lazorevo North (Final Bike Chase)") \
    \
    /* Finale */ \
    X(S191A, "s191a", "Snake Eater", "Zaozyorje West") \
    X(S191A_0, "s191a_0", "Snake Eater", "Zaozyorje West") \
    X(S191B, "s191b", "Snake Eater", "Zaozyorje West") \
    X(S192A, "s192a", "Snake Eater", "Zaozyorje East") \
    X(S192A_0, "s192a_0", "Snake Eater", "Zaozyorje East") \
    X(S201A, "s201a", "Snake Eater", "Rokovj Bereg (The Boss)") \
    X(S201A_0, "s201a_0", "Snake Eater", "Rokovj Bereg (The Boss)") \
    X(S201A_1, "s201a_1", "Snake Eater", "Rokovj Bereg (The Boss)") \
    X(S211A, "s211a", "Snake Eater", "Wig: Interior")\
    X(S211A_0, "s211a_0", "Snake Eater", "Wig: Interior")\
    X(S211A_1, "s211a_1", "Snake Eater", "Wig: Interior")\
    X(S211A_3, "s211a_3", "Snake Eater", "Wig: Interior")\
    X(S221A_0, "s221a_0", "Snake Eater", "Great Falls, VA")\
    X(S223A_0, "s223a_0", "Snake Eater", "Washington, D.C. / Arlington, VA") \
    X(T001A_0, "t001a_0", "Snake Eater", "? (JP STAGE)")


namespace MGS3Stages
{
#define X(name, id, mode, disp) constexpr const char* name = id;
    MGS3_STAGE_LIST
#undef X
}

#define MG1_STAGE_LIST \
    X(INIT,    "init",    "Init", "Init") \
    X(MG1,     "mg1",     "Metal Gear (MSX)", "Metal Gear") \
    X(MG2,     "mg2",     "Metal Gear 2: Solid Snake", "Metal Gear 2: Solid Snake") \
    X(R_SNA01, "r_sna01", "Snake", "Snake")

namespace MG1Stages
{
#define X(name, id, mode, disp) constexpr const char* name = id;
    MG1_STAGE_LIST
#undef X
}
