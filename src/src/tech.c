/*      SCCS Id: @(#)tech.c    3.2     98/Oct/30        */
/*      Original Code by Warren Cheung (Basis: spell.c, attrib.c) */
/*      Copyright (c) M. Stephenson 1988                          */
/* NetHack may be freely redistributed.  See license for details. */

/* All of the techs from cmd.c are ported here */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, Shiganai Sakusha, 2004-2007
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

/* #define DEBUG */		/* turn on for diagnostics */

static boolean FDECL(gettech, (int *));
static boolean FDECL(dotechmenu, (int, int *));
static void NDECL(doblitzlist);
static int FDECL(get_tech_no,(int));
static int FDECL(techeffects, (int));
static void FDECL(hurtmon, (struct monst *,int));
static int FDECL(mon_to_zombie, (int));
STATIC_PTR int NDECL(tinker);
STATIC_PTR int NDECL(draw_energy);
static const struct innate_tech * NDECL(role_tech);
static const struct innate_tech * NDECL(race_tech);
static int NDECL(doblitz);
static int NDECL(blitz_chi_strike);
static int NDECL(blitz_e_fist);
static int NDECL(blitz_pummel);
static int NDECL(blitz_g_slam);
static int NDECL(blitz_dash);
static int NDECL(blitz_power_surge);
static int NDECL(blitz_spirit_bomb);

static NEARDATA schar delay;            /* moves left for tinker/energy draw */
static NEARDATA const char revivables[] = { ALLOW_FLOOROBJ, FOOD_CLASS, 0 };

/* 
 * Do try to keep the names <= 25 chars long, or else the
 * menu will look bad :B  WAC
 */
 
STATIC_OVL NEARDATA const char *tech_names[] = {
	"no technique",
	"berserk",
	"kiii",
	"research",
	"surgery",
	"reinforce memory",
	"missile flurry",
	"weapon practice",
	"eviscerate",
	"healing hands",
	"calm steed",
	"turn undead",
	"vanish",
	"cutthroat",
	"blessing",
	"elemental fist",
	"primal roar",
	"liquid leap",
	"critical strike",
	"sigil of control",
	"sigil of tempest",
	"sigil of discharge",
	"raise zombies",
	"revivification",
	"ward against flame",
	"ward against ice",
	"ward against lightning",
	"tinker",
	"rage eruption",
	"blink",
	"chi strike",
	"draw energy",
	"chi healing",
	"disarm",
	"dazzle",
	"chained blitz",
	"pummel",
	"ground slam",
	"air dash",
	"power surge",
	"spirit bomb",
	"draw blood",
	""
};

static const struct innate_tech 
	/* Roles */
	arc_tech[] = { {   1, T_RESEARCH, 1},
		       {   0, 0, 0} },
	bar_tech[] = { {   1, T_BERSERK, 1},
		       {   0, 0, 0} },
	cav_tech[] = { {   1, T_PRIMAL_ROAR, 1},
		       {   0, 0, 0} },
	fla_tech[] = { {   1, T_REINFORCE, 1},
		       {   3, T_POWER_SURGE, 1},
		       {   5, T_DRAW_ENERGY, 1},
		       {  10, T_SIGIL_TEMPEST, 1},
		       {  20, T_SIGIL_DISCHARGE, 1},
		       {   0, 0, 0} },
	hea_tech[] = { {   1, T_SURGERY, 1},
		       {  20, T_REVIVE, 1},
		       {   0, 0, 0} },
	ice_tech[] = { {   1, T_REINFORCE, 1},
		       {   5, T_DRAW_ENERGY, 1},
		       {  10, T_SIGIL_TEMPEST, 1},
		       {  12, T_POWER_SURGE, 1},
		       {  20, T_SIGIL_DISCHARGE, 1},
		       {   0, 0, 0} },
	kni_tech[] = { {   1, T_TURN_UNDEAD, 1},
		       {   1, T_HEAL_HANDS, 1},
		       {   0, 0, 0} },
	mon_tech[] = { {   1, T_PUMMEL, 1},
		       {   1, T_DASH, 1},
		       {   1, T_BLITZ, 1},
		       {   2, T_CHI_STRIKE, 1},
	  	       {   4, T_CHI_HEALING, 1},
	  	       {   6, T_E_FIST, 1},
		       {   8, T_DRAW_ENERGY, 1},
		       {  10, T_G_SLAM, 1},
		       {  11, T_WARD_FIRE, 1},
		       {  13, T_WARD_COLD, 1},
		       {  15, T_WARD_ELEC, 1},
		       {  17, T_SPIRIT_BOMB, 1},
		       {  20, T_POWER_SURGE, 1},
		       {   0, 0, 0} },
	nec_tech[] = { {   1, T_REINFORCE, 1},
		       {   1, T_RAISE_ZOMBIES, 1},
		       {  10, T_POWER_SURGE, 1},
		       {  15, T_SIGIL_TEMPEST, 1},
		       {   0, 0, 0} },
	pri_tech[] = { {   1, T_TURN_UNDEAD, 1},
		       {   1, T_BLESSING, 1},
		       {   0, 0, 0} },
	ran_tech[] = { {   1, T_FLURRY, 1},
		       {   0, 0, 0} },
	rog_tech[] = { {   1, T_CRIT_STRIKE, 1},
		       {  15, T_CUTTHROAT, 1},
		       {   0, 0, 0} },
	sam_tech[] = { {   1, T_KIII, 1},
		       {   0, 0, 0} },
	tou_tech[] = { /* Put Tech here */
		       {   0, 0, 0} },
	und_tech[] = { {   1, T_TURN_UNDEAD, 1},
		       {   1, T_PRACTICE, 1},
		       {   0, 0, 0} },
	val_tech[] = { {   1, T_PRACTICE, 1},
		       {   0, 0, 0} },
#ifdef YEOMAN
	yeo_tech[] = {
#ifdef STEED
		       {   1, T_CALM_STEED, 1},
#endif
		       {   0, 0, 0} },
#endif
	wiz_tech[] = { {   1, T_REINFORCE, 1},
		       {   3, T_DRAW_ENERGY, 1},
		       {   5, T_POWER_SURGE, 1},
		       {   7, T_SIGIL_CONTROL, 1},
		       {  14, T_SIGIL_TEMPEST, 1},
		       {  20, T_SIGIL_DISCHARGE, 1},
		       {   0, 0, 0} },		       
	/* Races */
	dop_tech[] = { {   1, T_LIQUID_LEAP, 1},
		       {   0, 0, 0} },
	dwa_tech[] = { {   1, T_RAGE, 1},
		       {   0, 0, 0} },
	elf_tech[] = { /* Put Tech here */
		       {   0, 0, 0} },
	gno_tech[] = { {   1, T_VANISH, 1},
		       {   7, T_TINKER, 1},
		       {   0, 0, 0} },
	hob_tech[] = { {   1, T_BLINK, 1},
		       {   0, 0, 0} },
	lyc_tech[] = { {   1, T_EVISCERATE, 1},
		       {  10, T_BERSERK, 1},
		       {   0, 0, 0} },
	vam_tech[] = { {   1, T_DAZZLE, 1},
		       {   1, T_DRAW_BLOOD, 1},
		       {   0, 0, 0} };
	/* Orc */

/* Local Macros 
 * these give you direct access to the player's list of techs.  
 * Are you sure you don't want to use tech_inuse,  which is the
 * extern function for checking whether a fcn is inuse
 */

#define techt_inuse(tech)       tech_list[tech].t_inuse
#define techtout(tech)        tech_list[tech].t_tout
#define techlev(tech)         (u.ulevel - tech_list[tech].t_lev)
#define techid(tech)          tech_list[tech].t_id
#if 0 /*JP*/
#define techname(tech)        (tech_names[techid(tech)])
#else
#define techname(tech)        (jtrns_obj('T', tech_names[techid(tech)]))
#endif
#define techlet(tech)  \
        ((char)((tech < 26) ? ('a' + tech) : ('A' + tech - 26)))

/* A simple pseudorandom number generator
 *
 * This should generate fairly random numbers that will be 
 * mod LP_HPMOD from 2 to 9,  with 0 mod LP_HPMOD 
 * but can't use the normal RNG since can_limitbreak() must
 * return the same state on the same turn.
 * This also has to depend on things that do NOT change during 
 * save and restore,  and also should only change between turns
 */
#if 0 /* Probably overkill */
#define LB_CYCLE 259993L	/* number of turns before the pattern repeats */
#define LB_BASE1 ((long) (monstermoves + u.uhpmax + 300L))
#define LB_BASE2 ((long) (moves + u.uenmax + u.ulevel + 300L))
#define LB_STRIP 6	/* Remove the last few bits as they tend to be less random */
#endif
 
#define LB_CYCLE 101L	/* number of turns before the pattern repeats */
#define LB_BASE1 ((long) (monstermoves + u.uhpmax + 10L))
#define LB_BASE2 ((long) (moves + u.uenmax + u.ulevel + 10L))
#define LB_STRIP 3	/* Remove the last few bits as they tend to be less random */
 
#define LB_HPMOD ((long) ((u.uhp * 10 / u.uhpmax > 2) ? \
        			(u.uhp * 10 / u.uhpmax) : 2))

#define can_limitbreak() (!Upolyd && (u.uhp*10 < u.uhpmax) && \
        		  (u.uhp == 1 || (!((((LB_BASE1 * \
        		  LB_BASE2) % LB_CYCLE) >> LB_STRIP) \
        		  % LB_HPMOD))))
        
/* Whether you know the tech */
boolean
tech_known(tech)
	short tech;
{
	int i;
	for (i = 0; i < MAXTECH; i++) {
		if (techid(i) == tech) 
		     return TRUE;
	}
	return FALSE;
}

/* Called to prematurely stop a technique */
void
aborttech(tech)
{
	int i;

	i = get_tech_no(tech);
	if (tech_list[i].t_inuse) {
	    switch (tech_list[i].t_id) {
		case T_RAGE:
		    u.uhpmax -= tech_list[i].t_inuse - 1;
		    if (u.uhpmax < 1)
			u.uhpmax = 0;
		    u.uhp -= tech_list[i].t_inuse - 1;
		    if (u.uhp < 1)
			u.uhp = 1;
		    break;
		case T_POWER_SURGE:
		    u.uenmax -= tech_list[i].t_inuse - 1;
		    if (u.uenmax < 1)
			u.uenmax = 0;
		    u.uen -= tech_list[i].t_inuse - 1;
		    if (u.uen < 0)
			u.uen = 0;
		    break;
	    }
	    tech_list[i].t_inuse = 0;
	}
}

/* Called to teach a new tech.  Level is starting tech level */
void
learntech(tech, mask, tlevel)
	short tech;
	long mask;
	int tlevel;
{
	int i;
	const struct innate_tech *tp;

	i = get_tech_no(tech);
	if (tlevel > 0) {
	    if (i < 0) {
		i = get_tech_no(NO_TECH);
		if (i < 0) {
		    impossible("No room for new technique?");
		    return;
		}
	    }
	    tlevel = u.ulevel ? u.ulevel - tlevel : 0;
	    if (tech_list[i].t_id == NO_TECH) {
		tech_list[i].t_id = tech;
		tech_list[i].t_lev = tlevel;
		tech_list[i].t_inuse = 0; /* not in use */
		tech_list[i].t_intrinsic = 0;
	    }
	    else if (tech_list[i].t_intrinsic & mask) {
		impossible("Tech already known.");
		return;
	    }
	    if (mask == FROMOUTSIDE) {
		tech_list[i].t_intrinsic &= ~OUTSIDE_LEVEL;
		tech_list[i].t_intrinsic |= tlevel & OUTSIDE_LEVEL;
	    }
	    if (tlevel < tech_list[i].t_lev)
		tech_list[i].t_lev = tlevel;
	    tech_list[i].t_intrinsic |= mask;
	    tech_list[i].t_tout = 0; /* Can use immediately*/
	}
	else if (tlevel < 0) {
	    if (i < 0 || !(tech_list[i].t_intrinsic & mask)) {
		impossible("Tech not known.");
		return;
	    }
	    tech_list[i].t_intrinsic &= ~mask;
	    if (!(tech_list[i].t_intrinsic & INTRINSIC)) {
		if (tech_list[i].t_inuse)
		    aborttech(tech);
		tech_list[i].t_id = NO_TECH;
		return;
	    }
	    /* Re-calculate lowest t_lev */
	    if (tech_list[i].t_intrinsic & FROMOUTSIDE)
		tlevel = tech_list[i].t_intrinsic & OUTSIDE_LEVEL;
	    if (tech_list[i].t_intrinsic & FROMEXPER) {
		for(tp = role_tech(); tp->tech_id; tp++)
		    if (tp->tech_id == tech)
			break;
		if (!tp->tech_id)
		    impossible("No inate technique for role?");
		else if (tlevel < 0 || tp->ulevel - tp->tech_lev < tlevel)
		    tlevel = tp->ulevel - tp->tech_lev;
	    }
	    if (tech_list[i].t_intrinsic & FROMRACE) {
		for(tp = race_tech(); tp->tech_id; tp++)
		    if (tp->tech_id == tech)
			break;
		if (!tp->tech_id)
		    impossible("No inate technique for race?");
		else if (tlevel < 0 || tp->ulevel - tp->tech_lev < tlevel)
		    tlevel = tp->ulevel - tp->tech_lev;
	    }
	    tech_list[i].t_lev = tlevel;
	}
	else
	    impossible("Invalid Tech Level!");
}

/*
 * Return TRUE if a tech was picked, with the tech index in the return
 * parameter.  Otherwise return FALSE.
 */
static boolean
gettech(tech_no)
        int *tech_no;
{
        int i, ntechs, idx;
	char ilet, lets[BUFSZ], qbuf[QBUFSZ];

	for (ntechs = i = 0; i < MAXTECH; i++)
	    if (techid(i) != NO_TECH) ntechs++;
	if (ntechs == 0)  {
/*JP
            You("don't know any techniques right now.");
*/
	    You("特殊技能を全く持っていない．");
	    return FALSE;
	}
	if (flags.menu_style == MENU_TRADITIONAL) {
            if (ntechs == 1)  Strcpy(lets, "a");
            else if (ntechs < 27)  Sprintf(lets, "a-%c", 'a' + ntechs - 1);
            else if (ntechs == 27)  Sprintf(lets, "a-z A");
            else Sprintf(lets, "a-z A-%c", 'A' + ntechs - 27);

	    for(;;)  {
/*JP
                Sprintf(qbuf, "Perform which technique? [%s ?]", lets);
*/
                Sprintf(qbuf, "どの特殊技能を使いますか？ [%s ?]", lets);
		if ((ilet = yn_function(qbuf, (char *)0, '\0')) == '?')
		    break;

		if (index(quitchars, ilet))
		    return FALSE;

		if (letter(ilet) && ilet != '@') {
		    /* in a-zA-Z, convert back to an index */
		    if (lowc(ilet) == ilet)     /* lower case */
			idx = ilet - 'a';
		    else
			idx = ilet - 'A' + 26;

                    if (idx < ntechs)
			for(i = 0; i < MAXTECH; i++)
			    if (techid(i) != NO_TECH) {
				if (idx-- == 0) {
				    *tech_no = i;
				    return TRUE;
				}
			    }
		}
/*JP
                You("don't know that technique.");
*/
                You("その特殊技能を知らない．");
	    }
	}
        return dotechmenu(PICK_ONE, tech_no);
}

static boolean
dotechmenu(how, tech_no)
	int how;
        int *tech_no;
{
	winid tmpwin;
	int i, n, len, longest, techs_useable, tlevel;
	char buf[BUFSZ], let = 'a';
	const char *prefix;
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;         /* zero out all bits */

	techs_useable = 0;

	if (!iflags.menu_tab_sep) {
	    /* find the length of the longest tech */
	    for (longest = 0, i = 0; i < MAXTECH; i++) {
		if (techid(i) == NO_TECH) continue;
		if ((len = strlen(techname(i))) > longest)
		    longest = len;
	    }
#if 0 /*JP*/
	    Sprintf(buf, "    %-*s Level   Status", longest, "Name");
#else
	    if ((len = strlen("技能名")) > longest) longest = len;
	    Sprintf(buf, "    %-*s レベル  状態", longest, "技能名");
#endif
	} else
#if 0 /*JP*/
	    Sprintf(buf, "Name\tLevel\tStatus");
#else
	    Sprintf(buf, "技能名\tレベル\t状態");
#endif

	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);

	for (i = 0; i < MAXTECH; i++) {
	    if (techid(i) == NO_TECH)
		continue;
	    tlevel = techlev(i);
	    if (!techtout(i) && tlevel > 0) {
		/* Ready to use */
		techs_useable++;
		prefix = "";
		any.a_int = i + 1;
	    } else {
		prefix = "    ";
		any.a_int = 0;
	    }
#ifdef WIZARD
	    if (wizard) 
		if (!iflags.menu_tab_sep)			
		    Sprintf(buf, "%s%-*s %2d%c%c%c   %s(%i)",
			    prefix, longest, techname(i), tlevel,
			    tech_list[i].t_intrinsic & FROMEXPER ? 'X' : ' ',
			    tech_list[i].t_intrinsic & FROMRACE ? 'R' : ' ',
			    tech_list[i].t_intrinsic & FROMOUTSIDE ? 'O' : ' ',
#if 0 /*JP*/
			    tech_inuse(techid(i)) ? "Active" :
			    tlevel <= 0 ? "Beyond recall" :
			    can_limitbreak() ? "LIMIT" :
			    !techtout(i) ? "Prepared" : 
			    techtout(i) > 100 ? "Not Ready" : "Soon",
#else
			    tech_inuse(techid(i)) ? "使用中" :
			    tlevel <= 0 ? "忘却" :
			    can_limitbreak() ? "限界" :
			    !techtout(i) ? "準備完了" : 
			    techtout(i) > 100 ? "準備中" : "もうすぐ",
#endif
			    techtout(i));
		else
		    Sprintf(buf, "%s%s\t%2d%c%c%c\t%s(%i)",
			    prefix, techname(i), tlevel,
			    tech_list[i].t_intrinsic & FROMEXPER ? 'X' : ' ',
			    tech_list[i].t_intrinsic & FROMRACE ? 'R' : ' ',
			    tech_list[i].t_intrinsic & FROMOUTSIDE ? 'O' : ' ',
#if 0 /*JP*/
			    tech_inuse(techid(i)) ? "Active" :
			    tlevel <= 0 ? "Beyond recall" :
			    can_limitbreak() ? "LIMIT" :
			    !techtout(i) ? "Prepared" : 
			    techtout(i) > 100 ? "Not Ready" : "Soon",
#else
			    tech_inuse(techid(i)) ? "使用中" :
			    tlevel <= 0 ? "忘却" :
			    can_limitbreak() ? "限界" :
			    !techtout(i) ? "準備完了" : 
			    techtout(i) > 100 ? "準備中" : "もうすぐ",
#endif
			    techtout(i));
	    else
#endif
	    if (!iflags.menu_tab_sep)			
		Sprintf(buf, "%s%-*s %5d   %s",
			prefix, longest, techname(i), tlevel,
#if 0 /*JP*/
			tech_inuse(techid(i)) ? "Active" :
			tlevel <= 0 ? "Beyond recall" :
			can_limitbreak() ? "LIMIT" :
			!techtout(i) ? "Prepared" : 
			techtout(i) > 100 ? "Not Ready" : "Soon");
#else
			tech_inuse(techid(i)) ? "使用中" :
			tlevel <= 0 ? "忘却" :
			can_limitbreak() ? "限界" :
			!techtout(i) ? "準備完了" : 
			techtout(i) > 100 ? "準備中" : "もうすぐ");
#endif
	    else
		Sprintf(buf, "%s%s\t%5d\t%s",
			prefix, techname(i), tlevel,
#if 0 /*JP*/
			tech_inuse(techid(i)) ? "Active" :
			tlevel <= 0 ? "Beyond recall" :
			can_limitbreak() ? "LIMIT" :
			!techtout(i) ? "Prepared" : 
			techtout(i) > 100 ? "Not Ready" : "Soon");
#else
			tech_inuse(techid(i)) ? "使用中" :
			tlevel <= 0 ? "忘却" :
			can_limitbreak() ? "限界" :
			!techtout(i) ? "準備完了" : 
			techtout(i) > 100 ? "準備中" : "もうすぐ");
#endif

	    add_menu(tmpwin, NO_GLYPH, &any,
		    techtout(i) ? 0 : let, 0, ATR_NONE, buf, MENU_UNSELECTED);
	    if (let++ == 'z') let = 'A';
	}

	if (!techs_useable) 
	    how = PICK_NONE;

#if 0 /*JP*/
	end_menu(tmpwin, how == PICK_ONE ? "Choose a technique" :
					   "Currently known techniques");
#else
	end_menu(tmpwin, how == PICK_ONE ? "特殊技能選択" :
					   "現在覚えている特殊技能");
#endif

	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	if (n > 0) {
	    *tech_no = selected[0].item.a_int - 1;
	    free((genericptr_t)selected);
	    return TRUE;
	}
	return FALSE;
}

static int
get_tech_no(tech)
int tech;
{
	int i;

	for (i = 0; i < MAXTECH; i++) {
		if (techid(i) == tech) {
			return(i);
		}
	}
	return (-1);
}

int
dotech()
{
	int tech_no;

	if (gettech(&tech_no))
	    return techeffects(tech_no);
	return 0;
}

static NEARDATA const char kits[] = { TOOL_CLASS, 0 };

static struct obj *
use_medical_kit(type, feedback, verb)
int type;
boolean feedback;
char *verb;
{
    struct obj *obj, *otmp;
    makeknown(MEDICAL_KIT);
    if (!(obj = carrying(MEDICAL_KIT))) {
#if 0 /*JP*/
	if (feedback) You("need a medical kit to do that.");
#else
	if (feedback)
	    pline("それを行うには%sが必要だ．", jtrns_obj('(', "medical kit"));
#endif
	return (struct obj *)0;
    }
    for (otmp = invent; otmp; otmp = otmp->nobj)
	if (otmp->otyp == MEDICAL_KIT && otmp != obj)
	    break;
    if (otmp) {	/* More than one medical kit */
	obj = getobj(kits, verb);
	if (!obj)
	    return (struct obj *)0;
    }
    for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
	if (otmp->otyp == type)
	    break;
    if (!otmp) {
	if (feedback)
#if 0 /*JP*/
	    You_cant("find any more %s in %s.",
		    makeplural(simple_typename(type)), yname(obj));
#else
	    You("それ以上%sの中から%sを見つけられなかった．",
		    yname(obj), makeplural(simple_typename(type)));
#endif
	return (struct obj *)0;
    }
    return otmp;
}

/* gettech is reworked getspell */
/* reworked class special effects code */
/* adapted from specialpower in cmd.c */
static int
techeffects(tech_no)
int tech_no;
{
	/* These variables are used in various techs */
	struct obj *obj, *otmp;
#if 0 /*JP*/
	const char *str;
#endif
	struct monst *mtmp;
	int num;
	char Your_buf[BUFSZ];
	char allowall[2];
	int i, j, t_timeout = 0;


	/* check timeout */
	if (tech_inuse(techid(tech_no))) {
/*JP
	    pline("This technique is already active!");
*/
	    pline("その特殊技能は現在使用中だ！");
	    return (0);
	}
        if (techtout(tech_no) && !can_limitbreak()) {
/*JP
	    You("have to wait %s before using your technique again.",
                (techtout(tech_no) > 100) ?
                        "for a while" : "a little longer");
*/
	    pline("その特殊技能が再び使用できるようになるまで，%s時間がかかる．",
                (techtout(tech_no) > 100) ?
                        "かなり" : "しばらく");
#ifdef WIZARD
/*JP
            if (!wizard || (yn("Use technique anyways?") == 'n'))
*/
            if (!wizard || (yn("無理矢理使いますか？") == 'n'))
#endif
                return(0);
        }

	/* switch to the tech and do stuff */
        switch (techid(tech_no)) {
            case T_RESEARCH:
		/* WAC stolen from the spellcasters...'A' can identify from
        	   historical research*/
		if(Hallucination || Stunned || Confusion) {
/*JP
		    You("can't concentrate right now!");
*/
		    You("この状態では集中できない！");
		    return(0);
		} else if((ACURR(A_INT) + ACURR(A_WIS)) < rnd(60)) {
/*JP
			pline("Nothing in your pack looks familiar.");
*/
			Your("持ち物は見慣れた品ばかりだ．");
                    t_timeout = rn1(500,500);
		    break;
		} else if(invent) {
/*JP
			You("examine your possessions.");
*/
			You("自分の持ち物を検査した．");
			identify_pack((int) ((techlev(tech_no) / 10) + 1));
		} else {
			/* KMH -- fixed non-compliant string */
/*JP
		    You("are already quite familiar with the contents of your pack.");
*/
		    You("自分の持ち物に何があるか全て知っている．");
		    break;
		}
                t_timeout = rn1(500,1500);
		break;
            case T_EVISCERATE:
		/* only when empty handed, in human form */
		if (Upolyd || uwep || uarmg) {
#if 0 /*JP*/
		    You_cant("do this while %s!", Upolyd ? "polymorphed" :
			    uwep ? "holding a weapon" : "wearing gloves");
#else
		    You_cant("%sしている間はその技能を使えない！",
			    Upolyd ? "変化" :
			    uwep ? "武器を装備" : "小手を装備");
#endif
		    return 0;
		}
/*JP
		Your("fingernails extend into claws!");
*/
		Your("爪は鋭く尖った！");
		aggravate();
		techt_inuse(tech_no) = d(2,4) + techlev(tech_no)/5 + 2;
		t_timeout = rn1(1000,1000);
		break;
            case T_BERSERK:
/*JP
		You("fly into a berserk rage!");
*/
		You("内から沸き立つ兇暴な感情を爆発させた！");
		techt_inuse(tech_no) = d(2,8) +
               		(techlev(tech_no)/5) + 2;
		incr_itimeout(&HFast, techt_inuse(tech_no));
		t_timeout = rn1(1000,500);
		break;
            case T_REINFORCE:
		/* WAC spell-users can study their known spells*/
		if(Hallucination || Stunned || Confusion) {
/*JP
		    You("can't concentrate right now!");
*/
		    You("この状態では集中できない！");
		    break;
               	} else {
/*JP
		    You("concentrate...");
*/
		    You("集中した．．．");
		    if (studyspell()) t_timeout = rn1(1000,500); /*in spell.c*/
		}
               break;
            case T_FLURRY:
#if 0 /*JP*/
                Your("%s %s become blurs as they reach for your quiver!",
			uarmg ? "gloved" : "bare",      /* Del Lamb */
			makeplural(body_part(HAND)));
#else /*小手をつけているかどうかは省略*/
                Your("矢筒にやる%sが，かすむほど素早く動いた！",
			makeplural(body_part(HAND)));
#endif
                techt_inuse(tech_no) = rnd((int) (techlev(tech_no)/6 + 1)) + 2;
                t_timeout = rn1(1000,500);
		break;
            case T_PRACTICE:
                if(!uwep || (weapon_type(uwep) == P_NONE)) {
/*JP
		    You("are not wielding a weapon!");
*/
		    You("武器を装備していない！");
		    return(0);
		} else if(uwep->known == TRUE) {
                    practice_weapon();
		} else {
                    if (not_fully_identified(uwep)) {
/*JP
                        You("examine %s.", doname(uwep));
*/
                        You("%sを調べた．", doname(uwep));
                            if (rnd(15) <= ACURR(A_INT)) {
                                makeknown(uwep->otyp);
                                uwep->known = TRUE;
/*JP
                                You("discover it is %s",doname(uwep));
*/
                                You("それが%sであるとわかった．",doname(uwep));
                                } else
/*JP
                     pline("Unfortunately, you didn't learn anything new.");
*/
                     pline("不幸な事に，何ら得るものは無かった．");
                    } 
                /*WAC Added practicing code - in weapon.c*/
                    practice_weapon();
		}
                t_timeout = rn1(500,500);
		break;
            case T_SURGERY:
		if (Hallucination || Stunned || Confusion) {
/*JP
		    You("are in no condition to perform surgery!");
*/
		    You("とても手術にかかれる状態ではない！");
		    break;
		}
		if (Sick || Slimed) {
		    if (carrying(SCALPEL)) {
/*JP
			pline("Using your scalpel (ow!), you cure your infection!");
*/
			You("メスを手に，自分で自分を（！）手術した．");
			make_sick(0L, (char *)0, TRUE, SICK_ALL);
			Slimed = 0;
			if (Upolyd) {
			    u.mh -= 5;
			    if (u.mh < 1)
				rehumanize();
			} else if (u.uhp > 6)
			    u.uhp -= 5;
			else
			    u.uhp = 1;
                        t_timeout = rn1(500,500);
			flags.botl = TRUE;
			break;
/*JP
		    } else pline("If only you had a scalpel...");
*/
		    } else pline("メスさえあれば．．．");
		}
		if (Upolyd ? u.mh < u.mhmax : u.uhp < u.uhpmax) {
		    otmp = use_medical_kit(BANDAGE, FALSE,
			    "dress your wounds with");
		    if (otmp) {
			check_unpaid(otmp);
			if (otmp->quan > 1L) {
			    otmp->quan--;
			    otmp->ocontainer->owt = weight(otmp->ocontainer);
			} else {
			    obj_extract_self(otmp);
			    obfree(otmp, (struct obj *)0);
			}
/*JP
			pline("Using %s, you dress your wounds.", yname(otmp));
*/
			You("%sを傷口に巻いた．", yname(otmp));
			healup(techlev(tech_no) * (rnd(2)+1) + rn1(5,5),
			  0, FALSE, FALSE);
		    } else {
/*JP
			You("strap your wounds as best you can.");
*/
			You("とりあえず傷口を縛った．");
			healup(techlev(tech_no) + rn1(5,5), 0, FALSE, FALSE);
		    }
                    t_timeout = rn1(1000,500);
		    flags.botl = TRUE;
/*JP
		} else You("don't need your healing powers!");
*/
		} else You("治療の必要はない！");
		break;
            case T_HEAL_HANDS:
		if (Slimed) {
/*JP
		    Your("body is on fire!");
*/
		    Your("身体は炎に包まれた！");
		    burn_away_slime();
		    t_timeout = 3000;
		} else if (Sick) {
/*JP
		    You("lay your hands on the foul sickness...");
*/
		    You("患部に%sをかざした．．．", makeplural(body_part(HAND)));
		    make_sick(0L, (char*)0, TRUE, SICK_ALL);
		    t_timeout = 3000;
		} else if (Upolyd ? u.mh < u.mhmax : u.uhp < u.uhpmax) {
/*JP
		    pline("A warm glow spreads through your body!");
*/
		    pline("あたたかい力があなたの身体に広がった！");
		    healup(techlev(tech_no) * 4, 0, FALSE, FALSE);
		    t_timeout = 3000;
		} else
		    pline(nothing_happens);
		break;
            case T_KIII:
/*JP
		You("scream \"KIIILLL!\"");
*/
		You("叫んだ．「殺してやる殺してやる殺してやる．．．！」");
		aggravate();
                techt_inuse(tech_no) = rnd((int) (techlev(tech_no)/6 + 1)) + 2;
                t_timeout = rn1(1000,500);
		break;
#ifdef STEED
	    case T_CALM_STEED:
                if (u.usteed) {
/*JP
                        pline("%s gets tamer.", Monnam(u.usteed));
*/
                        pline("%sは調教された．", Monnam(u.usteed));
                        tamedog(u.usteed, (struct obj *) 0);
                        t_timeout = rn1(1000,500);
                } else
/*JP
                        Your("technique is only effective when riding a monster.");
*/
                        pline("この技能は乗馬中でないと使用できない．");
                break;
#endif
            case T_TURN_UNDEAD:
                return(turn_undead());
	    case T_VANISH:
		if (Invisible && Fast) {
/*JP
			You("are already quite nimble and undetectable.");
*/
			You("既に素早いし敵に気づかれもしない．");
		}
                techt_inuse(tech_no) = rn1(50,50) + techlev(tech_no);
/*JP
		if (!Invisible) pline("In a puff of smoke,  you disappear!");
		if (!Fast) You("feel more nimble!");
*/
		if (!Invisible) Your("姿は煙と共に消え失せた！");
		if (!Fast) You("更に素早くなったような気がした！");
		incr_itimeout(&HInvis, techt_inuse(tech_no));
		incr_itimeout(&HFast, techt_inuse(tech_no));
		newsym(u.ux,u.uy);      /* update position */
		t_timeout = rn1(1000,500);
		break;
	    case T_CRIT_STRIKE:
		if (!getdir((char *)0)) return(0);
		if (!u.dx && !u.dy) {
		    /* Hopefully a mistake ;B */
/*JP
		    You("decide against that idea.");
*/
		    pline("それは良い考えとはいえない．");
		    return(0);
		}
		mtmp = m_at(u.ux + u.dx, u.uy + u.dy);
		if (!mtmp) {
/*JP
		    You("perform a flashy twirl!");
*/
		    Your("攻撃は派手に宙をきった！");
		    return (0);
		} else {
		    int oldhp = mtmp->mhp;
		    int tmp;

		    if (!attack(mtmp)) return(0);
		    if (!DEADMONSTER(mtmp) && mtmp->mhp < oldhp &&
			    !noncorporeal(mtmp->data) && !unsolid(mtmp->data)) {
/*JP
			You("strike %s vital organs!", s_suffix(mon_nam(mtmp)));
*/
			You("%sの重要な臓器を攻撃した！", mon_nam(mtmp));
			/* Base damage is always something, though it may be
			 * reduced to zero if the hero is hampered. However,
			 * since techlev will never be zero, stiking vital
			 * organs will always do _some_ damage.
			 */
			tmp = mtmp->mhp > 1 ? mtmp->mhp / 2 : 1;
			if (!humanoid(mtmp->data) || is_golem(mtmp->data) ||
				mtmp->data->mlet == S_CENTAUR) {
/*JP
			    You("are hampered by the differences in anatomy.");
*/
			    pline("相手の身体構造が人間とは違っていたため弱点を突けなかった．");
			    tmp /= 2;
			}
			tmp += techlev(tech_no);
			t_timeout = rn1(1000, 500);
			hurtmon(mtmp, tmp);
		    }
		}
		break;
	    case T_CUTTHROAT:
		if (!is_blade(uwep)) {
#if 0 /*JP*/
		    You("need a blade to perform cutthroat!");
#else
		    pline("首を掻き切るには刃のついた武器が必要だ．");
#endif
		    return 0;
		}
	    	if (!getdir((char *)0)) return 0;
		if (!u.dx && !u.dy) {
		    /* Hopefully a mistake ;B */
/*JP
		    pline("Things may be going badly, but that's extreme.");
*/
		    pline("失敗しそうだ．派手ではあるが．");
		    return 0;
		}
		mtmp = m_at(u.ux + u.dx, u.uy + u.dy);
		if (!mtmp) {
/*JP
		    You("attack...nothing!");
*/
		    You("攻撃した．．．空を切った！");
		    return 0;
		} else {
		    int oldhp = mtmp->mhp;

		    if (!attack(mtmp)) return 0;
		    if (!DEADMONSTER(mtmp) && mtmp->mhp < oldhp) {
			if (!has_head(mtmp->data) || u.uswallow)
#if 0 /*JP*/
			    You_cant("perform cutthroat on %s!", mon_nam(mtmp));
#else
			    /*[喉掻き切り->首刎ね]pline("掻き切ろうにも%sには喉がない！",mon_nam(mtmp));*/
			    pline("%sの首が見当たらない！",mon_nam(mtmp));
#endif
			else {
			    int tmp = 0;

			    if (rn2(5) < (techlev(tech_no)/10 + 1)) {
#if 0 /*JP*/
				You("sever %s head!", s_suffix(mon_nam(mtmp)));
#else
				/*[喉掻き切り->首刎ね] You("%sの喉を掻き切った！", s_suffix(mon_nam(mtmp)));*/
				You("%sの首を掻き切った！", s_suffix(mon_nam(mtmp)));
#endif
				tmp = mtmp->mhp;
			    } else {
/*JP
				You("hurt %s badly!", s_suffix(mon_nam(mtmp)));
*/
				You("%sに重傷を負わせた！", s_suffix(mon_nam(mtmp)));
				tmp = mtmp->mhp / 2;
			    }
			    tmp += techlev(tech_no);
			    t_timeout = rn1(1000,500);
			    hurtmon(mtmp, tmp);
			}
		    }
		}
		break;
	    case T_BLESSING:
		allowall[0] = ALL_CLASSES; allowall[1] = '\0';
		
		if ( !(obj = getobj(allowall, "bless"))) return(0);
#if 0 /*JP*/
		pline("An aura of holiness surrounds your hands!");
#else
		pline("聖なるオーラがあなたの%sを包んだ！", makeplural(body_part(HAND)));
#endif
                if (!Blind) (void) Shk_Your(Your_buf, obj);
		if (obj->cursed) {
                	if (!Blind)
#if 0 /*JP*/
                    		pline("%s %s %s.",Your_buf,
						  aobjnam(obj, "softly glow"),
						  hcolor(NH_AMBER));
#else
				pline("%s%sは%sやわらかく輝いた．", Your_buf,
						  xname(obj), 
						  jconj_adj(hcolor(NH_AMBER)));
#endif
				uncurse(obj);
				obj->bknown=1;
		} else if(!obj->blessed) {
			if (!Blind) {
#if 0 /*JP*/
				str = hcolor(NH_LIGHT_BLUE);
				pline("%s %s with a%s %s aura.",
					  Your_buf,
					  aobjnam(obj, "softly glow"),
					  index(vowels, *str) ? "n" : "", str);
#else
				pline("%s%sは%sオーラに包まれた．", Your_buf,
					  xname(obj), hcolor(NH_LIGHT_BLUE));
#endif
			}
			bless(obj);
			obj->bknown=1;
		} else {
			if (obj->bknown) {
/*JP
				pline ("That object is already blessed!");
*/
				pline ("その品は既に祝福されている！");
				return(0);
			}
			obj->bknown=1;
/*JP
			pline("The aura fades.");
*/
			pline("オーラは消えた．");
		}
		t_timeout = rn1(1000,500);
		break;
	    case T_E_FIST: 
	    	blitz_e_fist();
#if 0
		str = makeplural(body_part(HAND));
                You("focus the powers of the elements into your %s", str);
                techt_inuse(tech_no) = rnd((int) (techlev(tech_no)/3 + 1)) + d(1,4) + 2;
#endif
		t_timeout = rn1(1000,500);
	    	break;
	    case T_PRIMAL_ROAR:	    	
/*JP
	    	You("let out a bloodcurdling roar!");
*/
	    	You("身の毛もよだつ雄叫びをあげた！");
	    	aggravate();

		techt_inuse(tech_no) = d(2,6) + (techlev(tech_no)) + 2;

		incr_itimeout(&HFast, techt_inuse(tech_no));

	    	for(i = -5; i <= 5; i++) for(j = -5; j <= 5; j++)
		    if(isok(u.ux+i, u.uy+j) && (mtmp = m_at(u.ux+i, u.uy+j))) {
		    	if (mtmp->mtame != 0 && !mtmp->isspell) {
		    	    struct permonst *ptr = mtmp->data;
			    struct monst *mtmp2;
		    	    int ttime = techt_inuse(tech_no);
		    	    int type = little_to_big(monsndx(ptr));
		    	    
		    	    mtmp2 = tamedog(mtmp, (struct obj *) 0);
			    if (mtmp2)
				mtmp = mtmp2;

		    	    if (type && type != monsndx(ptr)) {
				ptr = &mons[type];
		    	    	mon_spec_poly(mtmp, ptr, ttime, FALSE,
					canseemon(mtmp), FALSE, TRUE);
		    	    }
		    	}
		    }
		t_timeout = rn1(1000,500);
	    	break;
	    case T_LIQUID_LEAP: {
	    	coord cc;
	    	int dx, dy, sx, sy, range;

/*JP
		pline("Where do you want to leap to?");
*/
		pline("どこに移動しますか？");
    		cc.x = sx = u.ux;
		cc.y = sy = u.uy;

/*JP
		getpos(&cc, TRUE, "the desired position");
*/
		getpos(&cc, TRUE, "希望する位置");
		if (cc.x == -10) return 0; /* user pressed esc */

		dx = cc.x - u.ux;
		dy = cc.y - u.uy;
		/* allow diagonals */
	    	if (dx && dy && dx != dy && dx != -dy) {
#if 0 /*JP*/
		    You("can only leap in straight lines!");
#else
		    You("直線上でないと移動できない！");
#endif
		    return 0;
	    	} else if (distu(cc.x, cc.y) > 19 + techlev(tech_no)) {
#if 0 /*JP*/
		    pline("Too far!");
#else
		    pline("遠すぎる！");
#endif
		    return 0;
		} else if (m_at(cc.x, cc.y) || !isok(cc.x, cc.y) ||
			IS_ROCK(levl[cc.x][cc.y].typ) ||
			sobj_at(BOULDER, cc.x, cc.y) ||
			closed_door(cc.x, cc.y)) {
#if 0 /*JP*/
		    You_cant("flow there!"); /* MAR */
#else
		    You_cant("そこには流れ込めない！");
#endif
		    return 0;
		} else {
/*JP
		    You("liquify!");
*/
		    You("液体と化した！");
		    if (Punished) {
/*JP
			You("slip out of the iron chain.");
*/
			You("鉄の鎖から抜け出した．");
			unpunish();
		    }
		    if(u.utrap) {
			switch(u.utraptype) {
			    case TT_BEARTRAP: 
/*JP
				You("slide out of the bear trap.");
*/
				You("熊の罠から抜け出した．");
				break;
			    case TT_PIT:
/*JP
				You("leap from the pit!");
*/
				You("落し穴から流れ出た！");
				break;
			    case TT_WEB:
/*JP
				You("flow through the web!");
*/
				You("蜘蛛の巣を透過した！");
				break;
			    case TT_LAVA:
/*JP
				You("separate from the lava!");
*/
				You("溶岩から分離した！");
				u.utrap = 0;
				break;
			    case TT_INFLOOR:
				u.utrap = 0;
/*JP
				You("ooze out of the floor!");
*/
				You("床から流れて抜けた！");
			}
			u.utrap = 0;
		    }
		    /* Fry the things in the path ;B */
		    if (dx) range = dx;
		    else range = dy;
		    if (range < 0) range = -range;
		    
		    dx = sgn(dx);
		    dy = sgn(dy);
		    
		    while (range-- > 0) {
		    	int tmp_invul = 0;
		    	
		    	if (!Invulnerable) Invulnerable = tmp_invul = 1;
			sx += dx; sy += dy;
			tmp_at(DISP_BEAM, zapdir_to_glyph(dx, dy, AD_ACID-1));
			tmp_at(sx,sy);
			delay_output(); /* wait a little */
		    	if ((mtmp = m_at(sx, sy)) != 0) {
			    int chance;
			    
			    chance = rn2(20);
		    	    if (!chance || (3 - chance) > AC_VALUE(find_mac(mtmp)))
		    	    	break;
			    setmangry(mtmp);
/*JP
		    	    You("catch %s in your acid trail!", mon_nam(mtmp));
*/
		    	    You("通り道に残した酸の跡に%sを巻き込んだ！", mon_nam(mtmp));
		    	    if (!resists_acid(mtmp)) {
				int tmp = 1;
				/* Need to add a to-hit */
				tmp += d(2,4);
				tmp += rn2((int) (techlev(tech_no)/5 + 1));
/*JP
				if (!Blind) pline_The("acid burns %s!", mon_nam(mtmp));
*/
				if (!Blind) pline("酸は%sを焼いた！", mon_nam(mtmp));
				hurtmon(mtmp, tmp);
/*JP
			    } else if (!Blind) pline_The("acid doesn't affect %s!", mon_nam(mtmp));
*/
			    } else if (!Blind) pline("酸は%sには効果がなかった！", mon_nam(mtmp));
			}
			/* Clean up */
			tmp_at(DISP_END,0);
			if (tmp_invul) Invulnerable = 0;
		    }

		    /* A little Sokoban guilt... */
		    if (In_sokoban(&u.uz))
			change_luck(-1);
/*JP
		    You("reform!");
*/
		    You("元の姿に戻った！");
		    teleds(cc.x, cc.y, FALSE);
		    nomul(-1);
		    nomovemsg = "";
	    	}
		t_timeout = rn1(1000,500);
	    	break;
	    }
            case T_SIGIL_TEMPEST: 
		/* Have enough power? */
		num = 50 - techlev(tech_no)/5;
		if (u.uen < num) {
/*JP
			You("don't have enough power to invoke the sigil!");
*/
			You("印を結ぶ充分な魔力がない！");
			return (0);
		}
		u.uen -= num;

		/* Invoke */
/*JP
		You("invoke the sigil of tempest!");
*/
		You("嵐の印を結んだ！");
                techt_inuse(tech_no) = d(1,6) + rnd(techlev(tech_no)/5 + 1) + 2;
		u_wipe_engr(2);
		return(0);
		break;
            case T_SIGIL_CONTROL:
		/* Have enough power? */
		num = 30 - techlev(tech_no)/5;
		if (u.uen < num) {
/*JP
			You("don't have enough power to invoke the sigil!");
*/
			You("印を結ぶ充分な魔力がない！");
			return (0);
		}
		u.uen -= num;

		/* Invoke */
/*JP
		You("invoke the sigil of control!");
*/
		You("操りの印を結んだ！");
                techt_inuse(tech_no) = d(1,4) + rnd(techlev(tech_no)/5 + 1) + 2;
		u_wipe_engr(2);
		return(0);
		break;
            case T_SIGIL_DISCHARGE:
		/* Have enough power? */
		num = 100 - techlev(tech_no)/5;
		if (u.uen < num) {
/*JP
			You("don't have enough power to invoke the sigil!");
*/
			You("印を結ぶ充分な魔力がない！");
			return (0);
		}
		u.uen -= num;

		/* Invoke */
/*JP
		You("invoke the sigil of discharge!");
*/
		You("解放の印を結んだ！");
                techt_inuse(tech_no) = d(1,4) + rnd(techlev(tech_no)/5 + 1) + 2;
		u_wipe_engr(2);
		return(0);
		break;
            case T_RAISE_ZOMBIES:
/*JP
            	You("chant the ancient curse...");
*/
		You("太古の呪いを詠唱した．．．");
		for(i = -1; i <= 1; i++) for(j = -1; j <= 1; j++) {
		    int corpsenm;

		    if (!isok(u.ux+i, u.uy+j)) continue;
		    for (obj = level.objects[u.ux+i][u.uy+j]; obj; obj = otmp) {
			otmp = obj->nexthere;

			if (obj->otyp != CORPSE) continue;
			/* Only generate undead */
			corpsenm = mon_to_zombie(obj->corpsenm);
			if (corpsenm != -1 && !cant_create(&corpsenm, TRUE) &&
			  (!obj->oxlth || obj->oattached != OATTACHED_MONST)) {
			    /* Maintain approx. proportion of oeaten to cnutrit
			     * so that the zombie's HP relate roughly to how
			     * much of the original corpse was left.
			     */
			    if (obj->oeaten)
				obj->oeaten =
					eaten_stat(mons[corpsenm].cnutrit, obj);
			    obj->corpsenm = corpsenm;
			    mtmp = revive(obj);
			    if (mtmp) {
				if (!resist(mtmp, SPBOOK_CLASS, 0, TELL)) {
				   mtmp = tamedog(mtmp, (struct obj *) 0);
/*JP
				   You("dominate %s!", mon_nam(mtmp));
*/
				   You("%sを支配した！", mon_nam(mtmp));
				} else setmangry(mtmp);
			    }
			}
		    }
		}
		nomul(-2); /* You need to recover */
		nomovemsg = 0;
		t_timeout = rn1(1000,500);
		break;
            case T_REVIVE: 
		if (u.uswallow) {
		    You(no_elbow_room);
		    return 0;
		}
            	num = 100 - techlev(tech_no); /* WAC make this depend on mon? */
            	if ((Upolyd && u.mh <= num) || (!Upolyd && u.uhp <= num)){
/*JP
		    You("don't have the strength to perform revivification!");
*/
		    You("蘇生を行うだけの充分な力がない！");
		    return 0;
            	}

            	obj = getobj((const char *)revivables, "revive");
            	if (!obj) return (0);
            	mtmp = revive(obj);
            	if (mtmp) {
#ifdef BLACKMARKET
		    if (Is_blackmarket(&u.uz))
			setmangry(mtmp);
		    else
#endif
		    if (mtmp->isshk)
			make_happy_shk(mtmp, FALSE);
		    else if (!resist(mtmp, SPBOOK_CLASS, 0, NOTELL))
			(void) tamedog(mtmp, (struct obj *) 0);
		}
            	if (Upolyd) u.mh -= num;
            	else u.uhp -= num;
		t_timeout = rn1(1000,500);
            	break;
	    case T_WARD_FIRE:
		/* Already have it intrinsically? */
		if (HFire_resistance & FROMOUTSIDE) return (0);

/*JP
		You("invoke the ward against flame!");
*/
		You("火除けの呪言を唱えた！");
		HFire_resistance += rn1(100,50);
		HFire_resistance += techlev(tech_no);
		t_timeout = rn1(1000,500);

	    	break;
	    case T_WARD_COLD:
		/* Already have it intrinsically? */
		if (HCold_resistance & FROMOUTSIDE) return (0);

/*JP
		You("invoke the ward against ice!");
*/
		You("冷気除けの呪言を唱えた！");
		HCold_resistance += rn1(100,50);
		HCold_resistance += techlev(tech_no);
		t_timeout = rn1(1000,500);

	    	break;
	    case T_WARD_ELEC:
		/* Already have it intrinsically? */
		if (HShock_resistance & FROMOUTSIDE) return (0);

/*JP
		You("invoke the ward against lightning!");
*/
		You("雷除けの呪言を唱えた！");
		HShock_resistance += rn1(100,50);
		HShock_resistance += techlev(tech_no);
		t_timeout = rn1(1000,500);

	    	break;
	    case T_TINKER:
		if (Blind) {
/*JP
			You("can't do any tinkering if you can't see!");
*/
			You("目が見えないので細工ができない！");
			return (0);
		}
		if (!uwep) {
/*JP
			You("aren't holding an object to work on!");
*/
			You("作業すべき品を手にしていない！");
			return (0);
		}
#if 0 /*JP*/
		You("are holding %s.", doname(uwep));
		if (yn("Start tinkering on this?") != 'y') return(0);
		You("start working on %s",doname(uwep));
#else
		You("%sを手にしている．", doname(uwep));
		if (yn("これに細工を加えますか？") != 'y') return(0);
		You("%sの加工をはじめた．",doname(uwep));
#endif
		delay=-150 + techlev(tech_no);
/*JP
		set_occupation(tinker, "tinkering", 0);
*/
		set_occupation(tinker, "細工する", 0);
		break;
	    case T_RAGE:     	
		if (Upolyd) {
/*JP
			You("cannot focus your anger!");
*/
			You("怒りを一点に集中できない！");
			return(0);
		}
/*JP
	    	You("feel the anger inside you erupt!");
*/
	    	You("内なる怒りの爆発を感じた！");
		num = 50 + (4 * techlev(tech_no));
	    	techt_inuse(tech_no) = num + 1;
		u.uhpmax += num;
		u.uhp += num;
		t_timeout = rn1(1000,500);
		break;	    
	    case T_BLINK:
/*JP
	    	You("feel the flow of time slow down.");
*/
	    	You("時の流れが遅くなったように感じた．");
                techt_inuse(tech_no) = rnd(techlev(tech_no) + 1) + 2;
		t_timeout = rn1(1000,500);
	    	break;
            case T_CHI_STRIKE:
            	if (!blitz_chi_strike()) return(0);
                t_timeout = rn1(1000,500);
		break;
            case T_DRAW_ENERGY:
            	if (u.uen == u.uenmax) {
#if 0 /*JP*/
            		if (Hallucination) You("are fully charged!");
			else You("cannot hold any more energy!");
#else
            		if (Hallucination) You("めいっぱい充電されてます！");
			else You("これ以上魔力を吸収できない！");
#endif
			return(0);           		
            	}
/*JP
                You("begin drawing energy from your surroundings!");
*/
                You("周囲の魔力を吸収し始めた！");
		delay=-15;
/*JP
		set_occupation(draw_energy, "drawing energy", 0);                
*/
		set_occupation(draw_energy, "魔力を吸収する", 0);                
                t_timeout = rn1(1000,500);
		break;
            case T_CHI_HEALING:
            	if (u.uen < 1) {
/*JP
            		You("are too weak to attempt this!");
*/
            		You("それを行うにはあまりにも衰弱しすぎている！");
            		return(0);
            	}
/*JP
		You("direct your internal energy to restoring your body!");
*/
		You("内なる力を肉体の再生の為に注ぎこんだ！");
                techt_inuse(tech_no) = techlev(tech_no)*2 + 4;
                t_timeout = rn1(1000,500);
		break;	
	    case T_DISARM:
	    	if (P_SKILL(weapon_type(uwep)) == P_NONE) {
/*JP
	    		You("aren't wielding a proper weapon!");
*/
	    		You("適切な武器を装備していない！");
	    		return(0);
	    	}
	    	if ((P_SKILL(weapon_type(uwep)) < P_SKILLED) || (Blind)) {
/*JP
	    		You("aren't capable of doing this!");
*/
	    		pline("あなたにはそれを扱う能力がない！");
	    		return(0);
	    	}
		if (u.uswallow) {
/*JP
	    		pline("What do you think %s is?  A sword swallower?",
				mon_nam(u.ustuck));
*/
	    		pline("%sを何だと思ってるんだい？ 剣呑みの大道芸人だとでも？",
				mon_nam(u.ustuck));
	    		return(0);
		}

	    	if (!getdir((char *)0)) return(0);
		if (!u.dx && !u.dy) {
			/* Hopefully a mistake ;B */
/*JP
			pline("Why don't you try wielding something else instead.");
*/
			pline("代わりに他の相手を狙ってみたら？");
			return(0);
		}
		mtmp = m_at(u.ux + u.dx, u.uy + u.dy);
		if (!mtmp || !canspotmon(mtmp)) {
			if (memory_is_invisible(u.ux + u.dx, u.uy + u.dy))
/*JP
			    You("don't know where to aim for!");
*/
			    You("どの部位を狙えばよいかわからない！");
			else
/*JP
			    You("don't see anything there!");
*/
			    pline("そこには何もいないように見える．");
			return (0);
		}
	    	obj = MON_WEP(mtmp);   /* can be null */
	    	if (!obj) {
/*JP
	    		You_cant("disarm an unarmed foe!");
*/
	    		pline("丸腰の相手に対して武器落としは意味がない！");
	    		return(0);
	    	}
		/* Blindness dealt with above */
		if (!mon_visible(mtmp)
#ifdef INVISIBLE_OBJECTS
				|| obj->oinvis && !See_invisible
#endif
				) {
/*JP
	    		You_cant("see %s weapon!", s_suffix(mon_nam(mtmp)));
*/
	    		You("%sの武器が見えない！", mon_nam(mtmp));
	    		return(0);
		}
		num = ((rn2(techlev(tech_no) + 15)) 
			* (P_SKILL(weapon_type(uwep)) - P_SKILLED + 1)) / 10;

/*JP
		You("attempt to disarm %s...",mon_nam(mtmp));
*/
		You("%sの武器を叩き落そうとした．．．",mon_nam(mtmp));
		/* WAC can't yank out cursed items */
                if (num > 0 && (!Fumbling || !rn2(10)) && !obj->cursed) {
		    int roll;
		    obj_extract_self(obj);
		    possibly_unwield(mtmp, FALSE);
		    setmnotwielded(mtmp, obj);
		    roll = rn2(num + 1);
		    if (roll > 3) roll = 3;
		    switch (roll) {
			case 2:
			    /* to floor near you */
#if 0 /*JP*/
			    You("knock %s %s to the %s!",
				s_suffix(mon_nam(mtmp)),
#else
			    You("%sの%sを%sに叩き落した！",
				mon_nam(mtmp),
#endif
				xname(obj),
				surface(u.ux, u.uy));
			    if (obj->otyp == CRYSKNIFE &&
				    (!obj->oerodeproof || !rn2(10))) {
				obj->otyp = WORM_TOOTH;
				obj->oerodeproof = 0;
			    }
			    place_object(obj, u.ux, u.uy);
			    stackobj(obj);
			    break;
			case 3:
#if 0
			    if (!rn2(25)) {
				/* proficient at disarming, but maybe not
				   so proficient at catching weapons */
				int hitu, hitvalu;

				hitvalu = 8 + obj->spe;
				hitu = thitu(hitvalu,
					dmgval(obj, &youmonst),
					obj, xname(obj));
				if (hitu)
/*JP
				    pline("%s hits you as you try to snatch it!",
*/
				    pline("%sを奪おうとしたらあなたに当たった！",
					    The(xname(obj)));
				place_object(obj, u.ux, u.uy);
				stackobj(obj);
				break;
			    }
#endif /* 0 */
			    /* right into your inventory */
/*JP
			    You("snatch %s %s!", s_suffix(mon_nam(mtmp)),
*/
			    You("%sの%sを奪った！", mon_nam(mtmp),
				    xname(obj));
			    if (obj->otyp == CORPSE &&
				    touch_petrifies(&mons[obj->corpsenm]) &&
				    !uarmg && !Stone_resistance &&
				    !(poly_when_stoned(youmonst.data) &&
					polymon(PM_STONE_GOLEM))) {
				char kbuf[BUFSZ];

#if 0 /*JP*/
				Sprintf(kbuf, "%s corpse",
					an(mons[obj->corpsenm].mname));
				pline("Snatching %s is a fatal mistake.", kbuf);
#else
				pline("%sの死体を奪ったのは致命的な間違いだ．",
					jtrns_mon(mons[obj->corpsenm].mname));
				Sprintf(kbuf, "%sの死体に触れて",
					jtrns_mon(mons[obj->corpsenm].mname));
#endif
				instapetrify(kbuf);
			    }
/*JP
			    obj = hold_another_object(obj, "You drop %s!",
*/
			    obj = hold_another_object(obj, "%sを落した！",
				    doname(obj), (const char *)0);
			    break;
			default:
			    /* to floor beneath mon */
/*JP
			    You("knock %s from %s grasp!", the(xname(obj)),
*/
			    You("%sを%sの握りからひっぱった！", the(xname(obj)),
				    s_suffix(mon_nam(mtmp)));
			    if (obj->otyp == CRYSKNIFE &&
				    (!obj->oerodeproof || !rn2(10))) {
				obj->otyp = WORM_TOOTH;
				obj->oerodeproof = 0;
			    }
			    place_object(obj, mtmp->mx, mtmp->my);
			    stackobj(obj);
			    break;
		    }
		} else if (mtmp->mcanmove && !mtmp->msleeping)
/*JP
		    pline("%s evades your attack.", Monnam(mtmp));
*/
		    pline("%sはあなたの攻撃をかわした．", Monnam(mtmp));
		else
/*JP
		    You("fail to dislodge %s %s.", s_suffix(mon_nam(mtmp)),
*/
		    You("%sの%sを叩き落す事に失敗した．", s_suffix(mon_nam(mtmp)),
			    xname(obj));
		wakeup(mtmp);
		if (!mtmp->mcanmove && !rn2(10)) {
		    mtmp->mcanmove = 1;
		    mtmp->mfrozen = 0;
		}
		break;
	    case T_DAZZLE:
	    	/* Short range stun attack */
	    	if (Blind) {
/*JP
	    		You("can't see anything!");
*/
	    		You("何も見えない！");
	    		return(0);
	    	}
	    	if (!getdir((char *)0)) return(0);
		if (!u.dx && !u.dy) {
			/* Hopefully a mistake ;B */
/*JP
			You("can't see yourself!");
*/
			You("あなた自身の位置がわからない！");
			return(0);
		}
		for(i = 0; (i  <= ((techlev(tech_no) / 8) + 1) 
			&& isok(u.ux + (i*u.dx), u.uy + (i*u.dy))); i++) {
		    mtmp = m_at(u.ux + (i*u.dx), u.uy + (i*u.dy));
		    if (mtmp && canseemon(mtmp)) break;
		}
		if (!mtmp || !canseemon(mtmp)) {
/*JP
			You("fail to make eye contact with anything!");
*/
			You("見つめるべき相手が見当たらない！");
			return (0);
		}
/*JP
                You("stare at %s.", mon_nam(mtmp));
*/
                You("%sを見つめた．", mon_nam(mtmp));
                if (!haseyes(mtmp->data))
/*JP
                	pline("..but %s has no eyes!", mon_nam(mtmp));
*/
                	pline("．．しかし%sには目がない！", mon_nam(mtmp));
                else if (!mtmp->mcansee)
/*JP
                	pline("..but %s cannot see you!", mon_nam(mtmp));
*/
                	pline("．．しかし%sはあなたを見る事が出来ない！", mon_nam(mtmp));
                if ((rn2(6) + rn2(6) + (techlev(tech_no) - mtmp->m_lev)) > 10) {
/*JP
			You("dazzle %s!", mon_nam(mtmp));
*/
			You("%sを金縛りにした！", mon_nam(mtmp));
			mtmp->mcanmove = 0;
			mtmp->mfrozen = rnd(10);
		} else {
/*JP
                       pline("%s breaks the stare!", Monnam(mtmp));
*/
                       pline("%sはにらみを打ち破った！", Monnam(mtmp));
		}
               	t_timeout = rn1(50,25);
	    	break;
	    case T_BLITZ:
	    	if (uwep || (u.twoweap && uswapwep)) {
/*JP
			You("can't do this while wielding a weapon!");
*/
			pline("武器を持っていてはこの技は使えない！");
	    		return(0);
	    	} else if (uarms) {
/*JP
			You("can't do this while holding a shield!");
*/
			pline("盾を持っていてはこの技は使えない！");
	    		return(0);
	    	}
	    	if (!doblitz()) return (0);		
		
                t_timeout = rn1(1000,500);
	    	break;
            case T_PUMMEL:
	    	if (uwep || (u.twoweap && uswapwep)) {
/*JP
			You("can't do this while wielding a weapon!");
*/
			pline("武器を持っていてはこの技は使えない！");
	    		return(0);
	    	} else if (uarms) {
/*JP
			You("can't do this while holding a shield!");
*/
			pline("盾を持っていてはこの技は使えない！");
	    		return(0);
	    	}
		if (!getdir((char *)0)) return(0);
		if (!u.dx && !u.dy) {
/*JP
			You("flex your muscles.");
*/
			You("筋肉をピクピクさせた．");
			return(0);
		}
            	if (!blitz_pummel()) return(0);
                t_timeout = rn1(1000,500);
		break;
            case T_G_SLAM:
	    	if (uwep || (u.twoweap && uswapwep)) {
/*JP
			You("can't do this while wielding a weapon!");
*/
			pline("武器を持っていてはこの技は使えない！");
	    		return(0);
	    	} else if (uarms) {
/*JP
			You("can't do this while holding a shield!");
*/
			pline("盾を持っていてはこの技は使えない！");
	    		return(0);
	    	}
		if (!getdir((char *)0)) return(0);
		if (!u.dx && !u.dy) {
/*JP
			You("flex your muscles.");
*/
			You("筋肉をピクピクさせた．");
			return(0);
		}
            	if (!blitz_g_slam()) return(0);
                t_timeout = rn1(1000,500);
		break;
            case T_DASH:
		if (!getdir((char *)0)) return(0);
		if (!u.dx && !u.dy) {
/*JP
			You("stretch.");
*/
			You("筋肉を伸ばした．");
			return(0);
		}
            	if (!blitz_dash()) return(0);
                t_timeout = rn1(50, 25);
		break;
            case T_POWER_SURGE:
            	if (!blitz_power_surge()) return(0);
		t_timeout = rn1(1000,500);
		break;            	
            case T_SPIRIT_BOMB:
	    	if (uwep || (u.twoweap && uswapwep)) {
/*JP
			You("can't do this while wielding a weapon!");
*/
			pline("武器を持っていてはこの技は使えない！");
	    		return(0);
	    	} else if (uarms) {
/*JP
			You("can't do this while holding a shield!");
*/
			pline("盾を持っていてはこの技は使えない！");
	    		return(0);
	    	}
		if (!getdir((char *)0)) return(0);
            	if (!blitz_spirit_bomb()) return(0);
		t_timeout = rn1(1000,500);
		break;            	
	    case T_DRAW_BLOOD:
		if (!maybe_polyd(is_vampire(youmonst.data),
		  Race_if(PM_VAMPIRE))) {
		    /* ALI
		     * Otherwise we get problems with what we create:
		     * potions of vampire blood would no longer be
		     * appropriate.
		     */
/*JP
		    You("must be in your natural form to draw blood.");
*/
		    pline("採血するには本来の姿に戻る必要がある．");
		    return(0);
		}
		obj = use_medical_kit(PHIAL, TRUE, "draw blood with");
		if (!obj)
		    return 0;
		if (u.ulevel <= 1) {
/*JP
		    You_cant("seem to find a vein.");
*/
		    You("静脈の位置がわからなかった．");
		    return 0;
		}
		check_unpaid(obj);
		if (obj->quan > 1L)
		    obj->quan--;
		else {
		    obj_extract_self(obj);
		    obfree(obj, (struct obj *)0);
		}
#if 0 /*JP*/
		pline("Using your medical kit, you draw off a phial of your blood.");
		losexp("drawing blood", TRUE);
#else
		pline("%sを使い，自分の血を一瓶抜き取った．"
		    , jtrns_obj('(', "medical kit"));
		losexp("血を抜いて", TRUE);
#endif
		if (u.uexp > 0)
		    u.uexp = newuexp(u.ulevel - 1);
		otmp = mksobj(POT_VAMPIRE_BLOOD, FALSE, FALSE);
		otmp->cursed = obj->cursed;
		otmp->blessed = obj->blessed;
		(void) hold_another_object(otmp,
/*JP
			"You fill, but have to drop, %s!", doname(otmp),
*/
			"いっぱいいっぱいで，%sを落とさなければならない！", doname(otmp),
			(const char *)0);
		t_timeout = rn1(1000, 500);
		break;
	    default:
	    	pline ("Error!  No such effect (%i)", tech_no);
		break;
        }
        if (!can_limitbreak())
	    techtout(tech_no) = (t_timeout * (100 - techlev(tech_no))/100);

	/*By default,  action should take a turn*/
	return(1);
}

/* Whether or not a tech is in use.
 * 0 if not in use, turns left if in use. Tech is done when techinuse == 1
 */
int
tech_inuse(tech_id)
int tech_id;
{
        int i;

        if (tech_id < 1 || tech_id > MAXTECH) {
                impossible ("invalid tech: %d", tech_id);
                return(0);
        }
        for (i = 0; i < MAXTECH; i++) {
                if (techid(i) == tech_id) {
                        return (techt_inuse(i));
                }
        }
	return (0);
}

void
tech_timeout()
{
	int i;
	
        for (i = 0; i < MAXTECH; i++) {
	    if (techid(i) == NO_TECH)
		continue;
	    if (techt_inuse(i)) {
	    	/* Check if technique is done */
	        if (!(--techt_inuse(i)))
	        switch (techid(i)) {
		    case T_EVISCERATE:
/*JP
			You("retract your claws.");
*/
			You("爪を引っ込めた．");
			/* You're using bare hands now,  so new msg for next attack */
			unweapon=TRUE;
			/* Lose berserk status */
			repeat_hit = 0;
			break;
		    case T_BERSERK:
/*JP
			The("red haze in your mind clears.");
*/
			pline("心の中の赤いもやが晴れた．");
			break;
		    case T_KIII:
/*JP
			You("calm down.");
*/
			You("落ち着いた．");
			break;
		    case T_FLURRY:
/*JP
			You("relax.");
*/
			You("緊張が解けた．");
			break;
		    case T_E_FIST:
/*JP
			You("feel the power dissipate.");
*/
			You("力が消えたのを感じた．");
			break;
		    case T_SIGIL_TEMPEST:
/*JP
			pline_The("sigil of tempest fades.");
*/
			pline_The("嵐の印は消えていった．");
			break;
		    case T_SIGIL_CONTROL:
/*JP
			pline_The("sigil of control fades.");
*/
			pline_The("操りの印は消えていった．");
			break;
		    case T_SIGIL_DISCHARGE:
/*JP
			pline_The("sigil of discharge fades.");
*/
			pline_The("解放の印は消えていった．");
			break;
		    case T_RAGE:
/*JP
			Your("anger cools.");
*/
			Your("怒りは鎮まった．");
			break;
		    case T_POWER_SURGE:
/*JP
			pline_The("awesome power within you fades.");
*/
			pline("あなたの内の偉大な力は消えていった．");
			break;
		    case T_BLINK:
/*JP
			You("sense the flow of time returning to normal.");
*/
			pline("あなたの時の流れは元に戻った．");
			break;
		    case T_CHI_STRIKE:
/*JP
			You("feel the power in your hands dissipate.");
*/
			You("手に宿った力が消えたのを感じた．");
			break;
		    case T_CHI_HEALING:
/*JP
			You("feel the healing power dissipate.");
*/
			You("癒しの力が消えたのを感じた．");
			break;
	            default:
	            	break;
	        } else switch (techid(i)) {
	        /* During the technique */
		    case T_RAGE:
			/* Bleed but don't kill */
			if (u.uhpmax > 1) u.uhpmax--;
			if (u.uhp > 1) u.uhp--;
			break;
		    case T_POWER_SURGE:
			/* Bleed off power.  Can go to zero as 0 power is not fatal */
			if (u.uenmax > 1) u.uenmax--;
			if (u.uen > 0) u.uen--;
			break;
	            default:
	            	break;
	        }
	    } 

	    if (techtout(i) > 0) techtout(i)--;
        }
}

void
docalm()
{
	int i, tech, n = 0;

	for (i = 0; i < MAXTECH; i++) {
	    tech = techid(i);
	    if (tech != NO_TECH && techt_inuse(i)) {
		aborttech(tech);
		n++;
	    }
	}
	if (n)
/*JP
	    You("calm down.");
*/
	    You("落ち着いた．");
}

static void
hurtmon(mtmp, tmp)
struct monst *mtmp;
int tmp;
{
	mtmp->mhp -= tmp;
	if (mtmp->mhp < 1) killed (mtmp);
#ifdef SHOW_DMG
	else showdmg(tmp);
#endif
}

static const struct 	innate_tech *
role_tech()
{
	switch (Role_switch) {
		case PM_ARCHEOLOGIST:	return (arc_tech);
		case PM_BARBARIAN:	return (bar_tech);
		case PM_CAVEMAN:	return (cav_tech);
		case PM_FLAME_MAGE:	return (fla_tech);
		case PM_HEALER:		return (hea_tech);
		case PM_ICE_MAGE:	return (ice_tech);
		case PM_KNIGHT:		return (kni_tech);
		case PM_MONK: 		return (mon_tech);
		case PM_NECROMANCER:	return (nec_tech);
		case PM_PRIEST:		return (pri_tech);
		case PM_RANGER:		return (ran_tech);
		case PM_ROGUE:		return (rog_tech);
		case PM_SAMURAI:	return (sam_tech);
#ifdef TOURIST        
		case PM_TOURIST:	return (tou_tech);
#endif        
		case PM_UNDEAD_SLAYER:	return (und_tech);
		case PM_VALKYRIE:	return (val_tech);
		case PM_WIZARD:		return (wiz_tech);
#ifdef YEOMAN
		case PM_YEOMAN:		return (yeo_tech);
#endif
		default: 		return ((struct innate_tech *) 0);
	}
}

static const struct     innate_tech *
race_tech()
{
	switch (Race_switch) {
		case PM_DOPPELGANGER:	return (dop_tech);
#ifdef DWARF
		case PM_DWARF:		return (dwa_tech);
#endif
		case PM_ELF:
		case PM_DROW:		return (elf_tech);
		case PM_GNOME:		return (gno_tech);
		case PM_HOBBIT:		return (hob_tech);
		case PM_HUMAN_WEREWOLF:	return (lyc_tech);
		case PM_VAMPIRE:	return (vam_tech);
		default: 		return ((struct innate_tech *) 0);
	}
}

void
adjtech(oldlevel,newlevel)
int oldlevel, newlevel;
{
	const struct   innate_tech  
		*tech = role_tech(), *rtech = race_tech();
	long mask = FROMEXPER;

	while (tech || rtech) {
	    /* Have we finished with the tech lists? */
	    if (!tech || !tech->tech_id) {
	    	/* Try the race intrinsics */
	    	if (!rtech || !rtech->tech_id) break;
	    	tech = rtech;
	    	rtech = (struct innate_tech *) 0;
		mask = FROMRACE;
	    }
		
	    for(; tech->tech_id; tech++)
		if(oldlevel < tech->ulevel && newlevel >= tech->ulevel) {
		    if (tech->ulevel != 1 && !tech_known(tech->tech_id))
#if 0 /*JP*/
			You("learn how to perform %s!",
			  tech_names[tech->tech_id]);
#else
			You("%sの技能の使い方を覚えた！",
			  jtrns_obj('T', tech_names[tech->tech_id]));
#endif
		    learntech(tech->tech_id, mask, tech->tech_lev);
		} else if (oldlevel >= tech->ulevel && newlevel < tech->ulevel
		    && tech->ulevel != 1) {
		    learntech(tech->tech_id, mask, -1);
		    if (!tech_known(tech->tech_id))
#if 0 /*JP*/
			You("lose the ability to perform %s!",
			  tech_names[tech->tech_id]);
#else
			You("%sの技能の使い方を忘れてしまった！",
			  jtrns_obj('T', tech_names[tech->tech_id]));
#endif
		}
	}
}

int
mon_to_zombie(monnum)
int monnum;
{
	if ((&mons[monnum])->mlet == S_ZOMBIE) return monnum;  /* is already zombie */
	if ((&mons[monnum])->mlet == S_KOBOLD) return PM_KOBOLD_ZOMBIE;
	if ((&mons[monnum])->mlet == S_GNOME) return PM_GNOME_ZOMBIE;
	if (is_orc(&mons[monnum])) return PM_ORC_ZOMBIE;
	if (is_dwarf(&mons[monnum])) return PM_DWARF_ZOMBIE;
	if (is_elf(&mons[monnum])) return PM_ELF_ZOMBIE;
	if (is_human(&mons[monnum])) return PM_HUMAN_ZOMBIE;
	if (monnum == PM_ETTIN) return PM_ETTIN_ZOMBIE;
	if (is_giant(&mons[monnum])) return PM_GIANT_ZOMBIE;
	/* Is it humanoid? */
	if (!humanoid(&mons[monnum])) return (-1);
	/* Otherwise,  return a ghoul or ghast */
	if (!rn2(4)) return PM_GHAST;
	else return PM_GHOUL;
}


/*WAC tinker code*/
STATIC_PTR int
tinker(VOID_ARGS)
{
	int chance;
	struct obj *otmp = uwep;


	if (delay) {    /* not if (delay++), so at end delay == 0 */
		delay++;
#if 0
		use_skill(P_TINKER, 1); /* Tinker skill */
#endif
		/*WAC a bit of practice so even if you're interrupted
		you won't be wasting your time ;B*/
		return(1); /* still busy */
	}

	if (!uwep)
		return (0);

/*JP
	You("finish your tinkering.");
*/
	You("加工を終えた．");
	chance = 5;
/*	chance += PSKILL(P_TINKER); */
	if (rnl(10) < chance) {		
		upgrade_obj(otmp);
	} else {
		/* object downgrade  - But for now,  nothing :) */
	}

	setuwep(otmp, FALSE);
/*JP
	You("now hold %s!", doname(otmp));
*/
	You("今%sを手にしている！", doname(otmp));
	return(0);
}

/*WAC  draw energy from surrounding objects */
STATIC_PTR int
draw_energy(VOID_ARGS)
{
	int powbonus = 1;
	if (delay) {    /* not if (delay++), so at end delay == 0 */
		delay++;
		confdir();
		if(isok(u.ux + u.dx, u.uy + u.dy)) {
			switch((&levl[u.ux + u.dx][u.uy + u.dy])->typ) {
			    case ALTAR: /* Divine power */
			    	powbonus =  (u.uenmax > 28 ? u.uenmax / 4
			    			: 7);
				break;
			    case THRONE: /* Regal == pseudo divine */
			    	powbonus =  (u.uenmax > 36 ? u.uenmax / 6
			    			: 6);			    		 	
				break;
			    case CLOUD: /* Air */
			    case TREE: /* Earth */
			    case LAVAPOOL: /* Fire */
			    case ICE: /* Water - most ordered form */
			    	powbonus = 5;
				break;
			    case AIR:
			    case MOAT: /* Doesn't freeze */
			    case WATER:
			    	powbonus = 4;
				break;
			    case POOL: /* Can dry up */
			    	powbonus = 3;
				break;
			    case FOUNTAIN:
			    	powbonus = 2;
				break;
			    case SINK:  /* Cleansing water */
			    	if (!rn2(3)) powbonus = 2;
				break;
			    case TOILET: /* Water Power...but also waste! */
			    	if (rn2(100) < 50)
			    		powbonus = 2;
			    	else powbonus = -2;
				break;
			    case GRAVE:
			    	powbonus = -4;
				break;
			    default:
				break;
			}
		}
		u.uen += powbonus;
		if (u.uen > u.uenmax) {
			delay = 0;
			u.uen = u.uenmax;
		}
		if (u.uen < 1) u.uen = 0;
		flags.botl = 1;
		return(1); /* still busy */
	}
/*JP
	You("finish drawing energy from your surroundings.");
*/
	You("周囲の魔力を吸収し終えた．");
	return(0);
}

static const char 
/*JP
	*Enter_Blitz = "Enter Blitz Command[. to end]: ";
*/
	*Enter_Blitz = "コマンド入力[ピリオド(.)で終了]: ";

/* Keep commands that reference the same blitz together 
 * Keep the BLITZ_START before the BLITZ_CHAIN before the BLITZ_END
 */
static const struct blitz_tab blitzes[] = { 	
	{"LLDDR", 5, blitz_chi_strike, T_CHI_STRIKE, BLITZ_START},
	{"LLDDRDR", 7, blitz_chi_strike, T_CHI_STRIKE, BLITZ_START},
	{"RR",  2, blitz_dash, T_DASH, BLITZ_START},
	{"LL",  2, blitz_dash, T_DASH, BLITZ_START},
	{"UURRDDL", 7, blitz_e_fist, T_E_FIST, BLITZ_START},
	{"URURRDDLDL", 10, blitz_e_fist, T_E_FIST, BLITZ_START},
	{"DDRRDDRR", 8, blitz_power_surge, T_POWER_SURGE, BLITZ_START},
	{"DRDRDRDR", 8, blitz_power_surge, T_POWER_SURGE, BLITZ_START},
	{"LRL", 3, blitz_pummel, T_PUMMEL, BLITZ_CHAIN},
	{"RLR", 3, blitz_pummel, T_PUMMEL, BLITZ_CHAIN},
	{"DDDD", 4, blitz_g_slam, T_G_SLAM, BLITZ_END},
	{"DUDUUDDD", 8, blitz_spirit_bomb, T_SPIRIT_BOMB, BLITZ_END},
	{"", 0, (void *)0, 0, BLITZ_END} /* Array terminator */
};

#define MAX_BLITZ 50
#define MIN_CHAIN 2
#define MAX_CHAIN 5

/* parse blitz input */
static int
doblitz()
{
	int i, j, dx, dy, bdone = 0, tech_no;
	char buf[BUFSZ];
	char *bp;
	int blitz_chain[MAX_CHAIN], blitz_num;
        
	tech_no = (get_tech_no(T_BLITZ));

	if (tech_no == -1) {
		return(0);
	}
	
	if (u.uen < 10) {
/*JP
		You("are too weak to attempt this!");
*/
		You("それを行えないほど衰弱している！");
            	return(0);
	}

	bp = buf;
	
	if (!getdir((char *)0)) return(0);
	if (!u.dx && !u.dy) {
		return(0);
	}
	
	dx = u.dx;
	dy = u.dy;

	doblitzlist();

    	for (i= 0; i < MAX_BLITZ; i++) {
		if (!getdir(Enter_Blitz)) return(0); /* Get directional input */
    		if (!u.dx && !u.dy && !u.dz) break;
    		if (u.dx == -1) {
    			*(bp) = 'L';
    			bp++;
    		} else if (u.dx == 1) {
    			*(bp) = 'R';
    			bp++;
    		}
    		if (u.dy == -1) {
    			*(bp) = 'U';
    			bp++;
    		} else if (u.dy == 1) {
    			*(bp) = 'D';
    			bp++;
    		}
    		if (u.dz == -1) {
    			*(bp) = '>';
    			bp++;
    		} else if (u.dz == 1) {
    			*(bp) = '<';
    			bp++;
    		}
    	}
	*(bp) = '.';
	bp++;
	*(bp) = '\0';
	bp = buf;

	/* Point of no return - You've entered and terminated a blitz, so... */
    	u.uen -= 10;

    	/* parse input */
    	/* You can't put two of the same blitz in a row */
    	blitz_num = 0;
    	while(strncmp(bp, ".", 1)) {
	    bdone = 0;
	    for (j = 0; blitzes[j].blitz_len; j++) {
	    	if (blitz_num >= MAX_CHAIN || 
	    	    blitz_num >= (MIN_CHAIN + (techlev(tech_no) / 10)))
	    		break; /* Trying to chain too many blitz commands */
		else if (!strncmp(bp, blitzes[j].blitz_cmd, blitzes[j].blitz_len)) {
	    		/* Trying to chain in a command you don't know yet */
			if (!tech_known(blitzes[j].blitz_tech))
				break;
	    		if (blitz_num) {
				/* Check if trying to chain two of the exact same 
				 * commands in a row 
				 */
	    			if (j == blitz_chain[(blitz_num - 1)]) 
	    				break;
	    			/* Trying to chain after chain finishing command */
	    			if (blitzes[blitz_chain[(blitz_num - 1)]].blitz_type 
	    							== BLITZ_END)
	    				break;
	    			/* Trying to put a chain starter after starting
	    			 * a chain
	    			 * Note that it's OK to put two chain starters in a 
	    			 * row
	    			 */
	    			if ((blitzes[j].blitz_type == BLITZ_START) &&
	    			    (blitzes[blitz_chain[(blitz_num - 1)]].blitz_type 
	    							!= BLITZ_START))
	    				break;
	    		}
			bp += blitzes[j].blitz_len;
			blitz_chain[blitz_num] = j;
			blitz_num++;
			bdone = 1;
			break;
		}
	    }
	    if (!bdone) {
/*JP
		You("stumble!");
*/
		You("こけた！");
		return(1);
	    }
    	}
	for (i = 0; i < blitz_num; i++) {
	    u.dx = dx;
	    u.dy = dy;
	    if (!( (*blitzes[blitz_chain[i]].blitz_funct)() )) break;
	}
	
    	/* done */
	return(1);
}

static void
doblitzlist()
{
	winid tmpwin;
	int i, n;
	char buf[BUFSZ];
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;         /* zero out all bits */

#if 0 /*JP*/
        Sprintf(buf, "%16s %10s %-17s", "[LU = Left Up]", "[U = Up]", "[RU = Right Up]");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
        Sprintf(buf, "%16s %10s %-17s", "[L = Left]", "", "[R = Right]");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
        Sprintf(buf, "%16s %10s %-17s", "[LD = Left Down]", "[D = Down]", "[RD = Right Down]");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);        
#else
        Sprintf(buf, "%16s %10s %-17s", "[LU = 左上]", "[U = 上]", "[RU = 右上]");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
        Sprintf(buf, "%16s %10s %-17s", "[L = 左]", "", "[R = 右]");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
        Sprintf(buf, "%16s %10s %-17s", "[LD = 左下]", "[D = 下]", "[RD = 右下]");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
#endif

        Sprintf(buf, "%-30s %10s   %s", "Name", "Type", "Command");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);

        for (i = 0; blitzes[i].blitz_len; i++) {
	    if (tech_known(blitzes[i].blitz_tech)) {
                Sprintf(buf, "%-30s %10s   %s",
                    (i && blitzes[i].blitz_tech == blitzes[(i-1)].blitz_tech ?
/*JP
                    	"" : tech_names[blitzes[i].blitz_tech]), 
*/
                	"" : jtrns_obj('T', tech_names[blitzes[i].blitz_tech])), 
                    (blitzes[i].blitz_type == BLITZ_START ? 
/*JP
                    	"starter" :
*/
                    	"開始" :
                    	(blitzes[i].blitz_type == BLITZ_CHAIN ? 
/*JP
	                    	"chain" : 
*/
	                    	"連鎖" : 
	                    	(blitzes[i].blitz_type == BLITZ_END ? 
/*JP
                    			"finisher" : "unknown"))),
*/
                    			"決め技" : "不明"))),
                    blitzes[i].blitz_cmd);

		add_menu(tmpwin, NO_GLYPH, &any,
                         0, 0, ATR_NONE, buf, MENU_UNSELECTED);
	    }
	}
/*JP
        end_menu(tmpwin, "Currently known blitz manoeuvers");
*/
        end_menu(tmpwin, "現在判明している連続技の一覧");

	n = select_menu(tmpwin, PICK_NONE, &selected);
	destroy_nhwindow(tmpwin);
	return;
}

static int
blitz_chi_strike(VOID_ARGS)
{
	int tech_no;
	
	tech_no = (get_tech_no(T_CHI_STRIKE));

	if (tech_no == -1) {
		return(0);
	}

	if (u.uen < 1) {
/*JP
		You("are too weak to attempt this!");
*/
		You("それを行えないほど衰弱している！");
            	return(0);
	}
/*JP
	You("feel energy surge through your hands!");
*/
	You("拳が光ってうなるのを感じた！");
	techt_inuse(tech_no) = techlev(tech_no) + 4;
	return(1);
}

static int
blitz_e_fist(VOID_ARGS)
{
	int tech_no;
	const char *str;
	
	tech_no = (get_tech_no(T_E_FIST));

	if (tech_no == -1) {
		return(0);
	}
	
	str = makeplural(body_part(HAND));
/*JP
	You("focus the powers of the elements into your %s.", str);
*/
	You("精霊の力を%sに込めた．", str);
	techt_inuse(tech_no) = rnd((int) (techlev(tech_no)/3 + 1)) + d(1,4) + 2;
	return 1;
}

/* Assumes u.dx, u.dy already set up */
static int
blitz_pummel(VOID_ARGS)
{
	int i = 0, tech_no;
	struct monst *mtmp;
	tech_no = (get_tech_no(T_PUMMEL));

	if (tech_no == -1) {
		return(0);
	}

/*JP
	You("let loose a barrage of blows!");
*/
	You("連続で拳を放った！");

	if (u.uswallow)
	    mtmp = u.ustuck;
	else
	    mtmp = m_at(u.ux + u.dx, u.uy + u.dy);

	if (!mtmp) {
/*JP
		You("strike nothing.");
*/
		pline("何にも当たらなかった．");
		return (0);
	}
	if (!attack(mtmp)) return (0);
	
	/* Perform the extra attacks
	 */
	for (i = 0; (i < 4); i++) {
	    if (rn2(70) > (techlev(tech_no) + 30)) break;

	    if (u.uswallow)
		mtmp = u.ustuck;
	    else
		mtmp = m_at(u.ux + u.dx, u.uy + u.dy);

	    if (!mtmp) return (1);
	    if (!attack(mtmp)) return (1);
	} 
	
	return(1);
}

/* Assumes u.dx, u.dy already set up */
static int
blitz_g_slam(VOID_ARGS)
{
	int tech_no, tmp, canhitmon, objenchant;
	struct monst *mtmp;
	struct trap *chasm;

	tech_no = (get_tech_no(T_G_SLAM));

	if (tech_no == -1) {
		return(0);
	}

	mtmp = m_at(u.ux + u.dx, u.uy + u.dy);
	if (!mtmp) {
/*JP
		You("strike nothing.");
*/
		pline("何にも当たらなかった．");
		return (0);
	}
	if (!attack(mtmp)) return (0);

	/* Slam the monster into the ground */
	mtmp = m_at(u.ux + u.dx, u.uy + u.dy);
	if (!mtmp || u.uswallow) return(1);

/*JP
	You("hurl %s downwards...", mon_nam(mtmp));
*/
	You("%sを下方に投げ飛ばした．．．", mon_nam(mtmp));
	if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) return(1);

	if (need_four(mtmp)) canhitmon = 4;
	else if (need_three(mtmp)) canhitmon = 3;
	else if (need_two(mtmp)) canhitmon = 2;
	else if (need_one(mtmp)) canhitmon = 1;
	else canhitmon = 0;
	if (Upolyd) {
	    if (hit_as_four(&youmonst))	objenchant = 4;
	    else if (hit_as_three(&youmonst)) objenchant = 3;
	    else if (hit_as_two(&youmonst)) objenchant = 2;
	    else if (hit_as_one(&youmonst)) objenchant = 1;
	    else if (need_four(&youmonst)) objenchant = 4;
	    else if (need_three(&youmonst)) objenchant = 3;
	    else if (need_two(&youmonst)) objenchant = 2;
	    else if (need_one(&youmonst)) objenchant = 1;
	    else objenchant = 0;
	} else
	    objenchant = u.ulevel / 4;

	tmp = (5 + rnd(6) + (techlev(tech_no) / 5));
	
	chasm = maketrap(u.ux + u.dx, u.uy + u.dy, PIT);
	if (chasm) {
	    if (!is_flyer(mtmp->data) && !is_clinger(mtmp->data))
		mtmp->mtrapped = 1;
	    chasm->tseen = 1;
	    levl[u.ux + u.dx][u.uy + u.dy].doormask = 0;
/*JP
	    pline("%s slams into the ground, creating a crater!", Monnam(mtmp));
*/
	    pline("%sは地面に叩き付けられ，クレーターが出来た！", Monnam(mtmp));
	    tmp *= 2;
	}

/*JP
	mselftouch(mtmp, "Falling, ", TRUE);
*/
	mselftouch(mtmp, "落下中，", TRUE);
	if (!DEADMONSTER(mtmp)) {
	    if (objenchant < canhitmon)
/*JP
		pline("%s doesn't seem to be harmed.", Monnam(mtmp));
*/
		pline("%sは傷ついたように見えない．", Monnam(mtmp));
	    else if ((mtmp->mhp -= tmp) <= 0) {
		if(!cansee(u.ux + u.dx, u.uy + u.dy))
/*JP
		    pline("It is destroyed!");
*/
		    pline("何者かは死んだ！");
		else {
#if 0 /*JP*/
		    You("destroy %s!", 	
		    	mtmp->mtame
			    ? x_monnam(mtmp, ARTICLE_THE, "poor",
				mtmp->mnamelth ? SUPPRESS_SADDLE : 0, FALSE)
			    : mon_nam(mtmp));
#else
		    pline("%s%sは死んだ！", mtmp->mtame ? "かわいそうな" : "",
			      mon_nam(mtmp));
#endif
		}
		xkilled(mtmp,0);
	    }
	}

	return(1);
}

/* Assumes u.dx, u.dy already set up */
static int
blitz_dash(VOID_ARGS)
{
	int tech_no;
	tech_no = (get_tech_no(T_DASH));

	if (tech_no == -1) {
		return(0);
	}
	
	if ((!Punished || carried(uball)) && !u.utrap)
/*JP
	    You("dash forwards!");
*/
	    You("前方にダッシュした！");
	hurtle(u.dx, u.dy, 2, FALSE);
	multi = 0;		/* No paralysis with dash */
	return 1;
}

static int
blitz_power_surge(VOID_ARGS)
{
	int tech_no, num;
	
	tech_no = (get_tech_no(T_POWER_SURGE));

	if (tech_no == -1) {
		return(0);
	}

	if (Upolyd) {
/*JP
		You("cannot tap into your full potential in this form.");
*/
		pline("この姿ではあなたの力を最大限引き出す事はできない．");
		return(0);
	}
/*JP
    	You("tap into the full extent of your power!");
*/
    	You("ステップを踏み，力を最大限まで高めた！");
	num = 50 + (2 * techlev(tech_no));
    	techt_inuse(tech_no) = num + 1;
	u.uenmax += num;
	u.uen = u.uenmax;
	return 1;
}

/* Assumes u.dx, u.dy already set up */
static int
blitz_spirit_bomb(VOID_ARGS)
{
	int tech_no, num;
	int sx = u.ux, sy = u.uy, i;
	
	tech_no = (get_tech_no(T_SPIRIT_BOMB));

	if (tech_no == -1) {
		return(0);
	}

/*JP
	You("gather your energy...");
*/
	You("魔力を集めはじめた．．．");
	
	if (u.uen < 10) {
/*JP
		pline("But it fizzles out.");
*/
		pline("しかしそれは散ってしまった．");
		u.uen = 0;
	}

	num = 10 + (techlev(tech_no) / 5);
	num = (u.uen < num ? u.uen : num);
	
	u.uen -= num;
	
	for( i = 0; i < 2; i++) {		
	    if (!isok(sx,sy) || !cansee(sx,sy) || 
	    		IS_STWALL(levl[sx][sy].typ) || u.uswallow)
	    	break;

	    /* Display the center of the explosion */
	    tmp_at(DISP_FLASH, explosion_to_glyph(EXPL_MAGICAL, S_explode5));
	    tmp_at(sx, sy);
	    delay_output();
	    tmp_at(DISP_END, 0);

	    sx += u.dx;
	    sy += u.dy;
	}
	/* Magical Explosion */
	explode(sx, sy, 10, (d(3,6) + num), WAND_CLASS, EXPL_MAGICAL);
	return 1;
}

#ifdef DEBUG
void
wiz_debug_cmd() /* in this case, allow controlled loss of techniques */
{
	int tech_no, id, n = 0;
	long mask;
	if (gettech(&tech_no)) {
		id = techid(tech_no);
		if (id == NO_TECH) {
		    impossible("Unknown technique ([%d])?", tech_no);
		    return;
		}
		mask = tech_list[tech_no].t_intrinsic;
		if (mask & FROMOUTSIDE) n++;
		if (mask & FROMRACE) n++;
		if (mask & FROMEXPER) n++;
		if (!n) {
		    impossible("No intrinsic masks set (0x%lX).", mask);
		    return;
		}
		n = rn2(n);
		if (mask & FROMOUTSIDE && !n--) mask = FROMOUTSIDE;
		if (mask & FROMRACE && !n--) mask = FROMRACE;
		if (mask & FROMEXPER && !n--) mask = FROMEXPER;
		learntech(id, mask, -1);
		if (!tech_known(id))
		    You("lose the ability to perform %s.", tech_names[id]);
	}
}
#endif /* DEBUG */
