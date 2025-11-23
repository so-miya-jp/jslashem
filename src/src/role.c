/*	SCCS Id: @(#)role.c	3.4	2003/01/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985-1999. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, 2004-2008
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"


/*** Table of all roles ***/
/* According to AD&D, HD for some classes (ex. Wizard) should be smaller
 * (4-sided for wizards).  But this is not AD&D, and using the AD&D
 * rule here produces an unplayable character.  Thus I have used a minimum
 * of an 10-sided hit die for everything.  Another AD&D change: wizards get
 * a minimum strength of 4 since without one you can't teleport or cast
 * spells. --KAA
 *
 * As the wizard has been updated (wizard patch 5 jun '96) their HD can be
 * brought closer into line with AD&D. This forces wizards to use magic more
 * and distance themselves from their attackers. --LSZ
 *
 * With the introduction of races, some hit points and energy
 * has been reallocated for each race.  The values assigned
 * to the roles has been reduced by the amount allocated to
 * humans.  --KMH
 *
 * God names use a leading underscore to flag goddesses.
 */
/*JP
  ï∂éöóÒÇÃç≈èâÇÃàÍï∂éöñ⁄Çå©ÇÈÉRÅ[ÉhÇ™êèèäÇ…ë∂ç›Ç∑ÇÈÇÃÇ≈ÅC
  âpåÍñºÇécÇµÇƒÇ®Ç≠ÅD (see you.h)
*/

const struct Role roles[] = {
{	{"Archeologist", 0}, {
#if 1 /*JP*/
	"çlå√äwé“", 0}, {
#endif
#if 0 /*JP*/
	{"Digger",      0},
	{"Field Worker",0},
	{"Investigator",0},
	{"Exhumer",     0},
	{"Excavator",   0},
	{"Spelunker",   0},
	{"Speleologist",0},
	{"Collector",   0},
	{"Curator",     0} },
#else /*JP*/
	{"åäå@ÇË",	0},
	{"åªíní≤ç∏é“",      0},
	{"å§ãÜé“",      0},
	{"î≠å@é“",	0},
	{"å@çÌé“",	0},
	{"íTåüé“",	0},
	{"ì¥åAäwé“",    0},
	{"é˚èWâ∆",	0},
	{"îéï®äŸí∑",	0} },
#endif /*JP*/
	"Quetzalcoatl", "Camaxtli", "Huhetotl", /* Central American */
#if 0 /*JP*/
	"Arc", "the College of Archeology", "the Tomb of the Toltec Kings",
#else
	"Arc", "çlå√äwëÂäw", "ÉgÉãÉeÉJâ§â∆ÇÃïÊ",
#endif
	PM_ARCHEOLOGIST, NON_PM, NON_PM,
	PM_LORD_CARNARVON, PM_STUDENT, PM_MINION_OF_HUHETOTL,
	NON_PM, PM_HUMAN_MUMMY, S_SNAKE, S_MUMMY,
#if 0
	ART_WEREBANE, ART_GRAYSWANDIR,
#endif
	ART_ORB_OF_DETECTION,
	MH_HUMAN|MH_ELF|MH_DWARF|MH_GNOME|MH_HOBBIT|MH_VAMPIRE | 
	  ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL|ROLE_NEUTRAL,
	/* Str Int Wis Dex Con Cha */
	{   7, 10, 10,  7,  7,  7 },
	{  20, 20, 20, 10, 20, 10 },
	/* Init   Lower  Higher */
	{ 11, 0,  0, 8,  1, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },14,	/* Energy */
	10, 5, 0, 2, 10, A_INT, SPE_MAGIC_MAPPING,   -4
},
{	{"Barbarian", 0}, {
#if 1 /*JP*/
	"ñÏîÿêl", 0}, {
#endif
#if 0 /*JP*/
	{"Plunderer",   "Plunderess"},
	{"Pillager",    0},
	{"Bandit",      0},
	{"Brigand",     0},
	{"Raider",      0},
	{"Reaver",      0},
	{"Slayer",      0},
	{"Chieftain",   "Chieftainess"},
	{"Conqueror",   "Conqueress"} },
#else /*JP*/
	{"ìêÇﬁé“",	0},
	{"ó™íDÇ∑ÇÈé“",	0},
	{"Ç»ÇÁÇ∏é“",	0},
	{"èPåÇÇ∑ÇÈé“",	0},
	{"êNó™Ç∑ÇÈé“",	0},
	{"ã≠íDÇ∑ÇÈé“",	0},
	{"éEùCÇ∑ÇÈé“",	0},
	{"éÒóÃ",	"èóéÒóÃ"},
	{"ê™ïûâ§",	0} },
#endif /*JP*/
	"Mitra", "Crom", "Set", /* Hyborian */
#if 0 /*JP*/
	"Bar", "the Camp of the Duali Tribe", "the Duali Oasis",
#else
	"Bar", "ÉfÉÖÉAÉäë∞ÇÃÉLÉÉÉìÉv", "ÉfÉÖÉAÉäë∞ÇÃÉIÉAÉVÉX",
#endif
	PM_BARBARIAN, NON_PM, NON_PM,
	PM_PELIAS, PM_CHIEFTAIN, PM_THOTH_AMON,
	PM_OGRE, PM_TROLL, S_OGRE, S_TROLL,
#if 0
	ART_CLEAVER, ART_DEATHSWORD,
#endif
	ART_HEART_OF_AHRIMAN,
	MH_HUMAN|MH_ELF|MH_ORC|MH_WERE|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
	  ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */
	{  16,  7,  7, 15, 16,  6 },
	{  30,  6,  7, 20, 30,  7 },
	/* Init   Lower  Higher */
	{ 14, 0,  0,10,  2, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },10,	/* Energy */
	10, 14, 0, 0,  8, A_INT, SPE_HASTE_SELF,      -4
},
{	{"Caveman", "Cavewoman"}, {
#if 1 /*JP*/
	"ì¥åAêl", 0}, {
#endif
#if 0 /*JP*/
	{"Troglodyte",  0},
	{"Aborigine",   0},
	{"Wanderer",    0},
	{"Vagrant",     0},
	{"Wayfarer",    0},
	{"Roamer",      0},
	{"Nomad",       0},
	{"Rover",       0},
	{"Pioneer",     0} },
#else /*JP*/
	{"åäãèêl",	0},
	{"å¥èZñØ",	0},
	{"ï˙òQé“",	0},
	{"ó¨òQé“",	0},
	{"ó∑çsé“",	0},
	{"ï˙óVé“",	0},
	{"óVñqñØ",	0},
	{"ó¨òQé“",	0},
	{"êÊãÏé“",	0} },
#endif /*JP*/
	"Anu", "_Ishtar", "Anshar", /* Babylonian */
#if 0 /*JP*/
	"Cav", "the Caves of the Ancestors", "the Dragon's Lair",
#else
	"Cav", "å√ë„êlÇÃì¥åA", "ó≥ÇÃëÉåä",
#endif
	PM_CAVEMAN, PM_CAVEWOMAN, PM_LITTLE_DOG,
	PM_SHAMAN_KARNOV, PM_NEANDERTHAL, PM_CHROMATIC_DRAGON,
	PM_BUGBEAR, PM_HILL_GIANT, S_HUMANOID, S_GIANT,
#if 0
	ART_GIANTKILLER, ART_SKULLCRUSHER,
#endif
	ART_SCEPTRE_OF_MIGHT,
	MH_HUMAN|MH_DWARF|MH_GNOME|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
	  ROLE_LAWFUL|ROLE_NEUTRAL,
	/* Str Int Wis Dex Con Cha */
	{  10,  7,  7,  7,  8,  6 },
	{  30,  6,  7, 20, 30,  7 },
	/* Init   Lower  Higher */
	{ 14, 0,  0, 8,  2, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },10,	/* Energy */
	0, 12, 0, 1,  8, A_INT, SPE_DIG,             -4
},
{	{"Flame Mage", 0}, {
#if 1 /*JP*/
	"âäÇÃñÇèpét", 0}, {
#endif
#if 0 /*JP*/
	{"Spark",         0},   /* WAC was Igniter */
	{"Igniter",       0},
	{"Broiler",       0},   /* WAC was Igniter */
	{"Combuster",     0},   /* WAC was Torcher */
	{"Torcher",       0},
	{"Scorcher",      0},   /* WAC was Torcher */
	{"Incinerator",   0},
	{"Disintegrator", 0},   /* WAC was Incinerator */
	{"Flame-Master",  0} },
#else /*JP*/
	{"âŒâ‘ÇéUÇÁÇ∑é“",0},   /* WAC was Igniter */
	{"âŒéÌÇçÏÇÈé“",  0},
	{"Ç†Ç‘ÇÈé“",      0},   /* WAC was Igniter */
	{"îRÇ‚Ç∑é“",      0},   /* WAC was Torcher */
	{"âäÇè„Ç∞ÇÈé“",  0},
	{"èƒÇ´è≈Ç™Ç∑é“",  0},   /* WAC was Torcher */
	{"è¡ÇµíYÇ…Ç∑ÇÈé“",0},
	{"îRÇ‚ÇµêsÇ≠Ç∑é“",0},   /* WAC was Incinerator */
	{"âäÇÃéxîzé“",    0} },
#endif /*JP*/
	"Earth", "Fire", "Ash", /* Special */
#if 0 /*JP*/
	"Fla", "the great Circle of Flame", "the Water Mage's Cave",
#else
	"Fla", "âäÇÃàÃëÂÇ»ÇÈín", "êÖÇÃñÇèpétÇÃì¥åA",
#endif
	PM_FLAME_MAGE, NON_PM, PM_HELL_HOUND_PUP,
	PM_HIGH_FLAME_MAGE, PM_IGNITER, PM_WATER_MAGE,
	PM_WATER_ELEMENTAL, PM_RUST_MONSTER, S_ELEMENTAL, S_RUSTMONST,
#if 0
	ART_FIREWALL, ART_FIRE_BRAND,
#endif
	ART_CANDLE_OF_ETERNAL_FLAME,
	MH_HUMAN|MH_ELF|MH_GNOME|MH_ORC|MH_HOBBIT | 
	ROLE_MALE|ROLE_FEMALE|ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */  /* Direct copy from Wizard */
	{   7, 10,  7,  7,  7,  7 },
	{  10, 30, 10, 20, 20, 10 },
	/* Init   Lower  Higher */
	{ 10, 0,  0, 8,  1, 0 },	/* Hit points */
	{  4, 3,  0, 2,  0, 3 },12,	/* Energy */
	0, 1, 0, 
	2, 10, A_INT, SPE_FIREBALL,        -4 /* From old role.c */
},
{	{"Healer", 0}, {
#if 1 /*JP*/
	"ñÚét", 0}, {
#endif
#if 0 /*JP*/
	{"Rhizotomist",    0},
	{"Empiric",        0},
	{"Embalmer",       0},
	{"Dresser",        0},
	{"Medicus ossium", "Medica ossium"},
	{"Herbalist",      0},
	{"Magister",       "Magistra"},
	{"Physician",      0},
	{"Chirurgeon",     0} },
#else /*JP*/
	{"ñÚëêçÃÇË",      0},
	{"à„étå©èKÇ¢",	0},
	{"ä≈åÏét",	"ä≈åÏïw"},
	{"à„étèïéË",	0},
	{"ê⁄çúà„",	0},
	{"à„étéÂîC",	"ä≈åÏéÂîC"},
	{"à„äwã≥ét",	0},
	{"ì‡â»à„",	0},
	{"äOâ»à„",	0} },
#endif /*JP*/
	"_Athena", "Hermes", "Poseidon", /* Greek */
#if 0 /*JP*/
	"Hea", "the Temple of Epidaurus", "the Temple of Coeus",
#else
	"Hea", "ÉGÉsÉ_ÉEÉçÉXéõâ@", "ÉRÉCÉIÉXéõâ@",
#endif
	PM_HEALER, NON_PM, NON_PM,
	PM_HIPPOCRATES, PM_ATTENDANT, PM_CYCLOPS,
	PM_GIANT_RAT, PM_SNAKE, S_RODENT, S_YETI,
#if 0
	ART_DELUDER, ART_MIRRORBRIGHT,
#endif
	ART_STAFF_OF_AESCULAPIUS,
	MH_HUMAN|MH_GNOME|MH_ELF|MH_HOBBIT | ROLE_MALE|ROLE_FEMALE |
	ROLE_NEUTRAL,
	/* Str Int Wis Dex Con Cha */
	{   7,  7, 13,  7, 11, 16 },
	{  15, 20, 20, 15, 25, 5 },
	/* Init   Lower  Higher */
	{ 11, 0,  0, 8,  1, 0 },	/* Hit points */
	{  1, 4,  0, 1,  0, 2 },20,	/* Energy */
	10, 3,-3, 2, 10, A_WIS, SPE_CURE_SICKNESS,   -4
},
{	{"Ice Mage", 0}, {
#if 1 /*JP*/
	"ïXÇÃñÇèpét", 0}, {
#endif
#if 0 /*JP*/
	{"Cooler",        0},   /* WAC was Chiller */
	{"Condenser",     0},   /* WAC was Chiller */
	{"Chiller",       0},
	{"Froster",       0},
	{"Permafroster",  0},   /* WAC was Froster */
	{"Icer",          0},   /* WAC was Froster */
	{"Freezer",       0},
	{"Sublimer",      0},   /* WAC was Freezer */
	{"Ice-Master",    0} },
#else /*JP*/
	{"ó¡ÇµÇ≠Ç∑ÇÈé“",  0},
	{"ó‚Ç∑é“",        0},
	{"ìÄÇ¶Ç≥ÇπÇÈé“",  0},
	{"ïXåãÇ≥ÇπÇÈé“",  0},
	{"ìÄåãÇ≥ÇπÇÈé“",  0},
	{"âiãvìÄåãé“",    0},
	{"ê√é~Ç≥ÇπÇÈé“",  0},
	{"ê‚ëŒìÄåãé“",    0},
	{"ïXÇÃéxîzé“",    0} },
#endif /*JP*/
	"Air", "Frost", "Smoke", /* Special */
#if 0 /*JP*/
	"Ice", "the great Ring of Ice", "the Earth Mage's Cave",
#else
	"Ice", "ïXÇÃàÃëÂÇ»ÇÈín", "ëÂínÇÃñÇèpétÇÃì¥åA",
#endif
	PM_ICE_MAGE, NON_PM, PM_WINTER_WOLF_CUB,
	PM_HIGH_ICE_MAGE, PM_FROSTER, PM_EARTH_MAGE,
	PM_RUST_MONSTER, PM_XORN, S_RUSTMONST, S_XORN,
#if 0
	ART_DEEP_FREEZE, ART_FROST_BRAND,
#endif
	ART_STORM_WHISTLE,
	MH_HUMAN|MH_ELF|MH_GNOME|MH_ORC|MH_HOBBIT|MH_VAMPIRE | 
	ROLE_MALE|ROLE_FEMALE|ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */  /* Direct copy from Wizard */
	{   7, 10,  7,  7,  7,  7 },
	{  10, 30, 10, 20, 20, 10 },
	/* Init   Lower  Higher */
	{ 10, 0,  0, 8,  1, 0 },	/* Hit points */
	{  4, 3,  0, 2,  0, 3 },12,	/* Energy */
	0, 1, 0, 
	2, 10, A_INT, SPE_CONE_OF_COLD,    -4 /* From old role.c */
},
{	{"Knight", 0}, {
#if 1 /*JP*/
	"ãRém", 0}, {
#endif
#if 0 /*JP*/
	{"Gallant",     0},
	{"Esquire",     0},
	{"Bachelor",    0},
	{"Sergeant",    0},
	{"Knight",      0},
	{"Banneret",    0},
	{"Chevalier",   "Chevaliere"},
	{"Seignieur",   "Dame"},
	{"Paladin",     0} },
#else /*JP*/
	{"å©èKÇ¢",	0},
	{"ï‡ï∫",	0},
	{"êÌém",	"èóêÌém"},
	{"ãRï∫",	0},
	{"èdêÌém",	0},
	{"ãRém",	0},
	{"èdãRém",	0},
	{"åMãRém",	0},
	{"êπãRém",	0} },
#endif /*JP*/
	"Lugh", "_Brigit", "Manannan Mac Lir", /* Celtic */
#if 0 /*JP*/
	"Kni", "Camelot Castle", "the Isle of Glass",
#else
	"Kni", "ÉLÉÉÉÅÉçÉbÉgèÈ", "ÉKÉâÉXÇÃìá",
#endif
	PM_KNIGHT, NON_PM, PM_PONY,
	PM_KING_ARTHUR, PM_PAGE, PM_IXOTH,
	PM_QUASIT, PM_OCHRE_JELLY, S_IMP, S_JELLY,
#if 0
	ART_DRAGONBANE, ART_DEMONBANE,
#endif
	ART_MAGIC_MIRROR_OF_MERLIN,
	MH_HUMAN | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL,
	/* Str Int Wis Dex Con Cha */
	{  13,  7, 14,  8, 10, 17 },
	{  30, 15, 15, 10, 20, 10 },
	/* Init   Lower  Higher */
	{ 14, 0,  0, 8,  2, 0 },	/* Hit points */
	{  1, 4,  0, 1,  0, 2 },10,	/* Energy */
	10, 8,-2, 0,  9, A_WIS, SPE_TURN_UNDEAD,     -4
},
{	{"Monk", 0}, {
#if 1 /*JP*/
	"ïêì¨â∆", 0}, {
#endif
#if 0 /*JP*/
	{"Candidate",         0},
	{"Novice",            0},
	{"Initiate",          0},
	{"Student of Stones", 0},
	{"Student of Waters", 0},
	{"Student of Metals", 0},
	{"Student of Winds",  0},
	{"Student of Fire",   0},
	{"Master",            0} },
#else /*JP*/
	{"ì¸ñÂäÛñ]é“",	0},
	{"èâêSé“",	0},
	{"ì¸ñÂé“",	0},
	{"ìyÇÃèKÇ¢éË",	0},
	{"êÖÇÃèKÇ¢éË",	0},
	{"ã‡ÇÃèKÇ¢éË",	0},
	{"ïóÇÃèKÇ¢éË",	0},
	{"âŒÇÃèKÇ¢éË",	0},
	{"ñ∆ãñäFì`",	0} },
#endif /*JP*/
	"Shan Lai Ching", "Chih Sung-tzu", "Huan Ti", /* Chinese */
#if 0 /*JP*/
	"Mon", "the Monastery of Chan-Sune",
	  "the Monastery of the Earth-Lord",
#else
	"Mon", "É`ÉÉÉìÅEÉXÅ[èCìπâ@", "ínâ§ÇÃèCìπâ@",
#endif
	PM_MONK, NON_PM, NON_PM,
	PM_GRAND_MASTER, PM_ABBOT, PM_MASTER_KAEN,
	PM_EARTH_ELEMENTAL, PM_XORN, S_ELEMENTAL, S_XORN,
#if 0
	ART_GAUNTLETS_OF_DEFENSE, ART_WHISPERFEET,
#endif
	ART_EYES_OF_THE_OVERWORLD,
	MH_HUMAN|MH_HOBBIT | ROLE_MALE|ROLE_FEMALE |
	  ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */
	{  10,  7,  8,  8,  7,  7 },
	{  25, 10, 20, 20, 15, 10 },
	/* Init   Lower  Higher */
	{ 12, 0,  0, 8,  1, 0 },	/* Hit points */
	{  2, 2,  0, 2,  0, 2 },10,	/* Energy */
	10, 8,-2, 2, 20, A_WIS, SPE_RESTORE_ABILITY, -4
},
{	{"Necromancer", 0}, {
#if 1 /*JP*/
	"éÄêlégÇ¢", 0}, {
#endif
#if 0 /*JP*/
	{"Gravedigger",  0},
	{"Embalmer", 0},
	{"Mortician", 0},
	{"Zombie Lord", 0},
	{"Ghoul Master",0},
	{"Necromancer", 0},
	{"Necromancer", 0},
	{"Undead Master", 0},
	{"Lich Lord", 0} },
#else /*JP*/
	{"ïÊå@ÇË",  0},
	{"É~ÉCÉâêEêl", 0},
	{"ëíãVâÆ", 0},
	{"É]ÉìÉrÉçÅ[Éh", 0},
	{"ÉOÅ[ÉãÉ}ÉXÉ^Å[",0},
	{"ÉlÉNÉçÉ}ÉìÉTÅ[", 0},
	{"ÉlÉNÉçÉ}ÉìÉTÅ[", 0},
	{"ÉAÉìÉfÉbÉhÉ}ÉXÉ^Å[", 0},
	{"ÉäÉbÉ`ÉçÅ[Éh", 0} },
#endif /*JP*/
	"Nharlotep", "Zugguthobal", "Gothuulbe", /* Assorted slimy things */
#if 0 /*JP*/
	"Nec", "the Tower of the Dark Lord", "the Lair of Maugneshaagar",
#else
	"Nec", "É_Å[ÉNÉçÅ[ÉhÇÃìÉ", "É}ÉEÉOÉlÉbÉVÉÉÉKÅ[ÇÃâBÇÍâ∆",
#endif
	PM_NECROMANCER, NON_PM, PM_GHOUL,
	PM_DARK_LORD, PM_EMBALMER, PM_MAUGNESHAAGAR,
	PM_NUPPERIBO, PM_MONGBAT, S_BAT, S_IMP,
#if 0
	ART_SERPENT_S_TONGUE, ART_GRIMTOOTH,
#endif
	ART_GREAT_DAGGER_OF_GLAURGNAA,
	MH_HUMAN|MH_ELF|MH_ORC|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */  /* Direct copy from Wizard */
	{   7, 10,  7,  7,  7,  7 },
	{  10, 30, 10, 20, 20, 10 },
	/* Init   Lower  Higher */
	{ 10, 0,  0, 8,  1, 0 },	/* Hit points */
	{  4, 3,  0, 2,  0, 3 },12,	/* Energy */
	0, 1, 0, 
	2, 10, A_INT, SPE_SUMMON_UNDEAD,   -4
},
{	{"Priest", "Priestess"}, {
#if 1 /*JP*/
	"ëmóµ", "ìÚëm"}, {
#endif
#if 0 /*JP*/
	{"Aspirant",    0},
	{"Acolyte",     0},
	{"Adept",       0},
	{"Priest",      "Priestess"},
	{"Curate",      0},
	{"Canon",       "Canoness"},
	{"Lama",        0},
	{"Patriarch",   "Matriarch"},
	{"High Priest", "High Priestess"} },
#else /*JP*/
	{"èCìπém",	"èCìπèó"},
	{"éòé“",	0},
	{"éòç’",	0},
	{"ëmóµ",	"ìÚëm"},
	{"èïîCéiç’",	0},
	{"êπé“",	"êπèó"},
	{"éiã≥",	0},
	{"ëÂéiã≥",	0},
	{"ëÂëmè„",	"èóã≥çc"} },
#endif /*JP*/
	0, 0, 0,	/* chosen randomly from among the other roles */
#if 0 /*JP*/
	"Pri", "the Great Temple", "the Temple of Nalzok",
#else
	"Pri", "àÃëÂÇ»ÇÈéõâ@", "ÉiÉãÉ]ÉNéõâ@",
#endif
	PM_PRIEST, PM_PRIESTESS, NON_PM,
	PM_ARCH_PRIEST, PM_ACOLYTE, PM_NALZOK,
	PM_HUMAN_ZOMBIE, PM_WRAITH, S_ZOMBIE, S_WRAITH,
#if 0
	ART_DISRUPTER, ART_SUNSWORD,
#endif
	ART_MITRE_OF_HOLINESS,
	MH_HUMAN|MH_ELF|MH_HOBBIT | ROLE_MALE|ROLE_FEMALE |
	  ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */
	{   7,  7, 10,  7,  7,  7 },
	{  15, 10, 30, 15, 20, 10 },
	/* Init   Lower  Higher */
	{ 12, 0,  0, 8,  1, 0 },	/* Hit points */
	{  4, 3,  0, 2,  0, 2 },10,	/* Energy */
	0, 3,-2, 2, 10, A_WIS, SPE_REMOVE_CURSE,    -4
},
  /* Note:  Rogue precedes Ranger so that use of `-R' on the command line
     retains its traditional meaning. */
{	{"Rogue", 0}, {
#if 1 /*JP*/
	"ìêëØ", 0}, {
#endif
#if 0 /*JP*/
	{"Footpad",     0},
	{"Cutpurse",    0},
	{"Rogue",       0},
	{"Pilferer",    0},
	{"Robber",      0},
	{"Burglar",     0},
	{"Filcher",     0},
	{"Magsman",     "Magswoman"},
	{"Thief",       0} },
#else /*JP*/
	{"í«Ç¢ÇÕÇ¨",	0},
	{"Ç–Ç¡ÇΩÇ≠ÇË",	0},
	{"ÉXÉä",	0},
	{"Ç≤ÇÎÇ¬Ç´",	0},
	{"Ç±ÇªÇ«ÇÎ",	0},
	{"ãÛëÉ",	0},
	{"ìDñ_",	"èóìDñ_"},
	{"ã≠ìê",	0},
	{"ëÂìDñ_",	0} },
#endif
	"Issek", "Mog", "Kos", /* Nehwon */
#if 0 /*JP*/
	"Rog", "the Thieves' Guild Hall", "the Assassins' Guild Hall",
#else
	"Rog", "ìêëØÉMÉãÉh", "à√éEé“ÉMÉãÉh",
#endif
	PM_ROGUE, NON_PM, NON_PM,
	PM_MASTER_OF_THIEVES, PM_THUG, PM_MASTER_ASSASSIN,
	PM_LEPRECHAUN, PM_GUARDIAN_NAGA, S_NYMPH, S_NAGA,
#if 0
	ART_DOOMBLADE, ART_BAT_FROM_HELL,
#endif
	ART_MASTER_KEY_OF_THIEVERY,
	MH_HUMAN|MH_ORC|MH_WERE|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
	  ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */
	{   7,  7,  7, 10,  7,  6 },
	{  20, 10, 10, 30, 20, 10 },
	/* Init   Lower  Higher */
	{ 10, 0,  0, 8,  1, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },11,	/* Energy */
	10, 8, 0, 1,  9, A_INT, SPE_DETECT_TREASURE, -4
},
{	{"Ranger", 0}, {
#if 1 /*JP*/
	"ÉåÉìÉWÉÉÅ[", 0}, {
#endif
#if 0	/* OBSOLETE */
	{"Edhel",       "Elleth"},
	{"Edhel",       "Elleth"},      /* elf-maid */
	{"Ohtar",       "Ohtie"},       /* warrior */
	{"Kano",			/* commander (Q.) ['a] */
			"Kanie"},	/* educated guess, until further research- SAC */
	{"Arandur",			/* king's servant, minister (Q.) - guess */
			"Aranduriel"},	/* educated guess */
	{"Hir",         "Hiril"},       /* lord, lady (S.) ['ir] */
	{"Aredhel",     "Arwen"},       /* noble elf, maiden (S.) */
	{"Ernil",       "Elentariel"},  /* prince (S.), elf-maiden (Q.) */
	{"Elentar",     "Elentari"},	/* Star-king, -queen (Q.) */
	"Solonor Thelandira", "Aerdrie Faenya", "Lolth", /* Elven */
#endif
#if 0 /*JP*/
	{"Tenderfoot",    0},
	{"Lookout",       0},
	{"Trailblazer",   0},
	{"Reconnoiterer", "Reconnoiteress"},
	{"Scout",         0},
	{"Arbalester",    0},	/* One skilled at crossbows */
	{"Archer",        0},
	{"Sharpshooter",  0},
	{"Marksman",      "Markswoman"} },
#else /*JP*/
	{"êVïƒ",	0},
	{"å©í£ÇË",	0},
	{"êÊì±",	0},
	{"í„é@",	0},
	{"êÀåÛ",	0},
	{"ã|ï∫",	0},	/* One skilled at crossbows */
	{"íÜãâã|ï∫",	0},
	{"è„ãâã|ï∫",	0},
	{"ë_åÇâ§",	0} },
#endif /*JP*/
	"Mercury", "_Venus", "Mars", /* Roman/planets */
#if 0 /*JP*/
	"Ran", "Orion's camp", "the cave of the wumpus",
#else
	"Ran", "ÉIÉäÉIÉìÇÃÉLÉÉÉìÉv", "ÉèÉìÉpÉXÇÃì¥åA",
#endif
	PM_RANGER, NON_PM, PM_LITTLE_DOG /* Orion & canis major */,
	PM_ORION, PM_HUNTER, PM_SCORPIUS,
	PM_FOREST_CENTAUR, PM_SCORPION, S_CENTAUR, S_SPIDER,
#if 0
	0, 0,
#endif
	ART_LONGBOW_OF_DIANA,
	MH_HUMAN|MH_ELF|MH_GNOME|MH_ORC|MH_WERE|MH_HOBBIT |
	ROLE_MALE|ROLE_FEMALE | ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */
	{  13, 13, 13,  9, 13,  7 },
	{  30, 10, 10, 20, 20, 10 },
	/* Init   Lower  Higher */
	{ 13, 0,  0, 6,  1, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },12,	/* Energy */
	10, 9, 2, 1, 10, A_INT, SPE_INVISIBILITY,   -4
},
{	{"Samurai", 0}, {
#if 1 /*JP*/
	"éò", 0}, {
#endif
#if 0 /*JP*/
	{"Hatamoto",    0},  /* Banner Knight */
	{"Ronin",       0},  /* no allegiance */
	{"Ninja",       "Kunoichi"},  /* secret society */
	{"Joshu",       0},  /* heads a castle */
	{"Ryoshu",      0},  /* has a territory */
	{"Kokushu",     0},  /* heads a province */
	{"Daimyo",      0},  /* a samurai lord */
	{"Kuge",        0},  /* Noble of the Court */
	{"Shogun",      0} },/* supreme commander, warlord */
#else /*JP*/
	{"ä¯ñ{",	0},  /* Banner Knight */
	{"òQêl",	0},  /* no allegiance */
	{"îEé“",	"Ç≠ÉmàÍ"},  /* secret society */
	{"èÈéÂ",	0},  /* heads a castle */
	{"óÃéÂ",	0},  /* has a territory */
	{"çëéÂ",	0},  /* heads a province */
	{"ëÂñº",	"çòå≥"},  /* a samurai lord */
	{"åˆâ∆",	0},  /* Noble of the Court */
	{"è´åR",	"ëÂâú"} },  /* supreme commander, warlord */
#endif /*JP*/
	"_Amaterasu Omikami", "Raijin", "Susanowo", /* Japanese */
#if 0 /*JP*/
	"Sam", "the Castle of the Taro Clan", "the Shogun's Castle",
#else
	"Sam", "ëæòYàÍë∞ÇÃèÈ", "è´åRÇÃèÈ",
#endif
	PM_SAMURAI, NON_PM, PM_LITTLE_DOG,
	PM_LORD_SATO, PM_ROSHI, PM_ASHIKAGA_TAKAUJI,
	PM_WOLF, PM_STALKER, S_DOG, S_ELEMENTAL,
#if 0
	ART_SNICKERSNEE, ART_DRAGONBANE,
#endif
	ART_TSURUGI_OF_MURAMASA,
	MH_HUMAN | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL,
	/* Str Int Wis Dex Con Cha */
	{  10,  8,  7, 10, 17,  6 },
	{  30, 10,  8, 30, 14,  8 },
	/* Init   Lower  Higher */
	{ 13, 0,  0, 8,  1, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },11,	/* Energy */
	10, 10, 0, 0,  8, A_INT, SPE_CLAIRVOYANCE,    -4
},
#ifdef TOURIST
{	{"Tourist", 0}, {
#if 1 /*JP*/
	"äœåıãq", 0}, {
#endif
#if 0 /*JP*/
	{"Rambler",     0},
	{"Sightseer",   0},
	{"Excursionist",0},
	{"Peregrinator","Peregrinatrix"},
	{"Traveler",    0},
	{"Journeyer",   0},
	{"Voyager",     0},
	{"Explorer",    0},
	{"Adventurer",  0} },
#else /*JP*/
	{"ÉvÅ[ëæòY",	"ÉvÅ[éq"},
	{"äœåıãq",	0},
	{"é¸óVó∑çsé“",  0},
	{"ï’óé“",      0},
	{"ó∑çsé“",	0},
	{"ó∑êl",	0},
	{"çqäCé“",	0},
	{"íTçıé“",	0},
	{"ñ`åØé“",	0} },
#endif /*JP*/
	"Blind Io", "_The Lady", "Offler", /* Discworld */
#if 0 /*JP*/
	"Tou", "Ankh-Morpork", "the Thieves' Guild Hall",
#else
	"Tou", "ÉAÉìÉNÉÇÉãÉ|Å[ÉN", "ìêëØÉMÉãÉh",
#endif
	PM_TOURIST, NON_PM, NON_PM,
	PM_TWOFLOWER, PM_GUIDE, PM_MASTER_OF_THIEVES,
	PM_GIANT_SPIDER, PM_FOREST_CENTAUR, S_SPIDER, S_CENTAUR,
#if 0
	ART_WHISPERFEET, ART_LUCKBLADE,
#endif
	ART_YENDORIAN_EXPRESS_CARD,
	MH_HUMAN|MH_HOBBIT | ROLE_MALE|ROLE_FEMALE | ROLE_NEUTRAL,
	/* Str Int Wis Dex Con Cha */
	{   7, 10,  6,  7,  7, 10 },
	{  15, 10, 10, 15, 30, 20 },
	/* Init   Lower  Higher */
	{  8, 0,  0, 8,  0, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },14,	/* Energy */
	0, 5, 1, 2, 10, A_INT, SPE_CHARM_MONSTER,   -4
},
#endif
{	{"Undead Slayer", 0}, {
#if 1 /*JP*/
	"ÉAÉìÉfÉbÉhÉXÉåÉCÉÑÅ[", 0}, {
#endif
#if 0 /*JP*/
	{"Assistant",    0},
	{"Eliminator",   0},
	{"Eliminator",   0},
	{"Exterminator", 0},
	{"Exterminator", 0},
	{"Destroyer",   0},
	{"Vindicator",  0},
	{"Vindicator",  0},
	{"Undead Slayer", 0} },
#else /*JP*/
	{"èïéË",     0},
	{"éEùCé“",   0},
	{"éEùCé“",   0},
	{"ürñ≈é“",   0},
	{"ürñ≈é“",   0},
	{"îjâÛé“",   0},
	{"ïúèQé“",   0},
	{"ïúèQé“",   0},
	{"ïséÄÇñ≈Ç∑ÇÈé“", 0} },
#endif /*JP*/
	"Seeker", "Osiris", "Seth", /* Egyptian */
#if 0 /*JP*/
	"Und", "the Temple of Light", "the Crypt of Dracula",
#else
	"Und", "åıÇÃê_ìa", "ÉhÉâÉLÉÖÉâÇÃínâ∫é∫",
#endif
	PM_UNDEAD_SLAYER, NON_PM, NON_PM,
	PM_VAN_HELSING, PM_EXTERMINATOR, PM_COUNT_DRACULA,
	PM_HUMAN_MUMMY, PM_VAMPIRE, S_MUMMY, S_VAMPIRE,
#if 0
	ART_HOLY_SPEAR_OF_LIGHT, ART_SUNSWORD,
#endif
	ART_STAKE_OF_VAN_HELSING,
	MH_HUMAN|MH_ELF|MH_GNOME|MH_ORC|MH_WERE|MH_HOBBIT |
	ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */ /* Modified from Knight */
	{  13,  7, 14,  8, 10, 10 },
	{  20, 15, 15, 10, 20, 10 },
	/* Init   Lower  Higher */
	{ 14, 0,  0, 8,  2, 0 },	/* Hit points */
	{  1, 4,  0, 1,  0, 2 },10,	/* Energy */
	10, 8,-2, 0,  9, A_WIS, SPE_TURN_UNDEAD,     -4
},
{	{"Valkyrie", 0}, {
#if 1 /*JP*/
	"ÉèÉãÉLÉÖÅ[Éå", 0}, {
#endif
#if 0 /*JP*/
	{"Stripling",   0},
	{"Skirmisher",  0},
	{"Fighter",     0},
	{"Man-at-arms", "Woman-at-arms"},
	{"Warrior",     0},
	{"Swashbuckler",0},
	{"Hero",        "Heroine"},
	{"Champion",    0},
	{"Lord",        "Lady"} },
#else /*JP*/
	{"å©èKÇ¢",	0},
	{"êÀåÛ",	0},
	{"êÌém",	"èóêÌém"},
	{"ãRï∫",    "åïÇÃâ≥èó"},
	{"êÌì¨ï∫",	"êÌâ≥èó"},
	{"ìÀåÇï∫",  "èÇéùÇ¬â≥èó"},
	{"âpóY",	0},
	{"ã≠é“",	"èóåÜ"},
	{"ãMêl",	"ãMïwêl"} },
#endif /*JP*/
	"Tyr", "Odin", "Loki", /* Norse */
#if 0 /*JP*/
	"Val", "the Shrine of Destiny", "the cave of Surtur",
#else
	"Val", "â^ñΩÇÃêπì∞", "ÉXÉãÉgÇÃì¥åA",
#endif
	PM_VALKYRIE, NON_PM, NON_PM /*PM_WINTER_WOLF_CUB*/,
	PM_NORN, PM_WARRIOR, PM_LORD_SURTUR,
	PM_FIRE_ANT, PM_FIRE_GIANT, S_ANT, S_GIANT,
#if 0
	ART_MJOLLNIR, ART_FROST_BRAND,
#endif
	ART_ORB_OF_FATE,
	MH_HUMAN|MH_DWARF | ROLE_FEMALE | ROLE_LAWFUL|ROLE_NEUTRAL,
	/* Str Int Wis Dex Con Cha */
	{  10,  7,  7,  7, 12,  7 },
	{  30,  6,  7, 20, 30,  7 },
	/* Init   Lower  Higher */
	{ 14, 0,  0, 8,  2, 0 },	/* Hit points */
	{  1, 0,  0, 1,  0, 1 },10,	/* Energy */
	0, 10,-2, 0,  9, A_WIS, SPE_LIGHTNING,    -4
},
{	{"Wizard", 0}, {
#if 1 /*JP*/
	"ñÇñ@égÇ¢", 0}, {
#endif
#if 0 /*JP*/
	{"Evoker",      0},
	{"Conjurer",    0},
	{"Thaumaturge", 0},
	{"Magician",    0},
	{"Warlock",     "Witch"},
	{"Enchanter",   "Enchantress"},
	{"Sorcerer",    "Sorceress"},
	{"Wizard",      0},
	{"Mage",        0} },
#else /*JP*/
	{"éËïiét",	0},
	{"äÔèpét",	0},
	{"éÙèpét",	0},
	{"óÏî}ét",	0},
	{"ñÇêl",	"ñÇèó"},
	{"è¢ä´ét",	0},
	{"ñ@èpét",      0},
	{"ñÇñ@égÇ¢",      0},
	{"ëÂñÇñ@égÇ¢",	0} },
#endif /*JP*/
	"Ptah", "Thoth", "Anhur", /* Egyptian */
#if 0 /*JP*/
	"Wiz", "the Lonely Tower", "the Tower of Darkness",
#else
	"Wiz", "í≤òaÇÃìÉ", "à√çïÇÃìÉ",
#endif
	PM_WIZARD, NON_PM, PM_KITTEN,
	PM_NEFERET_THE_GREEN, PM_APPRENTICE, PM_DARK_ONE,
	PM_VAMPIRE_BAT, PM_XORN, S_BAT, S_WRAITH,
#if 0
	ART_MAGICBANE, ART_DELUDER,
#endif
	ART_EYE_OF_THE_AETHIOPICA,
	MH_HUMAN|MH_ELF|MH_GNOME|MH_ORC|MH_HOBBIT|MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE |
	  ROLE_NEUTRAL|ROLE_CHAOTIC,
	/* Str Int Wis Dex Con Cha */
	{   7, 10,  7,  7,  7,  7 },
	{  10, 30, 10, 20, 20, 10 },
	/* Init   Lower  Higher */
	{ 10, 0,  0, 8,  1, 0 },	/* Hit points */
	{  4, 3,  0, 2,  0, 3 },12,	/* Energy */
	0, 1, 0, 3, 10, A_INT, SPE_MAGIC_MISSILE,   -4
},
#ifdef YEOMAN
{	{"Yeoman", 0}, {
#if 1 /*JP*/
	"âqém", 0}, {
#endif
	/* Landowner titles, naval ranks and positions */
	/* We intentionally avoid Lieutenant and Captain */
#if 0 /*JP*/
	{"Usher",          0},
	{"Steward",        "Stewardess"},
	{"Keeper",         0},
	{"Marshal",        0},
	{"Master Steward", "Master Stewardess"},
	{"Chamberlain",    0},
	{"Constable",      0},
	{"Chancellor",     0},
	{"Regent",         0} },
#else
	{"àƒì‡åW",         0},
	{"ê¢òbñ",         "ÉÅÉCÉh"},
	{"éÁâq",           0},
	{"ãVéÆåW",         0},
	{"é∑éñí∑",         "ÉÅÉCÉhí∑"},
	{"éòè]",           0},
	{"èÈéÁ",           0},
	{"éiñ@äØ",         0},
	{"ê€ê≠",         0} },
#endif
	"His Majesty", "His Holiness", "The Commons", /* The three estates */
#if 0 /*JP*/
	"Yeo", "London", "the inner ward",
#else
	"Yeo", "ÉçÉìÉhÉì", "íÜêïóÃàÊ",
#endif
	PM_YEOMAN, NON_PM, PM_PONY,
	PM_CHIEF_YEOMAN_WARDER, PM_YEOMAN_WARDER, PM_COLONEL_BLOOD,
	PM_RAVEN, PM_WEREWOLF, S_RODENT, S_DOG,
#if 0
	ART_REAPER, ART_SWORD_OF_JUSTICE,
#endif
	ART_CROWN_OF_SAINT_EDWARD,
	MH_HUMAN|MH_ELF|MH_HOBBIT | ROLE_MALE|ROLE_LAWFUL,
	/* Str Int Wis Dex Con Cha */
	{  12,  7, 10, 12, 12,  7 },
	{  20, 15, 15, 10, 20, 10 },
	/* Init   Lower  Higher */
	{ 14, 0,  0, 8,  2, 0 },	/* Hit points */
	{  1, 4,  0, 1,  0, 2 },10,	/* Energy */
	10, 8,-2, 0,  9, A_WIS, SPE_KNOCK,     -4
},
#endif
/* Array terminator */
{{0, 0}}
};


/* The player's role, created at runtime from initial
 * choices.  This may be munged in role_init().
 */
struct Role urole =
#if 0 /*JP*/
{	{"Undefined", 0}, { {0, 0}, {0, 0}, {0, 0},
#else
{	{"Undefined", 0}, {"ì‰", 0}, { {0, 0}, {0, 0}, {0, 0},
#endif
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
	"L", "N", "C", "Xxx", "home", "locate",
	NON_PM, NON_PM, NON_PM, NON_PM, NON_PM, NON_PM,
	NON_PM, NON_PM, 0, 0, 
#if 0
	0, 0,
#endif
	0, 0,
	/* Str Int Wis Dex Con Cha */
	{   7,  7,  7,  7,  7,  7 },
	{  20, 15, 15, 20, 20, 10 },
	/* Init   Lower  Higher */
	{ 10, 0,  0, 8,  1, 0 },	/* Hit points */
	{  2, 0,  0, 2,  0, 3 },14,	/* Energy */
	0, 10, 0, 0,  4, A_INT, 0, -3
};

/* Table of all races */
const struct Race races[] = {
#if 0 /*JP*/
{	"doppelganger", "doppelganger", "doppelganger-kind", "Dop",
#else
{	"doppelganger", "ÉhÉbÉyÉãÉQÉìÉKÅ[", "doppelganger", "doppelganger-kind", "Dop",
#endif
	{0, 0},
	PM_DOPPELGANGER, NON_PM, PM_HUMAN_MUMMY, PM_HUMAN_ZOMBIE,
	MH_HUMAN | ROLE_MALE|ROLE_FEMALE | ROLE_NEUTRAL|ROLE_CHAOTIC,
	MH_HUMAN, MH_WERE, MH_ELF|MH_GNOME|MH_DWARF,
	/*    Str     Int Wis Dex Con Cha */
	{      1,      3,  3,  1,  1,  1 },
	{ STR18(100), 20, 20, 20, 20, 15 },
	/* Init   Lower  Higher */
	{  0, 0,  0, 1,  1, 0 },	/* Hit points */
	{  7, 0,  5, 0,  5, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"drow", "droven", "drovenkind", "Dro",
#else
{	"drow", "ÉhÉçÉE", "droven", "drovenkind", "Dro",
#endif
	{0, 0},
	PM_DROW, NON_PM, PM_ELF_MUMMY, PM_ELF_ZOMBIE,
	MH_ELF | ROLE_MALE|ROLE_FEMALE | ROLE_CHAOTIC,
	MH_ELF, MH_ELF, MH_ORC,
	/*  Str    Int Wis Dex Con Cha */
	{    3,     3,  3,  3,  3,  3 },
	{   18,    20, 20, 18, 16, 18 },
	/* Init   Lower  Higher */
	{  1, 0,  0, 1,  1, 0 },	/* Hit points */
	{  2, 0,  3, 0,  3, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"dwarf", "dwarven", "dwarvenkind", "Dwa",
#else
{	"dwarf", "ÉhÉèÅ[Ét", "dwarven", "dwarvenkind", "Dwa",
#endif
	{0, 0},
	PM_DWARF, NON_PM, PM_DWARF_MUMMY, PM_DWARF_ZOMBIE,
	MH_DWARF | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL,
	MH_DWARF, MH_DWARF|MH_GNOME, MH_ORC,
	/*    Str     Int Wis Dex Con Cha */
	{      3,      3,  3,  3,  3,  3 },
	{ STR18(100), 16, 16, 20, 20, 16 },
	/* Init   Lower  Higher */
	{  4, 0,  0, 3,  2, 0 },	/* Hit points */
	{  0, 0,  0, 0,  0, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"elf", "elven", "elvenkind", "Elf",
#else
{	"elf", "ÉGÉãÉt", "elven", "elvenkind", "Elf",
#endif
	{0, 0},
	PM_ELF, NON_PM, PM_ELF_MUMMY, PM_ELF_ZOMBIE,
	MH_ELF | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL|ROLE_NEUTRAL,
	MH_ELF, MH_ELF, MH_ORC,
	/*  Str    Int Wis Dex Con Cha */
	{    3,     3,  3,  3,  3,  3 },
	{   18,    20, 20, 18, 16, 18 },
	/* Init   Lower  Higher */
	{  1, 0,  0, 1,  1, 0 },	/* Hit points */
	{  2, 0,  3, 0,  3, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"gnome", "gnomish", "gnomehood", "Gno",
#else
{	"gnome", "ÉmÅ[ÉÄ", "gnomish", "gnomehood", "Gno",
#endif
	{0, 0},
	PM_GNOME, NON_PM, PM_GNOME_MUMMY, PM_GNOME_ZOMBIE,
	MH_GNOME | ROLE_MALE|ROLE_FEMALE | ROLE_NEUTRAL,
	MH_GNOME, MH_DWARF|MH_GNOME, MH_HUMAN|MH_HOBBIT,
	/*  Str    Int Wis Dex Con Cha */
	{    3,     3,  3,  3,  3,  3 },
	{STR18(50),19, 18, 18, 18, 18 },
	/* Init   Lower  Higher */
	{  1, 0,  0, 1,  0, 0 },	/* Hit points */
	{  2, 0,  2, 0,  2, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"hobbit", "hobbit", "hobbit-kind", "Hob",
#else
{	"hobbit", "ÉzÉrÉbÉg", "hobbit", "hobbit-kind", "Hob",
#endif
	{0, 0},
	PM_HOBBIT, NON_PM, NON_PM, NON_PM,
	MH_HOBBIT | ROLE_MALE|ROLE_FEMALE | ROLE_LAWFUL|ROLE_NEUTRAL,
	MH_HOBBIT, MH_HOBBIT, MH_GNOME|MH_ORC,
	/*  Str    Int Wis Dex Con Cha */
	{   3,      3,  3,  3,  3,  3 },
	{  18,     16, 18, 18, 20, 20 },
	/* Init   Lower  Higher */
	{  2, 0,  0, 2,  1, 0 },	/* Hit points */
	{  2, 0,  2, 1,  2, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"human", "human", "humanity", "Hum",
	{"man", "woman"},
#else
{	"human", "êlä‘", "human", "humanity", "Hum",
	{"íj", "èó"},
#endif
	PM_HUMAN, NON_PM, PM_HUMAN_MUMMY, PM_HUMAN_ZOMBIE,
	MH_HUMAN | ROLE_MALE|ROLE_FEMALE |
	  ROLE_LAWFUL|ROLE_NEUTRAL|ROLE_CHAOTIC,
	MH_HUMAN, 0, MH_GNOME|MH_ORC,
	/*    Str     Int Wis Dex Con Cha */
	{      3,      3,  3,  3,  3,  3 },
	{ STR18(100), 18, 18, 18, 18, 18 },
	/* Init   Lower  Higher */
	{  2, 0,  0, 2,  1, 0 },	/* Hit points */
	{  1, 0,  2, 0,  2, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"lycanthrope", "lycanthropic", "lycanthropehood", "Lyc",
#else
{	"lycanthrope", "èbêl", "lycanthropic", "lycanthropehood", "Lyc",
#endif
	{0, 0},
	PM_HUMAN_WEREWOLF, NON_PM, PM_HUMAN_MUMMY, PM_HUMAN_ZOMBIE,
	MH_WERE | ROLE_MALE|ROLE_FEMALE | ROLE_CHAOTIC,
	MH_WERE, 0, MH_ELF|MH_GNOME|MH_DWARF,
	/*    Str     Int Wis Dex Con Cha */
	{      4,      1,  1,  4,  4,  2 },
	{ STR19(19), 15, 15, 20, 19, 15 },
	/* Init   Lower  Higher */
	{  4, 0,  0, 2,  2, 0 },	/* Hit points */
	{  5, 0,  4, 0,  4, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"orc", "orcish", "orcdom", "Orc",
#else
{	"orc", "ÉIÅ[ÉN", "orcish", "orcdom", "Orc",
#endif
	{0, 0},
	PM_ORC, NON_PM, PM_ORC_MUMMY, PM_ORC_ZOMBIE,
	MH_ORC | ROLE_MALE|ROLE_FEMALE | ROLE_CHAOTIC,
	MH_ORC, 0, MH_HUMAN|MH_ELF|MH_DWARF|MH_HOBBIT,
	/*  Str    Int Wis Dex Con Cha */
	{   3,      3,  3,  3,  3,  3 },
	{STR18(50),16, 16, 18, 18, 16 },
	/* Init   Lower  Higher */
	{  1, 0,  0, 1,  0, 0 },	/* Hit points */
	{  1, 0,  1, 0,  1, 0 }		/* Energy */
},
#if 0 /*JP*/
{	"vampire", "vampiric", "vampirehood", "Vam",
#else
{	"vampire", "ãzååãS", "vampiric", "vampirehood", "Vam",
#endif
	{0, 0},
	PM_VAMPIRE, NON_PM, PM_HUMAN_MUMMY, PM_HUMAN_ZOMBIE,
	MH_VAMPIRE | ROLE_MALE|ROLE_FEMALE | ROLE_CHAOTIC,
	MH_VAMPIRE, 0, MH_ELF|MH_GNOME|MH_HOBBIT|MH_DWARF|MH_ORC,
	/*    Str     Int Wis Dex Con Cha */
	{      4,      0,  0,  4,  3,  4 },
	{ STR19(19), 18, 18, 20, 20, 20 },
	/* Init   Lower  Higher */
	{  3, 0,  0, 3,  2, 0 },	/* Hit points */
	{  3, 0,  4, 0,  4, 0 }		/* Energy */
},
/* Array terminator */
{ 0, 0, 0, 0 }};


/* The player's race, created at runtime from initial
 * choices.  This may be munged in role_init().
 */
struct Race urace =
#if 0 /*JP*/
{	"something", "undefined", "something", "Xxx",
#else
{	"something", "ì‰", "undefined", "something", "Xxx",
#endif
	{0, 0},
	NON_PM, NON_PM, NON_PM, NON_PM,
	0, 0, 0, 0,
	/*    Str     Int Wis Dex Con Cha */
	{      3,      3,  3,  3,  3,  3 },
	{ STR18(100), 18, 18, 18, 18, 18 },
	/* Init   Lower  Higher */
	{  2, 0,  0, 2,  1, 0 },	/* Hit points */
	{  1, 0,  2, 0,  2, 0 }		/* Energy */
};

/* Table of all genders */
const struct Gender genders[] = {
#if 0 /*JP*/
	{"male",	"he",	"him",	"his",	"Mal",	ROLE_MALE},
	{"female",	"she",	"her",	"her",	"Fem",	ROLE_FEMALE},
	{"neuter",	"it",	"it",	"its",	"Ntr",	ROLE_NEUTER}
#else
	/*JP: ÉIÉvÉVÉáÉìÇ≈égÇ¡ÇƒÇ¢ÇÈÇÃÇ≈âpåÍñºÇécÇµÇƒÇ®Ç≠ÅD (see you.h) */
	{"male",	"íjê´",	"îﬁ",	"îﬁ",	"îﬁÇÃ",	"Mal",	ROLE_MALE},
	{"female",	"èóê´",	"îﬁèó",	"îﬁèó",	"îﬁèóÇÃ","Fem",	ROLE_FEMALE},
	{"neuter",	"íÜê´",	"ÇªÇÍ",	"ÇªÇÍ",	"ÇªÇÃ",	"Ntr",	ROLE_NEUTER}
#endif
};

#ifdef MAC_MPW
const size_t maxGender = sizeof genders/sizeof genders[0];
#endif /* MAC_MPW */

/* Table of all alignments */
const struct Align aligns[] = {
#if 0 /*JP*/
	{"law",		"lawful",	"Law",	ROLE_LAWFUL,	A_LAWFUL},
	{"balance",	"neutral",	"Neu",	ROLE_NEUTRAL,	A_NEUTRAL},
	{"chaos",	"chaotic",	"Cha",	ROLE_CHAOTIC,	A_CHAOTIC},
	{"evil",	"unaligned",	"Una",	0,		A_NONE}
#else
	{"law",		"íÅèò",	"lawful",	"Law",	ROLE_LAWFUL,	A_LAWFUL},
	{"balance",	"íÜóß",	"neutral",	"Neu",	ROLE_NEUTRAL,	A_NEUTRAL},
	{"chaos",	"ç¨ì◊",	"chaotic",	"Cha",	ROLE_CHAOTIC,	A_CHAOTIC},
	{"evil",	"ñ≥êS",	"unaligned",	"Una",	0,		A_NONE}
#endif
};

STATIC_DCL char * FDECL(promptsep, (char *, int));
STATIC_DCL int FDECL(role_gendercount, (int));
STATIC_DCL int FDECL(race_alignmentcount, (int));

/* used by str2XXX() */
static char NEARDATA randomstr[] = "random";

#ifdef MAC_MPW
const size_t maxAlign = sizeof aligns/sizeof aligns[0];
#endif /* MAC_MPW */

boolean
validrole(rolenum)
	int rolenum;
{
	return (rolenum >= 0 && rolenum < SIZE(roles)-1);
}


int
randrole()
{
	return (rn2(SIZE(roles)-1));
}


int
str2role(str)
	char *str;
{
	int i, len;

	/* Is str valid? */
	if (!str || !str[0])
	    return ROLE_NONE;

	/* Match as much of str as is provided */
	len = strlen(str);
	for (i = 0; roles[i].name.m; i++) {
	    /* Does it match the male name? */
	    if (!strncmpi(str, roles[i].name.m, len))
		return i;
	    /* Or the female name? */
	    if (roles[i].name.f && !strncmpi(str, roles[i].name.f, len))
		return i;
	    /* Or the filecode? */
	    if (!strcmpi(str, roles[i].filecode))
		return i;
	}

	if ((len == 1 && (*str == '*' || *str == '@')) ||
		!strncmpi(str, randomstr, len))
	    return ROLE_RANDOM;

	/* Couldn't find anything appropriate */
	return ROLE_NONE;
}


boolean
validrace(rolenum, racenum)
	int rolenum, racenum;
{
	/* Assumes validrole */
	/* WAC -- checks ROLE_GENDMASK and ROLE_ALIGNMASK as well (otherwise, there 
	 * might not be an allowed gender or alignment for that role
	 */
	return (racenum >= 0 && racenum < SIZE(races)-1 &&
		(roles[rolenum].allow & races[racenum].allow & ROLE_RACEMASK) &&
		(roles[rolenum].allow & races[racenum].allow & ROLE_GENDMASK) &&
		(roles[rolenum].allow & races[racenum].allow & ROLE_ALIGNMASK));
}


int
randrace(rolenum)
	int rolenum;
{
	int i, n = 0;

	/* Count the number of valid races */
	for (i = 0; races[i].noun; i++)
/*	    if (roles[rolenum].allow & races[i].allow & ROLE_RACEMASK)*/
	    if (validrace(rolenum,i))
	    	n++;

	/* Pick a random race */
	/* Use a factor of 100 in case of bad random number generators */
	if (n) n = rn2(n*100)/100;
	for (i = 0; races[i].noun; i++)
/*	    if (roles[rolenum].allow & races[i].allow & ROLE_RACEMASK) {*/
	    if (validrace(rolenum,i)) {
	    	if (n) n--;
	    	else return (i);
	    }

	/* This role has no permitted races? */
	return (rn2(SIZE(races)-1));
}

/*
 * [ALI] Find the player equivalent race for a monster from its M2 flags.
 */

int
mrace2race(mflags2)
	int mflags2;
{
	int i;

	/* Look for a race with the correct selfmask */
	for (i = 0; races[i].noun; i++)
	    if (mflags2 & races[i].selfmask) {
		/* Where more than one player race has the same monster race,
		 * return the base race.
		 */
		if (mflags2 & MH_HUMAN && races[i].malenum != PM_HUMAN)
		    continue;
		if (mflags2 & MH_ELF && races[i].malenum != PM_ELF)
		    continue;
		return i;
	    }
	return ROLE_NONE;
}

int
str2race(str)
	char *str;
{
	int i, len;

	/* Is str valid? */
	if (!str || !str[0])
	    return ROLE_NONE;

	/* Match as much of str as is provided */
	len = strlen(str);
	for (i = 0; races[i].noun; i++) {
	    /* Does it match the noun? */
	    if (!strncmpi(str, races[i].noun, len))
		return i;
	    /* Or the filecode? */
	    if (!strcmpi(str, races[i].filecode))
		return i;
	}

	if ((len == 1 && (*str == '*' || *str == '@')) ||
		!strncmpi(str, randomstr, len))
	    return ROLE_RANDOM;

	/* Couldn't find anything appropriate */
	return ROLE_NONE;
}


boolean
validgend(rolenum, racenum, gendnum)
	int rolenum, racenum, gendnum;
{
	/* Assumes validrole and validrace */
	return (gendnum >= 0 && gendnum < ROLE_GENDERS &&
		(roles[rolenum].allow & races[racenum].allow &
		 genders[gendnum].allow & ROLE_GENDMASK));
}


int
randgend(rolenum, racenum)
	int rolenum, racenum;
{
	int i, n = 0;

	/* Count the number of valid genders */
	for (i = 0; i < ROLE_GENDERS; i++)
/*	    if (roles[rolenum].allow & races[racenum].allow &
	    		genders[i].allow & ROLE_GENDMASK) */
	    if (validgend(rolenum, racenum, i))
	    	n++;

	/* Pick a random gender */
	if (n) n = rn2(n);
	for (i = 0; i < ROLE_GENDERS; i++)
/*	    if (roles[rolenum].allow & races[racenum].allow &
	    		genders[i].allow & ROLE_GENDMASK) {*/
	    if (validgend(rolenum, racenum, i)) {
	    	if (n) n--;
	    	else return (i);
	    }

	/* This role/race has no permitted genders? */
	return (rn2(ROLE_GENDERS));
}


int
str2gend(str)
	char *str;
{
	int i, len;

	/* Is str valid? */
	if (!str || !str[0])
	    return ROLE_NONE;

	/* Match as much of str as is provided */
	len = strlen(str);
	for (i = 0; i < ROLE_GENDERS; i++) {
	    /* Does it match the adjective? */
	    if (!strncmpi(str, genders[i].adj, len))
		return i;
	    /* Or the filecode? */
	    if (!strcmpi(str, genders[i].filecode))
		return i;
	}
	if ((len == 1 && (*str == '*' || *str == '@')) ||
		!strncmpi(str, randomstr, len))
	    return ROLE_RANDOM;

	/* Couldn't find anything appropriate */
	return ROLE_NONE;
}


boolean
validalign(rolenum, racenum, alignnum)
	int rolenum, racenum, alignnum;
{
	/* Assumes validrole and validrace */
	return (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
		(roles[rolenum].allow & races[racenum].allow &
		 aligns[alignnum].allow & ROLE_ALIGNMASK));
}


int
randalign(rolenum, racenum)
	int rolenum, racenum;
{
	int i, n = 0;

	/* Count the number of valid alignments */
	for (i = 0; i < ROLE_ALIGNS; i++)
	    if (roles[rolenum].allow & races[racenum].allow &
	    		aligns[i].allow & ROLE_ALIGNMASK)
	    	n++;

	/* Pick a random alignment */
	if (n) n = rn2(n);
	for (i = 0; i < ROLE_ALIGNS; i++)
	    if (roles[rolenum].allow & races[racenum].allow &
	    		aligns[i].allow & ROLE_ALIGNMASK) {
	    	if (n) n--;
	    	else return (i);
	    }

	/* This role/race has no permitted alignments? */
	return (rn2(ROLE_ALIGNS));
}


int
str2align(str)
	char *str;
{
	int i, len;

	/* Is str valid? */
	if (!str || !str[0])
	    return ROLE_NONE;

	/* Match as much of str as is provided */
	len = strlen(str);
	for (i = 0; i < ROLE_ALIGNS; i++) {
	    /* Does it match the adjective? */
	    if (!strncmpi(str, aligns[i].adj, len))
		return i;
	    /* Or the filecode? */
	    if (!strcmpi(str, aligns[i].filecode))
		return i;
	}
	if ((len == 1 && (*str == '*' || *str == '@')) ||
		!strncmpi(str, randomstr, len))
	    return ROLE_RANDOM;

	/* Couldn't find anything appropriate */
	return ROLE_NONE;
}

/* is rolenum compatible with any racenum/gendnum/alignnum constraints? */
boolean
ok_role(rolenum, racenum, gendnum, alignnum)
int rolenum, racenum, gendnum, alignnum;
{
    int i;
    short allow;

    if (rolenum >= 0 && rolenum < SIZE(roles)-1) {
	allow = roles[rolenum].allow;

	if (racenum >= 0 && racenum < SIZE(races)-1)
	    allow &= races[racenum].allow;
	if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
		!(allow & genders[gendnum].allow & ROLE_GENDMASK))
	    return FALSE;
	if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
		!(allow & aligns[alignnum].allow & ROLE_ALIGNMASK))
	    return FALSE;

	if (!(allow & ROLE_RACEMASK) || !(allow & ROLE_GENDMASK) ||
		!(allow & ROLE_ALIGNMASK))
	    return FALSE;
	return TRUE;
    } else {
	for (i = 0; i < SIZE(roles)-1; i++) {
	    allow = roles[i].allow;
	    if (racenum >= 0 && racenum < SIZE(races)-1)
		allow &= races[racenum].allow;
	    if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
		    !(allow & genders[gendnum].allow & ROLE_GENDMASK))
		continue;
	    if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
		    !(allow & aligns[alignnum].allow & ROLE_ALIGNMASK))
		continue;
	    if (!(allow & ROLE_RACEMASK) || !(allow & ROLE_GENDMASK) ||
		    !(allow & ROLE_ALIGNMASK))
		continue;
	    return TRUE;
	}
	return FALSE;
    }
}

/* pick a random role subject to any racenum/gendnum/alignnum constraints */
/* If pickhow == PICK_RIGID a role is returned only if there is  */
/* a single possibility */
int
pick_role(racenum, gendnum, alignnum, pickhow)
int racenum, gendnum, alignnum, pickhow;
{
    int i;
    int roles_ok = 0;

    for (i = 0; i < SIZE(roles)-1; i++) {
	if (ok_role(i, racenum, gendnum, alignnum))
	    roles_ok++;
    }
    if (roles_ok == 0 || (roles_ok > 1 && pickhow == PICK_RIGID))
	return ROLE_NONE;
    roles_ok = rn2(roles_ok);
    for (i = 0; i < SIZE(roles)-1; i++) {
	if (ok_role(i, racenum, gendnum, alignnum)) {
	    if (roles_ok == 0)
		return i;
	    else
		roles_ok--;
	}
    }
    return ROLE_NONE;
}

/* is racenum compatible with any rolenum/gendnum/alignnum constraints? */
boolean
ok_race(rolenum, racenum, gendnum, alignnum)
int rolenum, racenum, gendnum, alignnum;
{
    int i;
    short allow;

    if (racenum >= 0 && racenum < SIZE(races)-1) {
	allow = races[racenum].allow;

	if (rolenum >= 0 && rolenum < SIZE(roles)-1)
	    allow &= roles[rolenum].allow;
	if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
		!(allow & genders[gendnum].allow & ROLE_GENDMASK))
	    return FALSE;
	if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
		!(allow & aligns[alignnum].allow & ROLE_ALIGNMASK))
	    return FALSE;

	if (!(allow & ROLE_RACEMASK) || !(allow & ROLE_GENDMASK) ||
		!(allow & ROLE_ALIGNMASK))
	    return FALSE;
	return TRUE;
    } else {
	for (i = 0; i < SIZE(races)-1; i++) {
	    allow = races[i].allow;
	    if (rolenum >= 0 && rolenum < SIZE(roles)-1)
		allow &= roles[rolenum].allow;
	    if (gendnum >= 0 && gendnum < ROLE_GENDERS &&
		    !(allow & genders[gendnum].allow & ROLE_GENDMASK))
		continue;
	    if (alignnum >= 0 && alignnum < ROLE_ALIGNS &&
		    !(allow & aligns[alignnum].allow & ROLE_ALIGNMASK))
		continue;
	    if (!(allow & ROLE_RACEMASK) || !(allow & ROLE_GENDMASK) ||
		    !(allow & ROLE_ALIGNMASK))
		continue;
	    return TRUE;
	}
	return FALSE;
    }
}

/* pick a random race subject to any rolenum/gendnum/alignnum constraints */
/* If pickhow == PICK_RIGID a race is returned only if there is  */
/* a single possibility */
int
pick_race(rolenum, gendnum, alignnum, pickhow)
int rolenum, gendnum, alignnum, pickhow;
{
    int i;
    int races_ok = 0;

    for (i = 0; i < SIZE(races)-1; i++) {
	if (ok_race(rolenum, i, gendnum, alignnum))
	    races_ok++;
    }
    if (races_ok == 0 || (races_ok > 1 && pickhow == PICK_RIGID))
	return ROLE_NONE;
    races_ok = rn2(races_ok);
    for (i = 0; i < SIZE(races)-1; i++) {
	if (ok_race(rolenum, i, gendnum, alignnum)) {
	    if (races_ok == 0)
		return i;
	    else
		races_ok--;
	}
    }
    return ROLE_NONE;
}

/* is gendnum compatible with any rolenum/racenum/alignnum constraints? */
/* gender and alignment are not comparable (and also not constrainable) */
boolean
ok_gend(rolenum, racenum, gendnum, alignnum)
int rolenum, racenum, gendnum, alignnum;
{
    int i;
    short allow;

    if (gendnum >= 0 && gendnum < ROLE_GENDERS) {
	allow = genders[gendnum].allow;

	if (rolenum >= 0 && rolenum < SIZE(roles)-1)
	    allow &= roles[rolenum].allow;
	if (racenum >= 0 && racenum < SIZE(races)-1)
	    allow &= races[racenum].allow;
		    
	if (!(allow & ROLE_GENDMASK))
	    return FALSE;
	return TRUE;
    } else {
	for (i = 0; i < ROLE_GENDERS; i++) {
	    allow = genders[i].allow;
	    if (rolenum >= 0 && rolenum < SIZE(roles)-1)
		allow &= roles[rolenum].allow;
	    if (racenum >= 0 && racenum < SIZE(races)-1)
		allow &= races[racenum].allow;
	    if (allow & ROLE_GENDMASK)
	    return TRUE;
	}
	return FALSE;
    }
}

/* pick a random gender subject to any rolenum/racenum/alignnum constraints */
/* gender and alignment are not comparable (and also not constrainable) */
/* If pickhow == PICK_RIGID a gender is returned only if there is  */
/* a single possibility */
int
pick_gend(rolenum, racenum, alignnum, pickhow)
int rolenum, racenum, alignnum, pickhow;
{
    int i;
    int gends_ok = 0;

    for (i = 0; i < ROLE_GENDERS; i++) {
	if (ok_gend(rolenum, racenum, i, alignnum))
	    gends_ok++;
    }
    if (gends_ok == 0 || (gends_ok > 1 && pickhow == PICK_RIGID))
	return ROLE_NONE;
    gends_ok = rn2(gends_ok);
    for (i = 0; i < ROLE_GENDERS; i++) {
	if (ok_gend(rolenum, racenum, i, alignnum)) {
	    if (gends_ok == 0)
		return i;
	    else
		gends_ok--;
	}
    }
    return ROLE_NONE;
}

/* is alignnum compatible with any rolenum/racenum/gendnum constraints? */
/* alignment and gender are not comparable (and also not constrainable) */
boolean
ok_align(rolenum, racenum, gendnum, alignnum)
int rolenum, racenum, gendnum, alignnum;
{
    int i;
    short allow;

    if (alignnum >= 0 && alignnum < ROLE_ALIGNS) {
	allow = aligns[alignnum].allow;

	if (rolenum >= 0 && rolenum < SIZE(roles)-1)
	    allow &= roles[rolenum].allow;
	if (racenum >= 0 && racenum < SIZE(races)-1)
	    allow &= races[racenum].allow;
		    
	if (!(allow & ROLE_ALIGNMASK))
	    return FALSE;
	return TRUE;
    } else {
	for (i = 0; i < ROLE_ALIGNS; i++) {
	    allow = races[i].allow;
	    if (rolenum >= 0 && rolenum < SIZE(roles)-1)
		allow &= roles[rolenum].allow;
	    if (racenum >= 0 && racenum < SIZE(races)-1)
		allow &= races[racenum].allow;
	    if (allow & ROLE_ALIGNMASK)
	    return TRUE;
	}
	return FALSE;
    }
}

/* pick a random alignment subject to any rolenum/racenum/gendnum constraints */
/* alignment and gender are not comparable (and also not constrainable) */
/* If pickhow == PICK_RIGID an alignment is returned only if there is  */
/* a single possibility */
int
pick_align(rolenum, racenum, gendnum, pickhow)
int rolenum, racenum, gendnum, pickhow;
{
    int i;
    int aligns_ok = 0;

    for (i = 0; i < ROLE_ALIGNS; i++) {
	if (ok_align(rolenum, racenum, gendnum, i))
	    aligns_ok++;
    }
    if (aligns_ok == 0 || (aligns_ok > 1 && pickhow == PICK_RIGID))
	return ROLE_NONE;
    aligns_ok = rn2(aligns_ok);
    for (i = 0; i < ROLE_ALIGNS; i++) {
	if (ok_align(rolenum, racenum, gendnum, i)) {
	    if (aligns_ok == 0)
		return i;
	    else
		aligns_ok--;
	}
    }
    return ROLE_NONE;
}

void
rigid_role_checks()
{
    /* Some roles are limited to a single race, alignment, or gender and
     * calling this routine prior to XXX_player_selection() will help
     * prevent an extraneous prompt that actually doesn't allow
     * you to choose anything further. Note the use of PICK_RIGID which
     * causes the pick_XX() routine to return a value only if there is one
     * single possible selection, otherwise it returns ROLE_NONE.
     *
     */
    if (flags.initrole == ROLE_RANDOM) {
	/* If the role was explicitly specified as ROLE_RANDOM
	 * via -uXXXX-@ then choose the role in here to narrow down
	 * later choices. Pick a random role in this case.
	 */
	flags.initrole = pick_role(flags.initrace, flags.initgend,
					flags.initalign, PICK_RANDOM);
	if (flags.initrole < 0)
	    flags.initrole = randrole();
    }
    if (flags.initrole != ROLE_NONE) {
	if (flags.initrace == ROLE_NONE)
	     flags.initrace = pick_race(flags.initrole, flags.initgend,
						flags.initalign, PICK_RIGID);
	if (flags.initalign == ROLE_NONE)
	     flags.initalign = pick_align(flags.initrole, flags.initrace,
						flags.initgend, PICK_RIGID);
	if (flags.initgend == ROLE_NONE)
	     flags.initgend = pick_gend(flags.initrole, flags.initrace,
						flags.initalign, PICK_RIGID);
    }
}

#define BP_ALIGN	0
#define BP_GEND		1
#define BP_RACE		2
#define BP_ROLE		3
#define NUM_BP		4

STATIC_VAR char pa[NUM_BP], post_attribs;

STATIC_OVL char *
promptsep(buf, num_post_attribs)
char *buf;
int num_post_attribs;
{
#if 0 /*JP*/
	const char *conj = "and ";
	if (num_post_attribs > 1
	    && post_attribs < num_post_attribs && post_attribs > 1)
	 	Strcat(buf, ","); 
	Strcat(buf, " ");
	--post_attribs;
	if (!post_attribs && num_post_attribs > 1) Strcat(buf, conj);
#else
	if(num_post_attribs > post_attribs){
		Strcat(buf, "ÅC");
	}
	--post_attribs;
#endif
	return buf;
}

STATIC_OVL int
role_gendercount(rolenum)
int rolenum;
{
	int gendcount = 0;
	if (validrole(rolenum)) {
		if (roles[rolenum].allow & ROLE_MALE) ++gendcount;
		if (roles[rolenum].allow & ROLE_FEMALE) ++gendcount;
		if (roles[rolenum].allow & ROLE_NEUTER) ++gendcount;
	}
	return gendcount;
}

STATIC_OVL int
race_alignmentcount(racenum)
int racenum;
{
	int aligncount = 0;
	if (racenum != ROLE_NONE && racenum != ROLE_RANDOM) {
		if (races[racenum].allow & ROLE_CHAOTIC) ++aligncount;
		if (races[racenum].allow & ROLE_LAWFUL) ++aligncount;
		if (races[racenum].allow & ROLE_NEUTRAL) ++aligncount;
	}
	return aligncount;
}

char *
root_plselection_prompt(suppliedbuf, buflen, rolenum, racenum, gendnum, alignnum)
char *suppliedbuf;
int buflen, rolenum, racenum, gendnum, alignnum;
{
	int k, gendercount = 0, aligncount = 0;
	char buf[BUFSZ];
/*JP
	static char err_ret[] = " character's";
*/
	static char err_ret[] = "ÉLÉÉÉâÉNÉ^Å[ÇÃ";
	boolean donefirst = FALSE;

	if (!suppliedbuf || buflen < 1) return err_ret;

	/* initialize these static variables each time this is called */
	post_attribs = 0;
	for (k=0; k < NUM_BP; ++k)
		pa[k] = 0;
	buf[0] = '\0';
	*suppliedbuf = '\0';
	
	/* How many alignments are allowed for the desired race? */
	if (racenum != ROLE_NONE && racenum != ROLE_RANDOM)
		aligncount = race_alignmentcount(racenum);

	if (alignnum != ROLE_NONE && alignnum != ROLE_RANDOM) {
		/* if race specified, and multiple choice of alignments for it */
		if ((racenum >= 0) && (aligncount > 1)) {
#if 0 /*JP*/
			if (donefirst) Strcat(buf, " ");
			Strcat(buf, aligns[alignnum].adj);
#else
			if (donefirst) Strcat(buf, "ÇÃ");
			Strcat(buf, aligns[alignnum].j);
#endif
			donefirst = TRUE;
		} else {
#if 0 /*JP*/
			if (donefirst) Strcat(buf, " ");
			Strcat(buf, aligns[alignnum].adj);
#else
			if (donefirst) Strcat(buf, "ÇÃ");
			Strcat(buf, aligns[alignnum].j);
#endif
			donefirst = TRUE;
		}
	} else {
		/* if alignment not specified, but race is specified
			and only one choice of alignment for that race then
			don't include it in the later list */
		if ((((racenum != ROLE_NONE && racenum != ROLE_RANDOM) &&
			ok_race(rolenum, racenum, gendnum, alignnum))
		      && (aligncount > 1))
		     || (racenum == ROLE_NONE || racenum == ROLE_RANDOM)) {
			pa[BP_ALIGN] = 1;
			post_attribs++;
		}
	}
	/* <your lawful> */

	/* How many genders are allowed for the desired role? */
	if (validrole(rolenum))
		gendercount = role_gendercount(rolenum);

	if (gendnum != ROLE_NONE  && gendnum != ROLE_RANDOM) {
		if (validrole(rolenum)) {
		     /* if role specified, and multiple choice of genders for it,
			and name of role itself does not distinguish gender */
			if ((rolenum != ROLE_NONE) && (gendercount > 1)
						&& !roles[rolenum].name.f) {
#if 0 /*JP*/
				if (donefirst) Strcat(buf, " ");
				Strcat(buf, genders[gendnum].adj);
#else
				if (donefirst) Strcat(buf, "ÇÃ");
				Strcat(buf, genders[gendnum].j);
#endif
				donefirst = TRUE;
			}
	        } else {
#if 0 /*JP*/
			if (donefirst) Strcat(buf, " ");
	        	Strcat(buf, genders[gendnum].adj);
#else
			if (donefirst) Strcat(buf, "ÇÃ");
			Strcat(buf, genders[gendnum].j);
#endif
			donefirst = TRUE;
	        }
	} else {
		/* if gender not specified, but role is specified
			and only one choice of gender then
			don't include it in the later list */
		if ((validrole(rolenum) && (gendercount > 1)) || !validrole(rolenum)) {
			pa[BP_GEND] = 1;
			post_attribs++;
		}
	}
	/* <your lawful female> */

	if (racenum != ROLE_NONE && racenum != ROLE_RANDOM) {
		if (validrole(rolenum) && ok_race(rolenum, racenum, gendnum, alignnum)) {
#if 0 /*JP*/
			if (donefirst) Strcat(buf, " "); 
			Strcat(buf, (rolenum == ROLE_NONE) ?
				races[racenum].noun :
				races[racenum].adj);
#else
			if (donefirst) Strcat(buf, "ÇÃ"); 
			Strcat(buf, races[racenum].j);
#endif
			donefirst = TRUE;
		} else if (!validrole(rolenum)) {
#if 0 /*JP*/
			if (donefirst) Strcat(buf, " ");
			Strcat(buf, races[racenum].noun);
#else
			if (donefirst) Strcat(buf, "ÇÃ"); 
			Strcat(buf, races[racenum].j);
#endif
			donefirst = TRUE;
		} else {
			pa[BP_RACE] = 1;
			post_attribs++;
		}
	} else {
		pa[BP_RACE] = 1;
		post_attribs++;
	}
	/* <your lawful female gnomish> || <your lawful female gnome> */

	if (validrole(rolenum)) {
/*JP
		if (donefirst) Strcat(buf, " ");
*/
		if (donefirst) Strcat(buf, "ÇÃ");
		if (gendnum != ROLE_NONE) {
#if 0 /*JP*/
		    if (gendnum == 1  && roles[rolenum].name.f)
			Strcat(buf, roles[rolenum].name.f);
		    else
  			Strcat(buf, roles[rolenum].name.m);
#else
		    if (gendnum == 1  && roles[rolenum].jname.f)
			Strcat(buf, roles[rolenum].jname.f);
		    else
  			Strcat(buf, roles[rolenum].jname.m);
#endif
		} else {
#if 0 /*JP*/
			if (roles[rolenum].name.f) {
				Strcat(buf, roles[rolenum].name.m);
				Strcat(buf, "/");
				Strcat(buf, roles[rolenum].name.f);
			} else 
				Strcat(buf, roles[rolenum].name.m);
#else
			if (roles[rolenum].jname.f) {
				Strcat(buf, roles[rolenum].jname.m);
				Strcat(buf, "/");
				Strcat(buf, roles[rolenum].jname.f);
			} else 
				Strcat(buf, roles[rolenum].jname.m);
#endif
		}
		donefirst = TRUE;
	} else if (rolenum == ROLE_NONE) {
		pa[BP_ROLE] = 1;
		post_attribs++;
	}
	
	if ((racenum == ROLE_NONE || racenum == ROLE_RANDOM) && !validrole(rolenum)) {
#if 0 /*JP*/
		if (donefirst) Strcat(buf, " ");
		Strcat(buf, "character");
#else
		if (donefirst) Strcat(buf, "ÇÃ");
		Strcat(buf, "ÉLÉÉÉâÉNÉ^Å[");
#endif
		donefirst = TRUE;
	}
#if 1 /*JP*/
	Strcat(buf, "ÇÃ");
#endif
	/* <your lawful female gnomish cavewoman> || <your lawful female gnome>
	 *    || <your lawful female character>
	 */
	if (buflen > (int) (strlen(buf) + 1)) {
		Strcpy(suppliedbuf, buf);
		return suppliedbuf;
	} else
		return err_ret;
}

char *
build_plselection_prompt(buf, buflen, rolenum, racenum, gendnum, alignnum)
char *buf;
int buflen, rolenum, racenum, gendnum, alignnum;
{
/*JP
	const char *defprompt = "Shall I pick a character for you? [ynq] ";
*/
	const char *defprompt = "ìKìñÇ…ÉLÉÉÉâÉNÉ^Å[ÇëIÇÒÇ≈ÇÊÇ¢Ç≈Ç∑Ç©ÅH[ynq] ";
	int num_post_attribs = 0;
	char tmpbuf[BUFSZ];
	
	if (buflen < QBUFSZ)
		return (char *)defprompt;

#if 0 /*JP*/
	Strcpy(tmpbuf, "Shall I pick ");
	if (racenum != ROLE_NONE || validrole(rolenum))
		Strcat(tmpbuf, "your ");
	else {
		Strcat(tmpbuf, "a ");
	}
#else
	Strcpy(tmpbuf, "");
#endif
	/* <your> */

	(void)  root_plselection_prompt(eos(tmpbuf), buflen - strlen(tmpbuf),
					rolenum, racenum, gendnum, alignnum);
	Sprintf(buf, "%s", s_suffix(tmpbuf));

	/* buf should now be:
	 * < your lawful female gnomish cavewoman's> || <your lawful female gnome's>
	 *    || <your lawful female character's>
	 *
         * Now append the post attributes to it
	 */

	num_post_attribs = post_attribs;
	if (post_attribs) {
		if (pa[BP_RACE]) {
			(void) promptsep(eos(buf), num_post_attribs);
/*JP
			Strcat(buf, "race");
*/
			Strcat(buf, "éÌë∞");
		}
		if (pa[BP_ROLE]) {
			(void) promptsep(eos(buf), num_post_attribs);
/*JP
			Strcat(buf, "role");
*/
			Strcat(buf, "êEã∆");
		}
		if (pa[BP_GEND]) {
			(void) promptsep(eos(buf), num_post_attribs);
/*JP
			Strcat(buf, "gender");
*/
			Strcat(buf, "ê´ï ");
		}
		if (pa[BP_ALIGN]) {
			(void) promptsep(eos(buf), num_post_attribs);
/*JP
			Strcat(buf, "alignment");
*/
			Strcat(buf, "ëÆê´");
		}
	}
/*JP
	Strcat(buf, " for you? [ynq] ");
*/
	Strcat(buf, "ÇìKìñÇ…ëIÇÒÇ≈ÇÊÇÎÇµÇ¢Ç≈Ç∑Ç©ÅH[ynq] ");
	return buf;
}

#undef BP_ALIGN
#undef BP_GEND
#undef BP_RACE
#undef BP_ROLE
#undef NUM_BP

void
plnamesuffix()
{
	char *sptr, *eptr;
	int i;

	/* Look for tokens delimited by '-' */
	if ((eptr = index(plname, '-')) != (char *) 0)
	    *eptr++ = '\0';
	while (eptr) {
	    /* Isolate the next token */
	    sptr = eptr;
	    if ((eptr = index(sptr, '-')) != (char *)0)
		*eptr++ = '\0';

	    /* Try to match it to something */
	    if ((i = str2role(sptr)) != ROLE_NONE)
		flags.initrole = i;
	    else if ((i = str2race(sptr)) != ROLE_NONE)
		flags.initrace = i;
	    else if ((i = str2gend(sptr)) != ROLE_NONE)
		flags.initgend = i;
	    else if ((i = str2align(sptr)) != ROLE_NONE)
		flags.initalign = i;
	}
	if(!plname[0]) {
	    askname();
	    plnamesuffix();
	}

	/* commas in the plname confuse the record file, convert to spaces */
	for (sptr = plname; *sptr; sptr++) {
		if (*sptr == ',') *sptr = ' ';
	}
}


/*
 *	Special setup modifications here:
 *
 *	Unfortunately, this is going to have to be done
 *	on each newgame or restore, because you lose the permonst mods
 *	across a save/restore.  :-)
 *
 *	1 - The Rogue Leader is the Tourist Nemesis.
 *	2 - Priests start with a random alignment - convert the leader and
 *	    guardians here.
 *	3 - Elves can have one of two different leaders, but can't work it
 *	    out here because it requires hacking the level file data (see
 *	    sp_lev.c).
 *
 * This code also replaces quest_init().
 */
void
role_init()
{
	int alignmnt;

	/* Strip the role letter out of the player name.
	 * This is included for backwards compatibility.
	 */
	plnamesuffix();

	/* Check for a valid role.  Try flags.initrole first. */
	if (!validrole(flags.initrole)) {
	    /* Try the player letter second */
	    if ((flags.initrole = str2role(pl_character)) < 0)
	    	/* None specified; pick a random role */
	    	flags.initrole = randrole();
	}

	/* We now have a valid role index.  Copy the role name back. */
	/* This should become OBSOLETE */
	Strcpy(pl_character, roles[flags.initrole].name.m);
	pl_character[PL_CSIZ-1] = '\0';

	/* Check for a valid race */
	if (!validrace(flags.initrole, flags.initrace))
	    flags.initrace = randrace(flags.initrole);

	/* Check for a valid gender.  If new game, check both initgend
	 * and female.  On restore, assume flags.female is correct. */
	if (flags.pantheon == -1) {	/* new game */
	    if (!validgend(flags.initrole, flags.initrace, flags.female))
		flags.female = !flags.female;
	}
	if (!validgend(flags.initrole, flags.initrace, flags.initgend))
	    /* Note that there is no way to check for an unspecified gender. */
	    flags.initgend = flags.female;

	/* Check for a valid alignment */
	if (!validalign(flags.initrole, flags.initrace, flags.initalign))
	    /* Pick a random alignment */
	    flags.initalign = randalign(flags.initrole, flags.initrace);
	alignmnt = aligns[flags.initalign].value;

	/* Initialize urole and urace */
	urole = roles[flags.initrole];
	urace = races[flags.initrace];

	/* Fix up the quest leader */
	if (urole.ldrnum != NON_PM) {
	    mons[urole.ldrnum].msound = MS_LEADER;
	    mons[urole.ldrnum].mflags2 |= (M2_PEACEFUL);
	    mons[urole.ldrnum].mflags3 |= M3_CLOSE;
	    mons[urole.ldrnum].maligntyp = alignmnt * 3;
	}

	/* Fix up the quest guardians */
	if (urole.guardnum != NON_PM) {
	    mons[urole.guardnum].mflags2 |= (M2_PEACEFUL);
	    mons[urole.guardnum].maligntyp = alignmnt * 3;
	}

	/* Fix up the quest nemesis */
	if (urole.neminum != NON_PM) {
	    mons[urole.neminum].msound = MS_NEMESIS;
	    mons[urole.neminum].mflags2 &= ~(M2_PEACEFUL);
	    mons[urole.neminum].mflags2 |= (M2_NASTY|M2_STALK|M2_HOSTILE);
	    mons[urole.neminum].mflags3 |= M3_WANTSARTI | M3_WAITFORU;
	}

	/* Fix up the god names */
	if (flags.pantheon == -1) {		/* new game */
	    flags.pantheon = flags.initrole;	/* use own gods */
	    while (!roles[flags.pantheon].lgod)	/* unless they're missing */
		flags.pantheon = randrole();
	}
	if (!urole.lgod) {
	    urole.lgod = roles[flags.pantheon].lgod;
	    urole.ngod = roles[flags.pantheon].ngod;
	    urole.cgod = roles[flags.pantheon].cgod;
	}

#if 0 /* Now in polyself.c, init_uasmon() */
	/* Fix up infravision */
	if (mons[urace.malenum].mflags3 & M3_INFRAVISION) {
	    /* although an infravision intrinsic is possible, infravision
	     * is purely a property of the physical race.  This means that we
	     * must put the infravision flag in the player's current race
	     * (either that or have separate permonst entries for
	     * elven/non-elven members of each class).  The side effect is that
	     * all NPCs of that class will have (probably bogus) infravision,
	     * but since infravision has no effect for NPCs anyway we can
	     * ignore this.
	     */
	    mons[urole.malenum].mflags3 |= M3_INFRAVISION;
	    if (urole.femalenum != NON_PM)
	    	mons[urole.femalenum].mflags3 |= M3_INFRAVISION;
	}
#endif

	/* Artifacts are fixed in hack_artifacts() */

	/* Success! */
	return;
}

#if 0 /*JP*/
/*
  Ç†Ç¢Ç≥Ç¬ÇÕì˙ñ{åÍÇ∆ÇµÇƒé©ëRÇ…Ç»ÇÈÇÊÇ§ëÂÇ´Ç≠édólÇïœçX
 */
const char *
Hello(mtmp)
struct monst *mtmp;
{
	switch (Role_switch) {
	case PM_KNIGHT:
	    return ("Salutations"); /* Olde English */
	case PM_SAMURAI:
	    return (mtmp && mtmp->data == &mons[PM_SHOPKEEPER] ?
	    		"Irasshaimase" : "Konnichi wa"); /* Japanese */
#ifdef TOURIST
	case PM_TOURIST:
	    return ("Aloha");       /* Hawaiian */
#endif
	case PM_VALKYRIE:
	    return (
#ifdef MAIL
	    		mtmp && mtmp->data == &mons[PM_MAIL_DAEMON] ? "Hallo" :
#endif
	    		"Velkommen");   /* Norse */
	default:
	    return ("Hello");
	}
}

const char *
Goodbye()
{
	switch (Role_switch) {
	case PM_KNIGHT:
	    return ("Fare thee well");  /* Olde English */
	case PM_SAMURAI:
	    return ("Sayonara");        /* Japanese */
#ifdef TOURIST
	case PM_TOURIST:
	    return ("Aloha");           /* Hawaiian */
#endif
	case PM_VALKYRIE:
	    return ("Farvel");          /* Norse */
	default:
	    return ("Goodbye");
	}
}

#else /*JP*/

const char *
Hello(mtmp, nameflg)
struct monst *mtmp;
int nameflg;
{
    static char helo_buf[BUFSZ];


    switch (Role_switch) {
    case PM_KNIGHT:
	if(nameflg)
	    Sprintf(helo_buf, "ÇÊÇ≠ÇºéQÇ¡ÇΩ%sÇÊ", plname);
	else
	    Sprintf(helo_buf, "ÇÊÇ≠ÇºéQÇ¡ÇΩ");
	break;
    case PM_SAMURAI:
	if(nameflg)
	    Sprintf(helo_buf, "ÇÊÇ≠ÇºéQÇÁÇÍÇΩ%sÇÊ", plname);
	else
	    Sprintf(helo_buf, "ÇÊÇ≠ÇºéQÇÁÇÍÇΩ");
	break;
#ifdef TOURIST
    case PM_TOURIST:
	if(nameflg)
	    Sprintf(helo_buf, "ÉAÉçÅ[Én%s", plname);
	else
	    Sprintf(helo_buf, "ÉAÉçÅ[Én");
	break;
#endif
    case PM_VALKYRIE:
	if(nameflg)
	    Sprintf(helo_buf, "ç∞ÇÃéÁåÏé“%sÇÊ", plname);
	else
	    Sprintf(helo_buf, "ç∞ÇÃéÁåÏé“");
	break;
    default:
	if(nameflg)
	    Sprintf(helo_buf, "ÇÊÇ§Ç±Çª%s", plname);
	else
	    Sprintf(helo_buf, "ÇÊÇ§Ç±Çª");
	break;
    }

    return helo_buf;
}

const char *
Goodbye(int nameflg)
{
    static char helo_buf[BUFSZ];

    switch (Role_switch) {
    case PM_KNIGHT:
	if(nameflg)
	    Sprintf(helo_buf, "Ç≥ÇÁÇŒåhÂiÇ»ÇÈãRémÇÃ%sÇÊ", plname);
	else
	    Sprintf(helo_buf, "Ç≥ÇÁÇŒåhÂiÇ»ÇÈ");
	break;
    case PM_SAMURAI:
	if(nameflg)
	    Sprintf(helo_buf, "Ç≥ÇÁÇŒïêémìπÇéuÇ∑%sÇÊ", plname);
	else
	    Sprintf(helo_buf, "Ç≥ÇÁÇŒïêémìπÇéuÇ∑");
	break;
#ifdef TOURIST
    case PM_TOURIST:
	if(nameflg)
	    Sprintf(helo_buf, "ÉAÉçÅ[Én%s", plname);
	else
	    Sprintf(helo_buf, "ÉAÉçÅ[Én");
	break;
#endif
    case PM_VALKYRIE:
	if(nameflg)
	    Sprintf(helo_buf, "Ç≥ÇÁÇŒç∞ÇÃéÁåÏé“%sÇÊ", plname);
	else
	    Sprintf(helo_buf, "Ç≥ÇÁÇŒç∞ÇÃéÁåÏé“");
	break;
    default:
	if(nameflg)
	    Sprintf(helo_buf, "Ç≥ÇÊÇ§Ç»ÇÁ%s", plname);
	else
	    Sprintf(helo_buf, "Ç≥ÇÊÇ§Ç»ÇÁ");
	break;
    }

    return helo_buf;
}
#endif /*JP*/
/* role.c */
