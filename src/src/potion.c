/*	SCCS Id: @(#)potion.c	3.4	2002/10/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, Shiganai Sakusha, 2004-2008
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"


/* KMH, intrinsics patch
 * There are many changes here to support >32-bit properties.
 * Also, blessed potions are once again permitted to convey
 * permanent intrinsics.
 */


#ifdef OVLB
boolean notonhead = FALSE;

static NEARDATA int nothing, unkn;
static NEARDATA const char beverages[] = { POTION_CLASS, 0 };

STATIC_DCL long FDECL(itimeout, (long));
STATIC_DCL long FDECL(itimeout_incr, (long,int));
STATIC_DCL void NDECL(ghost_from_bottle);
STATIC_DCL short FDECL(mixtype, (struct obj *,struct obj *));

STATIC_DCL void FDECL(healup_mon, (struct monst *, int,int,BOOLEAN_P,BOOLEAN_P));
	/* For healing monsters - analogous to healup for players */


/* force `val' to be within valid range for intrinsic timeout value */
STATIC_OVL long
itimeout(val)
long val;
{
    if (val >= TIMEOUT) val = TIMEOUT;
    else if (val < 1) val = 0;

    return val;
}

/* increment `old' by `incr' and force result to be valid intrinsic timeout */
STATIC_OVL long
itimeout_incr(old, incr)
long old;
int incr;
{
    return itimeout((old & TIMEOUT) + (long)incr);
}

/* set the timeout field of intrinsic `which' */
void
set_itimeout(which, val)
long *which, val;
{
    *which &= ~TIMEOUT;
    *which |= itimeout(val);
}

/* increment the timeout field of intrinsic `which' */
void
incr_itimeout(which, incr)
long *which;
int incr;
{
    set_itimeout(which, itimeout_incr(*which, incr));
}

void
make_confused(xtime,talk)
long xtime;
boolean talk;
{
	long old = HConfusion;

	if (!xtime && old) {
		if (talk)
#if 0 /*JP*/
		    You_feel("less %s now.",
			Hallucination ? "trippy" : "confused");
#else
		    You("%sがおさまった．",
			Hallucination ? "ヘロヘロ" : "混乱");
#endif
	}
	if ((xtime && !old) || (!xtime && old)) flags.botl = TRUE;

	set_itimeout(&HConfusion, xtime);
}

void
make_stunned(xtime,talk)
long xtime;
boolean talk;
{
	long old = HStun;

	if (!xtime && old) {
		if (talk)
#if 0 /*JP*/
		    You_feel("%s now.",
			Hallucination ? "less wobbly" : "a bit steadier");
#else
		    You_feel("%s．",
			Hallucination ? "へろりらがおさまった" : "だんだんしっかりしてきた");
#endif
	}
	if (xtime && !old) {
		if (talk) {
#ifdef STEED
			if (u.usteed)
/*JP
				You("wobble in the saddle.");
*/
				You("鞍の上でぐらぐらした．");
			else
#endif
/*JP
			You("%s...", stagger(youmonst.data, "stagger"));
*/
			You("%s．．．", stagger(youmonst.data, "くらくらした"));
		}
	}
	if ((!xtime && old) || (xtime && !old)) flags.botl = TRUE;

	set_itimeout(&HStun, xtime);
}

void
make_sick(xtime, cause, talk, type)
long xtime;
const char *cause;	/* sickness cause */
boolean talk;
int type;
{
	long old = Sick;

	if (xtime > 0L) {
	    if (Sick_resistance) return;
	    if (!old) {
		/* newly sick */
/*JP
		You_feel("deathly sick.");
*/
		You("病気で死にそうだ．");
	    } else {
		/* already sick */
#if 0 /*JP*/
		if (talk) You_feel("%s worse.",
			      xtime <= Sick/2L ? "much" : "even");
#else
		if (talk) Your("病状は%s悪化したようだ．",
			      xtime <= Sick/2L ? "致命的に" : "さらに");
#endif
	    }
	    set_itimeout(&Sick, xtime);
	    u.usick_type |= type;
	    flags.botl = TRUE;
	} else if (old && (type & u.usick_type)) {
	    /* was sick, now not */
	    u.usick_type &= ~type;
	    if (u.usick_type) { /* only partly cured */
/*JP
		if (talk) You_feel("somewhat better.");
*/
		if (talk) You("ちょっとよくなった．");
		set_itimeout(&Sick, Sick * 2); /* approximation */
	    } else {
/*JP
		if (talk) pline("What a relief!");
*/
		if (talk) pline("ああ助かった！");
		Sick = 0L;		/* set_itimeout(&Sick, 0L) */
	    }
	    flags.botl = TRUE;
	}

	if (Sick) {
	    exercise(A_CON, FALSE);
	    if (cause) {
		(void) strncpy(u.usick_cause, cause, sizeof(u.usick_cause));
		u.usick_cause[sizeof(u.usick_cause)-1] = 0;
		}
	    else
		u.usick_cause[0] = 0;
	} else
	    u.usick_cause[0] = 0;
}

void
make_vomiting(xtime, talk)
long xtime;
boolean talk;
{
	long old = Vomiting;

	if(!xtime && old)
/*JP
	    if(talk) You_feel("much less nauseated now.");
*/
	    if(talk) You("吐き気がおさまった．");

	set_itimeout(&Vomiting, xtime);
}

/*JP
static const char vismsg[] = "vision seems to %s for a moment but is %s now.";
*/
static const char vismsg[] = "視界は一瞬%sが，%s．";
/*JP
static const char eyemsg[] = "%s momentarily %s.";
*/
static const char eyemsg[] = "%sは一瞬%s．";

void
make_blinded(xtime, talk)
long xtime;
boolean talk;
{
	long old = Blinded;
	boolean u_could_see, can_see_now;
#if 0 /*JP*/
	int eyecnt;
	char buf[BUFSZ];
#endif

	/* we need to probe ahead in case the Eyes of the Overworld
	   are or will be overriding blindness */
	u_could_see = !Blind;
	Blinded = xtime ? 1L : 0L;
	can_see_now = !Blind;
	Blinded = old;		/* restore */

	if (u.usleep) talk = FALSE;

	if (can_see_now && !u_could_see) {	/* regaining sight */
	    if (talk) {
		if (Hallucination)
/*JP
		    pline("Far out!  Everything is all cosmic again!");
*/
		    pline("素晴らしい！また何もかもが全宇宙的視野だ！");
		else
/*JP
		    You("can see again.");
*/
		    You("また見えるようになった．");
	    }
	} else if (old && !xtime) {
	    /* clearing temporary blindness without toggling blindness */
	    if (talk) {
		if (!haseyes(youmonst.data)) {
		    strange_feeling((struct obj *)0, (char *)0);
		} else if (Blindfolded) {
#if 0 /*JP*/
		    Strcpy(buf, body_part(EYE));
		    eyecnt = eyecount(youmonst.data);
		    Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
			 (eyecnt == 1) ? "itches" : "itch");
#else
		    Your(eyemsg, body_part(EYE), "かゆくなった");
#endif
		} else {	/* Eyes of the Overworld */
#if 0 /*JP*/
		    Your(vismsg, "brighten",
			 Hallucination ? "sadder" : "normal");
#else
		    Your(vismsg, "まぶしく輝いた",
			 Hallucination ? "再び陰惨な状態に逆戻りした" : "すぐに元に戻った");
#endif
		}
	    }
	}

	if (u_could_see && !can_see_now) {	/* losing sight */
	    if (talk) {
		if (Hallucination)
/*JP
		    pline("Oh, bummer!  Everything is dark!  Help!");
*/
		    pline("暗いよー，狭いよー，恐いよー！");
		else
/*JP
		    pline("A cloud of darkness falls upon you.");
*/
		    pline("暗黒の雲があなたを覆った．");
	    }
	    /* Before the hero goes blind, set the ball&chain variables. */
	    if (Punished) set_bc(0);
	} else if (!old && xtime) {
	    /* setting temporary blindness without toggling blindness */
	    if (talk) {
		if (!haseyes(youmonst.data)) {
		    strange_feeling((struct obj *)0, (char *)0);
		} else if (Blindfolded) {
#if 0 /*JP*/
		    Strcpy(buf, body_part(EYE));
		    eyecnt = eyecount(youmonst.data);
		    Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
			 (eyecnt == 1) ? "twitches" : "twitch");
#else
		    Your(eyemsg, body_part(EYE), "ピクピクした");
#endif
		} else {	/* Eyes of the Overworld */
#if 0 /*JP*/
		    Your(vismsg, "dim",
			 Hallucination ? "happier" : "normal");
#else
		    Your(vismsg, "薄暗くなった",
			 Hallucination ? "再び幸福な状態になった" : "すぐに元に戻った");
#endif
		}
	    }
	}

	set_itimeout(&Blinded, xtime);

	if (u_could_see ^ can_see_now) {  /* one or the other but not both */
	    flags.botl = 1;
	    vision_full_recalc = 1;	/* blindness just got toggled */
	    if (Blind_telepat || Infravision) see_monsters();
	}
}

boolean
make_hallucinated(xtime, talk, mask)
long xtime;	/* nonzero if this is an attempt to turn on hallucination */
boolean talk;
long mask;	/* nonzero if resistance status should change by mask */
{
	long old = HHallucination;
	boolean changed = 0;
	const char *message, *verb;

#if 0 /*JP*/
	message = (!xtime) ? "Everything %s SO boring now." :
			     "Oh wow!  Everything %s so cosmic!";
#else
	message = (!xtime) ? "何もかもが＊退屈＊に%sる．" :
			     "ワーオ！何もかも全宇宙的視野で%sる！";
#endif
/*JP
	verb = (!Blind) ? "looks" : "feels";
*/
	verb = (!Blind) ? "見え" : "感じ";

	if (mask) {
	    if (HHallucination) changed = TRUE;

	    if (!xtime) EHalluc_resistance |= mask;
	    else EHalluc_resistance &= ~mask;
	} else {
	    if (!EHalluc_resistance && (!!HHallucination != !!xtime))
		changed = TRUE;
	    set_itimeout(&HHallucination, xtime);

	    /* clearing temporary hallucination without toggling vision */
	    if (!changed && !HHallucination && old && talk) {
		if (!haseyes(youmonst.data)) {
		    strange_feeling((struct obj *)0, (char *)0);
		} else if (Blind) {
#if 0 /*JP*/
		    char buf[BUFSZ];
		    int eyecnt = eyecount(youmonst.data);

		    Strcpy(buf, body_part(EYE));
		    Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
			 (eyecnt == 1) ? "itches" : "itch");
#else
		    Your(eyemsg, body_part(EYE), "かゆくなった");
#endif
		} else {	/* Grayswandir */
/*JP
		    Your(vismsg, "flatten", "normal");
*/
		    Your(vismsg, "平凡になった", "すぐに元に戻った");
		}
	    }
	}

	if (changed) {
	    if (u.uswallow) {
		swallowed(0);	/* redraw swallow display */
	    } else {
		/* The see_* routines should be called *before* the pline. */
		see_monsters();
		see_objects();
		see_traps();
	    }

	    /* for perm_inv and anything similar
	    (eg. Qt windowport's equipped items display) */
	    update_inventory();

	    flags.botl = 1;
	    if (talk) pline(message, verb);
	}
	return changed;
}

STATIC_OVL void
ghost_from_bottle()
{
	struct monst *mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, NO_MM_FLAGS);

	if (!mtmp) {
/*JP
		pline("This bottle turns out to be empty.");
*/
		pline("瓶は空っぽだった．");
		return;
	}
	if (Blind) {
/*JP
		pline("As you open the bottle, %s emerges.", something);
*/
		pline("瓶を開けると，何かが飛び出てきた．");
		return;
	}
/*JP
	pline("As you open the bottle, an enormous %s emerges!",
*/
	pline("瓶を開けると，中から巨大な%sが出てきた！",
/*JP
		Hallucination ? rndmonnam() : (const char *)"ghost");
*/
		Hallucination ? rndmonnam() : (const char *)"幽霊");
	if(flags.verbose)
/*JP
	    You("are frightened to death, and unable to move.");
*/
	    You("まっさおになって驚き，動けなくなった．");
	nomul(-3);
/*JP
	nomovemsg = "You regain your composure.";
*/
	nomovemsg = "あなたは平静を取り戻した．";
}

/* "Quaffing is like drinking, except you spill more."  -- Terry Pratchett
 */
int
dodrink()
{
	register struct obj *otmp;
	const char *potion_descr;
	char quaffables[SIZE(beverages) + 2];
	char *qp = quaffables;

	if (Strangled) {
/*JP
		pline("If you can't breathe air, how can you drink liquid?");
*/
		pline("息もできないのに，どうやって液体を飲むんだい？");
		return 0;
	}

	*qp++ = ALLOW_FLOOROBJ;
	if (!u.uswallow && (IS_FOUNTAIN(levl[u.ux][u.uy].typ) ||
#ifdef SINKS
			    IS_SINK(levl[u.ux][u.uy].typ) ||
			    IS_TOILET(levl[u.ux][u.uy].typ) ||
#endif
			    Underwater || IS_POOL(levl[u.ux][u.uy].typ)))
	    *qp++ = ALLOW_THISPLACE;
	Strcpy(qp, beverages);

	otmp = getobj(quaffables, "drink");
	if (otmp == &thisplace) {
	    if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		drinkfountain();
		return 1;
	    }
#ifdef SINKS
	    else if (IS_SINK(levl[u.ux][u.uy].typ)) {
		drinksink();
		return 1;
	    }
	    else if (IS_TOILET(levl[u.ux][u.uy].typ)) {
		drinktoilet();
		return 1;
	    }
#endif
/*JP
	    pline("Do you know what lives in this water!");
*/
	    pline("この水中にはどんな生物がいるのか知っているのかい！");
	    return 1;
	}
	if(!otmp) return(0);
	otmp->in_use = TRUE;		/* you've opened the stopper */

#define POTION_OCCUPANT_CHANCE(n) (13 + 2*(n))	/* also in muse.c */

	potion_descr = OBJ_DESCR(objects[otmp->otyp]);
	if (potion_descr) {
	    if (!strcmp(potion_descr, "milky") &&
		    flags.ghost_count < MAXMONNO &&
		    !rn2(POTION_OCCUPANT_CHANCE(flags.ghost_count))) {
		ghost_from_bottle();
		if (carried(otmp)) useup(otmp);
		else useupf(otmp, 1L);
		return(1);
	    } else if (!strcmp(potion_descr, "smoky") &&
		    (flags.djinni_count < MAXMONNO) &&
		    !rn2(POTION_OCCUPANT_CHANCE(flags.djinni_count))) {
		djinni_from_bottle(otmp);
		if (carried(otmp)) useup(otmp);
		else useupf(otmp, 1L);
		return(1);
	    }
	}
	return dopotion(otmp);
}

int
dopotion(otmp)
register struct obj *otmp;
{
	int retval;

	otmp->in_use = TRUE;
	nothing = unkn = 0;


	if((retval = peffects(otmp)) >= 0) return(retval);

	if(nothing) {
	    unkn++;
#if 0 /*JP*/
	    You("have a %s feeling for a moment, then it passes.",
		  Hallucination ? "normal" : "peculiar");
#else
	    You("%s気分におそわれたが，すぐに消え去った．",
		  Hallucination ? "ありきたりな" : "独特な");
#endif
	}
	if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
		if(!unkn) {
			makeknown(otmp->otyp);
			more_experienced(0,10);
		} else if(!objects[otmp->otyp].oc_uname)
			docall(otmp);
	}
	if (carried(otmp)) useup(otmp);
	else if (mcarried(otmp)) m_useup(otmp->ocarry, otmp);
	else if (otmp->where == OBJ_FLOOR) useupf(otmp, 1L);
	else dealloc_obj(otmp);		/* Dummy potion */
	return(1);
}

/* return -1 if potion is used up,  0 if error,  1 not used */
int
peffects(otmp)
	register struct obj	*otmp;
{
	register int i, ii, lim;


	/* KMH, balance patch -- this is too cruel for novices */
#if 0
	/* sometimes your constitution can be a little _too_ high! */
	if ((Role_if(PM_BARBARIAN) || ACURR(A_CON) > 15) && !rn2(5)) {
		pline("Strange ...");
		nothing++;
		return(-1);
	}
#endif

	switch(otmp->otyp){
	case POT_RESTORE_ABILITY:
	case SPE_RESTORE_ABILITY:
		unkn++;
		if(otmp->cursed) {
/*JP
		    pline("Ulch!  This makes you feel mediocre!");
*/
		    pline("うげ！ 自分が凡庸になってしまったような気がする！");
		    break;
		} else {
#if 0 /*JP*/
		    pline("Wow!  This makes you feel %s!",
			  (otmp->blessed) ?
				(unfixable_trouble_count(FALSE) ? "better" : "great")
			  : "good");
#else
		    pline("ワーオ！気分が%sよくなった！",
			  (otmp->blessed) ?
				(unfixable_trouble_count(FALSE) ? "とても" : "より")
			  : "");
#endif
		    i = rn2(A_MAX);		/* start at a random point */
		    for (ii = 0; ii < A_MAX; ii++) {
			lim = AMAX(i);
			if (i == A_STR && u.uhs >= 3) --lim;	/* WEAK */
			if (ABASE(i) < lim) {
			    ABASE(i) = lim;
			    flags.botl = 1;
			    /* only first found if not blessed */
			    if (!otmp->blessed) break;
			}
			if(++i >= A_MAX) i = 0;
		    }
		}
		break;
	case POT_HALLUCINATION:
		if (Hallucination || Halluc_resistance) nothing++;
		else makeknown(otmp->otyp);
		(void) make_hallucinated(itimeout_incr(HHallucination,
					   rn1(200, 600 - 300 * bcsign(otmp))),
				  TRUE, 0L);
		break;
	case POT_AMNESIA:
#if 0 /*JP*/
		pline(Hallucination? "This tastes like champagne!" :
			"This liquid bubbles and fizzes as you drink it.");
#else
		pline(Hallucination? "これはシャンパンによく似た味だ！" :
			"あなたが飲むと液体はシュッという音を立てて泡立った．");
#endif
		forget((!otmp->blessed? ALL_SPELLS : 0) | ALL_MAP);
		if (Hallucination)
#if 0 /*JP*/
		    pline("Hakuna matata!");
#else
		    /* スワヒリ語で「小さい事は気にするな」の意 */
		    pline("ハクナマタタ！");
#endif
		else
#if 0 /*JP*/
		    You_feel("your memories dissolve.");
#else
		    You("記憶がぼやけていくのを感じた．");
#endif

		/* Blessed amnesia makes you forget lycanthropy, sickness */
		if (otmp->blessed) {
		    if (u.ulycn >= LOW_PM && !Race_if(PM_HUMAN_WEREWOLF)) {
#if 0 /*JP*/
			You("forget your affinity to %s!",
					makeplural(mons[u.ulycn].mname));
#else
			You("%sへの親近感を忘れ去った！",
					jtrns_mon(mons[u.ulycn].mname));
#endif
			if (youmonst.data == &mons[u.ulycn])
			    you_unwere(FALSE);
			u.ulycn = NON_PM;	/* cure lycanthropy */
		    }
		    make_sick(0L, (char *) 0, TRUE, SICK_ALL);

		    /* You feel refreshed */
		    u.uhunger += 50 + rnd(50);
		    newuhs(FALSE);
		} else
		    exercise(A_WIS, FALSE);
		break;
	case POT_WATER:
		if(!otmp->blessed && !otmp->cursed) {
/*JP
		    pline("This tastes like water.");
*/
		    pline("水のような味がする．");
		    u.uhunger += rnd(10);
		    newuhs(FALSE);
		    break;
		}
		unkn++;
		if(is_undead(youmonst.data) || is_demon(youmonst.data) ||
				u.ualign.type == A_CHAOTIC) {
		    if(otmp->blessed) {
/*JP
			pline("This burns like acid!");
*/
			pline("酸のように舌がひりひりする！");
			exercise(A_CON, FALSE);
			if (u.ulycn >= LOW_PM && !Race_if(PM_HUMAN_WEREWOLF)) {
#if 0 /*JP*/
			    Your("affinity to %s disappears!",
				 makeplural(mons[u.ulycn].mname));
#else
			    Your("%sへの親近感はなくなった！",
			    	jtrns_mon(mons[u.ulycn].mname));
#endif
			    if (youmonst.data == &mons[u.ulycn])
				you_unwere(FALSE);
			    u.ulycn = NON_PM;	/* cure lycanthropy */
			}
/*JP
			losehp(d(6,6), "potion of holy water", KILLED_BY_AN);
*/
			losehp(d(6,6), "聖水で", KILLED_BY_AN);
		    } else if(otmp->cursed) {
/*JP
			You_feel("quite proud of yourself.");
*/
			You("自尊心を感じた．");
			healup(d(6,6),0,0,0);
			if (u.ulycn >= LOW_PM && !Upolyd) you_were();
			exercise(A_CON, TRUE);
		    }
		} else {
		    if(otmp->blessed) {
/*JP
			You_feel("full of awe.");
*/
			You("畏怖の念に満たされた．");
			if(u.ualign.type == A_LAWFUL) healup(d(6,6),0,0,0);                        
			make_sick(0L, (char *) 0, TRUE, SICK_ALL);
			exercise(A_WIS, TRUE);
			exercise(A_CON, TRUE);
			if (u.ulycn >= LOW_PM && !Race_if(PM_HUMAN_WEREWOLF)) {
			    you_unwere(TRUE);	/* "Purified" */
			}
			/* make_confused(0L,TRUE); */
		    } else {
			if(u.ualign.type == A_LAWFUL) {
/*JP
			    pline("This burns like acid!");
*/
			    pline("酸のように舌がひりひりする！");
#if 0 /*JP*/
			    losehp(d(6,6), "potion of unholy water",
				KILLED_BY_AN);
#else
			    losehp(d(2,6), "不浄な水で", KILLED_BY_AN);
#endif
			} else
/*JP
			    You_feel("full of dread.");
*/
			    You("恐怖の虜になった．");
			if (u.ulycn >= LOW_PM && !Upolyd) you_were();
			exercise(A_CON, FALSE);
		    }
		}
		break;
	case POT_BOOZE:
		unkn++;
#if 0 /*JP*/
		pline("Ooph!  This tastes like %s%s!",
		      otmp->odiluted ? "watered down " : "",
		      Hallucination ? "dandelion wine" : "liquid fire");
#else
		pline("うぇっぷ！これは%s%sのような味がする！",
		      otmp->odiluted ? "水割りの" : "",
		      Hallucination ? "タンポポワイン" : "燃料オイル");
#endif
		if (!otmp->blessed)
		    make_confused(itimeout_incr(HConfusion, d(3,8)), FALSE);
		/* the whiskey makes us feel better */
		if (!otmp->odiluted) healup(1, 0, FALSE, FALSE);
		u.uhunger += 10 * (2 + bcsign(otmp));
		newuhs(FALSE);
		exercise(A_WIS, FALSE);
		if(otmp->cursed) {
/*JP
			You("pass out.");
*/
			You("気絶した．");
			multi = -rnd(15);
/*JP
			nomovemsg = "You awake with a headache.";
*/
			nomovemsg = "頭痛とともに目覚めた．";
		}
		break;
	case POT_ENLIGHTENMENT:
		if(otmp->cursed) {
			unkn++;
/*JP
			You("have an uneasy feeling...");
*/
			You("不安な気持になった．．．");
			exercise(A_WIS, FALSE);
		} else {
			if (otmp->blessed) {
				(void) adjattrib(A_INT, 1, FALSE);
				(void) adjattrib(A_WIS, 1, FALSE);
			}
/*JP
			You_feel("self-knowledgeable...");
*/
			You("自分自身が判るような気がした．．．");
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(0);
/*JP
			pline_The("feeling subsides.");
*/
			pline("その感じはなくなった．");
			exercise(A_WIS, TRUE);
		}
		break;
	case SPE_INVISIBILITY:
		/* spell cannot penetrate mummy wrapping */
		if (BInvis && uarmc->otyp == MUMMY_WRAPPING) {
/*JP
			You_feel("rather itchy under your %s.", xname(uarmc));
*/
			You("%sの下がムズムズした．", xname(uarmc));
			break;
		}
		/* FALLTHRU */
	case POT_INVISIBILITY:
		if (Invis || Blind || BInvis) {
		    nothing++;
		} else {
		    self_invis_message();
		}
		if (otmp->blessed) HInvis |= FROMOUTSIDE;
		else incr_itimeout(&HInvis, rn1(15,31));
		newsym(u.ux,u.uy);	/* update position */
		if(otmp->cursed) {
/*JP
		    pline("For some reason, you feel your presence is known.");
*/
		    pline("なぜか，存在が知られているような気がした．");
		    aggravate();
		}
		break;
	case POT_SEE_INVISIBLE:
		/* tastes like fruit juice in Rogue */
	case POT_FRUIT_JUICE:
	    {
		int msg = Invisible && !Blind;

		unkn++;
		if (otmp->cursed)
#if 0 /*JP*/
		    pline("Yecch!  This tastes %s.",
			  Hallucination ? "overripe" : "rotten");
#else
		    pline("オェー！これは%s味がする．",
			  Hallucination ? "熟しすぎた" : "腐った");
#endif
		else
#if 0 /*JP*/
		    pline(Hallucination ?
		      "This tastes like 10%% real %s%s all-natural beverage." :
				"This tastes like %s%s.",
			  otmp->odiluted ? "reconstituted " : "",
			  fruitname(TRUE));
#else
		    pline(Hallucination ?
			   "これは%s天然%s果汁10%%の無濃縮還元飲料のような味がする．" :
			   "これは%s%sの味がする．",
			  otmp->odiluted ? "成分調整された" : "", fruitname(FALSE));
#endif
		if (otmp->otyp == POT_FRUIT_JUICE) {
		    u.uhunger += (otmp->odiluted ? 5 : 10) * (2 + bcsign(otmp));
		    newuhs(FALSE);
		    break;
		}
		if (!otmp->cursed) {
			/* Tell them they can see again immediately, which
			 * will help them identify the potion...
			 */
			make_blinded(0L,TRUE);
		}
		if (otmp->blessed)
			HSee_invisible |= FROMOUTSIDE;
		else
			incr_itimeout(&HSee_invisible, rn1(100,750));
		set_mimic_blocking(); /* do special mimic handling */
		see_monsters();	/* see invisible monsters */
		newsym(u.ux,u.uy); /* see yourself! */
		if (msg && !Blind) { /* Blind possible if polymorphed */
/*JP
		    You("can see through yourself, but you are visible!");
*/
		    You("透明である．しかし見えるようになった！");
		    unkn--;
		}
		break;
	    }
	case POT_PARALYSIS:
		if (Free_action)
/*JP
		    You("stiffen momentarily.");
*/
		    You("一瞬動けなくなった．");             
		else {
		    if (Levitation || Is_airlevel(&u.uz)||Is_waterlevel(&u.uz))
/*JP
			You("are motionlessly suspended.");
*/
			You("空中で動けなくなった．");
#ifdef STEED
		    else if (u.usteed)
/*JP
			You("are frozen in place!");
*/
			You("その場で動けなくなった！");
#endif
		    else
#if 0 /*JP*/
			Your("%s are frozen to the %s!",
			     makeplural(body_part(FOOT)), surface(u.ux, u.uy));
#else
			You("動けなくなった！");
#endif
		    nomul(-(rn1(10, 25 - 12*bcsign(otmp))));
		    nomovemsg = You_can_move_again;
		    exercise(A_DEX, FALSE);
		}
		break;
	case POT_SLEEPING:
		if(Sleep_resistance || Free_action)
/*JP
		    You("yawn.");
*/
		    You("あくびをした．");
		else {
/*JP
		    You("suddenly fall asleep!");
*/
		    pline("突然眠ってしまった！");
		    fall_asleep(-rn1(10, 25 - 12*bcsign(otmp)), TRUE);
		}
		break;
	case POT_MONSTER_DETECTION:
	case SPE_DETECT_MONSTERS:
		if (otmp->blessed) {
		    int x, y;

		    if (Detect_monsters) nothing++;
		    unkn++;
		    /* after a while, repeated uses become less effective */
		    if (HDetect_monsters >= 300L)
			i = 1;
		    else
			i = rn1(40,21);
		    incr_itimeout(&HDetect_monsters, i);
		    for (x = 1; x < COLNO; x++) {
			for (y = 0; y < ROWNO; y++) {
			    if (memory_is_invisible(x, y)) {
				unmap_object(x, y);
				newsym(x,y);
			    }
			    if (MON_AT(x,y)) unkn = 0;
			}
		    }
		    see_monsters();
/*JP
		    if (unkn) You_feel("lonely.");
*/
		    if (unkn) You("心細くなった．");
		    break;
		}
		if (monster_detect(otmp, 0))
			return(1);		/* nothing detected */
		exercise(A_WIS, TRUE);
		break;
	case POT_OBJECT_DETECTION:
	case SPE_DETECT_TREASURE:
		if (object_detect(otmp, 0))
			return(1);		/* nothing detected */
		exercise(A_WIS, TRUE);
		break;
	case POT_SICKNESS:
/*JP
		pline("Yecch!  This stuff tastes like poison.");
*/
		pline("ウェー！この薬は毒のような味がする．");
		if (otmp->blessed) {
#if 0 /*JP*/
		    pline("(But in fact it was mildly stale %s.)",
			  fruitname(TRUE));
#else
		    pline("(だが実のところ，それは少し古くなった%sだ．)",
			  fruitname(TRUE));
#endif
		    if (!Role_if(PM_HEALER)) {
			/* NB: blessed otmp->fromsink is not possible */
/*JP
			losehp(1, "mildly contaminated potion", KILLED_BY_AN);
*/
			losehp(1, "軽く汚染された薬で", KILLED_BY_AN);
		    }
		} else {
		    if(Poison_resistance)
#if 0 /*JP*/
			pline(
			  "(But in fact it was biologically contaminated %s.)",
			      fruitname(TRUE));
#else
			pline(
			  "(だが実のところ，それは生物学的に汚染された%sだ．)",
			      fruitname(TRUE));
#endif
		    if (Role_if(PM_HEALER))
/*JP
			pline("Fortunately, you have been immunized.");
*/
			pline("幸運なことに，あなたには免疫がある．");
		    else {
			int typ = rn2(A_MAX);

			if (!Fixed_abil) {
			    poisontell(typ);
			    (void) adjattrib(typ,
			    		Poison_resistance ? -1 : -rn1(4,3),
			    		TRUE);
			}
			if(!Poison_resistance) {
			    if (otmp->fromsink)
				losehp(rnd(10)+5*!!(otmp->cursed),
/*JP
				       "contaminated tap water", KILLED_BY);
*/
				       "汚染された水道水を飲んで", KILLED_BY_AN);
			    else
				losehp(rnd(10)+5*!!(otmp->cursed),
/*JP
				       "contaminated potion", KILLED_BY_AN);
*/
				       "汚染された薬を飲んで", KILLED_BY_AN);
			}
			exercise(A_CON, FALSE);
		    }
		}
		if(Hallucination) {
/*JP
			You("are shocked back to your senses!");
*/
			You("五感に衝撃を受けた！");
			(void) make_hallucinated(0L,FALSE,0L);
		}
		break;
	case POT_CONFUSION:
		if(!Confusion) {
		    if (Hallucination) {
/*JP
			pline("What a trippy feeling!");
*/
			pline("なんかヘロヘロする！");
			unkn++;
		    } else
/*JP
			pline("Huh, What?  Where am I?");
*/
			pline("ほえ？私は誰？");
		} else	nothing++;
		make_confused(itimeout_incr(HConfusion,
					    rn1(7, 16 - 8 * bcsign(otmp))),
			      FALSE);
		break;
	case POT_CLAIRVOYANCE:
		/* KMH -- handle cursed, blessed, blocked */
		if (otmp->cursed)
			nothing++;
		else if (!BClairvoyant) {
/*JP
			if (Hallucination) pline("Dude! See-through walls!");
*/
			if (Hallucination) pline("見てみなよ！壁がシースルーだ！");
			do_vicinity_map();
		}
		if (otmp->blessed)
			incr_itimeout(&HClairvoyant, rn1(50, 100));
		break;
	case POT_ESP:
	{
#if 0 /*JP*/
		const char *mod;
#else
		char mod;
#endif


		/* KMH -- handle cursed, blessed */
		if (otmp->cursed) {
#if 0 /*JP*/
			if (HTelepat) mod = "less ";
			else {
			    unkn++;
			    mod = NULL;
			}
#else
			if (HTelepat) mod = -1;
			else {
			    unkn++;
			    mod = 0;
			}
#endif
			HTelepat = 0;
		} else if (otmp->blessed) {
#if 0 /*JP*/
			mod = "fully ";
#else
			mod = 2;
#endif
			incr_itimeout(&HTelepat, rn1(100, 200));
			HTelepat |= FROMOUTSIDE;
		} else {
#if 0 /*JP*/
			mod = "more ";
#else
			mod = 1;
#endif
			incr_itimeout(&HTelepat, rn1(50, 100));
		}
		if (mod)
#if 0 /*JP*/
			You_feel(Hallucination ?
				"%sin touch with the cosmos." :
				"%smentally acute.", mod);
#else
			if ( Hallucination )
				You_feel("宇宙と%sような気がした．",
					(mod==1)?"接触している":
					(mod==2)?"完全に一体化している":
					"の交信が途絶えた");
			else
				You_feel("精神的な鋭敏さが%sような気がした．",
					(mod==1)?"強まった":
					(mod==2)?"完全になった":
					"消えてしまった");
#endif
		see_monsters();
		break;
	}
	/* KMH, balance patch -- removed
	case POT_FIRE_RESISTANCE:
	       if(!(HFire_resistance & FROMOUTSIDE)) {
		if (Hallucination)
		   pline("You feel, like, totally cool!");
		   else You("feel cooler.");
		   HFire_resistance += rn1(100,50);
		   unkn++;
		   HFire_resistance |= FROMOUTSIDE;
		}
		break;*/
	case POT_INVULNERABILITY:
		incr_itimeout(&Invulnerable, rn1(4, 8 + 4 * bcsign(otmp)));
		You_feel(Hallucination ?
#if 0 /*JP*/
				"like a super-duper hero!" : "invulnerable!");
#else
				"極上のヒーローのようだ！" : "無敵になった気がした！");
#endif
		break;
	case POT_GAIN_ABILITY:
		if(otmp->cursed) {
/*JP
		    pline("Ulch!  That potion tasted foul!");
*/
		    pline("ウェ！悪臭がする！");
		    unkn++;
		} else if (Fixed_abil) {
		    nothing++;
		} else {      /* If blessed, increase all; if not, try up to */
		    int itmp; /* 6 times to find one which can be increased. */
		    i = -1;		/* increment to 0 */
		    for (ii = A_MAX; ii > 0; ii--) {
			i = (otmp->blessed ? i + 1 : rn2(A_MAX));
			/* only give "your X is already as high as it can get"
			   message on last attempt (except blessed potions) */
			itmp = (otmp->blessed || ii == 1) ? 0 : -1;
			if (adjattrib(i, 1, itmp) && !otmp->blessed)
			    break;
		    }
		}
		break;
	case POT_SPEED:
		if(Wounded_legs && !otmp->cursed
#ifdef STEED
		   && !u.usteed	/* heal_legs() would heal steeds legs */
#endif
						) {
			heal_legs();
			unkn++;
			break;
		} /* and fall through */
	case SPE_HASTE_SELF:
		if (!Very_fast)
#if 0 /*JP*/
			You("are suddenly moving %sfaster.",
				Fast ? "" : "much ");
#else
			You("突然%s速く移動できるようになった．",
				Fast ? "" : "とても");
#endif
		else {
/*JP
			Your("%s get new energy.",
*/
			pline("%sにエネルギーが注ぎこまれるような感じがした．",
				makeplural(body_part(LEG)));
			unkn++;
		}
		exercise(A_DEX, TRUE);
		incr_itimeout(&HFast, rn1(10, 100 + 60 * bcsign(otmp)));
		break;
	case POT_BLINDNESS:
		if(Blind) nothing++;
		make_blinded(itimeout_incr(Blinded,
					   rn1(200, 250 - 125 * bcsign(otmp))),
			     (boolean)!Blind);
		break;

	case POT_GAIN_LEVEL:
		if (otmp->cursed) {
			unkn++;
			/* they went up a level */
			if((ledger_no(&u.uz) == 1 && u.uhave.amulet) ||
				Can_rise_up(u.ux, u.uy, &u.uz)) {
/*JP
			    const char *riseup ="rise up, through the %s!";
*/
			    const char *riseup ="宙に浮き，%sを突き抜けた！";
			    /* [ALI] Special handling for quaffing potions
			     * off the floor (otmp won't be valid after
			     * we change levels otherwise).
			     */
			    if (otmp->where == OBJ_FLOOR) {
				if (otmp->quan > 1)
					(void) splitobj(otmp, 1);
				/* Make sure you're charged if in shop */
				otmp->quan++;
				useupf(otmp, 1);
				obj_extract_self(otmp);
			    }
			    if(ledger_no(&u.uz) == 1) {
			        You(riseup, ceiling(u.ux,u.uy));
				goto_level(&earth_level, FALSE, FALSE, FALSE);
			    } else {
			        register int newlev = depth(&u.uz)-1;
				d_level newlevel;

				get_level(&newlevel, newlev);
				if(on_level(&newlevel, &u.uz)) {
/*JP
				    pline("It tasted bad.");
*/
				    pline("とても不味い．");
				    break;
				} else You(riseup, ceiling(u.ux,u.uy));
				goto_level(&newlevel, FALSE, FALSE, FALSE);
			    }
			}
/*JP
			else You("have an uneasy feeling.");
*/
			else You("不安な気持になった．");
			break;
		}
		pluslvl(FALSE);
		if (otmp->blessed)
			/* blessed potions place you at a random spot in the
			 * middle of the new level instead of the low point
			 */
			u.uexp = rndexp(TRUE);
		break;
	case POT_HEALING:
/*JP
		You_feel("better.");
*/
		You("気分がよくなった．");
		healup(d(5,6) + 5 * bcsign(otmp),
		       !otmp->cursed ? 1 : 0, 1+1*!!otmp->blessed, !otmp->cursed);
		exercise(A_CON, TRUE);
		break;
	case POT_EXTRA_HEALING:
/*JP
		You_feel("much better.");
*/
		You("気分がとてもよくなった．");
		healup(d(6,8) + 5 * bcsign(otmp),
		       otmp->blessed ? 5 : !otmp->cursed ? 2 : 0,
		       !otmp->cursed, TRUE);
		(void) make_hallucinated(0L,TRUE,0L);
		exercise(A_CON, TRUE);
		exercise(A_STR, TRUE);
		break;
	case POT_FULL_HEALING:
/*JP
		You_feel("completely healed.");
*/
		You("完全に回復した．");
		healup(400, 4+4*bcsign(otmp), !otmp->cursed, TRUE);
		/* Restore one lost level if blessed */
		if (otmp->blessed && u.ulevel < u.ulevelmax) {
		    /* when multiple levels have been lost, drinking
		       multiple potions will only get half of them back */
		    u.ulevelmax -= 1;
		    pluslvl(FALSE);
		}
		(void) make_hallucinated(0L,TRUE,0L);
		exercise(A_STR, TRUE);
		exercise(A_CON, TRUE);
		break;
	case POT_LEVITATION:
	case SPE_LEVITATION:
		if (otmp->cursed) HLevitation &= ~I_SPECIAL;
		if(!Levitation) {
			/* kludge to ensure proper operation of float_up() */
			HLevitation = 1;
			float_up();
			/* reverse kludge */
			HLevitation = 0;
			if (otmp->cursed && !Is_waterlevel(&u.uz)) {
	if((u.ux != xupstair || u.uy != yupstair)
	   && (u.ux != sstairs.sx || u.uy != sstairs.sy || !sstairs.up)
	   && (!xupladder || u.ux != xupladder || u.uy != yupladder)
	) {
#if 0 /*JP*/
					You("hit your %s on the %s.",
						body_part(HEAD),
						ceiling(u.ux,u.uy));
#else
					You("%sを%sにぶつけた．",
						body_part(HEAD),
						ceiling(u.ux,u.uy));
#endif
#if 0 /*JP*/
					losehp(uarmh ? 1 : rnd(10),
						"colliding with the ceiling",
						KILLED_BY);
#else
					losehp(uarmh ? 1 : rnd(10),
						"天井に衝突して",
						KILLED_BY);
#endif
				} else (void) doup();
			}
		} else
			nothing++;
		if (otmp->blessed) {
		    incr_itimeout(&HLevitation, rn1(50,250));
		    HLevitation |= I_SPECIAL;
		} else incr_itimeout(&HLevitation, rn1(140,10));
		spoteffects(FALSE);	/* for sinks */
		break;
	case POT_GAIN_ENERGY:			/* M. Stephenson */
		{       register int num , num2;
			if(otmp->cursed)
/*JP
			    You_feel("lackluster.");
*/
			    You("意気消沈した．");
			else
/*JP
			    pline("Magical energies course through your body.");
*/
			    pline("魔法のエネルギーがあなたの体に満ちた．");
			num = rnd(25) + 5 * otmp->blessed + 10;                        
			num2 = rnd(2) + 2 * otmp->blessed + 1;
			u.uenmax += (otmp->cursed) ? -num2 : num2;
			u.uen += (otmp->cursed) ? -num : num;
			if(u.uenmax <= 0) u.uenmax = 0;
			if(u.uen <= 0) u.uen = 0;
			if(u.uen > u.uenmax) {
				u.uenmax += ((u.uen - u.uenmax) / 2);
				u.uen = u.uenmax;
			}
			flags.botl = 1;
			exercise(A_WIS, TRUE);
		}
		break;
	case POT_OIL:				/* P. Winner */
		{
			boolean good_for_you = FALSE;

			if (otmp->lamplit) {
			    if (likes_fire(youmonst.data)) {
/*JP
				pline("Ahh, a refreshing drink.");
*/
				pline("あぁ，この一杯は生き返る．");
				good_for_you = TRUE;
			    } else {
/*JP
				You("burn your %s.", body_part(FACE));
*/
				Your("%sは黒焦げになった．", body_part(FACE));
				losehp(d(Fire_resistance ? 1 : 3, 4),
/*JP
				       "burning potion of oil", KILLED_BY_AN);
*/
				       "燃えている油を飲んで", KILLED_BY_AN);
			    }
			} else if(otmp->cursed)
/*JP
			    pline("This tastes like castor oil.");
*/
			    pline("ひまし油のような味がする．");
			else
/*JP
			    pline("That was smooth!");
*/
			    pline("口あたりがよい！");
			exercise(A_WIS, good_for_you);
		}
		break;
	case POT_ACID:
		if (Acid_resistance)
			/* Not necessarily a creature who _likes_ acid */
/*JP
			pline("This tastes %s.", Hallucination ? "tangy" : "sour");
*/
			pline("%s味がする．", Hallucination ? "ぴりっとした" : "酸っぱい");
		else {
#if 0 /*JP*/
			pline("This burns%s!", otmp->blessed ? " a little" :
					otmp->cursed ? " a lot" : " like acid");
			losehp(d(otmp->cursed ? 2 : 1, otmp->blessed ? 4 : 8),
					"potion of acid", KILLED_BY_AN);
#else
			pline("喉が%s！", otmp->blessed ? "少し灼かれた" :
					otmp->cursed ? "灼け爛れた" : "酸に灼かれた");
			losehp(d(otmp->cursed ? 2 : 1, otmp->blessed ? 4 : 8),
					"酸の薬を飲んで", KILLED_BY_AN);
#endif
			exercise(A_CON, FALSE);
		}
		if (Stoned) fix_petrification();
		unkn++; /* holy/unholy water can burn like acid too */
		break;
	case POT_POLYMORPH:
/*JP
		You_feel("a little %s.", Hallucination ? "normal" : "strange");
*/
		You("少し%sな感じがした．", Hallucination ? "ありきたり" : "奇妙");
		if (!Unchanging) polyself(FALSE);
		break;
	case POT_BLOOD:
	case POT_VAMPIRE_BLOOD:
		unkn++;
		u.uconduct.unvegan++;
		if (maybe_polyd(is_vampire(youmonst.data), Race_if(PM_VAMPIRE))) {
		    violated_vegetarian();
		    if (otmp->cursed)
#if 0 /*JP*/
			pline("Yecch!  This %s.", Hallucination ?
			"liquid could do with a good stir" : "blood has congealed");
#else
			pline("ゲェー！こりゃ%s．", Hallucination ?
			"よくかき混ぜないとダメだ" : "血が固まっちまってる");
#endif
		    else pline(Hallucination ?
#if 0 /*JP*/
		      "The %s liquid stirs memories of home." :
		      "The %s blood tastes delicious.",
			  otmp->odiluted ? "watery" : "thick");
#else
		      "%s液体が故郷の記憶を呼び起こす．" :
		      "%s血液は美味しい．",
			  otmp->odiluted ? "水っぽい" : "濃い");
#endif
		    if (!otmp->cursed)
			lesshungry((otmp->odiluted ? 1 : 2) *
			  (otmp->otyp == POT_VAMPIRE_BLOOD ? 400 :
			  otmp->blessed ? 15 : 10));
		    if (otmp->otyp == POT_VAMPIRE_BLOOD && otmp->blessed) {
			int num = newhp();
			if (Upolyd) {
			    u.mhmax += num;
			    u.mh += num;
			} else {
			    u.uhpmax += num;
			    u.uhp += num;
			}
		    }
		} else if (otmp->otyp == POT_VAMPIRE_BLOOD) {
		    /* [CWC] fix conducts for potions of (vampire) blood -
		       doesn't use violated_vegetarian() to prevent
		       duplicated "you feel guilty" messages */
		    u.uconduct.unvegetarian++;
		    if (u.ualign.type == A_LAWFUL || Role_if(PM_MONK)) {
#if 0 /*JP*/
			You_feel("%sguilty about drinking such a vile liquid.",
				Role_if(PM_MONK) ? "especially " : "");
#else
			You("下劣なものを飲んでしまうことによる罪を%s感じた．",
				Role_if(PM_MONK) ? "激しく" : "");
#endif
			u.ugangr++;
			adjalign(-15);
		    } else if (u.ualign.type == A_NEUTRAL)
			adjalign(-3);
		    exercise(A_CON, FALSE);
		    if (!Unchanging && polymon(PM_VAMPIRE))
			u.mtimedone = 0;	/* "Permament" change */
		} else {
		    violated_vegetarian();
#if 0 /*JP*/
		    pline("Ugh.  That was vile.");
#else
		    pline("ウゲェー，こりゃ酷い．");
#endif
		    make_vomiting(Vomiting+d(10,8), TRUE);
		}
		break;
	default:
		impossible("What a funny potion! (%u)", otmp->otyp);
		return(0);
	}
	return(-1);
}

#ifdef	JPEXTENSION
void
make_totter(xtime, talk)
long xtime;	/* nonzero if this is an attempt to turn on hallucination */
boolean talk;
{
	const char *message = 0;

	if (!xtime)
	    message = "方向感覚が正常になった．";
	else
	    message = "方向感覚が麻痺した．";

	set_itimeout(&Totter, xtime);
	pline(message);
}
#endif

void
healup(nhp, nxtra, curesick, cureblind)
	int nhp, nxtra;
	register boolean curesick, cureblind;
{
	if (nhp) {
		if (Upolyd) {
			u.mh += nhp;
			if(u.mh > u.mhmax) u.mh = (u.mhmax += nxtra);
		} else {
			u.uhp += nhp;
			if(u.uhp > u.uhpmax) u.uhp = (u.uhpmax += nxtra);
		}
	}
	if(cureblind)	make_blinded(0L,TRUE);
	if(curesick)	make_sick(0L, (char *) 0, TRUE, SICK_ALL);
	flags.botl = 1;
	return;
}

void
healup_mon(mtmp, nhp, nxtra, curesick, cureblind)
	struct monst *mtmp;
	int nhp, nxtra;
	register boolean curesick, cureblind;
{
	if (nhp) {
		mtmp->mhp += nhp;
		if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = (mtmp->mhpmax += nxtra);
	}
#if 0
	if(cureblind) ; /* NOT DONE YET */
	if(curesick)  ; /* NOT DONE YET */
#endif 
	return;
}

void
strange_feeling(obj,txt)
register struct obj *obj;
register const char *txt;
{
	if (flags.beginner || !txt)
#if 0 /*JP*/
		You("have a %s feeling for a moment, then it passes.",
		Hallucination ? "normal" : "strange");
#else
		You("%s気分におそわれたが，すぐに消え去った．",
		Hallucination ? "ありきたりな" : "奇妙な");
#endif
	else
		pline(txt);

	if(!obj)	/* e.g., crystal ball finds no traps */
		return;

	if(obj->dknown && !objects[obj->otyp].oc_name_known &&
						!objects[obj->otyp].oc_uname)
		docall(obj);
	if (carried(obj)) useup(obj);
	else useupf(obj, 1L);
}

const char *bottlenames[] = {
#if 0 /*JP*/
	"bottle", "phial", "flagon", "carafe", "flask", "jar", "vial"
#else
	"瓶","玻璃瓶","一升瓶","水差し","フラスコ","壷","ガラス瓶"
#endif
};

const char *
bottlename()
{
	return bottlenames[rn2(SIZE(bottlenames))];
}

/* WAC -- monsters can throw potions around too! */
void
potionhit(mon, obj, your_fault)
register struct monst *mon; /* Monster that got hit */
register struct obj *obj;
boolean your_fault;
{
	register const char *botlnam = bottlename();
	boolean isyou = (mon == &youmonst);
	int distance;

	if(isyou) {
		distance = 0;
/*JP
		pline_The("%s crashes on your %s and breaks into shards.",
*/
		pline("%sがあなたの%sの上で壊れ破片となった．",
			botlnam, body_part(HEAD));
/*JP
		losehp(rnd(2), "thrown potion", KILLED_BY_AN);
*/
		losehp(rnd(2), "投げられた薬で", KILLED_BY_AN);
	} else {
		distance = distu(mon->mx,mon->my);
/*JP
		if (!cansee(mon->mx,mon->my)) pline("Crash!");
*/
		if (!cansee(mon->mx,mon->my)) pline("ガシャン！");
		else {
		    char *mnam = mon_nam(mon);
		    char buf[BUFSZ];

		    if(has_head(mon->data)) {
/*JP
			Sprintf(buf, "%s %s",
*/
			Sprintf(buf, "%sの%s",
				s_suffix(mnam),
/*JP
				(notonhead ? "body" : "head"));
*/
				(notonhead ? "身体" : "頭"));
		    } else {
			Strcpy(buf, mnam);
		    }
/*JP
		    pline_The("%s crashes on %s and breaks into shards.",
*/
		    pline("%sが%sの上で壊れ破片となった．",
			   botlnam, buf);
		}
		if(rn2(5) && mon->mhp > 1)
			mon->mhp--;
	}

	/* oil doesn't instantly evaporate */
	if (obj->otyp != POT_OIL && cansee(mon->mx,mon->my))
/*JP
		pline("%s.", Tobjnam(obj, "evaporate"));
*/
		pline("%sは蒸発した．", xname(obj));

    if (isyou) {
	switch (obj->otyp) {
	case POT_OIL:
		if (obj->lamplit)
		    splatter_burning_oil(u.ux, u.uy);
		break;
	case POT_POLYMORPH:
/*JP
		You_feel("a little %s.", Hallucination ? "normal" : "strange");
*/
		You("%sな感じがした．", Hallucination ? "ありきたり" : "奇妙");
		if (!Unchanging && !Antimagic) polyself(FALSE);
		break;
	case POT_ACID:
		if (!Acid_resistance) {
#if 0 /*JP*/
		    pline("This burns%s!", obj->blessed ? " a little" :
				    obj->cursed ? " a lot" : "");
#else
		    pline("%s灼かれる！", obj->blessed ? "少し" :
				    obj->cursed ? "はげしく" : "");
#endif
#if 0 /*JP*/
		    losehp(d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8),
				    "potion of acid", KILLED_BY_AN);
#else
		    losehp(d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8),
				    "酸の薬を浴びて", KILLED_BY_AN);
#endif
		}
		break;
	case POT_AMNESIA:
		/* Uh-oh! */
		if (uarmh && is_helmet(uarmh) && 
			rn2(10 - (uarmh->cursed? 8 : 0)))
		    get_wet(uarmh, TRUE);
		break;
	}
    } else {
	boolean angermon = TRUE;

	if (!your_fault) angermon = FALSE;
	switch (obj->otyp) {
	case POT_HEALING:
 do_healing:
		if (mon->data == &mons[PM_PESTILENCE]) goto do_illness;
		angermon = FALSE;
		if (canseemon(mon))
/*JP
			pline("%s looks better.", Monnam(mon));
*/
			pline("%sは気分がよさそうだ．", Monnam(mon));
		healup_mon(mon, d(5,6) + 5 * bcsign(obj),
			!obj->cursed ? 1 : 0, 1+1*!!obj->blessed, !obj->cursed);
		break;
	case POT_EXTRA_HEALING:
		if (mon->data == &mons[PM_PESTILENCE]) goto do_illness;
		angermon = FALSE;
		if (canseemon(mon))
/*JP
			pline("%s looks much better.", Monnam(mon));
*/
			pline("%sはとても気分がよさそうだ．", Monnam(mon));
		healup_mon(mon, d(6,8) + 5 * bcsign(obj),
			obj->blessed ? 5 : !obj->cursed ? 2 : 0,
			!obj->cursed, TRUE);
		break;
	case POT_FULL_HEALING:
		if (mon->data == &mons[PM_PESTILENCE]) goto do_illness;
		/*FALLTHRU*/
	case POT_RESTORE_ABILITY:
	case POT_GAIN_ABILITY:
		angermon = FALSE;
		    if (canseemon(mon))
/*JP
			pline("%s looks sound and hale again.", Monnam(mon));
*/
			pline("%sは元気になったようだ．", Monnam(mon));
		healup_mon(mon, 400, 5+5*!!(obj->blessed), !(obj->cursed), 1);
		break;
	case POT_SICKNESS:
		if (mon->data == &mons[PM_PESTILENCE]) goto do_healing;
		if (dmgtype(mon->data, AD_DISE) ||
			   dmgtype(mon->data, AD_PEST) || /* won't happen, see prior goto */
			   resists_poison(mon)) {
		    if (canseemon(mon))
#if 0 /*JP*/
			pline("%s looks unharmed.", Monnam(mon));
#else
			pline("%sはなんともないようだ．", Monnam(mon));
#endif
		    break;
		}
 do_illness:
		if((mon->mhpmax > 3) && !resist(mon, POTION_CLASS, 0, NOTELL))
			mon->mhpmax /= 2;
		if((mon->mhp > 2) && !resist(mon, POTION_CLASS, 0, NOTELL))
			mon->mhp /= 2;
		if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
		if (canseemon(mon))
/*JP
		    pline("%s looks rather ill.", Monnam(mon));
*/
		    pline("%sはかなり気分が悪くなったようだ．", Monnam(mon));
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!resist(mon, POTION_CLASS, 0, NOTELL))  mon->mconf = TRUE;
		break;
#if 0 /* NH 3.3.0 version */
	case POT_POLYMORPH:
		(void) bhitm(mon, obj);
		break;
#endif
	case POT_POLYMORPH:
	    /* [Tom] polymorph potion thrown 
	     * [max] added poor victim a chance to resist 
	     * magic resistance protects from polymorph traps, so make
	     * it guard against involuntary polymorph attacks too... 
	     */
	    if (resists_magm(mon)) {
                shieldeff(mon->mx, mon->my);
	    } else if (!resist (mon, POTION_CLASS, 0, NOTELL)) {
/*JP
                mon_poly(mon, your_fault, "%s changes!");
*/
		mon_poly(mon, your_fault, "%sは変化した！");
                if (!Hallucination && canspotmon (mon))
                                makeknown (POT_POLYMORPH);
	    }
  		break;
	case POT_INVISIBILITY:
		angermon = FALSE;
		mon_set_minvis(mon);
		break;
	case POT_SLEEPING:
		/* wakeup() doesn't rouse victims of temporary sleep */
		if (sleep_monst(mon, rnd(12), POTION_CLASS)) {
/*JP
		    pline("%s falls asleep.", Monnam(mon));
*/
		    pline("%sは眠ってしまった．", Monnam(mon));
		    slept_monst(mon);
		}
		break;
	case POT_PARALYSIS:
		if (mon->mcanmove) {
			mon->mcanmove = 0;
			/* really should be rnd(5) for consistency with players
			 * breathing potions, but...
			 */
			mon->mfrozen = rnd(25);
		}
		break;
	case POT_SPEED:
		angermon = FALSE;
		mon_adjust_speed(mon, 1, obj);
		break;
	case POT_BLINDNESS:
		if(haseyes(mon->data)) {
		    register int btmp = 64 + rn2(32) +
			rn2(32) * !resist(mon, POTION_CLASS, 0, NOTELL);
		    btmp += mon->mblinded;
		    mon->mblinded = min(btmp,127);
		    mon->mcansee = 0;
		}
		break;
	case POT_WATER:
		if (is_undead(mon->data) || is_demon(mon->data) ||
			is_were(mon->data)) {
		    if (obj->blessed) {
#if 0 /*JP*/
			pline("%s %s in pain!", Monnam(mon),
			      is_silent(mon->data) ? "writhes" : "shrieks");
#else
			pline("%sは苦痛%s！", Monnam(mon),
			      is_silent(mon->data) ? "に身もだえした" : "の叫び声をあげた");
#endif
			mon->mhp -= d(2,6);
			if (mon->mhp < 1) {
			    if (your_fault)
				killed(mon);
			    else
				monkilled(mon, "", AD_ACID);
			}
			else if (is_were(mon->data) && !is_human(mon->data))
			    new_were(mon);	/* revert to human */
		    } else if (obj->cursed) {
			angermon = FALSE;
			if (canseemon(mon))
/*JP
			    pline("%s looks healthier.", Monnam(mon));
*/
			    pline("%sはより元気になったように見える．", Monnam(mon));
			mon->mhp += d(2,6);
			if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
			if (is_were(mon->data) && is_human(mon->data) &&
				!Protection_from_shape_changers)
			    new_were(mon);	/* transform into beast */
		    }
		} else if(mon->data == &mons[PM_GREMLIN]) {
		    angermon = FALSE;
		    (void)split_mon(mon, (struct monst *)0);
		} else if(mon->data == &mons[PM_FLAMING_SPHERE] ||
			mon->data == &mons[PM_IRON_GOLEM]) {
		    if (canseemon(mon))
#if 0 /*JP*/
			pline("%s %s.", Monnam(mon),
				mon->data == &mons[PM_IRON_GOLEM] ?
				"rusts" : "flickers");
#else
			pline("%sは%s．", Monnam(mon),
				mon->data == &mons[PM_IRON_GOLEM] ?
				"錆びた" : "ちらついた");
#endif
		    mon->mhp -= d(1,6);
		    if (mon->mhp < 1) {
			if (your_fault)
			    killed(mon);
			else
			    monkilled(mon, "", AD_ACID);
		    }
		}
		break;
	case POT_AMNESIA:
		switch (monsndx(mon->data)) {
		case PM_GREMLIN:
		    /* Gremlins multiply... */
		    mon->mtame = FALSE;	
		    (void)split_mon(mon, (struct monst *)0);
		    break;
		case PM_FLAMING_SPHERE:
		case PM_IRON_GOLEM:
#if 0 /*JP*/
		    if (canseemon(mon)) pline("%s %s.", Monnam(mon),
			    monsndx(mon->data) == PM_IRON_GOLEM ?
			    "rusts" : "flickers");
#else
		    if (canseemon(mon)) pline("%sは%s．", Monnam(mon),
			    monsndx(mon->data) == PM_IRON_GOLEM ?
			    "錆びた" : "ちらついた");
#endif
		    mon->mhp -= d(1,6);
		    if (mon->mhp < 1)
			if (your_fault)
			    killed(mon);
			else
			    monkilled(mon, "", AD_ACID);
		    else
			mon->mtame = FALSE;	
		    break;
		case PM_WIZARD_OF_YENDOR:
		    if (your_fault) {
			if (canseemon(mon)) 
#if 0 /*JP*/
			    pline("%s laughs at you!", Monnam(mon));
#else
			    pline("%sはあなたに向かって笑いかけた！", Monnam(mon));
#endif
			forget(1);
		    }
		    break;
		case PM_MEDUSA:
		    if (canseemon(mon))
/*JP
			pline("%s looks like %s's having a bad hair day!", 
*/
			pline("%sは整髪料の質の悪さを気にしているようだ！", 
					Monnam(mon), mhe(mon));
		    break;
		case PM_CROESUS:
		    if (canseemon(mon))
#if 0 /*JP*/
		        pline("%s says: 'My gold! I must count my gold!'", 
#else
		        pline("『ワシの金！ワシの金を数えなければ！』と%sはつぶやいた．", 
#endif
					Monnam(mon));
		    break;
		case PM_DEATH:
 		    if (canseemon(mon))
/*JP
		        pline("%s pauses, then looks at you thoughtfully!", 
*/
		        pline("%sは足を止め，あなたをつくづくと見つめた．！", 
					Monnam(mon));
		    break;
		case PM_FAMINE:
		    if (canseemon(mon))
/*JP
		        pline("%s looks unusually hungry!", Monnam(mon));
*/
		        pline("%sは異常に空腹なように見える！", Monnam(mon));
		    break;
		case PM_PESTILENCE:
		    if (canseemon(mon))
/*JP
		        pline("%s looks unusually well!", Monnam(mon));
*/
		        pline("%sは異常に元気そうに見える！", Monnam(mon));
		    break;
		default:
		    if (mon->data->msound == MS_NEMESIS && canseemon(mon)
				    && your_fault)
/*JP
			pline("%s curses your ancestors!", Monnam(mon));
*/
			pline("%sはあなたの先祖を呪った！", Monnam(mon));
		    else if (mon->isshk) {
			angermon = FALSE;
			if (canseemon(mon))
/*JP
			    pline("%s looks at you curiously!", 
*/
			    pline("%sはあなたを好奇のまなざしで見つめた．", 
					    Monnam(mon));
			make_happy_shk(mon, FALSE);
		    } else if (!is_covetous(mon->data) && !rn2(4) &&
				    !resist(mon, POTION_CLASS, 0, 0)) {
			angermon = FALSE;
			if (canseemon(mon)) {
			    if (mon->msleeping) {
				wakeup(mon);
/*JP
				pline("%s wakes up looking bewildered!", 
*/
				pline("%sは目覚めたが戸惑っているようだ！", 
						Monnam(mon));
			    } else
/*JP
				pline("%s looks bewildered!", Monnam(mon));
*/
				pline("%sは戸惑っているようだ！", Monnam(mon));
			    mon->mpeaceful = TRUE;
			    mon->mtame = FALSE;	
			}
		    }
		    break;
		}
		break;
	case POT_OIL:
		if (obj->lamplit)
			splatter_burning_oil(mon->mx, mon->my);
		break;
/*
	case POT_GAIN_LEVEL:
	case POT_LEVITATION:
	case POT_FRUIT_JUICE:
	case POT_MONSTER_DETECTION:
	case POT_OBJECT_DETECTION:
		break;
*/
	/* KMH, balance patch -- added */
	case POT_ACID:
		if (!resists_acid(mon) && !resist(mon, POTION_CLASS, 0, NOTELL)) {
#if 0 /*JP*/
		    pline("%s %s in pain!", Monnam(mon),
			  is_silent(mon->data) ? "writhes" : "shrieks");
#else
		    pline("%sは苦痛%s！", Monnam(mon),
			  is_silent(mon->data) ? "に身もだえした" : "の叫び声をあげた");
#endif
		    mon->mhp -= d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8);
		    if (mon->mhp < 1) {
			if (your_fault)
			    killed(mon);
			else
			    monkilled(mon, "", AD_ACID);
		    }
		}
		break;
	}
	if (angermon)
	    wakeup(mon);
	else
	    mon->msleeping = 0;
    }

	/* Note: potionbreathe() does its own docall() */
	if ((distance==0 || ((distance < 3) && rn2(5))) &&
	    (!breathless(youmonst.data) || haseyes(youmonst.data)))
		potionbreathe(obj);
	else if (obj->dknown && !objects[obj->otyp].oc_name_known &&
		   !objects[obj->otyp].oc_uname && cansee(mon->mx,mon->my))
		docall(obj);
	if(*u.ushops && obj->unpaid) {
	        register struct monst *shkp =
			shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE));

		if(!shkp)
		    obj->unpaid = 0;
		else {
		    (void)stolen_value(obj, u.ux, u.uy,
				 (boolean)shkp->mpeaceful, FALSE, TRUE);
		    subfrombill(obj, shkp);
		}
	}
	obfree(obj, (struct obj *)0);
}

/* vapors are inhaled or get in your eyes */
void
potionbreathe(obj)
register struct obj *obj;
{
	register int i, ii, isdone, kn = 0;

	switch(obj->otyp) {
	case POT_RESTORE_ABILITY:
	case POT_GAIN_ABILITY:
		if(obj->cursed) {
		    if (!breathless(youmonst.data))
/*JP
			pline("Ulch!  That potion smells terrible!");
*/
			pline("ウゲェ！薬はものすごい臭いがする！");
		    else if (haseyes(youmonst.data)) {
#if 0 /*JP*/
			int numeyes = eyecount(youmonst.data);
			Your("%s sting%s!",
			     (numeyes == 1) ? body_part(EYE) : makeplural(body_part(EYE)),
			     (numeyes == 1) ? "s" : "");
#else
			Your("%sがずきずきする！", body_part(EYE));
#endif
		    }
		    break;
		} else {
		    i = rn2(A_MAX);		/* start at a random point */
		    for(isdone = ii = 0; !isdone && ii < A_MAX; ii++) {
			if(ABASE(i) < AMAX(i)) {
			    ABASE(i)++;
			    /* only first found if not blessed */
			    isdone = !(obj->blessed);
			    flags.botl = 1;
			}
			if(++i >= A_MAX) i = 0;
		    }
		}
		break;
	case POT_FULL_HEALING:
		if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
		if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
		/*FALL THROUGH*/
	case POT_EXTRA_HEALING:
		if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
		if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
		/*FALL THROUGH*/
	case POT_HEALING:
		if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
		if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
		exercise(A_CON, TRUE);
		break;
	case POT_SICKNESS:
		if (!Role_if(PM_HEALER)) {
			if (Upolyd) {
			    if (u.mh <= 5) u.mh = 1; else u.mh -= 5;
			} else {
			    if (u.uhp <= 5) u.uhp = 1; else u.uhp -= 5;
			}
			flags.botl = 1;
			exercise(A_CON, FALSE);
		}
		break;
	case POT_HALLUCINATION:
/*JP
		You("have a momentary vision.");
*/
		You("一瞬幻影につつまれた．");
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!Confusion)
/*JP
			You_feel("somewhat dizzy.");
*/
			You("めまいを感じた．");
		make_confused(itimeout_incr(HConfusion, rnd(5)), FALSE);
		break;
	case POT_INVISIBILITY:
		if (!Blind && !Invis) {
		    kn++;
#if 0 /*JP*/
		    pline("For an instant you %s!",
			See_invisible ? "could see right through yourself"
			: "couldn't see yourself");
#else
		    pline("一瞬自分の身体が%s見えなくなった！",
			See_invisible ? "正しく"
			: "");
#endif
		}
		break;
	case POT_PARALYSIS:
		kn++;
		if (!Free_action) {
/*JP
		    pline("%s seems to be holding you.", Something);
*/
		    pline("%sがあなたをつかまえているような気がした．", Something);
		    nomul(-rnd(5));
		    nomovemsg = You_can_move_again;
		    exercise(A_DEX, FALSE);
/*JP
		} else You("stiffen momentarily.");
*/
		} else You("一瞬硬直した．");
		break;
	case POT_SLEEPING:
		kn++;
		if (!Free_action && !Sleep_resistance) {
/*JP
		    You_feel("rather tired.");
*/
		    You("すこし疲れた．");
		    nomul(-rnd(5));
		    nomovemsg = You_can_move_again;
		    exercise(A_DEX, FALSE);
/*JP
		} else You("yawn.");
*/
		} else You("あくびをした．");
		break;
	case POT_SPEED:
/*JP
		if (!Fast) Your("knees seem more flexible now.");
*/
		if (!Fast) Your("膝はよりすばやく動くようになった．");
		incr_itimeout(&HFast, rnd(5));
		exercise(A_DEX, TRUE);
		break;
	case POT_BLINDNESS:
		if (!Blind && !u.usleep) {
		    kn++;
/*JP
		    pline("It suddenly gets dark.");
*/
		    pline("突然暗くなった．");
		}
		make_blinded(itimeout_incr(Blinded, rnd(5)), FALSE);
		if (!Blind && !u.usleep) Your(vision_clears);
		break;
	case POT_WATER:
		if(u.umonnum == PM_GREMLIN) {
		    (void)split_mon(&youmonst, (struct monst *)0);
		} else if (u.ulycn >= LOW_PM) {
		    /* vapor from [un]holy water will trigger
		       transformation but won't cure lycanthropy */
		    if (obj->blessed && youmonst.data == &mons[u.ulycn])
			you_unwere(FALSE);
		    else if (obj->cursed && !Upolyd)
			you_were();
		}
		break;
	case POT_AMNESIA:
		if(u.umonnum == PM_GREMLIN)
		    (void)split_mon(&youmonst, (struct monst *)0);
		else if(u.umonnum == PM_FLAMING_SPHERE) {
/*JP
		    You("flicker!");
*/
		    You("ちらついた！");
/*JP
		    losehp(d(1,6),"potion of amnesia", KILLED_BY_AN);
*/
		    losehp(d(1,6),"記憶喪失の薬で", KILLED_BY_AN);
		} else if(u.umonnum == PM_IRON_GOLEM) {
/*JP
		    You("rust!");
*/
		    You("錆びた！");
/*JP
		    losehp(d(1,6),"potion of amnesia", KILLED_BY_AN);
*/
		    losehp(d(1,6),"記憶喪失の薬で", KILLED_BY_AN);
		}
/*JP
		You_feel("dizzy!");
*/
		You("めまいを感じた！");
		forget(1 + rn2(5));
		break;
	case POT_ACID:
	case POT_POLYMORPH:
		exercise(A_CON, FALSE);
		break;
	case POT_BLOOD:
	case POT_VAMPIRE_BLOOD:
		if (maybe_polyd(is_vampire(youmonst.data), Race_if(PM_VAMPIRE))) {
		    exercise(A_WIS, FALSE);
/*JP
		    You_feel("a %ssense of loss.",
		      obj->otyp == POT_VAMPIRE_BLOOD ? "terrible " : "");
*/
		    You("%s喪失感におそわれた．",
		      obj->otyp == POT_VAMPIRE_BLOOD ? "恐ろしい" : "");
		} else
		    exercise(A_CON, FALSE);
		break;
/*
	case POT_GAIN_LEVEL:
	case POT_LEVITATION:
	case POT_FRUIT_JUICE:
	case POT_MONSTER_DETECTION:
	case POT_OBJECT_DETECTION:
	case POT_OIL:
		break;
*/
	}
	/* note: no obfree() */
	if (obj->dknown) {
	    if (kn)
		makeknown(obj->otyp);
	    else if (!objects[obj->otyp].oc_name_known &&
						!objects[obj->otyp].oc_uname && !Blind)
		docall(obj);
	}
}

STATIC_OVL short
mixtype(o1, o2)
register struct obj *o1, *o2;
/* returns the potion type when o1 is dipped in o2 */
{
	/* cut down on the number of cases below */
	if (o1->oclass == POTION_CLASS &&
	    (o2->otyp == POT_GAIN_LEVEL ||
	     o2->otyp == POT_GAIN_ENERGY ||
	     o2->otyp == POT_HEALING ||
	     o2->otyp == POT_EXTRA_HEALING ||
	     o2->otyp == POT_FULL_HEALING ||
	     o2->otyp == POT_ENLIGHTENMENT ||
	     o2->otyp == POT_FRUIT_JUICE)) {
		struct obj *swp;

		swp = o1; o1 = o2; o2 = swp;
	}

	switch (o1->otyp) {
		case POT_HEALING:
			switch (o2->otyp) {
			    case POT_SPEED:
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_EXTRA_HEALING;
			}
			break;
		case POT_EXTRA_HEALING:
			switch (o2->otyp) {
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_FULL_HEALING;
			}
			break;
		case POT_FULL_HEALING:
			switch (o2->otyp) {
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_GAIN_ABILITY;
			}
			break;
		case UNICORN_HORN:
			switch (o2->otyp) {
			    case POT_SICKNESS:
				return POT_FRUIT_JUICE;
			    case POT_HALLUCINATION:
			    case POT_BLINDNESS:
			    case POT_CONFUSION:
			    case POT_BLOOD:
			    case POT_VAMPIRE_BLOOD:
				return POT_WATER;
			}
			break;
		case AMETHYST:		/* "a-methyst" == "not intoxicated" */
			if (o2->otyp == POT_BOOZE)
			    return POT_FRUIT_JUICE;
			break;
		case POT_GAIN_LEVEL:
		case POT_GAIN_ENERGY:
			switch (o2->otyp) {
			    case POT_CONFUSION:
				return (rn2(3) ? POT_BOOZE : POT_ENLIGHTENMENT);
			    case POT_HEALING:
				return POT_EXTRA_HEALING;
			    case POT_EXTRA_HEALING:
				return POT_FULL_HEALING;
			    case POT_FULL_HEALING:
				return POT_GAIN_ABILITY;
			    case POT_FRUIT_JUICE:
				return POT_SEE_INVISIBLE;
			    case POT_BOOZE:
				return POT_HALLUCINATION;
			}
			break;
		case POT_FRUIT_JUICE:
			switch (o2->otyp) {
			    case POT_SICKNESS:
				return POT_SICKNESS;
			    case POT_BLOOD:
				return POT_BLOOD;
			    case POT_VAMPIRE_BLOOD:
				return POT_VAMPIRE_BLOOD;
			    case POT_SPEED:
				return POT_BOOZE;
			    case POT_GAIN_LEVEL:
			    case POT_GAIN_ENERGY:
				return POT_SEE_INVISIBLE;
			}
			break;
		case POT_ENLIGHTENMENT:
			switch (o2->otyp) {
			    case POT_LEVITATION:
				if (rn2(3)) return POT_GAIN_LEVEL;
				break;
			    case POT_FRUIT_JUICE:
				return POT_BOOZE;
			    case POT_BOOZE:
				return POT_CONFUSION;
			}
			break;
	}
	/* MRKR: Extra alchemical effects. */

	if (o2->otyp == POT_ACID && o1->oclass == GEM_CLASS) {
	  const char *potion_descr;

	  /* Note: you can't create smoky, milky or clear potions */

	  switch (o1->otyp) {

	    /* white */

	  case DILITHIUM_CRYSTAL:
	    /* explodes - special treatment in dodip */
	    /* here we just want to return something non-zero */
	    return POT_WATER;
	    break;
	  case DIAMOND:
	    /* won't dissolve */
	    potion_descr = NULL;
	    break;
	  case OPAL:
	    potion_descr = "cloudy";
	    break;

	    /* red */

	  case RUBY:
	    potion_descr = "ruby";
	    break;
	  case GARNET:
	    potion_descr = "pink";
	    break;
	  case JASPER:
	    potion_descr = "purple-red";
	    break;

	    /* orange */

	  case JACINTH:
	    potion_descr = "orange";
	    break;
	  case AGATE:
	    potion_descr = "swirly";
	    break;

	    /* yellow */

	  case CITRINE:
	    potion_descr = "yellow";
	    break;
	  case CHRYSOBERYL:
	    potion_descr = "golden";
	    break;

	    /* yellowish brown */

	  case AMBER:
	    potion_descr = "brown";
	    break;
	  case TOPAZ:
	    potion_descr = "murky";
	    break;

	    /* green */

	  case EMERALD:
	    potion_descr = "emerald";
	    break;
	  case TURQUOISE:
	    potion_descr = "sky blue";
	    break;
	  case AQUAMARINE:
	    potion_descr = "cyan";
	    break;
	  case JADE:
	    potion_descr = "dark green";
	    break;

	    /* blue */

	  case SAPPHIRE:
	    potion_descr = "brilliant blue";
	    break;

	    /* violet */

	  case AMETHYST:
	    potion_descr = "magenta";
	    break;
	  case FLUORITE:
	    potion_descr = "white";
	    break;

	    /* black */

	  case BLACK_OPAL:
	    potion_descr = "black";
	    break;
	  case JET:
	    potion_descr = "dark";
	    break;
	  case OBSIDIAN:
	    potion_descr = "effervescent";
	    break;
	  default: potion_descr = NULL;
	  }

	  if (potion_descr) {
	    int typ;

	    /* find a potion that matches the description */

	    for (typ = bases[POTION_CLASS];
		 objects[typ].oc_class == POTION_CLASS;
		 typ++) {

	      if (strcmp(potion_descr, OBJ_DESCR(objects[typ])) == 0) {
		return typ;
	      }
	    }
	  }
	}

	return 0;
}

/* Bills an object that's about to be downgraded, assuming that's not already
 * been done */
STATIC_OVL
void
pre_downgrade_obj(obj, used)
register struct obj *obj;
boolean *used;
{
    boolean dummy = FALSE;

    if (!used) used = &dummy;
#if 0 /*JP*/
    if (!*used) Your("%s for a moment.", aobjnam(obj, "sparkle"));
#else
    if (!*used) Your("%sは一瞬泡立った．", xname(obj));
#endif
    if(obj->unpaid && costly_spot(u.ux, u.uy) && !*used) {
#if 0 /*JP*/
	You("damage it, you pay for it.");
#else
	You("それの質を落してしまったため，賠償せねばならない．");
#endif
	bill_dummy_object(obj);
    }
    *used = TRUE;
}

/* Implements the downgrading effect of potions of amnesia and Lethe water */
STATIC_OVL
void
downgrade_obj(obj, nomagic, used)
register struct obj *obj;
int nomagic;	/* The non-magical object to downgrade to */
boolean *used;
{
    pre_downgrade_obj(obj, used);
    obj->otyp = nomagic;
    obj->spe = 0;
    obj->owt = weight(obj);
    flags.botl = TRUE;
}

boolean
get_wet(obj, amnesia)
register struct obj *obj;
boolean amnesia;
/* returns TRUE if something happened (potion should be used up) */
{
	char Your_buf[BUFSZ];
	boolean used = FALSE;

	if (snuff_lit(obj)) return(TRUE);

	if (obj->greased) {
		grease_protect(obj,(char *)0,&youmonst);
		return(FALSE);
	}
	(void) Shk_Your(Your_buf, obj);
	/* (Rusting shop goods ought to be charged for.) */
	switch (obj->oclass) {
	    case POTION_CLASS:
		if (obj->otyp == POT_WATER) {
		    if (amnesia) {
#if 0 /*JP*/
			Your("%s to sparkle.", aobjnam(obj,"start"));
#else
			Your("%sは泡立ちはじめた．", xname(obj));
#endif
			obj->odiluted 	= 0;
			obj->otyp 	= POT_AMNESIA;
			used 		= TRUE;
			break;
		    }
		    return FALSE;
		}

		/* Diluting a !ofAmnesia just gives water... */
		if (obj->otyp == POT_AMNESIA) {
#if 0 /*JP*/
			Your("%s flat.", aobjnam(obj, "become"));
#else
			Your("%sは気が抜けてきた．", xname(obj));
#endif
			obj->odiluted = 0;
			obj->otyp = POT_WATER;
			used = TRUE;
			break;
		}

		/* KMH -- Water into acid causes an explosion */
		if (obj->otyp == POT_ACID) {
/*JP
			pline("It boils vigorously!");
*/
			pline("激しく沸騰した！");
/*JP
			You("are caught in the explosion!");
*/
			You("爆発に巻き込まれた！");
/*JP
			losehp(Acid_resistance ? rnd(5) : rnd(10),
			       "elementary chemistry", KILLED_BY);
*/
			losehp(rnd(10), "初等化学実験で", KILLED_BY);
			if (amnesia) {
/*JP
			    You_feel("a momentary lapse of reason!");
*/
			    You("一瞬，爆発の原因を失念してしまった！");
			    forget(2 + rn2(3));
			}
			makeknown(obj->otyp);
			used = TRUE;
			break;
		}
		if (amnesia)
#if 0 /*JP*/
		    pline("%s %s completely.", Your_buf, aobjnam(obj,"dilute"));
#else
		    pline("%s%sは完全に薄まった．", Your_buf, xname(obj));
#endif
		else
#if 0 /*JP*/
		    pline("%s %s%s.", Your_buf, aobjnam(obj,"dilute"),
		      		obj->odiluted ? " further" : "");
#else
		    pline("%s%sは%s薄まった．", Your_buf, xname(obj),
			  obj->odiluted ? "さらに" : "");
#endif
		if(obj->unpaid && costly_spot(u.ux, u.uy)) {
/*JP
		    You("dilute it, you pay for it.");
*/
		    You("薄めてしまった，賠償せねばならない．");
		    bill_dummy_object(obj);
		}
		if (obj->odiluted || amnesia) {
			obj->odiluted = 0;
#ifdef UNIXPC
			obj->blessed = FALSE;
			obj->cursed = FALSE;
#else
			obj->blessed = obj->cursed = FALSE;
#endif
			obj->otyp = POT_WATER;
		} else obj->odiluted++;
		used = TRUE;
		break;
	    case SCROLL_CLASS:
		if (obj->otyp != SCR_BLANK_PAPER
#ifdef MAIL
		    && obj->otyp != SCR_MAIL
#endif
		    ) {
			if (!Blind) {
#if 0 /*JP*/
				boolean oq1 = obj->quan == 1L;
				pline_The("scroll%s %s.",
					  oq1 ? "" : "s", otense(obj, "fade"));
#else
				pline("巻物の文字は薄れた．");
#endif
			}
			if(obj->unpaid && costly_spot(u.ux, u.uy)) {
/*JP
			    You("erase it, you pay for it.");
*/
			    You("文字を消してしまった，賠償せねばならない．");
			    bill_dummy_object(obj);
			}
			obj->otyp = SCR_BLANK_PAPER;
			obj->spe = 0;
			used = TRUE;
		} 
		break;
	    case SPBOOK_CLASS:
		if (obj->otyp != SPE_BLANK_PAPER) {
			if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
/*JP
	pline("%s suddenly heats up; steam rises and it remains dry.",
*/
	pline("%sは突然熱くなり，蒸気がたちこめ，乾いてしまった．",
				The(xname(obj)));
			} else {
			    if (!Blind) {
#if 0 /*JP*/
				    boolean oq1 = obj->quan == 1L;
				    pline_The("spellbook%s %s.",
					oq1 ? "" : "s", otense(obj, "fade"));
#else
				    pline("魔法書の文字は薄れた．");
#endif
			    }
			    if(obj->unpaid) {
				subfrombill(obj, shop_keeper(*u.ushops));
/*JP
			        You("erase it, you pay for it.");
*/
			        You("文字を消してしまった，賠償せねばならない．");
			        bill_dummy_object(obj);
			    }
			    obj->otyp = SPE_BLANK_PAPER;
			}
			used = TRUE;
		}
		break;
	    case GEM_CLASS:
		if (amnesia && (obj->otyp == LUCKSTONE ||
			obj->otyp == LOADSTONE || obj->otyp == HEALTHSTONE ||
			obj->otyp == TOUCHSTONE))
		    downgrade_obj(obj, FLINT, &used);
		break;
	    case TOOL_CLASS:
		/* Artifacts aren't downgraded by amnesia */
		if (amnesia && !obj->oartifact) {
		    switch (obj->otyp) {
			case MAGIC_LAMP:
			    /* Magic lamps forget their djinn... */
			    downgrade_obj(obj, OIL_LAMP, &used);
			    break;
			case MAGIC_CANDLE:
			    downgrade_obj(obj, 
					    rn2(2)? WAX_CANDLE : TALLOW_CANDLE,
					    &used);
			    break;
			case DRUM_OF_EARTHQUAKE:
			    downgrade_obj(obj, LEATHER_DRUM, &used);
			    break;
			case MAGIC_WHISTLE:
			    /* Magic whistles lose their powers... */
			    downgrade_obj(obj, TIN_WHISTLE, &used);
			    break;
			case MAGIC_FLUTE:
			    /* Magic flutes sound normal again... */
			    downgrade_obj(obj, WOODEN_FLUTE, &used);
			    break;
			case MAGIC_HARP:
			    /* Magic harps sound normal again... */
			    downgrade_obj(obj, WOODEN_HARP, &used);
			    break;
			case FIRE_HORN:
			case FROST_HORN:
			case HORN_OF_PLENTY:
			    downgrade_obj(obj, TOOLED_HORN, &used);
			    break;
			case MAGIC_MARKER:
			    /* Magic markers run... */
			    if (obj->spe > 0) {
				pre_downgrade_obj(obj, &used);
				if ((obj->spe -= (3 + rn2(10))) < 0) 
				    obj->spe = 0;
			    }
			    break;
		    }
		}

		/* The only other tools that can be affected are pick axes and 
		 * unicorn horns... */
		if (!is_weptool(obj)) break;
		/* Drop through for disenchantment and rusting... */
		/* fall through */
	    case ARMOR_CLASS:
	    case WEAPON_CLASS:
	    case WAND_CLASS:
	    case RING_CLASS:
	    /* Just "fall through" to generic rustprone check for now. */
	    /* fall through */
	    default:
		switch(artifact_wet(obj, FALSE)) {
		    case -1: break;
		    default:
			return TRUE;
		}
		/* !ofAmnesia acts as a disenchanter... */
		if (amnesia && obj->spe > 0) {
		    pre_downgrade_obj(obj, &used);
		    drain_item(obj);
		}
		if (!obj->oerodeproof && is_rustprone(obj) &&
		    (obj->oeroded < MAX_ERODE) && !rn2(2)) {
#if 0 /*JP*/
			pline("%s %s some%s.",
			      Your_buf, aobjnam(obj, "rust"),
			      obj->oeroded ? " more" : "what");
#else
			pline("%s%sは%s錆びた．", Your_buf, xname(obj),
			      obj->oeroded ? "さらに" : "");
#endif
			obj->oeroded++;
			if(obj->unpaid && costly_spot(u.ux, u.uy) && !used) {
/*JP
			    You("damage it, you pay for it.");
*/
			    You("価値を下げてしまった，賠償せねばならない．");
			    bill_dummy_object(obj);
			}
			used = TRUE;
		} 
		break;
	}
	/* !ofAmnesia might strip away fooproofing... */
	if (amnesia && obj->oerodeproof && !rn2(13)) {
	    pre_downgrade_obj(obj, &used);
	    obj->oerodeproof = FALSE;
	}

	/* !ofAmnesia also strips blessed/cursed status... */

	if (amnesia && (obj->cursed || obj->blessed)) {
	    /* Blessed objects are valuable, cursed objects aren't, unless
	     * they're water.
	     */
	    if (obj->blessed || obj->otyp == POT_WATER)
		pre_downgrade_obj(obj, &used);
	    else if (!used) {
/*JP
		Your("%s for a moment.", aobjnam(obj, "sparkle"));
*/
		Your("%sは一瞬泡立った．", xname(obj));
		used = TRUE;
	    }
	    uncurse(obj);
	    unbless(obj);
	}

	if (used) 
	    update_inventory();
	else 
/*JP
	    pline("%s %s wet.", Your_buf, aobjnam(obj,"get"));
*/
	pline("%s%sは濡れた．", Your_buf, xname(obj));

	return used;
}


/* KMH, balance patch -- idea by Dylan O'Donnell <dylanw@demon.net>
 * The poor hacker's polypile.  This includes weapons, armor, and tools.
 * To maintain balance, magical categories (amulets, scrolls, spellbooks,
 * potions, rings, and wands) should NOT be supported.
 * Polearms are not currently implemented.
 */
int
upgrade_obj(obj)
register struct obj *obj;
/* returns 1 if something happened (potion should be used up) 
 * returns 0 if nothing happened
 * returns -1 if object exploded (potion should be used up) 
 */
{
	int chg, otyp = obj->otyp, otyp2;
	xchar ox, oy;
	long owornmask;
	struct obj *otmp;
	boolean explodes;
	char buf[BUFSZ];

	/* Check to see if object is valid */
	if (!obj)
		return 0;
	(void)snuff_lit(obj);
	if (obj->oartifact)
		/* WAC -- Could have some funky fx */
		return 0;

	switch (obj->otyp)
	{
		/* weapons */
		case ORCISH_DAGGER:
			obj->otyp = DAGGER;
			break;
		case GREAT_DAGGER:
		case DAGGER:
			if (!rn2(2)) obj->otyp = ELVEN_DAGGER;
			else obj->otyp = DARK_ELVEN_DAGGER;
			break;
		case ELVEN_DAGGER:
		case DARK_ELVEN_DAGGER:
			obj->otyp = GREAT_DAGGER;
			break;
		case KNIFE:
			obj->otyp = STILETTO;
			break;
		case STILETTO:
			obj->otyp = KNIFE;
			break;
		case AXE:
			obj->otyp = BATTLE_AXE;
			break;
		case BATTLE_AXE:
			obj->otyp = AXE;
			break;
		case PICK_AXE:
			obj->otyp = DWARVISH_MATTOCK;
			break;
		case DWARVISH_MATTOCK:
			obj->otyp = PICK_AXE;
			break;
		case ORCISH_SHORT_SWORD:
			obj->otyp = SHORT_SWORD;
			break;
		case ELVEN_SHORT_SWORD:
		case DARK_ELVEN_SHORT_SWORD:
		case SHORT_SWORD:
			obj->otyp = DWARVISH_SHORT_SWORD;
			break;
		case DWARVISH_SHORT_SWORD:
			if (!rn2(2)) obj->otyp = ELVEN_SHORT_SWORD;
			else obj->otyp = DARK_ELVEN_SHORT_SWORD;
			break;
		case BROADSWORD:
			obj->otyp = ELVEN_BROADSWORD;
			break;
		case ELVEN_BROADSWORD:
			obj->otyp = BROADSWORD;
			break;
		case CLUB:
			obj->otyp = AKLYS;
			break;
		case AKLYS:
			obj->otyp = CLUB;
			break;
		case WAR_HAMMER:
			obj->otyp = HEAVY_HAMMER;
			break;
		case HEAVY_HAMMER:
			obj->otyp = WAR_HAMMER;
			break;
		case ELVEN_BOW:
		case DARK_ELVEN_BOW:
		case YUMI:
		case ORCISH_BOW:
			obj->otyp = BOW;
			break;
		case BOW:
			switch (rn2(3)) {
				case 0: obj->otyp = ELVEN_BOW; break;
				case 1: obj->otyp = DARK_ELVEN_BOW; break;
				case 2: obj->otyp = YUMI; break;
			}
			break;
		case ELVEN_ARROW:
		case DARK_ELVEN_ARROW:
		case YA:
		case ORCISH_ARROW:
			obj->otyp = ARROW;
			break;
		case ARROW:
			switch (rn2(3)) {
				case 0: obj->otyp = ELVEN_ARROW; break;
				case 1: obj->otyp = DARK_ELVEN_ARROW; break;
				case 2: obj->otyp = YA; break;
			}
			break;
		/* armour */
		case ELVEN_MITHRIL_COAT:
			obj->otyp = DARK_ELVEN_MITHRIL_COAT;
			break;
		case DARK_ELVEN_MITHRIL_COAT:
			obj->otyp = ELVEN_MITHRIL_COAT;
			break;
		case ORCISH_CHAIN_MAIL:
			obj->otyp = CHAIN_MAIL;
			break;
		case CHAIN_MAIL:
			obj->otyp = ORCISH_CHAIN_MAIL;
			break;
		case STUDDED_LEATHER_ARMOR:
		case LEATHER_JACKET:
			obj->otyp = LEATHER_ARMOR;
			break;
		case LEATHER_ARMOR:
			obj->otyp = STUDDED_LEATHER_ARMOR;
			break;
		/* robes */
		case ROBE:
			if (!rn2(2)) obj->otyp = ROBE_OF_PROTECTION;
			else obj->otyp = ROBE_OF_POWER;
			break;
		case ROBE_OF_PROTECTION:
		case ROBE_OF_POWER:
			obj->otyp = ROBE;
			break;
		/* cloaks */
		case CLOAK_OF_PROTECTION:
		case CLOAK_OF_INVISIBILITY:
		case CLOAK_OF_MAGIC_RESISTANCE:
		case CLOAK_OF_DISPLACEMENT:
		case DWARVISH_CLOAK:
		case ORCISH_CLOAK:
			if (!rn2(2)) obj->otyp = OILSKIN_CLOAK;
			else obj->otyp = ELVEN_CLOAK;
			break;
		case OILSKIN_CLOAK:
		case ELVEN_CLOAK:
			switch (rn2(4)) {
				case 0: obj->otyp = CLOAK_OF_PROTECTION; break;
				case 1: obj->otyp = CLOAK_OF_INVISIBILITY; break;
				case 2: obj->otyp = CLOAK_OF_MAGIC_RESISTANCE; break;
				case 3: obj->otyp = CLOAK_OF_DISPLACEMENT; break;
			}
			break;
		/* helms */
		case FEDORA:
			obj->otyp = ELVEN_LEATHER_HELM;
			break;
		case ELVEN_LEATHER_HELM:
			obj->otyp = FEDORA;
			break;
		case DENTED_POT:
			obj->otyp = ORCISH_HELM;
			break;
		case ORCISH_HELM:
		case HELM_OF_BRILLIANCE:
		case HELM_OF_TELEPATHY:
			obj->otyp = DWARVISH_IRON_HELM;
			break;
		case DWARVISH_IRON_HELM:
			if (!rn2(2)) obj->otyp = HELM_OF_BRILLIANCE;
			else obj->otyp = HELM_OF_TELEPATHY;
			break;
		case CORNUTHAUM:
			obj->otyp = DUNCE_CAP;
			break;
		case DUNCE_CAP:
			obj->otyp = CORNUTHAUM;
			break;
		/* gloves */
		case LEATHER_GLOVES:
			if (!rn2(2)) obj->otyp = GAUNTLETS_OF_SWIMMING;
			else obj->otyp = GAUNTLETS_OF_DEXTERITY;
			break;
		case GAUNTLETS_OF_SWIMMING:
		case GAUNTLETS_OF_DEXTERITY:
			obj->otyp = LEATHER_GLOVES;
			break;
		/* shields */
		case ELVEN_SHIELD:
			if (!rn2(2)) obj->otyp = URUK_HAI_SHIELD;
			else obj->otyp = ORCISH_SHIELD;
			break;
		case URUK_HAI_SHIELD:
		case ORCISH_SHIELD:
			obj->otyp = ELVEN_SHIELD;
			break;
		case DWARVISH_ROUNDSHIELD:
			obj->otyp = LARGE_SHIELD;
			break;
		case LARGE_SHIELD:
			obj->otyp = DWARVISH_ROUNDSHIELD;
			break;
		/* boots */
		case LOW_BOOTS:
			obj->otyp = HIGH_BOOTS;
			break;
		case HIGH_BOOTS:
			obj->otyp = LOW_BOOTS;
			break;
		/* NOTE:  Supposedly,  HIGH_BOOTS should upgrade to any of the
			other magic leather boots (except for fumble).  IRON_SHOES
			should upgrade to the iron magic boots,  unless
			the iron magic boots are fumble */
		/* rings,  amulets */
		case LARGE_BOX:
		case ICE_BOX:
			obj->otyp = CHEST;
			break;
		case CHEST:
			obj->otyp = ICE_BOX;
			break;
		case SACK:
			obj->otyp = rn2(5) ? OILSKIN_SACK : BAG_OF_HOLDING;
			break;
		case OILSKIN_SACK:
			obj->otyp = BAG_OF_HOLDING;
			break;
		case BAG_OF_HOLDING:
			obj->otyp = OILSKIN_SACK;
			break;
#ifdef TOURIST
		case TOWEL:
			obj->otyp = BLINDFOLD;
			break;
		case BLINDFOLD:
			obj->otyp = TOWEL;
			break;
		case CREDIT_CARD:
#endif
		case LOCK_PICK:
			obj->otyp = SKELETON_KEY;
			break;
		case SKELETON_KEY:
			obj->otyp = LOCK_PICK;
			break;
		case TALLOW_CANDLE:
			obj->otyp = WAX_CANDLE;
			break;
		case WAX_CANDLE:
			obj->otyp = TALLOW_CANDLE;
			break;
		case OIL_LAMP:
			obj->otyp = BRASS_LANTERN;
			break;
		case BRASS_LANTERN:
			obj->otyp = OIL_LAMP;
			break;
		case TIN_WHISTLE:
			obj->otyp = MAGIC_WHISTLE;
			break;
		case MAGIC_WHISTLE:
			obj->otyp = TIN_WHISTLE;
			break;
		case WOODEN_FLUTE:
			obj->otyp = MAGIC_FLUTE;
			obj->spe = rn1(5,10);
			break;
		case MAGIC_FLUTE:
			obj->otyp = WOODEN_FLUTE;
			break;
		case TOOLED_HORN:
			obj->otyp = rn1(HORN_OF_PLENTY - TOOLED_HORN, FROST_HORN);
			obj->spe = rn1(5,10);
			obj->known = 0;
			break;
		case HORN_OF_PLENTY:
		case FIRE_HORN:
		case FROST_HORN:
			obj->otyp = TOOLED_HORN;
			break;
		case WOODEN_HARP:
			obj->otyp = MAGIC_HARP;
			obj->spe = rn1(5,10);
			obj->known = 0;
			break;
		case MAGIC_HARP:
			obj->otyp = WOODEN_HARP;
			break;
#ifdef STEED
		case LEASH:
			obj->otyp = SADDLE;
			break;
		case SADDLE:
			obj->otyp = LEASH;
			break;
#endif
		case TIN_OPENER:
			obj->otyp = TINNING_KIT;
			obj->spe = rn1(30,70);
			obj->known = 0;
			break;
		case TINNING_KIT:
			obj->otyp = TIN_OPENER;
			break;
		case CRYSTAL_BALL:
			/* "ball-point pen" */
			obj->otyp = MAGIC_MARKER;
			/* Keep the charges (crystal ball usually less than marker) */
			break;
		case MAGIC_MARKER:
			obj->otyp = CRYSTAL_BALL;
			chg = rn1(10,3);
			if (obj->spe > chg)
				obj->spe = chg;
			obj->known = 0;
			break;
		case K_RATION:
		case C_RATION:
		case LEMBAS_WAFER:
			if (!rn2(2)) obj->otyp = CRAM_RATION;
			else obj->otyp = FOOD_RATION;
			break;
		case FOOD_RATION:
		case CRAM_RATION:
			obj->otyp = LEMBAS_WAFER;
			break;
		case LOADSTONE:
			obj->otyp = FLINT;
			break;
		case FLINT:
			if (!rn2(2)) obj->otyp = LUCKSTONE;
			else obj->otyp = HEALTHSTONE;
			break;
		default:
			/* This object is not upgradable */
			return 0;
	}

	if (artifact_name(ONAME(obj), &otyp2) && otyp2 == obj->otyp) {
	    int n;
	    char c1, c2;

	    Strcpy(buf, ONAME(obj));
	    n = rn2((int)strlen(buf));
	    c1 = lowc(buf[n]);
	    do c2 = 'a' + rn2('z'-'a'); while (c1 == c2);
	    buf[n] = (buf[n] == c1) ? c2 : highc(c2);  /* keep same case */
	    if (oname(obj, buf) != obj)
		panic("upgrade_obj: unhandled realloc");
	}

	if ((!carried(obj) || obj->unpaid) &&
#ifdef UNPOLYPILE
		!is_hazy(obj) &&
#endif
		get_obj_location(obj, &ox, &oy, BURIED_TOO|CONTAINED_TOO) &&
		costly_spot(ox, oy)) {
	    char objroom = *in_rooms(ox, oy, SHOPBASE);
	    register struct monst *shkp = shop_keeper(objroom);

	    if ((!obj->no_charge ||
		 (Has_contents(obj) &&
		    (contained_cost(obj, shkp, 0L, FALSE, FALSE) != 0L)))
	       && inhishop(shkp)) {
		if(shkp->mpeaceful) {
		    if(*u.ushops && *in_rooms(u.ux, u.uy, 0) ==
			    *in_rooms(shkp->mx, shkp->my, 0) &&
			    !costly_spot(u.ux, u.uy))
			make_angry_shk(shkp, ox, oy);
		    else {
/*JP
			pline("%s gets angry!", Monnam(shkp));
*/
			pline("%sは激怒した！", Monnam(shkp));
			hot_pursuit(shkp);
		    }
/*JP
		} else Norep("%s is furious!", Monnam(shkp));
*/
		} else Norep("%sは怒った！", Monnam(shkp));
		otyp2 = obj->otyp;
		obj->otyp = otyp;
		/*
		 * [ALI] When unpaid containers are upgraded, the
		 * old container is billed as a dummy object, but
		 * it's contents are unaffected and will remain
		 * either unpaid or not as appropriate.
		 */
		otmp = obj->cobj;
		obj->cobj = NULL;
		if (costly_spot(u.ux, u.uy) && objroom == *u.ushops)
		    bill_dummy_object(obj);
		else
		    (void) stolen_value(obj, ox, oy, FALSE, FALSE, FALSE);
		obj->otyp = otyp2;
		obj->cobj = otmp;
	    }
	}

	/* The object was transformed */
	obj->owt = weight(obj);
	obj->oclass = objects[obj->otyp].oc_class;
	if (!objects[obj->otyp].oc_uses_known)
	    obj->known = 1;

	if (carried(obj)) {
	    if (obj == uskin) rehumanize();
	    /* Quietly remove worn item if no longer compatible --ALI */
	    owornmask = obj->owornmask;
	    if (owornmask & W_ARM && !is_suit(obj))
		owornmask &= ~W_ARM;
	    if (owornmask & W_ARMC && !is_cloak(obj))
		owornmask &= ~W_ARMC;
	    if (owornmask & W_ARMH && !is_helmet(obj))
		owornmask &= ~W_ARMH;
	    if (owornmask & W_ARMS && !is_shield(obj))
		owornmask &= ~W_ARMS;
	    if (owornmask & W_ARMG && !is_gloves(obj))
		owornmask &= ~W_ARMG;
	    if (owornmask & W_ARMF && !is_boots(obj))
		owornmask &= ~W_ARMF;
#ifdef TOURIST
	    if (owornmask & W_ARMU && !is_shirt(obj))
		owornmask &= ~W_ARMU;
#endif
	    if (owornmask & W_TOOL && obj->otyp != BLINDFOLD &&
	      obj->otyp != TOWEL && obj->otyp != LENSES)
		owornmask &= ~W_TOOL;
	    otyp2 = obj->otyp;
	    obj->otyp = otyp;
	    if (obj->otyp == LEASH && obj->leashmon) o_unleash(obj);
	    remove_worn_item(obj, TRUE);
	    obj->otyp = otyp2;
	    obj->owornmask = owornmask;
	    setworn(obj, obj->owornmask);
	    puton_worn_item(obj);
	}

	if (obj->otyp == BAG_OF_HOLDING && Has_contents(obj)) {
	    explodes = FALSE;

	    for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
		if (mbag_explodes(otmp, 0)) { 
		    explodes = TRUE;
		    break;
		}

            if (explodes) {
#if 0 /*JP*/
		pline("As you upgrade your bag, you are blasted by a magical explosion!");
#else
		pline("鞄をアップグレードしたとたん，魔法の爆発を浴びた！");
#endif
		delete_contents(obj);
		if (carried(obj))
		    useup(obj);
		else
		    useupf(obj, obj->quan);
#if 0 /*JP*/
		losehp(d(6,6), "magical explosion", KILLED_BY_AN);
#else
		losehp(d(6,6),"魔法の爆発で", KILLED_BY_AN);
#endif
		return -1;
	    }
	}
	return 1;
}

int
dodip()
{
	struct obj *potion, *obj, *singlepotion;
	const char *tmp;
	uchar here;
	char allowall[2], qbuf[QBUFSZ], Your_buf[BUFSZ];
	short mixture;
	int res;

	allowall[0] = ALL_CLASSES; allowall[1] = '\0';
	if(!(obj = getobj(allowall, "dip")))
		return(0);

	here = levl[u.ux][u.uy].typ;
	/* Is there a fountain to dip into here? */
	if (IS_FOUNTAIN(here)) {
/*JP
		if(yn("Dip it into the fountain?") == 'y') {
*/
		if(yn("泉に浸しますか？") == 'y') {
			dipfountain(obj);
			return(1);
		}
	} else if (IS_TOILET(here)) {        
/*JP
		if(yn("Dip it into the toilet?") == 'y') {
*/
		if(yn("トイレに浸しますか？") == 'y') {
			diptoilet(obj);
			return(1);
		}
	} else if (is_pool(u.ux,u.uy)) {
		tmp = waterbody_name(u.ux,u.uy);
/*JP
		Sprintf(qbuf, "Dip it into the %s?", tmp);
*/
		Sprintf(qbuf, "%sに浸しますか？", tmp);
		if (yn(qbuf) == 'y') {
		    if (Levitation) {
			floating_above(tmp);
#ifdef STEED
		    } else if (u.usteed && !is_swimmer(u.usteed->data) &&
			    P_SKILL(P_RIDING) < P_BASIC) {
			rider_cant_reach(); /* not skilled enough to reach */
#endif
		    } else {
			(void) get_wet(obj, level.flags.lethe);
			if (obj->otyp == POT_ACID) useup(obj);
		    }
		    return 1;
		}
	}

	if(!(potion = getobj(beverages, "dip into")))
		return(0);
	if (potion == obj && potion->quan == 1L) {
/*JP
		pline("That is a potion bottle, not a Klein bottle!");
*/
		pline("これは薬瓶だ！クラインの壷じゃない！");
		return 0;
	}

	if(potion->otyp != POT_WATER && obj->otyp == POT_WATER) {
	  /* swap roles, to ensure symmetry */
	  struct obj *otmp = potion;
	  potion = obj;
	  obj = otmp;
	} 
	potion->in_use = TRUE;          /* assume it will be used up */
	if(potion->otyp == POT_WATER) {
		boolean useeit = !Blind;
		if (useeit) (void) Shk_Your(Your_buf, obj);
		if (potion->blessed) {
			if (obj->cursed) {
				if (useeit)
#if 0 /*JP*/
				    pline("%s %s %s.",
					  Your_buf,
					  aobjnam(obj, "softly glow"),
					  hcolor(NH_AMBER));
#else
				    pline("%s%sはうっすらと%s輝いた．",
					  Your_buf,
					  xname(obj),
					  jconj_adj(hcolor(NH_AMBER)));
#endif
				uncurse(obj);
				obj->bknown=1;
	poof:
				if(!(objects[potion->otyp].oc_name_known) &&
				   !(objects[potion->otyp].oc_uname))
					docall(potion);
				useup(potion);
				return(1);
			} else if(!obj->blessed) {
				if (useeit) {
				    tmp = hcolor(NH_LIGHT_BLUE);
#if 0 /*JP*/
				    pline("%s %s with a%s %s aura.",
					  Your_buf,
					  aobjnam(obj, "softly glow"),
					  index(vowels, *tmp) ? "n" : "", tmp);
#else
				    pline("%s%sはぼんやりとした%sオーラにつつまれた．",
					  Your_buf, xname(obj), tmp);
#endif
				}
				bless(obj);
				obj->bknown=1;
				goto poof;
			}
		} else if (potion->cursed) {
			if (obj->blessed) {
				if (useeit)
#if 0 /*JP*/
				    pline("%s %s %s.",
					  Your_buf,
					  aobjnam(obj, "glow"),
					  hcolor((const char *)"brown"));
#else
				    pline("%s%sは%s輝いた．",
					  Your_buf, xname(obj),
					  jconj_adj(hcolor((const char *)"茶色の")));
#endif
				unbless(obj);
				obj->bknown=1;
				goto poof;
			} else if(!obj->cursed) {
				if (useeit) {
				    tmp = hcolor(NH_BLACK);
#if 0 /*JP*/
				    pline("%s %s with a%s %s aura.",
					  Your_buf,
					  aobjnam(obj, "glow"),
					  index(vowels, *tmp) ? "n" : "", tmp);
#else
				    pline("%s%sは%sオーラにつつまれた．",
					  Your_buf, xname(obj),tmp);
#endif
				}
				curse(obj);
				obj->bknown=1;
				goto poof;
			}
		} else {
			switch(artifact_wet(obj,TRUE)) {
				/* Assume ZT_xxx is AD_xxx-1 */
				case -1: break;
				default:
					zap_over_floor(u.ux, u.uy,
					  (artifact_wet(obj,TRUE)-1), NULL);
					break;
			}
			if (get_wet(obj, FALSE))
			    goto poof;
		}
	} else if (potion->otyp == POT_AMNESIA) {
	    if (potion == obj) {
		obj->in_use = FALSE;
		potion = splitobj(obj, 1L);
		potion->in_use = TRUE;
	    }
	    if (get_wet(obj, TRUE)) goto poof;
	}
	/* WAC - Finn Theoderson - make polymorph and gain level msgs similar
	 * 	 Give out name of new object and allow user to name the potion
	 */
	else if (obj->otyp == POT_POLYMORPH ||
		potion->otyp == POT_POLYMORPH) {
	    /* some objects can't be polymorphed */
	    if (obj->otyp == potion->otyp ||	/* both POT_POLY */
		    obj->otyp == WAN_POLYMORPH ||
		    obj->otyp == SPE_POLYMORPH ||
		    obj == uball || obj == uskin ||
		    obj_resists(obj->otyp == POT_POLYMORPH ?
				potion : obj, 5, 95)) {
		pline(nothing_happens);
	    } else {
	    	boolean was_wep = FALSE, was_swapwep = FALSE, was_quiver = FALSE;
		short save_otyp = obj->otyp;
		/* KMH, conduct */
		u.uconduct.polypiles++;

		if (obj == uwep) was_wep = TRUE;
		else if (obj == uswapwep) was_swapwep = TRUE;
		else if (obj == uquiver) was_quiver = TRUE;

		obj = poly_obj(obj, STRANGE_OBJECT);

		if (was_wep) setuwep(obj, TRUE);
		else if (was_swapwep) setuswapwep(obj, TRUE);
		else if (was_quiver) setuqwep(obj);

		if (obj->otyp != save_otyp) {
			makeknown(POT_POLYMORPH);
			useup(potion);
			prinv((char *)0, obj, 0L);
			return 1;
		} else {
/*JP
			pline("Nothing seems to happen.");
*/
			pline("何も起こらなかったようだ．");
			goto poof;
		}
	    }
	    potion->in_use = FALSE;	/* didn't go poof */
	    return(1);
#ifdef UNPOLYPILE
	} else if (potion->otyp == POT_RESTORE_ABILITY && is_hazy(obj)) {
		/* KMH -- Restore ability will stop unpolymorphing */
		stop_timer(UNPOLY_OBJ, (genericptr_t) obj);
		obj->oldtyp = STRANGE_OBJECT;
		if (!Blind)
/*JP
			pline("%s seems less hazy.", Yname2(obj));
*/
			pline("%sのぼんやりした感じがなくなった．", Yname2(obj));
		useup(potion);
		return (1);
#endif
	} else if(obj->oclass == POTION_CLASS && obj->otyp != potion->otyp) {
		/* Mixing potions is dangerous... */
/*JP
		pline_The("potions mix...");
*/
		pline("薬は調合された．．．");
		/* KMH, balance patch -- acid is particularly unstable */
		if (obj->cursed || obj->otyp == POT_ACID ||
		    potion->cursed || potion->otyp == POT_ACID || !rn2(10)) {
/*JP
			pline("BOOM!  They explode!");
*/
			pline("バーン！爆発した！");
			exercise(A_STR, FALSE);
			if (!breathless(youmonst.data) || haseyes(youmonst.data))
				potionbreathe(obj);
			useup(obj);
			useup(potion);
			/* MRKR: an alchemy smock ought to be */
			/* some protection against this: */
			losehp(Acid_resistance ? rnd(5) : rnd(10),
/*JP
			       "alchemic blast", KILLED_BY_AN);
*/
			       "調合の失敗で", KILLED_BY_AN);
			return(1);
		}

		obj->blessed = obj->cursed = obj->bknown = 0;
		if (Blind || Hallucination) obj->dknown = 0;

		if ((mixture = mixtype(obj, potion)) != 0) {
			obj->otyp = mixture;
		} else {
		    switch (obj->odiluted ? 1 : rnd(8)) {
			case 1:
				obj->otyp = POT_WATER;
				break;
			case 2:
			case 3:
				obj->otyp = POT_SICKNESS;
				break;
			case 4:
				{
				  struct obj *otmp;
				  otmp = mkobj(POTION_CLASS,FALSE);
				  obj->otyp = otmp->otyp;
				  obfree(otmp, (struct obj *)0);
				}
				break;
			default:
				if (!Blind)
/*JP
			  pline_The("mixture glows brightly and evaporates.");
*/
				  pline("混ぜると薬は明るく輝き，蒸発した．");
				useup(obj);
				useup(potion);
				return(1);
		    }
		}

		obj->odiluted = (obj->otyp != POT_WATER);

		if (obj->otyp == POT_WATER && !Hallucination) {
#if 0 /*JP*/
			pline_The("mixture bubbles%s.",
				Blind ? "" : ", then clears");
#else
			pline("薬を混ぜると泡だ%s．",
				Blind ? "った" : "ち，澄んだ");
#endif
		} else if (!Blind) {
#if 0 /*JP*/
			pline_The("mixture looks %s.",
				hcolor(OBJ_DESCR(objects[obj->otyp])));
#else
			pline("混ぜた薬は%s薬に見える．",
				jtrns_obj('!',OBJ_DESCR(objects[obj->otyp])));
#endif
		}

		useup(potion);
		return(1);
	}
#ifdef INVISIBLE_OBJECTS
	if (!always_visible(obj)) {
	    if (potion->otyp == POT_INVISIBILITY && !obj->oinvis) {
		obj->oinvis = TRUE;
		if (!Blind)
#if 0 /*JP*/
		    pline(!See_invisible ? "Where did %s go?" :
			  "Gee!  All of a sudden you can see right through %s.",
			  the(xname(obj)));
#else
		    pline(!See_invisible ? "%sはどこに行った？" :
			  "ゲーッ！%sは突然透き通って見えるようになった！",
			  xname(obj));
#endif /*JP*/
		goto poof;
	    } else if (potion->otyp == POT_SEE_INVISIBLE && obj->oinvis) {
		obj->oinvis = FALSE;
		if (!Blind) {
		    if (!See_invisible)
/*JP
			pline("So that's where %s went!", the(xname(obj)));
*/
			pline("%sは今までどこに行ってたんだ！", xname(obj));
		    else
#if 0 /*JP*/
			You("can no longer see through %s.",
				the(xname(obj)));
#else
			You("%sはもう透き通っているように見えない．", xname(obj));
#endif /*JP*/
		}
		goto poof;
	    }
	}
#endif

	if(is_poisonable(obj)) {
	    if(potion->otyp == POT_SICKNESS && !obj->opoisoned) {
		char buf[BUFSZ];
		if (potion->quan > 1L)
/*JP
		    Sprintf(buf, "One of %s", the(xname(potion)));
*/
		    Sprintf(buf, "%sのひとつ", the(xname(potion)));
		else
		    Strcpy(buf, The(xname(potion)));
/*JP
		pline("%s forms a coating on %s.",
*/
		pline("%sが%sに塗られた．",
		      buf, the(xname(obj)));
		obj->opoisoned = TRUE;
		goto poof;
	    } else if(obj->opoisoned &&
		      (potion->otyp == POT_HEALING ||
		       potion->otyp == POT_EXTRA_HEALING ||
		       potion->otyp == POT_FULL_HEALING)) {
/*JP
		pline("A coating wears off %s.", the(xname(obj)));
*/
		pline("毒が%sから剥げおちた．", the(xname(obj)));
		obj->opoisoned = 0;
		goto poof;
	    }
	}

	if (potion->otyp == POT_OIL) {
	    boolean wisx = FALSE;
	    if (potion->lamplit) {	/* burning */
		int omat = objects[obj->otyp].oc_material;
		/* the code here should be merged with fire_damage */
		if (catch_lit(obj)) {
		    /* catch_lit does all the work if true */
		} else if (obj->oerodeproof || obj_resists(obj, 5, 95) ||
			   !is_flammable(obj) || obj->oclass == FOOD_CLASS) {
#if 0 /*JP*/
		    pline("%s %s to burn for a moment.",
			  Yname2(obj), otense(obj, "seem"));
#else
		    pline("%sは一瞬燃え上がったように見えた．", Yname2(obj));
#endif
		} else {
		    if ((omat == PLASTIC || omat == PAPER) && !obj->oartifact)
			obj->oeroded = MAX_ERODE;
#if 0 /*JP*/
		    pline_The("burning oil %s %s.",
			    obj->oeroded == MAX_ERODE ? "destroys" : "damages",
			    yname(obj));
#else
		    pline("%sは燃えている油によって%s．",
			    yname(obj),
			    obj->oeroded == MAX_ERODE ? "破壊された" : "傷つけられた");
#endif
		    if (obj->oeroded == MAX_ERODE) {
			obj_extract_self(obj);
			obfree(obj, (struct obj *)0);
			obj = (struct obj *) 0;
		    } else {
			/* we know it's carried */
			if (obj->unpaid) {
			    /* create a dummy duplicate to put on bill */
/*JP
			    verbalize("You burnt it, you bought it!");
*/
			    verbalize("燃やしたのなら買ってもらうよ！");
			    bill_dummy_object(obj);
			}
			obj->oeroded++;
		    }
		}
	    } else if (potion->cursed) {
/*JP
		pline_The("potion spills and covers your %s with oil.",
*/
		pline("薬は飛び散りあなたの%sを油まみれにした．",
			  makeplural(body_part(FINGER)));
		incr_itimeout(&Glib, d(2,10));
	    } else if (obj->oclass != WEAPON_CLASS && !is_weptool(obj)) {
		/* the following cases apply only to weapons */
		goto more_dips;
	    /* Oil removes rust and corrosion, but doesn't unburn.
	     * Arrows, etc are classed as metallic due to arrowhead
	     * material, but dipping in oil shouldn't repair them.
	     */
	    } else if ((!is_rustprone(obj) && !is_corrodeable(obj)) ||
			is_ammo(obj) || (!obj->oeroded && !obj->oeroded2)) {
		/* uses up potion, doesn't set obj->greased */
#if 0 /*JP*/
		pline("%s %s with an oily sheen.",
		      Yname2(obj), otense(obj, "gleam"));
#else
		pline("%sは油の光沢できらりと光った．",
		      Yname2(obj));
#endif
	    } else {
#if 0 /*JP*/
		pline("%s %s less %s.",
		      Yname2(obj), otense(obj, "are"),
		      (obj->oeroded && obj->oeroded2) ? "corroded and rusty" :
			obj->oeroded ? "rusty" : "corroded");
#else
		pline("%sの%sが取れた．", Yname2(obj),
		      (obj->oeroded && obj->oeroded2) ? "腐食と錆" :
		      obj->oeroded ? "錆" : "腐食");
#endif
		if (obj->oeroded > 0) obj->oeroded--;
		if (obj->oeroded2 > 0) obj->oeroded2--;
		wisx = TRUE;
	    }
	    exercise(A_WIS, wisx);
	    makeknown(potion->otyp);
	    useup(potion);
	    return 1;
	} else if (potion->otyp == POT_GAIN_LEVEL) {
	    res = upgrade_obj(obj);
	    if (res != 0) {
		if (res == 1) {
		    /* The object was upgraded */
/*JP
		    pline("Hmm!  You don't recall dipping that into the potion.");
*/
		    pline("むむ！ 浸してしまった物はリコールできない．");
		    prinv((char *)0, obj, 0L);
		} /* else potion exploded */
		if (!objects[potion->otyp].oc_name_known &&
			!objects[potion->otyp].oc_uname)
		    docall(potion);
		useup(potion);
		update_inventory();
		exercise(A_WIS, TRUE);
		return 1;
	    }
	    /* no return here, go for Interesting... message */
	}

	/* KMH, balance patch -- acid affects damage(proofing) */
	if (potion->otyp == POT_ACID && (obj->oclass == ARMOR_CLASS ||
		obj->oclass == WEAPON_CLASS || is_weptool(obj))) {
	    if (!potion->blessed && obj->oerodeproof) {
/*JP
		pline("%s %s golden shield.",  Yname2(obj),
			(obj->quan > 1L) ? "lose their" : "loses its");
*/
		/*日本語では単複の別は省略*/
		pline("%sは金色の輝きを失った．",  Yname2(obj));
		obj->oerodeproof = 0;
		makeknown(potion->otyp);
	    } else {
/*JP
		pline("%s looks a little dull.", Yname2(obj));
*/
		pline("%sの色が少し鈍ったように見える．", Yname2(obj));
		if (!objects[potion->otyp].oc_name_known &&
			!objects[potion->otyp].oc_uname)
		    docall(potion);
	    }
	    exercise(A_WIS, FALSE);
  	    useup(potion);
	    return 1;
	}
    more_dips:

	/* Allow filling of MAGIC_LAMPs to prevent identification by player */
	if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP) &&
	   (potion->otyp == POT_OIL)) {

	    /* Turn off engine before fueling, turn off fuel too :-)  */
	    if (obj->lamplit || potion->lamplit) {
		useup(potion);
		explode(u.ux, u.uy, ZT_SPELL(ZT_FIRE), d(6,6), 0, EXPL_FIERY);
		exercise(A_WIS, FALSE);
		return 1;
	    }
	    /* Adding oil to an empty magic lamp renders it into an oil lamp */
	    if ((obj->otyp == MAGIC_LAMP) && obj->spe == 0) {
		obj->otyp = OIL_LAMP;
		obj->age = 0;
	    }
	    if (obj->age > 1000L) {
/*JP
                pline("%s %s full.", Yname2(obj), otense(obj, "are"));
*/
		pline("%sにはたっぷり入っている．", Yname2(obj));
		potion->in_use = FALSE; /* didn't go poof */
	    } else {
/*JP
                You("fill your %s with oil.", yname(obj));
*/
		You("%sに油を入れた．", yname(obj));
		check_unpaid(potion);	/* Yendorian Fuel Tax */
		obj->age += 2*potion->age;	/* burns more efficiently */
		if (obj->age > 1500L) obj->age = 1500L;
		useup(potion);
		exercise(A_WIS, TRUE);
	    }
	    makeknown(POT_OIL);
	    obj->spe = 1;
	    update_inventory();
	    return 1;
	}

	potion->in_use = FALSE;         /* didn't go poof */
	if ((obj->otyp == UNICORN_HORN || obj->oclass == GEM_CLASS) &&
	    (mixture = mixtype(obj, potion)) != 0) {
#if 0 /*JP*/
		char oldbuf[BUFSZ], newbuf[BUFSZ];
#else
		char oldbuf[BUFSZ];
#endif
		short old_otyp = potion->otyp;
		boolean old_dknown = FALSE;
		boolean more_than_one = potion->quan > 1;

		oldbuf[0] = '\0';
		if (potion->dknown) {
		    old_dknown = TRUE;
#if 0 /*JP*/
		    Sprintf(oldbuf, "%s ",
			    hcolor(OBJ_DESCR(objects[potion->otyp])));
#else
		    Sprintf(oldbuf, "%s",
			    jtrns_obj('!', hcolor(OBJ_DESCR(objects[potion->otyp]))));
#endif
		}
		/* with multiple merged potions, split off one and
		   just clear it */
		if (potion->quan > 1L) {
		    singlepotion = splitobj(potion, 1L);
		} else singlepotion = potion;
		
		/* MRKR: Gems dissolve in acid to produce new potions */

		if (obj->oclass == GEM_CLASS && potion->otyp == POT_ACID) {
		    struct obj *singlegem = (obj->quan > 1L ? 
					     splitobj(obj, 1L) : obj);

		    singlegem->in_use = TRUE;
		    if (potion->otyp == POT_ACID && 
		      (obj->otyp == DILITHIUM_CRYSTAL || 
		       potion->cursed || !rn2(10))) {
			/* Just to keep them on their toes */

			singlepotion->in_use = TRUE;
			if (Hallucination && obj->otyp == DILITHIUM_CRYSTAL) {
			    /* Thanks to Robin Johnson */
/*JP
			    pline("Warning, Captain!  The warp core has been breached!");
*/
			    pline("危険です艦長！ このままではワープコアが破壊されます！");
			}
/*JP
			pline("BOOM!  %s explodes!", The(xname(singlegem)));
*/
			pline("ボーンッ！ %sが爆発した！", xname(singlegem));
			exercise(A_STR, FALSE);
			if (!breathless(youmonst.data) || haseyes(youmonst.data))
			    potionbreathe(singlepotion);
			useup(singlegem);
			useup(singlepotion);
			/* MRKR: an alchemy smock ought to be */
			/* some protection against this: */
			losehp(Acid_resistance ? rnd(5) : rnd(10), 
/*JP
			       "alchemic blast", KILLED_BY_AN);
*/
			       "調合の失敗で", KILLED_BY_AN);
			return(1);	  
		    }

/*JP
		    pline("%s dissolves in %s.", The(xname(singlegem)), 
*/
		    pline("%sは%sに溶けた．", The(xname(singlegem)), 
			  the(xname(singlepotion)));
		    makeknown(POT_ACID);
		    useup(singlegem);
		}

		if(singlepotion->unpaid && costly_spot(u.ux, u.uy)) {
/*JP
		    You("use it, you pay for it.");
*/
		    You("使ってしまった，賠償せねばならない．");
		    bill_dummy_object(singlepotion);
		}

		if (singlepotion->otyp == mixture) {		  
		    /* no change - merge it back in */
		    if (more_than_one && !merged(&potion, &singlepotion)) {
			/* should never happen */
			impossible("singlepotion won't merge with parent potion.");
		    }
		} else {		  
		singlepotion->otyp = mixture;
		singlepotion->blessed = 0;
		if (mixture == POT_WATER)
		    singlepotion->cursed = singlepotion->odiluted = 0;
		else
		    singlepotion->cursed = obj->cursed;  /* odiluted left as-is */
		singlepotion->bknown = FALSE;
		if (Blind) {
		    singlepotion->dknown = FALSE;
		} else {
		    singlepotion->dknown = !Hallucination;
#if 0 /*JP*/
		    if (mixture == POT_WATER && singlepotion->dknown)
			Sprintf(newbuf, "clears");
		    else
			Sprintf(newbuf, "turns %s",
				hcolor(OBJ_DESCR(objects[mixture])));
		    pline_The("%spotion%s %s.", oldbuf,
			      more_than_one ? " that you dipped into" : "",
			      newbuf);
#else
		    if (mixture == POT_WATER && singlepotion->dknown)
			pline("%s薬は透明になった．", oldbuf);
		    else
			pline("%s薬は%s薬になった．", oldbuf, jtrns_obj('!',
				hcolor(OBJ_DESCR(objects[mixture]))));
#endif
		    if(!objects[old_otyp].oc_uname &&
			!objects[old_otyp].oc_name_known && old_dknown) {
			struct obj fakeobj;
			fakeobj = zeroobj;
			fakeobj.dknown = 1;
			fakeobj.otyp = old_otyp;
			fakeobj.oclass = POTION_CLASS;
			docall(&fakeobj);
		    }
		}
		obj_extract_self(singlepotion);
		singlepotion = hold_another_object(singlepotion,
/*JP
					"You juggle and drop %s!",
*/
					"お手玉して%sを落としてしまった！",
					doname(singlepotion), (const char *)0);
		update_inventory();
		}

		return(1);
	}

/*JP
	pline("Interesting...");
*/
	pline("面白い．．．");
	return(1);
}


void
djinni_from_bottle(obj)
register struct obj *obj;
{
	struct monst *mtmp;
	int genie_type;        
	int chance;

#if 0
	/* KMH -- See comments in monst.c */
	switch (rn2(4)) {
		default:
		case 0: genie_type = PM_DJINNI; break;
		case 1: genie_type = PM_EFREETI; break;
		case 2: genie_type = PM_MARID; break;
		case 3: genie_type = PM_DAO; break;
	}
#else
	genie_type = PM_DJINNI;
#endif
	if(!(mtmp = makemon(&mons[genie_type], u.ux, u.uy, NO_MM_FLAGS))){
#if 0/*JP*/
		pline("It turns out to be empty.");
#else
		if (obj->otyp == MAGIC_LAMP ) {
			pline("ランプは空っぽだった．");
		} else {
			pline("瓶は空っぽだった．");
		}
#endif
		return;
	}

	if (!Blind) {
#if 0 /*JP*/
		pline("In a cloud of smoke, %s emerges!", a_monnam(mtmp));
		pline("%s speaks.", Monnam(mtmp));
#else
		pline("煙の中から，%sが現れた！", a_monnam(mtmp));
		pline("%sは話しかけた．", Monnam(mtmp));
#endif
	} else {
#if 0 /*JP*/
		You("smell acrid fumes.");
		pline("%s speaks.", Something);
#else
		pline("ツンとする匂いがした．");
		pline("%sが話しかけた．", Something);
#endif
	}

	chance = rn2(5);
	if (obj->blessed) chance = (chance == 4) ? rnd(4) : 0;
	else if (obj->cursed) chance = (chance == 0) ? rn2(4) : 4;
	/* 0,1,2,3,4:  b=80%,5,5,5,5; nc=20%,20,20,20,20; c=5%,5,5,5,80 */

	switch (chance) {
/*JP
	case 0 : verbalize("I am in your debt.  I will grant one wish!");
*/
	case 0 : verbalize("お前には借りができた．１つ願いをかなえてやろう！");
		makewish();
		mongone(mtmp);
		break;
/*JP
	case 1 : verbalize("Thank you for freeing me!");
*/
	case 1 : verbalize("私を助けてくれたことに感謝する！");
		(void) tamedog(mtmp, (struct obj *)0);
		break;
/*JP
	case 2 : verbalize("You freed me!");
*/
	case 2 : verbalize("解放してくれたのはお前か！");
		mtmp->mpeaceful = TRUE;
		set_malign(mtmp);
		break;
/*JP
	case 3 : verbalize("It is about time!");
*/
	case 3 : verbalize("さらばだ！");
/*JP
		pline("%s vanishes.", Monnam(mtmp));
*/
		pline("%sは消えた．", Monnam(mtmp));
		mongone(mtmp);
		break;
/*JP
	default: verbalize("You disturbed me, fool!");
*/
	default: verbalize("おまえは私の眠りを妨げた．おろかものめ！");
		break;
	}
}

/* clone a gremlin or mold (2nd arg non-null implies heat as the trigger);
   hit points are cut in half (odd HP stays with original) */
struct monst *
split_mon(mon, mtmp)
struct monst *mon,	/* monster being split */
	     *mtmp;	/* optional attacker whose heat triggered it */
{
	struct monst *mtmp2;
	char reason[BUFSZ];

	reason[0] = '\0';
#if 0 /*JP*/
	if (mtmp) Sprintf(reason, " from %s heat",
			  (mtmp == &youmonst) ? (const char *)"your" :
			      (const char *)s_suffix(mon_nam(mtmp)));
#else
	if (mtmp)
	    Sprintf(reason, "%sの熱で", (mtmp == &youmonst)
		? (const char *)"あなた" : mon_nam(mtmp));
#endif

	if (mon == &youmonst) {
	    mtmp2 = cloneu();
	    if (mtmp2) {
		mtmp2->mhpmax = u.mhmax / 2;
		u.mhmax -= mtmp2->mhpmax;
		flags.botl = 1;
/*JP
		You("multiply%s!", reason);
*/
		You("%s分裂した！", reason);
	    }
	} else {
	    mtmp2 = clone_mon(mon, 0, 0);
	    if (mtmp2) {
		mtmp2->mhpmax = mon->mhpmax / 2;
		mon->mhpmax -= mtmp2->mhpmax;
		if (canspotmon(mon))
/*JP
		    pline("%s multiplies%s!", Monnam(mon), reason);
*/
		    pline("%sは%s分裂した！", Monnam(mon), reason);
	    }
	}
	return mtmp2;
}

#endif /* OVLB */

/*potion.c*/
