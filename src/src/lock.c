/*	SCCS Id: @(#)lock.c	3.4	2000/02/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, 2004-2007
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"

STATIC_PTR int NDECL(picklock);
STATIC_PTR int NDECL(forcelock);
STATIC_PTR int NDECL(forcedoor);

/* at most one of `door' and `box' should be non-null at any given time */
STATIC_VAR NEARDATA struct xlock_s {
	struct rm  *door;
	struct obj *box;
	int picktyp, chance, usedtime;
	/* ALI - Artifact doors */
	int key;			/* Key being used (doors only) */
} xlock;

#ifdef OVLB

STATIC_DCL const char *NDECL(lock_action);
STATIC_DCL boolean FDECL(obstructed,(int,int));
STATIC_DCL void FDECL(chest_shatter_msg, (struct obj *));

boolean
picking_lock(x, y)
	int *x, *y;
{
	if (occupation == picklock || occupation == forcedoor) {
	    *x = u.ux + u.dx;
	    *y = u.uy + u.dy;
	    return TRUE;
	} else {
	    *x = *y = 0;
	    return FALSE;
	}
}

boolean
picking_at(x, y)
int x, y;
{
	return (boolean)(occupation == picklock && xlock.door == &levl[x][y]);
}

/* produce an occupation string appropriate for the current activity */
STATIC_OVL const char *
lock_action()
{
	/* "unlocking"+2 == "locking" */
	static const char *actions[] = {
/* 
** 英語は un をつけるだけで逆の意味になるが，日本語はそうはいかない．
** 誰だ？こんな数バイトけちるコード書いたやつは？
*/
#if 0 /*JP*/
		/* [0] */	"unlocking the door",
		/* [1] */	"unlocking the chest",
		/* [2] */	"unlocking the box",
		/* [3] */	"picking the lock"
#else
		/* [0] */	"扉の錠をはずす", 
		/* [1] */	"宝箱の錠をはずす",
		/* [2] */	"箱の錠をはずす",  
		/* [3] */	"錠をはずす"    
#endif /*JP*/
	};

	/* if the target is currently unlocked, we're trying to lock it now */
	if (xlock.door && !(xlock.door->doormask & D_LOCKED))
#if 0 /*JP*/
		return actions[0]+2;	/* "locking the door" */
#else
		return "扉に鍵をかける";
#endif
	else if (xlock.box && !xlock.box->olocked)
/*JP
		return xlock.box->otyp == CHEST ? actions[1]+2 : actions[2]+2;
*/
		return xlock.box->otyp == CHEST ? "宝箱に鍵をかける" : "箱に鍵をかける";
	/* otherwise we're trying to unlock it */
	else if (xlock.picktyp == LOCK_PICK)
		return actions[3];	/* "picking the lock" */
#ifdef TOURIST
	else if (xlock.picktyp == CREDIT_CARD)
		return actions[3];	/* same as lock_pick */
#endif
	else if (xlock.door)
		return actions[0];	/* "unlocking the door" */
	else
		return xlock.box->otyp == CHEST ? actions[1] : actions[2];
}

STATIC_PTR
int
picklock(VOID_ARGS)	/* try to open/close a lock */
{

	if (xlock.box) {
	    if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy)) {
		return((xlock.usedtime = 0));		/* you or it moved */
	    }
	} else {		/* door */
	    if(xlock.door != &(levl[u.ux+u.dx][u.uy+u.dy])) {
		return((xlock.usedtime = 0));		/* you moved */
	    }
	    switch (xlock.door->doormask) {
		case D_NODOOR:
/*JP
		    pline("This doorway has no door.");
*/
		    pline("この戸口には扉がない．");
		    return((xlock.usedtime = 0));
		case D_ISOPEN:
/*JP
		    You("cannot lock an open door.");
*/
		    pline("開いた扉に鍵はかけられない．");
		    return((xlock.usedtime = 0));
		case D_BROKEN:
/*JP
		    pline("This door is broken.");
*/
		    pline("この扉は壊れてしまっている．");
		    return((xlock.usedtime = 0));
	    }
	}

	if (xlock.usedtime++ >= 50 || nohands(youmonst.data)) {
/*JP
	    You("give up your attempt at %s.", lock_action());
*/
	    pline("%sのをあきらめた．", lock_action());
	    exercise(A_DEX, TRUE);	/* even if you don't succeed */
	    return((xlock.usedtime = 0));
	}

	if(rn2(100) >= xlock.chance) return(1);		/* still busy */

/*JP
	You("succeed in %s.", lock_action());
*/
	You("%sのに成功した．", lock_action());
	if (xlock.door) {
	    if(xlock.door->doormask & D_TRAPPED) {
/*JP
		    b_trapped("door", FINGER);
*/
		    b_trapped("扉", FINGER);
		    xlock.door->doormask = D_NODOOR;
		    unblock_point(u.ux+u.dx, u.uy+u.dy);
		    if (*in_rooms(u.ux+u.dx, u.uy+u.dy, SHOPBASE))
			add_damage(u.ux+u.dx, u.uy+u.dy, 0L);
		    newsym(u.ux+u.dx, u.uy+u.dy);
	    } else if(xlock.door->doormask == D_LOCKED)
		xlock.door->doormask = D_CLOSED;
	    else xlock.door->doormask = D_LOCKED;
	} else {
	    xlock.box->olocked = !xlock.box->olocked;
	    if(xlock.box->otrapped)	
		(void) chest_trap(xlock.box, FINGER, FALSE);
	}
	exercise(A_DEX, TRUE);
	return((xlock.usedtime = 0));
}

STATIC_PTR
int
forcelock(VOID_ARGS)	/* try to force a locked chest */
{

	register struct obj *otmp;

	if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy))
		return((xlock.usedtime = 0));		/* you or it moved */

	if (xlock.usedtime++ >= 50 || !uwep || nohands(youmonst.data)) {
/*JP
	    You("give up your attempt to force the lock.");
*/
	    pline("錠をこじ開けるのをあきらめた．");
	    if(xlock.usedtime >= 50)		/* you made the effort */
	      exercise((xlock.picktyp) ? A_DEX : A_STR, TRUE);
	    return((xlock.usedtime = 0));
	}

	if(xlock.picktyp == 1) {     /* blade */

	    if(rn2(1000-(int)uwep->spe) > (992-greatest_erosion(uwep)*10) &&
	       !uwep->cursed && !obj_resists(uwep, 0, 99)) {
		/* for a +0 weapon, probability that it survives an unsuccessful
		 * attempt to force the lock is (.992)^50 = .67
		 */
#if 0 /*JP*/
		pline("%sour %s broke!",
		      (uwep->quan > 1L) ? "One of y" : "Y", xname(uwep));
#else
		pline("%sは壊れてしまった！",xname(uwep));
#endif
		useup(uwep);
/*JP
		You("give up your attempt to force the lock.");
*/
		pline("錠をこじ開けるのをあきらめた．");
		exercise(A_DEX, TRUE);
		return((xlock.usedtime = 0));
	    }
	} else if(xlock.picktyp == 0)                 /* blunt */
	    wake_nearby();	/* due to hammering on the container */

	if(rn2(100) >= xlock.chance) return(1);		/* still busy */

/*JP
	You("succeed in forcing the lock.");
*/
	pline("錠をこじ開けた．");
	xlock.box->olocked = 0;
	xlock.box->obroken = 1;
	if((xlock.picktyp == 0 && !rn2(3)) || (xlock.picktyp == 2 && !rn2(5))) {
	    struct monst *shkp;
	    boolean costly;
	    long loss = 0L;

	    costly = (*u.ushops && costly_spot(u.ux, u.uy));
	    shkp = costly ? shop_keeper(*u.ushops) : 0;

#if 0 /*JP*/
	    pline("In fact, you've totally destroyed %s.",
		  the(xname(xlock.box)));
#else
	    pline("実際のところ，%sを完全に壊してしまった．",
		  xname(xlock.box));
#endif

	    /* Put the contents on ground at the hero's feet. */
	    while ((otmp = xlock.box->cobj) != 0) {
		obj_extract_self(otmp);
		/* [ALI] Allowing containers to be destroyed is complicated
		 * (because they might contain indestructible objects).
		 * Since this is very unlikely to occur in practice simply
		 * avoid the possibility.
		 */
		if (!evades_destruction(otmp) && !Has_contents(otmp) &&
		  (!rn2(3) || otmp->oclass == POTION_CLASS)) {
		    chest_shatter_msg(otmp);
		    if (costly)
		        loss += stolen_value(otmp, u.ux, u.uy,
					     (boolean)shkp->mpeaceful, TRUE,
					     TRUE);
		    if (otmp->quan == 1L) {
			obfree(otmp, (struct obj *) 0);
			continue;
		    }
		    useup(otmp);
		}
		if (xlock.box->otyp == ICE_BOX && otmp->otyp == CORPSE) {
		    otmp->age = monstermoves - otmp->age; /* actual age */
		    start_corpse_timeout(otmp);
		}
		place_object(otmp, u.ux, u.uy);
		stackobj(otmp);
	    }

	    if (costly)
		loss += stolen_value(xlock.box, u.ux, u.uy,
				     (boolean)shkp->mpeaceful, TRUE, TRUE);
/*JP
	    if(loss) You("owe %ld %s for objects destroyed.", loss, currency(loss));
*/
	    if(loss) You("器物破損で%ld%sの借りをつくった．", loss, currency(loss));
	    delobj(xlock.box);
	}
	exercise((xlock.picktyp) ? A_DEX : A_STR, TRUE);
	return((xlock.usedtime = 0));
}

STATIC_PTR
int
forcedoor(VOID_ARGS)      /* try to break/pry open a door */
{

	if(xlock.door != &(levl[u.ux+u.dx][u.uy+u.dy])) {
	    return((xlock.usedtime = 0));           /* you moved */
	} 
	switch (xlock.door->doormask) {
	    case D_NODOOR:
#if 0 /*JP*/
		pline("This doorway has no door.");
#else
		pline("この戸口には扉がない．");
#endif
		return((xlock.usedtime = 0));
	    case D_ISOPEN:
#if 0 /*JP*/
		You("cannot lock an open door.");
#else
		pline("開いた扉に鍵はかけられない．");
#endif
		return((xlock.usedtime = 0));
	    case D_BROKEN:
#if 0 /*JP*/
		pline("This door is broken.");
#else
		pline("この扉は壊れてしまっている．");
#endif
		return((xlock.usedtime = 0));
	}
	
	if (xlock.usedtime++ >= 50 || nohands(youmonst.data)) {
#if 0 /*JP*/
	    You("give up your attempt at %s the door.",
	    	(xlock.picktyp == 2 ? "melting" : xlock.picktyp == 1 ? 
	    		"prying open" : "breaking down"));
#else
	    You("扉を%s開けようとするのをあきらめた．",
	    	(xlock.picktyp == 2 ? "溶かし" : xlock.picktyp == 1 ? 
	    		"こじ" : "壊して"));
#endif
	    exercise(A_STR, TRUE);      /* even if you don't succeed */
	    return((xlock.usedtime = 0));
	}

	if(rn2(100) > xlock.chance) return(1);          /* still busy */

#if 0 /*JP*/
	You("succeed in %s the door.",
	    	(xlock.picktyp == 2 ? "melting" : xlock.picktyp == 1 ? 
	    		"prying open" : "breaking down"));
#else
	You("扉を%s開けた．",
	    	(xlock.picktyp == 2 ? "溶かし" : xlock.picktyp == 1 ? 
	    		"こじ" : "壊して"));
#endif

	if(xlock.door->doormask & D_TRAPPED) {
#if 0 /*JP*/
	    b_trapped("door", 0);
#else
	    b_trapped("扉", 0);
#endif
	    xlock.door->doormask = D_NODOOR;
	} else if (xlock.picktyp == 1)
	    xlock.door->doormask = D_BROKEN;
	else xlock.door->doormask = D_NODOOR;
	unblock_point(u.ux+u.dx, u.uy+u.dy);
	if (*in_rooms(u.ux+u.dx, u.uy+u.dy, SHOPBASE)) {
	    add_damage(u.ux+u.dx, u.uy+u.dy, 400L);
/*JP
	    pay_for_damage("break", FALSE);
*/
	    pay_for_damage("壊す", FALSE);

	    if (in_town(u.ux+u.dx, u.uy+u.dy)) {
		struct monst *mtmp;
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if (DEADMONSTER(mtmp)) continue;
		    if((mtmp->data == &mons[PM_WATCHMAN] ||
			mtmp->data == &mons[PM_WATCH_CAPTAIN]) &&
			couldsee(mtmp->mx, mtmp->my) &&
			mtmp->mpeaceful) {
			if (canspotmon(mtmp))
/*JP
			    pline("%s yells:", Amonnam(mtmp));
*/
			    pline("%sは叫んだ：", Amonnam(mtmp));
			else
/*JP
			    You_hear("someone yell:");
*/
			    You_hear("誰かが叫んでいるのを聞いた：");
/*JP
			verbalize("Halt, thief!  You're under arrest!");
*/
			verbalize("止まれ泥棒！おまえを逮捕する！");
			(void) angry_guards(FALSE);
			break;
		    }
		}
	    }
	}
	if (Blind)
	    feel_location(u.ux+u.dx, u.uy+u.dy);    /* we know we broke it */
	else
	    newsym(u.ux+u.dx, u.uy+u.dy);
	
	exercise(A_STR, TRUE);
	return((xlock.usedtime = 0));
}

#endif /* OVLB */
#ifdef OVL0

void
reset_pick()
{
	xlock.usedtime = xlock.chance = xlock.picktyp = 0;
	xlock.door = 0;
	xlock.box = 0;
}

#endif /* OVL0 */
#ifdef OVLB

int
pick_lock(pickp) /* pick a lock with a given object */
	struct	obj	**pickp;
{
	int picktyp, c, ch;
	coord cc;
	int key;
	struct rm	*door;
	struct obj	*otmp;
	struct	obj	*pick = *pickp;
	char qbuf[QBUFSZ];

	picktyp = pick->otyp;

	/* check whether we're resuming an interrupted previous attempt */
	if (xlock.usedtime && picktyp == xlock.picktyp) {
/*JP
	    static char no_longer[] = "Unfortunately, you can no longer %s %s.";
*/
	    static char no_longer[] = "ざんねんながら，あなたは%s%s．";

	    if (nohands(youmonst.data)) {
/*JP
		const char *what = (picktyp == LOCK_PICK) ? "pick" : "key";
*/
		const char *what = (picktyp == LOCK_PICK) ? "工具" : "鍵";
#ifdef TOURIST
/*JP
		if (picktyp == CREDIT_CARD) what = "card";
*/
		if (picktyp == CREDIT_CARD) what = "カード";
#endif
/*JP
		pline(no_longer, "hold the", what);
*/
		pline(no_longer, what, "をつかめない");
		reset_pick();
		return 0;
	    } else if (xlock.box && !can_reach_floor()) {
/*JP
		pline(no_longer, "reach the", "lock");
*/
		pline(no_longer, "錠", "に届かない");
		reset_pick();
		return 0;
	    } else if (!xlock.door || xlock.key == pick->oartifact) {
		const char *action = lock_action();
/*JP
		You("resume your attempt at %s.", action);
*/
		pline("%sのを再開した．", action);
		set_occupation(picklock, action, 0);
		return(1);
	    }
	}

	if(nohands(youmonst.data)) {
/*JP
		You_cant("hold %s -- you have no hands!", doname(pick));
*/
		You("%sをつかむことができない！手がないんだもの！", xname(pick));
		return(0);
	}

	if((picktyp != LOCK_PICK &&
#ifdef TOURIST
	    picktyp != CREDIT_CARD &&
#endif
	    picktyp != SKELETON_KEY)) {
		impossible("picking lock with object %d?", picktyp);
		return(0);
	}
	ch = 0;		/* lint suppression */

/*JP
	if(!get_adjacent_loc((char *)0, "Invalid location!", u.ux, u.uy, &cc)) return 0;
*/
	if(!get_adjacent_loc((char *)0, "位置がおかしい！", u.ux, u.uy, &cc)) return 0;
	if (cc.x == u.ux && cc.y == u.uy) {	/* pick lock on a container */
	    const char *verb;
	    boolean it;
	    int count;

	    if (u.dz < 0) {
#if 0 /*JP*/
		There("isn't any sort of lock up %s.",
		      Levitation ? "here" : "there");
#else
		pline("%sには鍵をかけるような物はない．",
		      Levitation ? "ここ" : "上の方");
#endif
		return 0;
	    } else if (is_lava(u.ux, u.uy)) {
/*JP
		pline("Doing that would probably melt your %s.",
*/
		pline("そんなことをしたら%sが溶けてしまう．",
		      xname(pick));
		return 0;
	    } else if (is_pool(u.ux, u.uy) && !Underwater) {
/*JP
		pline_The("water has no lock.");
*/
		pline("水に鍵穴はない．");
		return 0;
	    }

	    count = 0;
	    c = 'n';			/* in case there are no boxes here */
	    for(otmp = level.objects[cc.x][cc.y]; otmp; otmp = otmp->nexthere)
		if (Is_box(otmp)) {
		    ++count;
		    if (!can_reach_floor()) {
/*JP
			You_cant("reach %s from up here.", the(xname(otmp)));
*/
			Your("今の高い位置からでは%sに届かない．", the(xname(otmp)));
			return 0;
		    }
		    it = 0;
#if 0 /*JP*/
		    if (otmp->obroken) verb = "fix";
		    else if (!otmp->olocked) verb = "lock", it = 1;
		    else if (picktyp != LOCK_PICK) verb = "unlock", it = 1;
		    else verb = "pick";
#else
		    if (otmp->obroken) verb = "修復する";
		    else if (!otmp->olocked) verb = "鍵をかける", it = 1;
		    else if (picktyp != LOCK_PICK) verb = "鍵をはずす", it = 1;
		    else verb = "こじあける";
#endif /*JP*/
#if 0 /*JP*/
		    Sprintf(qbuf, "There is %s here, %s %s?",
		    	    safe_qbuf("", sizeof("There is  here, unlock its lock?"),
			    	doname(otmp), an(simple_typename(otmp->otyp)), "a box"),
			    verb, it ? "it" : "its lock");
#else
		    Sprintf(qbuf, "ここには%sがある．%s？",
		    	    safe_qbuf("", sizeof("ここにはがある．錠をはずしますか？"),
			    	doname(otmp), an(simple_typename(otmp->otyp)), "箱"),
			    verb);
#endif

		    c = ynq(qbuf);
		    if(c == 'q') return(0);
		    if(c == 'n') continue;

		    if (otmp->obroken) {
/*JP
			You_cant("fix its broken lock with %s.", doname(pick));
*/
			You("壊れた錠を%sで修復できない．", doname(pick));
			return 0;
		    }
#ifdef TOURIST
		    else if (picktyp == CREDIT_CARD && !otmp->olocked) {
			/* credit cards are only good for unlocking */
/*JP
			You_cant("do that with %s.", doname(pick));
*/
			pline("%sじゃそんなことはできない．", doname(pick));
			return 0;
		    }
#endif
		    switch(picktyp) {
#ifdef TOURIST
			case CREDIT_CARD:
			    if(!rn2(20) && !pick->blessed && !pick->oartifact) {
#if 0 /*JP*/
				Your("credit card breaks in half!");
#else
				Your("クレジットカードは真っ二つに折れた！");
#endif
				useup(pick);
				*pickp = (struct obj *)0;
				return(1);
			    }
			    ch = ACURR(A_DEX) + 20*Role_if(PM_ROGUE);
			    break;
#endif
			case LOCK_PICK:
			    if(!rn2(Role_if(PM_ROGUE) ? 40 : 30) &&
			    		!pick->blessed && !pick->oartifact) {
#if 0 /*JP*/
				You("break your pick!");
#else
				You("工具を壊してしまった！");
#endif
				useup(pick);
				*pickp = (struct obj *)0;
				return(1);
			    }
			    ch = 4*ACURR(A_DEX) + 25*Role_if(PM_ROGUE);
			    break;
			case SKELETON_KEY:
			    if(!rn2(15) && !pick->blessed && !pick->oartifact) {
#if 0 /*JP*/
				Your("key didn't quite fit the lock and snapped!");
#else
				Your("鍵は全く鍵穴に合わずポキンと折れてしまった！");
#endif
				useup(pick);
				*pickp = (struct obj *)0;
				return(1);
			    }
			    ch = 75 + ACURR(A_DEX);
			    break;
			default:	ch = 0;
		    }
		    if(otmp->cursed) ch /= 2;

		    xlock.picktyp = picktyp;
		    xlock.box = otmp;
		    xlock.door = 0;
		    break;
		}
	    if (c != 'y') {
		if (!count)
/*JP
		    There("doesn't seem to be any sort of lock here.");
*/
		    pline("ここには鍵穴のついている物はないようだ．");
		return(0);		/* decided against all boxes */
	    }
	} else {			/* pick the lock in a door */
	    struct monst *mtmp;

	    if (u.utrap && u.utraptype == TT_PIT) {
/*JP
		You_cant("reach over the edge of the pit.");
*/
		pline("落し穴の中からは届かない．");
		return(0);
	    }

	    door = &levl[cc.x][cc.y];
	    if ((mtmp = m_at(cc.x, cc.y)) && canseemon(mtmp)
			&& mtmp->m_ap_type != M_AP_FURNITURE
			&& mtmp->m_ap_type != M_AP_OBJECT) {
#ifdef TOURIST
		if (picktyp == CREDIT_CARD &&
		    (mtmp->isshk || mtmp->data == &mons[PM_ORACLE]))
/*JP
		    verbalize("No checks, no credit, no problem.");
*/
		    verbalize("いつもニコニコ現金払い．");
		else
#endif
/*JP
		    pline("I don't think %s would appreciate that.", mon_nam(mtmp));
*/
		    pline("%sがその価値を認めるとは思えない．", mon_nam(mtmp));
		return(0);
	    }
	    if(!IS_DOOR(door->typ)) {
		if (is_drawbridge_wall(cc.x,cc.y) >= 0)
#if 0 /*JP*/
		    You("%s no lock on the drawbridge.",
				Blind ? "feel" : "see");
#else
		    pline("跳ね橋には鍵穴がない%s．",
				Blind ? "ようだ" : "ように見える");
#endif
		else
#if 0 /*JP*/
		    You("%s no door there.",
				Blind ? "feel" : "see");
#else
		    pline("ここには扉がない%s．",
				Blind ? "ようだ" : "ように見える");
#endif
		return(0);
	    }
	    switch (door->doormask) {
		case D_NODOOR:
/*JP
		    pline("This doorway has no door.");
*/
	            pline("この戸口には扉がない．");
		    return(0);
		case D_ISOPEN:
/*JP
		    You("cannot lock an open door.");
*/
		    pline("開いた扉に鍵はかけられない．");
		    return(0);
		case D_BROKEN:
/*JP
		    pline("This door is broken.");
*/
		    pline("この扉は壊れてしまっている．");
		    return(0);
		default:
#ifdef TOURIST
		    /* credit cards are only good for unlocking */
		    if(picktyp == CREDIT_CARD && !(door->doormask & D_LOCKED)) {
/*JP
			You_cant("lock a door with a credit card.");
*/
		        You("クレジットカードで鍵をかけることはできない．");
			return(0);
		    }
#endif
		    /* ALI - Artifact doors */
		    key = artifact_door(cc.x, cc.y);

#if 0 /*JP*/
		    Sprintf(qbuf,"%sock it?",
			(door->doormask & D_LOCKED) ? "Unl" : "L" );
#else
		    Sprintf(qbuf,"%sますか？",
			(door->doormask & D_LOCKED) ? "はずし" : "かけ" );
#endif

		    c = yn(qbuf);
		    if(c == 'n') return(0);

		    switch(picktyp) {
#ifdef TOURIST
			case CREDIT_CARD:
			    if(!rn2(Role_if(PM_TOURIST) ? 30 : 20) &&
				    !pick->blessed && !pick->oartifact) {
#if 0 /*JP*/
				You("break your card off in the door!");
#else
				pline("扉の隙間に差し込んだカードは折れてしまった！");
#endif
				useup(pick);
				*pickp = (struct obj *)0;
				return(0);
			    }
			    ch = 2*ACURR(A_DEX) + 20*Role_if(PM_ROGUE);
			    break;
#endif
			case LOCK_PICK:
			    if(!rn2(Role_if(PM_ROGUE) ? 40 : 30) &&
				    !pick->blessed && !pick->oartifact) {
#if 0 /*JP*/
				You("break your pick!");
#else
				You("工具を壊してしまった！");
#endif
				useup(pick);
				*pickp = (struct obj *)0;
				return(0);
			    }
			    ch = 3*ACURR(A_DEX) + 30*Role_if(PM_ROGUE);
			    break;
			case SKELETON_KEY:
			    if(!rn2(15) && !pick->blessed && !pick->oartifact) {
#if 0 /*JP*/
				Your("key wasn't designed for this door and broke!");
#else
				Your("鍵はこの扉のための鍵ではなかったので壊れてしまった！");
#endif
				useup(pick);
				*pickp = (struct obj *)0;
				return(0);
			    }
			    ch = 70 + ACURR(A_DEX);
			    break;
			default:    ch = 0;
		    }
		    xlock.door = door;
		    xlock.box = 0;

		    /* ALI - Artifact doors */
		    xlock.key = pick->oartifact;
		    if (key && xlock.key != key) {
			if (picktyp == SKELETON_KEY) {
#if 0 /*JP*/
			    Your("key doesn't seem to fit.");
#else
			    Your("鍵は合わないようだ．");
#endif
			    return(0);
			}
			else ch = -1;		/* -1 == 0% chance */
		    }
	    }
	}
	flags.move = 0;
	xlock.chance = ch;
	xlock.picktyp = picktyp;
	xlock.usedtime = 0;
	set_occupation(picklock, lock_action(), 0);
	return(1);
}

int
doforce()		/* try to force a chest with your weapon */
{
	register struct obj *otmp;
	register int x, y, c, picktyp;
	struct rm       *door;
	char qbuf[QBUFSZ];

	if (!uwep) { /* Might want to make this so you use your shoulder */
#if 0 /*JP*/
	    You_cant("force anything without a weapon.");
#else
	    pline("武器なしで鍵をこじあけることはできない．");
#endif
	     return(0);
	}

	if (u.utrap && u.utraptype == TT_WEB) {
#if 0 /*JP*/
	    You("are entangled in a web!");
#else
	    You("くもの巣に絡まった！");
#endif
	    return(0);
#ifdef LIGHTSABERS
	} else if (uwep && is_lightsaber(uwep)) {
	    if (!uwep->lamplit) {
#if 0 /*JP*/
		Your("lightsaber is deactivated!");
#else
		Your("%sは起動していない！", xname(uwep));
#endif
		return(0);
	    }
#endif
	} else if(uwep->otyp == LOCK_PICK ||
#ifdef TOURIST
	    uwep->otyp == CREDIT_CARD ||
#endif
	    uwep->otyp == SKELETON_KEY) {
	    	return pick_lock(&uwep);
	/* not a lightsaber or lockpicking device*/
	} else if(!uwep ||     /* proper type test */
	   (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep) &&
	    uwep->oclass != ROCK_CLASS) ||
	   (objects[uwep->otyp].oc_skill < P_DAGGER) ||
	   (objects[uwep->otyp].oc_skill > P_LANCE) ||
	   uwep->otyp == FLAIL || uwep->otyp == AKLYS
#ifdef KOPS
	   || uwep->otyp == RUBBER_HOSE
#endif
	  ) {
#if 0 /*JP*/
	    You_cant("force anything without a %sweapon.",
		  (uwep) ? "proper " : "");
#else
	    pline("%s武器なしで鍵をこじあけることはできない．",
		  (uwep) ? "適切な" : "");
#endif
	    return(0);
	}

#ifdef LIGHTSABERS
	if (is_lightsaber(uwep))
	    picktyp = 2;
	else
#endif
	picktyp = is_blade(uwep) ? 1 : 0;
	if(xlock.usedtime && picktyp == xlock.picktyp) {
	    if (xlock.box) {

		if (!can_reach_floor()) {
#if 0 /*JP*/
		    pline("Unfortunately, you can no longer reach the lock.");
#else
		    pline("ざんねんながら，あなたは鍵穴に届かない．");
#endif
		    return 0;
		}
#if 0 /*JP*/
		You("resume your attempt to force the lock.");
		set_occupation(forcelock, "forcing the lock", 0);
#else
		pline("箱の錠をこじあけるのを再開した．");
		set_occupation(forcelock, "鍵をこじあける", 0);
#endif
		return(1);
	    } else if (xlock.door) {
#if 0 /*JP*/
		You("resume your attempt to force the door.");
		set_occupation(forcedoor, "forcing the door", 0);
#else
		pline("扉をこじあけるのを再開した．");
		set_occupation(forcedoor, "扉をこじあける", 0);
#endif
		return(1);
	    }
	}

	/* A lock is made only for the honest man, the thief will break it. */
	xlock.box = (struct obj *)0;

	if(!getdir((char *)0)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if (x == u.ux && y == u.uy) {
	    if (u.dz < 0) {
#if 0 /*JP*/
		There("isn't any sort of lock up %s.",
		      Levitation ? "here" : "there");
#else
		pline("%sには鍵がかかっていそうな物はない．",
		      Levitation ? "ここ" : "そこ");
#endif
		return 0;
	    } else if (is_lava(u.ux, u.uy)) {
/*JP
		pline("Doing that would probably melt your %s.",
*/
		pline("そんなことをしたら%sが溶けてしまう．",
		      xname(uwep));
		return 0;
	    } else if (is_pool(u.ux, u.uy) && !Underwater) {
/*JP
		pline_The("water has no lock.");
*/
		pline("水に鍵穴はない．");
		return 0;
	    }

	    for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere)
		if(Is_box(otmp)) {
		    if (!can_reach_floor()) {
/*JP
			You_cant("reach %s from up here.", the(xname(otmp)));
*/
			Your("今の高い位置からでは%sに届かない．", the(xname(otmp)));
		    return 0;
		    }

		if (otmp->obroken || !otmp->olocked) {
#if 0 /*JP*/
		    There("is %s here, but its lock is already %s.",
			  doname(otmp), otmp->obroken ? "broken" : "unlocked");
#else
		    pline("ここには%sがあるが，その錠はもう%s．",
			  doname(otmp), otmp->obroken ? "壊れている" : "はずされている");
#endif
		    continue;
		}
#if 0 /*JP*/
		Sprintf(qbuf,"There is %s here, force its lock?",
			safe_qbuf("", sizeof("There is  here, force its lock?"),
				doname(otmp), an(simple_typename(otmp->otyp)),
				"a box"));
#else
		Sprintf(qbuf,"ここには%sがある，錠をこじあけますか？",
			safe_qbuf("", sizeof("ここにはがある，錠をこじあけますか？"),
				doname(otmp), an(simple_typename(otmp->otyp)),
				"箱"));
#endif

		c = ynq(qbuf);
		if(c == 'q') return(0);
		if(c == 'n') continue;

#ifdef LIGHTSABERS
		if(picktyp == 2)
#if 0 /*JP*/
		    You("begin melting it with your %s.", xname(uwep));
#else
		    You("%sで錠前を溶かし始めた．", xname(uwep));
#endif
		else
#endif
		if(picktyp)
#if 0 /*JP*/
		    You("force your %s into a crack and pry.", xname(uwep));
#else
		    You("%sを鍵穴に入れてガチャガチャした．",xname(uwep));
#endif
		else
#if 0 /*JP*/
		    You("start bashing it with your %s.", xname(uwep));
#else
		    pline("%sで殴りつけた．", xname(uwep));
#endif
		xlock.box = otmp;
#ifdef LIGHTSABERS
		if (is_lightsaber(uwep))
		    xlock.chance = uwep->spe * 2 + 75;
		else
#endif
		    xlock.chance = (uwep->spe + objects[uwep->otyp].oc_wldam) * 2;
		xlock.picktyp = picktyp;
		xlock.usedtime = 0;
		break;
	    }
	    if(xlock.box)   {
	    	xlock.door = 0;
#if 0 /*JP*/
	    	set_occupation(forcelock, "forcing the lock", 0);
#else
	    	set_occupation(forcelock, "錠をこじあける", 0);
#endif
	    	return(1);
	    }
	} else {		/* break down/open door */
	    struct monst *mtmp;

	    door = &levl[x][y];
	    if ((mtmp = m_at(x, y)) && canseemon(mtmp)
			&& mtmp->m_ap_type != M_AP_FURNITURE
			&& mtmp->m_ap_type != M_AP_OBJECT) {

		if (mtmp->isshk || mtmp->data == &mons[PM_ORACLE])		
#if 0 /*JP*/
		    verbalize("What do you think you are, a Jedi?"); /* Phantom Menace */
#else
		    verbalize("自分が何をしているのか判っているのか？ジェダイよ．"); /* Phantom Menace */
#endif
		else
#if 0 /*JP*/
		    pline("I don't think %s would appreciate that.", mon_nam(mtmp));
#else
		    pline("%sはそれを評価しないんじゃないかな．", mon_nam(mtmp));
#endif
		return(0);
	    }
	    /* Lightsabers dig through doors and walls via dig.c */
	    if (is_pick(uwep) ||
#ifdef LIGHTSABERS
		    is_lightsaber(uwep) ||
#endif
		    is_axe(uwep)) 
	    	return use_pick_axe2(uwep);

	    if(!IS_DOOR(door->typ)) { 
		if (is_drawbridge_wall(x,y) >= 0)
#if 0 /*JP*/
		    pline("The drawbridge is too solid to force open.");
#else
		    pline("自明な方法じゃ跳ね橋は降りない．");
#endif
		else
#if 0 /*JP*/
		    You("%s no door there.",
				Blind ? "feel" : "see");
#else
		pline("そこには扉はない%s．",
				Blind ? "ようだ" : "ように見える");
#endif
		return(0);
	    }
	    /* ALI - artifact doors */
	    if (artifact_door(x, y)) {
#if 0 /*JP*/
		pline("This door is too solid to force open.");
#else
		pline("この扉はあまりにも強固でこじ開けることができない．");
#endif
		return 0;
	    }
	    switch (door->doormask) {
		case D_NODOOR:
#if 0 /*JP*/
		    pline("This doorway has no door.");
#else
		    pline("この戸口には扉がない．");
#endif
		    return(0);
		case D_ISOPEN:
#if 0 /*JP*/
		    You("cannot force an open door.");
#else
		    pline("開いた扉はこじ開けられない．");
#endif
		    return(0);
		case D_BROKEN:
#if 0 /*JP*/
		    pline("This door is broken.");
#else
		    pline("この扉は壊れてしまっている．");
#endif
		    return(0);
		default:
#if 0 /*JP*/
		    c = yn("Break down the door?");
#else
		    c = yn("扉を破壊しますか？");
#endif
		    if(c == 'n') return(0);

		    if(picktyp == 1)
#if 0 /*JP*/
			You("force your %s into a crack and pry.", xname(uwep));
#else
			You("%sを割れ目にねじ込んでこじ開けた．", xname(uwep));
#endif
		    else
#if 0 /*JP*/
			You("start bashing it with your %s.", xname(uwep));
#else
			You("%sで扉を殴りつけはじめた．", xname(uwep));
#endif
#ifdef LIGHTSABERS
		    if (is_lightsaber(uwep))
			xlock.chance = uwep->spe + 38;
		    else
#endif
			xlock.chance = uwep->spe + objects[uwep->otyp].oc_wldam;
		    xlock.picktyp = picktyp;
		    xlock.usedtime = 0;    
		    xlock.door = door;
		    xlock.box = 0;
/*JP
		    set_occupation(forcedoor, "forcing the door", 0);
*/
		    set_occupation(forcedoor, "鍵をこじあける", 0);
	return(1);
	    }
	}
/*JP
	You("decide not to force the issue.");
*/
	pline("それは無意味な行為だ．");
	return(0);
}

int
doopen()		/* try to open a door */
{
	coord cc;
	register struct rm *door;
	struct monst *mtmp;

	if (nohands(youmonst.data)) {
/*JP
	    You_cant("open anything -- you have no hands!");
*/
	    You("何も開けることができない！手がないんだもの！");
	    return 0;
	}

	if (u.utrap && u.utraptype == TT_PIT) {
/*JP
	    You_cant("reach over the edge of the pit.");
*/
	    pline("落し穴の中からは届かない．");
	    return 0;
	}

	if(!get_adjacent_loc((char *)0, (char *)0, u.ux, u.uy, &cc)) return(0);

	if((cc.x == u.ux) && (cc.y == u.uy)) return(0);

	if ((mtmp = m_at(cc.x,cc.y))			&&
		mtmp->m_ap_type == M_AP_FURNITURE	&&
		(mtmp->mappearance == S_hcdoor ||
			mtmp->mappearance == S_vcdoor)	&&
		!Protection_from_shape_changers)	 {

	    stumble_onto_mimic(mtmp);
	    return(1);
	}

	door = &levl[cc.x][cc.y];

	if(!IS_DOOR(door->typ)) {
		if (is_db_wall(cc.x,cc.y)) {
/*JP
		    There("is no obvious way to open the drawbridge.");
*/
		    pline("跳ね橋を降ろす明白な方法はない．");
		    return(0);
		}
#if 0 /*JP*/
		You("%s no door there.",
				Blind ? "feel" : "see");
#else
		pline("そこには扉はない%s．",
				Blind ? "ようだ" : "ように見える");
#endif
		return(0);
	}

	if (!(door->doormask & D_CLOSED)) {
#if 0 /*JP*/
	    const char *mesg;

	    switch (door->doormask) {
	    case D_BROKEN: mesg = " is broken"; break;
	    case D_NODOOR: mesg = "way has no door"; break;
	    case D_ISOPEN: mesg = " is already open"; break;
	    default:	   mesg = " is locked"; break;
	    }
	    pline("This door%s.", mesg);
#else
	    switch(door->doormask) {
		case D_BROKEN:
		  pline("扉は壊れている．"); 
		  break;
		case D_NODOOR:
		  pline("出入口には扉がない．");
		  break;
		case D_ISOPEN:
		  pline("扉はもう開いている．");
		  break;
		default:
		  pline("扉には鍵が掛かっている．"); 
		  break;
	    }
#endif
	    if (Blind) feel_location(cc.x,cc.y);
	    return(0);
	}

	if(verysmall(youmonst.data)) {
/*JP
	    pline("You're too small to pull the door open.");
*/
	    You("小さすぎて扉を開けられない．");
	    return(0);
	}

	/* door is known to be CLOSED */
	if (rnl(20) < (ACURRSTR+ACURR(A_DEX)+ACURR(A_CON))/3) {
/*JP
	    pline_The("door opens.");
*/
	    pline("扉は開いた．");
	    if(door->doormask & D_TRAPPED) {
/*JP
		b_trapped("door", FINGER);
*/
		b_trapped("扉", FINGER);
		door->doormask = D_NODOOR;
		if (*in_rooms(cc.x, cc.y, SHOPBASE)) add_damage(cc.x, cc.y, 0L);
	    } else
		door->doormask = D_ISOPEN;
	    if (Blind)
		feel_location(cc.x,cc.y);	/* the hero knows she opened it  */
	    else
		newsym(cc.x,cc.y);
	    unblock_point(cc.x,cc.y);		/* vision: new see through there */
	} else {
	    exercise(A_STR, TRUE);
/*JP
	    pline_The("door resists!");
*/
	    pline("なかなか開かない！");
	}

	return(1);
}

STATIC_OVL
boolean
obstructed(x,y)
register int x, y;
{
	register struct monst *mtmp = m_at(x, y);

	if(mtmp && mtmp->m_ap_type != M_AP_FURNITURE) {
		if (mtmp->m_ap_type == M_AP_OBJECT) goto objhere;
#if 0 /*JP*/
		pline("%s stands in the way!", !canspotmon(mtmp) ?
			"Some creature" : Monnam(mtmp));
#else
		pline("%sが立ちふさがっている！", !canspotmon(mtmp) ?
			"何者か" : Monnam(mtmp));
#endif
		if (!canspotmon(mtmp))
		    map_invisible(mtmp->mx, mtmp->my);
		return(TRUE);
	}
	if (OBJ_AT(x, y)) {
/*JP
objhere:	pline("%s's in the way.", Something);
*/
objhere:	pline("何かが出入口にある．");
		return(TRUE);
	}
	return(FALSE);
}

int
doclose()		/* try to close a door */
{
	register int x, y;
	register struct rm *door;
	struct monst *mtmp;

	if (nohands(youmonst.data)) {
/*JP
	    You_cant("close anything -- you have no hands!");
*/
	    You("閉めることができない！手がないんだもの！");
	    return 0;
	}

	if (u.utrap && u.utraptype == TT_PIT) {
/*JP
	    You_cant("reach over the edge of the pit.");
*/
	    pline("落し穴の中からは届かない．");
	    return 0;
	}

	if(!getdir((char *)0)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if((x == u.ux) && (y == u.uy)) {
/*JP
		You("are in the way!");
*/
		pline("あなたが出入口にいるので閉まらない！");
		return(1);
	}

	if ((mtmp = m_at(x,y))				&&
		mtmp->m_ap_type == M_AP_FURNITURE	&&
		(mtmp->mappearance == S_hcdoor ||
			mtmp->mappearance == S_vcdoor)	&&
		!Protection_from_shape_changers)	 {

	    stumble_onto_mimic(mtmp);
	    return(1);
	}

	door = &levl[x][y];

	if(!IS_DOOR(door->typ)) {
		if (door->typ == DRAWBRIDGE_DOWN)
/*JP
		    There("is no obvious way to close the drawbridge.");
*/
		    pline("跳ね橋を上げる明白な手段が見当たらない．");
		else
#if 0 /*JP*/
		    You("%s no door there.",
				Blind ? "feel" : "see");
#else
		    pline("そこに扉はない%s．",
				Blind ? "ようだ" : "ように見える");
#endif
		return(0);
	}

	if(door->doormask == D_NODOOR) {
/*JP
	    pline("This doorway has no door.");
*/
	    pline("この戸口には扉がない．");
	    return(0);
	}

	if(obstructed(x, y)) return(0);

	if(door->doormask == D_BROKEN) {
/*JP
	    pline("This door is broken.");
*/
	    pline("この扉は壊れてしまっている．");
	    return(0);
	}

	if(door->doormask & (D_CLOSED | D_LOCKED)) {
/*JP
	    pline("This door is already closed.");
*/
	    pline("扉はもう閉じている．");
	    return(0);
	}

	if(door->doormask == D_ISOPEN) {
	    if(verysmall(youmonst.data)
#ifdef STEED
		&& !u.usteed
#endif
		) {
/*JP
		 pline("You're too small to push the door closed.");
*/
		 You("小さすぎて扉を閉められない．");
		 return(0);
	    }
	    if (
#ifdef STEED
		 u.usteed ||
#endif
		rn2(25) < (ACURRSTR+ACURR(A_DEX)+ACURR(A_CON))/3) {
/*JP
		pline_The("door closes.");
*/
		pline("扉は閉じた．");
		door->doormask = D_CLOSED;
		if (Blind)
		    feel_location(x,y);	/* the hero knows she closed it */
		else
		    newsym(x,y);
		block_point(x,y);	/* vision:  no longer see there */
	    }
	    else {
	        exercise(A_STR, TRUE);
/*JP
	        pline_The("door resists!");
*/
	        pline("なかなか閉まらない！");
	    }
	}

	return(1);
}

boolean			/* box obj was hit with spell effect otmp */
boxlock(obj, otmp)	/* returns true if something happened */
register struct obj *obj, *otmp;	/* obj *is* a box */
{
	register boolean res = 0;

	switch(otmp->otyp) {
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
	    if (!obj->olocked) {	/* lock it; fix if broken */
/*JP
		pline("Klunk!");
*/
		pline("カチ！");
		obj->olocked = 1;
		obj->obroken = 0;
		res = 1;
	    } /* else already closed and locked */
	    break;
	case WAN_OPENING:
	case SPE_KNOCK:
	    if (obj->olocked) {		/* unlock; couldn't be broken */
/*JP
		pline("Klick!");
*/
		pline("コンコン！");
		obj->olocked = 0;
		res = 1;
	    } else			/* silently fix if broken */
		obj->obroken = 0;
	    break;
	case WAN_POLYMORPH:
	case SPE_POLYMORPH:
	    /* maybe start unlocking chest, get interrupted, then zap it;
	       we must avoid any attempt to resume unlocking it */
	    if (xlock.box == obj)
		reset_pick();
	    break;
	}
	return res;
}

boolean			/* Door/secret door was hit with spell effect otmp */
doorlock(otmp,x,y)	/* returns true if something happened */
struct obj *otmp;
int x, y;
{
	register struct rm *door = &levl[x][y];
	boolean res = TRUE;
	int loudness = 0;
	const char *msg = (const char *)0;
#if 0 /*JP*/
	const char *dustcloud = "A cloud of dust";
	const char *quickly_dissipates = "quickly dissipates";
#else
	const char *dustcloud = "ほこり";
	const char *quickly_dissipates = "あっと言うまに飛び散った";
#endif
	int key = artifact_door(x, y);		/* ALI - Artifact doors */
	
	if (door->typ == SDOOR) {
	    switch (otmp->otyp) {
	    case WAN_OPENING:
	    case SPE_KNOCK:
	    case WAN_STRIKING:
	    case SPE_FORCE_BOLT:
		if (key)	/* Artifact doors are revealed only */
		    cvt_sdoor_to_door(door);
		else {
		door->typ = DOOR;
		door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
		}
		newsym(x,y);
/*JP
		if (cansee(x,y)) pline("A door appears in the wall!");
*/
		if (cansee(x,y)) pline("壁から扉が現れた！");
		if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK)
		    return TRUE;
		break;		/* striking: continue door handling below */
	    case WAN_LOCKING:
	    case SPE_WIZARD_LOCK:
	    default:
		return FALSE;
	    }
	}

	switch(otmp->otyp) {
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
#ifdef REINCARNATION
	    if (Is_rogue_level(&u.uz)) {
	    	boolean vis = cansee(x,y);
		/* Can't have real locking in Rogue, so just hide doorway */
/*JP
		if (vis) pline("%s springs up in the older, more primitive doorway.",
*/
		if (vis) pline("古くさい，原始的な出入口に%sが立ちこめた．",
			dustcloud);
		else
/*JP
			You_hear("a swoosh.");
*/
			You_hear("シューッという音を聞いた．");
		if (obstructed(x,y)) {
/*JP
			if (vis) pline_The("cloud %s.",quickly_dissipates);
*/
			if (vis) pline("ほこりは%s．",quickly_dissipates);
			return FALSE;
		}
		block_point(x, y);
		door->typ = SDOOR;
/*JP
		if (vis) pline_The("doorway vanishes!");
*/
		if (vis) pline("出入口は消えた！");
		newsym(x,y);
		return TRUE;
	    }
#endif
	    if (obstructed(x,y)) return FALSE;
	    /* Don't allow doors to close over traps.  This is for pits */
	    /* & trap doors, but is it ever OK for anything else? */
	    if (t_at(x,y)) {
		/* maketrap() clears doormask, so it should be NODOOR */
		pline(
#if 0 /*JP*/
		"%s springs up in the doorway, but %s.",
#else
		"%sが出入口に立ちこめたが，%s．",
#endif
		dustcloud, quickly_dissipates);
		return FALSE;
	    }

	    switch (door->doormask & ~D_TRAPPED) {
	    case D_CLOSED:
		if (key)
#if 0 /*JP*/
		    msg = "The door closes!";
#else
		    msg = "扉が閉まった！";
#endif
		else
#if 0 /*JP*/
		msg = "The door locks!";
#else
		    msg = "扉に鍵がかかった！";
#endif
		break;
	    case D_ISOPEN:
		if (key)
#if 0 /*JP*/
		    msg = "The door swings shut!";
#else
		    msg = "扉は勢いよく閉まった！";
#endif
		else
#if 0 /*JP*/
		msg = "The door swings shut, and locks!";
#else
		    msg = "扉は勢いよく閉まり，鍵がかかった！";
#endif
		break;
	    case D_BROKEN:
		if (key)
#if 0 /*JP*/
		    msg = "The broken door reassembles!";
#else
		    msg = "壊れた扉が集まって元通りになった！";
#endif
		else
#if 0 /*JP*/
		msg = "The broken door reassembles and locks!";
#else
		    msg = "壊れた扉が集まって元通りになり，鍵がかかった！";
#endif
		break;
	    case D_NODOOR:
		msg =
#if 0 /*JP*/
		"A cloud of dust springs up and assembles itself into a door!";
#else
		"ほこりがたちこめ，集まって扉になった！";
#endif
		break;
	    default:
		res = FALSE;
		break;
	    }
	    block_point(x, y);
	    if (key)
		door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
	    else
	    door->doormask = D_LOCKED | (door->doormask & D_TRAPPED);
	    newsym(x,y);
	    break;
	case WAN_OPENING:
	case SPE_KNOCK:
	    if (!key && door->doormask & D_LOCKED) {
#if 0 /*JP*/
		msg = "The door unlocks!";
#else
		msg = "扉の鍵ははずれた！";
#endif
		door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
	    } else res = FALSE;
	    break;
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
	    if (!key && door->doormask & (D_LOCKED | D_CLOSED)) {
		if (door->doormask & D_TRAPPED) {
		    if (MON_AT(x, y))
			(void) mb_trapped(m_at(x,y));
		    else if (flags.verbose) {
			if (cansee(x,y))
/*JP
			    pline("KABOOM!!  You see a door explode.");
*/
			    pline("ちゅどーん！扉が爆発した．");
			else if (flags.soundok)
/*JP
			    You_hear("a distant explosion.");
*/
			    You_hear("遠くの爆発音を聞いた．");
		    }
		    door->doormask = D_NODOOR;
		    unblock_point(x,y);
		    newsym(x,y);
		    loudness = 40;
		    break;
		}
		door->doormask = D_BROKEN;
		if (flags.verbose) {
		    if (cansee(x,y))
/*JP
			pline_The("door crashes open!");
*/
			pline("扉は壊れ開いた！");
		    else if (flags.soundok)
/*JP
			You_hear("a crashing sound.");
*/
			You_hear("何かが壊れる音を聞いた．");
		}
		unblock_point(x,y);
		newsym(x,y);
		/* force vision recalc before printing more messages */
		if (vision_full_recalc) vision_recalc(0);
		loudness = 20;
	    } else res = FALSE;
	    break;
	default: impossible("magic (%d) attempted on door.", otmp->otyp);
	    break;
	}
	if (msg && cansee(x,y)) pline(msg);
	if (loudness > 0) {
	    /* door was destroyed */
	    wake_nearto(x, y, loudness);
	    if (*in_rooms(x, y, SHOPBASE)) add_damage(x, y, 0L);
	}

	if (res && picking_at(x, y)) {
	    /* maybe unseen monster zaps door you're unlocking */
	    stop_occupation();
	    reset_pick();
	}
	return res;
}

STATIC_OVL void
chest_shatter_msg(otmp)
struct obj *otmp;
{
	const char *disposition;
	const char *thing;
	long save_Blinded;

	if (otmp->oclass == POTION_CLASS) {
#if 0 /*JP*/
		You("%s %s shatter!", Blind ? "hear" : "see", an(bottlename()));
#else
		if (Blind)
		    You_hear("%sが割れる音を聞いた！", bottlename());
		else
		    pline("%sが割れた！", bottlename());
#endif /*JP*/
		if (!breathless(youmonst.data) || haseyes(youmonst.data))
			potionbreathe(otmp);
		return;
	}
	/* We have functions for distant and singular names, but not one */
	/* which does _both_... */
	save_Blinded = Blinded;
	Blinded = 1;
	thing = singular(otmp, xname);
	Blinded = save_Blinded;
	switch (objects[otmp->otyp].oc_material) {
/*JP
	case PAPER:	disposition = "is torn to shreds";
*/
	case PAPER:	disposition = "は千切れ飛んだ";
		break;
/*JP
	case WAX:	disposition = "is crushed";
*/
	case WAX:	disposition = "は砕け散った";
		break;
/*JP
	case VEGGY:	disposition = "is pulped";
*/
	case VEGGY:	disposition = "はどろどろになった";
		break;
/*JP
	case FLESH:	disposition = "is mashed";
*/
	case FLESH:	disposition = "は叩き潰された";
		break;
/*JP
	case GLASS:	disposition = "shatters";
*/
	case GLASS:	disposition = "は割れた";
		break;
/*JP
	case WOOD:	disposition = "splinters to fragments";
*/
	case WOOD:	disposition = "は木っ端微塵になった";
		break;
/*JP
	default:	disposition = "is destroyed";
*/
	default:	disposition = "は破壊された";
		break;
	}
/*JP
	pline("%s %s!", An(thing), disposition);
*/
	pline("%s%s！", thing, disposition);
}

/* ALI - Kevin Hugo's artifact doors.
 * Return the artifact which unlocks the door at (x, y), or
 * zero if it is an ordinary door.
 * Note: Not all doors are listed in the doors array (eg., doors
 * dynamically converted from secret doors). Since only trapped
 * and artifact doors are needed this isn't a problem. If we ever
 * implement trapped secret doors we will have to extend this.
 */

int
artifact_door(x, y)
int x, y;
{
    int i;

    for(i = 0; i < doorindex; i++) {
	if (x == doors[i].x && y == doors[i].y)
	    return doors[i].arti_key;
    }
    return 0;
}

#endif /* OVLB */

/*lock.c*/
