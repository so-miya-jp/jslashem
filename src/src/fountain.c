/*	SCCS Id: @(#)fountain.c	3.4	2003/03/23	*/
/*	Copyright Scott R. Turner, srt@ucla, 10/27/86 */
/* NetHack may be freely redistributed.  See license for details. */

/* Code for drinking from fountains. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, 2004-2006
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

STATIC_DCL void NDECL(dowatersnakes);
STATIC_DCL void NDECL(dowaterdemon);
STATIC_DCL void NDECL(dowaternymph);
STATIC_PTR void FDECL(gush, (int,int,genericptr_t));
STATIC_DCL void NDECL(dofindgem);

void
floating_above(what)
const char *what;
{
/*JP
    You("are floating high above the %s.", what);
*/
    You("%sの遥か上方に浮いている．", what);
}

STATIC_OVL void
dowatersnakes() /* Fountain of snakes! */
{
    register int num = rn1(5,2);
    struct monst *mtmp;

    if (!(mvitals[PM_WATER_MOCCASIN].mvflags & G_GONE)) {
	if (!Blind)
#if 0 /*JP*/
	    pline("An endless stream of %s pours forth!",
		  Hallucination ? makeplural(rndmonnam()) : "snakes");
#else
	    pline("%sの群れがとめどなく流れ出てきた！",
		  Hallucination ? makeplural(rndmonnam()) : "蛇");
#endif
	else
/*JP
	    You_hear("%s hissing!", something);
*/
	    You_hear("シーッという音を聞いた！");
	while(num-- > 0)
	    if((mtmp = makemon(&mons[PM_WATER_MOCCASIN],
			u.ux, u.uy, NO_MM_FLAGS)) && t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
    } else
/*JP
	pline_The("fountain bubbles furiously for a moment, then calms.");
*/
	pline("泉は突然激しく泡だち，やがて静かになった．");
}

STATIC_OVL
void
dowaterdemon() /* Water demon */
{
    register struct monst *mtmp;

    if(!(mvitals[PM_WATER_DEMON].mvflags & G_GONE)) {
	if((mtmp = makemon(&mons[PM_WATER_DEMON],u.ux,u.uy, NO_MM_FLAGS))) {
	    if (!Blind)
#if 0 /*JP*/
		You("unleash %s!", a_monnam(mtmp));
#else
		You("%sを解き放した！", a_monnam(mtmp));
#endif
	    else
#if 0 /*JP*/
		You_feel("the presence of evil.");
#else
		You_feel("邪悪な存在を感じた！");
#endif
/* ------------===========STEPHEN WHITE'S NEW CODE============------------ */
	/* Give those on low levels a (slightly) better chance of survival */
	/* 35% at level 1, 30% at level 2, 25% at level 3, etc... */            
	if (rnd(100) > (60 + 5*level_difficulty())) {
#if 0 /*JP*/
		pline("Grateful for %s release, %s grants you a wish!",
		      mhis(mtmp), mhe(mtmp));
#else
		pline("%sは解放をとても感謝し，のぞみをかなえてくれるようだ！",
		      mhe(mtmp));
#endif
		makewish();
		mongone(mtmp);
	    } else if (t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
	}
    } else
#if 0 /*JP*/
	pline_The("fountain bubbles furiously for a moment, then calms.");
#else
	pline("泉は突然激しく泡だち，やがて静かになった．");
#endif
}

STATIC_OVL void
dowaternymph() /* Water Nymph */
{
	register struct monst *mtmp;

	if(!(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
	   (mtmp = makemon(&mons[PM_WATER_NYMPH],u.ux,u.uy, NO_MM_FLAGS))) {
		if (!Blind)
/*JP
		   You("attract %s!", a_monnam(mtmp));
*/
		   pline("%sを呼び寄せてしまった！", a_monnam(mtmp));
		else
/*JP
		   You_hear("a seductive voice.");
*/
		   You_hear("魅惑的な声を聞いた．");
		mtmp->msleeping = 0;
		if (t_at(mtmp->mx, mtmp->my))
		    (void) mintrap(mtmp);
	} else
		if (!Blind)
/*JP
		   pline("A large bubble rises to the surface and pops.");
*/
		   pline("大きな泡が沸き出てはじけた．");
		else
/*JP
		   You_hear("a loud pop.");
*/
		   You_hear("大きな泡がはじける音を聞いた．");
}

void
dogushforth(drinking) /* Gushing forth along LOS from (u.ux, u.uy) */
int drinking;
{
	int madepool = 0;

	do_clear_area(u.ux, u.uy, 7, gush, (genericptr_t)&madepool);
	if (!madepool) {
	    if (drinking)
/*JP
		Your("thirst is quenched.");
*/
		Your("渇きは癒された．");
	    else
/*JP
		pline("Water sprays all over you.");
*/
		pline("水しぶきがあなたにかかった．");
	}
}

STATIC_PTR void
gush(x, y, poolcnt)
int x, y;
genericptr_t poolcnt;
{
	register struct monst *mtmp;
	register struct trap *ttmp;

	if (((x+y)%2) || (x == u.ux && y == u.uy) ||
	    (rn2(1 + distmin(u.ux, u.uy, x, y)))  ||
	    (levl[x][y].typ != ROOM) ||
	    (sobj_at(BOULDER, x, y)) || nexttodoor(x, y))
		return;

	if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
		return;

	if (!((*(int *)poolcnt)++))
/*JP
	    pline("Water gushes forth from the overflowing fountain!");
*/
	    pline("泉から水がどどっと溢れ出た！");

	/* Put a pool at x, y */
	levl[x][y].typ = POOL;
	/* No kelp! */
	del_engr_at(x, y);
	water_damage(level.objects[x][y], FALSE, TRUE);

	if ((mtmp = m_at(x, y)) != 0)
		(void) minliquid(mtmp);
	else
		newsym(x,y);
}

STATIC_OVL void
dofindgem() /* Find a gem in the sparkling waters. */
{
/*JP
	if (!Blind) You("spot a gem in the sparkling waters!");
*/
	if (!Blind) pline("きらめく水の中に宝石を見つけた！");
/*JP
	else You_feel("a gem here!");
*/
	else pline("宝石があるようだ！");
	(void) mksobj_at(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE-1),
			 u.ux, u.uy, FALSE, FALSE);
	SET_FOUNTAIN_LOOTED(u.ux,u.uy);
	newsym(u.ux, u.uy);
	exercise(A_WIS, TRUE);			/* a discovery! */
}

void
dryup(x, y, isyou)
xchar x, y;
boolean isyou;
{
	if (IS_FOUNTAIN(levl[x][y].typ) &&
	    (!rn2(3) || FOUNTAIN_IS_WARNED(x,y))) {
		if(isyou && in_town(x, y) && !FOUNTAIN_IS_WARNED(x,y)) {
			struct monst *mtmp;
			SET_FOUNTAIN_WARNED(x,y);
			/* Warn about future fountain use. */
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
			    if (DEADMONSTER(mtmp)) continue;
			    if ((mtmp->data == &mons[PM_WATCHMAN] ||
				mtmp->data == &mons[PM_WATCH_CAPTAIN]) &&
			       couldsee(mtmp->mx, mtmp->my) &&
			       mtmp->mpeaceful) {
/*JP
				pline("%s yells:", Amonnam(mtmp));
*/
				pline("%sは叫んだ：", Amonnam(mtmp));
/*JP
				verbalize("Hey, stop using that fountain!");
*/
				verbalize("おい，泉を汚すな！");
				break;
			    }
			}
			/* You can see or hear this effect */
/*JP
			if(!mtmp) pline_The("flow reduces to a trickle.");
*/
			if(!mtmp) pline("流れはちょろちょろになった．");
			return;
		}
#ifdef WIZARD
		if (isyou && wizard) {
/*JP
			if (yn("Dry up fountain?") == 'n')
*/
			if (yn("泉を飲みほしますか？") == 'n')
				return;
		}
#endif
		/* replace the fountain with ordinary floor */
		levl[x][y].typ = ROOM;
		levl[x][y].looted = 0;
		levl[x][y].blessedftn = 0;
/*JP
		if (cansee(x,y)) pline_The("fountain dries up!");
*/
		if (cansee(x,y)) pline("泉は干上がった！");
		/* The location is seen if the hero/monster is invisible */
		/* or felt if the hero is blind.			 */
		newsym(x, y);
		level.flags.nfountains--;
		if(isyou && in_town(x, y))
		    (void) angry_guards(FALSE);
	}
}

void
drinkfountain()
{
	/* What happens when you drink from a fountain? */
	register boolean mgkftn = (levl[u.ux][u.uy].blessedftn == 1);
	register int fate = rnd(30);

	if (Levitation) {
/*JP
		floating_above("fountain");
*/
		floating_above("泉");
		return;
	}

	if (mgkftn && u.uluck >= 0 && fate >= 10) {
		int i, ii, littleluck = (u.uluck < 4);

/*JP
		pline("Wow!  This makes you feel great!");
*/
		pline("ワォ！とても気持ちよくなった！");
		/* blessed restore ability */
		for (ii = 0; ii < A_MAX; ii++)
		    if (ABASE(ii) < AMAX(ii)) {
			ABASE(ii) = AMAX(ii);
			flags.botl = 1;
		    }
		/* gain ability, blessed if "natural" luck is high */
		i = rn2(A_MAX);		/* start at a random attribute */
		for (ii = 0; ii < A_MAX; ii++) {
		    if (adjattrib(i, 1, littleluck ? -1 : 0) && littleluck)
			break;
		    if (++i >= A_MAX) i = 0;
		}
		display_nhwindow(WIN_MESSAGE, FALSE);
/*JP
		pline("A wisp of vapor escapes the fountain...");
*/
		pline("煙のかたまりが泉から逃げた．．．");
		exercise(A_WIS, TRUE);
		levl[u.ux][u.uy].blessedftn = 0;
		return;
	}

	if (fate < 10) {
/*JP
		pline_The("cool draught refreshes you.");
*/
		pline("冷たい一杯でさっぱりした．");
		u.uhunger += rnd(10); /* don't choke on water */
		newuhs(FALSE);
		if(mgkftn) return;
	} else {
	    switch (fate) {

		case 19: /* Self-knowledge */

/*JP
			You_feel("self-knowledgeable...");
*/
			You("自分自身が判るような気がした．．．");
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(0);
			exercise(A_WIS, TRUE);
/*JP
			pline_The("feeling subsides.");
*/
			pline("その感じはなくなった．");
			break;

		case 20: /* Foul water */

/*JP
			pline_The("water is foul!  You gag and vomit.");
*/
			pline("水はひどく不快な味がした！あなたは吐き戻した．");
			morehungry(rn1(20, 11));
			vomit();
			break;

		case 21: /* Poisonous */

/*JP
			pline_The("water is contaminated!");
*/
			pline("水は汚染されている！");
			if (Poison_resistance) {
#if 0 /*JP*/
			   pline(
			      "Perhaps it is runoff from the nearby %s farm.",
				 fruitname(FALSE));
#else
			   pline(
			      "たぶん，これは近くの%sの農場から流れている．",
				 fruitname(FALSE));
#endif
/*JP
			   losehp(rnd(4),"unrefrigerated sip of juice",
*/
			   losehp(rnd(4),"腐ったジュースを飲んで",
				KILLED_BY_AN);
			   break;
			}
			losestr(rn1(4,3));
/*JP
			losehp(rnd(10),"contaminated water", KILLED_BY);
*/
			losehp(rnd(10),"汚染された水を飲んで", KILLED_BY);
			exercise(A_CON, FALSE);
			break;

		case 22: /* Fountain of snakes! */

			dowatersnakes();
			break;

		case 23: /* Water demon */
			dowaterdemon();
			break;

		case 24: /* Curse an item */ {
			register struct obj *obj;

/*JP
			pline("This water's no good!");
*/
			pline("この水はとてもまずい！");
			morehungry(rn1(20, 11));
			exercise(A_CON, FALSE);
			for(obj = invent; obj ; obj = obj->nobj)
				if (!rn2(5))	curse(obj);
			break;
			}

		case 25: /* See invisible */

			if (Blind) {
			    if (Invisible) {
/*JP
				You("feel transparent.");
*/
			    You("透明になった気がする．");
			    } else {
/*JP
			    	You("feel very self-conscious.");
*/
			    	You("自意識過剰に感じた．");
/*JP
			    	pline("Then it passes.");
*/
			    	pline("その感じは消えた．");
			    }
			} else {
/*JP
			   You("see an image of someone stalking you.");
*/
			   You("何かが自分の後をつけている映像を見た．");
/*JP
			   pline("But it disappears.");
*/
			   pline("しかし，それは消えてしまった．");
			}
			HSee_invisible |= FROMOUTSIDE;
			newsym(u.ux,u.uy);
			exercise(A_WIS, TRUE);
			break;

		case 26: /* See Monsters */

			(void) monster_detect((struct obj *)0, 0);
			exercise(A_WIS, TRUE);
			break;

		case 27: /* Find a gem in the sparkling waters. */

			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}

		case 28: /* Water Nymph */

			dowaternymph();
			break;

		case 29: /* Scare */ {
			register struct monst *mtmp;

/*JP
			pline("This water gives you bad breath!");
*/
			pline("水を飲んだら息が臭くなった！");
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			    if(!DEADMONSTER(mtmp))
				monflee(mtmp, 0, FALSE, FALSE);
			}
			break;

		case 30: /* Gushing forth in this room */

			dogushforth(TRUE);
			break;

		default:

/*JP
			pline("This tepid water is tasteless.");
*/
			pline("このなまぬるい水は味がない．");
			break;
	    }
	}
	dryup(u.ux, u.uy, TRUE);
}

void
dipfountain(obj)
register struct obj *obj;
{
	if (Levitation) {
/*JP
		floating_above("fountain");
*/
		floating_above("泉");
		return;
	}

	/* Don't grant Excalibur when there's more than one object.  */
	/* (quantity could be > 1 if merged daggers got polymorphed) */

	if (obj->otyp == LONG_SWORD && obj->quan == 1L
	    && u.ulevel > 4 && !rn2(8) && !obj->oartifact
	    && !exist_artifact(LONG_SWORD, artiname(ART_EXCALIBUR))) {

		if (u.ualign.type != A_LAWFUL) {
			/* Ha!  Trying to cheat her. */
/*JP
			pline("A freezing mist rises from the water and envelopes the sword.");
*/
			pline("氷の霧が水から立ち昇り，剣をつつんだ．");
/*JP
			pline_The("fountain disappears!");
*/
			pline("泉は消えてしまった！");
			curse(obj);
			if (obj->spe > -6 && !rn2(3)) obj->spe--;
			obj->oerodeproof = FALSE;
			exercise(A_WIS, FALSE);
		} else {
			/* The lady of the lake acts! - Eric Backus */
			/* Be *REAL* nice */
/*JP
	  pline("From the murky depths, a hand reaches up to bless the sword.");
*/
	  pline("にごった深みから，剣を祝福せんと手が伸びてきた．");
/*JP
			pline("As the hand retreats, the fountain disappears!");
*/
			pline("手が退くと，泉は消えてしまった！");
			obj = oname(obj, artiname(ART_EXCALIBUR));
			discover_artifact(ART_EXCALIBUR);
			bless(obj);
			obj->oeroded = obj->oeroded2 = 0;
			obj->oerodeproof = TRUE;
			exercise(A_WIS, TRUE);
		}
		update_inventory();
		levl[u.ux][u.uy].typ = ROOM;
		levl[u.ux][u.uy].looted = 0;
		newsym(u.ux, u.uy);
		level.flags.nfountains--;
		if(in_town(u.ux, u.uy))
		    (void) angry_guards(FALSE);
		return;
	} else if (get_wet(obj, FALSE) && !rn2(2))
		return;

	/* Acid and water don't mix */
	if (obj->otyp == POT_ACID) {
	    useup(obj);
	    return;
	}

	switch (rnd(30)) {
		case 10: /* Curse the item */
			curse(obj);
			break;
		case 11:
		case 12:
		case 13:
		case 14: /* Uncurse the item */
			if(obj->cursed) {
			    if (!Blind)
/*JP
				pline_The("water glows for a moment.");
*/
				pline("水は輝きだした．");
			    uncurse(obj);
			} else {
/*JP
			    pline("A feeling of loss comes over you.");
*/
			    pline("奇妙な脱力感があなたをおそった．");
			}
			break;
		case 15:
		case 16: /* Water Demon */
			dowaterdemon();
			break;
		case 17:
		case 18: /* Water Nymph */
			dowaternymph();
			break;
		case 19:
		case 20: /* an Endless Stream of Snakes */
			dowatersnakes();
			break;
		case 21:
		case 22:
		case 23: /* Find a gem */
			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}
		case 24:
		case 25: /* Water gushes forth */
			dogushforth(FALSE);
			break;
		case 26: /* Strange feeling */
/*JP
			pline("A strange tingling runs up your %s.",
*/
			pline("奇妙なしびれがあなたの%sに走った．",
							body_part(ARM));
			break;
		case 27: /* Strange feeling */
/*JP
			You_feel("a sudden chill.");
*/
			You("突然寒けを感じた．");
			break;
		case 28: /* Strange feeling */
/*JP
			pline("An urge to take a bath overwhelms you.");
*/
			pline("水浴びをしたいという欲望にかられた．");
#ifndef GOLDOBJ
			if (u.ugold > 10) {
			    u.ugold -= somegold() / 10;
/*JP
			    You("lost some of your gold in the fountain!");
*/
			    You("金貨を数枚，泉に落としてしまった！");
			    CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			    exercise(A_WIS, FALSE);
			}
#else
			{
			    long money = money_cnt(invent);
			    struct obj *otmp;
                            if (money > 10) {
				/* Amount to loose.  Might get rounded up as fountains don't pay change... */
			        money = somegold(money) / 10; 
			        for (otmp = invent; otmp && money > 0; otmp = otmp->nobj) if (otmp->oclass == COIN_CLASS) {
				    int denomination = objects[otmp->otyp].oc_cost;
				    long coin_loss = (money + denomination - 1) / denomination;
                                    coin_loss = min(coin_loss, otmp->quan);
				    otmp->quan -= coin_loss;
				    money -= coin_loss * denomination;				  
				    if (!otmp->quan) delobj(otmp);
				}
/*JP
			        You("lost some of your money in the fountain!");
*/
			        You("お金を数枚，泉に落としてしまった！");
				CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			        exercise(A_WIS, FALSE);
                            }
			}
#endif
			break;
		case 29: /* You see coins */

		/* We make fountains have more coins the closer you are to the
		 * surface.  After all, there will have been more people going
		 * by.	Just like a shopping mall!  Chris Woodbury  */

		    if (FOUNTAIN_IS_LOOTED(u.ux,u.uy)) break;
		    SET_FOUNTAIN_LOOTED(u.ux,u.uy);
		    (void) mkgold((long)
			(rnd((dunlevs_in_dungeon(&u.uz)-dunlev(&u.uz)+1)*2)+5),
			u.ux, u.uy);
		    if (!Blind)
/*JP
		pline("Far below you, you see coins glistening in the water.");
*/
		You("遥か下の水中に金貨の輝きをみつけた．");
		    exercise(A_WIS, TRUE);
		    newsym(u.ux,u.uy);
		    break;
	}
	update_inventory();
	dryup(u.ux, u.uy, TRUE);
}

#ifdef SINKS
void
diptoilet(obj)
register struct obj *obj;
{
	if (Levitation) {
#if 0 /*JP*/
	    floating_above("toilet");
#else
	    floating_above("トイレ");
#endif
	    return;
	}
	(void) get_wet(obj, FALSE);
	/* KMH -- acid and water don't mix */
	if (obj->otyp == POT_ACID) {
	    useup(obj);
	    return;
	}
	if(is_poisonable(obj)) {
#if 0 /*JP*/
	    if (flags.verbose)  You("cover it in filth.");
#else
	    if (flags.verbose)  You("それに汚物をつけた．");
#endif
	    obj->opoisoned = TRUE;
	}
	if (obj->oclass == FOOD_CLASS) {
#if 0 /*JP*/
	    if (flags.verbose)  pline("My! It certainly looks tastier now...");
#else
	    if (flags.verbose)  pline("おや！おいしそうに見える．．．");
#endif
	    obj->orotten = TRUE;
	}
#if 0 /*JP*/
	if (flags.verbose)  pline("Yuck!");
#else
	if (flags.verbose)  pline("ゲーッ！");
#endif
}


void
breaksink(x,y)
int x, y;
{
    if(cansee(x,y) || (x == u.ux && y == u.uy))
/*JP
	pline_The("pipes break!  Water spurts out!");
*/
	pline("配管が壊れ水が噴出した！");
    level.flags.nsinks--;
    levl[x][y].doormask = 0;
    levl[x][y].typ = FOUNTAIN;
    level.flags.nfountains++;
    newsym(x,y);
}

void
breaktoilet(x,y)
int x, y;
{
    register int num = rn1(5,2);
    struct monst *mtmp;
#if 0 /*JP*/
    pline("The toilet suddenly shatters!");
#else
    pline("トイレは突然こなごなになった！");
#endif
    level.flags.nsinks--;
    levl[x][y].typ = FOUNTAIN;
    level.flags.nfountains++;
    newsym(x,y);
    if (!rn2(3)) {
      if (!(mvitals[PM_BABY_CROCODILE].mvflags & G_GONE)) {
	if (!Blind) {
#if 0 /*JP*/
	    if (!Hallucination) pline("Oh no! Crocodiles come out from the pipes!");
	    else pline("Oh no! Tons of poopies!");
#else
	    if (!Hallucination) pline("うおっ！配管からクロコダイルが出てきた！");
	    else pline("うおっ！ポパイが沢山出てきた！");
#endif
	} else
#if 0 /*JP*/
	    You("hear something scuttling around you!");
#else
	    pline("何かがあなたの周りを駆け回っている！");
#endif
	while(num-- > 0)
	    if((mtmp = makemon(&mons[PM_BABY_CROCODILE],u.ux,u.uy, NO_MM_FLAGS)) &&
	       t_at(mtmp->mx, mtmp->my))
		(void) mintrap(mtmp);
      } else
#if 0 /*JP*/
	pline("The sewers seem strangely quiet.");
#else
	pline("下水道は妙に静かだ．");
#endif
    }
}

void
drinksink()
{
	struct obj *otmp;
	struct monst *mtmp;

	if (Levitation) {
/*JP
		floating_above("sink");
*/
		floating_above("流し台");
		return;
	}
	switch(rn2(20)) {
/*JP
		case 0: You("take a sip of very cold water.");
*/
		case 0: You("とても冷たい水を一口飲んだ．");
			break;
/*JP
		case 1: You("take a sip of very warm water.");
*/
		case 1: You("とてもぬるい水を一口飲んだ．");
			break;
/*JP
		case 2: You("take a sip of scalding hot water.");
*/
		case 2: You("とても熱い水を一口飲んだ．");
			if (Fire_resistance)
/*JP
				pline("It seems quite tasty.");
*/
				pline("とてもおいしい水だ．");
/*JP
			else losehp(rnd(6), "sipping boiling water", KILLED_BY);
*/
			else losehp(rnd(6), "沸騰した水を飲んで", KILLED_BY);
			break;
		case 3: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
/*JP
				pline_The("sink seems quite dirty.");
*/
				pline("流し台はとても汚ならしい．");
			else {
				mtmp = makemon(&mons[PM_SEWER_RAT],
						u.ux, u.uy, NO_MM_FLAGS);
#if 0 /*JP*/
				if (mtmp) pline("Eek!  There's %s in the sink!",
					(Blind || !canspotmon(mtmp)) ?
					"something squirmy" :
					a_monnam(mtmp));
#else
				if (mtmp) pline("げ！流し台に%sがいる！",
					(Blind || !canspotmon(mtmp)) ?
					"何かもぞもぞ動くもの" :
					a_monnam(mtmp));
#endif
			}
			break;
		case 4: do {
				otmp = mkobj(POTION_CLASS,FALSE);
				if (otmp->otyp == POT_WATER) {
					obfree(otmp, (struct obj *)0);
					otmp = (struct obj *) 0;
				}
			} while(!otmp);
			otmp->cursed = otmp->blessed = 0;
#if 0 /*JP*/
			pline("Some %s liquid flows from the faucet.",
			      Blind ? "odd" :
			      hcolor(OBJ_DESCR(objects[otmp->otyp])));
#else
			pline("蛇口から%s液体が流れた．",
			      Blind ? "奇妙な" :
			      hcolor(jtrns_obj('!',OBJ_DESCR(objects[otmp->otyp]))));
#endif
			otmp->dknown = !(Blind || Hallucination);
			otmp->fromsink = 1; /* kludge for docall() */
			/* dopotion() deallocs dummy potions */
			(void) dopotion(otmp);
			break;
		case 5: if (!(levl[u.ux][u.uy].looted & S_LRING)) {
#if 0 /*JP*/
			    You("find a ring in the sink!");
#else
			    You("流し台に指輪をみつけた！");
#endif
			    (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
			    levl[u.ux][u.uy].looted |= S_LRING;
			    exercise(A_WIS, TRUE);
			    newsym(u.ux,u.uy);
#if 0 /*JP*/
			} else pline("Some dirty water backs up in the drain.");
#else
			} else pline("汚水が排水口から逆流してきた．");
#endif
			break;
		case 6: breaksink(u.ux,u.uy);
			break;
#if 0 /*JP*/
		case 7: pline_The("water moves as though of its own will!");
#else
		case 7: pline("水が意思を持っているかのように動いた！");
#endif
			if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
			    || !makemon(&mons[PM_WATER_ELEMENTAL],
					u.ux, u.uy, NO_MM_FLAGS))
#if 0 /*JP*/
				pline("But it quiets down.");
#else
				pline("しかし，静かになった．");
#endif
			break;
#if 0 /*JP*/
		case 8: pline("Yuk, this water tastes awful.");
#else
		case 8: pline("オェ，とてもひどい味がする．");
#endif
			more_experienced(1,0);
			newexplevel();
			break;
#if 0 /*JP*/
		case 9: pline("Gaggg... this tastes like sewage!  You vomit.");
#else
		case 9: pline("ゲェー．下水のような味がする！あなたは吐き戻した．");
#endif
			morehungry(rn1(30-ACURR(A_CON), 11));
			vomit();
			break;
		case 10:
			/* KMH, balance patch -- new intrinsic */
#if 0 /*JP*/
			pline("This water contains toxic wastes!");
#else
			pline("この水は有毒な廃液を含んでいる！");
#endif
			if (!Unchanging) {
			if (!Unchanging) {
#if 0 /*JP*/
				You("undergo a freakish metamorphosis!");
#else
				You("奇形な変化をしはじめた！");
#endif
				polyself(FALSE);
			}
			}
			break;
		/* more odd messages --JJB */
#if 0 /*JP*/
		case 11: You_hear("clanking from the pipes...");
#else
		case 11: You_hear("配管のカチンという音を聞いた．．．");
#endif
			break;
#if 0 /*JP*/
		case 12: You_hear("snatches of song from among the sewers...");
#else
		case 12: You_hear("下水の中からとぎれとぎれの歌を聞いた．．．");
#endif
			break;
		case 19: if (Hallucination) {
#if 0 /*JP*/
		   pline("From the murky drain, a hand reaches up... --oops--");
#else
		   pline("暗い排水口から手が伸びてきた．．--おっと--");
#endif
				break;
			}
#if 0 /*JP*/
		default: You("take a sip of %s water.",
			rn2(3) ? (rn2(2) ? "cold" : "warm") : "hot");
#else
		default: You("%s水を一口飲んだ．",
			rn2(3) ? (rn2(2) ? "冷い" : "ぬるい") : "熱い");
#endif
	}
}

void
drinktoilet()
{
	if (Levitation) {
#if 0 /*JP*/
		floating_above("toilet");
#else
		floating_above("トイレ");
#endif
		return;
	}
	if ((youmonst.data->mlet == S_DOG) && (rn2(5))){
#if 0 /*JP*/
		pline("The toilet water is quite refreshing!");
#else
		pline("トイレの水はあなたをさっぱりさせた！");
#endif
		u.uhunger += 10;
		return;
	}
	switch(rn2(9)) {
/*
		static NEARDATA struct obj *otmp;
 */
		case 0: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
#if 0 /*JP*/
				pline("The toilet seems quite dirty.");
#else
				pline("トイレはとても汚ならしい．");
#endif
			else {
				static NEARDATA struct monst *mtmp;

				mtmp = makemon(&mons[PM_SEWER_RAT], u.ux, u.uy,
					NO_MM_FLAGS);
#if 0 /*JP*/
				pline("Eek!  There's %s in the toilet!",
					Blind ? "something squirmy" :
					a_monnam(mtmp));
#else
				pline("げ！トイレに%sがいる！",
					Blind ? "何かもぞもぞ動くもの":
					a_monnam(mtmp));
#endif
			}
			break;
		case 1: breaktoilet(u.ux,u.uy);
			break;
#if 0 /*JP*/
		case 2: pline("Something begins to crawl out of the toilet!");
#else
		case 2: pline("何かがトイレから這い出てきた！");
#endif
			if (mvitals[PM_BROWN_PUDDING].mvflags & G_GONE
			    || !makemon(&mons[PM_BROWN_PUDDING], u.ux, u.uy,
					NO_MM_FLAGS))
#if 0 /*JP*/
				pline("But it slithers back out of sight.");
#else
				pline("しかしそれは滑べり落ちて消え去った．");
#endif
			break;
		case 3:
		case 4: if (mvitals[PM_BABY_CROCODILE].mvflags & G_GONE)
#if 0 /*JP*/
				pline("The toilet smells fishy.");
#else
				pline("トイレはなま臭い．");
#endif
			else {
				static NEARDATA struct monst *mtmp;

				mtmp = makemon(&mons[PM_BABY_CROCODILE], u.ux,
					 u.uy, NO_MM_FLAGS);
#if 0 /*JP*/
				pline("Egad!  There's %s in the toilet!",
					Blind ? "something squirmy" :
					a_monnam(mtmp));
#else
				pline("うおっと！トイレに%sがいる！",
					Blind ? "おぞましいもの":
					a_monnam(mtmp));
#endif
			}
			break;
#if 0 /*JP*/
		default: pline("Gaggg... this tastes like sewage!  You vomit.");
#else
		default: pline("ゲェー．下水のような味がする！あなたは吐き戻した．");
#endif
			morehungry(rn1(30-ACURR(A_CON), 11));
			vomit();
	}
}
#endif /* SINKS */


void
whetstone_fountain_effects(obj)
register struct obj *obj;
{
	if (Levitation) {
#if 0 /*JP*/
		floating_above("fountain");
#else
		floating_above("泉");
#endif
		return;
	}

	switch (rnd(30)) {
		case 10: /* Curse the item */
			curse(obj);
			break;
		case 11:
		case 12:
		case 13:
		case 14: /* Uncurse the item */
			if(obj->cursed) {
			    if (!Blind)
#if 0 /*JP*/
				pline_The("water glows for a moment.");
#else
				pline_The("水が一瞬輝いた．");
#endif
			    uncurse(obj);
			} else {
/*JP
			    pline("A feeling of loss comes over you.");
*/
			    pline("奇妙な脱力感があなたをおそった．");
			}
			break;
		case 15:
		case 16: /* Water Demon */
			dowaterdemon();
			break;
		case 17:
		case 18: /* Water Nymph */
			dowaternymph();
			break;
		case 19:
		case 20: /* an Endless Stream of Snakes */
			dowatersnakes();
			break;
		case 21:
		case 22:
		case 23: /* Find a gem */
			if (!FOUNTAIN_IS_LOOTED(u.ux,u.uy)) {
				dofindgem();
				break;
			}
		case 24:
		case 25: /* Water gushes forth */
			dogushforth(FALSE);
			break;
		case 26: /* Strange feeling */
/*JP
			pline("A strange tingling runs up your %s.",
*/
			pline("奇妙なしびれがあなたの%sに走った．",
							body_part(ARM));
			break;
		case 27: /* Strange feeling */
/*JP
			You_feel("a sudden chill.");
*/
			You("突然寒けを感じた．");
			break;
		case 28: /* Strange feeling */
/*JP
			pline("An urge to take a bath overwhelms you.");
*/
			pline("水浴びをしたいという欲望にかられた．");
#ifndef GOLDOBJ
			if (u.ugold > 10) {
			    u.ugold -= somegold() / 10;
/*JP
			    You("lost some of your gold in the fountain!");
*/
			    You("金貨を数枚，泉に落としてしまった！");
			    CLEAR_FOUNTAIN_LOOTED(u.ux,u.uy);
			    exercise(A_WIS, FALSE);
			}
#else
			{
			    long money = money_cnt(invent);
			    struct obj *otmp;
                            if (money > 10) {
				/* Amount to loose.  Might get rounded up as fountains don't pay change... */
			        money = somegold(money) / 10; 
			        for (otmp = invent; otmp && money > 0; otmp = otmp->nobj) if (otmp->oclass == COIN_CLASS) {
				    int denomination = objects[otmp->otyp].oc_cost;
				    long coin_loss = (money + denomination - 1) / denomination;
                                    coin_loss = min(coin_loss, otmp->quan);
				    otmp->quan -= coin_loss;
				    money -= coin_loss * denomination;				  
				    if (!otmp->quan) delobj(otmp);
				}
/*JP
			        You("lost some of your money in the fountain!");
*/
			        You("お金を数枚，泉に落としてしまった！");
			        levl[u.ux][u.uy].looted &= ~F_LOOTED;
			        exercise(A_WIS, FALSE);
                            }
			}
#endif
			break;
		case 29: /* You see coins */

		/* We make fountains have more coins the closer you are to the
		 * surface.  After all, there will have been more people going
		 * by.	Just like a shopping mall!  Chris Woodbury  */

		    if (levl[u.ux][u.uy].looted) break;
		    levl[u.ux][u.uy].looted |= F_LOOTED;
		    (void) mkgold((long)
			(rnd((dunlevs_in_dungeon(&u.uz)-dunlev(&u.uz)+1)*2)+5),
			u.ux, u.uy);
		    if (!Blind)
/*JP
		pline("Far below you, you see coins glistening in the water.");
*/
		You("遥か下の水中に金貨の輝きをみつけた．");
		    exercise(A_WIS, TRUE);
		    newsym(u.ux,u.uy);
		    break;
	}
	update_inventory();
	dryup(u.ux, u.uy, TRUE);
}

#ifdef SINKS

void
whetstone_toilet_effects(obj)
register struct obj *obj;
{
	if (Levitation) {
/*JP
	    floating_above("toilet");
*/
	    floating_above("トイレ");
	    return;
	}
	if(is_poisonable(obj)) {
/*JP
	    if (flags.verbose)  You("cover it in filth.");
*/
	    if (flags.verbose)  You("それに汚物をつけた．");
	    obj->opoisoned = TRUE;
	}
#if 0 /*JP*/
	if (flags.verbose)  pline("Yuck!");
#else
	if (flags.verbose)  pline("ゲーッ！");
#endif
}

void
whetstone_sink_effects(obj)
register struct obj *obj;
{
	struct monst *mtmp;

	if (Levitation) {
		floating_above("sink");
		return;
	}
	switch(rn2(20)) {
		case 0: if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
				pline_The("sink seems quite dirty.");
			else {
				mtmp = makemon(&mons[PM_SEWER_RAT],
						u.ux, u.uy, NO_MM_FLAGS);
#if 0 /*JP*/
				pline("Eek!  There's %s in the sink!",
					Blind ? "something squirmy" :
					a_monnam(mtmp));
#else
				pline("げ！流し台に%sがいる！",
					Blind ? "おぞましいもの":
					a_monnam(mtmp));
#endif
			}
			break;
		case 1: if (!(levl[u.ux][u.uy].looted & S_LRING)) {
#if 0 /*JP*/
			    You("find a ring in the sink!");
#else
			    You("流し台に指輪をみつけた！");
#endif
			    (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
			    levl[u.ux][u.uy].looted |= S_LRING;
			    exercise(A_WIS, TRUE);
			    newsym(u.ux,u.uy);
#if 0 /*JP*/
			} else pline("Some dirty water backs up in the drain.");
#else
			} else pline("汚ない水が排水口から逆流してきた．");
#endif
			break;
		case 2: breaksink(u.ux,u.uy);
			break;
#if 0 /*JP*/
		case 3: pline_The("water moves as though of its own will!");
#else
		case 3: pline("水が意思を持っているかのように動いた！");
#endif
			if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
			    || !makemon(&mons[PM_WATER_ELEMENTAL],
					u.ux, u.uy, NO_MM_FLAGS))
#if 0 /*JP*/
				pline("But it quiets down.");
#else
				pline("しかし，静かになった．");
#endif
			break;
		case 4:
#if 0 /*JP*/
			pline("This water contains toxic wastes!");
#else
			pline("この水は有毒な廃液を含んでいる！");
#endif
			obj = poly_obj(obj, STRANGE_OBJECT);
			u.uconduct.polypiles++;
			break;
#if 0 /*JP*/
		case 5: You_hear("clanking from the pipes...");
#else
		case 5: You_hear("配管のカチンという音を聞いた．．．");
#endif
			break;
#if 0 /*JP*/
		case 6: You_hear("snatches of song from among the sewers...");
#else
		case 6: You_hear("下水の中からとぎれとぎれの歌を聞いた．．．");
#endif
			break;
		case 19: if (Hallucination) {
#if 0 /*JP*/
		   pline("From the murky drain, a hand reaches up... --oops--");
#else
		   pline("暗い排水口から手が伸びてきた．．--おっと--");
#endif
				break;
			}
		default:
			break;
	}
}

#endif /* SINKS */

/*fountain.c*/
