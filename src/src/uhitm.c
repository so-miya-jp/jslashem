/*	SCCS Id: @(#)uhitm.c	3.4	2003/02/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, 2004-2006
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

STATIC_DCL boolean FDECL(known_hitum, (struct monst *,int,int *,struct attack *));
STATIC_DCL void FDECL(steal_it, (struct monst *, struct attack *));
#if 0
STATIC_DCL boolean FDECL(hitum, (struct monst *,int,int,struct attack *));
#endif
STATIC_DCL boolean FDECL(hmon_hitmon, (struct monst *,struct obj *,int));
#ifdef STEED
STATIC_DCL int FDECL(joust, (struct monst *,struct obj *));
#endif
STATIC_DCL void NDECL(demonpet);
STATIC_DCL boolean FDECL(m_slips_free, (struct monst *mtmp,struct attack *mattk));
STATIC_DCL int FDECL(explum, (struct monst *,struct attack *));
STATIC_DCL void FDECL(start_engulf, (struct monst *));
STATIC_DCL void NDECL(end_engulf);
STATIC_DCL int FDECL(gulpum, (struct monst *,struct attack *));
STATIC_DCL boolean FDECL(hmonas, (struct monst *,int));
STATIC_DCL void FDECL(nohandglow, (struct monst *));
STATIC_DCL boolean FDECL(shade_aware, (struct obj *));

static int NDECL(martial_dmg);

extern boolean notonhead;	/* for long worms */
/* The below might become a parameter instead if we use it a lot */
static int dieroll;
static int rolls[2][2];
#define dice(x) rolls[0][x]
#define tohit(x) rolls[1][x]
#define UWEP_ROLL	0
#define USWAPWEP_ROLL	1

/* Used to flag attacks caused by Stormbringer's maliciousness. */
static boolean override_confirmation = 0;

/* Used to control whether Drow's sleep attack should succeed. */
static boolean barehanded_hit = 0;

/* WAC for mhit,  two weapon attacking */
#define HIT_UWEP 	1
#define HIT_USWAPWEP 	2
#define HIT_BODY 	4		/* Hit with other body part */
#define HIT_OTHER 	8		/* Hit without touching */
#define HIT_FATAL 	16

#define PROJECTILE(obj)	((obj) && is_ammo(obj))

/* modified from hurtarmor() in mhitu.c */
/* This is not static because it is also used for monsters rusting monsters */
void
hurtmarmor(mdef, attk)
struct monst *mdef;
int attk;
{
	int	hurt;
	struct obj *target;

	switch(attk) {
	    /* 0 is burning, which we should never be called with */
	    case AD_RUST: hurt = 1; break;
	    case AD_CORR: hurt = 3; break;
	    default: hurt = 2; break;
	}
	/* What the following code does: it keeps looping until it
	 * finds a target for the rust monster.
	 * Head, feet, etc... not covered by metal, or covered by
	 * rusty metal, are not targets.  However, your body always
	 * is, no matter what covers it.
	 */
	while (1) {
	    switch(rn2(5)) {
	    case 0:
		target = which_armor(mdef, W_ARMH);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    case 1:
		target = which_armor(mdef, W_ARMC);
		if (target) {
		    (void)rust_dmg(target, xname(target), hurt, TRUE, mdef);
		    break;
		}
		if ((target = which_armor(mdef, W_ARM)) != (struct obj *)0) {
		    (void)rust_dmg(target, xname(target), hurt, TRUE, mdef);
#ifdef TOURIST
		} else if ((target = which_armor(mdef, W_ARMU)) != (struct obj *)0) {
		    (void)rust_dmg(target, xname(target), hurt, TRUE, mdef);
#endif
		}
		break;
	    case 2:
		target = which_armor(mdef, W_ARMS);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    case 3:
		target = which_armor(mdef, W_ARMG);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    case 4:
		target = which_armor(mdef, W_ARMF);
		if (!target || !rust_dmg(target, xname(target), hurt, FALSE, mdef))
		    continue;
		break;
	    }
	    break; /* Out of while loop */
	}
}

/*
 * Now returns a bit mask of attacks that may proceed. Note that barehanded
 * returns HIT_UWEP. -ALI
 */

int
attack_checks(mtmp, barehanded)
register struct monst *mtmp;
boolean barehanded;
{
	int retval;
	char qbuf[QBUFSZ];

	if (barehanded || !u.twoweap || !uswapwep)
		retval = HIT_UWEP;
	else
		retval = HIT_UWEP | HIT_USWAPWEP;

	/* if you're close enough to attack, alert any waiting monster */
	mtmp->mstrategy &= ~STRAT_WAITMASK;

	if (u.uswallow && mtmp == u.ustuck) return retval;

	if (flags.forcefight) {
		/* Do this in the caller, after we checked that the monster
		 * didn't die from the blow.  Reason: putting the 'I' there
		 * causes the hero to forget the square's contents since
		 * both 'I' and remembered contents are stored in .glyph.
		 * If the monster dies immediately from the blow, the 'I' will
		 * not stay there, so the player will have suddenly forgotten
		 * the square's contents for no apparent reason.
		if (!canspotmon(mtmp) &&
		    !memory_is_invisible(u.ux+u.dx, u.uy+u.dy))
			map_invisible(u.ux+u.dx, u.uy+u.dy);
		 */
		return retval;
	}

	/* Put up an invisible monster marker, but with exceptions for
	 * monsters that hide and monsters you've been warned about.
	 * The former already prints a warning message and
	 * prevents you from hitting the monster just via the hidden monster
	 * code below; if we also did that here, similar behavior would be
	 * happening two turns in a row.  The latter shows a glyph on
	 * the screen, so you know something is there.
	 */
	if (!canspotmon(mtmp) &&
		    !glyph_is_warning(glyph_at(u.ux+u.dx,u.uy+u.dy)) &&
		    !memory_is_invisible(u.ux+u.dx, u.uy+u.dy) &&
		    !(!Blind && mtmp->mundetected && hides_under(mtmp->data))) {
/*JP
		pline("Wait!  There's %s there you can't see!",
*/
		pline("待て！そこには姿の見えない%sがいる！",
			something);
		map_invisible(u.ux+u.dx, u.uy+u.dy);
		/* if it was an invisible mimic, treat it as if we stumbled
		 * onto a visible mimic
		 */
		if(mtmp->m_ap_type && !Protection_from_shape_changers) {
		    if(!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data,AD_STCK))
			setustuck(mtmp);
		}
		wakeup(mtmp); /* always necessary; also un-mimics mimics */
		return 0;
	}

	if (mtmp->m_ap_type && !Protection_from_shape_changers &&
	   !sensemon(mtmp) &&
	   !glyph_is_warning(glyph_at(u.ux+u.dx,u.uy+u.dy))) {
		/* If a hidden mimic was in a square where a player remembers
		 * some (probably different) unseen monster, the player is in
		 * luck--he attacks it even though it's hidden.
		 */
		if (memory_is_invisible(mtmp->mx, mtmp->my)) {
		    seemimic(mtmp);
		    return retval;
		}
		stumble_onto_mimic(mtmp);
		return 0;
	}

	if (mtmp->mundetected && !canseemon(mtmp) &&
		!glyph_is_warning(glyph_at(u.ux+u.dx,u.uy+u.dy)) &&
		(hides_under(mtmp->data) || mtmp->data->mlet == S_EEL)) {
	    mtmp->mundetected = mtmp->msleeping = 0;
	    newsym(mtmp->mx, mtmp->my);
	    if (memory_is_invisible(mtmp->mx, mtmp->my)) {
		seemimic(mtmp);
		return retval;
	    }
	    if (!(Blind ? Blind_telepat : Unblind_telepat)) {
		struct obj *obj;

		if (Blind || (is_pool(mtmp->mx,mtmp->my) && !Underwater))
/*JP
		    pline("Wait!  There's a hidden monster there!");
*/
		    pline("待て！そこには怪物が隠れている！");
		else if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0)
#if 0 /*JP*/
		    pline("Wait!  There's %s hiding under %s!",
			  an(l_monnam(mtmp)), doname(obj));
#else
		    pline("待て！%sの下に%sが隠れている！",
			  doname(obj), l_monnam(mtmp));
#endif
		return 0;
	    }
	}

	/*
	 * make sure to wake up a monster from the above cases if the
	 * hero can sense that the monster is there.
	 */
	if ((mtmp->mundetected || mtmp->m_ap_type) && sensemon(mtmp)) {
	    mtmp->mundetected = 0;
	    wakeup(mtmp);
	}

	if (flags.confirm && mtmp->mpeaceful
	    && !Confusion && !Hallucination && !Stunned) {
		/* Intelligent chaotic weapons (Stormbringer) want blood */
		if (!barehanded &&
		  uwep && uwep->oartifact == ART_STORMBRINGER) {
			override_confirmation = HIT_UWEP;
			return retval;
		}
		if (canspotmon(mtmp)) {
/*JP
			Sprintf(qbuf, "Really attack %s?", mon_nam(mtmp));
*/
			Sprintf(qbuf, "本当に%sを攻撃するの？", mon_nam(mtmp));
			if (yn(qbuf) != 'y') {
				/* Stormbringer is not tricked so easily */
				if (!barehanded && u.twoweap && uswapwep &&
				  uswapwep->oartifact == ART_STORMBRINGER) {
					override_confirmation = HIT_USWAPWEP;
					/* Lose primary attack */
					return HIT_USWAPWEP;
				}
				flags.move = 0;
				return 0;
			}
		}
	}

	return retval;
}

/*
 * It is unchivalrous for a knight to attack the defenseless or from behind.
 */
void
check_caitiff(mtmp)
struct monst *mtmp;
{
	if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL &&
	    (!mtmp->mcanmove || mtmp->msleeping ||
	     (mtmp->mflee && !mtmp->mavenge)) &&
	    u.ualign.record > -10) {
/*JP
	    You("caitiff!");
*/
	    pline("やっていることがまるで蛮族だ！");
	    adjalign(-1);
	}
}

schar
find_roll_to_hit(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	int tmp2;

	tmp = 1 + Luck + abon() + find_mac(mtmp) + u.uhitinc +
		maybe_polyd(youmonst.data->mlevel, u.ulevel);

	check_caitiff(mtmp);

/*	attacking peaceful creatures is bad for the samurai's giri */
	if (Role_if(PM_SAMURAI) && mtmp->mpeaceful &&
	    u.ualign.record > -10) {
/*JP
	    You("dishonorably attack the innocent!");
*/
	    You("恥ずべきことに罪無き者を手にかけた！");
	    adjalign(-1);
	}

/*	Adjust vs. (and possibly modify) monster state.		*/

	if(mtmp->mstun) tmp += 2;
	if(mtmp->mflee) tmp += 2;

	if (mtmp->msleeping) {
		mtmp->msleeping = 0;
		tmp += 2;
	}
	if(!mtmp->mcanmove) {
		tmp += 4;
		if(!rn2(10)) {
			mtmp->mcanmove = 1;
			mtmp->mfrozen = 0;
		}
	}

	if (is_orc(mtmp->data) && maybe_polyd(is_elf(youmonst.data),
			Race_if(PM_ELF)))
	    tmp++;

#if 0
	if(Role_if(PM_MONK) && !Upolyd) {
	    if (uarm) {
/*JP
		Your("armor is rather cumbersome...");
*/
	    	pline("鎧が邪魔だ．．．");
		tmp -= urole.spelarmr;
	    } else if (!uwep && !uarms) {
		tmp += (u.ulevel / 3) + 2;
	}
	}
#endif

	if(Role_if(PM_MONK) && !Upolyd) {
		if(!uwep && (!u.twoweap || !uswapwep) && !uarms && 
		  (!uarm || (uarm && uarm->otyp >= ROBE && 
		  	uarm->otyp <= ROBE_OF_WEAKNESS)))
		  	
		  tmp += (u.ulevel / 3) + 2;
		else if (!uwep && (!u.twoweap || !uswapwep)) {
#if 0 /*JP*/
		   pline("Your armor is rather cumbersome...");
#else
		   pline("鎧がちょっと邪魔だ．．．");
#endif
		   tmp += (u.ulevel / 9) + 1;
		}
	}
	/* special class effect uses... */
	if (tech_inuse(T_KIII)) tmp += 4;
	if (tech_inuse(T_BERSERK)) tmp += 2;

/*	with a lot of luggage, your agility diminishes */
	if ((tmp2 = near_capacity()) != 0) tmp -= (tmp2*2) - 1;
	if (u.utrap) tmp -= 3;
/*	Some monsters have a combination of weapon attacks and non-weapon
 *	attacks.  It is therefore wrong to add hitval to tmp; we must add
 *	it only for the specific attack (in hmonas()).
 */
/* WAC This is now taken care of later in player's case - for twoweapon */
/*
	if (uwep && !Upolyd) {
		tmp += hitval(uwep, mtmp);
		tmp += weapon_hit_bonus(uwep);
	}
*/
	return tmp;
}

/* try to attack; return FALSE if monster evaded */
/* u.dx and u.dy must be set */
boolean
attack(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	register struct permonst *mdat = mtmp->data;
	int mhit;

	/* This section of code provides protection against accidentally
	 * hitting peaceful (like '@') and tame (like 'd') monsters.
	 * Protection is provided as long as player is not: blind, confused,
	 * hallucinating or stunned.
	 * changes by wwp 5/16/85
	 * More changes 12/90, -dkh-. if its tame and safepet, (and protected
	 * 07/92) then we assume that you're not trying to attack. Instead,
	 * you'll usually just swap places if this is a movement command
	 */
	/* Intelligent chaotic weapons (Stormbringer) want blood */
	if (is_safepet(mtmp) && !flags.forcefight) {
	    if ((!uwep || uwep->oartifact != ART_STORMBRINGER) 
		&& (!u.twoweap || !uswapwep 
		   || uswapwep->oartifact != ART_STORMBRINGER)){
		/* there are some additional considerations: this won't work
		 * if in a shop or Punished or you miss a random roll or
		 * if you can walk thru walls and your pet cannot (KAA) or
		 * if your pet is a long worm (unless someone does better).
		 * there's also a chance of displacing a "frozen" monster.
		 * sleeping monsters might magically walk in their sleep.
		 */
		boolean foo = (Punished || !rn2(7) || is_longworm(mtmp->data)),
			inshop = FALSE;
		char *p;

		for (p = in_rooms(mtmp->mx, mtmp->my, SHOPBASE); *p; p++)
		    if (tended_shop(&rooms[*p - ROOMOFFSET])) {
			inshop = TRUE;
			break;
		    }

		if (inshop || foo ||
			(IS_ROCK(levl[u.ux][u.uy].typ) &&
					!passes_walls(mtmp->data))) {
		    char buf[BUFSZ];

		    monflee(mtmp, rnd(6), FALSE, FALSE);
		    Strcpy(buf, y_monnam(mtmp));
		    buf[0] = highc(buf[0]);
/*JP
		    You("stop.  %s is in the way!", buf);
*/
		    You("止まった．行く手に%sがいる！", buf);
		    return(TRUE);
		} else if ((mtmp->mfrozen || (! mtmp->mcanmove)
				|| (mtmp->data->mmove == 0)) && rn2(6)) {
/*JP
		    pline("%s doesn't seem to move!", Monnam(mtmp));
*/
		    pline("%sは動きそうにない！", Monnam(mtmp));
		    return(TRUE);
		} else return(FALSE);
	    }
	}

	/* possibly set in attack_checks;
	   examined in known_hitum, called via hitum or hmonas below */
	override_confirmation = 0;
	mhit = attack_checks(mtmp, !uwep);
	if (!mhit) return(TRUE);

	if (Upolyd) {
		/* certain "pacifist" monsters don't attack */
		if(noattacks(youmonst.data)) {
/*JP
			You("have no way to attack monsters physically.");
*/
			You("物理的に怪物を攻撃するすべがない．");
			mtmp->mstrategy &= ~STRAT_WAITMASK;
			goto atk_done;
		}
	}

/*JP
	if(check_capacity("You cannot fight while so heavily loaded."))
*/
	if(check_capacity("あなたは物を持ちすぎており戦うことができない．"))
	    goto atk_done;

	if (u.twoweap && !can_twoweapon())
		untwoweapon();

	if(unweapon) {
	    unweapon = FALSE;
	    if(flags.verbose) {
		if(uwep)
/*JP
		    You("begin bashing monsters with your %s.",
*/
		    You("%sで怪物をなぐりつけた．",
			aobjnam(uwep, (char *)0));
		else if (tech_inuse(T_EVISCERATE))
/*JP
		    You("begin slashing monsters with your claws.");
*/
		    You("爪でモンスターをなで斬りにした．");
		else if (!cantwield(youmonst.data)) {
		    if (P_SKILL(P_MARTIAL_ARTS) >= P_EXPERT)
/*JP
			You("assume a martial arts stance.");
*/
			You("マーシャルアーツの構えをとった．");
#if 0 /*JP*/
		    else You("begin %sing monsters with your %s %s.",
			Role_if(PM_MONK) ? "strik" : "bash",
			uarmg ? "gloved" : "bare",	/* Del Lamb */
			makeplural(body_part(HAND)));
#else
		    else You("%s%sで怪物を%sつけた．",
			uarmg ? "グローブをはめた" : "",
			body_part(HAND),
			Role_if(PM_MONK) ? "打ち" : "なぐり");
#endif
	    }
	}
	}

	exercise(A_STR, TRUE);		/* you're exercising muscles */
	/* andrew@orca: prevent unlimited pick-axe attacks */
	u_wipe_engr(3);

	/* Is the "it died" check actually correct? */
	if(mdat->mlet == S_LEPRECHAUN && !mtmp->mfrozen && !mtmp->msleeping &&
	   !mtmp->mconf && mtmp->mcansee && !rn2(7) &&
	   (m_move(mtmp, 0) == 2 ||			    /* it died */
	   mtmp->mx != u.ux+u.dx || mtmp->my != u.uy+u.dy)) /* it moved */
		return(FALSE);

	tmp = find_roll_to_hit(mtmp);
	
	(void) hmonas(mtmp, tmp); /* hmonas handles all attacks now */
	
	/* berserk lycanthropes calm down after the enemy is dead */
	if (mtmp->mhp <= 0) repeat_hit = 0;
/*
	if (Upolyd)
		(void) hmonas(mtmp, tmp);
	else
		(void) hitum(mtmp, tmp, mhit, youmonst.data->mattk);
*/		
	mtmp->mstrategy &= ~STRAT_WAITMASK;

atk_done:
	/* see comment in attack_checks() */
	/* we only need to check for this if we did an attack_checks()
	 * and it returned 0 (it's okay to attack), and the monster didn't
	 * evade.
	 */
	if (flags.forcefight && mtmp->mhp > 0 && !canspotmon(mtmp) &&
	    !memory_is_invisible(u.ux+u.dx, u.uy+u.dy) &&
	    !(u.uswallow && mtmp == u.ustuck))
		map_invisible(u.ux+u.dx, u.uy+u.dy);

	return(TRUE);
}

STATIC_OVL boolean
known_hitum(mon, mattack, mhit, uattk)   /* returns TRUE if monster still lives */
register struct monst *mon;
int mattack;			/* Which weapons you attacked with -ALI */
register int *mhit;
struct attack *uattk;
{
	register boolean malive = TRUE;

	if (override_confirmation) {
	    /* this may need to be generalized if weapons other than
	       Stormbringer acquire similar anti-social behavior... */
	    if (flags.verbose)
		if (override_confirmation == HIT_UWEP)
/*JP
		    Your("bloodthirsty blade attacks!");
*/
		    Your("武器は血に飢えている！");
		else
/*JP
		    pline("The black blade will not be thwarted!");
*/
		    pline("誰にも黒の刃を妨げることはできない！");
	}

	if(!*mhit) {
	    if (mattack & HIT_UWEP)
		missum(mon, tohit(UWEP_ROLL), dice(UWEP_ROLL), uattk);
	    if (mattack & HIT_USWAPWEP)
	    	missum(mon, tohit(USWAPWEP_ROLL), dice(USWAPWEP_ROLL), uattk);
	} else {
	    int oldhp = mon->mhp,
		x = u.ux + u.dx, y = u.uy + u.dy;

	    /* we hit the monster; be careful: it might die or
	       be knocked into a different location */
	    notonhead = (mon->mx != x || mon->my != y);
	    if (*mhit & HIT_UWEP) {
		/* KMH, conduct */
		if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
		    u.uconduct.weaphit++;
		dieroll = dice(UWEP_ROLL);
		malive = hmon(mon, uwep, 0);
	    } else if (mattack & HIT_UWEP)
		missum(mon, tohit(UWEP_ROLL), dice(UWEP_ROLL), uattk);
	    if ((mattack & HIT_USWAPWEP) && malive && m_at(x, y) == mon) {
		/* KMH, ethics */
	    	if (*mhit & HIT_USWAPWEP) {
		    if (uswapwep) u.uconduct.weaphit++;
		    dieroll = dice(USWAPWEP_ROLL);
		    malive = hmon(mon, uswapwep, 0);
	    	} else
		    missum(mon, tohit(USWAPWEP_ROLL), dice(USWAPWEP_ROLL), uattk);
	    }
	    if (malive) {
		/* monster still alive */
		if(!rn2(25) && mon->mhp < mon->mhpmax/2
			    && !(u.uswallow && mon == u.ustuck)) {
		    /* maybe should regurgitate if swallowed? */
		    if(!rn2(3)) {
			monflee(mon, rnd(100), FALSE, TRUE);
		    } else monflee(mon, 0, FALSE, TRUE);

		    if(u.ustuck == mon && !u.uswallow && !sticks(youmonst.data))
			setustuck(0);
		}
		/* Vorpal Blade hit converted to miss */
		/* could be headless monster or worm tail */
		if (mon->mhp == oldhp) {
		    *mhit = 0;
		    /* a miss does not break conduct */
		    if (uwep &&
			(uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
			--u.uconduct.weaphit;
		}
		if (mon->wormno && *mhit) {
		    int dohit = *mhit;
		    if (!u.twoweap || (dohit & HIT_UWEP)) {
			if (cutworm(mon, x, y, uwep))
			    dohit = 0;	/* Don't try and cut a worm twice */
		    }
		    if (u.twoweap && (dohit & HIT_USWAPWEP))
			(void) cutworm(mon, x, y, uswapwep);
		}
	    }

	    /* Lycanthropes sometimes go a little berserk! 
	     * If special is on,  they will multihit and stun!
	     */
	    if ((Race_if(PM_HUMAN_WEREWOLF) && (mon->mhp > 0)) ||
				tech_inuse(T_EVISCERATE)) {
		if (tech_inuse(T_EVISCERATE)) {
		    /*make slashing message elsewhere*/
		    if (repeat_hit == 0) {
			/* [max] limit to 4 (0-3) */
			repeat_hit = (tech_inuse(T_EVISCERATE) > 5) ?
						4 : (tech_inuse(T_EVISCERATE) - 2);
			/* [max] limit to 4 */
			mon->mfrozen = (tech_inuse(T_EVISCERATE) > 5) ?
						4 : (tech_inuse(T_EVISCERATE) - 2); 
		    }
		    mon->mstun = 1;
		    mon->mcanmove = 0;
		} else if (!rn2(24)) {
		    repeat_hit += rn2(4)+1;
		    /* Length of growl depends on how angry you get */
		    switch (repeat_hit) {
		    	case 0: /* This shouldn't be possible, but... */
#if 0 /*JP*/
			case 1: pline("Grrrrr!"); break;
			case 2: pline("Rarrrgh!"); break;
			case 3: pline("Grrarrgh!"); break;
			case 4: pline("Rarggrrgh!"); break;
			case 5: pline("Raaarrrrrr!"); break;
			case 6: 
			default:pline("Grrrrrrarrrrg!"); break;
#else
			case 1: pline("グルルルルルッ！"); break;
			case 2: pline("ガルルルルルルッ！"); break;
			case 3: pline("グルルァッルルッッ！"); break;
			case 4: pline("ガルァグルルググルァ！"); break;
			case 5: pline("ガァァッッルッルルルッ！"); break;
			case 6: 
			default:pline("グルルルルルルラルルルルッ！"); break;
#endif
		    }
		}
	    }
	}
	return(malive);
}

#if 0 /* Obsolete */
STATIC_OVL boolean
hitum(mon, tmp, mhit, uattk)          /* returns TRUE if monster still lives */
struct monst *mon;
int tmp;
int mhit;
struct attack *uattk;
{
	boolean malive;
	int mattack = mhit;
	int tmp1 = tmp, tmp2 = tmp;

	if (mhit & HIT_UWEP)
	{
		if (uwep) tmp1 += hitval(uwep, mon);
	
		tohit(UWEP_ROLL) = tmp1;
	
		if (tmp1 <= (dice(UWEP_ROLL) = rnd(20)) && !u.uswallow)
			mhit &= ~HIT_UWEP;

		if (tmp1 > dice(UWEP_ROLL)) exercise(A_DEX, TRUE);
#ifdef DEBUG
		pline("(%i/20)", tmp1);
#endif
	}
	
	if (mhit & HIT_USWAPWEP && u.twoweap) {
		if (uswapwep) tmp2 += hitval(uswapwep, mon) - 2;

		tohit(USWAPWEP_ROLL) = tmp2;

		if (tmp2 <= (dice(USWAPWEP_ROLL) = rnd(20)) && !u.uswallow)
			mhit &= ~HIT_USWAPWEP;

		if (tmp2 > dice(USWAPWEP_ROLL)) exercise(A_DEX, TRUE);
#ifdef DEBUG
		pline("((%i/20))", tmp2);
#endif
	}
	
	malive = known_hitum(mon, mattack, &mhit, uattk);
	(void) passive(mon, mhit, malive, AT_WEAP);
	/* berserk lycanthropes calm down after the enemy is dead */
	if (!malive) repeat_hit = 0;
	return(malive);
}
#endif

/* WAC Seperate martial arts damage function */
int
martial_dmg()
{
        int damage;
        /* WAC   plateau at 16 if Monk and Grand Master (6d4)
                            13 if Grand Master
                            11 if Master
                             9 if Expert
                             7 if Skilled
                             5 if Basic  (1d4)
         */

        if ((Role_if(PM_MONK) && !Upolyd)
                && (P_SKILL(P_MARTIAL_ARTS) == P_GRAND_MASTER)
                && (u.ulevel > 16)) damage = d(6,2);                                
        else if (u.ulevel > (2*(P_SKILL(P_MARTIAL_ARTS) - P_BASIC) + 5))
                damage = d((int) (P_SKILL(P_MARTIAL_ARTS) - P_UNSKILLED),2);
        else
                damage = d((int) ((u.ulevel+2)/3),2);

        if((!uarm || (uarm && (uarm->otyp >= ROBE &&
            uarm->otyp <= ROBE_OF_WEAKNESS))) && (!uarms))
                damage *= 2;
        else damage += 2;
        return (damage);
}

boolean			/* general "damage monster" routine */
hmon(mon, obj, thrown)		/* return TRUE if mon still alive */
struct monst *mon;
struct obj *obj;
int thrown;	/* 0: not thrown, 1: launched with uwep,
		   2: launched with uswapwep, 3: thrown by some other means */
{
	boolean result, anger_guards;

	anger_guards = (mon->mpeaceful &&
			    (mon->ispriest || mon->isshk ||
			     mon->data == &mons[PM_WATCHMAN] ||
			     mon->data == &mons[PM_WATCH_CAPTAIN]));
	result = hmon_hitmon(mon, obj, thrown);
	if (mon->ispriest && !rn2(2)) ghod_hitsu(mon);
	if (anger_guards) (void)angry_guards(!flags.soundok);
	return result;
}

/* guts of hmon() */
STATIC_OVL boolean
hmon_hitmon(mon, obj, thrown)
struct monst *mon;
struct obj *obj;
int thrown;
{
	int tmp, canhitmon = 0, objenchant;
	struct permonst *mdat = mon->data;
	int barehand_silver_rings = 0;
	/* The basic reason we need all these booleans is that we don't want
	 * a "hit" message when a monster dies, so we have to know how much
	 * damage it did _before_ outputting a hit message, but any messages
	 * associated with the damage don't come out until _after_ outputting
	 * a hit message.
	 */
	boolean hittxt = FALSE, destroyed = FALSE, already_killed = FALSE;
	boolean get_dmg_bonus = TRUE;
	boolean ispoisoned = FALSE, needpoismsg = FALSE, poiskilled = FALSE;
	boolean silvermsg = FALSE, silverobj = FALSE;
	boolean valid_weapon_attack = FALSE;
	boolean unarmed = !uwep && !uarm && !uarms;
#ifdef STEED
	int jousting = 0;
#endif
	boolean vapekilled = FALSE; /* WAC added boolean for vamps vaporize */
	boolean burnmsg = FALSE;
	boolean no_obj = !obj;	/* since !obj can change if weapon breaks, etc. */
	boolean noeffect;
	int wtype;
	struct obj *monwep;
	struct obj *launcher;
	char yourbuf[BUFSZ];
#if 0 /*JP*/
	char unconventional[BUFSZ];	/* substituted for word "attack" in msg */
#endif
	char saved_oname[BUFSZ];

	if (thrown == 1) launcher = uwep;
	else if (thrown == 2) launcher = uswapwep;
	else launcher = 0;

	objenchant = !thrown && no_obj || obj->spe < 0 ? 0 : obj->spe;

	if (need_one(mon))    canhitmon = 1;
	if (need_two(mon))    canhitmon = 2;
	if (need_three(mon))  canhitmon = 3;
	if (need_four(mon))   canhitmon = 4;

	/*
	 * If you are a creature that can hit as a +2 weapon, then YOU can
	 * hit as a +2 weapon. - SW
	 */

	if (Upolyd) {       /* Is Upolyd correct? */
	    /* a monster that needs a +1 weapon to hit it hits as a +1 weapon... */
	    if (need_one(&youmonst))		objenchant = 1;
	    if (need_two(&youmonst))		objenchant = 2;
	    if (need_three(&youmonst))		objenchant = 3;
	    if (need_four(&youmonst))		objenchant = 4;
	    /* overridden by specific flags */
	    if (hit_as_one(&youmonst))		objenchant = 1;
	    if (hit_as_two(&youmonst))		objenchant = 2;
	    if (hit_as_three(&youmonst))	objenchant = 3;
	    if (hit_as_four(&youmonst))		objenchant = 4;
	}

#if 0 /*JP*/
	unconventional[0] = '\0';
#endif
	saved_oname[0] = '\0';

	wakeup(mon);
	if(!thrown && no_obj) {      /* attack with bare hands */
	    if (Role_if(PM_MONK) && !Upolyd && u.ulevel/4 > objenchant)
		objenchant = u.ulevel/4;
	    noeffect = objenchant < canhitmon;
	    if (martial_bonus()) {
		if (mdat == &mons[PM_SHADE]) {
		    tmp = rn2(3);
		} else {
		    tmp = martial_dmg();
		}
	    } else {
	    if (mdat == &mons[PM_SHADE])
		tmp = 0;
		else tmp = rnd(2);
	    }

	    valid_weapon_attack = (tmp > 1);

	    /* blessed gloves give bonuses when fighting 'bare-handed' */
	    if (uarmg && uarmg->blessed && (is_undead(mdat) || is_demon(mdat)))
		tmp += rnd(4);
	    
	    if (uarmg && uarmg->spe) tmp += uarmg->spe; /* WAC plusses from gloves */

	    /* So do silver rings.  Note: rings are worn under gloves, so you
	     * don't get both bonuses.
	     */
	    if (!uarmg) {
		if (uleft && objects[uleft->otyp].oc_material == SILVER)
		    barehand_silver_rings++;
		if (uright && objects[uright->otyp].oc_material == SILVER)
		    barehand_silver_rings++;
		if (barehand_silver_rings && hates_silver(mdat)) {
		    tmp += rnd(20);
		    silvermsg = TRUE;
		}
	    }

	    /* WAC - Hand-to-Hand Combat Techniques */

	    if ((tech_inuse(T_CHI_STRIKE))  && (u.uen > 0)) {
#if 0 /*JP*/
		You("feel a surge of force.");
#else
		You("力の高まりを感じた．");
#endif
		tmp += (u.uen > (10 + (u.ulevel / 5)) ? 
			 (10 + (u.ulevel / 5)) : u.uen);
		u.uen -= (10 + (u.ulevel / 5));
		if (u.uen < 0) u.uen = 0;
	    }
	    
	    if (tech_inuse(T_E_FIST)) {
	    	int dmgbonus = 0;
		hittxt = TRUE;
		dmgbonus = noeffect ? 0 : d(2,4);
		switch (rn2(4)) {
		    case 0: /* Fire */
/*JP
			if (!Blind) pline("%s is on fire!", Monnam(mon));
*/
			if (!Blind) pline("%sは燃えた！", Monnam(mon));
			dmgbonus += destroy_mitem(mon, SCROLL_CLASS, AD_FIRE);
			dmgbonus += destroy_mitem(mon, SPBOOK_CLASS, AD_FIRE);
			if (noeffect || resists_fire(mon)) {
			    if (!noeffect)
				shieldeff(mon->mx, mon->my);
			    if (!Blind) 
/*JP
				pline_The("fire doesn't heat %s!", mon_nam(mon));
*/
				pline_The("炎は%sに影響がない！", mon_nam(mon));
			    golemeffects(mon, AD_FIRE, dmgbonus);
			    if (!noeffect)
				dmgbonus = 0;
			    else
				noeffect = 0;
			}
			/* only potions damage resistant players in destroy_item */
			dmgbonus += destroy_mitem(mon, POTION_CLASS, AD_FIRE);
			break;
		    case 1: /* Cold */
/*JP
		    	if (!Blind) pline("%s is covered in frost!", Monnam(mon));
*/
		    	if (!Blind) pline("%sは氷で覆われた！", Monnam(mon));
			if (noeffect || resists_cold(mon)) {
			    if (!noeffect)
				shieldeff(mon->mx, mon->my);
			    if (!Blind) 
/*JP
				pline_The("frost doesn't chill %s!", mon_nam(mon));
*/
				pline_The("氷は%sを凍らせることができない！", mon_nam(mon));
			    golemeffects(mon, AD_COLD, dmgbonus);
			    dmgbonus = 0;
			    noeffect = 0;
			}
			dmgbonus += destroy_mitem(mon, POTION_CLASS, AD_COLD);
			break;
		    case 2: /* Elec */
/*JP
			if (!Blind) pline("%s is zapped!", Monnam(mon));
*/
			if (!Blind) pline("%sは電撃をくらった！", Monnam(mon));
			dmgbonus += destroy_mitem(mon, WAND_CLASS, AD_ELEC);
			if (noeffect || resists_elec(mon)) {
			    if (!noeffect)
				shieldeff(mon->mx, mon->my);
			    if (!Blind)
/*JP
				pline_The("zap doesn't shock %s!", mon_nam(mon));
*/
				pline_The("電撃は%sに影響を与えない！", mon_nam(mon));
			    golemeffects(mon, AD_ELEC, dmgbonus);
			    if (!noeffect)
				dmgbonus = 0;
			    else
				noeffect = 0;
			}
			/* only rings damage resistant players in destroy_item */
			dmgbonus += destroy_mitem(mon, RING_CLASS, AD_ELEC);
			break;
		    case 3: /* Acid */
			if (!Blind)
/*JP
			    pline("%s is covered in acid!", Monnam(mon));
*/
			    pline("%sは酸を浴びた！", Monnam(mon));
			if (noeffect || resists_acid(mon)) {
			    if (!Blind)
/*JP
				pline_The("acid doesn't burn %s!", Monnam(mon));
*/
				pline_The("酸は%sを侵すことができない！", Monnam(mon));
			    dmgbonus = 0;
			    noeffect = 0;
			}
			break;
		}
		if (dmgbonus > 0)
		    tmp += dmgbonus;
	    } /* Techinuse Elemental Fist */		

	} else {
	    if (obj->oartifact == ART_MAGICBANE) objenchant = 4;
	    else if (obj->oartifact) objenchant += 2;

#ifdef LIGHTSABERS
	    if (is_lightsaber(obj)) objenchant = 4;
#endif

	    if (is_poisonable(obj) && obj->opoisoned)
		ispoisoned = TRUE;

	    noeffect = objenchant < canhitmon && !ispoisoned;

	    Strcpy(saved_oname, cxname(obj));
	    if(obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
	       obj->oclass == GEM_CLASS) {

		/* is it not a melee weapon? */
		/* KMH, balance patch -- new macros */
		if (/* if you strike with a bow... */
		    is_launcher(obj) ||
		    /* or strike with a missile in your hand... */
		    (!thrown && (is_missile(obj) || is_ammo(obj))) ||
		    /* or use a pole at short range and not mounted... */
		    (!thrown &&
#ifdef STEED
		     !u.usteed &&
#endif
		     is_pole(obj)) ||
#ifdef LIGHTSABERS
		    /* lightsaber that isn't lit ;) */
		    (is_lightsaber(obj) && !obj->lamplit) ||
#endif
		    /* or throw a missile without the proper bow... */
		    (thrown == 1 && is_ammo(obj) && 
		    	!ammo_and_launcher(obj, launcher)) || 
		    /* This case isn't actually needed so far since 
		     * you can only throw in two-weapon mode when both
		     * launchers take the same ammo
		     */
		    (thrown == 2 && is_ammo(obj) && 
		    	!ammo_and_launcher(obj, launcher))) {
		    /* then do only 1-2 points of damage */
		    if (mdat == &mons[PM_SHADE] && obj->otyp != SILVER_ARROW)
			tmp = 0;
		    else
			tmp = rnd(2);
		    if (!thrown && (obj == uwep || obj == uswapwep) && 
				obj->otyp == BOOMERANG && !rnl(4) == 4-1) {
			boolean more_than_1 = (obj->quan > 1L);

#if 0 /*JP*/
			pline("As you hit %s, %s%s %s breaks into splinters.",
			      mon_nam(mon), more_than_1 ? "one of " : "",
			      shk_your(yourbuf, obj), xname(obj));
#else
			pline("%sを攻撃すると，%s%s%sはこっぱみじんになった．",
			      mon_nam(mon),
			      shk_your(yourbuf, obj), xname(obj),
			       more_than_1 ? "のひとつ" : "");
#endif
			if (!more_than_1) {
			    if (obj == uwep)
				uwepgone();   /* set unweapon */
			    else
				setuswapwep((struct obj *)0, FALSE);
			}
			useup(obj);
			if (!more_than_1) obj = (struct obj *) 0;
			hittxt = TRUE;
			if (mdat != &mons[PM_SHADE])
			    tmp++;
		   }
		} else {
		    tmp = dmgval(obj, mon);
		    /* a minimal hit doesn't exercise proficiency */
		    valid_weapon_attack = (tmp > 1);
#if 0
		    if (!valid_weapon_attack || mon == u.ustuck || u.twoweap) {
#endif
		    if (!valid_weapon_attack || mon == u.ustuck) {
			;	/* no special bonuses */
		    } else if (mon->mflee && Role_if(PM_ROGUE) && !Upolyd) {
/*JP
			You("strike %s from behind!", mon_nam(mon));
*/
			You("%sを背後から攻撃した！", mon_nam(mon));
			tmp += rnd(u.ulevel);
			hittxt = TRUE;
		    } else if (dieroll == 2 && obj == uwep &&
			  !u.twoweap &&
			  obj->oclass == WEAPON_CLASS &&
			  (bimanual(obj) ||
			    (Role_if(PM_SAMURAI) && obj->otyp == KATANA && !uarms)) &&
			  ((wtype = uwep_skill_type()) != P_NONE &&
			    P_SKILL(wtype) >= P_SKILLED) &&
			  ((monwep = MON_WEP(mon)) != 0 &&
			   !is_flimsy(monwep) &&
			   !obj_resists(monwep,
				 50 + 15 * greatest_erosion(obj), 100))) {
			/*
			 * 2.5% chance of shattering defender's weapon when
			 * using a two-handed weapon; less if uwep is rusted.
			 * [dieroll == 2 is most successful non-beheading or
			 * -bisecting hit, in case of special artifact damage;
			 * the percentage chance is (1/20)*(50/100).]
			 * WAC.	Bimanual, or samurai and Katana without shield.
			 *	No twoweapon.
			 */
			setmnotwielded(mon,monwep);
			MON_NOWEP(mon);
			mon->weapon_check = NEED_WEAPON;
#if 0 /*JP*/
			pline("%s %s %s from the force of your blow!",
			      s_suffix(Monnam(mon)), xname(monwep),
			      otense(monwep, "shatter"));
#else
			pline("%sの%sはあなたの一撃で粉々になった！",
			      Monnam(mon), xname(monwep));
#endif
			m_useup(mon, monwep);
			/* If someone just shattered MY weapon, I'd flee! */
			if (rn2(4)) {
			    monflee(mon, d(2,3), TRUE, TRUE);
			}
			hittxt = TRUE;
		    }

		    if (obj->oartifact &&
			artifact_hit(&youmonst, mon, obj, &tmp, dieroll)) {
			if(mon->mhp <= 0) /* artifact killed monster */
			    return FALSE;
			if (tmp == 0) return TRUE;
			hittxt = TRUE;
		    }
		    if (objects[obj->otyp].oc_material == SILVER
				&& hates_silver(mdat)) {
			silvermsg = TRUE; silverobj = TRUE;
		    }
#ifdef STEED
		    if (u.usteed && !thrown && tmp > 0 &&
			    weapon_type(obj) == P_LANCE && mon != u.ustuck) {
			jousting = joust(mon, obj);
			/* exercise skill even for minimal damage hits */
			if (jousting) valid_weapon_attack = TRUE;
		    }
#endif
		    if (thrown && (is_ammo(obj) || is_missile(obj))) {
#ifdef P_SPOON
			if (obj->oartifact == ART_HOUCHOU) {
/*JP
			    pline("There is a bright flash as it hits %s.",
*/
			    pline("%sに当たるとギラリと光った．",
				the(mon_nam(mon)));
			    tmp = dmgval(obj, mon);
			}
#endif /* P_SPOON */
			if (ammo_and_launcher(obj, launcher)) {
			    if (launcher->oartifact)
				tmp += spec_dbon(launcher, mon, tmp);
			    /* Elves and Samurai do extra damage using
			     * their bows&arrows; they're highly trained.
			     * WAC Only elves get dmg bonus from flurry. Change?
			     */
			    if (Role_if(PM_SAMURAI) &&
				    obj->otyp == YA && launcher->otyp == YUMI)
				tmp++;
			    else if (Race_if(PM_ELF)) {
				if (obj->otyp == ELVEN_ARROW &&
					launcher->otyp == ELVEN_BOW) {
				tmp++;
				    /* WAC Extra damage if in special ability*/
				    if (tech_inuse(T_FLURRY)) tmp += 2;
				} else if (objects[obj->otyp].oc_skill == P_BOW
					&& tech_inuse(T_FLURRY)) {
				tmp++;
				}
			    } else if (Race_if(PM_DROW)) {
				if (obj->otyp == DARK_ELVEN_ARROW &&
					launcher->otyp == DARK_ELVEN_BOW) {
				    tmp += 2;
				    /* WAC Mucho damage if in special ability*/
				    if (tech_inuse(T_FLURRY)) tmp *= 2;
				} else if (objects[obj->otyp].oc_skill == P_BOW
					&& tech_inuse(T_FLURRY)) {
				    tmp++;
				}
			    }
			}
		    }
		    /* MRKR: Hitting with a lit torch does extra */
		    /*       fire damage, but uses up the torch  */
		    /*       more quickly.                       */

		    if(obj->otyp == TORCH && obj->lamplit
		       && !resists_fire(mon)) {

		      burnmsg = TRUE;

		      tmp++;
		      if (resists_cold(mon)) tmp += rnd(3);

		      /* Additional damage due to burning armor */
		      /* & equipment is delayed to below, after */
		      /* the hit messages are printed. */
		    }
		}
	    } else if(obj->oclass == POTION_CLASS) {
		if (!u.twoweap || obj == uwep) {
		if (obj->quan > 1L)
		    obj = splitobj(obj, 1L);
		else
		    setuwep((struct obj *)0, FALSE);
		} else if (u.twoweap && obj == uswapwep) {
		    if (obj->quan > 1L)
			setworn(splitobj(obj, 1L), W_SWAPWEP);
		    else
			setuswapwep((struct obj *)0, FALSE);
		}
		freeinv(obj);
		potionhit(mon, obj, TRUE);
		if (mon->mhp <= 0) return FALSE;	/* killed */
		hittxt = TRUE;
		/* in case potion effect causes transformation */
		mdat = mon->data;
		tmp = (mdat == &mons[PM_SHADE]) ? 0 : 1;
	    } else {
		if (mdat == &mons[PM_SHADE] && !shade_aware(obj)) {
		    tmp = 0;
#if 0 /*JP*/
		    Strcpy(unconventional, cxname(obj));
#endif
		} else {
		switch(obj->otyp) {
		    case BOULDER:		/* 1d20 */
		    case HEAVY_IRON_BALL:	/* 1d25 */
		    case IRON_CHAIN:		/* 1d4+1 */
			tmp = dmgval(obj, mon);
			break;
		    case MIRROR:
			if (breaktest(obj)) {
/*JP
			    You("break %s mirror.  That's bad luck!",
*/
			    You("%s鏡を割ってしまった．なんて縁起の悪い！",
				shk_your(yourbuf, obj));
			    change_luck(-2);
			    useup(obj);
			    obj = (struct obj *) 0;
			    unarmed = FALSE;	/* avoid obj==0 confusion */
			    get_dmg_bonus = FALSE;
			    hittxt = TRUE;
			}
			tmp = 1;
			break;
#ifdef TOURIST
		    case EXPENSIVE_CAMERA:
/*JP
			You("succeed in destroying %s camera.  Congratulations!",
*/
			You("%sカメラを壊すことに成功した．おめでとう！",
			    shk_your(yourbuf, obj));
			useup(obj);
			return(TRUE);
			/*NOTREACHED*/
			break;
#endif
		    case CORPSE:		/* fixed by polder@cs.vu.nl */
			if (touch_petrifies(&mons[obj->corpsenm])) {
#if 0 /*JP*/
			    static const char withwhat[] = "corpse";
#endif
			    tmp = 1;
			    hittxt = TRUE;
#if 0 /*JP*/
			    You("hit %s with %s %s.", mon_nam(mon),
				obj->dknown ? the(mons[obj->corpsenm].mname) :
				an(mons[obj->corpsenm].mname),
				(obj->quan > 1) ? makeplural(withwhat) : withwhat);
#else
			    You("%sを%sの死体で攻撃した．", mon_nam(mon),
				jtrns_mon(mons[obj->corpsenm].mname));
#endif
			    if (!munstone(mon, TRUE))
				minstapetrify(mon, TRUE);
			    if (resists_ston(mon)) break;
			    /* note: hp may be <= 0 even if munstoned==TRUE */
			    return (boolean) (mon->mhp > 0);
#if 0
			} else if (touch_petrifies(mdat)) {
			    /* maybe turn the corpse into a statue? */
#endif
			}
			tmp = (obj->corpsenm >= LOW_PM ?
					mons[obj->corpsenm].msize : 0) + 1;
			break;
		    case EGG:
		      {
#define useup_eggs(o)	{ if (thrown) obfree(o,(struct obj *)0); \
			  else useupall(o); \
			  o = (struct obj *)0; }	/* now gone */
			long cnt = obj->quan;

			tmp = 1;		/* nominal physical damage */
			get_dmg_bonus = FALSE;
			hittxt = TRUE;		/* message always given */
			/* egg is always either used up or transformed, so next
			   hand-to-hand attack should yield a "bashing" mesg */
			if (obj == uwep) unweapon = TRUE;
			if (obj->spe && obj->corpsenm >= LOW_PM) {
			    if (obj->quan < 5)
				change_luck((schar) -(obj->quan));
			    else
				change_luck(-5);
			}

			if (touch_petrifies(&mons[obj->corpsenm])) {
			    /*learn_egg_type(obj->corpsenm);*/
#if 0 /*JP*/
			    pline("Splat! You hit %s with %s %s egg%s!",
				mon_nam(mon),
				obj->known ? "the" : cnt > 1L ? "some" : "a",
				obj->known ? mons[obj->corpsenm].mname : "petrifying",
				plur(cnt));
#else
			    pline("ビチャッ！あなたは%sに%s%sの卵を命中させた！",
				mon_nam(mon), cnt > 1L ? "複数の" : "",
				obj->known ? jtrns_mon(mons[obj->corpsenm].mname) : "石化");
#endif
#if 0
			    obj->known = 1;	/* (not much point...) */
#endif
			    useup_eggs(obj);
			    if (!munstone(mon, TRUE))
				minstapetrify(mon, TRUE);
			    if (resists_ston(mon)) break;
			    return (boolean) (mon->mhp > 0);
			} else {	/* ordinary egg(s) */
			    const char *eggp =
#if 0 /*JP*/
				     (obj->corpsenm != NON_PM && obj->known) ?
					      the(mons[obj->corpsenm].mname) :
					      (cnt > 1L) ? "some" : "an";
#else
				     (obj->corpsenm != NON_PM && obj->known) ?
					      jtrns_mon(mons[obj->corpsenm].mname) : "";
#endif
#if 0 /*JP*/
			    You("hit %s with %s egg%s.",
				mon_nam(mon), eggp, plur(cnt));
#else
			    You("%sに%s%s%s卵を命中させた．", mon_nam(mon),
				(cnt > 1L) ? "複数の" : "",
				eggp, *eggp ? "の" : "");
#endif
			    if (touch_petrifies(mdat) && !stale_egg(obj)) {
#if 0 /*JP*/
				pline_The("egg%s %s alive any more...",
				      plur(cnt),
				      (cnt == 1L) ? "isn't" : "aren't");
#else
				pline("もう卵が孵化することはないだろう．．．");
#endif
				if (obj->timed) obj_stop_timers(obj);
				obj->otyp = ROCK;
				obj->oclass = GEM_CLASS;
				obj->oartifact = 0;
				obj->spe = 0;
				obj->known = obj->dknown = obj->bknown = 0;
				obj->owt = weight(obj);
				if (thrown) place_object(obj, mon->mx, mon->my);
			    } else {
/*JP
				pline("Splat!");
*/
				pline("ビチャッ！");
				useup_eggs(obj);
				exercise(A_WIS, FALSE);
			    }
			}
			break;
#undef useup_eggs
		      }
		    case CLOVE_OF_GARLIC:	/* no effect against demons */
			if (is_undead(mdat)) {
			    monflee(mon, d(2, 4), FALSE, TRUE);
			}
			tmp = 1;
			break;
		    case CREAM_PIE:
		    case BLINDING_VENOM:
			mon->msleeping = 0;
			if (can_blnd(&youmonst, mon, (uchar)
				    (obj->otyp == BLINDING_VENOM
				     ? AT_SPIT : AT_WEAP), obj)) {
			    if (Blind) {
				pline(obj->otyp == CREAM_PIE ?
/*JP
				      "Splat!" : "Splash!");
*/
				      "ベチャッ！" : "ピチャッ！");
			    } else if (obj->otyp == BLINDING_VENOM) {
#if 0 /*JP*/
				pline_The("venom blinds %s%s!", mon_nam(mon),
					  mon->mcansee ? "" : " further");
#else
				pline("毒液が%sの目を%s潰した！", mon_nam(mon),
					mon->mcansee ? "" : "さらに");
#endif
			    } else {
				char *whom = mon_nam(mon);
				char *what = The(xname(obj));
				if (!thrown && obj->quan > 1)
				    what = An(singular(obj, xname));
				/* note: s_suffix returns a modifiable buffer */
				if (haseyes(mdat)
				    && mdat != &mons[PM_FLOATING_EYE])
#if 0 /*JP*/
				    whom = strcat(strcat(s_suffix(whom), " "),
						  mbodypart(mon, FACE));
#else
				    whom = strcat(strcat(whom, "の"),
						  mbodypart(mon, FACE));
#endif
#if 0 /*JP*/
				pline("%s %s over %s!",
				      what, vtense(what, "splash"), whom);
#else
				pline("%sは%sにぶちまけられた！",
				      what, whom);
#endif
			    }
			    setmangry(mon);
			    mon->mcansee = 0;
			    tmp = rn1(25, 21);
			    if(((int) mon->mblinded + tmp) > 127)
				mon->mblinded = 127;
			    else mon->mblinded += tmp;
			} else {
/*JP
			    pline(obj->otyp==CREAM_PIE ? "Splat!" : "Splash!");
*/
			    pline(obj->otyp==CREAM_PIE ? "ベチャッ！" : "ピチャッ！");
			    setmangry(mon);
			}
			if (thrown) obfree(obj, (struct obj *)0);
			else useup(obj);
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			tmp = 0;
			break;
		    case ACID_VENOM: /* thrown (or spit) */
			if (resists_acid(mon)) {
/*JP
				Your("venom hits %s harmlessly.",
*/
				pline("毒液は%sに命中したが効果がなかった．",
					mon_nam(mon));
				tmp = 0;
			} else {
/*JP
				Your("venom burns %s!", mon_nam(mon));
*/
				Your("毒液が%sを灼いた！", mon_nam(mon));
				tmp = dmgval(obj, mon);
			}
			if (thrown) obfree(obj, (struct obj *)0);
			else useup(obj);
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			break;
		    default:
			/* non-weapons can damage because of their weight */
			/* (but not too much) */
			tmp = obj->owt/100;
			if(tmp < 1) tmp = 1;
			else tmp = rnd(tmp);
			if(tmp > 6) tmp = 6;
			/*
			 * Things like silver wands can arrive here so
			 * so we need another silver check.
			 */
			if (objects[obj->otyp].oc_material == SILVER
						&& hates_silver(mdat)) {
				tmp += rnd(20);
				silvermsg = TRUE; silverobj = TRUE;
			}
		    }
		}
	    }
	}

	/****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG)
	 *      *OR* if attacking bare-handed!! */

	if (get_dmg_bonus && tmp > 0) {
		tmp += u.udaminc;
		/* If you throw using a propellor, you don't get a strength
		 * bonus but you do get an increase-damage bonus.
		 */
		if(!thrown || !obj || !uwep || !ammo_and_launcher(obj, launcher))
		    tmp += dbon();
	}

	/*
	 * Ki special ability, see cmd.c in function special_ability.
	 * In this case, we do twice damage! Wow!
	 *
	 * Berserk special ability only does +4 damage. - SW
	 */
	/*Lycanthrope claws do +level bare hands dmg
                (multi-hit, stun/freeze)..- WAC*/

	if (tech_inuse(T_KIII)) tmp *= 2;
	if (tech_inuse(T_BERSERK)) tmp += 4;
	if (tech_inuse(T_EVISCERATE)) {
		tmp += rnd((int) (u.ulevel/2 + 1)) + (u.ulevel/2); /* [max] was only + u.ulevel */
#if 0 /*JP*/
                You("slash %s!", mon_nam(mon));
#else
                You("%sを切り裂いた！", mon_nam(mon));
#endif
		hittxt = TRUE;
	}

	if (valid_weapon_attack) {
	    struct obj *wep;

	    /* to be valid a projectile must have had the correct projector */
	    wep = PROJECTILE(obj) ? launcher : obj;
	    tmp += weapon_dam_bonus(wep);
	    /* [this assumes that `!thrown' implies wielded...] */
	    wtype = weapon_type(wep);
	    if (thrown || !u.twoweap || !rn2(2)) use_skill(wtype, 1);
	    else if (u.twoweap) use_skill(P_TWO_WEAPON_COMBAT,1);
	}

	if (ispoisoned) {
	    int nopoison = (10 - (obj->owt/10));            
	    if(nopoison < 2) nopoison = 2;
	    if Role_if(PM_SAMURAI) {
/*JP
		You("dishonorably use a poisoned weapon!");
*/
		You("恥ずべきことに毒の武器を使用した！");
		adjalign(-sgn(u.ualign.type));
	    } else if ((u.ualign.type == A_LAWFUL) && (u.ualign.record > -10)) {
/*JP
		You_feel("like an evil coward for using a poisoned weapon.");
*/
		You("毒の武器を使用するのは卑怯で邪悪だと感じた．");
		adjalign(-1);
	    }
	    if (obj && !rn2(nopoison)) {
		obj->opoisoned = FALSE;
#if 0 /*JP*/
		Your("%s %s no longer poisoned.", xname(obj),
		     otense(obj, "are"));
#else
		Your("%sにはもう毒が塗られていない．", xname(obj));
#endif
	    }
	    if (resists_poison(mon))
		needpoismsg = TRUE;
	    else if (rn2(10))
		tmp += rnd(6);
	    else poiskilled = TRUE;
	}
	  
	if (tmp < 1) {
	    /* make sure that negative damage adjustment can't result
	       in inadvertently boosting the victim's hit points */
	    tmp = 0;
	    if (mdat == &mons[PM_SHADE]) {
		if (!hittxt) {
#if 0 /*JP*/
		    const char *what = unconventional[0] ? unconventional : "attack";
		    Your("%s %s harmlessly through %s.",
		    	what, vtense(what, "pass"),
			mon_nam(mon));
#else
		    Your("攻撃は%sをすり抜けた．", mon_nam(mon));
#endif
		    hittxt = TRUE;
		}
	    } else {
		if (get_dmg_bonus) tmp = 1;
	    }
	}

#ifdef STEED
	if (jousting) {
	    tmp += d(2, (obj == uwep) ? 10 : 2);        /* [was in dmgval()] */
#if 0 /*JP*/
	    You("joust %s%s",
			 mon_nam(mon), canseemon(mon) ? exclam(tmp) : ".");
#else
	    You("%sに突撃した%s",
			 mon_nam(mon), canseemon(mon) ? exclam(tmp) : "．");
#endif
	    if (jousting < 0) {
/*JP
		Your("%s shatters on impact!", xname(obj));
*/
		Your("%sは衝撃で砕け散った！", xname(obj));
		/* (must be either primary or secondary weapon to get here) */
		u.twoweap = FALSE;      /* untwoweapon() is too verbose here */
		if (obj == uwep) uwepgone();            /* set unweapon */
		/* minor side-effect: broken lance won't split puddings */
		useup(obj);
		obj = 0;
	    }
	    /* avoid migrating a dead monster */
	    if (mon->mhp > tmp) {
		mhurtle(mon, u.dx, u.dy, 1);
		mdat = mon->data; /* in case of a polymorph trap */
		if (DEADMONSTER(mon)) already_killed = TRUE;
	    }
	    hittxt = TRUE;
	} else
#endif

	/* VERY small chance of stunning opponent if unarmed. */
	if (unarmed && tmp > 1 && !thrown && !obj && !Upolyd) {
	    if (rnd(100) < P_SKILL(P_BARE_HANDED_COMBAT) &&
			!bigmonst(mdat) && !thick_skinned(mdat)) {
		if (canspotmon(mon))
#if 0 /*JP*/
		    pline("%s %s from your powerful strike!", Monnam(mon),
			  makeplural(stagger(mon->data, "stagger")));
#else
		    pline("%sはあなたの会心の一撃で%s！",
			  Monnam(mon), stagger(mon->data, "よろめいた"));
#endif
		/* avoid migrating a dead monster */
		if (mon->mhp > tmp) {
		    mhurtle(mon, u.dx, u.dy, 1);
		    mdat = mon->data; /* in case of a polymorph trap */
		    if (DEADMONSTER(mon)) already_killed = TRUE;
		}
		hittxt = TRUE;
	    }
	}

	if (tmp && noeffect) {
	    if (silvermsg)
		tmp = 8;
	    else {
#if 0 /*JP*/
		Your("attack doesn't seem to harm %s.", mon_nam(mon));
#else
		Your("攻撃は%sにダメージを与えられなかったようだ．", mon_nam(mon));
#endif
		hittxt = TRUE;
		tmp = 0;
	    }
	}

        /* WAC Added instant kill from wooden stakes vs vampire */
        /* based off Poison Code */
        /* fixed stupid mistake - check that obj exists before comparing...*/
        if (obj && obj->otyp == WOODEN_STAKE && is_vampire(mdat)) {
            if (Role_if(PM_UNDEAD_SLAYER) 
              || (P_SKILL(weapon_type(obj)) >= P_EXPERT)
              || obj->oartifact == ART_STAKE_OF_VAN_HELSING) {
                if (!rn2(10)) {
/*JP
                    You("plunge your stake into the heart of %s.",
*/
                    You("%sの心臓に杭を突き刺した．",
                        mon_nam(mon));
                    vapekilled = TRUE;
                } else {
/*JP
                    You("drive your stake into %s.", mon_nam(mon));
*/
                    You("%sに杭を打ち込んだ．", mon_nam(mon));
                    tmp += rnd(6) + 2;
                    hittxt = TRUE;
                }
            } else {
/*JP
                You("drive your stake into %s.", mon_nam(mon));
*/
                You("%sに杭を打ち込んだ．", mon_nam(mon));
                tmp += rnd(6);
                hittxt = TRUE;
            }
        }

	/* Special monk strikes */
	if (Role_if(PM_MONK) && !Upolyd && !thrown && no_obj &&
		(!uarm || (uarm && uarm->otyp >= ROBE &&
		 uarm->otyp <= ROBE_OF_WEAKNESS)) && !uarms &&
		 distu(mon->mx, mon->my) <= 2) {
	    /* just so we don't need another variable ... */
	    canhitmon = rnd(100);
	    if (canhitmon < u.ulevel / 8 && !thick_skinned(mdat)) {
		if (canspotmon(mon))
/*JP
		    You("strike %s extremely hard!", mon_nam(mon));
*/
		    You("%sに極めて大きな打撃を与えた！", mon_nam(mon));
		tmp *= 2;
		hittxt = TRUE;
	    } else if (canhitmon < u.ulevel / 4 && !thick_skinned(mdat)) {
		if (canspotmon(mon))
/*JP
		    You("strike %s very hard!", mon_nam(mon));
*/
		    You("%sに極めて大きな打撃を与えた！", mon_nam(mon));
		tmp += tmp / 2;
		hittxt = TRUE;
	    } else if (canhitmon < u.ulevel / 2 && !bigmonst(mon->data) &&
		    !thick_skinned(mdat)) {
		if (canspotmon(mon))
#if 0 /*JP*/
		    pline("%s %s from your powerful strike!", Monnam(mon),
			  makeplural(stagger(mon->data, "stagger")));
#else
		    pline("%sはあなたの会心の一撃で%s！", Monnam(mon),
			  stagger(mon->data, "よろめいた"));
#endif
		/* avoid migrating a dead monster */
		if (mon->mhp > tmp) {
		    mhurtle(mon, u.dx, u.dy, 1);
		    mdat = mon->data; /* in case of a polymorph trap */
		    if (DEADMONSTER(mon)) already_killed = TRUE;
		}
		hittxt = TRUE;
	    }
	}

	if (!already_killed) mon->mhp -= tmp;
	/* adjustments might have made tmp become less than what
	   a level draining artifact has already done to max HP */
	if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
	if (mon->mhp < 1)
		destroyed = TRUE;
	/* fixed bug with hitting tame monster with non-magic weapon */        
	if (mon->mtame && (!mon->mflee || mon->mfleetim) && tmp > 0) {

		abuse_dog(mon);
		monflee(mon, 10 * rnd(tmp), FALSE, FALSE);
	}
	if((mdat == &mons[PM_BLACK_PUDDING] || mdat == &mons[PM_BROWN_PUDDING])
		   && obj /* && obj == uwep -- !thrown and obj == weapon */
		   && !thrown
		   && objects[obj->otyp].oc_material == IRON
		   && mon->mhp > 1 && !thrown && !mon->mcan
		   /* && !destroyed  -- guaranteed by mhp > 1 */ ) {
		if (clone_mon(mon, 0, 0)) {
/*JP
			pline("%s divides as you hit it!", Monnam(mon));
*/
			pline("あなたの攻撃で%sは分裂した！", Monnam(mon));
			hittxt = TRUE;
		}
	}

	if (!hittxt &&			/*( thrown => obj exists )*/
	  (!destroyed || (thrown && m_shot.n > 1 && m_shot.o == obj->otyp))) {
		if (thrown) hit(mshot_xname(obj), mon, exclam(tmp));
/*JP
		else if (!flags.verbose) You("hit it.");
*/
		else if(!flags.verbose) pline("攻撃は命中した．");
#if 0 /*JP*/
		else You("%s %s%s", Role_if(PM_BARBARIAN) ? "smite" : "hit",
			 mon_nam(mon), canseemon(mon) ? exclam(tmp) : ".");
#else
		else Your("%sへの%sは命中した%s", mon_nam(mon),
			Role_if(PM_BARBARIAN) ? "強打" : "攻撃",
			canseemon(mon) ? exclam(tmp) : "．");
#endif
	}

	if (burnmsg) {
	  /* A chance of setting the monster's */
	  /* armour + equipment on fire */
	  /* (this does not do any extra damage) */

	  if (!Blind) {
#if 0 /*JP*/
	    Your("%s %s %s.", xname(obj),
		 (mon->data == &mons[PM_WATER_ELEMENTAL]) ?
		 "vaporizes part of" : "burns", mon_nam(mon));
#else
	    Your("%sは%s%s．", xname(obj), mon_nam(mon),
		 (mon->data == &mons[PM_WATER_ELEMENTAL]) ?
		 "の一部を蒸発させた" : "を燃やした");
#endif
	  }

	  if (!rn2(2) && burnarmor(mon)) {
	    if (!rn2(3)) 
	      (void)destroy_mitem(mon, POTION_CLASS, AD_FIRE);
	    if (!rn2(3)) 
	      (void)destroy_mitem(mon, SCROLL_CLASS, AD_FIRE);
	    if (!rn2(5)) 
	      (void)destroy_mitem(mon, SPBOOK_CLASS, AD_FIRE);
	  }

	  if (mon->data == &mons[PM_WATER_ELEMENTAL]) {
	    if (!Blind) {
#if 0 /*JP*/
	      Your("%s goes out.", xname(obj));
#else
	      Your("%sの火が消えた．", xname(obj));
#endif
	    }
	    end_burn(obj, TRUE);
	  }
	  else {
	    /* use up the torch more quickly */	    
	    burn_faster(obj, 1);
	  }
	}
	
	if (silvermsg) {
		const char *fmt;
		char *whom = mon_nam(mon);
		char silverobjbuf[BUFSZ];

		if (canspotmon(mon)) {
		    if (barehand_silver_rings == 1)
/*JP
			fmt = "Your silver ring sears %s!";
*/
			fmt = "あなたの銀の指輪が%sを焼き焦がした！";
		    else if (barehand_silver_rings == 2)
/*JP
			fmt = "Your silver rings sear %s!";
*/
			fmt = "あなたの銀の指輪が%sを焼き焦がした！";
		    else if (silverobj && saved_oname[0]) {
#if 0 /*JP*/
		    	Sprintf(silverobjbuf, "Your %s%s %s %%s!",
		    		strstri(saved_oname, "silver") ?
					"" : "silver ",
				saved_oname, vtense(saved_oname, "sear"));
#else
		    	Sprintf(silverobjbuf, "%s%sが%%sを焼き焦がした！",
		    		saved_oname, 
		    		strstri(saved_oname, "銀") ? "" : "の銀");
#endif
		    	fmt = silverobjbuf;
		    } else
/*JP
			fmt = "The silver sears %s!";
*/
			fmt = "銀が%sを焼き焦がした！";
		} else {
		    *whom = highc(*whom);	/* "it" -> "It" */
/*JP
		    fmt = "%s is seared!";
*/
		    fmt = "%sは焼き焦げた！";
		}
		/* note: s_suffix returns a modifiable buffer */
		if (!noncorporeal(mdat))
/*JP
		    whom = strcat(s_suffix(whom), " flesh");
*/
		    whom = strcat(s_suffix(whom), "の肉体");
		pline(fmt, whom);
	}

	if (needpoismsg)
/*JP
		pline_The("poison doesn't seem to affect %s.", mon_nam(mon));
*/
		pline("毒は%sに効かなかったようだ．", mon_nam(mon));
	if (poiskilled) {
/*JP
		pline_The("poison was deadly...");
*/
		pline("毒は致死量だった．．．");
		if (!already_killed) xkilled(mon, 0);
		return FALSE;
/* For vamps */
        } else if (vapekilled) {
                if (cansee(mon->mx, mon->my))
#if 0 /*JP*/
                    pline("%s%ss body vaporizes!", Monnam(mon),
                            canseemon(mon) ? "'" : "");                
#else
                    pline("%sの肉体が蒸発した！", Monnam(mon));
#endif
                if (!already_killed) xkilled(mon, 2);
		return FALSE;
	} else if (destroyed) {
		if (!already_killed)
		    killed(mon);	/* takes care of most messages */
	} else if(u.umconf && !thrown) {
		nohandglow(mon);
		if (!mon->mconf && !resist(mon, SPBOOK_CLASS, 0, NOTELL)) {
			mon->mconf = 1;
			if (!mon->mstun && mon->mcanmove && !mon->msleeping &&
				canseemon(mon))
/*JP
			    pline("%s appears confused.", Monnam(mon));
*/
			    pline("%sは混乱しているようだ．", Monnam(mon));
		}
	}

#ifdef SHOW_DMG
	if (!destroyed) showdmg(tmp);
#endif
	return((boolean)(destroyed ? FALSE : TRUE));
}

STATIC_OVL boolean
shade_aware(obj)
struct obj *obj;
{
	if (!obj) return FALSE;
	/*
	 * The things in this list either
	 * 1) affect shades.
	 *  OR
	 * 2) are dealt with properly by other routines
	 *    when it comes to shades.
	 */
	if (obj->otyp == BOULDER || obj->otyp == HEAVY_IRON_BALL
	    || obj->otyp == IRON_CHAIN		/* dmgval handles those first three */
	    || obj->otyp == MIRROR		/* silver in the reflective surface */
	    || obj->otyp == CLOVE_OF_GARLIC	/* causes shades to flee */
	    || objects[obj->otyp].oc_material == SILVER)
		return TRUE;
	return FALSE;
}

/* check whether slippery clothing protects from hug or wrap attack */
/* [currently assumes that you are the attacker] */
STATIC_OVL boolean
m_slips_free(mdef, mattk)
struct monst *mdef;
struct attack *mattk;
{
	struct obj *obj;

	if (mattk->adtyp == AD_DRIN) {
	    /* intelligence drain attacks the head */
	    obj = which_armor(mdef, W_ARMH);
	} else {
	    /* grabbing attacks the body */
	    obj = which_armor(mdef, W_ARMC);		/* cloak */
	    if (!obj) obj = which_armor(mdef, W_ARM);	/* suit */
#ifdef TOURIST
	    if (!obj) obj = which_armor(mdef, W_ARMU);	/* shirt */
#endif
	}

	/* if your cloak/armor is greased, monster slips off; this
	   protection might fail (33% chance) when the armor is cursed */
	if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK) &&
		(!obj->cursed || rn2(3))) {
#if 0 /*JP*/
	    You("%s %s %s %s!",
		mattk->adtyp == AD_WRAP ?
			"slip off of" : "grab, but cannot hold onto",
		s_suffix(mon_nam(mdef)),
		obj->greased ? "greased" : "slippery",
		/* avoid "slippery slippery cloak"
		   for undiscovered oilskin cloak */
		(obj->greased || objects[obj->otyp].oc_name_known) ?
			xname(obj) : cloak_simple_name(obj));
#else
	    You("%s%sとしたが，%s%s%sで滑った！",
		mon_nam(mdef),
		mattk->adtyp == AD_WRAP ? "に巻きつこう" : "を掴もう",
		mhis(mdef), obj->greased ? "脂の塗られた" : "滑りやすい",
		(obj->greased || objects[obj->otyp].oc_name_known) ?
		xname(obj) : cloak_simple_name(obj));
#endif

	    if (obj->greased && !rn2(2)) {
/*JP
		pline_The("grease wears off.");
*/
		pline("脂は落ちてしまった．");
		obj->greased = 0;
	    }
	    return TRUE;
	}
	return FALSE;
}

/* used when hitting a monster with a lance while mounted */
STATIC_OVL int	/* 1: joust hit; 0: ordinary hit; -1: joust but break lance */
joust(mon, obj)
struct monst *mon;	/* target */
struct obj *obj;	/* weapon */
{
    int skill_rating, joust_dieroll;

    if (Fumbling || Stunned) return 0;
    /* sanity check; lance must be wielded in order to joust */
    if (obj != uwep && (obj != uswapwep || !u.twoweap)) return 0;

    /* if using two weapons, use worse of lance and two-weapon skills */
    skill_rating = P_SKILL(weapon_type(obj));	/* lance skill */
    if (u.twoweap && P_SKILL(P_TWO_WEAPON_COMBAT) < skill_rating)
	skill_rating = P_SKILL(P_TWO_WEAPON_COMBAT);
    if (skill_rating == P_ISRESTRICTED) skill_rating = P_UNSKILLED; /* 0=>1 */

    /* odds to joust are expert:80%, skilled:60%, basic:40%, unskilled:20% */
    if ((joust_dieroll = rn2(5)) < skill_rating) {
	if (joust_dieroll == 0 && rnl(50) == (50-1) &&
		!unsolid(mon->data) && !obj_resists(obj, 0, 100))
	    return -1;	/* hit that breaks lance */
	return 1;	/* successful joust */
    }
    return 0;	/* no joust bonus; revert to ordinary attack */
}

/*
 * Send in a demon pet for the hero.  Exercise wisdom.
 *
 * This function used to be inline to damageum(), but the Metrowerks compiler
 * (DR4 and DR4.5) screws up with an internal error 5 "Expression Too Complex."
 * Pulling it out makes it work.
 */
STATIC_OVL void
demonpet()
{
	int i;
	struct permonst *pm;
	struct monst *dtmp;

/*JP
	pline("Some hell-p has arrived!");
*/
	pline("地獄から援軍が現れた！");
	i = !rn2(6) ? ndemon(u.ualign.type) : NON_PM;
	pm = i != NON_PM ? &mons[i] : youmonst.data;
	if ((dtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS)) != 0)
	    (void)tamedog(dtmp, (struct obj *)0);
	exercise(A_WIS, TRUE);
}

/*
 * Player uses theft attack against monster.
 *
 * If the target is wearing body armor, take all of its possesions;
 * otherwise, take one object.  [Is this really the behavior we want?]
 *
 * This routine implicitly assumes that there is no way to be able to
 * resist petfication (ie, be polymorphed into a xorn or golem) at the
 * same time as being able to steal (poly'd into nymph or succubus).
 * If that ever changes, the check for touching a cockatrice corpse
 * will need to be smarter about whether to break out of the theft loop.
 */
STATIC_OVL void
steal_it(mdef, mattk)
struct monst *mdef;
struct attack *mattk;
{
	struct obj *otmp, *stealoid, **minvent_ptr;
	long unwornmask;

	if (!mdef->minvent) return;		/* nothing to take */

	/* look for worn body armor */
	stealoid = (struct obj *)0;
	if (could_seduce(&youmonst, mdef, mattk)) {
	    /* find armor, and move it to end of inventory in the process */
	    minvent_ptr = &mdef->minvent;
	    while ((otmp = *minvent_ptr) != 0)
		if (otmp->owornmask & W_ARM) {
		    if (stealoid) panic("steal_it: multiple worn suits");
		    *minvent_ptr = otmp->nobj;	/* take armor out of minvent */
		    stealoid = otmp;
		    stealoid->nobj = (struct obj *)0;
		} else {
		    minvent_ptr = &otmp->nobj;
		}
	    *minvent_ptr = stealoid;	/* put armor back into minvent */
	}

	if (stealoid) {		/* we will be taking everything */
	    if (gender(mdef) == (int) u.mfemale &&
			youmonst.data->mlet == S_NYMPH)
/*JP
		You("charm %s.  She gladly hands over her possessions.",
*/
		You("%sをうっとりさせた．彼女はよろこんで持ち物を差し出した．",
		    mon_nam(mdef));
	    else
#if 0 /*JP*/
		You("seduce %s and %s starts to take off %s clothes.",
		    mon_nam(mdef), mhe(mdef), mhis(mdef));
#else
		You("%sを誘惑した．%sは服を脱ぎはじめた．",
		    mon_nam(mdef), mhe(mdef));
#endif
	}

	while ((otmp = mdef->minvent) != 0) {
	    if (!Upolyd) break;		/* no longer have ability to steal */
	    /* take the object away from the monster */
	    obj_extract_self(otmp);
	    if ((unwornmask = otmp->owornmask) != 0L) {
		mdef->misc_worn_check &= ~unwornmask;
		if (otmp->owornmask & W_WEP) {
		    setmnotwielded(mdef,otmp);
		    MON_NOWEP(mdef);
		}
		otmp->owornmask = 0L;
		update_mon_intrinsics(mdef, otmp, FALSE, FALSE);

		if (otmp == stealoid)	/* special message for final item */
#if 0 /*JP*/
		    pline("%s finishes taking off %s suit.",
			  Monnam(mdef), mhis(mdef));
#else
		    pline("%sは脱ぎ終えた．", Monnam(mdef));
#endif
	    }
	    /* give the object to the character */
#if 0 /*JP*/
	    otmp = hold_another_object(otmp, "You snatched but dropped %s.",
				       doname(otmp), "You steal: ");
#else
	    otmp = hold_another_object(otmp, "あなたは%sを取り上げようとして落としてしまった．",
				       doname(otmp), "を盗んだ．");
#endif
	    if (otmp->where != OBJ_INVENT) continue;
	    if (otmp->otyp == CORPSE &&
		    touch_petrifies(&mons[otmp->corpsenm]) && !uarmg) {
		char kbuf[BUFSZ];

/*JP
		Sprintf(kbuf, "stolen %s corpse", mons[otmp->corpsenm].mname);
*/
		Sprintf(kbuf, "%sの死体を盗んで", jtrns_mon(mons[otmp->corpsenm].mname));
		instapetrify(kbuf);
		break;		/* stop the theft even if hero survives */
	    }
	    /* more take-away handling, after theft message */
	    if (unwornmask & W_WEP) {		/* stole wielded weapon */
		possibly_unwield(mdef, FALSE);
	    } else if (unwornmask & W_ARMG) {	/* stole worn gloves */
		mselftouch(mdef, (const char *)0, TRUE);
		if (mdef->mhp <= 0)	/* it's now a statue */
		    return;		/* can't continue stealing */
	    }

	    if (!stealoid) break;	/* only taking one item */
	}
}

int
damageum(mdef, mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register struct permonst *pd = mdef->data;
	register int	tmp = d((int)mattk->damn, (int)mattk->damd);
	int armpro;
	boolean negated;
	register int    enchantlvl = 0;
	boolean noeffect = FALSE;

	armpro = magic_negation(mdef);
	/* since hero can't be cancelled, only defender's armor applies */
	negated = !((rn2(3) >= armpro) || !rn2(50));

	if (hit_as_one(&youmonst))    enchantlvl = 1; 
	if (hit_as_two(&youmonst))    enchantlvl = 2;         
	if (hit_as_three(&youmonst))  enchantlvl = 3; 
	if (hit_as_four(&youmonst))   enchantlvl = 4;         

	if (need_one(mdef)   && enchantlvl < 1) noeffect = TRUE; 
	if (need_two(mdef)   && enchantlvl < 2) noeffect = TRUE;      
	if (need_three(mdef) && enchantlvl < 3) noeffect = TRUE;  
	if (need_four(mdef)  && enchantlvl < 4) noeffect = TRUE;  

	if (is_demon(youmonst.data) && !rn2(13) && !uwep
		&& u.umonnum != PM_SUCCUBUS && u.umonnum != PM_INCUBUS
		&& u.umonnum != PM_BALROG) {
	    demonpet();
	    return(0);
	}
	switch(mattk->adtyp) {
	    case AD_STUN:
		if(!Blind)
#if 0 /*JP*/
		    pline("%s %s for a moment.", Monnam(mdef),
			  makeplural(stagger(mdef->data, "stagger")));
#else
		    pline("%sは一瞬%s．", Monnam(mdef),
			  stagger(mdef->data, "よろめいた"));
#endif
		mdef->mstun = 1;
		goto physical;
	    case AD_LEGS:
	     /* if (u.ucancelled) { */
	     /*    tmp = 0;	    */
	     /*    break;	    */
	     /* }		    */
		goto physical;
	    case AD_WERE:	    /* no special effect on monsters */
	    case AD_HEAL:	    /* likewise */
	    case AD_PHYS:
 physical:
		if(mattk->aatyp == AT_WEAP) {
		    if(uwep) tmp = 0;
		} else if(mattk->aatyp == AT_KICK) {
		    if(thick_skinned(mdef->data)) tmp = 0;
		    if(mdef->data == &mons[PM_SHADE]) {
			if (!(uarmf && uarmf->blessed)) {
			    impossible("bad shade attack function flow?");
			    tmp = 0;
			} else
			    tmp = rnd(4); /* bless damage */
		    }
		} else if(mattk->aatyp == AT_HUGS &&
			u.umonnum == PM_ROPE_GOLEM) {
		    if (breathless(mdef->data)) tmp = (tmp + 1) / 2;
		}
		break;
	    case AD_FIRE:
		if (negated) {
		    tmp = 0;
		    break;
		}
		if (!Blind)
/*JP
		    pline("%s is %s!", Monnam(mdef),
*/
		    pline("%sは%s！", Monnam(mdef),
			  on_fire(mdef->data, mattk));
		if (pd == &mons[PM_STRAW_GOLEM] ||
		    pd == &mons[PM_PAPER_GOLEM]) {
		    if (!Blind)
/*JP
		    	pline("%s burns completely!", Monnam(mdef));
*/
		    	pline("%sは燃え尽きた！", Monnam(mdef));
		    xkilled(mdef,2);
		    tmp = 0;
		    break;
		    /* Don't return yet; keep hp<1 and tmp=0 for pet msg */
		}
		if (pd == &mons[PM_STRAW_GOLEM] ||
			pd == &mons[PM_PAPER_GOLEM] ||
		    pd == &mons[PM_WAX_GOLEM]) {
		    if (!Blind)
/*JP
			pline("%s falls to pieces!", Monnam(mdef));
*/
			pline("%sはバラバラに崩れた！", Monnam(mdef));
			xkilled(mdef,3);
			return(2);
		}
		tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if (resists_fire(mdef)) {
		    if (!Blind)
/*JP
			pline_The("fire doesn't heat %s!", mon_nam(mdef));
*/
			pline("炎は%sを焼くことができない！", mon_nam(mdef));
		    golemeffects(mdef, AD_FIRE, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only potions damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		break;
	    case AD_COLD:
		if (negated) {
		    tmp = 0;
		    break;
		}
/*JP
		if (!Blind) pline("%s is covered in frost!", Monnam(mdef));
*/
		if (!Blind) pline("%sは氷で覆われた！", Monnam(mdef));
		if (resists_cold(mdef)) {
		    shieldeff(mdef->mx, mdef->my);
		    if (!Blind)
/*JP
			pline_The("frost doesn't chill %s!", mon_nam(mdef));
*/
			pline("冷気は%sを凍らせることができない！", mon_nam(mdef));
		    golemeffects(mdef, AD_COLD, tmp);
		    tmp = 0;
		}
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		break;
	    case AD_ELEC:
		if (negated) {
		    tmp = 0;
		    break;
		}
/*JP
		if (!Blind) pline("%s is zapped!", Monnam(mdef));
*/
		if (!Blind) pline("%sは電撃をくらった！", Monnam(mdef));
		tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		if (resists_elec(mdef)) {
		    if (!Blind)
/*JP
			pline_The("zap doesn't shock %s!", mon_nam(mdef));
*/
			pline("電撃は%sに影響を与えない！", mon_nam(mdef));
		    golemeffects(mdef, AD_ELEC, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only rings damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		break;
	    case AD_ACID:
		if (resists_acid(mdef)) tmp = 0;
		break;
	    case AD_STON:
		if (!munstone(mdef, TRUE))
		    minstapetrify(mdef, TRUE);
		tmp = 0;
		break;
#ifdef SEDUCE
	    case AD_SSEX:
#endif
	    case AD_SEDU:
	    case AD_SITM:
		steal_it(mdef, mattk);
		tmp = 0;
		break;
	    case AD_SGLD:
#ifndef GOLDOBJ
		if (mdef->mgold) {
		    u.ugold += mdef->mgold;
		    mdef->mgold = 0;
/*JP
		    Your("purse feels heavier.");
*/
		    You("財布が重くなったようだ．");
		}
#else
                /* This you as a leprechaun, so steal
                   real gold only, no lesser coins */
	        {
		    struct obj *mongold = findgold(mdef->minvent);
	            if (mongold) {
		        obj_extract_self(mongold);  
		        if (merge_choice(invent, mongold) || inv_cnt() < 52) {
			    addinv(mongold);
/*JP
			    Your("purse feels heavier.");
*/
			    You("財布が重くなったようだ．");
			} else {
/*JP
                            You("grab %s's gold, but find no room in your knapsack.", mon_nam(mdef));
*/
                            You("%sの金貨をつかんだが，ナップザックに入らなかった．", mon_nam(mdef));
			    dropy(mongold);
		        }
		    }
	        }
#endif
		exercise(A_DEX, TRUE);
		tmp = 0;
		break;
	    case AD_TLPT:
		if (tmp <= 0) tmp = 1;
		if (!negated && tmp < mdef->mhp) {
		    char nambuf[BUFSZ];
		    boolean u_saw_mon = canseemon(mdef) ||
					(u.uswallow && u.ustuck == mdef);
		    /* record the name before losing sight of monster */
		    Strcpy(nambuf, Monnam(mdef));
		    if (u_teleport_mon(mdef, FALSE) &&
			    u_saw_mon && !canseemon(mdef))
/*JP
			pline("%s suddenly disappears!", nambuf);
*/
			pline("%sは突然消えた！", nambuf);
		}
		break;
	    case AD_BLND:
		if (can_blnd(&youmonst, mdef, mattk->aatyp, (struct obj*)0)) {
		    if(!Blind && mdef->mcansee)
/*JP
			pline("%s is blinded.", Monnam(mdef));
*/
			pline("%sは目が見えなくなった．", Monnam(mdef));
		    mdef->mcansee = 0;
		    tmp += mdef->mblinded;
		    if (tmp > 127) tmp = 127;
		    mdef->mblinded = tmp;
		}
		tmp = 0;
		break;
	    case AD_CURS:
		if (night() && !rn2(10) && !mdef->mcan) {
		    if (mdef->data == &mons[PM_CLAY_GOLEM]) {
			if (!Blind)
#if 0 /*JP*/
			    pline("Some writing vanishes from %s head!",
				s_suffix(mon_nam(mdef)));
#else
			    pline("%sの頭から何かの文字が幾つか消えた！", mon_nam(mdef));
#endif
			xkilled(mdef, 0);
			/* Don't return yet; keep hp<1 and tmp=0 for pet msg */
		    } else {
			mdef->mcan = 1;
/*JP
			You("chuckle.");
*/
			You("くすくす笑った．");
		    }
		}
		tmp = 0;
		break;
	    case AD_DRLI:
		if (!negated && !rn2(3) && !resists_drli(mdef)) {
			int xtmp = d(2,6);
			if (mdef->mhp < xtmp) xtmp = mdef->mhp;
			if (maybe_polyd(is_vampire(youmonst.data), 
			    Race_if(PM_VAMPIRE)) && mattk->aatyp == AT_BITE &&
			    has_blood(pd)) {
				/* For the life of a creature is in the blood
				   (Lev 17:11) */
				if (flags.verbose)
/*JP
				    You("feed on the lifeblood.");
*/
				    You("生き血を啜った．");
				/* [ALI] Biting monsters does not count against
				   eating conducts. The draining of life is
				   considered to be primarily a non-physical
				   effect */
				lesshungry(xtmp * 6);
			}
/*JP
			pline("%s suddenly seems weaker!", Monnam(mdef));
*/
			pline("%sは突然弱くなったようだ！", Monnam(mdef));
			mdef->mhpmax -= xtmp;
#ifdef SHOW_DMG
			if (xtmp < mdef->mhp) showdmg(xtmp);
#endif
			if ((mdef->mhp -= xtmp) <= 0 || !mdef->m_lev) {
/*JP
				pline("%s dies!", Monnam(mdef));
*/
				pline("%sは死んだ！", Monnam(mdef));
				xkilled(mdef,0);
			} else
				mdef->m_lev--;
		tmp = 0;
		}
		break;
	    case AD_RUST:
		if (pd == &mons[PM_IRON_GOLEM]) {
/*JP
			pline("%s falls to pieces!", Monnam(mdef));
*/
			pline("%sはバラバラに崩れた！", Monnam(mdef));
			xkilled(mdef,0);
		}
		hurtmarmor(mdef, AD_RUST);
		tmp = 0;
		break;
	    case AD_CORR:
		hurtmarmor(mdef, AD_CORR);
		tmp = 0;
		break;
	    case AD_DCAY:
		if (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM]) {
/*JP
			pline("%s falls to pieces!", Monnam(mdef));
*/
			pline("%sはバラバラに崩れた！", Monnam(mdef));
			xkilled(mdef,0);
		}
		hurtmarmor(mdef, AD_DCAY);
		tmp = 0;
		break;
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!negated && !rn2(8)) {
/*JP
		    Your("%s was poisoned!", mpoisons_subj(&youmonst, mattk));
*/
		    Your("%sは毒を帯びている！", mpoisons_subj(&youmonst, mattk));
		    if (resists_poison(mdef))
/*JP
			pline_The("poison doesn't seem to affect %s.",
*/
			pline("毒は%sには効かないようだ．",
				mon_nam(mdef));
		    else {
			if (!rn2(10)) {
/*JP
			    Your("poison was deadly...");
*/
			    Your("与えた毒は致死量だった．．．");
			    tmp = mdef->mhp;
			} else tmp += rn1(10,6);
		    }
		}
		break;
	    case AD_DRIN:
		if (notonhead || !has_head(mdef->data)) {
/*JP
		    pline("%s doesn't seem harmed.", Monnam(mdef));
*/
		    pline("%sは傷ついてないようだ．", Monnam(mdef));
		    tmp = 0;
		    if (!Unchanging && mdef->data == &mons[PM_GREEN_SLIME]) {
			if (!Slimed) {
/*JP
			    You("suck in some slime and don't feel very well.");
*/
			    You("スライムを吸ってしまい，とても気分が悪くなった．");
			    Slimed = 10L;
#if 1 /*JP*/
				Sprintf(dkiller_buf, "%sを啜って", jtrns_mon(mdef->data->mname));
				delayed_killer = dkiller_buf;
#endif
			}
		    }
		    break;
		}
		if (m_slips_free(mdef, mattk)) break;

		if ((mdef->misc_worn_check & W_ARMH) && rn2(8)) {
#if 0 /*JP*/
		    pline("%s helmet blocks your attack to %s head.",
			  s_suffix(Monnam(mdef)), mhis(mdef));
#else
		    pline("%sの兜が頭への攻撃を防いだ．",
			  Monnam(mdef));
#endif
		    break;
		}

/*JP
		You("eat %s brain!", s_suffix(mon_nam(mdef)));
*/
		You("%sの脳を食べた！", mon_nam(mdef));
		u.uconduct.food++;
		if (touch_petrifies(mdef->data) && !Stone_resistance && !Stoned) {
		    Stoned = 5;
		    killer_format = KILLED_BY_AN;
#if 0 /*JP*/
		    delayed_killer = mdef->data->mname;
#else
		    Sprintf(dkiller_buf, "%sの脳で", jtrns_mon(mdef->data->mname));
		    delayed_killer = dkiller_buf;
#endif
		}
		if (!vegan(mdef->data))
		    u.uconduct.unvegan++;
		if (!vegetarian(mdef->data))
		    violated_vegetarian();
		if (mindless(mdef->data)) {
/*JP
		    pline("%s doesn't notice.", Monnam(mdef));
*/
		    pline("%sは気にかけていない．", Monnam(mdef));
		    break;
		}
		tmp += rnd(10);
		morehungry(-rnd(30)); /* cannot choke */
		if (ABASE(A_INT) < AMAX(A_INT)) {
			ABASE(A_INT) += rnd(4);
			if (ABASE(A_INT) > AMAX(A_INT))
				ABASE(A_INT) = AMAX(A_INT);
			flags.botl = 1;
		}
		exercise(A_WIS, TRUE);
		break;
	    case AD_STCK:
		if (!negated && !sticks(mdef->data))
		    setustuck(mdef); /* it's now stuck to you */
		break;
	    case AD_WRAP:
		if (!sticks(mdef->data)) {
		    if (!u.ustuck && !rn2(10)) {
			if (m_slips_free(mdef, mattk)) {
			    tmp = 0;
			} else {
/*JP
			    You("swing yourself around %s!",
*/
			    You("%sに巻きついた！",
				  mon_nam(mdef));
			    setustuck(mdef);
			}
		    } else if(u.ustuck == mdef) {
			/* Monsters don't wear amulets of magical breathing */
			if (is_pool(u.ux,u.uy) && !is_swimmer(mdef->data) &&
			    !amphibious(mdef->data)) {
/*JP
			    You("drown %s...", mon_nam(mdef));
*/
			    You("%sを溺れさせた．．．", mon_nam(mdef));
			    tmp = mdef->mhp;
			} else if(mattk->aatyp == AT_HUGS)
/*JP
			    pline("%s is being crushed.", Monnam(mdef));
*/
			    pline("%sは押し潰されている．", Monnam(mdef));
		    } else {
			tmp = 0;
			if (flags.verbose)
/*JP
			    You("brush against %s %s.",
*/
			    You("%sの%sに触れた．",
				s_suffix(mon_nam(mdef)),
				mbodypart(mdef, LEG));
		    }
		} else tmp = 0;
		break;
	    case AD_PLYS:
		if (!negated && mdef->mcanmove && !rn2(3) && tmp < mdef->mhp) {
/*JP
		    if (!Blind) pline("%s is frozen by you!", Monnam(mdef));
*/
		    if (!Blind) pline("%sは動けなくなった！", Monnam(mdef));
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    case AD_TCKL:
		if (!negated && mdef->mcanmove && !rn2(3) && tmp < mdef->mhp) {
/*JP
		    if (!Blind) You("mercilessly tickle %s!", mon_nam(mdef));
*/
		    if (!Blind) You("容赦なく%sをくすぐった！", mon_nam(mdef));
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    case AD_SLEE:
		if (mattk->aatyp == AT_GAZE && mon_reflects(mdef, (char *)0)) {
		    tmp = 0;
#if 0 /*JP*/
		    (void) mon_reflects(mdef, "But it reflects from %s %s!");
#else
		    (void) mon_reflects(mdef, "しかしそれは%sの%sで反射した！");
#endif
		    if (Sleep_resistance || Free_action) {
#if 0 /*JP*/
			pline("You yawn.");
#else
			pline("あなたはあくびした．");
#endif
			break;
		    } else {
			nomul(-rnd(10));
			u.usleep = 1;
#if 0 /*JP*/
			nomovemsg = "You wake up.";
			if (Blind)  You("are put to sleep!");
			else You("are put to sleep by your reflected gaze!");
#else
			nomovemsg = "目を覚ました．";
			if (Blind)  You("突然眠りにおちた！");
			else You("反射してきた睨みで突然眠りにおちた！");
#endif
			break;
		    }
		}

		if (!negated && !mdef->msleeping &&
			(mattk->aatyp != AT_WEAP || barehanded_hit) &&
			sleep_monst(mdef, rnd(10), -1)) {
		    if (!Blind)
/*JP
			pline("%s is put to sleep by you!", Monnam(mdef));
*/
			You("%sを眠らせた！", Monnam(mdef));
		    slept_monst(mdef);
		}
		else
		    tmp = 0;
		break;
	    case AD_SLIM:
		if (negated) break;	/* physical damage only */
		if (!rn2(4) && !flaming(mdef->data) &&
				mdef->data != &mons[PM_GREEN_SLIME]) {
/*JP
		    You("turn %s into slime.", mon_nam(mdef));
*/
	    	    You("%sをスライムに変えた．", mon_nam(mdef));
		    (void) newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, !Blind);
		    tmp = 0;
		}
		break;
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
		/* There's no msomearmor() function, so just do damage */
	     /* if (negated) break; */
		break;
	    case AD_SLOW:
		if (!negated && mdef->mspeed != MSLOW) {
		    unsigned int oldspeed = mdef->mspeed;

		    mon_adjust_speed(mdef, -1, (struct obj *)0);
		    if (mdef->mspeed != oldspeed && canseemon(mdef))
/*JP
			pline("%s slows down.", Monnam(mdef));
*/
			pline("%sはのろくなった．", Monnam(mdef));
		}
		break;
	    case AD_CONF:
		if (!mdef->mconf) {
		    if (canseemon(mdef))
/*JP
			pline("%s looks confused.", Monnam(mdef));
*/
			pline("%sは混乱したようだ．", Monnam(mdef));
		    mdef->mconf = 1;
		}
		else
		{
		    if (canseemon(mdef))
#if 0 /*JP*/
			pline("%s is getting more and more confused.",
#else
			pline("%sはますます混乱した．",
#endif
				Monnam(mdef));
		    mdef->mconf++;
		}
		break;
	    case AD_POLY:
		if (tmp < mdef->mhp) {
		    if (resists_magm(mdef)) {
			/* magic resistance protects from polymorph traps,
			 * so make it guard against involuntary polymorph
			 * attacks too... */
			shieldeff(mdef->mx, mdef->my);
#if 0
		    } else if (!rn2(25) || !mon_poly(mdef)) {
			if (canseemon(mdef)) {
			    pline("%s shudders!", Monnam(mdef));
			}
			/* no corpse after system shock */
			tmp = rnd(30);
#endif
#if 0 /*JP*/
		    } else if (!mon_poly(mdef, TRUE,
			    "%s undergoes a freakish metamorphosis!"))
#else
		    } else if (!mon_poly(mdef, TRUE,
			    "%sは奇形な変化をしはじめた！"))
#endif
			/* prevent killing the monster again - 
			 * could be killed in mon_poly */
			tmp = 0;
		}
		break;
		/* WAC -- for death gazes - but all messages should be generic */
	    case AD_DETH:
		if (rn2(16)) {
		    /* Just damage */
		    break;
		}
		if (mattk->aatyp == AT_GAZE) 
#if 0 /*JP*/
		    You("look directly at %s!", mon_nam(mdef));
#else
		    You("%sのいる方向を見た！", mon_nam(mdef));
#endif
		if ((mattk->aatyp == AT_GAZE) && (mon_reflects(mdef, (char *)0))) {
		    /* WAC reflected gaze 
		     * Oooh boy...that was a bad move :B 
		     */
		    tmp = 0;
		    shieldeff(mdef->mx, mdef->my);
#if 0 /*JP*/
		    (void) mon_reflects(mdef, "But it reflects from %s %s!");
#else
		    (void) mon_reflects(mdef, "しかしそれは%sの%sで反射した！");
#endif
		    if (Antimagic) {
#if 0 /*JP*/
			You("shudder momentarily...");
#else
			You("一瞬震えた．．．");
#endif
			break;
		    }
#if 0 /*JP*/
		    You("die...");
#else
		    You("死んだ．．．");
#endif
		    killer_format = KILLED_BY;
#if 0 /*JP*/
		    killer = "a reflected gaze of death";
#else
		    killer = "反射された死の睨みで";
#endif
		    done(DIED);
		} else if (is_undead(mdef->data)) {
		    /* Still does normal damage */
#if 0 /*JP*/
		    if (!Blind) pline("Something didn't work...");
#else
		    if (!Blind) pline("何かが動かなくなった．．．");
#endif
		    break;
		} else if (resists_magm(mdef)) {
		    if (!Blind)
#if 0 /*JP*/
			pline("%s shudders momentarily...", Monnam(mdef));
#else
			pline("%sは一瞬震えた．．．", Monnam(mdef));
#endif
		} else {
		    tmp = mdef->mhp;
		}
		break;
	    case AD_DREN:
	    	if (resists_magm(mdef)) {
		    if (!Blind) {
			shieldeff(mdef->mx,mdef->my);
#if 0 /*JP*/
			pline("%s is unaffected.", Monnam(mdef));
#else
			pline("%sは影響を受けない．", Monnam(mdef));
#endif
		    }
	    	} else {
	    	    mon_drain_en(mdef, 
				((mdef->m_lev > 0) ? (rnd(mdef->m_lev)) : 0) + 1);
	    	}
		break;
	    case AD_CALM:	/* KMH -- koala attack */
		/* Certain monsters aren't even made peaceful. */
		if (!mdef->iswiz && mdef->data != &mons[PM_MEDUSA] &&
				!(mdef->data->mflags3 & M3_COVETOUS) &&
				!(mdef->data->geno & G_UNIQ)) {
#if 0 /*JP*/
		    pline("You calm %s.", mon_nam(mdef));
#else
		    You("%sを落ちつかせた．", mon_nam(mdef));
#endif
		    mdef->mpeaceful = 1;
		    mdef->mtame = 0;
		    tmp = 0;
		}
		break;
	    default:	tmp = 0;
		break;
	}

	mdef->mstrategy &= ~STRAT_WAITFORU; /* in case player is very fast */
	if (tmp && noeffect && !DEADMONSTER(mdef)) {
#if 0 /*JP*/
	     You("don't seem to harm %s.", mon_nam(mdef));
#else
	     You("%sにダメージを与えられなかったようだ．", mon_nam(mdef));
#endif
	     tmp = 0;
	     return 1;
	}

#ifdef SHOW_DMG
	if (tmp < mdef->mhp) showdmg(tmp);
#endif
	
	/* if tmp == 0, DON'T xkilled/killed the monster even if hp < 1 
	 *	- xkilled/killed via other method... */

	if((mdef->mhp -= tmp) < 1) {
	    if (mdef->mtame && !cansee(mdef->mx,mdef->my)) {
/*JP
		You_feel("embarrassed for a moment.");
*/
		You("一瞬気まずくなった．");
		if (tmp) xkilled(mdef, 0); /* !tmp but hp<1: already killed */
	    } else if (!flags.verbose) {
/*JP
		You("destroy it!");
*/
		You("倒した！");
		if (tmp) xkilled(mdef, 0);
	    } else
		if (tmp) killed(mdef);
	    return(2);
	}
	return(1);
}

STATIC_OVL int
explum(mdef, mattk)
register struct monst *mdef;
register struct attack *mattk;
{
#if 1 /*JP*/
	char *jad = "";
#endif
	register int tmp = d((int)mattk->damn, (int)mattk->damd);

/*JP
	You("explode!");
*/
	You("爆発した！");
	switch(mattk->adtyp) {
	    boolean resistance; /* only for cold/fire/elec */

	    case AD_BLND:
		if (!resists_blnd(mdef)) {
/*JP
		    pline("%s is blinded by your flash of light!", Monnam(mdef));
*/
		    pline("%sはまばゆい光で目がくらんだ！", Monnam(mdef));
		    mdef->mblinded = min((int)mdef->mblinded + tmp, 127);
		    mdef->mcansee = 0;
		}
		break;
	    case AD_HALU:
		if (haseyes(mdef->data) && mdef->mcansee) {
/*JP
		    pline("%s is affected by your flash of light!",
*/
		    pline("%sはまばゆい光の影響を受けた！",
			  Monnam(mdef));
		    mdef->mconf = 1;
		}
		break;
	    case AD_COLD:
#if 1 /*JP*/
		jad = "冷気";
#endif
		resistance = resists_cold(mdef);
		goto common;
	    case AD_FIRE:
#if 1 /*JP*/
		jad = "炎";
#endif
		resistance = resists_fire(mdef);
		goto common;
	    case AD_ELEC:
#if 1 /*JP*/
		jad = "電撃";
#endif
		resistance = resists_elec(mdef);
common:
		if (!resistance) {
/*JP
		    pline("%s gets blasted!", Monnam(mdef));
*/
		    pline("%sは%sの爆風を浴びた！", Monnam(mdef), jad);
		    mdef->mhp -= tmp;
		    if (mdef->mhp <= 0) {
			 killed(mdef);
			 return(2);
		    }
		} else {
		    shieldeff(mdef->mx, mdef->my);
		    if (is_golem(mdef->data))
			golemeffects(mdef, (int)mattk->adtyp, tmp);
		    else
/*JP
			pline_The("blast doesn't seem to affect %s.",
*/
			pline("%sの爆風は%sに影響を与えなかったようだ．", jad,
				mon_nam(mdef));
		}
		break;
	    default:
		break;
	}
	return(1);
}

STATIC_OVL void
start_engulf(mdef)
struct monst *mdef;
{
	if (!Invisible) {
		map_location(u.ux, u.uy, TRUE);
		tmp_at(DISP_ALWAYS, mon_to_glyph(&youmonst));
		tmp_at(mdef->mx, mdef->my);
	}
/*JP
	You("engulf %s!", mon_nam(mdef));
*/
	You("%sを飲み込んだ！", mon_nam(mdef));
	delay_output();
	delay_output();
}

STATIC_OVL void
end_engulf()
{
	if (!Invisible) {
		tmp_at(DISP_END, 0);
		newsym(u.ux, u.uy);
	}
}

STATIC_OVL int
gulpum(mdef,mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register int tmp;
	register int dam = d((int)mattk->damn, (int)mattk->damd);
	struct obj *otmp;
	/* Not totally the same as for real monsters.  Specifically, these
	 * don't take multiple moves.  (It's just too hard, for too little
	 * result, to program monsters which attack from inside you, which
	 * would be necessary if done accurately.)  Instead, we arbitrarily
	 * kill the monster immediately for AD_DGST and we regurgitate them
	 * after exactly 1 round of attack otherwise.  -KAA
	 */

	if(mdef->data->msize >= MZ_HUGE) return 0;

	if(u.uhunger < 1500 && !u.uswallow) {
	    for (otmp = mdef->minvent; otmp; otmp = otmp->nobj)
		(void) snuff_lit(otmp);

	    if(!touch_petrifies(mdef->data) || Stone_resistance) {
#ifdef LINT	/* static char msgbuf[BUFSZ]; */
		char msgbuf[BUFSZ];
#else
		static char msgbuf[BUFSZ];
#endif
		start_engulf(mdef);
		switch(mattk->adtyp) {
		    case AD_DGST:
			/* eating a Rider or its corpse is fatal */
			if (is_rider(mdef->data)) {
/*JP
			 pline("Unfortunately, digesting any of it is fatal.");
*/
			 pline("残念ながら，それを食べるのは致命的な間違いだ．");
			    end_engulf();
#if 0 /*JP:T*/
			    Sprintf(msgbuf, "unwisely tried to eat %s",
				    mdef->data->mname);
			    killer = msgbuf;
			    killer_format = NO_KILLER_PREFIX;
#else
			    Sprintf(msgbuf, "愚かにも%sを食べて",
				    jtrns_mon(mdef->data->mname));
			    killer = msgbuf;
			    killer_format = KILLED_BY;
#endif
			    done(DIED);
			    return 0;		/* lifesaved */
			}

			if (Slow_digestion) {
			    dam = 0;
			    break;
			}

			/* KMH, conduct */
			u.uconduct.food++;
			if (!vegan(mdef->data))
			     u.uconduct.unvegan++;
			if (!vegetarian(mdef->data))
			     violated_vegetarian();

			/* Use up amulet of life saving */
			if (!!(otmp = mlifesaver(mdef))) m_useup(mdef, otmp);

			newuhs(FALSE);
			xkilled(mdef,2);
			if (mdef->mhp > 0) { /* monster lifesaved */
/*JP
			    You("hurriedly regurgitate the sizzling in your %s.",
*/
			    You("%sの中でシューシューという音を立てているものを大急ぎで吐き戻した．",
				body_part(STOMACH));
			} else {
			    tmp = 1 + (mdef->data->cwt >> 8);
			    if (corpse_chance(mdef, &youmonst, TRUE) &&
				!(mvitals[monsndx(mdef->data)].mvflags &
				  G_NOCORPSE)) {
				/* nutrition only if there can be a corpse */
				u.uhunger += (mdef->data->cnutrit+1) / 2;
			    } else tmp = 0;
/*JP
			    Sprintf(msgbuf, "You totally digest %s.",
*/
			    Sprintf(msgbuf, "あなたは%sを完全に消化した．",
					    mon_nam(mdef));
			    if (tmp != 0) {
				/* setting afternmv = end_engulf is tempting,
				 * but will cause problems if the player is
				 * attacked (which uses his real location) or
				 * if his See_invisible wears off
				 */
/*JP
				You("digest %s.", mon_nam(mdef));
*/
				You("%sを消化している．", mon_nam(mdef));
				if (Slow_digestion) tmp *= 2;
				nomul(-tmp);
				nomovemsg = msgbuf;
			    } else pline("%s", msgbuf);
			    if (mdef->data == &mons[PM_GREEN_SLIME]) {
#if 0 /*JP*/
				Sprintf(msgbuf, "%s isn't sitting well with you.",
					The(mdef->data->mname));
#else
				Sprintf(msgbuf, "%sはあなたとうまく折り合いをつけられないようだ．",
					jtrns_mon(mdef->data->mname));
#endif
				if (!Unchanging) {
					Slimed = 5L;
					flags.botl = 1;
				}
			    } else
			    exercise(A_CON, TRUE);
			}
			end_engulf();
			return(2);
		    case AD_PHYS:
			if (youmonst.data == &mons[PM_FOG_CLOUD]) {
/*JP
			    pline("%s is laden with your moisture.",
*/
			    pline("%sはあなたの湿気に苦しめられている．",
				  Monnam(mdef));
			    if (amphibious(mdef->data) &&
				!flaming(mdef->data)) {
				dam = 0;
/*JP
				pline("%s seems unharmed.", Monnam(mdef));
*/
				pline("%sは傷ついていないようだ．", Monnam(mdef));
			    }
			} else
/*JP
			    pline("%s is pummeled with your debris!",
*/
			    pline("%sを瓦礫で痛めつけた！",
				  Monnam(mdef));
			break;
		    case AD_ACID:
/*JP
			pline("%s is covered with your goo!", Monnam(mdef));
*/
			pline("%sを粘液で覆った！", Monnam(mdef));
			if (resists_acid(mdef)) {
/*JP
			    pline("It seems harmless to %s.", mon_nam(mdef));
*/
			    pline("しかし，%sはなんともない．", mon_nam(mdef));
			    dam = 0;
			}
			break;
		    case AD_BLND:
			if (can_blnd(&youmonst, mdef, mattk->aatyp, (struct obj *)0)) {
			    if (mdef->mcansee)
/*JP
				pline("%s can't see in there!", Monnam(mdef));
*/
				pline("%sは目が見えなくなった！", mon_nam(mdef));
			    mdef->mcansee = 0;
			    dam += mdef->mblinded;
			    if (dam > 127) dam = 127;
			    mdef->mblinded = dam;
			}
			dam = 0;
			break;
		    case AD_ELEC:
			if (rn2(2)) {
/*JP
			    pline_The("air around %s crackles with electricity.", mon_nam(mdef));
*/
			    pline("%sの回りの空気は電気でピリピリしている．", mon_nam(mdef));
			    if (resists_elec(mdef)) {
/*JP
				pline("%s seems unhurt.", Monnam(mdef));
*/
				pline("しかし，%sは平気なようだ．", Monnam(mdef));
				dam = 0;
			    }
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_COLD:
			if (rn2(2)) {
			    if (resists_cold(mdef)) {
/*JP
				pline("%s seems mildly chilly.", Monnam(mdef));
*/
				pline("%sは涼しげだ．", Monnam(mdef));
				dam = 0;
			    } else
/*JP
				pline("%s is freezing to death!",Monnam(mdef));
*/
				pline("%sは凍死しそうだ！",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_FIRE:
			if (rn2(2)) {
			    if (resists_fire(mdef)) {
/*JP
				pline("%s seems mildly hot.", Monnam(mdef));
*/
				pline("%sは温まったようだ．", Monnam(mdef));
				dam = 0;
			    } else
/*JP
				pline("%s is burning to a crisp!",Monnam(mdef));
*/
				pline("%sは燃えてカラカラになった！",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		}
		end_engulf();
		if ((mdef->mhp -= dam) <= 0) {
		    killed(mdef);
		    if (mdef->mhp <= 0)	/* not lifesaved */
			return(2);
		}
#if 0 /*JP*/
		You("%s %s!", is_animal(youmonst.data) ? "regurgitate"
			: "expel", mon_nam(mdef));
#else
		You("%sを%sした！", mon_nam(mdef),
		    is_animal(youmonst.data) ? "吐き戻" : "排出");
#endif
		if (Slow_digestion || is_animal(youmonst.data)) {
/*JP
		    pline("Obviously, you didn't like %s taste.",
*/
		    pline("どうも%sの味は好きになれない．",
			  s_suffix(mon_nam(mdef)));
		}
	    } else {
		char kbuf[BUFSZ];

/*JP
		You("bite into %s.", mon_nam(mdef));
*/
		You("%sに噛みついた．", mon_nam(mdef));
/*JP
		Sprintf(kbuf, "swallowing %s whole", an(mdef->data->mname));
*/
		Sprintf(kbuf, "%sを完全に飲み込んで", a_monnam(mdef));
		instapetrify(kbuf);
	    }
	}
	return(0);
}

void
missum(mdef, target, roll, mattk)
register struct monst *mdef;
register struct attack *mattk;
register int target;
register int roll;
{
	register boolean nearmiss = (target == roll);
	register struct obj *blocker = (struct obj *)0;
	long mwflags = mdef->misc_worn_check;

		/* 3 values for blocker
		 *	No blocker:  (struct obj *) 0  
		 * 	Piece of armour:  object
		 */

	/* This is a hack,  since there is no fast equivalent for uarm, uarms, etc.  
	 * Technically, we really should check from the inside out...
	 */
	if (target < roll) {
	    for (blocker = mdef->minvent; blocker; blocker = blocker->nobj) {
		if (blocker->owornmask & mwflags) {
			target += ARM_BONUS(blocker);
			if (target > roll) break;
		}
	    }
	}
	
	if (could_seduce(&youmonst, mdef, mattk)) {
/*JP
		You("pretend to be friendly to %s.", mon_nam(mdef));
*/
		You("%sに友好的なふりをした．", mon_nam(mdef));
	} else if(canspotmon(mdef) && flags.verbose) {
		if (nearmiss || !blocker) {
/*JP
		    You("%smiss %s.", (nearmiss ? "just " : ""),mon_nam(mdef));
*/
		    Your("%sへの攻撃は%s外れた．", mon_nam(mdef), (nearmiss ? "ぎりぎり" : ""));
		} else {
        	    /* Blocker */
#if 0 /*JP*/
        	    pline("%s %s %s your attack.", 
        		s_suffix(Monnam(mdef)),
        		aobjnam(blocker, (char *)0),
        		(rn2(2) ? "blocks" : "deflects"));    
#else
        	    pline("%sはあなたの攻撃を%sによって%s．", 
        		s_suffix(Monnam(mdef)),
        		aobjnam(blocker, (char *)0),
        		(rn2(2) ? "防いだ" : "弾いた"));    
#endif
		}
	} else {
/*JP
		You("%smiss it.", ((flags.verbose && nearmiss) ? "just " : ""));
*/
		Your("何者かへの攻撃は%s外れた．", ((flags.verbose && nearmiss) ? "ぎりぎり" : ""));
	}
	if (!mdef->msleeping && mdef->mcanmove)
		wakeup(mdef);
}

/*
 * [WAC] This code now handles twoweapon in the following way:
 *	-monster with one or 2 AT_WEAP can get primary and secondary attacks
 *	-monster with AT_WEAP and another hand attack(s) will lose one
 *		of the other hand attacks
 *	-monster with several hand attacks will do primary weapon and secondary
 *		weapon attacks with the first 2 hand attacks,  then claw attacks
 *		for the rest
 *	-Code assumes that a single AT_WEAP or 2 AT_WEAP entries still mean 
 *		at most 1 or 2 hands attack.  i.e. 1 handed monsters with AT_WEAP
 *		or monsters with 3+ handed monsters where more than 2 hands are
 *		AT_WEAP are not handled properly
 *		(I don't think any exist yet)
 * This code now handles ALL hand to hand whether you are poly'ed or not
 * (uses your current race as the monster type)
 *
 * [ALI] Returns TRUE if you hit (and maybe killed) the monster.
 */
STATIC_OVL boolean
hmonas(mon, tmp)		/* attack monster as a monster. */
register struct monst *mon;
register int tmp;
{
	struct attack *mattk, alt_attk;
	int	i, sum[NATTK];
#if 0
	int	hittmp = 0;
#endif
	int	nsum = 0;
	int	dhit = 0;
	int 	mhit = 0; /* Used to pass the attacks used */
	int 	tmp1, tmp2;
	boolean Old_Upolyd = Upolyd;
	static const int hit_touch[] = {0, HIT_BODY, HIT_BODY|HIT_FATAL};
	static const int hit_notouch[] = {0, HIT_OTHER, HIT_OTHER|HIT_FATAL};
	
	/* Keeps track of which weapon hands have been used */
	boolean used_uwep = FALSE;

	for(i = 0; i < NATTK; i++) {
	    mhit = 0; /* Clear all previous attacks */

	    sum[i] = 0;
	    mattk = getmattk(youmonst.data, i, sum, &alt_attk);
	    
	    switch(mattk->aatyp) {
		case AT_WEAP:
use_weapon:	
	/* Certain monsters don't use weapons when encountered as enemies,
	 * but players who polymorph into them have hands or claws and thus
	 * should be able to use weapons.  This shouldn't prohibit the use
	 * of most special abilities, either.
	 */
	/* Potential problem: if the monster gets multiple weapon attacks,
	 * we currently allow the player to get each of these as a weapon
	 * attack.  Is this really desirable?
	 * [WAC] See Above ...  anyways,  this was changed in 3.3.0 so that
	 * only attack 0 would give a weapon attack...
	 * [ALI] Most monsters should get multiple weapon attacks since they
	 * only have two hands. There are exceptions such as mariliths which
	 * should get two weapon attacks and four barehanded attacks. Such
	 * monsters should be special cased in AT_CLAW below.
	 */
			mhit = used_uwep ? HIT_USWAPWEP : HIT_UWEP;
			used_uwep = !used_uwep;
			if (mhit == HIT_USWAPWEP && !u.twoweap)
			    continue;	/* Skip this attack */

			/* WAC if attacking cockatrice/etc, player is smart
			   if wielding a weapon.  So don't let him
			   touch the monster */
			if ((uwep || u.twoweap && uswapwep) &&
				(mhit == HIT_UWEP && !uwep ||
				 mhit == HIT_USWAPWEP && !uswapwep) &&
				(touch_petrifies(mon->data) ||
				 mon->data == &mons[PM_MEDUSA]))
			    break;

			dhit = mhit; /* Clear the miss counter as attacks miss */
			tmp1 = tmp2 = tmp;

#ifdef DEBUG
			pline("%i/20", tmp);
#endif

			if (mhit & HIT_UWEP) {
			    if (uwep) tmp1 = tmp + hitval(uwep, mon);
			    tohit(UWEP_ROLL) = tmp1;
			    if (tmp1 <= (dice(UWEP_ROLL) = rnd(20)) &&
				    !u.uswallow)
				dhit &= ~HIT_UWEP; /* missed */
				
			    if (tmp1 > dice(UWEP_ROLL)) exercise(A_DEX, TRUE);
#ifdef DEBUG
			    pline("(%i/20)", tmp1);
#endif
			}

			if (mhit & HIT_USWAPWEP) {
			    if (uswapwep)
				tmp2 = tmp + hitval(uswapwep, mon) - 2;

			    tohit(USWAPWEP_ROLL) = tmp2;

			    if (tmp2 <= (dice(USWAPWEP_ROLL) = rnd(20)) &&
				    !u.uswallow)
				dhit &= ~HIT_USWAPWEP;

			    if (tmp2 > dice(USWAPWEP_ROLL))
				exercise(A_DEX, TRUE);
#ifdef DEBUG
			    pline("((%i/20))", tmp2);
#endif
			}

			if (dhit && mattk->adtyp == AD_SLEE)
			    barehanded_hit = (dhit & HIT_UWEP) && !uwep ||
			      (dhit & HIT_USWAPWEP) && !uswapwep;

#if 0 /* Old code */
			if (uwep) {
			    hittmp = hitval(uwep, mon);
			    hittmp += weapon_hit_bonus(uwep);
			    tmp += hittmp;
			}
			if (tmp > (dice(UWEP_ROLL) = rnd(20)) || u.uswallow)
			    dhit = HIT_UWEP;
			else dhit = 0;
			/* KMH -- Don't accumulate to-hit bonuses */
			if (uwep) tmp -= hittmp;
#endif
			/* Enemy dead, before any special abilities used */
			if (!known_hitum(mon,mhit,&dhit,mattk)) {
			    sum[i] = dhit | HIT_FATAL;
			    break;
			} else sum[i] = dhit;
			/* might be a worm that gets cut in half */
			if (m_at(u.ux+u.dx, u.uy+u.dy) != mon) return((boolean)(nsum != 0));
			/* Do not print "You hit" message, since known_hitum
			 * already did it.
			 */
			if (dhit && mattk->adtyp != AD_SPEL
				&& mattk->adtyp != AD_PHYS)
			    if (damageum(mon,mattk) == 2)
				sum[i] |= HIT_FATAL;
			break;
		case AT_CLAW:
			if (!cantwield(youmonst.data) &&
				u.umonnum != PM_MARILITH)
			    goto use_weapon;
#ifdef SEDUCE
#if 0	/* Shouldn't matter where the first AT_CLAW is anymore
			/* succubi/incubi are humanoid, but their _second_
			 * attack is AT_CLAW, not their first...
			 */
			if (i==1 && uwep && (u.umonnum == PM_SUCCUBUS ||
				u.umonnum == PM_INCUBUS)) goto use_weapon;
#endif
#endif
		case AT_BITE:
			/* [ALI] Vampires are also smart. They avoid biting
			   monsters if doing so would be fatal */
			if ((uwep || u.twoweap && uswapwep) &&
				is_vampire(youmonst.data) &&
				(is_rider(mon->data) ||
				 mon->data == &mons[PM_GREEN_SLIME]))
			    break;
		case AT_STNG:
		case AT_TUCH:
		case AT_BUTT:
		case AT_TENT:
			if (i==0 && uwep && (youmonst.data->mlet==S_LICH)) goto use_weapon;
			if ((uwep || u.twoweap && uswapwep) &&
				(touch_petrifies(mon->data) ||
				 mon->data == &mons[PM_MEDUSA]))
			    break;
		case AT_KICK:
			if ((dhit = (tmp > (dieroll = rnd(20)) || u.uswallow)) != 0) {
			    int compat;

			    if (!u.uswallow &&
				(compat=could_seduce(&youmonst, mon, mattk))) {
#if 0 /*JP*/
				You("%s %s %s.",
				    mon->mcansee && haseyes(mon->data)
				    ? "smile at" : "talk to",
				    mon_nam(mon),
				    compat == 2 ? "engagingly":"seductively");
#else
				You("%sへ%s%s．",
				    mon_nam(mon),
				    compat == 2 ? "魅力的に":"誘惑的に",
				    mon->mcansee && haseyes(mon->data)
				    ? "微笑みかけた" : "話しかけた");
#endif
				/* doesn't anger it; no wakeup() */
				sum[i] = hit_notouch[damageum(mon, mattk)];
				break;
			    }
			    wakeup(mon);
			    /* maybe this check should be in damageum()? */
			    if (mon->data == &mons[PM_SHADE] &&
					!(mattk->aatyp == AT_KICK &&
					    uarmf && uarmf->blessed)) {
/*JP
				Your("attack passes harmlessly through %s.",
*/
				Your("攻撃は%sを傷つけずにすり抜けた．",
				    mon_nam(mon));
				break;
			    }
			    if (mattk->aatyp == AT_KICK)
/*JP
				    You("kick %s.", mon_nam(mon));
*/
				    You("%sを蹴った．", mon_nam(mon));
			    else if (mattk->aatyp == AT_BITE)
/*JP
				    You("bite %s.", mon_nam(mon));
*/
				    You("%sに噛みついた．", mon_nam(mon));
			    else if (mattk->aatyp == AT_STNG)
/*JP
				    You("sting %s.", mon_nam(mon));
*/
				    You("%sを突き刺した．", mon_nam(mon));
			    else if (mattk->aatyp == AT_BUTT)
/*JP
				    You("butt %s.", mon_nam(mon));
*/
				    You("%sに頭突きをくらわした．", mon_nam(mon));
			    else if (mattk->aatyp == AT_TUCH)
/*JP
				    You("touch %s.", mon_nam(mon));
*/
				    You("%sに触れた．", mon_nam(mon));
			    else if (mattk->aatyp == AT_TENT)
/*JP
				    Your("tentacles suck %s.", mon_nam(mon));
*/
				    Your("触手が%sの体液を吸いとった．", mon_nam(mon));
/*JP
			    else You("hit %s.", mon_nam(mon));
*/
			    else Your("%sへの攻撃は命中した．", mon_nam(mon));
			    sum[i] = hit_touch[damageum(mon, mattk)];
			} else
			    missum(mon, tmp, dieroll, mattk);
			break;

		case AT_HUGS:
			/* automatic if prev two attacks succeed, or if
			 * already grabbed in a previous attack
			 */
			dhit = 1;
			wakeup(mon);
			if (mon->data == &mons[PM_SHADE])
/*JP
			    Your("hug passes harmlessly through %s.",
*/
			    You("%sを羽交い絞めにしようとしたが通り抜けた．",
				mon_nam(mon));
			else if (!sticks(mon->data) && !u.uswallow) {
			    if (mon==u.ustuck) {
#if 0 /*JP*/
				pline("%s is being %s.", Monnam(mon),
				    u.umonnum==PM_ROPE_GOLEM ?
				    breathless(mon->data) ? "strangled" :
				    "choked" : "crushed");
#else
				pline("%sは%s．", Monnam(mon),
				    u.umonnum==PM_ROPE_GOLEM ?
				    breathless(mon->data) ? "巻きつかれている" :
				    "首を絞められている" : "押し潰されている");
#endif
				sum[i] = hit_touch[damageum(mon, mattk)];
			    } else if(i >= 2 && sum[i-1] && sum[i-2]) {
/*JP
				You("grab %s!", mon_nam(mon));
*/
				You("%sをつかまえた！", mon_nam(mon));
				setustuck(mon);
				sum[i] = hit_touch[damageum(mon, mattk)];
			    }
			}
			break;

		case AT_EXPL:	/* automatic hit if next to */
			dhit = -1;
			wakeup(mon);
			sum[i] = hit_notouch[explum(mon, mattk)];
			break;

		case AT_ENGL:
			if((dhit = (tmp > (dieroll = rnd(20+i))))) {
				wakeup(mon);
				if (mon->data == &mons[PM_SHADE])
/*JP
				    Your("attempt to surround %s is harmless.",
*/
				    You("%sを飲みこもうとしたが失敗した．",
					mon_nam(mon));
				else {
				    sum[i]= hit_touch[gulpum(mon,mattk)];
				    if (sum[i] & HIT_FATAL &&
					    (mon->data->mlet == S_ZOMBIE ||
						mon->data->mlet == S_MUMMY) &&
					    rn2(5) &&
					    !Sick_resistance) {
#if 0 /*JP*/
					You_feel("%ssick.",
					    (Sick) ? "very " : "");
#else
					You_feel("%s気分が悪い．",
					    (Sick) ? "とても" : "");
#endif
					mdamageu(mon, rnd(8));
				    }
				}
			} else
				missum(mon, tmp, dieroll, mattk);
			break;

		case AT_MAGC:
			/* No check for uwep; if wielding nothing we want to
			 * do the normal 1-2 points bare hand damage...
			 */
			if (i == 0 && (youmonst.data->mlet==S_KOBOLD
				|| youmonst.data->mlet==S_ORC
				|| youmonst.data->mlet==S_GNOME
				)) goto use_weapon;

		case AT_NONE:
		case AT_BOOM:
			continue;
			/* Not break--avoid passive attacks from enemy */

		case AT_BREA:
		case AT_SPIT:
			dhit = 0;
			break;

		case AT_GAZE:   /* WAC -- can be either ranged attack OR close */
			if (Blind) {
				dhit = 0;
				break;
			}
			if (!canseemon(mon) && rn2(3)) {
#if 0 /*JP*/
				You("gaze around,  but miss!");
#else
				You("あたりを睨みつけたが外した！");
#endif
				dhit = 0;
				break;
			}
#if 0 /*JP*/
			You("gaze at %s...", mon_nam(mon));
#else
			You("%sを睨みつけた．．．", mon_nam(mon));
#endif

			if ((mon->data==&mons[PM_MEDUSA]) && !mon->mcan) {
#if 0 /*JP*/
				pline("Gazing at the awake Medusa is not a very good idea.");
#else
				pline("メデューサを睨むのは全く良くない考えだ．");
#endif
				/* as if gazing at a sleeping anything is fruitful... */
#if 0 /*JP*/
				You("turn to stone...");
#else
				You("石になった．．．");
#endif
				killer_format = KILLED_BY;
#if 0 /*JP*/
				killer = "deliberately gazing at Medusa's hideous countenance";
#else
				killer = "恐ろしいメデューサの顔を故意に見つめて";
#endif
				done(STONING);
			} else if (!mon->mcansee || mon->msleeping) {
/*JP
				pline("But nothing happens.");
*/
				pline("何も起こらなかった．");
				dhit = 0;
				break;
			} else if (Invis && !perceives(mon->data)) {
/*JP
				pline("%s seems not to notice your gaze.", Monnam(mon));
*/
				pline("%sはあなたの睨みに気付いていないようだ．", Monnam(mon));
				break;
			}
			sum[i] = hit_notouch[damageum(mon, mattk)];
			break;

		case AT_MULTIPLY:
			/* Not a #monster ability -- this is something that the
			 * player must figure out -RJ */
			cloneu();
			break;

		default: /* Strange... */
			impossible("strange attack of yours (%d)",
				 mattk->aatyp);
	    }
	    if (dhit == -1) {
		u.mh = -1;	/* dead in the current form */
		rehumanize();
	    }
	    if (sum[i] & HIT_FATAL)
		return((boolean)passive(mon, sum[i], 0, mattk->aatyp));
							/* defender dead */
	    else {
		(void) passive(mon, sum[i], 1, mattk->aatyp);
		nsum |= sum[i];
	    }
	    if (Upolyd != Old_Upolyd)
		break; /* No extra attacks if no longer a monster */
	    if (multi < 0)
		break; /* If paralyzed while attacking, i.e. floating eye */
	}
	return((boolean)(nsum != 0));
}

/*	Special (passive) attacks on you by monsters done here.		*/

int
passive(mon, mhit, malive, aatyp)
register struct monst *mon;
register int mhit;
register int malive;
uchar aatyp;
{
	register struct permonst *ptr = mon->data;
	register int i, tmp;
	struct obj *target = mhit & HIT_UWEP ? uwep :
		mhit & HIT_USWAPWEP ? uswapwep : (struct obj *)0;
/*	char buf[BUFSZ]; */


	if (mhit && aatyp == AT_BITE && is_vampire(youmonst.data)) {
	    if (bite_monster(mon))
		return 2;			/* lifesaved */
	}
	for(i = 0; ; i++) {
	    if(i >= NATTK) return(malive | mhit);	/* no passive attacks */
	    if(ptr->mattk[i].aatyp == AT_NONE /*||
	       ptr->mattk[i].aatyp == AT_BOOM*/) break; /* try this one */
	}
	/* Note: tmp not always used */
	if (ptr->mattk[i].damn)
	    tmp = d((int)ptr->mattk[i].damn, (int)ptr->mattk[i].damd);
	else if(ptr->mattk[i].damd)
	    tmp = d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
	else
	    tmp = 0;

/*	These affect you even if they just died */
	switch(ptr->mattk[i].adtyp) {

	  case AD_ACID:
	    if(mhit && rn2(2)) {
/*JP
		if (Blind || !flags.verbose) You("are splashed!");
*/
		if (Blind || !flags.verbose) You("何かを浴びせられた！");
/*JP
		else	You("are splashed by %s acid!",
*/
		else	You("%sの酸を浴びせられた！", 
			                s_suffix(mon_nam(mon)));

		if (!Acid_resistance)
			mdamageu(mon, tmp);
		if(!rn2(30)) erode_armor(&youmonst, TRUE);
	    }
	    if (mhit) {
		if (aatyp == AT_KICK) {
		    if (uarmf && !rn2(6))
			(void)rust_dmg(uarmf, xname(uarmf), 3, TRUE, &youmonst);
		} else if (aatyp == AT_WEAP || aatyp == AT_CLAW ||
			   aatyp == AT_MAGC || aatyp == AT_TUCH)
		    passive_obj(mon, target, &(ptr->mattk[i]));
	    }
	    exercise(A_STR, FALSE);
	    break;
	  case AD_STON:
	    if (mhit) {		/* successful attack */
		long protector = attk_protection((int)aatyp);
		boolean barehanded = mhit & HIT_BODY ||
			mhit & HIT_UWEP && !uwep ||
			mhit & HIT_USWAPWEP && !uswapwep;

		/* hero using monsters' AT_MAGC attack is hitting hand to
		   hand rather than casting a spell */
		if (aatyp == AT_MAGC) protector = W_ARMG;

		if (protector == 0L ||		/* no protection */
			(protector == W_ARMG && !uarmg && barehanded) ||
			(protector == W_ARMF && !uarmf) ||
			(protector == W_ARMH && !uarmh) ||
			(protector == (W_ARMC|W_ARMG) && (!uarmc || !uarmg))) {
		if (!Stone_resistance &&
			    !(poly_when_stoned(youmonst.data) &&
				polymon(PM_STONE_GOLEM))) {
/*JP
			You("turn to stone...");
*/
			You("石になった．．．");
			done_in_by(mon);
			return 2;
		}
	      }
	    }
	    break;
	  case AD_RUST:
	    if(mhit && !mon->mcan) {
		if (aatyp == AT_KICK) {
		    if (uarmf)
			(void)rust_dmg(uarmf, xname(uarmf), 1, TRUE, &youmonst);
		} else if (aatyp == AT_WEAP || aatyp == AT_CLAW ||
			   aatyp == AT_MAGC || aatyp == AT_TUCH)
		    passive_obj(mon, target, &(ptr->mattk[i]));
	    }
	    break;
	  case AD_CORR:
	    if(mhit && !mon->mcan) {
		if (aatyp == AT_KICK) {
		    if (uarmf)
			(void)rust_dmg(uarmf, xname(uarmf), 3, TRUE, &youmonst);
		} else if (aatyp == AT_WEAP || aatyp == AT_CLAW ||
			   aatyp == AT_MAGC || aatyp == AT_TUCH)
		    passive_obj(mon, target, &(ptr->mattk[i]));
	    }
	    break;
	  case AD_MAGM:
	    /* wrath of gods for attacking Oracle */
	    if(Antimagic) {
		shieldeff(u.ux, u.uy);
/*JP
		pline("A hail of magic missiles narrowly misses you!");
*/
		pline("魔法の矢の雨をなんとかかわした！");
	    } else {
/*JP
		You("are hit by magic missiles appearing from thin air!");
*/
		pline("突如空中に現れた魔法の矢が命中した！");
		mdamageu(mon, tmp);
	    }
	    break;
	  default:
	    break;
	}

/*	These only affect you if they still live */

	if(malive && !mon->mcan && rn2(3)) {

	    switch(ptr->mattk[i].adtyp) {

	      case AD_DRST:
	       if (!Strangled && !Breathless) {
#if 0 /*JP*/
		 pline("You inhale a cloud of spores!");
		 poisoned("spores", A_STR, "spore cloud", 30);
#else
		 pline("あなたは胞子の雲を吸い込んでしまった！");
		 poisoned("胞子", A_STR, "胞子の雲", 30);
#endif
	       } else {
#if 0 /*JP*/
		 pline("A cloud of spores surrounds you!");
#else
		 pline("胞子の雲はあなたを取り巻いた！");
#endif
	       }
	      break;
	      case AD_PLYS:
		if(ptr == &mons[PM_FLOATING_EYE]) {
		    if (!canseemon(mon)) {
			break;
		    }
		    if(mon->mcansee) {
/*JP
			if (ureflects("%s gaze is reflected by your %s.",
*/
		    if (ureflects("%sのにらみは%sによって反射された．",
				    s_suffix(Monnam(mon))))
			    ;
			else if (Free_action)
/*JP
			    You("momentarily stiffen under %s gaze!",
*/
			    You("%sのにらみで一瞬硬直した！",
				    s_suffix(mon_nam(mon)));
			else {
/*JP
			    You("are frozen by %s gaze!",
*/
			    You("%sのにらみで動けなくなった！",
				  s_suffix(mon_nam(mon)));
			    nomul((ACURR(A_WIS) > 12 || rn2(4)) ? -tmp : -127);
			}
		    } else {
#if 0 /*JP*/
			pline("%s cannot defend itself.",
				Adjmonnam(mon,"blind"));
#else
			pline("%sは防御できない．",
				Adjmonnam(mon,"目の見えない"));
#endif
			if(!rn2(500)) change_luck(-1);
		    }
		} else if (Free_action) {
/*JP
		    You("momentarily stiffen.");
*/
		    You("一瞬硬直した．");
		} else { /* gelatinous cube */
/*JP
		    You("are frozen by %s!", mon_nam(mon));
*/
		    You("%sによって動けなくなった！", mon_nam(mon));
	    	    nomovemsg = 0;	/* default: "you can move again" */
		    nomul(-tmp);
		    exercise(A_DEX, FALSE);
		}
		break;
	      case AD_COLD:		/* brown mold or blue jelly */
		if(monnear(mon, u.ux, u.uy)) {
		    if(Cold_resistance) {
			shieldeff(u.ux, u.uy);
/*JP
			You_feel("a mild chill.");
*/
			You("ひんやりした．");
			ugolemeffects(AD_COLD, tmp);
			break;
		    }
/*JP
		    You("are suddenly very cold!");
*/
		    You("突然，猛烈に寒くなった！");
		    mdamageu(mon, tmp);
		/* monster gets stronger with your heat! */
		    mon->mhp += tmp / 2;
		    if (mon->mhpmax < mon->mhp) mon->mhpmax = mon->mhp;
		/* at a certain point, the monster will reproduce! */
		    if(mon->mhpmax > ((int) (mon->m_lev+1) * 8))
			(void)split_mon(mon, &youmonst);
		}
		break;
	      case AD_STUN:		/* specifically yellow mold */
		if(!Stunned)
		    make_stunned((long)tmp, TRUE);
		break;
	      case AD_FIRE:
		if(monnear(mon, u.ux, u.uy)) {
		    if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
/*JP
			You_feel("mildly warm.");
*/
			You("暖かさを感じた．");
			ugolemeffects(AD_FIRE, tmp);
			break;
		    }
/*JP
		    You("are suddenly very hot!");
*/
		    You("突然，猛烈に熱くなった！");
		    mdamageu(mon, tmp);
		}
		break;
	      case AD_ELEC:
		if(Shock_resistance) {
		    shieldeff(u.ux, u.uy);
/*JP
		    You_feel("a mild tingle.");
*/
		    You("ピリピリと痺れを感じた．");
		    ugolemeffects(AD_ELEC, tmp);
		    break;
		}
/*JP
		You("are jolted with electricity!");
*/
		You("電気ショックをうけた！");
		mdamageu(mon, tmp);
		break;
	      case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
		if (mhit) {
		    struct obj *obj = target;

		    if (aatyp == AT_KICK) {
			obj = uarmf;
			if (!obj) break;
		    } else if (aatyp == AT_BITE || aatyp == AT_BUTT ||
			       (aatyp >= AT_STNG && aatyp < AT_WEAP)) {
			break;		/* no object involved */
		    } else if (!obj && mhit & (HIT_UWEP | HIT_USWAPWEP))
			obj = uarmg;
		    passive_obj(mon, obj, &(ptr->mattk[i]));
	    	}
	    	break;
	      default:
		break;
	    }
	}
	return(malive | mhit);
}

/*
 * Special (passive) attacks on an attacking object by monsters done here.
 * Assumes the attack was successful.
 */
void
passive_obj(mon, obj, mattk)
register struct monst *mon;
register struct obj *obj;	/* null means pick uwep, uswapwep or uarmg */
struct attack *mattk;		/* null means we find one internally */
{
	register struct permonst *ptr = mon->data;
	register int i;

#if 0
	/* if caller hasn't specified an object, use uwep, uswapwep or uarmg */
	if (!obj) {
	    obj = (u.twoweap && uswapwep && !rn2(2)) ? uswapwep : uwep;
	    if (!obj && mattk->adtyp == AD_ENCH)
		obj = uarmg;		/* no weapon? then must be gloves */
	    if (!obj) return;		/* no object to affect */
	}
#else
	/* In Slash'EM, the caller always specifies the object */
	if (!obj) return;		/* no object to affect */
#endif

	/* if caller hasn't specified an attack, find one */
	if (!mattk) {
	    for(i = 0; ; i++) {
		if(i >= NATTK) return;	/* no passive attacks */
		if(ptr->mattk[i].aatyp == AT_NONE) break; /* try this one */
	    }
	    mattk = &(ptr->mattk[i]);
	}

	switch(mattk->adtyp) {

	case AD_ACID:
	    if(!rn2(6)) {
		erode_obj(obj, TRUE, FALSE);
	    }
	    break;
	case AD_RUST:
	    if(!mon->mcan) {
		erode_obj(obj, FALSE, FALSE);
	    }
	    break;
	case AD_CORR:
	    if(!mon->mcan) {
		erode_obj(obj, TRUE, FALSE);
	    }
	    break;
	case AD_ENCH:
	    if (!mon->mcan) {
		if (drain_item(obj) && carried(obj) &&
		    (obj->known || obj->oclass == ARMOR_CLASS)) {
/*JP
		    Your("%s less effective.", aobjnam(obj, "seem"));
*/
		    Your("%sから魔力が消えた．", xname(obj));
	    	}
	    	break;
	    }
	  default:
	    break;
	}

	if (carried(obj)) update_inventory();
}

/* Note: caller must ascertain mtmp is mimicking... */
void
stumble_onto_mimic(mtmp)
struct monst *mtmp;
{
#if 0 /*JP*/
	const char *fmt = "Wait!  That's %s!",
		   *generic = "a monster",
		   *what = 0;
#else
	const char *fmt = "待て！それは%sだ！",
		   *generic = "怪物",
		   *what = 0;
#endif

	if(!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data,AD_STCK))
	    setustuck(mtmp);

	if (Blind) {
	    if (!Blind_telepat)
		what = generic;		/* with default fmt */
	    else if (mtmp->m_ap_type == M_AP_MONSTER)
		what = a_monnam(mtmp);	/* differs from what was sensed */
	} else {
#ifdef DISPLAY_LAYERS
	    if (levl[u.ux+u.dx][u.uy+u.dy].mem_bg == S_hcdoor ||
		    levl[u.ux+u.dx][u.uy+u.dy].mem_bg == S_vcdoor)
/*JP
		fmt = "The door actually was %s!";
*/
		fmt = "扉は実は%sだった！";
	    else if (levl[u.ux+u.dx][u.uy+u.dy].mem_obj == GOLD_PIECE)
/*JP
		fmt = "That gold was %s!";
*/
		fmt = "金貨は%sだった！";
#else
	    int glyph = levl[u.ux+u.dx][u.uy+u.dy].glyph;

	    if (glyph_is_cmap(glyph) &&
		    (glyph_to_cmap(glyph) == S_hcdoor ||
		     glyph_to_cmap(glyph) == S_vcdoor))
/*JP
		fmt = "The door actually was %s!";
*/
		fmt = "扉は実は%sだった！";
	    else if (glyph_is_object(glyph) &&
		    glyph_to_obj(glyph) == GOLD_PIECE)
/*JP
		fmt = "That gold was %s!";
*/
		fmt = "金貨は%sだった！";
#endif

	    /* cloned Wiz starts out mimicking some other monster and
	       might make himself invisible before being revealed */
	    if (mtmp->minvis && !See_invisible)
		what = generic;
	    else
		what = a_monnam(mtmp);
	}
	if (what) pline(fmt, what);

	wakeup(mtmp);	/* clears mimicking */
}

STATIC_OVL void
nohandglow(mon)
struct monst *mon;
{
	char *hands=makeplural(body_part(HAND));

	if (!u.umconf || mon->mconf) return;
	if (u.umconf == 1) {
		if (Blind)
/*JP
			Your("%s stop tingling.", hands);
*/
			Your("%sの痺れがとれた．", hands);
		else
/*JP
			Your("%s stop glowing %s.", hands, hcolor(NH_RED));
*/
			Your("%sの%s輝きはなくなった．", hands, hcolor(NH_RED));
	} else {
		if (Blind)
/*JP
			pline_The("tingling in your %s lessens.", hands);
*/
			pline("%sの痺れがとれてきた．",hands);
		else
/*JP
			Your("%s no longer glow so brightly %s.", hands,
*/
			Your("%sの%s輝きがなくなってきた．",hands,
				hcolor(NH_RED));
	}
	u.umconf--;
}

int
flash_hits_mon(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;	/* source of flash */
{
	int tmp, amt, res = 0, useeit = canseemon(mtmp);

	if (mtmp->msleeping) {
	    mtmp->msleeping = 0;
	    if (useeit) {
/*JP
		pline_The("flash awakens %s.", mon_nam(mtmp));
*/
		pline("まばゆい光で%sが目を覚ました．", mon_nam(mtmp));
		res = 1;
	    }
	} else if (mtmp->data->mlet != S_LIGHT) {
	    if (!resists_blnd(mtmp)) {
		tmp = dist2(otmp->ox, otmp->oy, mtmp->mx, mtmp->my);
		if (useeit) {
/*JP
		    pline("%s is blinded by the flash!", Monnam(mtmp));
*/
		    pline("%sはまばゆい光で目がくらんだ！", Monnam(mtmp));
		    res = 1;
		}
		if (mtmp->data == &mons[PM_GREMLIN]) {
		    /* Rule #1: Keep them out of the light. */
		    amt = otmp->otyp == WAN_LIGHT ? d(1 + otmp->spe, 4) :
		          rn2(min(mtmp->mhp,4));
#if 0 /*JP*/
		    pline("%s %s!", Monnam(mtmp), amt > mtmp->mhp / 2 ?
			  "wails in agony" : "cries out in pain");
#else
		    pline("%sは%s！", Monnam(mtmp), amt > mtmp->mhp / 2 ?
			  "苦痛の声をあげた" : "激痛で叫んだ");
#endif
		    if ((mtmp->mhp -= amt) <= 0) {
			if (flags.mon_moving)
			    monkilled(mtmp, (char *)0, AD_BLND);
			else
			    killed(mtmp);
		    } else if (cansee(mtmp->mx,mtmp->my) && !canspotmon(mtmp)){
			map_invisible(mtmp->mx, mtmp->my);
		    }
		}
		if (mtmp->mhp > 0) {
		    if (!flags.mon_moving) setmangry(mtmp);
		    if (tmp < 9 && !mtmp->isshk && rn2(4)) {
			if (rn2(4))
			    monflee(mtmp, rnd(100), FALSE, TRUE);
			else
			    monflee(mtmp, 0, FALSE, TRUE);
		    }
		    mtmp->mcansee = 0;
		    mtmp->mblinded = (tmp < 3) ? 0 : rnd(1 + 50/tmp);
		}
	    }
	}
	return res;
}
/*uhitm.c*/
