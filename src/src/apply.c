/*	SCCS Id: @(#)apply.c	3.4	2003/11/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, 2004-2008
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"
#include "edog.h"

#ifdef OVLB

static const char tools[] = { TOOL_CLASS, WEAPON_CLASS, WAND_CLASS, 0 };
static const char all_count[] = { ALLOW_COUNT, ALL_CLASSES, 0 };
static const char tools_too[] = { ALL_CLASSES, TOOL_CLASS, POTION_CLASS,
				  WEAPON_CLASS, WAND_CLASS, GEM_CLASS, 0 };
static const char tinnables[] = { ALLOW_FLOOROBJ, FOOD_CLASS, 0 };

#ifdef TOURIST
STATIC_DCL int FDECL(use_camera, (struct obj *));
#endif
STATIC_DCL int FDECL(use_towel, (struct obj *));
STATIC_DCL boolean FDECL(its_dead, (int,int,int *));
STATIC_DCL int FDECL(use_stethoscope, (struct obj *));
STATIC_DCL void FDECL(use_whistle, (struct obj *));
STATIC_DCL void FDECL(use_magic_whistle, (struct obj *));
STATIC_DCL void FDECL(use_leash, (struct obj *));
STATIC_DCL int FDECL(use_mirror, (struct obj *));
STATIC_DCL void FDECL(use_bell, (struct obj **));
STATIC_DCL void FDECL(use_candelabrum, (struct obj *));
STATIC_DCL void FDECL(use_candle, (struct obj **));
STATIC_DCL void FDECL(use_lamp, (struct obj *));
STATIC_DCL int FDECL(use_torch, (struct obj *));
STATIC_DCL void FDECL(light_cocktail, (struct obj *));
STATIC_DCL void FDECL(use_tinning_kit, (struct obj *));
STATIC_DCL void FDECL(use_figurine, (struct obj **));
STATIC_DCL void FDECL(use_grease, (struct obj *));
STATIC_DCL void FDECL(use_trap, (struct obj *));
STATIC_DCL void FDECL(use_stone, (struct obj *));
STATIC_PTR int NDECL(set_trap);		/* occupation callback */
STATIC_DCL int FDECL(use_whip, (struct obj *));
STATIC_DCL int FDECL(use_pole, (struct obj *));
STATIC_DCL int FDECL(use_cream_pie, (struct obj *));
STATIC_DCL int FDECL(use_grapple, (struct obj *));
STATIC_DCL int FDECL(do_break_wand, (struct obj *));
STATIC_DCL boolean FDECL(figurine_location_checks,
				(struct obj *, coord *, BOOLEAN_P));
STATIC_DCL boolean NDECL(uhave_graystone);
STATIC_DCL void FDECL(add_class, (char *, CHAR_P));

#ifdef	AMIGA
void FDECL( amii_speaker, ( struct obj *, char *, int ) );
#endif

/*JP
const char no_elbow_room[] = "don't have enough elbow-room to maneuver.";
*/
const char no_elbow_room[] = "腕を動かせるだけの余裕がない．";

#ifdef TOURIST
STATIC_OVL int
use_camera(obj)
	struct obj *obj;
{
	register struct monst *mtmp;

	if(Underwater) {
/*JP
		pline("Using your camera underwater would void the warranty.");
*/
		pline("水中でのカメラの使用は保証の対象外となります．");
		return(0);
	}
	if(!getdir((char *)0)) return(0);

	if (obj->spe <= 0) {
		pline(nothing_happens);
		return (1);
	}
	consume_obj_charge(obj, TRUE);

	if (obj->cursed && !rn2(2)) {
		(void) zapyourself(obj, TRUE);
	} else if (u.uswallow) {
#if 0 /*JP*/
		You("take a picture of %s %s.", s_suffix(mon_nam(u.ustuck)),
		    mbodypart(u.ustuck, STOMACH));
#else
		You("%sの%sの写真を撮った．", mon_nam(u.ustuck),
		    mbodypart(u.ustuck, STOMACH));
#endif
	} else if (u.dz) {
/*JP
		You("take a picture of the %s.",
*/
		You("%sの写真を撮った．",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
	} else if (!u.dx && !u.dy) {
		(void) zapyourself(obj, TRUE);
	} else if ((mtmp = bhit(u.dx,u.dy,COLNO,FLASHED_LIGHT,
				(int FDECL((*),(MONST_P,OBJ_P)))0,
				(int FDECL((*),(OBJ_P,OBJ_P)))0,
				&obj)) != 0) {
		obj->ox = u.ux,  obj->oy = u.uy;
		(void) flash_hits_mon(mtmp, obj);
	}
	return 1;
}
#endif

STATIC_OVL int
use_towel(obj)
	struct obj *obj;
{
	if(!freehand()) {
/*JP
		You("have no free %s!", body_part(HAND));
*/
		You("%sが空いていない！", body_part(HAND));
		return 0;
	} else if (obj->owornmask) {
/*JP
		You("cannot use it while you're wearing it!");
*/
		You("身につけているものを使うことはできない！");
		return 0;
	} else if (obj->cursed) {
		long old;
		switch (rn2(3)) {
		case 2:
		    old = Glib;
		    incr_itimeout(&Glib, rn1(10, 3));
#if 0 /*JP*/
		    Your("%s %s!", makeplural(body_part(HAND)),
			(old ? "are filthier than ever" : "get slimy"));
#else
		    Your("両%sは%s！", body_part(HAND),
			(old ? "ますます汚なくなった" : "ぬるぬるになった"));
#endif
		    return 1;
		case 1:
		    if (!ublindf) {
			old = u.ucreamed;
			u.ucreamed += rn1(10, 3);
#if 0 /*JP*/
			pline("Yecch! Your %s %s gunk on it!", body_part(FACE),
			      (old ? "has more" : "now has"));
#else
			pline("ゲェー！あなたの%sは%sべとべとになった！", body_part(FACE),
			      (old ? "もっと" : ""));
#endif
			make_blinded(Blinded + (long)u.ucreamed - old, TRUE);
		    } else {
			const char *what = (ublindf->otyp == LENSES) ?
#if 0 /*JP*/
					    "lenses" : "blindfold";
#endif
					    "レンズ" : "目隠し";
			if (ublindf->cursed) {
#if 0 /*JP*/
			    You("push your %s %s.", what,
				rn2(2) ? "cock-eyed" : "crooked");
#else
			    You("%sを押し%sた．", what,
				rn2(2) ? "歪め" : "曲げ");
#endif
			} else {
			    struct obj *saved_ublindf = ublindf;
#if 0 /*JP*/
			    You("push your %s off.", what);
#else
			    You("%sを押しやった．", what);
#endif
			    Blindf_off(ublindf);
			    dropx(saved_ublindf);
			}
		    }
		    return 1;
		case 0:
		    break;
		}
	}

	if (Glib) {
		Glib = 0;
/*JP
		You("wipe off your %s.", makeplural(body_part(HAND)));
*/
		You("%sを拭いた．", makeplural(body_part(HAND)));
		return 1;
	} else if(u.ucreamed) {
		Blinded -= u.ucreamed;
		u.ucreamed = 0;

		if (!Blinded) {
/*JP
			pline("You've got the glop off.");
*/
			You("汚れを拭きとった．");
			Blinded = 1;
			make_blinded(0L,TRUE);
		} else {
/*JP
			Your("%s feels clean now.", body_part(FACE));
*/
			pline("%sはさっぱりした．", body_part(FACE));
		}
		return 1;
	}

/*JP
	Your("%s and %s are already clean.",
*/
	Your("%sや%sは汚れていない．",
		body_part(FACE), makeplural(body_part(HAND)));

	return 0;
}

/* maybe give a stethoscope message based on floor objects */
STATIC_OVL boolean
its_dead(rx, ry, resp)
int rx, ry, *resp;
{
	struct obj *otmp;
	struct trap *ttmp;

	if (!can_reach_floor()) return FALSE;

	/* additional stethoscope messages from jyoung@apanix.apana.org.au */
	if (Hallucination && sobj_at(CORPSE, rx, ry)) {
	    /* (a corpse doesn't retain the monster's sex,
	       so we're forced to use generic pronoun here) */
/*JP
	    You_hear("a voice say, \"It's dead, Jim.\"");
*/
	    You_hear("「それは死んでるぜ，ジム」という声が聞こえた．");
	    *resp = 1;
	    return TRUE;
	} else if (Role_if(PM_HEALER) && ((otmp = sobj_at(CORPSE, rx, ry)) != 0 ||
				    (otmp = sobj_at(STATUE, rx, ry)) != 0)) {
	    /* possibly should check uppermost {corpse,statue} in the pile
	       if both types are present, but it's not worth the effort */
	    if (vobj_at(rx, ry)->otyp == STATUE) otmp = vobj_at(rx, ry);
	    if (otmp->otyp == CORPSE) {
#if 0 /*JP*/
		You("determine that %s unfortunate being is dead.",
		    (rx == u.ux && ry == u.uy) ? "this" : "that");
#else
		You("%s不幸な生き物は死んでいると結論した．",
		    (rx == u.ux && ry == u.uy) ? "この" : "その");
#endif
	    } else {
		ttmp = t_at(rx, ry);
#if 0 /*JP*/
		pline("%s appears to be in %s health for a statue.",
		      The(mons[otmp->corpsenm].mname),
		      (ttmp && ttmp->ttyp == STATUE_TRAP) ?
			"extraordinary" : "excellent");
#else
		pline("%sは彫像にしては%sている．",
		      jtrns_mon(The(mons[otmp->corpsenm].mname)),
		      (ttmp && ttmp->ttyp == STATUE_TRAP) ?
			"信じられないほど生命力に溢れ" : "躍動感に満ち");
#endif
	    }
	    return TRUE;
	}
	return FALSE;
}

/*JP
static const char hollow_str[] = "a hollow sound.  This must be a secret %s!";
*/
static const char hollow_str[] = "うつろな音を聞いた．隠し%sに違いない！";

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless.  As a compromise, one use per turn is free, another
   uses up the turn; this makes curse status have a tangible effect. */
STATIC_OVL int
use_stethoscope(obj)
	register struct obj *obj;
{
	static long last_used_move = -1;
	static short last_used_movement = 0;
	struct monst *mtmp;
	struct rm *lev;
	int rx, ry, res;
	boolean interference = (u.uswallow && is_whirly(u.ustuck->data) &&
				!rn2(Role_if(PM_HEALER) ? 10 : 3));

	if (nohands(youmonst.data)) {	/* should also check for no ears and/or deaf */
#if 0 /*JP*/
		You("have no hands!");	/* not `body_part(HAND)' */
#else
		pline("あなたには手がない！");
#endif
		return 0;
	} else if (!freehand()) {
/*JP
		You("have no free %s.", body_part(HAND));
*/
		You("%sが空いていない．", body_part(HAND));
		return 0;
	}
	if (!getdir((char *)0)) return 0;

	res = (moves == last_used_move) &&
	      (youmonst.movement == last_used_movement);
	last_used_move = moves;
	last_used_movement = youmonst.movement;

#ifdef STEED
	if (u.usteed && u.dz > 0) {
		if (interference) {
/*JP
			pline("%s interferes.", Monnam(u.ustuck));
*/
			pline("%sがじゃまだ．", Monnam(u.ustuck));
			mstatusline(u.ustuck);
		} else
			mstatusline(u.usteed);
		return res;
	} else
#endif
	if (u.uswallow && (u.dx || u.dy || u.dz)) {
		mstatusline(u.ustuck);
		return res;
	} else if (u.uswallow && interference) {
/*JP
		pline("%s interferes.", Monnam(u.ustuck));
*/
		pline("%sがじゃまをした．", Monnam(u.ustuck));
		mstatusline(u.ustuck);
		return res;
	} else if (u.dz) {
		if (Underwater)
/*JP
		    You_hear("faint splashing.");
*/
		    You_hear("遠くでバシャバシャという音を聞いた．");
		else if (u.dz < 0 || !can_reach_floor())
#if 0 /*JP*/
		    You_cant("reach the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
#else
		    You("%sに届かない．",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
#endif
		else if (its_dead(u.ux, u.uy, &res))
		    ;	/* message already given */
		else if (Is_stronghold(&u.uz))
/*JP
		    You_hear("the crackling of hellfire.");
*/
		    You_hear("地獄の炎がパチパチ燃えている音を聞いた．");
		else
/*JP
		    pline_The("%s seems healthy enough.", surface(u.ux,u.uy));
*/
		    pline("%sは十分健康のようだ．", surface(u.ux,u.uy));
		return res;
	} else if (obj->cursed && !rn2(2)) {
/*JP
		You_hear("your heart beat.");
*/
		You_hear("自分の心臓の鼓動を聞いた．");
		return res;
	}
	if (Stunned || (Confusion && !rn2(5))) confdir();
	if (!u.dx && !u.dy) {
		ustatusline();
		return res;
	}
	rx = u.ux + u.dx; ry = u.uy + u.dy;
	if (!isok(rx,ry)) {
/*JP
		You_hear("a faint typing noise.");
*/
		You_hear("遠くでだれかがタイピングしている音を聞いた．");
		return 0;
	}
	if ((mtmp = m_at(rx,ry)) != 0) {
		mstatusline(mtmp);
		if (mtmp->mundetected) {
			mtmp->mundetected = 0;
			if (cansee(rx,ry)) newsym(mtmp->mx,mtmp->my);
		}
		if (!canspotmon(mtmp))
			map_invisible(rx,ry);
		return res;
	}
	if (memory_is_invisible(rx, ry)) {
		unmap_object(rx, ry);
		newsym(rx, ry);
/*JP
		pline_The("invisible monster must have moved.");
*/
		pline_The("見えない怪物は移動してしまったようだ．");
	}
	lev = &levl[rx][ry];
	switch(lev->typ) {
	case SDOOR:
/*JP
		You_hear(hollow_str, "door");
*/
		You_hear(hollow_str, "扉");
		cvt_sdoor_to_door(lev);		/* ->typ = DOOR */
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	case SCORR:
/*JP
		You_hear(hollow_str, "passage");
*/
		You_hear(hollow_str, "通路");
		lev->typ = CORR;
		unblock_point(rx,ry);
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	}

	if (!its_dead(rx, ry, &res))
#if 0 /*JP*/
	    You("hear nothing special.");	/* not You_hear()  */
#else
	    pline("別に何も聞こえない．");
#endif
	return res;
}

/*JP
static const char whistle_str[] = "produce a %s whistling sound.";
*/
static char whistle_str[] = "笛を吹いて%s音をたてた．";

STATIC_OVL void
use_whistle(obj)
struct obj *obj;
{
/*JP
	You(whistle_str, obj->cursed ? "shrill" : "high");
*/
	You(whistle_str, obj->cursed ? "不気味な" : "かん高い");
	wake_nearby();
}

STATIC_OVL void
use_magic_whistle(obj)
struct obj *obj;
{
	register struct monst *mtmp, *nextmon;

	if(obj->cursed && !rn2(2)) {
/*JP
		You("produce a high-pitched humming noise.");
*/
		You("調子の高いうなるような音をたてた．");
		wake_nearby();
	} else {
		int pet_cnt = 0;
/*JP
		You(whistle_str, Hallucination ? "normal" : "strange");
*/
		You(whistle_str, Hallucination ? "ありきたりな" : "奇妙な");
		for(mtmp = fmon; mtmp; mtmp = nextmon) {
		    nextmon = mtmp->nmon; /* trap might kill mon */
		    if (DEADMONSTER(mtmp)) continue;
		    if (mtmp->mtame) {
			if (mtmp->mtrapped) {
			    /* no longer in previous trap (affects mintrap) */
			    mtmp->mtrapped = 0;
			    fill_pit(mtmp->mx, mtmp->my);
			}
			mnexto(mtmp);
			if (canspotmon(mtmp)) ++pet_cnt;
			if (mintrap(mtmp) == 2) change_luck(-1);
		    }
		}
		if (pet_cnt > 0) makeknown(obj->otyp);
	}
}

boolean
um_dist(x,y,n)
register xchar x, y, n;
{
	return((boolean)(abs(u.ux - x) > n  || abs(u.uy - y) > n));
}

int
number_leashed()
{
	register int i = 0;
	register struct obj *obj;

	for(obj = invent; obj; obj = obj->nobj)
		if(obj->otyp == LEASH && obj->leashmon != 0) i++;
	return(i);
}

void
o_unleash(otmp)		/* otmp is about to be destroyed or stolen */
register struct obj *otmp;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->m_id == (unsigned)otmp->leashmon)
			mtmp->mleashed = 0;
	otmp->leashmon = 0;
}

void
m_unleash(mtmp, feedback)	/* mtmp is about to die, or become untame */
register struct monst *mtmp;
boolean feedback;
{
	register struct obj *otmp;

	if (feedback) {
	    if (canseemon(mtmp))
/*JP
		pline("%s pulls free of %s leash!", Monnam(mtmp), mhis(mtmp));
*/
		pline("%sは紐を引っぱって逃れた！", Monnam(mtmp));
	    else
/*JP
		Your("leash falls slack.");
*/
		Your("紐がたるんで落ちた．");
	}
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH &&
				otmp->leashmon == (int)mtmp->m_id)
			otmp->leashmon = 0;
	mtmp->mleashed = 0;
}

void
unleash_all()		/* player is about to die (for bones) */
{
	register struct obj *otmp;
	register struct monst *mtmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH) otmp->leashmon = 0;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		mtmp->mleashed = 0;
}

#define MAXLEASHED	2

/* ARGSUSED */
STATIC_OVL void
use_leash(obj)
struct obj *obj;
{
	coord cc;
	register struct monst *mtmp;
	int spotmon;

	if(!obj->leashmon && number_leashed() >= MAXLEASHED) {
/*JP
		You("cannot leash any more pets.");
*/
		You("これ以上ペットに紐をかけられない．");
		return;
	}

	if(!get_adjacent_loc((char *)0, (char *)0, u.ux, u.uy, &cc)) return;

	if((cc.x == u.ux) && (cc.y == u.uy)) {
#ifdef STEED
		if (u.usteed && u.dz > 0) {
		    mtmp = u.usteed;
		    spotmon = 1;
		    goto got_target;
		}
#endif
/*JP
		pline("Leash yourself?  Very funny...");
*/
		pline("自分を縛る？そりゃ傑作だ．．．");
		return;
	}

	if(!(mtmp = m_at(cc.x, cc.y))) {
/*JP
		There("is no creature there.");
*/
		pline("そこには生き物はいない．");
		return;
	}

	spotmon = canspotmon(mtmp);
#ifdef STEED
 got_target:
#endif

	/* KMH, balance patch -- This doesn't work properly.
	 * Pets need extra memory for their edog structure.
	 * Normally, this is handled by tamedog(), but that
	 * rejects all demons.  Our other alternative would
	 * be to duplicate tamedog()'s functionality here.
	 * Yuck.  So I've merged it into the nymph code below.
	if (((mtmp->data == &mons[PM_SUCCUBUS]) || (mtmp->data == &mons[PM_INCUBUS]))
	     && (!mtmp->mtame) && (spotmon) && (!mtmp->mleashed)) {
	       pline("%s smiles seductively at the sight of this prop!", Monnam(mtmp));
	       mtmp->mtame = 10;
	       mtmp->mpeaceful = 1;
	       set_malign(mtmp);
	}*/
	if ((mtmp->data->mlet == S_NYMPH || mtmp->data == &mons[PM_SUCCUBUS]
		 || mtmp->data == &mons[PM_INCUBUS])
	     && (spotmon) && (!mtmp->mleashed)) {
#if 0 /*JP*/
	       pline("%s looks shocked! \"I'm not that way!\"", Monnam(mtmp));
#else
	       pline("%sはショックを受けたように見える！「私にその気はない！」", Monnam(mtmp));
#endif
	       mtmp->mtame = 0;
	       mtmp->mpeaceful = 0;
	       mtmp->msleeping = 0;
	}
	if(!mtmp->mtame) {
	    if(!spotmon)
#if 0 /*JP*/
		There("is no creature there.");
#else
		pline("そこには生き物はいない．");
#endif
	    else
#if 0 /*JP*/
		pline("%s %s leashed!", Monnam(mtmp), (!obj->leashmon) ?
				"cannot be" : "is not");
#else
		pline("%sは紐で%s！", Monnam(mtmp), (!obj->leashmon) ?
				"結べない" : "結ばれていない");
#endif
	    return;
	}
	if(!obj->leashmon) {
		if(mtmp->mleashed) {
#if 0 /*JP*/
			pline("This %s is already leashed.",
			      spotmon ? l_monnam(mtmp) : "monster");
#else
			pline("%sはすでに結びつけられている．",
			      spotmon ? l_monnam(mtmp) : "怪物");
#endif
			return;
		}
#if 0 /*JP*/
		You("slip the leash around %s%s.",
		    spotmon ? "your " : "", l_monnam(mtmp));
#else
		You("%sを紐で結びつけた．",
		    l_monnam(mtmp));
#endif
		mtmp->mleashed = 1;
		obj->leashmon = (int)mtmp->m_id;
		mtmp->msleeping = 0;
		return;
	}
	if(obj->leashmon != (int)mtmp->m_id) {
#if 0 /*JP*/
		pline("This leash is not attached to that creature.");
#else
		pline("この紐はそれには結ばれていない．");
#endif
		return;
	} else {
		if(obj->cursed) {
#if 0 /*JP*/
			pline_The("leash would not come off!");
#else
			pline("紐がはずれない！");
#endif
			obj->bknown = TRUE;
			return;
		}
		mtmp->mleashed = 0;
		obj->leashmon = 0;
#if 0 /*JP*/
		You("remove the leash from %s%s.",
		    spotmon ? "your " : "", l_monnam(mtmp));
#else
		You("%sから紐をはずした．", l_monnam(mtmp));
#endif
		/* KMH, balance patch -- this is okay */
		if ((mtmp->data == &mons[PM_SUCCUBUS]) ||
				(mtmp->data == &mons[PM_INCUBUS]))
		{
#if 0 /*JP*/
		    pline("%s is infuriated!", Monnam(mtmp));
#else
		    pline("%sは激しく怒り出した！", Monnam(mtmp));
#endif
		    mtmp->mtame = 0;
		    mtmp->mpeaceful = 0;
		}

	}
	return;
}

struct obj *
get_mleash(mtmp)	/* assuming mtmp->mleashed has been checked */
register struct monst *mtmp;
{
	register struct obj *otmp;

	otmp = invent;
	while(otmp) {
		if(otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id)
			return(otmp);
		otmp = otmp->nobj;
	}
	return((struct obj *)0);
}

#endif /* OVLB */
#ifdef OVL1

boolean
next_to_u()
{
	register struct monst *mtmp;
	register struct obj *otmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if(mtmp->mleashed) {
			if (distu(mtmp->mx,mtmp->my) > 2) mnexto(mtmp);
			if (distu(mtmp->mx,mtmp->my) > 2) {
			    for(otmp = invent; otmp; otmp = otmp->nobj)
				if(otmp->otyp == LEASH &&
					otmp->leashmon == (int)mtmp->m_id) {
				    if(otmp->cursed) return(FALSE);
#if 0 /*JP*/
				    You_feel("%s leash go slack.",
					(number_leashed() > 1) ? "a" : "the");
#else
				    You("紐がたるんだような気がした．");
#endif
				    mtmp->mleashed = 0;
				    otmp->leashmon = 0;
				}
			}
		}
	}
#ifdef STEED
	/* no pack mules for the Amulet */
	if (u.usteed && mon_has_amulet(u.usteed)) return FALSE;
#endif
	return(TRUE);
}

#endif /* OVL1 */
#ifdef OVL0

void
check_leash(x, y)
register xchar x, y;
{
	register struct obj *otmp;
	register struct monst *mtmp;

	for (otmp = invent; otmp; otmp = otmp->nobj) {
	    if (otmp->otyp != LEASH || otmp->leashmon == 0) continue;
	    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if ((int)mtmp->m_id == otmp->leashmon) break; 
	    }
	    if (!mtmp) {
		impossible("leash in use isn't attached to anything?");
		otmp->leashmon = 0;
		continue;
	    }
	    if (dist2(u.ux,u.uy,mtmp->mx,mtmp->my) >
		    dist2(x,y,mtmp->mx,mtmp->my)) {
		if (!um_dist(mtmp->mx, mtmp->my, 3)) {
		    ;	/* still close enough */
		} else if (otmp->cursed && !breathless(mtmp->data)) {
		    if (um_dist(mtmp->mx, mtmp->my, 5) ||
			    (mtmp->mhp -= rnd(2)) <= 0) {
			long save_pacifism = u.uconduct.killer;

/*JP
			Your("leash chokes %s to death!", mon_nam(mtmp));
*/
			pline("%sは紐に絞め殺された！",mon_nam(mtmp));
			/* hero might not have intended to kill pet, but
			   that's the result of his actions; gain experience,
			   lose pacifism, take alignment and luck hit, make
			   corpse less likely to remain tame after revival */
			xkilled(mtmp, 0);	/* no "you kill it" message */
			/* life-saving doesn't ordinarily reset this */
			if (mtmp->mhp > 0) u.uconduct.killer = save_pacifism;
		    } else {
/*JP
			pline("%s chokes on the leash!", Monnam(mtmp));
*/
			pline("%sは紐で首を絞められた！", Monnam(mtmp));
			/* tameness eventually drops to 1 here (never 0) */
			if (mtmp->mtame && rn2(mtmp->mtame)) mtmp->mtame--;
		    }
		} else {
		    if (um_dist(mtmp->mx, mtmp->my, 5)) {
/*JP
			pline("%s leash snaps loose!", s_suffix(Monnam(mtmp)));
*/
			pline("%sの紐はパチンと外れた！", Monnam(mtmp));
			m_unleash(mtmp, FALSE);
		    } else {
/*JP
			You("pull on the leash.");
*/
				    You("紐を引っぱった．");
			if (mtmp->data->msound != MS_SILENT)
			    switch (rn2(3)) {
			    case 0:  growl(mtmp);   break;
			    case 1:  yelp(mtmp);    break;
			    default: whimper(mtmp); break;
			    }
		    }
		}
	    }
	}
}

#endif /* OVL0 */
#ifdef OVLB

#define WEAK	3	/* from eat.c */

/*JP
static const char look_str[] = "look %s.";
*/
static const char look_str[] = "%s見える．";

STATIC_OVL int
use_mirror(obj)
struct obj *obj;
{
	register struct monst *mtmp;
	register char mlet;
#ifdef INVISIBLE_OBJECTS
	boolean vis = !Blind && (!obj->oinvis || See_invisible);
#else
	boolean vis = !Blind;
#endif

	if(!getdir((char *)0)) return 0;
	if(obj->cursed && !rn2(2)) {
		if (vis)
#if 0 /*JP*/
			pline_The("mirror fogs up and doesn't reflect!");
#else
			pline("鏡は曇り，映らなくなった！");
#endif
		return 1;
	}
	if(!u.dx && !u.dy && !u.dz) {
		if(vis && !Invisible) {
		    if (u.umonnum == PM_FLOATING_EYE) {
			if (!Free_action) {
#if 0 /*JP*/
			pline(Hallucination ?
			      "Yow!  The mirror stares back!" :
			      "Yikes!  You've frozen yourself!");
#else
			pline(Hallucination ?
			      "おゎ！鏡があなたをにらみ返した！" :
			      "おゎ！あなたは自分を麻痺させてしまった！");
#endif
			nomul(-rnd((MAXULEV+6) - u.ulevel));
			nomovemsg = 0;
#if 0 /*JP*/
			} else You("stiffen momentarily under your gaze.");
#else
			} else You("一瞬自分のにらみで硬直した．");
#endif
		    } else if (is_vampire(youmonst.data))
#if 0 /*JP*/
			You("don't have a reflection.");
#else
			You("鏡に映らない．");
#endif
		    else if (u.umonnum == PM_UMBER_HULK) {
#if 0 /*JP*/
			pline("Huh?  That doesn't look like you!");
#else
			pline("ほえ？写ってるのはあなたじゃないみたいだ！");
#endif
			make_confused(HConfusion + d(3,4),FALSE);
		    } else if (Hallucination)
#if 0 /*JP*/
			You(look_str, hcolor((char *)0));
#else
			Your("%s顔が見える．", hcolor((char *)0));
#endif
		    else if (Sick)
#if 0 /*JP*/
			You(look_str, "peaked");
#else
			You(look_str, "顔色が悪く");
#endif
		    else if (u.uhs >= WEAK)
#if 0 /*JP*/
			You(look_str, "undernourished");
#else
			You(look_str, "栄養失調のように");
#endif
#if 0 /*JP*/
		    else You("look as %s as ever.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly");
#else
		    else You("あいかわらず%s見える．",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "美しく" : "りりしく") :
				"醜く");
#endif
		} else {
#if 0 /*JP*/
			You_cant("see your %s %s.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly",
				body_part(FACE));
#else
			You("自分の%s%sを見ることができない．",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "美しい" : "りりしい") :
				"醜い",
				body_part(FACE));
#endif
		}
		return 1;
	}
	if(u.uswallow) {
#if 0 /*JP*/
		if (vis) You("reflect %s %s.", s_suffix(mon_nam(u.ustuck)),
#else
		if (vis) You("%sの%sを映した．", mon_nam(u.ustuck),
#endif
		    mbodypart(u.ustuck, STOMACH));
		return 1;
	}
	if(Underwater) {
#ifdef INVISIBLE_OBJECTS
		if (!obj->oinvis)
#endif
#if 0 /*JP*/
		You(Hallucination ?
		    "give the fish a chance to fix their makeup." :
		    "reflect the murky water.");
#else
		pline(Hallucination ?
		    "魚に化粧直しの機会を与えてやった．":
		    "あなたは陰気な水を映した．");
#endif
		return 1;
	}
	if(u.dz) {
		if (vis)
#if 0 /*JP*/
		    You("reflect the %s.",
#else
		    You("%sを映した．",
#endif
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
		return 1;
	}
	mtmp = bhit(u.dx, u.dy, COLNO, INVIS_BEAM,
		    (int FDECL((*),(MONST_P,OBJ_P)))0,
		    (int FDECL((*),(OBJ_P,OBJ_P)))0,
		    &obj);
	if (!mtmp || !haseyes(mtmp->data))
		return 1;

	vis = canseemon(mtmp);
	mlet = mtmp->data->mlet;
	if (mtmp->msleeping) {
		if (vis)
#if 0 /*JP*/
		    pline ("%s is too tired to look at your mirror.",
#else
		    pline ("%sはとても疲れていて鏡を見ることができない．",
#endif
			    Monnam(mtmp));
	} else if (!mtmp->mcansee) {
	    if (vis)
#if 0 /*JP*/
		pline("%s can't see anything right now.", Monnam(mtmp));
#else
		pline("%sは今何も見ることができない．", Monnam(mtmp));
#endif
#ifdef INVISIBLE_OBJECTS
	} else if (obj->oinvis && !perceives(mtmp->data)) {
	    if (vis)
#if 0 /*JP*/
		pline("%s can't see your mirror.", Monnam(mtmp));
#else
		pline("%sには鏡が見えていない．", Monnam(mtmp));
#endif
#endif
	/* some monsters do special things */
	} else if (is_vampire(mtmp->data) || mlet == S_GHOST) {
	    if (vis)
#if 0 /*JP*/
		pline ("%s doesn't have a reflection.", Monnam(mtmp));
#else
		pline ("%sは鏡に写らない．", Monnam(mtmp));
#endif
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_MEDUSA]) {
#if 0 /*JP*/
		if (mon_reflects(mtmp, "The gaze is reflected away by %s %s!"))
#else
		if (mon_reflects(mtmp, "にらみは%sの%sで反射した！"))
#endif
			return 1;
		if (vis)
#if 0 /*JP*/
			pline("%s is turned to stone!", Monnam(mtmp));
#else
			pline("%sは石になった！", Monnam(mtmp));
#endif
		stoned = TRUE;
		killed(mtmp);
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_FLOATING_EYE]) {
		int tmp = d((int)mtmp->m_lev, (int)mtmp->data->mattk[0].damd);
		if (!rn2(4)) tmp = 120;
		if (vis)
#if 0 /*JP*/
			pline("%s is frozen by its reflection.", Monnam(mtmp));
		else You_hear("%s stop moving.",something);
#else
			pline("%sは自分の姿を見て動けなくなった．", Monnam(mtmp));
		else You_hear("%sが動きをとめた音を聞いた．",something);
#endif
		mtmp->mcanmove = 0;
		if ( (int) mtmp->mfrozen + tmp > 127)
			mtmp->mfrozen = 127;
		else mtmp->mfrozen += tmp;
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_UMBER_HULK]) {
		if (vis)
/*JP
			pline ("%s confuses itself!", Monnam(mtmp));
*/
			pline ("%sは自分のにらみで混乱した！", Monnam(mtmp));
		mtmp->mconf = 1;
	} else if(!mtmp->mcan && !mtmp->minvis && (mlet == S_NYMPH
				     || mtmp->data==&mons[PM_SUCCUBUS])) {
		if (vis) {
/*JP
		    pline ("%s admires herself in your mirror.", Monnam(mtmp));
*/
		    pline ("%sは鏡の中の自分の姿にうっとりした．", Monnam(mtmp));
/*JP
		    pline ("She takes it!");
*/
		    pline ("彼女はそれを奪った！");
/*JP
		} else pline ("It steals your mirror!");
*/
		} else pline ("何者かがあなたの鏡を盗んだ！");
		setnotworn(obj); /* in case mirror was wielded */
		freeinv(obj);
		(void) mpickobj(mtmp,obj);
		if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
	} else if (!is_unicorn(mtmp->data) && !humanoid(mtmp->data) &&
			(!mtmp->minvis || perceives(mtmp->data)) && rn2(5)) {
		if (vis)
/*JP
		    pline("%s is frightened by its reflection.", Monnam(mtmp));
*/
		    pline ("%sは鏡に映った自分に怯えた．", Monnam(mtmp));
		monflee(mtmp, d(2,4), FALSE, FALSE);
	} else if (!Blind) {
		if (mtmp->minvis && !See_invisible)
		    ;
		else if ((mtmp->minvis && !perceives(mtmp->data))
			 || !haseyes(mtmp->data))
#if 0 /*JP*/
		    pline("%s doesn't seem to notice its reflection.",
#else
		    pline("%sは鏡に映った自分に気がついてないようだ．",
#endif
			Monnam(mtmp));
		else
#if 0 /*JP*/
		    pline("%s ignores %s reflection.",
			  Monnam(mtmp), mhis(mtmp));
#else
		    pline("%sは鏡に映った自分を無視した．",
			  Monnam(mtmp));
#endif
	}
	return 1;
}

STATIC_OVL void
use_bell(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	struct monst *mtmp;
	boolean wakem = FALSE, learno = FALSE,
		ordinary = (obj->otyp != BELL_OF_OPENING || !obj->spe),
		invoking = (obj->otyp == BELL_OF_OPENING &&
			 invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy));

/*JP
	You("ring %s.", the(xname(obj)));
*/
	You("%sを鳴らした．", the(xname(obj)));

	if (Underwater || (u.uswallow && ordinary)) {
#ifdef	AMIGA
	    amii_speaker( obj, "AhDhGqEqDhEhAqDqFhGw", AMII_MUFFLED_VOLUME );
#endif
/*JP
	    pline("But the sound is muffled.");
*/
	    pline("しかし音はかき消された．");

	} else if (invoking && ordinary) {
	    /* needs to be recharged... */
/*JP
	    pline("But it makes no sound.");
*/
	    pline("しかし，音は鳴らなかった．");
	    learno = TRUE;	/* help player figure out why */

	} else if (ordinary) {
#ifdef	AMIGA
	    amii_speaker( obj, "ahdhgqeqdhehaqdqfhgw", AMII_MUFFLED_VOLUME );
#endif
	    if (obj->cursed && !rn2(4) &&
		    /* note: once any of them are gone, we stop all of them */
		    !(mvitals[PM_WOOD_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_MOUNTAIN_NYMPH].mvflags & G_GONE) &&
		    (mtmp = makemon(mkclass(S_NYMPH, 0),
					u.ux, u.uy, NO_MINVENT)) != 0) {
/*JP
		You("summon %s!", a_monnam(mtmp));
*/
		You("%sを召喚した！", a_monnam(mtmp));
		if (!obj_resists(obj, 93, 100)) {
/*JP
		    pline("%s shattered!", Tobjnam(obj, "have"));
*/
		    pline("%sは粉々になった！", xname(obj));
		    useup(obj);
		    *optr = 0;
		} else switch (rn2(3)) {
			default:
				break;
			case 1:
				mon_adjust_speed(mtmp, 2, (struct obj *)0);
				break;
			case 2: /* no explanation; it just happens... */
				nomovemsg = "";
				nomul(-rnd(2));
				break;
		}
	    }
	    wakem = TRUE;

	} else {
	    /* charged Bell of Opening */
	    consume_obj_charge(obj, TRUE);

	    if (u.uswallow) {
		if (!obj->cursed)
		    (void) openit();
		else
		    pline(nothing_happens);

	    } else if (obj->cursed) {
		coord mm;

		mm.x = u.ux;
		mm.y = u.uy;
		mkundead(&mm, FALSE, NO_MINVENT);
		wakem = TRUE;

	    } else  if (invoking) {
#if 0 /*JP*/
		pline("%s an unsettling shrill sound...",
		      Tobjnam(obj, "issue"));
#else
	        pline("%sは不気味な鋭い音を出した．．．",
		      xname(obj));
#endif
#ifdef	AMIGA
		amii_speaker( obj, "aefeaefeaefeaefeaefe", AMII_LOUDER_VOLUME );
#endif
		obj->age = moves;
		learno = TRUE;
		wakem = TRUE;

	    } else if (obj->blessed) {
		int res = 0;

#ifdef	AMIGA
		amii_speaker( obj, "ahahahDhEhCw", AMII_SOFT_VOLUME );
#endif
		if (uchain) {
		    unpunish();
		    res = 1;
		}
		res += openit();
		switch (res) {
		  case 0:  pline(nothing_happens); break;
/*JP
		  case 1:  pline("%s opens...", Something);
*/
		  case 1:  pline("何かが開いた．．．");
			   learno = TRUE; break;
/*JP
		  default: pline("Things open around you...");
*/
		  default: pline("まわりの物が開いた．．．");
			   learno = TRUE; break;
		}

	    } else {  /* uncursed */
#ifdef	AMIGA
		amii_speaker( obj, "AeFeaeFeAefegw", AMII_OKAY_VOLUME );
#endif
		if (findit() != 0) learno = TRUE;
		else pline(nothing_happens);
	    }

	}	/* charged BofO */

	if (learno) {
	    makeknown(BELL_OF_OPENING);
	    obj->known = 1;
	}
	if (wakem) wake_nearby();
}

STATIC_OVL void
use_candelabrum(obj)
register struct obj *obj;
{
#if 0 /*JP*/
	const char *s = (obj->spe != 1) ? "candles" : "candle";
#endif

	if(Underwater) {
/*JP
		You("cannot make fire under water.");
*/
		You("水中で火はおこせない．");
		return;
	}
	if(obj->lamplit) {
/*JP
		You("snuff the %s.", s);
*/
		You("ろうそくの炎を吹き消した．");
		end_burn(obj, TRUE);
		return;
	}
	if(obj->spe <= 0) {
/*JP
		pline("This %s has no %s.", xname(obj), s);
*/
		pline("この%sにはろうそくがない．", xname(obj));
		return;
	}
	if(u.uswallow || obj->cursed) {
		if (!Blind)
#if 0 /*JP*/
		    pline_The("%s %s for a moment, then %s.",
			      s, vtense(s, "flicker"), vtense(s, "die"));
#else
		    pline("ろうそくの炎は一瞬またたき，消えた．");
#endif
		return;
	}
	if(obj->spe < 7) {
#if 0 /*JP*/
		There("%s only %d %s in %s.",
		      vtense(s, "are"), obj->spe, s, the(xname(obj)));
#else
		pline("%sにはたった%d%sのろうそくしかない．",
		       xname(obj), obj->spe, numeral(obj));
#endif
		if (!Blind)
#if 0 /*JP*/
		    pline("%s lit.  %s dimly.",
			  obj->spe == 1 ? "It is" : "They are",
			  Tobjnam(obj, "shine"));
#else
		    pline("%sに火をつけた．%sはほのかに輝いた．",
		       xname(obj), xname(obj));
#endif
	} else {
#if 0 /*JP*/
		pline("%s's %s burn%s", The(xname(obj)), s,
			(Blind ? "." : " brightly!"));
#else
		pline("%sのろうそくは%s燃えあがった%s", The(xname(obj)),
			(Blind ? "" : "明るく"), (Blind ? "．" : "！"));
#endif
	}
	if (!invocation_pos(u.ux, u.uy)) {
/*JP
		pline_The("%s %s being rapidly consumed!", s, vtense(s, "are"));
*/
		pline("ろうそくはすごい速さで燃え尽きようとしている！");
		obj->age /= 2;
	} else {
		if(obj->spe == 7) {
		    if (Blind)
/*JP
		      pline("%s a strange warmth!", Tobjnam(obj, "radiate"));
*/
		      pline("%sは奇妙な暖かさを放っている！", xname(obj));
		    else
/*JP
		      pline("%s with a strange light!", Tobjnam(obj, "glow"));
*/
		      pline("%sは奇妙な光を放っている！", xname(obj));
		}
		obj->known = 1;
	}
	begin_burn(obj, FALSE);
}

STATIC_OVL void
use_candle(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	register struct obj *otmp;
#if 0 /*JP*/
	const char *s = (obj->quan != 1) ? "candles" : "candle";
#endif
	char qbuf[QBUFSZ];

	if(u.uswallow) {
		You(no_elbow_room);
		return;
	}
	if(Underwater) {
/*JP
		pline("Sorry, fire and water don't mix.");
*/
		pline("残念ながら，火と水はまざらない．");
		return;
	}

	otmp = carrying(CANDELABRUM_OF_INVOCATION);
	/* [ALI] Artifact candles can't be attached to candelabrum
	 *       (magic candles still can be).
	 */
	if(obj->oartifact || !otmp || otmp->spe == 7) {
		use_lamp(obj);
		return;
	}

#if 0 /*JP*/
	Sprintf(qbuf, "Attach %s", the(xname(obj)));
	Sprintf(eos(qbuf), " to %s?",
		safe_qbuf(qbuf, sizeof(" to ?"), the(xname(otmp)),
			the(simple_typename(otmp->otyp)), "it"));
#else
	Sprintf(qbuf, "%sを", the(xname(obj)));
	Sprintf(eos(qbuf), "%sに取りつけますか？",
		safe_qbuf(qbuf, sizeof("に取りつけますか？"), the(xname(otmp)),
			the(simple_typename(otmp->otyp)), "それ"));
#endif
	if(yn(qbuf) == 'n') {
		if (!obj->lamplit)
/*JP
		    You("try to light %s...", the(xname(obj)));
*/
		    You("%sに火をつけようとした．．．", the(xname(obj)));
		use_lamp(obj);
		return;
	} else {
		if ((long)otmp->spe + obj->quan > 7L)
		    obj = splitobj(obj, 7L - (long)otmp->spe);
		else *optr = 0;
#if 0 /*JP*/
		You("attach %ld%s %s to %s.",
		    obj->quan, !otmp->spe ? "" : " more",
		    s, the(xname(otmp)));
#else
		You("%ld%sのろうそくを%s%sへ取りつけた．",
		    obj->quan, numeral(obj),
		    !otmp->spe ? "" : "さらに", xname(otmp));
#endif
		if (obj->otyp == MAGIC_CANDLE) {
#if 0 /*JP*/
		    if (obj->lamplit)
			pline_The("new %s %s very ordinary.", s,
				vtense(s, "look"));
		    else
			pline("%s very ordinary.",
				(obj->quan > 1L) ? "They look" : "It looks");
#else
		    pline("%sろうそくはとても平凡に見える．",
			(obj->lamplit) ? "新しい" : "");
#endif
		    if (!otmp->spe)
			otmp->age = 600L;
		} else
		if (!otmp->spe || otmp->age > obj->age)
		    otmp->age = obj->age;
		otmp->spe += (int)obj->quan;
		if (otmp->lamplit && !obj->lamplit)
#if 0 /*JP*/
		    pline_The("new %s magically %s!", s, vtense(s, "ignite"));
#else
		    pline("新しいろうそくは魔法のごとく灯った！");
#endif
		else if (!otmp->lamplit && obj->lamplit)
#if 0 /*JP*/
		    pline("%s out.", (obj->quan > 1L) ? "They go" : "It goes");
#else
		    pline("炎は消えた．");
#endif
		if (obj->unpaid)
#if 0 /*JP*/
		    verbalize("You %s %s, you bought %s!",
			      otmp->lamplit ? "burn" : "use",
			      (obj->quan > 1L) ? "them" : "it",
			      (obj->quan > 1L) ? "them" : "it");
#else
		    verbalize("%sたなら，買ってもらおう！",
			otmp->lamplit ? "燃やし" : "使っ");
#endif
		if (obj->quan < 7L && otmp->spe == 7)
#if 0 /*JP*/
		    pline("%s now has seven%s candles attached.",
			  The(xname(otmp)), otmp->lamplit ? " lit" : "");
#else
		    pline("%sにはすでに7本の%sろうそくが取りつけられている．",
			  The(xname(otmp)), otmp->lamplit ? "火のついた" : "");
#endif
		/* candelabrum's light range might increase */
		if (otmp->lamplit) obj_merge_light_sources(otmp, otmp);
		/* candles are no longer a separate light source */
		if (obj->lamplit) end_burn(obj, TRUE);
		/* candles are now gone */
		useupall(obj);
	}
}

boolean
snuff_candle(otmp)  /* call in drop, throw, and put in box, etc. */
register struct obj *otmp;
{
	register boolean candle = Is_candle(otmp);

	if (((candle && otmp->oartifact != ART_CANDLE_OF_ETERNAL_FLAME)
		|| otmp->otyp == CANDELABRUM_OF_INVOCATION) &&
		otmp->lamplit) {
	    char buf[BUFSZ];
	    xchar x, y;
#if 0 /*JP*/
	    register boolean many = candle ? otmp->quan > 1L : otmp->spe > 1;
#endif

	    (void) get_obj_location(otmp, &x, &y, 0);
	    if (otmp->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
#if 0 /*JP*/
		pline("%s %scandle%s flame%s extinguished.",
		      Shk_Your(buf, otmp),
		      (candle ? "" : "candelabrum's "),
		      (many ? "s'" : "'s"), (many ? "s are" : " is"));
#else
		pline("%s%sろうそくの炎は消えた．",
		      Shk_Your(buf, otmp),
		      candle ? "" : "燭台の");
#endif
	   end_burn(otmp, TRUE);
	   return(TRUE);
	}
	return(FALSE);
}

/* called when lit lamp is hit by water or put into a container or
   you've been swallowed by a monster; obj might be in transit while
   being thrown or dropped so don't assume that its location is valid */
boolean
snuff_lit(obj)
struct obj *obj;
{
	xchar x, y;

	if (obj->lamplit) {
	    if (artifact_light(obj)) return FALSE; /* Artifact lights are never snuffed */
	    if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		obj->otyp == BRASS_LANTERN || obj->otyp == POT_OIL ||
		obj->otyp == TORCH) {
		(void) get_obj_location(obj, &x, &y, 0);
		if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
/*JP
		    pline("%s %s out!", Yname2(obj), otense(obj, "go"));
*/
		    pline("%sの灯りは消えた！", Yname2(obj));
		end_burn(obj, TRUE);
		return TRUE;
	    }
	    if (snuff_candle(obj)) return TRUE;
	}
	return FALSE;
}

/* Called when potentially lightable object is affected by fire_damage().
   Return TRUE if object was lit and FALSE otherwise --ALI */
boolean
catch_lit(obj)
struct obj *obj;
{
	xchar x, y;

	if (!obj->lamplit && (obj->otyp == MAGIC_LAMP || ignitable(obj))) {
	    if ((obj->otyp == MAGIC_LAMP ||
		 obj->otyp == CANDELABRUM_OF_INVOCATION) &&
		obj->spe == 0)
		return FALSE;
	    else if (obj->otyp != MAGIC_LAMP && obj->age == 0)
		return FALSE;
	    if (!get_obj_location(obj, &x, &y, 0))
		return FALSE;
	    if (obj->otyp == CANDELABRUM_OF_INVOCATION && obj->cursed)
		return FALSE;
	    if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		 obj->otyp == BRASS_LANTERN) && obj->cursed && !rn2(2))
		return FALSE;
	    if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
/*JP
		pline("%s %s light!", Yname2(obj), otense(obj, "catch"));
*/
		pline("%sの灯りがついた！", Yname2(obj));
	    if (obj->otyp == POT_OIL) makeknown(obj->otyp);
	    if (obj->unpaid && costly_spot(u.ux, u.uy) && (obj->where == OBJ_INVENT)) {
	        /* if it catches while you have it, then it's your tough luck */
		check_unpaid(obj);
#if 0 /*JP:T*/
	        verbalize("That's in addition to the cost of %s %s, of course.",
				Yname2(obj), obj->quan == 1 ? "itself" : "themselves");
#else
	        verbalize("もちろん，その分は%sの値段に上乗せさてもらうよ．", Yname2(obj));
#endif
		bill_dummy_object(obj);
	    }
	    begin_burn(obj, FALSE);
	    return TRUE;
	}
	return FALSE;
}

STATIC_OVL void
use_lamp(obj)
struct obj *obj;
{
	char buf[BUFSZ];
	char qbuf[QBUFSZ];

	if(Underwater) {
#if 0 /*JP*/
		pline("This is not a diving lamp.");
#else
		pline("これは潜水用のランプじゃない．");
#endif
		return;
	}
	if(obj->lamplit) {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN) {
#if 0 /*JP*/
		    pline("%s lamp is now off.", Shk_Your(buf, obj));
#else
		    pline("%sランプの灯は消えた．", Shk_Your(buf, obj));
#endif
#ifdef LIGHTSABERS
		} else if(is_lightsaber(obj)) {
		    if (obj->otyp == RED_DOUBLE_LIGHTSABER) {
			/* Do we want to activate dual bladed mode? */
			if (!obj->altmode && (!obj->cursed || rn2(4))) {
#if 0 /*JP*/
			    You("ignite the second blade of %s.", yname(obj));
#else
			    pline("%sの二つ目の刃を始動させた．", yname(obj));
#endif
			    obj->altmode = TRUE;
			    return;
			} else obj->altmode = FALSE;
		    }
		    lightsaber_deactivate(obj, TRUE);
		    return;
#endif
		} else if (artifact_light(obj)) {
#if 0 /*JP*/
		    You_cant("snuff out %s.", yname(obj));
#else
		    pline("%sを吹き消すことができない．", yname(obj));
#endif
		    return;
		} else {
#if 0 /*JP*/
		    You("snuff out %s.", yname(obj));
#else
		    pline("%sを吹き消した．", yname(obj));
#endif
		}
		end_burn(obj, TRUE);
		return;
	}
	/* magic lamps with an spe == 0 (wished for) cannot be lit */
	if ((!Is_candle(obj) && obj->age == 0)
			|| (obj->otyp == MAGIC_LAMP && obj->spe == 0)) {
		if ((obj->otyp == BRASS_LANTERN)
#ifdef LIGHTSABERS
			|| is_lightsaber(obj)
#endif
			)
#if 0 /*JP*/
			Your("%s has run out of power.", xname(obj));
#else
			Your("%sは電池が切れている．", xname(obj));
#endif
		else if (obj->otyp == TORCH) {
#if 0 /*JP*/
		        Your("torch has burnt out and cannot be relit.");
#else
		        Your("トーチは燃え尽きてしまってもう光らない．");
#endif
		}
#if 0 /*JP*/
		else pline("This %s has no oil.", xname(obj));
#else
		else pline("この%sは油を切らしている．", xname(obj));
#endif
		return;
	}
	if (obj->cursed && !rn2(2)) {
#if 0 /*JP*/
		pline("%s for a moment, then %s.",
		      Tobjnam(obj, "flicker"), otense(obj, "die"));
#else
		pline("%sの炎は一瞬またたき，消えた．", xname(obj));
#endif
	} else {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN) {
		    check_unpaid(obj);
#if 0 /*JP*/
		    pline("%s lamp is now on.", Shk_Your(buf, obj));
#else
		    pline("%sランプが灯った．", Shk_Your(buf, obj));
#endif
		} else if (obj->otyp == TORCH) {
		    check_unpaid(obj);
#if 0 /*JP*/
		    pline("%s flame%s burn%s%s",
			s_suffix(Yname2(obj)),
			plur(obj->quan),
			obj->quan > 1L ? "" : "s",
			Blind ? "." : " brightly!");
#else
		    pline("%s%s", xname(obj),
			Blind ? "に火をつけた．" : "の炎が明るく燃え上がった！");
#endif
#ifdef LIGHTSABERS
		} else if (is_lightsaber(obj)) {
		    /* WAC -- lightsabers */
		    /* you can see the color of the blade */
		    
		    if (!Blind) makeknown(obj->otyp);
#if 0 /*JP*/
		    You("ignite %s.", yname(obj));
#else
		    You("%sを始動させた．", yname(obj));
#endif
		    unweapon = FALSE;
#endif
		} else {	/* candle(s) */
#if 0 /*JP*/
		    Sprintf(qbuf, "Light all of %s?", the(xname(obj)));
#else
		    Sprintf(qbuf, "%sの全てに火をつけますか？", the(xname(obj)));
#endif
		    if (obj->quan > 1L && (yn(qbuf) == 'n')) {
			/* Check if player wants to light all the candles */
			struct obj *rest;	     /* the remaining candles */
			rest = splitobj(obj, obj->quan - 1L);
			obj_extract_self(rest);	     /* free from inv */
			obj->spe++;	/* this prevents merging */
#if 0 /*JP*/
			(void)hold_another_object(rest, "You drop %s!",
					  doname(rest), (const char *)0);
#else
			(void)hold_another_object(rest, "あなたは%sを落した！",
					  doname(rest), (const char *)0);
#endif
			obj->spe--;
		    }
#if 0 /*JP*/
		    pline("%s flame%s %s%s",
			s_suffix(Yname2(obj)),
			plur(obj->quan), otense(obj, "burn"),
			Blind ? "." : " brightly!");
#else
		    pline("%sは%s燃えあがった%s", Yname2(obj),
		    	(Blind ? "" : "明るく"), (Blind ? "．" : "！"));
#endif
		    if (obj->unpaid && costly_spot(u.ux, u.uy) &&
			  obj->otyp != MAGIC_CANDLE) {
#if 0 /*JP*/
			const char *ithem = obj->quan > 1L ? "them" : "it";
			verbalize("You burn %s, you bought %s!", ithem, ithem);
#else
			verbalize("灯をつけたなら，買ってもらおう！");
#endif
			bill_dummy_object(obj);
		    }
		}
		begin_burn(obj, FALSE);
	}
}

/* MRKR: Torches */

STATIC_OVL int
use_torch(obj)
struct obj *obj;
{
    struct obj *otmp = NULL;
    if (u.uswallow) {
	You(no_elbow_room);
	return 0;
    }
    if (Underwater) {
/*JP
	pline("Sorry, fire and water don't mix.");
*/
	pline("残念ながら，火と水はまざらない．");
	return 0;
    }
    /* You can use a torch in either wielded weapon slot */
    if (obj != uwep && (obj != uswapwep || !u.twoweap))
	if (!wield_tool(obj, (const char *)0)) return 0;

    if (obj->quan > 1L) {
	otmp = splitobj(obj, obj->quan - 1L);
	obj_extract_self(otmp);	/* free from inv */

    /* shouldn't merge */
	otmp->age += 20;	/* this prevents merging */
#if 0 /*JP*/
	otmp = hold_another_object(otmp, "You drop %s!",
#else
	otmp = hold_another_object(otmp, "あなたは%sを落した！",
#endif
				   doname(otmp), (const char *)0);
	otmp->age -= 20;
    }

    use_lamp(obj);

    return 1;
}

STATIC_OVL void
light_cocktail(obj)
	struct obj *obj;        /* obj is a potion of oil or a stick of dynamite */
{
	char buf[BUFSZ];
	const char *objnam =
#if 0 /*JP*/
#ifdef FIREARMS
	    obj->otyp == POT_OIL ? "potion" : "stick";
#else
	    "potion";
#endif
#else
#ifdef FIREARMS
	    obj->otyp == POT_OIL ? "油瓶" : "爆弾";
#else
	    "油瓶";
#endif
#endif

	if (u.uswallow) {
	    You(no_elbow_room);
	    return;
	}

	if(Underwater) {
#if 0 /*JP*/
		You("can't light this underwater!");
#else
		pline("水の中じゃ火が点かない！");
#endif
		return;
	}

	if (obj->lamplit) {
/*JP
	    You("snuff the lit %s.", objnam);
*/
	    You("%sの火を吹き消した．", objnam);
	    end_burn(obj, TRUE);
	    /*
	     * Free & add to re-merge potion.  This will average the
	     * age of the potions.  Not exactly the best solution,
	     * but its easy.
	     */
	    freeinv(obj);
	    (void) addinv(obj);
	    return;
	} else if (Underwater) {
/*JP
	    There("is not enough oxygen to sustain a fire.");
*/
	    pline("火をつけるのに十分な酸素がない．");
	    return;
	}

#if 0 /*JP*/
	You("light %s %s.%s", shk_your(buf, obj), objnam,
	    Blind ? "" : "  It gives off a dim light.");
#else
	You("%s%sに火をつけた．%s", shk_your(buf, obj), objnam, 
	    Blind ? "" : "それは暗い光をはなった．");
#endif
	if (obj->unpaid && costly_spot(u.ux, u.uy)) {
	    /* Normally, we shouldn't both partially and fully charge
	     * for an item, but (Yendorian Fuel) Taxes are inevitable...
	     */
#ifdef FIREARMS
	    if (obj->otyp != STICK_OF_DYNAMITE) {
#endif
	    check_unpaid(obj);
/*JP
	    verbalize("That's in addition to the cost of the potion, of course.");
*/
	    verbalize("もちろん，燃やした分は値段に上乗せさせてもらうよ．");
#ifdef FIREARMS
	    } else {
#if 0 /*JP*/
		const char *ithem = obj->quan > 1L ? "them" : "it";
		verbalize("You burn %s, you bought %s!", ithem, ithem);
#else
		verbalize("灯をつけたなら，買ってもらおう！");
#endif
	    }
#endif
	    bill_dummy_object(obj);
	}
	makeknown(obj->otyp);
#ifdef FIREARMS
	if (obj->otyp == STICK_OF_DYNAMITE) obj->yours=TRUE;
#endif

	if (obj->quan > 1L) {
	    obj = splitobj(obj, 1L);
	    begin_burn(obj, FALSE);	/* burn before free to get position */
	    obj_extract_self(obj);	/* free from inv */

	    /* shouldn't merge */
#if 0 /*JP*/
	    obj = hold_another_object(obj, "You drop %s!",
#else
	    obj = hold_another_object(obj, "あなたは%sを落した！",
#endif
				      doname(obj), (const char *)0);
	} else
	    begin_burn(obj, FALSE);
}

static NEARDATA const char cuddly[] = { TOOL_CLASS, GEM_CLASS, 0 };

int
dorub()
{
	struct obj *obj = getobj(cuddly, "rub");

	if (obj && obj->oclass == GEM_CLASS) {
	    if (is_graystone(obj)) {
		use_stone(obj);
		return 1;
	    } else {
/*JP
		pline("Sorry, I don't know how to use that.");
*/
		pline("残念．使い方がわからない．");
		return 0;
	    }
	}

	if (!obj || !wield_tool(obj, "rub")) return 0;

	/* now uwep is obj */
	if (uwep->otyp == MAGIC_LAMP) {
	    if (uwep->spe > 0 && !rn2(3)) {
		check_unpaid_usage(uwep, TRUE);		/* unusual item use */
		djinni_from_bottle(uwep);
		makeknown(MAGIC_LAMP);
		uwep->otyp = OIL_LAMP;
		uwep->spe = 0; /* for safety */
		uwep->age = rn1(500,1000);
		if (uwep->lamplit) begin_burn(uwep, TRUE);
		update_inventory();
	    } else if (rn2(2) && !Blind)
/*JP
		You("see a puff of smoke.");
*/
		pline("けむりが舞いあがった．");
	    else pline(nothing_happens);
	} else if (obj->otyp == BRASS_LANTERN) {
	    /* message from Adventure */
/*JP
	    pline("Rubbing the electric lamp is not particularly rewarding.");
*/
	    pline("電気ランプをこすっても意味はないと思うが．．．");
/*JP
	    pline("Anyway, nothing exciting happens.");
*/
	    pline("やっぱり，何も面白いことは起きなかった．");
	} else pline(nothing_happens);
	return 1;
}

int
dojump()
{
	/* Physical jump */
	return jump(0);
}

int
jump(magic)
int magic; /* 0=Physical, otherwise skill level */
{
	coord cc;

	if (!magic && (nolimbs(youmonst.data) || slithy(youmonst.data))) {
		/* normally (nolimbs || slithy) implies !Jumping,
		   but that isn't necessarily the case for knights */
#if 0 /*JP*/
		You_cant("jump; you have no legs!");
#else
		pline("跳べない．あなたには脚がない！");
#endif
		return 0;
	} else if (!magic && !Jumping) {
#if 0 /*JP*/
		You_cant("jump very far.");
#else
		You_cant("そんな遠くまで跳べない．");
#endif
		return 0;
	} else if (u.uswallow) {
		if (magic) {
#if 0 /*JP*/
			You("bounce around a little.");
#else
			You("少し跳ね返りまくった．");
#endif
			return 1;
		} else {
#if 0 /*JP*/
		pline("You've got to be kidding!");
#else
			pline("冗談はよしこさん！");
#endif
		return 0;
		}
		return 0;
	} else if (u.uinwater) {
		if (magic) {
#if 0 /*JP*/
			You("swish around a little.");
#else
			You("その場で音を立てて少し回転した．");
#endif
			return 1;
		} else {
#if 0 /*JP*/
		pline("This calls for swimming, not jumping!");
#else
			pline("それは『泳ぐ』であって，『跳ぶ』じゃない！");
#endif
		return 0;
		}
		return 0;
	} else if (u.ustuck) {
		if (u.ustuck->mtame && !Conflict && !u.ustuck->mconf) {
#if 0 /*JP*/
		    You("pull free from %s.", mon_nam(u.ustuck));
#else
		    You("%sから離れた．", mon_nam(u.ustuck));
#endif
		    setustuck(0);
		    return 1;
		}
		if (magic) {
#if 0 /*JP*/
			You("writhe a little in the grasp of %s!", mon_nam(u.ustuck));
#else
			You("%sから逃れようとジタバタした！", mon_nam(u.ustuck));
#endif
			return 1;
		} else {
#if 0 /*JP*/
		You("cannot escape from %s!", mon_nam(u.ustuck));
#else
			You("%sから逃れられない！", mon_nam(u.ustuck));
#endif
		return 0;
		}

		return 0;
	} else if (Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
		if (magic) {
#if 0 /*JP*/
			You("flail around a little.");
#else
			You("その場で揺れ動いた．");
#endif
			return 1;
		} else {
#if 0 /*JP*/
		You("don't have enough traction to jump.");
#else
			You("跳ぶための反動がつけられない．");
#endif
		return 0;
		}
	} else if (!magic && near_capacity() > UNENCUMBERED) {
#if 0 /*JP*/
		You("are carrying too much to jump!");
#else
		You("物を持ちすぎており跳ぶことができない！");
#endif
		return 0;
	} else if (!magic && (u.uhunger <= 100 || ACURR(A_STR) < 6)) {
#if 0 /*JP*/
		You("lack the strength to jump!");
#else
		pline("あなたには跳ぶだけの力がない！");
#endif
		return 0;
	} else if (Wounded_legs) {
 		long wl = (EWounded_legs & BOTH_SIDES);
		const char *bp = body_part(LEG);

		if (wl == BOTH_SIDES) bp = makeplural(bp);
#ifdef STEED
		if (u.usteed)
/*JP
		    pline("%s is in no shape for jumping.", Monnam(u.usteed));
*/
		    pline("%sは跳べる状態ではない．", Monnam(u.usteed));
		else
#endif
#if 0 /*JP*/
		Your("%s%s %s in no shape for jumping.",
		     (wl == LEFT_SIDE) ? "left " :
			(wl == RIGHT_SIDE) ? "right " : "",
		     bp, (wl == BOTH_SIDES) ? "are" : "is");
#else
		Your("%s%sは跳べる状態ではない．",
		     (wl == LEFT_SIDE) ? "左" :
		     (wl == RIGHT_SIDE) ? "右" : "両", bp);
#endif
		return 0;
	}
#ifdef STEED
	else if (u.usteed && u.utrap) {
/*JP
		pline("%s is stuck in a trap.", Monnam(u.usteed));
*/
		pline("%sは罠にひっかかっている．", Monnam(u.usteed));
		return (0);
	}
#endif

/*JP
	pline("Where do you want to jump?");
*/
	pline("どこに跳びますか？");
	cc.x = u.ux;
	cc.y = u.uy;
/*JP
	if (getpos(&cc, TRUE, "the desired position") < 0)
*/
	if (getpos(&cc, TRUE, "跳びたい場所") < 0)
		return 0;	/* user pressed ESC */
	if (!magic && !(HJumping & ~INTRINSIC) && !EJumping &&
			distu(cc.x, cc.y) != 5) {
		/* The Knight jumping restriction still applies when riding a
		 * horse.  After all, what shape is the knight piece in chess?
		 */
/*JP
		pline("Illegal move!");
*/
		pline("その移動は桂馬跳びじゃない！");
		return 0;
	} else if (distu(cc.x, cc.y) > (magic ? 6+magic*3 : 9)) {
/*JP
		pline("Too far!");
*/
		pline("遠すぎる！");
		return 0;
	} else if (!cansee(cc.x, cc.y)) {
/*JP
		You("cannot see where to land!");
*/
		pline("あなたには着地点が見えない！");
		return 0;
	} else if (!isok(cc.x, cc.y)) {
/*JP
		You("cannot jump there!");
*/
		You("そこには跳べない！");
		return 0;
	} else {
	    coord uc;
	    int range, temp;

	    if(u.utrap)
		switch(u.utraptype) {
		case TT_BEARTRAP: {
		    register long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
/*JP
		    You("rip yourself free of the bear trap!  Ouch!");
*/
		    You("自分を熊の罠からひきはがした！ いてっ！");
#ifdef STEED
			if (!u.usteed)
#endif
/*JP
		    losehp(rnd(10), "jumping out of a bear trap", KILLED_BY);
*/
		    losehp(rnd(10), "熊の罠から跳び出ようとして", KILLED_BY);
		    set_wounded_legs(side, rn1(1000,500));
		    break;
		  }
		case TT_PIT:
/*JP
		    You("leap from the pit!");
*/
		    You("落し穴から跳び出た！");
		    break;
		case TT_WEB:
/*JP
		    You("tear the web apart as you pull yourself free!");
*/
		    You("くもの巣を引き裂き，自由になった！");
		    deltrap(t_at(u.ux,u.uy));
		    break;
		case TT_LAVA:
/*JP
		    You("pull yourself above the lava!");
*/
		    You("身体を溶岩から引き抜いた！");
		    u.utrap = 0;
		    return 1;
		case TT_INFLOOR:
/*JP
		    You("strain your %s, but you're still stuck in the floor.",
*/
		    You("%sを引っぱってみたが，まだ床に埋まったままだ．",
			makeplural(body_part(LEG)));
		    set_wounded_legs(LEFT_SIDE, rn1(10, 11));
		    set_wounded_legs(RIGHT_SIDE, rn1(10, 11));
		    return 1;
		}

	    /*
	     * Check the path from uc to cc, calling hurtle_step at each
	     * location.  The final position actually reached will be
	     * in cc.
	     */
	    uc.x = u.ux;
	    uc.y = u.uy;
	    /* calculate max(abs(dx), abs(dy)) as the range */
	    range = cc.x - uc.x;
	    if (range < 0) range = -range;
	    temp = cc.y - uc.y;
	    if (temp < 0) temp = -temp;
	    if (range < temp)
		range = temp;
	    (void) walk_path(&uc, &cc, hurtle_step, (genericptr_t)&range);

	    /* A little Sokoban guilt... */
	    if (In_sokoban(&u.uz))
		change_luck(-1);

	    teleds(cc.x, cc.y, TRUE);
	    nomul(-1);
	    nomovemsg = "";
	    morehungry(rnd(25));
	    return 1;
	}
}

boolean
tinnable(corpse)
struct obj *corpse;
{
	if (corpse->otyp != CORPSE) return 0;
	if (corpse->oeaten) return 0;
	if (corpse->odrained) return 0;
	if (!mons[corpse->corpsenm].cnutrit) return 0;
	return 1;
}

STATIC_OVL void
use_tinning_kit(obj)
register struct obj *obj;
{
	register struct obj *corpse, *can;
/*
	char *badmove;
 */
	/* This takes only 1 move.  If this is to be changed to take many
	 * moves, we've got to deal with decaying corpses...
	 */
	if (obj->spe <= 0) {
#if 0 /*JP*/
		You("seem to be out of tins.");
#else
		pline("缶詰を作るための缶が切れたようだ．");
#endif
		return;
	}
	if (!(corpse = getobj((const char *)tinnables, "tin"))) return;
	if (corpse->otyp == CORPSE && (corpse->oeaten || corpse->odrained)) {
#if 0 /*JP*/
		You("cannot tin %s which is partly eaten.",something);
#else
		You("食べかけのものを缶詰にすることはできない．");
#endif
		return;
	}
	if (!tinnable(corpse)) {
#if 0 /*JP*/
		You_cant("tin that!");
#else
		You("それは缶詰にすることはできない！");
#endif
		return;
	}
	if (touch_petrifies(&mons[corpse->corpsenm])
		&& !Stone_resistance && !uarmg) {
	    char kbuf[BUFSZ];

	    if (poly_when_stoned(youmonst.data))
#if 0 /*JP*/
		You("tin %s without wearing gloves.",
			an(mons[corpse->corpsenm].mname));
#else
		You("小手なしで%sを缶詰にしようとした．",
			jtrns_mon(mons[corpse->corpsenm].mname));
#endif
	    else {
#if 0 /*JP*/
		pline("Tinning %s without wearing gloves is a fatal mistake...",
			an(mons[corpse->corpsenm].mname));
#else
		pline("%sを小手なしで缶詰にするのは致命的な間違いだ．．．",
			jtrns_mon(mons[corpse->corpsenm].mname));
#endif
#if 0 /*JP*/
		Sprintf(kbuf, "trying to tin %s without gloves",
			an(mons[corpse->corpsenm].mname));
#else
		Sprintf(kbuf, "小手をつけずに%sを缶詰にしようとして",
			jtrns_mon(mons[corpse->corpsenm].mname));
#endif
	    }
	    instapetrify(kbuf);
	}
	if (is_rider(&mons[corpse->corpsenm])) {
		(void) revive_corpse(corpse, FALSE);
#if 0 /*JP*/
		verbalize("Yes...  But War does not preserve its enemies...");
#else
		verbalize("そうだ．．．しかし「戦争」は敵に安らぎを与えぬ．．．");
#endif
		return;
	}
	if (mons[corpse->corpsenm].cnutrit == 0) {
#if 0 /*JP*/
		pline("That's too insubstantial to tin.");
#else
		pline("実体がないので缶詰にできない．");
#endif
		return;
	}
	consume_obj_charge(obj, TRUE);

	if ((can = mksobj(TIN, FALSE, FALSE)) != 0) {
#if 0 /*JP*/
	    static const char you_buy_it[] = "You tin it, you bought it!";
#else
	    static const char you_buy_it[] = "缶詰にしたのなら買ってもらうよ！";
#endif

	    can->corpsenm = corpse->corpsenm;
	    can->cursed = obj->cursed;
	    can->blessed = obj->blessed;
	    can->owt = weight(can);
	    can->known = 1;
#ifdef EATEN_MEMORY
	    /* WAC You know the type of tinned corpses */
	    if (mvitals[corpse->corpsenm].eaten < 255) 
	    	mvitals[corpse->corpsenm].eaten++;
#endif    
	    can->spe = -1;  /* Mark tinned tins. No spinach allowed... */
	    if (carried(corpse)) {
		if (corpse->unpaid)
		    verbalize(you_buy_it);
		useup(corpse);
	    } else if (mcarried(corpse)) {
		m_useup(corpse->ocarry, corpse);
	    } else {
		if (costly_spot(corpse->ox, corpse->oy) && !corpse->no_charge)
		    verbalize(you_buy_it);
		useupf(corpse, 1L);
	    }
/*JP
	    can = hold_another_object(can, "You make, but cannot pick up, %s.",
*/
	    can = hold_another_object(can, "缶詰にできたが，%sを持つことができない．",
				      doname(can), (const char *)0);
	} else impossible("Tinning failed.");
}


void
use_unicorn_horn(obj)
struct obj *obj;
{
#define PROP_COUNT 6		/* number of properties we're dealing with */
#define ATTR_COUNT (A_MAX*3)	/* number of attribute points we might fix */
	int idx, val, val_limit,
	    trouble_count, unfixable_trbl, did_prop, did_attr;
	int trouble_list[PROP_COUNT + ATTR_COUNT];
	int chance;	/* KMH */

	if (obj && obj->cursed) {
	    long lcount = (long) rnd(100);

	    switch (rn2(6)) {
	    case 0: make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON),20),
			xname(obj), TRUE, SICK_NONVOMITABLE);
		    break;
	    case 1: make_blinded(Blinded + lcount, TRUE);
		    break;
	    case 2: if (!Confusion)
#if 0 /*JP*/
			You("suddenly feel %s.",
			    Hallucination ? "trippy" : "confused");
#else
			You("突然%s．",
			    Hallucination ? "へろへろになった" : "混乱した");
#endif
		    make_confused(HConfusion + lcount, TRUE);
		    break;
	    case 3: make_stunned(HStun + lcount, TRUE);
		    break;
	    case 4: (void) adjattrib(rn2(A_MAX), -1, FALSE);
		    break;
	    case 5: (void) make_hallucinated(HHallucination + lcount, TRUE, 0L);
		    break;
	    }
	    return;
	}

/*
 * Entries in the trouble list use a very simple encoding scheme.
 */
#define prop2trbl(X)	((X) + A_MAX)
#define attr2trbl(Y)	(Y)
#define prop_trouble(X) trouble_list[trouble_count++] = prop2trbl(X)
#define attr_trouble(Y) trouble_list[trouble_count++] = attr2trbl(Y)

	trouble_count = unfixable_trbl = did_prop = did_attr = 0;

	/* collect property troubles */
	if (Sick) prop_trouble(SICK);
	if (Blinded > (long)u.ucreamed) prop_trouble(BLINDED);
	if (HHallucination) prop_trouble(HALLUC);
	if (Vomiting) prop_trouble(VOMITING);
	if (HConfusion) prop_trouble(CONFUSION);
	if (HStun) prop_trouble(STUNNED);

	unfixable_trbl = unfixable_trouble_count(TRUE);

	/* collect attribute troubles */
	for (idx = 0; idx < A_MAX; idx++) {
	    val_limit = AMAX(idx);
	    /* don't recover strength lost from hunger */
	    if (idx == A_STR && u.uhs >= WEAK) val_limit--;
	    /* don't recover more than 3 points worth of any attribute */
	    if (val_limit > ABASE(idx) + 3) val_limit = ABASE(idx) + 3;

	    for (val = ABASE(idx); val < val_limit; val++)
		attr_trouble(idx);
	    /* keep track of unfixed trouble, for message adjustment below */
	    unfixable_trbl += (AMAX(idx) - val_limit);
	}

	if (trouble_count == 0) {
	    pline(nothing_happens);
	    return;
	} else if (trouble_count > 1) {		/* shuffle */
	    int i, j, k;

	    for (i = trouble_count - 1; i > 0; i--)
		if ((j = rn2(i + 1)) != i) {
		    k = trouble_list[j];
		    trouble_list[j] = trouble_list[i];
		    trouble_list[i] = k;
		}
	}

#if 0	/* Old NetHack success rate */
	/*
	 *		Chances for number of troubles to be fixed
	 *		 0	1      2      3      4	    5	   6	  7
	 *   blessed:  22.7%  22.7%  19.5%  15.4%  10.7%   5.7%   2.6%	 0.8%
	 *  uncursed:  35.4%  35.4%  22.9%   6.3%    0	    0	   0	  0
	 */
	val_limit = rn2( d(2, (obj && obj->blessed) ? 4 : 2) );
	if (val_limit > trouble_count) val_limit = trouble_count;
#else	/* KMH's new success rate */
	/*
	 * blessed:  Tries all problems, each with chance given below.
	 * uncursed: Tries one problem, with chance given below.
	 * ENCHANT  +0 or less  +1   +2   +3   +4   +5   +6 or more
	 * CHANCE       30%     40%  50%  60%  70%  80%     90%
	 */
	val_limit = (obj && obj->blessed) ? trouble_count : 1;
	if (obj && obj->spe > 0)
		chance = (obj->spe < 6) ? obj->spe+3 : 9;
	else
		chance = 3;
#endif

	/* fix [some of] the troubles */
	for (val = 0; val < val_limit; val++) {
	    idx = trouble_list[val];

		if (rn2(10) < chance)	/* KMH */
	    switch (idx) {
	    case prop2trbl(SICK):
		make_sick(0L, (char *) 0, TRUE, SICK_ALL);
		did_prop++;
		break;
	    case prop2trbl(BLINDED):
		make_blinded((long)u.ucreamed, TRUE);
		did_prop++;
		break;
	    case prop2trbl(HALLUC):
		(void) make_hallucinated(0L, TRUE, 0L);
		did_prop++;
		break;
	    case prop2trbl(VOMITING):
		make_vomiting(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(CONFUSION):
		make_confused(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(STUNNED):
		make_stunned(0L, TRUE);
		did_prop++;
		break;
	    default:
		if (idx >= 0 && idx < A_MAX) {
		    ABASE(idx) += 1;
		    did_attr++;
		} else
		    panic("use_unicorn_horn: bad trouble? (%d)", idx);
		break;
	    }
	}

	if (did_attr)
#if 0 /*JP*/
	    pline("This makes you feel %s!",
		  (did_prop + did_attr) == (trouble_count + unfixable_trbl) ?
		  "great" : "better");
#else
	    pline("気分が%sよくなった！",
		  (did_prop + did_attr) == (trouble_count + unfixable_trbl) ?
		  "とても" : "より");
#endif
	else if (!did_prop)
/*JP
	    pline("Nothing seems to happen.");
*/
	    pline("何も起きなかったようだ．");

	flags.botl = (did_attr || did_prop);
#undef PROP_COUNT
#undef ATTR_COUNT
#undef prop2trbl
#undef attr2trbl
#undef prop_trouble
#undef attr_trouble
}

/*
 * Timer callback routine: turn figurine into monster
 */
void
fig_transform(arg, timeout)
genericptr_t arg;
long timeout;
{
	struct obj *figurine = (struct obj *)arg;
	struct monst *mtmp;
	coord cc;
	boolean cansee_spot, silent, okay_spot;
	boolean redraw = FALSE;
	char monnambuf[BUFSZ], carriedby[BUFSZ];

	if (!figurine) {
#ifdef DEBUG
	    pline("null figurine in fig_transform()");
#endif
	    return;
	}
	silent = (timeout != monstermoves); /* happened while away */
	okay_spot = get_obj_location(figurine, &cc.x, &cc.y, 0);
	if (figurine->where == OBJ_INVENT ||
	    figurine->where == OBJ_MINVENT)
		okay_spot = enexto(&cc, cc.x, cc.y,
				   &mons[figurine->corpsenm]);
	if (!okay_spot ||
	    !figurine_location_checks(figurine,&cc, TRUE)) {
		/* reset the timer to try again later */
		(void) start_timer((long)rnd(5000), TIMER_OBJECT,
				FIG_TRANSFORM, (genericptr_t)figurine);
		return;
	}

	cansee_spot = cansee(cc.x, cc.y);
	mtmp = make_familiar(figurine, cc.x, cc.y, TRUE);
	if (mtmp) {
	    Sprintf(monnambuf, "%s",an(m_monnam(mtmp)));
	    switch (figurine->where) {
		case OBJ_INVENT:
		    if (Blind)
#if 0 /*JP*/
			You_feel("%s %s from your pack!", something,
			    locomotion(mtmp->data,"drop"));
#else
			You_feel("%sがあなたの鞄から%s出るのを感じた！", something,
			    locomotion(mtmp->data, "転げ"));
#endif
		    else
#if 0 /*JP*/
			You("see %s %s out of your pack!",
			    monnambuf,
			    locomotion(mtmp->data,"drop"));
#else
			You("%sがあなたの鞄から%s出た！",
			    monnambuf,
			    locomotion(mtmp->data, "転げ"));
#endif
		    break;

		case OBJ_FLOOR:
		    if (cansee_spot && !silent) {
/*JP
			You("suddenly see a figurine transform into %s!",
*/
			Your("目の前で人形は%sに姿を変えた！",
				monnambuf);
			redraw = TRUE;	/* update figurine's map location */
		    }
		    break;

		case OBJ_MINVENT:
		    if (cansee_spot && !silent) {
			struct monst *mon;
			mon = figurine->ocarry;
			/* figurine carring monster might be invisible */
			if (canseemon(figurine->ocarry)) {
/*JP
			    Sprintf(carriedby, "%s pack",
*/
			    Sprintf(carriedby, "%sの鞄",
				     s_suffix(a_monnam(mon)));
			}
			else if (is_pool(mon->mx, mon->my))
/*JP
			    Strcpy(carriedby, "empty water");
*/
			    Strcpy(carriedby, "何もない水中");
			else
/*JP
			    Strcpy(carriedby, "thin air");
*/
			    Strcpy(carriedby, "何もない空中");
#if 0 /*JP*/
			You("see %s %s out of %s!", monnambuf,
			    locomotion(mtmp->data, "drop"), carriedby);
#else
			You("%sが%sから%s出るのを見た！", monnambuf,
			    carriedby, locomotion(mtmp->data, "転げ"));
#endif
		    }
		    break;
#if 0
		case OBJ_MIGRATING:
		    break;
#endif

		default:
		    impossible("figurine came to life where? (%d)",
				(int)figurine->where);
		break;
	    }
	}
	/* free figurine now */
	obj_extract_self(figurine);
	obfree(figurine, (struct obj *)0);
	if (redraw) newsym(cc.x, cc.y);
}

STATIC_OVL boolean
figurine_location_checks(obj, cc, quietly)
struct obj *obj;
coord *cc;
boolean quietly;
{
	xchar x,y;

	if (carried(obj) && u.uswallow) {
		if (!quietly)
/*JP
			You("don't have enough room in here.");
*/
			pline("ここには十分な場所がない．");
		return FALSE;
	}
	x = cc->x; y = cc->y;
	if (!isok(x,y)) {
		if (!quietly)
/*JP
			You("cannot put the figurine there.");
*/
			pline("ここには人形を置けない．");
		return FALSE;
	}
	if (IS_ROCK(levl[x][y].typ) &&
	    !(passes_walls(&mons[obj->corpsenm]) && may_passwall(x,y))) {
		if (!quietly)
#if 0 /*JP*/
		    You("cannot place a figurine in %s!",
			IS_TREE(levl[x][y].typ) ? "a tree" : "solid rock");
#else
		    pline("%sの中には人形を置けない！",
			IS_TREE(levl[x][y].typ) ? "木" : "固い石");
#endif
		return FALSE;
	}
	if (sobj_at(BOULDER,x,y) && !passes_walls(&mons[obj->corpsenm])
			&& !throws_rocks(&mons[obj->corpsenm])) {
		if (!quietly)
/*JP
			You("cannot fit the figurine on the boulder.");
*/
		    pline("岩の上には人形を置けない．");
		return FALSE;
	}
	return TRUE;
}

STATIC_OVL void
use_figurine(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	xchar x, y;
	coord cc;

	if (u.uswallow) {
		/* can't activate a figurine while swallowed */
		if (!figurine_location_checks(obj, (coord *)0, FALSE))
			return;
	}
	if(!getdir((char *)0)) {
		flags.move = multi = 0;
		return;
	}
	x = u.ux + u.dx; y = u.uy + u.dy;
	cc.x = x; cc.y = y;
	/* Passing FALSE arg here will result in messages displayed */
	if (!figurine_location_checks(obj, &cc, FALSE)) return;
#if 0 /*JP*/
	You("%s and it transforms.",
	    (u.dx||u.dy) ? "set the figurine beside you" :
	    (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ||
	     is_pool(cc.x, cc.y)) ?
		"release the figurine" :
	    (u.dz < 0 ?
		"toss the figurine into the air" :
		"set the figurine on the ground"));
#else
	pline("あなたが人形を%sと，それは姿を変えた．",
	    (u.dx||u.dy) ? "わきに置く" :
	    (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ||
	     is_pool(cc.x, cc.y)) ? "手放す" :
	    (u.dz < 0 ? "空中に投げ上げる" : "地面に置く"));
#endif
	(void) make_familiar(obj, cc.x, cc.y, FALSE);
	(void) stop_timer(FIG_TRANSFORM, (genericptr_t)obj);
	useup(obj);
	*optr = 0;
}

static NEARDATA const char lubricables[] = { ALL_CLASSES, ALLOW_NONE, 0 };
static NEARDATA const char need_to_remove_outer_armor[] =
/*JP
			"need to remove your %s to grease your %s.";
*/
			"%sに脂を塗るには%sをはずす必要がある．";

STATIC_OVL void
use_grease(obj)
struct obj *obj;
{
	struct obj *otmp;
	char buf[BUFSZ];

	if (Glib) {
#if 0 /*JP*/
	    pline("%s from your %s.", Tobjnam(obj, "slip"),
		  makeplural(body_part(FINGER)));
#else
	    pline("%sはあなたの%sから滑り落ちた．", xname(obj),
		  body_part(FINGER));
#endif
	    dropx(obj);
	    return;
	}

	if (obj->spe > 0) {
		if ((obj->cursed || Fumbling) && !rn2(2)) {
			consume_obj_charge(obj, TRUE);

#if 0 /*JP*/
			pline("%s from your %s.", Tobjnam(obj, "slip"),
			      makeplural(body_part(FINGER)));
#else
			pline("%sはあなたの%sから滑り落ちた．", xname(obj),
			      body_part(FINGER));
#endif
			dropx(obj);
			return;
		}
		otmp = getobj(lubricables, "grease");
		if (!otmp) return;
		if ((otmp->owornmask & WORN_ARMOR) && uarmc) {
			Strcpy(buf, xname(uarmc));
/*JP
			You(need_to_remove_outer_armor, buf, xname(otmp));
*/
			You(need_to_remove_outer_armor, xname(otmp), buf);
			return;
		}
#ifdef TOURIST
		if ((otmp->owornmask & WORN_SHIRT) && (uarmc || uarm)) {
#if 0 /*JP*/
			Strcpy(buf, uarmc ? xname(uarmc) : "");
			if (uarmc && uarm) Strcat(buf, " and ");
			Strcat(buf, uarm ? xname(uarm) : "");
			You(need_to_remove_outer_armor, buf, xname(otmp));
#else
			Strcpy(buf, uarmc ? xname(uarmc) : "");
			if (uarmc && uarm) Strcat(buf, "と");
			Strcat(buf, uarm ? xname(uarm) : "");
			You(need_to_remove_outer_armor, xname(otmp), buf);
#endif
			return;
		}
#endif
		consume_obj_charge(obj, TRUE);

		if (otmp != &zeroobj) {
/*JP
			You("cover %s with a thick layer of grease.",
*/
			You("%sに脂を丹念に塗った．",
			    yname(otmp));
			otmp->greased = 1;
			if (obj->cursed && !nohands(youmonst.data)) {
			    incr_itimeout(&Glib, rnd(15));
/*JP
			    pline("Some of the grease gets all over your %s.",
*/
			    Your("%sまで脂まみれになってしまった．",
				makeplural(body_part(HAND)));
			}
		} else {
			Glib += rnd(15);
/*JP
			You("coat your %s with grease.",
*/
			You("自分の%sに脂を塗った．",
			    makeplural(body_part(FINGER)));
		}
	} else {
	    if (obj->known)
/*JP
		pline("%s empty.", Tobjnam(obj, "are"));
*/
		pline("%sは空っぽだ．", xname(obj));
	    else
/*JP
		pline("%s to be empty.", Tobjnam(obj, "seem"));
*/
		pline("%sは空っぽのようだ．", xname(obj));
	}
	update_inventory();
}

static struct trapinfo {
	struct obj *tobj;
	xchar tx, ty;
	int time_needed;
	boolean force_bungle;
} trapinfo;

void
reset_trapset()
{
	trapinfo.tobj = 0;
	trapinfo.force_bungle = 0;
}

static struct whetstoneinfo {
	struct obj *tobj, *wsobj;
	int time_needed;
} whetstoneinfo;

void
reset_whetstone()
{
	whetstoneinfo.tobj = 0;
	whetstoneinfo.wsobj = 0;
}

/* occupation callback */
STATIC_PTR
int
set_whetstone(VOID_ARGS)
{
	struct obj *otmp = whetstoneinfo.tobj, *ows = whetstoneinfo.wsobj;
	int chance;

	if (!otmp || !ows) {
	    reset_whetstone();
	    return 0;
	} else
	if (!carried(otmp) || !carried(ows)) {
/*JP
	    You("seem to have mislaid %s.",
*/
	    You("%sをセットしていないようだ．",
		!carried(otmp) ? yname(otmp) : yname(ows));
	    reset_whetstone();
	    return 0;
	}

	if (--whetstoneinfo.time_needed > 0) {
	    int adj = 2;
	    if (Blind) adj--;
	    if (Fumbling) adj--;
	    if (Confusion) adj--;
	    if (Stunned) adj--;
	    if (Hallucination) adj--;
	    if (adj > 0)
		whetstoneinfo.time_needed -= adj;
	    return 1;
	}

	chance = 4 - (ows->blessed) + (ows->cursed*2) + (otmp->oartifact ? 3 : 0);

	if (!rn2(chance) && (ows->otyp == WHETSTONE)) {
	    /* Remove rust first, then sharpen dull edges */
	    if (otmp->oeroded) {
		otmp->oeroded--;
#if 0 /*JP*/
		pline("%s %s%s now.", Yname2(otmp),
		    (Blind ? "probably " : (otmp->oeroded ? "almost " : "")),
		    otense(otmp, "shine"));
#else
		pline("%sは%s輝いた．", Yname2(otmp),
		    (Blind ? "たぶん" : (otmp->oeroded ? "少し" : "")));
#endif
	    } else
	    if (otmp->spe < 0) {
		otmp->spe++;
#if 0 /*JP*/
		pline("%s %s %ssharper now.%s", Yname2(otmp),
		    otense(otmp, Blind ? "feel" : "look"),
		    (otmp->spe >= 0 ? "much " : ""),
		    Blind ? "  (Ow!)" : "");
#else
		pline("%sは%s鋭くなった%s．", Yname2(otmp),
		    (otmp->spe >= 0 ? "ますます" : "だいぶ"),
		    Blind ? "と感じる" : "ように見える");
#endif
	    }
	    makeknown(WHETSTONE);
	    reset_whetstone();
	} else {
	    if (Hallucination)
#if 0 /*JP*/
		pline("%s %s must be faulty!",
		    is_plural(ows) ? "These" : "This", xname(ows));
	    else pline("%s", Blind ? "Pheww!  This is hard work!" :
		"There are no visible effects despite your efforts.");
#else
		pline("この%sは欠陥品に間違いない！", xname(ows));
	    else pline("%s", Blind ? "フゥ！これは難しいな！" :
		"努力の割には目に見えるような効果はないようだ．");
#endif
	    reset_whetstone();
	}

	return 0;
}

/* use stone on obj. the stone doesn't necessarily need to be a whetstone. */
STATIC_OVL void
use_whetstone(stone, obj)
struct obj *stone, *obj;
{
	boolean fail_use = TRUE;
#if 0 /*JP*/
	const char *occutext = "sharpening";
#else
	const char *occutext = "研ぐ";
#endif
	int tmptime = 130 + (rnl(13) * 5);

	if (u.ustuck && sticks(youmonst.data)) {
#if 0 /*JP*/
	    You("should let go of %s first.", mon_nam(u.ustuck));
#else
	    You("まず最初に%sから出なければならない．", mon_nam(u.ustuck));
#endif
	} else
	if ((welded(uwep) && (uwep != stone)) ||
		(uswapwep && u.twoweap && welded(uswapwep) && (uswapwep != obj))) {
#if 0 /*JP*/
	    You("need both hands free.");
#else
	    You("両手を開ける必要がある．");
#endif
	} else
	if (nohands(youmonst.data)) {
#if 0 /*JP*/
	    You("can't handle %s with your %s.",
		an(xname(stone)), makeplural(body_part(HAND)));
#else
	    pline("あなたの%sでは%sを扱うことができない．",
		makeplural(body_part(HAND)), an(xname(stone)));
#endif
	} else
	if (verysmall(youmonst.data)) {
#if 0 /*JP*/
	    You("are too small to use %s effectively.", an(xname(stone)));
#else
	    You("小さすぎて%sを有効に使うことができない．", an(xname(stone)));
#endif
	} else
#ifdef GOLDOBJ
	if (obj == &goldobj) {
#if 0 /*JP*/
	    pline("Shopkeepers would spot the lighter coin%s immediately.",
		obj->quan > 1 ? "s" : "");
#else
	    pline("店主はただちに光る金貨に注目するだろう．");
#endif
	} else
#endif
	if (!is_pool(u.ux, u.uy) && !IS_FOUNTAIN(levl[u.ux][u.uy].typ)
#ifdef SINKS
	    && !IS_SINK(levl[u.ux][u.uy].typ) && !IS_TOILET(levl[u.ux][u.uy].typ)
#endif
	    ) {
	    if (carrying(POT_WATER) && objects[POT_WATER].oc_name_known) {
#if 0 /*JP*/
		pline("Better not waste bottled water for that.");
#else
		pline("それをするために瓶詰めの水を無駄遣いするのはよくない．");
#endif
	    } else
#if 0 /*JP*/
		You("need some water when you use that.");
#else
		pline("それを使うには水が必要だ．");
#endif
	} else
	if (Levitation && !Lev_at_will && !u.uinwater) {
#if 0 /*JP*/
	    You("can't reach the water.");
#else
	    You("水に届かない．");
#endif
	} else
	    fail_use = FALSE;

	if (fail_use) {
	    reset_whetstone();
	    return;
	}

	if (stone == whetstoneinfo.wsobj && obj == whetstoneinfo.tobj &&
	    carried(obj) && carried(stone)) {
#if 0 /*JP*/
	    You("resume %s %s.", occutext, yname(obj));
#else
	    You("%sを%sのを再開した．", yname(obj), occutext);
#endif
	    set_occupation(set_whetstone, occutext, 0);
	    return;
	}

	if (obj) {
	    int ttyp = obj->otyp;
	    boolean isweapon = (obj->oclass == WEAPON_CLASS || is_weptool(obj));
	    boolean isedged = (is_pick(obj) ||
				(objects[ttyp].oc_dir & (PIERCE|SLASH)));
	    if (obj == &zeroobj) {
#if 0 /*JP*/
		You("file your nails.");
#else
		You("爪をやすりがけした．");
#endif
	    } else
	    if (!isweapon || !isedged) {
#if 0 /*JP*/
		pline("%s sharp enough already.",
			is_plural(obj) ? "They are" : "It is");
#else
		pline("%sは十分に鋭い．",
			is_plural(obj) ? "それら" : "それ");
#endif
	    } else
	    if (stone->quan > 1) {
#if 0 /*JP*/
		pline("Using one %s is easier.", singular(stone, xname));
#else
		pline("%sはひとつだけのほうがより使いやすい．", singular(stone, xname));
#endif
	    } else
	    if (obj->quan > 1) {
#if 0 /*JP*/
		You("can apply %s only on one %s at a time.",
		    the(xname(stone)),
		    (obj->oclass == WEAPON_CLASS ? "weapon" : "item"));
#else
		You("一回につき%sをひとつの%sに対してしか使うことができない．",
		    the(xname(stone)),
		    (obj->oclass == WEAPON_CLASS ? "武器" : "道具"));
#endif
	    } else
	    if (!is_metallic(obj)) {
#if 0 /*JP*/
		pline("That would ruin the %s %s.",
			materialnm[objects[ttyp].oc_material],
		xname(obj));
#else
		pline("それは%sの%sを駄目にしてしまうだろう．",
			xname(obj), materialnm[objects[ttyp].oc_material]);
#endif
	    } else
	    if (((obj->spe >= 0) || !obj->known) && !obj->oeroded) {
#if 0 /*JP*/
		pline("%s %s sharp and pointy enough.",
			is_plural(obj) ? "They" : "It",
			otense(obj, Blind ? "feel" : "look"));
#else
		pline("それ%sは十分鋭い切っ先を持っている%s．",
			is_plural(obj) ? "ら" : "",
			Blind ? "ような気がする" : "ように見える");
#endif
	    } else {
		if (stone->cursed) tmptime *= 2;
		whetstoneinfo.time_needed = tmptime;
		whetstoneinfo.tobj = obj;
		whetstoneinfo.wsobj = stone;
#if 0 /*JP*/
		You("start %s %s.", occutext, yname(obj));
#else
		You("%sを研ぎ始めた．", yname(obj));
#endif
		set_occupation(set_whetstone, occutext, 0);
		if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)) whetstone_fountain_effects(obj);
#ifdef SINKS
		else if (IS_SINK(levl[u.ux][u.uy].typ)) whetstone_sink_effects(obj);
		else if (IS_TOILET(levl[u.ux][u.uy].typ)) whetstone_toilet_effects(obj);
#endif
	    }
#if 0 /*JP*/
	} else You("wave %s in the %s.", the(xname(stone)),
	    (IS_POOL(levl[u.ux][u.uy].typ) && Underwater) ? "water" : "air");
#else
	} else You("%sを%s振った．", the(xname(stone)),
	    (IS_POOL(levl[u.ux][u.uy].typ) && Underwater) ? "水中で" : "空中で");
#endif
}

/* touchstones - by Ken Arnold */
STATIC_OVL void
use_stone(tstone)
struct obj *tstone;
{
    struct obj *obj;
    boolean do_scratch;
    const char *streak_color, *choices;
    char stonebuf[QBUFSZ];
/*JP
    static const char scritch[] = "\"scritch, scritch\"";
*/
    static const char scritch[] = "「ゴシ，ゴシ」";
    static const char allowall[3] = { COIN_CLASS, ALL_CLASSES, 0 };
    static const char justgems[3] = { ALLOW_NONE, GEM_CLASS, 0 };
#ifndef GOLDOBJ
    struct obj goldobj;
#endif

    /* in case it was acquired while blinded */
    if (!Blind) tstone->dknown = 1;
    /* when the touchstone is fully known, don't bother listing extra
       junk as likely candidates for rubbing */
    choices = (tstone->otyp == TOUCHSTONE && tstone->dknown &&
		objects[TOUCHSTONE].oc_name_known) ? justgems : allowall;
/*JP
    Sprintf(stonebuf, "rub on the stone%s", plur(tstone->quan));
*/
    Sprintf(stonebuf, "rub on the stone");
    if ((obj = getobj(choices, stonebuf)) == 0)
	return;
#ifndef GOLDOBJ
    if (obj->oclass == COIN_CLASS) {
	u.ugold += obj->quan;	/* keep botl up to date */
	goldobj = *obj;
	dealloc_obj(obj);
	obj = &goldobj;
    }
#endif

    if (obj == tstone && obj->quan == 1) {
#if 0 /*JP*/
	You_cant("rub %s on itself.", the(xname(obj)));
#else
	You("%sをそれ自体でこすることはできない．", the(xname(obj)));
#endif
	return;
    }

    if (tstone->otyp == TOUCHSTONE && tstone->cursed &&
	    obj->oclass == GEM_CLASS && !is_graystone(obj) &&
	    !obj_resists(obj, 80, 100)) {
	if (Blind)
#if 0 /*JP*/
	    pline("You feel something shatter.");
#else
	    You("何かが粉々になるのを感じた．");
#endif
	else if (Hallucination)
#if 0 /*JP*/
	    pline("Oh, wow, look at the pretty shards.");
#else
	    pline("ワーォ！なんてきれいな破片なんだ．");
#endif
	else
#if 0 /*JP*/
	    pline("A sharp crack shatters %s%s.",
		  (obj->quan > 1) ? "one of " : "", the(xname(obj)));
#else
	    pline("強くこすりすぎて%s%sは粉々になってしまった．",
		  the(xname(obj)), (obj->quan > 1) ? "のひとつ" : "");
#endif
#ifndef GOLDOBJ
     /* assert(obj != &goldobj); */
#endif
	useup(obj);
	return;
    }

    if (Blind) {
	pline(scritch);
	return;
    } else if (Hallucination) {
#if 0 /*JP*/
	pline("Oh wow, man: Fractals!");
#else
	pline("ワーォ！フラクタル模様だ！");
#endif
	return;
    }

    do_scratch = FALSE;
    streak_color = 0;

    switch (obj->oclass) {
    case WEAPON_CLASS:
    case TOOL_CLASS:
	use_whetstone(tstone, obj);
	return;
    case GEM_CLASS:	/* these have class-specific handling below */
    case RING_CLASS:
	if (tstone->otyp != TOUCHSTONE) {
	    do_scratch = TRUE;
	} else if (obj->oclass == GEM_CLASS && (tstone->blessed ||
		(!tstone->cursed &&
		    (Role_if(PM_ARCHEOLOGIST) || Race_if(PM_GNOME))))) {
	    makeknown(TOUCHSTONE);
	    makeknown(obj->otyp);
	    prinv((char *)0, obj, 0L);
	    return;
	} else {
	    /* either a ring or the touchstone was not effective */
	    if (objects[obj->otyp].oc_material == GLASS) {
		do_scratch = TRUE;
		break;
	    }
	}
	streak_color = c_obj_colors[objects[obj->otyp].oc_color];
	break;		/* gem or ring */

    default:
	switch (objects[obj->otyp].oc_material) {
	case CLOTH:
#if 0 /*JP*/
	    pline("%s a little more polished now.", Tobjnam(tstone, "look"));
#else
	    pline("%sはさらにつやが出たように見える．", xname(tstone));
#endif
	    return;
	case LIQUID:
	    if (!obj->known)		/* note: not "whetstone" */
#if 0 /*JP*/
		You("must think this is a wetstone, do you?");
#else
		You("これは砥石だと思った？");
#endif
	    else
#if 0 /*JP*/
		pline("%s a little wetter now.", Tobjnam(tstone, "are"));
#else
		pline("%sは少し濡れた．", xname(tstone));
#endif
	    return;
	case WAX:
#if 0 /*JP*/
	    streak_color = "waxy";
#else
	    streak_color = "蝋の";
#endif
	    break;		/* okay even if not touchstone */
	case WOOD:
#if 0 /*JP*/
	    streak_color = "wooden";
#else
	    streak_color = "おがくずのような";
#endif
	    break;		/* okay even if not touchstone */
	case GOLD:
	    do_scratch = TRUE;	/* scratching and streaks */
#if 0 /*JP*/
	    streak_color = "golden";
#else
	    streak_color = "金色の";
#endif
	    break;
	case SILVER:
	    do_scratch = TRUE;	/* scratching and streaks */
#if 0 /*JP*/
	    streak_color = "silvery";
#else
	    streak_color = "銀色の";
#endif
	    break;
	default:
	    /* Objects passing the is_flimsy() test will not
	       scratch a stone.  They will leave streaks on
	       non-touchstones and touchstones alike. */
	    if (is_flimsy(obj))
		streak_color = c_obj_colors[objects[obj->otyp].oc_color];
	    else
		do_scratch = (tstone->otyp != TOUCHSTONE);
	    break;
	}
	break;		/* default oclass */
    }

#if 0 /*JP*/
    Sprintf(stonebuf, "stone%s", plur(tstone->quan));
#endif
    if (do_scratch)
#if 0 /*JP*/
	pline("You make %s%sscratch marks on the %s.",
	      streak_color ? streak_color : (const char *)"",
	      streak_color ? " " : "", stonebuf);
#else
	You("%sこすり跡を石につけた．",
              streak_color ? streak_color : (const char *)"");
#endif
    else if (streak_color)
#if 0 /*JP*/
	pline("You see %s streaks on the %s.", streak_color, stonebuf);
#else
	pline("石に%s筋がついた．", streak_color);
#endif
    else
	pline(scritch);
    return;
}

/* Place a landmine/bear trap.  Helge Hafting */
STATIC_OVL void
use_trap(otmp)
struct obj *otmp;
{
	int ttyp, tmp;
	const char *what = (char *)0;
	char buf[BUFSZ];
#if 0 /*JP*/
	const char *occutext = "setting the trap";
#else
	const char *occutext = "罠を仕掛けている";
#endif

	if (nohands(youmonst.data))
/*JP
	    what = "without hands";
*/
	    what = "手がないので";
	else if (Stunned)
/*JP
	    what = "while stunned";
*/
	    what = "くらくらしているので";
	else if (u.uswallow)
#if 0 /*JP*/
	    what = is_animal(u.ustuck->data) ? "while swallowed" :
			"while engulfed";
#else
	    what = is_animal(u.ustuck->data) ? "飲み込まれている間は" :
			"巻き込まれている間は";
#endif
	else if (Underwater)
/*JP
	    what = "underwater";
*/
	    what = "水面下では";
	else if (Levitation)
/*JP
	    what = "while levitating";
*/
	    what = "浮いている間は";
	else if (is_pool(u.ux, u.uy))
/*JP
	    what = "in water";
*/
	    what = "水中では";
	else if (is_lava(u.ux, u.uy))
/*JP
	    what = "in lava";
*/
	    what = "溶岩の中では";
	else if (On_stairs(u.ux, u.uy))
	    what = (u.ux == xdnladder || u.ux == xupladder) ?
/*JP
			"on the ladder" : "on the stairs";
*/
			"はしごの上では" : "階段の上では";
	else if (IS_FURNITURE(levl[u.ux][u.uy].typ) ||
		IS_ROCK(levl[u.ux][u.uy].typ) ||
		closed_door(u.ux, u.uy) || t_at(u.ux, u.uy))
/*JP
	    what = "here";
*/
	    what = "ここでは";
	if (what) {
/*JP
	    You_cant("set a trap %s!",what);
*/
	    pline("%s罠を仕掛けられない！",what);
	    reset_trapset();
	    return;
	}
	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	if (otmp == trapinfo.tobj &&
		u.ux == trapinfo.tx && u.uy == trapinfo.ty) {
#if 0 /*JP*/
	    You("resume setting %s %s.",
		shk_your(buf, otmp),
		defsyms[trap_to_defsym(what_trap(ttyp))].explanation);
#else
	    You("%s%sを仕掛けるのを再開した．", shk_your(buf, otmp),
		jtrns_obj('^', defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
#endif
	    set_occupation(set_trap, occutext, 0);
	    return;
	}
	trapinfo.tobj = otmp;
	trapinfo.tx = u.ux,  trapinfo.ty = u.uy;
	tmp = ACURR(A_DEX);
	trapinfo.time_needed = (tmp > 17) ? 2 : (tmp > 12) ? 3 :
				(tmp > 7) ? 4 : 5;
	if (Blind) trapinfo.time_needed *= 2;
	tmp = ACURR(A_STR);
	if (ttyp == BEAR_TRAP && tmp < 18)
	    trapinfo.time_needed += (tmp > 12) ? 1 : (tmp > 7) ? 2 : 4;
	/*[fumbling and/or confusion and/or cursed object check(s)
	   should be incorporated here instead of in set_trap]*/
#ifdef STEED
	if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
	    boolean chance;

	    if (Fumbling || otmp->cursed) chance = (rnl(10) > 3);
	    else  chance = (rnl(10) > 5);
#if 0 /*JP:T*/
	    You("aren't very skilled at reaching from %s.",
		mon_nam(u.usteed));
#else
	    pline("%sの上からではうまく仕掛けられないかもしれない．",
		mon_nam(u.usteed));
#endif
#if 0 /*JP*/
	    Sprintf(buf, "Continue your attempt to set %s?",
		the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
	    if(yn(buf) == 'y') {
#else
	    if(yn("それでも続ける？") == 'y') {
#endif
		if (chance) {
			switch(ttyp) {
			    case LANDMINE:	/* set it off */
			    	trapinfo.time_needed = 0;
			    	trapinfo.force_bungle = TRUE;
				break;
			    case BEAR_TRAP:	/* drop it without arming it */
				reset_trapset();
#if 0 /*JP*/
				You("drop %s!",
			  the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
#else
				You("%sを落した！",
	jtrns_obj('^', defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
#endif
				dropx(otmp);
				return;
			}
		}
	    } else {
	    	reset_trapset();
		return;
	    }
	}
#endif
#if 0 /*JP*/
	You("begin setting %s %s.",
	    shk_your(buf, otmp),
	    defsyms[trap_to_defsym(what_trap(ttyp))].explanation);
#else
	You("%s%sを仕掛けはじめた．", shk_your(buf, otmp),
	    jtrns_obj('^', defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
#endif
	set_occupation(set_trap, occutext, 0);
	return;
}

STATIC_PTR
int
set_trap(VOID_ARGS)
{
	struct obj *otmp = trapinfo.tobj;
	struct trap *ttmp;
	int ttyp;

	if (!otmp || !carried(otmp) ||
		u.ux != trapinfo.tx || u.uy != trapinfo.ty) {
	    /* ?? */
	    reset_trapset();
	    return 0;
	}

	if (--trapinfo.time_needed > 0) return 1;	/* still busy */

	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	ttmp = maketrap(u.ux, u.uy, ttyp);
	if (ttmp) {
	    ttmp->tseen = 1;
	    ttmp->madeby_u = 1;
	    newsym(u.ux, u.uy); /* if our hero happens to be invisible */
	    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
		add_damage(u.ux, u.uy, 0L);		/* schedule removal */
	    }
	    if (!trapinfo.force_bungle)
#if 0 /*JP*/
		You("finish arming %s.",
			the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
#else
		You("%sを仕掛け終えた．",
			jtrns_obj('^', defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
#endif
	    if (((otmp->cursed || Fumbling) && (rnl(10) > 5)) || trapinfo.force_bungle)
		dotrap(ttmp,
			(unsigned)(trapinfo.force_bungle ? FORCEBUNGLE : 0));
	} else {
	    /* this shouldn't happen */
/*JP
	    Your("trap setting attempt fails.");
*/
	    You("罠を仕掛けるのに失敗した．");
	}
	useup(otmp);
	reset_trapset();
	return 0;
}

STATIC_OVL int
use_whip(obj)
struct obj *obj;
{
    char buf[BUFSZ];
    struct monst *mtmp;
    struct obj *otmp;
    int rx, ry, proficient, res = 0;
#if 0 /*JP*/
    const char *msg_slipsfree = "The bullwhip slips free.";
    const char *msg_snap = "Snap!";
#else
    const char *msg_slipsfree = "鞭はほどけた．";
    const char *msg_snap = "ピシッ！";
#endif

    if (obj != uwep) {
	if (!wield_tool(obj, "lash")) return 0;
	else res = 1;
    }
    if (!getdir((char *)0)) return res;

    if (Stunned || (Confusion && !rn2(5))) confdir();
    rx = u.ux + u.dx;
    ry = u.uy + u.dy;
    mtmp = m_at(rx, ry);

    /* fake some proficiency checks */
    proficient = 0;
    if (Role_if(PM_ARCHEOLOGIST)) ++proficient;
    if (ACURR(A_DEX) < 6) proficient--;
    else if (ACURR(A_DEX) >= 14) proficient += (ACURR(A_DEX) - 14);
    if (Fumbling) --proficient;
    if (proficient > 3) proficient = 3;
    if (proficient < 0) proficient = 0;

    if (u.uswallow && attack(u.ustuck)) {
/*JP
	There("is not enough room to flick your bullwhip.");
*/
	pline("鞭を振るえるほど広くない．");

    } else if (Underwater) {
/*JP
	There("is too much resistance to flick your bullwhip.");
*/
	pline("水の抵抗がありすぎて鞭を振るえない．");

    } else if (u.dz < 0) {
/*JP
	You("flick a bug off of the %s.",ceiling(u.ux,u.uy));
*/
	You("%sの虫を打ち落した．",ceiling(u.ux,u.uy));

    } else if ((!u.dx && !u.dy) || (u.dz > 0)) {
	int dam;

#ifdef STEED
	/* Sometimes you hit your steed by mistake */
	if (u.usteed && !rn2(proficient + 2)) {
/*JP
	    You("whip %s!", mon_nam(u.usteed));
*/
	    You("%sを鞭打った！", mon_nam(u.usteed));
	    kick_steed();
	    return 1;
	}
#endif
	if (Levitation
#ifdef STEED
			|| u.usteed
#endif
		) {
	    /* Have a shot at snaring something on the floor */
	    otmp = level.objects[u.ux][u.uy];
	    if (otmp && otmp->otyp == CORPSE && otmp->corpsenm == PM_HORSE) {
/*JP
		pline("Why beat a dead horse?");
*/
		pline("どうして死んだ馬を鞭打つようなことをするの？");
		return 1;
	    }
	    if (otmp && proficient) {
#if 0 /*JP*/
		You("wrap your bullwhip around %s on the %s.",
		    an(singular(otmp, xname)), surface(u.ux, u.uy));
#else
		You("鞭を%sの上の%sにからませた．",
		    surface(u.ux, u.uy), an(singular(otmp, xname)));
#endif
		if (rnl(6) || pickup_object(otmp, 1L, TRUE) < 1)
		    pline(msg_slipsfree);
		return 1;
	    }
	}
	dam = rnd(2) + dbon() + obj->spe;
	if (dam <= 0) dam = 1;
/*JP
	You("hit your %s with your bullwhip.", body_part(FOOT));
*/
	You("自分の%sを鞭で打ちつけた．", body_part(FOOT));
#if 0 /*JP:T*/
	Sprintf(buf, "killed %sself with %s bullwhip", uhim(), uhis());
	losehp(dam, buf, NO_KILLER_PREFIX);
#else
	Strcpy(buf, "自分自身を鞭打って");
	losehp(dam, buf, KILLED_BY);
#endif
	flags.botl = 1;
	return 1;

    } else if ((Fumbling || Glib) && !rn2(5)) {
/*JP
	pline_The("bullwhip slips out of your %s.", body_part(HAND));
*/
	pline("鞭が%sからすべり落ちた．", body_part(HAND));
	dropx(obj);

    } else if (u.utrap && u.utraptype == TT_PIT) {
	/*
	 *     Assumptions:
	 *
	 *	if you're in a pit
	 *		- you are attempting to get out of the pit
	 *		- or, if you are applying it towards a small
	 *		  monster then it is assumed that you are
	 *		  trying to hit it.
	 *	else if the monster is wielding a weapon
	 *		- you are attempting to disarm a monster
	 *	else
	 *		- you are attempting to hit the monster
	 *
	 *	if you're confused (and thus off the mark)
	 *		- you only end up hitting.
	 *
	 */
	const char *wrapped_what = (char *)0;

	if (mtmp) {
	    if (bigmonst(mtmp->data)) {
		wrapped_what = strcpy(buf, mon_nam(mtmp));
	    } else if (proficient) {
		if (attack(mtmp)) return 1;
		else pline(msg_snap);
	    }
	}
	if (!wrapped_what) {
	    if (IS_FURNITURE(levl[rx][ry].typ))
		wrapped_what = something;
	    else if (sobj_at(BOULDER, rx, ry))
/*JP
		wrapped_what = "a boulder";
*/
		wrapped_what = "岩";
	}
	if (wrapped_what) {
	    coord cc;

	    cc.x = rx; cc.y = ry;
/*JP
	    You("wrap your bullwhip around %s.", wrapped_what);
*/
	    You("鞭を%sにからませた．", wrapped_what);
	    if (proficient && rn2(proficient + 2)) {
		if (!mtmp || enexto(&cc, rx, ry, youmonst.data)) {
/*JP
		    You("yank yourself out of the pit!");
*/
		    You("ぐいと引っぱって穴から抜け出した！");
		    teleds(cc.x, cc.y, TRUE);
		    u.utrap = 0;
		    vision_full_recalc = 1;
		}
	    } else {
		pline(msg_slipsfree);
	    }
	    if (mtmp) wakeup(mtmp);
	} else pline(msg_snap);

    } else if (mtmp) {
	if (!canspotmon(mtmp) &&
		!memory_is_invisible(rx, ry)) {
#if 0 /*JP*/
	   pline("A monster is there that you couldn't see.");
#else
	   pline("見えない怪物がいる．");
#endif
	   map_invisible(rx, ry);
	}
	otmp = MON_WEP(mtmp);	/* can be null */
	if (otmp) {
	    char onambuf[BUFSZ];
	    const char *mon_hand;
	    boolean gotit = proficient && (!Fumbling || !rn2(10));

	    Strcpy(onambuf, cxname(otmp));
	    if (gotit) {
		mon_hand = mbodypart(mtmp, HAND);
		if (bimanual(otmp)) mon_hand = makeplural(mon_hand);
	    } else
		mon_hand = 0;	/* lint suppression */

/*JP
	    You("wrap your bullwhip around %s %s.",
*/
	    You("鞭を%sの%sにからませた．",
		s_suffix(mon_nam(mtmp)), onambuf);
	    if (gotit && otmp->cursed) {
#if 0 /*JP*/
		pline("%s welded to %s %s%c",
		      (otmp->quan == 1L) ? "It is" : "They are",
		      mhis(mtmp), mon_hand,
		      !otmp->bknown ? '!' : '.');
#else
		pline("%sは%sの%sにくっついてしまっている%s",
		      onambuf,
		      mon_nam(mtmp), mon_hand,
		      !otmp->bknown ? "！" : "．");
#endif
		otmp->bknown = 1;
		gotit = FALSE;	/* can't pull it free */
	    }
	    if (gotit) {
		obj_extract_self(otmp);
		possibly_unwield(mtmp, FALSE);
		setmnotwielded(mtmp,otmp);

		switch (rn2(proficient + 1)) {
		case 2:
		    /* to floor near you */
/*JP
		    You("yank %s %s to the %s!", s_suffix(mon_nam(mtmp)),
*/
		    You("%sの%sを%sに引き落した！", mon_nam(mtmp),
			onambuf, surface(u.ux, u.uy));
		    place_object(otmp, u.ux, u.uy);
		    stackobj(otmp);
		    break;
		case 3:
		    /* right to you */
#if 0
		    if (!rn2(25)) {
			/* proficient with whip, but maybe not
			   so proficient at catching weapons */
			int hitu, hitvalu;

			hitvalu = 8 + otmp->spe;
			hitu = thitu(hitvalu,
				     dmgval(otmp, &youmonst),
				     otmp, (char *)0);
			if (hitu) {
/*JP
			    pline_The("%s hits you as you try to snatch it!",
*/
			    pline_The("%sを奪おうとしたらあなたに当たった！",
				the(onambuf));
			}
			place_object(otmp, u.ux, u.uy);
			stackobj(otmp);
			break;
		    }
#endif /* 0 */
		    /* right into your inventory */
/*JP
		    You("snatch %s %s!", s_suffix(mon_nam(mtmp)), onambuf);
*/
		    You("%sの%sを奪った！", mon_nam(mtmp), onambuf);
		    if (otmp->otyp == CORPSE &&
			    touch_petrifies(&mons[otmp->corpsenm]) &&
			    !uarmg && !Stone_resistance &&
			    !(poly_when_stoned(youmonst.data) &&
				polymon(PM_STONE_GOLEM))) {
			char kbuf[BUFSZ];

#if 0 /*JP*/
			Sprintf(kbuf, "%s corpse",
				an(mons[otmp->corpsenm].mname));
			pline("Snatching %s is a fatal mistake.", kbuf);
#else
			pline("%sの死体を奪ったのは致命的なミスだ．",
				jtrns_mon(mons[otmp->corpsenm].mname));
			Sprintf(kbuf, "%sの死体に触れて",
				jtrns_mon(mons[otmp->corpsenm].mname));
#endif
			instapetrify(kbuf);
		    }
/*JP
		    otmp = hold_another_object(otmp, "You drop %s!",
*/
		    otmp = hold_another_object(otmp, "%sを落した！",
					       doname(otmp), (const char *)0);
		    break;
		default:
		    /* to floor beneath mon */
/*JP
		    You("yank %s from %s %s!", the(onambuf),
*/
		    You("%sを%sの%sから叩き落とした！", the(xname(otmp)),
			s_suffix(mon_nam(mtmp)), mon_hand);
		    obj_no_longer_held(otmp);
		    place_object(otmp, mtmp->mx, mtmp->my);
		    stackobj(otmp);
		    break;
		}
	    } else {
		pline(msg_slipsfree);
	    }
	    wakeup(mtmp);
	} else {
	    if (mtmp->m_ap_type &&
		!Protection_from_shape_changers && !sensemon(mtmp))
		stumble_onto_mimic(mtmp);
/*JP
	    else You("flick your bullwhip towards %s.", mon_nam(mtmp));
*/
	    else You("%sに向って鞭を振るった．", mon_nam(mtmp));
	    if (proficient) {
		if (attack(mtmp)) return 1;
		else pline(msg_snap);
	    }
	}

    } else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
	    /* it must be air -- water checked above */
/*JP
	    You("snap your whip through thin air.");
*/
	    You("何もないところで鞭を振るった．");

    } else {
	pline(msg_snap);

    }
    return 1;
}


static const char
#if 0 /*JP*/
	not_enough_room[] = "There's not enough room here to use that.",
	where_to_hit[] = "Where do you want to hit?",
	cant_see_spot[] = "won't hit anything if you can't see that spot.",
	cant_reach[] = "can't reach that spot from here.";
#else
	not_enough_room[] = "それを使うだけの広さがない．",
	where_to_hit[] = "どれを狙う？",
	cant_see_spot[] = "場所が見えなければ狙えない．",
	cant_reach[] = "ここからそこへは届かない．";
#endif

/* Distance attacks by pole-weapons */
STATIC_OVL int
use_pole (obj)
	struct obj *obj;
{
#ifdef WEAPON_SKILLS
	int typ;
#endif
	int res = 0, max_range = 4;
	int min_range = obj->otyp == FISHING_POLE ? 1 : 4;
	coord cc;
	struct monst *mtmp;
	struct obj *otmp;
	boolean fishing;


	/* Are you allowed to use the pole? */
	if (u.uswallow) {
	    pline(not_enough_room);
	    return (0);
	}
	if (obj != uwep) {
	    if (!wield_tool(obj, "swing")) return(0);
	    else res = 1;
	}

	/* Prompt for a location */
	pline(where_to_hit);
	cc.x = u.ux;
	cc.y = u.uy;
/*JP
	if (getpos(&cc, TRUE, "the spot to hit") < 0)
*/
	if (getpos(&cc, TRUE, "狙う場所") < 0)
	    return 0;	/* user pressed ESC */

#ifdef WEAPON_SKILLS
	/* Calculate range */
	typ = weapon_type(obj);
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
	else if (P_SKILL(typ) <= P_SKILLED) max_range = 5;
	else max_range = 8;
#else
	max_range = 8;
#endif

	if (distu(cc.x, cc.y) > max_range) {
/*JP
	    pline("Too far!");
*/
	    pline("遠すぎる！");
	    return (res);
	} else if (distu(cc.x, cc.y) < min_range) {
/*JP
	    pline("Too close!");
*/
	    pline("近すぎる！");
	    return (res);
	} else if (!cansee(cc.x, cc.y) &&
		   ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0 ||
		    !canseemon(mtmp))) {
	    You(cant_see_spot);
	    return (res);
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
	    You(cant_reach);
	    return res;
	}

	/* What is there? */
	mtmp = m_at(cc.x, cc.y);

	if (obj->otyp == FISHING_POLE) {
	    fishing = is_pool(cc.x, cc.y);
	    /* Try a random effect */
	    switch (rnd(6))
	    {
		case 1:
		    /* Snag yourself */
#if 0 /*JP*/
		    You("hook yourself!");
		    losehp(rn1(10,10), "a fishing hook", KILLED_BY);
#else
		    You("自分自身を引っかけた！");
		    losehp(rn1(10,10), "釣竿で", KILLED_BY);
#endif
		    return 1;
		case 2:
		    /* Reel in a fish */
		    if (mtmp) {
			if ((bigmonst(mtmp->data) || strongmonst(mtmp->data))
				&& !rn2(2)) {
#if 0 /*JP*/
			    You("are yanked toward the %s", surface(cc.x,cc.y));
#else
			    You("%sの方に引っ張られた．", surface(cc.x,cc.y));
#endif
			    hurtle(sgn(cc.x-u.ux), sgn(cc.y-u.uy), 1, TRUE);
			    return 1;
			} else if (enexto(&cc, u.ux, u.uy, 0)) {
#if 0 /*JP*/
			    You("reel in %s!", mon_nam(mtmp));
#else
			    You("%sを釣り上げた！", mon_nam(mtmp));
#endif
			    mtmp->mundetected = 0;
			    rloc_to(mtmp, cc.x, cc.y);
			    return 1;
			}
		    }
		    break;
		case 3:
		    /* Snag an existing object */
		    if ((otmp = level.objects[cc.x][cc.y]) != (struct obj *)0) {
#if 0 /*JP*/
			You("snag an object from the %s!", surface(cc.x, cc.y));
#else
			You("%sから何かを引き上げた！", surface(cc.x, cc.y));
#endif
			pickup_object(otmp, 1, FALSE);
			/* If pickup fails, leave it alone */
			newsym(cc.x, cc.y);
			return 1;
		    }
		    break;
		case 4:
		    /* Snag some garbage */
		    if (fishing && flags.boot_count < 1 &&
			    (otmp = mksobj(LOW_BOOTS, TRUE, FALSE)) !=
			    (struct obj *)0) {
			flags.boot_count++;
#if 0 /*JP*/
			You("snag some garbage from the %s!",
#else
			You("%sから何かのゴミを引き上げた！",
#endif
				surface(cc.x, cc.y));
			if (pickup_object(otmp, 1, FALSE) <= 0) {
			    obj_extract_self(otmp);
			    place_object(otmp, u.ux, u.uy);
			    newsym(u.ux, u.uy);
			}
			return 1;
		    }
#ifdef SINKS
		    /* Or a rat in the sink/toilet */
		    if (!(mvitals[PM_SEWER_RAT].mvflags & G_GONE) &&
			    (IS_SINK(levl[cc.x][cc.y].typ) ||
			    IS_TOILET(levl[cc.x][cc.y].typ))) {
			mtmp = makemon(&mons[PM_SEWER_RAT], cc.x, cc.y,
				NO_MM_FLAGS);
#if 0 /*JP*/
			pline("Eek!  There's %s there!",
				Blind ? "something squirmy" : a_monnam(mtmp));
#else
			pline("げ！ここには%sがいる！",
				Blind ? "のたうつ何か" : a_monnam(mtmp));
#endif
			return 1;
		    }
#endif
		    break;
		case 5:
		    /* Catch your dinner */
		    if (fishing && (otmp = mksobj(CRAM_RATION, TRUE, FALSE)) !=
			    (struct obj *)0) {
#if 0 /*JP*/
			You("catch tonight's dinner!");
#else
			You("今日の晩飯を捕獲した！");
#endif
			if (pickup_object(otmp, 1, FALSE) <= 0) {
			    obj_extract_self(otmp);
			    place_object(otmp, u.ux, u.uy);
			    newsym(u.ux, u.uy);
			}
			return 1;
		    }
		    break;
		default:
		case 6:
		    /* Untrap */
		    /* FIXME -- needs to deal with non-adjacent traps */
		    break;
	    }
	}

	/* The effect didn't apply.  Attack the monster there. */
	if (mtmp) {
	    int oldhp = mtmp->mhp;

	    bhitpos = cc;
	    check_caitiff(mtmp);
	    (void) thitmonst(mtmp, uwep, 1);
	    /* check the monster's HP because thitmonst() doesn't return
	     * an indication of whether it hit.  Not perfect (what if it's a
	     * non-silver weapon on a shade?)
	     */
	    if (mtmp->mhp < oldhp)
		u.uconduct.weaphit++;
	} else
	    /* Now you know that nothing is there... */
	    pline(nothing_happens);
	return (1);
}

STATIC_OVL int
use_cream_pie(obj)
struct obj *obj;
{
	boolean wasblind = Blind;
	boolean wascreamed = u.ucreamed;
	boolean several = FALSE;

	if (obj->quan > 1L) {
		several = TRUE;
		obj = splitobj(obj, 1L);
	}
	if (Hallucination)
/*JP
		You("give yourself a facial.");
*/
		You("自分の顔にクリームパックをした．");
	else
#if 0 /*JP*/
		pline("You immerse your %s in %s%s.", body_part(FACE),
			several ? "one of " : "",
			several ? makeplural(the(xname(obj))) : the(xname(obj)));
#else
		pline("%s%sに%sを沈めた．",
			xname(obj),
			several ? "のひとつ" : "", body_part(FACE));
#endif
	if(can_blnd((struct monst*)0, &youmonst, AT_WEAP, obj)) {
		int blindinc = rnd(25);
		u.ucreamed += blindinc;
		make_blinded(Blinded + (long)blindinc, FALSE);
		if (!Blind || (Blind && wasblind))
#if 0 /*JP*/
			pline("There's %ssticky goop all over your %s.",
				wascreamed ? "more " : "",
				body_part(FACE));
#else
			pline("やっかいなねばねばが%s全体に%sついた．",
				body_part(FACE), wascreamed ? "さらに" : "");
#endif
		else /* Blind  && !wasblind */
#if 0 /*JP*/
			You_cant("see through all the sticky goop on your %s.",
				body_part(FACE));
#else
			pline("やっかいなねばねばが%s全体について何も見えなくなった．",
				body_part(FACE));
#endif
	}
	if (obj->unpaid) {
/*JP
		verbalize("You used it, you bought it!");
*/
		verbalize("使ったのなら買ってもらうよ！");
		bill_dummy_object(obj);
	}
	obj_extract_self(obj);
	delobj(obj);
	return(0);
}

STATIC_OVL int
use_grapple (obj)
	struct obj *obj;
{
	int res = 0, typ, max_range = 4, tohit;
	coord cc;
	struct monst *mtmp;
	struct obj *otmp;

	/* Are you allowed to use the hook? */
	if (u.uswallow) {
	    pline(not_enough_room);
	    return (0);
	}
	if (obj != uwep) {
	    if (!wield_tool(obj, "cast")) return(0);
	    else res = 1;
	}
     /* assert(obj == uwep); */

	/* Prompt for a location */
	pline(where_to_hit);
	cc.x = u.ux;
	cc.y = u.uy;
/*JP
	if (getpos(&cc, TRUE, "the spot to hit") < 0)
*/
	if (getpos(&cc, TRUE, "狙う場所") < 0)
	    return 0;	/* user pressed ESC */

	/* Calculate range */
	typ = uwep_skill_type();
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
	else if (P_SKILL(typ) == P_SKILLED) max_range = 5;
	else max_range = 8;
	if (distu(cc.x, cc.y) > max_range) {
/*JP
		pline("Too far!");
*/
		pline("遠すぎる！");
		return (res);
	} else if (!cansee(cc.x, cc.y)) {
		You(cant_see_spot);
		return (res);
	}

	/* What do you want to hit? */
	tohit = rn2(5);
	if (typ != P_NONE && P_SKILL(typ) >= P_SKILLED) {
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    anything any;
	    char buf[BUFSZ];
	    menu_item *selected;

	    any.a_void = 0;	/* set all bits to zero */
	    any.a_int = 1;	/* use index+1 (cant use 0) as identifier */
	    start_menu(tmpwin);
	    any.a_int++;
/*JP
	    Sprintf(buf, "an object on the %s", surface(cc.x, cc.y));
*/
	    Sprintf(buf, "%sにある物体", surface(cc.x, cc.y));
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	    any.a_int++;
#if 0 /*JP:T*/
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			"a monster", MENU_UNSELECTED);
#else
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			"怪物", MENU_UNSELECTED);
#endif
	    any.a_int++;
#if 0 /*JP*/
	    Sprintf(buf, "the %s", surface(cc.x, cc.y));
#else
	    Sprintf(buf, "%s", surface(cc.x, cc.y));
#endif
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
/*JP
	    end_menu(tmpwin, "Aim for what?");
*/
	    end_menu(tmpwin, "何を狙う？");
	    tohit = rn2(4);
	    if (select_menu(tmpwin, PICK_ONE, &selected) > 0 &&
			rn2(P_SKILL(typ) > P_SKILLED ? 20 : 2))
		tohit = selected[0].item.a_int - 1;
	    free((genericptr_t)selected);
	    destroy_nhwindow(tmpwin);
	}

	/* What did you hit? */
	switch (tohit) {
	case 0:	/* Trap */
	    /* FIXME -- untrap needs to deal with non-adjacent traps */
	    break;
	case 1:	/* Object */
	    if ((otmp = level.objects[cc.x][cc.y]) != 0) {
/*JP
		You("snag an object from the %s!", surface(cc.x, cc.y));
*/
		You("%sのものを引っ掛けた！", surface(cc.x, cc.y));
		(void) pickup_object(otmp, 1L, FALSE);
		/* If pickup fails, leave it alone */
		newsym(cc.x, cc.y);
		return (1);
	    }
	    break;
	case 2:	/* Monster */
	    if ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0) break;
	    if (verysmall(mtmp->data) && !rn2(4) &&
			enexto(&cc, u.ux, u.uy, (struct permonst *)0)) {
/*JP
		You("pull in %s!", mon_nam(mtmp));
*/
	    	You("%sを引っ張った！", mon_nam(mtmp));
		mtmp->mundetected = 0;
		rloc_to(mtmp, cc.x, cc.y);
		return (1);
	    } else if ((!bigmonst(mtmp->data) && !strongmonst(mtmp->data)) ||
		       rn2(4)) {
		(void) thitmonst(mtmp, uwep, 1);
		return (1);
	    }
	    /* FALL THROUGH */
	case 3:	/* Surface */
	    if (IS_AIR(levl[cc.x][cc.y].typ) || is_pool(cc.x, cc.y))
/*JP
		pline_The("hook slices through the %s.", surface(cc.x, cc.y));
*/
		pline("フックは%sをさっと切った．", surface(cc.x, cc.y));
	    else {
/*JP
		You("are yanked toward the %s!", surface(cc.x, cc.y));
*/
		You("%sへ引っぱられた！", surface(cc.x, cc.y));
		hurtle(sgn(cc.x-u.ux), sgn(cc.y-u.uy), 1, FALSE);
		spoteffects(TRUE);
	    }
	    return (1);
	default:	/* Yourself (oops!) */
	    if (P_SKILL(typ) <= P_BASIC) {
/*JP
		You("hook yourself!");
*/
		You("自分自身を引っ掛けた！");
/*JP
		losehp(rn1(10,10), "a grappling hook", KILLED_BY);
*/
		losehp(rn1(10,10), "自分自身を引っ掛けて", KILLED_BY);
		return (1);
	    }
	    break;
	}
	pline(nothing_happens);
	return (1);
}


#define BY_OBJECT	((struct monst *)0)

/* return 1 if the wand is broken, hence some time elapsed */
STATIC_OVL int
do_break_wand(obj)
    struct obj *obj;
{
    char confirm[QBUFSZ], the_wand[BUFSZ];

    Strcpy(the_wand, yname(obj));
#if 0 /*JP*/
    Sprintf(confirm, "Are you really sure you want to break %s?",
	safe_qbuf("", sizeof("Are you really sure you want to break ?"),
				the_wand, ysimple_name(obj), "the wand"));
#else
    Sprintf(confirm, "本当に%sを壊すの？",
	safe_qbuf("", sizeof("本当にを壊すの？"),
				the_wand, ysimple_name(obj), "杖"));
#endif
    if (yn(confirm) == 'n' ) return 0;

    if (nohands(youmonst.data)) {
#if 0 /*JP*/
	You_cant("break %s without hands!", the_wand);
#else
	You("手がないので%sを壊せない！", the_wand);
#endif
	return 0;
    } else if (ACURR(A_STR) < 10) {
#if 0 /*JP*/
	You("don't have the strength to break %s!", the_wand);
#else
	You("%sを壊すだけの力がない！", the_wand);
#endif
	return 0;
    }
#if 0 /*JP*/
    pline("Raising %s high above your %s, you break it in two!",
	  the_wand, body_part(HEAD));
#else
    pline("%sを%sの上にかかげ二つにへし折った！",
	  the_wand, body_part(HEAD));
#endif
    return wand_explode(obj, TRUE);
}

/* This function takes care of the effects wands exploding, via
 * user-specified 'applying' as well as wands exploding by accident
 * during use (called by backfire() in zap.c)
 *
 * If the effect is directly recognisable as pertaining to a 
 * specific wand, the wand should be makeknown()
 * Otherwise, if there is an ambiguous or indirect but visible effect
 * the wand should be allowed to be named by the user.
 *
 * If there is no obvious effect,  do nothing. (Should this be changed
 * to letting the user call that type of wand?)
 *
 * hero_broke is nonzero if the user initiated the action that caused
 * the wand to explode (zapping or applying).
 */
int
wand_explode(obj, hero_broke)
    struct obj *obj;
    boolean hero_broke;
{
/*JP
    static const char nothing_else_happens[] = "But nothing else happens...";
*/
    static const char nothing_else_happens[] = "しかし，何も起きなかった．．．";
    register int i, x, y;
    register struct monst *mon;
    int dmg, damage;
    boolean affects_objects;
    boolean shop_damage = FALSE;
    int expltype = EXPL_MAGICAL;
    char buf[BUFSZ];

    /* [ALI] Do this first so that wand is removed from bill. Otherwise,
     * the freeinv() below also hides it from setpaid() which causes problems.
     */
    if (carried(obj) ? obj->unpaid :
	    !obj->no_charge && costly_spot(obj->ox, obj->oy)) {
	if (hero_broke)
	check_unpaid(obj);		/* Extra charge for use */
	bill_dummy_object(obj);
    }

    current_wand = obj;		/* destroy_item might reset this */
    freeinv(obj);		/* hide it from destroy_item instead... */
    setnotworn(obj);		/* so we need to do this ourselves */

    if (obj->spe <= 0) {
	pline(nothing_else_happens);
	goto discard_broken_wand;
    }
    obj->ox = u.ux;
    obj->oy = u.uy;
    dmg = obj->spe * 4;
    affects_objects = FALSE;

    switch (obj->otyp) {
    case WAN_WISHING:
    case WAN_NOTHING:
    case WAN_LOCKING:
    case WAN_PROBING:
    case WAN_ENLIGHTENMENT:
    case WAN_OPENING:
    case WAN_SECRET_DOOR_DETECTION:
	pline(nothing_else_happens);
	goto discard_broken_wand;
    case WAN_DEATH:
    case WAN_LIGHTNING:
	dmg *= 4;
	goto wanexpl;
    case WAN_COLD:
	expltype = EXPL_FROSTY;
	dmg *= 2;
    case WAN_MAGIC_MISSILE:
    wanexpl:
	explode(u.ux, u.uy, ZT_MAGIC_MISSILE, dmg, WAND_CLASS, expltype);
	makeknown(obj->otyp);	/* explode described the effect */
	goto discard_broken_wand;
/*WAC for wands of fireball- no double damage
 * As well, effect is the same as fire, so no makeknown
 */
    case WAN_FIRE:
	dmg *= 2;
    case WAN_FIREBALL:
	expltype = EXPL_FIERY;
        explode(u.ux, u.uy, ZT_FIRE, dmg, WAND_CLASS, expltype);
	if (obj->dknown && !objects[obj->otyp].oc_name_known &&
		!objects[obj->otyp].oc_uname)
        docall(obj);
	goto discard_broken_wand;
    case WAN_STRIKING:
	/* we want this before the explosion instead of at the very end */
/*JP
	pline("A wall of force smashes down around you!");
*/
	pline("魔力の壁があなたの周囲をなぎ倒した！");
	dmg = d(1 + obj->spe,6);	/* normally 2d12 */
    case WAN_CANCELLATION:
    case WAN_POLYMORPH:
    case WAN_UNDEAD_TURNING:
    case WAN_DRAINING:	/* KMH */
	affects_objects = TRUE;
	break;
    case WAN_TELEPORTATION:
		/* WAC make tele trap if you broke a wand of teleport */
		/* But make sure the spot is valid! */
	    if ((obj->spe > 2) && rn2(obj->spe - 2) && !level.flags.noteleport &&
		    !u.uswallow && !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
		    !IS_ROCK(levl[u.ux][u.uy].typ) &&
		    !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {

			struct trap *ttmp;

			ttmp = maketrap(u.ux, u.uy, TELEP_TRAP);
			if (ttmp) {
				ttmp->madeby_u = 1;
				newsym(u.ux, u.uy); /* if our hero happens to be invisible */
				if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
					/* shopkeeper will remove it */
					add_damage(u.ux, u.uy, 0L);             
				}
			}
		}
	affects_objects = TRUE;
	break;
    case WAN_CREATE_HORDE: /* More damage than Create monster */
	        dmg *= 2;
	        break;
    case WAN_HEALING:
    case WAN_EXTRA_HEALING:
		dmg = 0;
		break;
    default:
	break;
    }

    /* magical explosion and its visual effect occur before specific effects */
    explode(obj->ox, obj->oy, ZT_MAGIC_MISSILE, dmg ? rnd(dmg) : 0, WAND_CLASS,
	    EXPL_MAGICAL);

    /* this makes it hit us last, so that we can see the action first */
    for (i = 0; i <= 8; i++) {
	bhitpos.x = x = obj->ox + xdir[i];
	bhitpos.y = y = obj->oy + ydir[i];
	if (!isok(x,y)) continue;

	if (obj->otyp == WAN_DIGGING) {
	    if(dig_check(BY_OBJECT, FALSE, x, y)) {
		if (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)) {
		    /* normally, pits and holes don't anger guards, but they
		     * do if it's a wall or door that's being dug */
		    watch_dig((struct monst *)0, x, y, TRUE);
		    if (*in_rooms(x,y,SHOPBASE)) shop_damage = TRUE;
		}		    
		digactualhole(x, y, BY_OBJECT,
			      (rn2(obj->spe) < 3 || !Can_dig_down(&u.uz)) ?
			       PIT : HOLE);
	    }
	    continue;
/* WAC catch Create Horde wands too */
/* MAR make the monsters around you */
	} else if(obj->otyp == WAN_CREATE_MONSTER
                || obj->otyp == WAN_CREATE_HORDE) {
	    /* u.ux,u.uy creates it near you--x,y might create it in rock */
	    (void) makemon((struct permonst *)0, u.ux, u.uy, NO_MM_FLAGS);
	    continue;
	} else {
	    if (x == u.ux && y == u.uy) {
		/* teleport objects first to avoid race with tele control and
		   autopickup.  Other wand/object effects handled after
		   possible wand damage is assessed */
		if (obj->otyp == WAN_TELEPORTATION &&
		    affects_objects && level.objects[x][y]) {
		    (void) bhitpile(obj, bhito, x, y);
		    if (flags.botl) bot();		/* potion effects */
			/* makeknown is handled in zapyourself */
		}
		damage = zapyourself(obj, FALSE);
		if (damage) {
		    if (hero_broke) {
#if 0 /*JP*/
		    Sprintf(buf, "killed %sself by breaking a wand", uhim());
		    losehp(damage, buf, NO_KILLER_PREFIX);
#else
		    Sprintf(buf, "自分自身で杖を壊してダメージを受け");
		    losehp(damage, buf, KILLED_BY);
#endif
		    } else
/*JP
			losehp(damage, "exploding wand", KILLED_BY_AN);
*/
			losehp(damage, "杖の爆発で", KILLED_BY_AN);
		}
		if (flags.botl) bot();		/* blindness */
	    } else if ((mon = m_at(x, y)) != 0 && !DEADMONSTER(mon)) {
		(void) bhitm(mon, obj);
	     /* if (flags.botl) bot(); */
	    }
	    if (affects_objects && level.objects[x][y]) {
		(void) bhitpile(obj, bhito, x, y);
		if (flags.botl) bot();		/* potion effects */
	    }
	}
    }

    /* Note: if player fell thru, this call is a no-op.
       Damage is handled in digactualhole in that case */
/*JP
    if (shop_damage) pay_for_damage("dig into", FALSE);
*/
    if (shop_damage) pay_for_damage("穴をあける", FALSE);

    if (obj->otyp == WAN_LIGHT)
	litroom(TRUE, obj);	/* only needs to be done once */

 discard_broken_wand:
    obj = current_wand;		/* [see dozap() and destroy_item()] */
    current_wand = 0;
    if (obj)
	delobj(obj);
    nomul(0);
    return 1;
}

STATIC_OVL boolean
uhave_graystone()
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(is_graystone(otmp))
			return TRUE;
	return FALSE;
}

STATIC_OVL void
add_class(cl, class)
char *cl;
char class;
{
	char tmp[2];
	tmp[0] = class;
	tmp[1] = '\0';
	Strcat(cl, tmp);
}

int
doapply()
{
	struct obj *obj;
	register int res = 1;
	register boolean can_use = FALSE;
	char class_list[MAXOCLASSES+2];

	if(check_capacity((char *)0)) return (0);

	if (carrying(POT_OIL) || uhave_graystone())
		Strcpy(class_list, tools_too);
	else
		Strcpy(class_list, tools);
	if (carrying(CREAM_PIE) || carrying(EUCALYPTUS_LEAF))
		add_class(class_list, FOOD_CLASS);

	obj = getobj(class_list, "use or apply");
	if(!obj) return 0;

	if (obj->oartifact && !touch_artifact(obj, &youmonst))
	    return 1;	/* evading your grasp costs a turn; just be
			   grateful that you don't drop it as well */

	if (obj->oclass == WAND_CLASS)
	    return do_break_wand(obj);

	switch(obj->otyp){
	case BLINDFOLD:
	case LENSES:
		if (obj == ublindf) {
		    if (!cursed(obj)) Blindf_off(obj);
		} else if (!ublindf)
		    Blindf_on(obj);
#if 0 /*JP*/
		else You("are already %s.",
			ublindf->otyp == TOWEL ?     "covered by a towel" :
			ublindf->otyp == BLINDFOLD ? "wearing a blindfold" :
						     "wearing lenses");
#else
		else You("もう%sている．",
			ublindf->otyp == TOWEL ?     "タオルを巻い" : 
			ublindf->otyp == BLINDFOLD ? "目隠しをつけ" :
						     "レンズをつけ");
#endif
		break;
	case CREAM_PIE:
		res = use_cream_pie(obj);
		break;
	case BULLWHIP:
		res = use_whip(obj);
		break;
	case GRAPPLING_HOOK:
		res = use_grapple(obj);
		break;
	case LARGE_BOX:
	case CHEST:
	case ICE_BOX:
	case SACK:
	case BAG_OF_HOLDING:
	case OILSKIN_SACK:
		res = use_container(&obj, 1);
		break;
	case BAG_OF_TRICKS:
		bagotricks(obj);
		break;
	case CAN_OF_GREASE:
		use_grease(obj);
		break;
#ifdef TOURIST
	case CREDIT_CARD:
#endif
	case LOCK_PICK:
	case SKELETON_KEY:
		(void) pick_lock(&obj);
		break;
	case PICK_AXE:
	case DWARVISH_MATTOCK: /* KMH, balance patch -- the mattock is a pick, too */
		res = use_pick_axe(obj);
		break;
	case FISHING_POLE:
		res = use_pole(obj);
		break;
	case TINNING_KIT:
		use_tinning_kit(obj);
		break;
	case LEASH:
		use_leash(obj);
		break;
#ifdef STEED
	case SADDLE:
		res = use_saddle(obj);
		break;
#endif
	case MAGIC_WHISTLE:
		use_magic_whistle(obj);
		break;
	case TIN_WHISTLE:
		use_whistle(obj);
		break;
	case EUCALYPTUS_LEAF:
		/* MRKR: Every Australian knows that a gum leaf makes an */
		/*	 excellent whistle, especially if your pet is a  */
		/*	 tame kangaroo named Skippy.			 */
		if (obj->blessed) {
		    use_magic_whistle(obj);
		    /* sometimes the blessing will be worn off */
		    if (!rn2(49)) {
			if (!Blind) {
			    char buf[BUFSZ];

#if 0 /*JP*/
			    pline("%s %s %s.", Shk_Your(buf, obj),
				  aobjnam(obj, "glow"), hcolor("brown"));
#else
			    pline("%s%sは%s輝いた．", Shk_Your(buf, obj), xname(obj),
				  jconj_adj(hcolor("茶色の")));
#endif
			    obj->bknown = 1;
			}
			unbless(obj);
		    }
		} else {
		    use_whistle(obj);
		}
		break;
	case STETHOSCOPE:
		res = use_stethoscope(obj);
		break;
	case MIRROR:
		res = use_mirror(obj);
		break;
# ifdef P_SPOON
	case SPOON:
/*JP
		pline("It's a finely crafted antique spoon; what do you want to do with it?");
*/
		pline("それは素晴らしく巧妙に作られたアンティークなスプーンだ．あなたは何に使いたいですか？");
		break;
# endif /* P_SPOON */
	case BELL:
	case BELL_OF_OPENING:
		use_bell(&obj);
		break;
	case CANDELABRUM_OF_INVOCATION:
		use_candelabrum(obj);
		break;
	case WAX_CANDLE:
/* STEPHEN WHITE'S NEW CODE */           
	case MAGIC_CANDLE:
	case TALLOW_CANDLE:
		use_candle(&obj);
		break;
#ifdef LIGHTSABERS
	case GREEN_LIGHTSABER:
#ifdef D_SABER
  	case BLUE_LIGHTSABER:
#endif
	case RED_LIGHTSABER:
	case RED_DOUBLE_LIGHTSABER:
		if (uwep != obj && !wield_tool(obj, (const char *)0)) break;
		/* Fall through - activate via use_lamp */
#endif
	case OIL_LAMP:
	case MAGIC_LAMP:
	case BRASS_LANTERN:
		use_lamp(obj);
		break;
	case TORCH:
	        res = use_torch(obj);
		break;
	case POT_OIL:
		light_cocktail(obj);
		break;
#ifdef TOURIST
	case EXPENSIVE_CAMERA:
		res = use_camera(obj);
		break;
#endif
	case TOWEL:
		res = use_towel(obj);
		break;
	case CRYSTAL_BALL:
		use_crystal_ball(obj);
		break;
/* STEPHEN WHITE'S NEW CODE */
/* KMH, balance patch -- source of abuse */
#if 0
	case ORB_OF_ENCHANTMENT:
	    if(obj->spe > 0) {
		
		check_unpaid(obj);
		if(uwep && (uwep->oclass == WEAPON_CLASS ||
			    uwep->otyp == PICK_AXE ||
			    uwep->otyp == UNICORN_HORN)) {
		if (uwep->spe < 5) {
		if (obj->blessed) {
				if (!Blind) pline("Your %s glows silver.",xname(uwep));
				uwep->spe += rnd(2);
		} else if (obj->cursed) {                               
				if (!Blind) pline("Your %s glows black.",xname(uwep));
				uwep->spe -= rnd(2);
		} else {
				if (rn2(3)) {
					if (!Blind) pline("Your %s glows bright for a moment." ,xname(uwep));
					uwep->spe += 1;
				} else {
					if (!Blind) pline("Your %s glows dark for a moment." ,xname(uwep));
					uwep->spe -= 1;
				}
		}
		} else pline("Nothing seems to happen.");                
		
		if (uwep->spe > 5) uwep->spe = 5;
				
		} else pline("The orb glows for a moment, then fades.");
		consume_obj_charge(obj, FALSE);
	    
	    } else pline("This orb is burnt out.");
	    break;
	case ORB_OF_CHARGING:
		if(obj->spe > 0) {
			register struct obj *otmp;
			makeknown(ORB_OF_CHARGING);
			consume_obj_charge(obj, TRUE);
			otmp = getobj(all_count, "charge");
			if (!otmp) break;
			recharge(otmp, obj->cursed ? -1 : (obj->blessed ? 1 : 0));
		} else pline("This orb is burnt out.");
		break;
	case ORB_OF_DESTRUCTION:
		useup(obj);
		pline("As you activate the orb, it explodes!");
		explode(u.ux, u.uy, ZT_SPELL(ZT_MAGIC_MISSILE), d(12,6), WAND_CLASS);
		check_unpaid(obj);
		break;
#endif
	case MAGIC_MARKER:
		res = dowrite(obj);
		break;
	case TIN_OPENER:
		if(!carrying(TIN)) {
/*JP
			You("have no tin to open.");
*/
			You("缶を持っていない．");
			goto xit;
		}
/*JP
		You("cannot open a tin without eating or discarding its contents.");
*/
		pline("中身を食べるか，捨てるかしないと缶を空にできない．");
		if(flags.verbose)
/*JP
			pline("In order to eat, use the 'e' command.");
*/
			pline("食べるには，'e'コマンドを使えばよい．");
		if(obj != uwep)
/*JP
    pline("Opening the tin will be much easier if you wield the tin opener.");
*/
    pline("缶切りを装備していれば，ずっと簡単に開けることができる．");
		goto xit;

	case FIGURINE:
		use_figurine(&obj);
		break;
	case UNICORN_HORN:
		use_unicorn_horn(obj);
		break;
	case WOODEN_FLUTE:
	case MAGIC_FLUTE:
	case TOOLED_HORN:
	case FROST_HORN:
	case FIRE_HORN:
	case WOODEN_HARP:
	case MAGIC_HARP:
	case BUGLE:
	case LEATHER_DRUM:
	case DRUM_OF_EARTHQUAKE:
	/* KMH, balance patch -- removed
	case PAN_PIPE_OF_SUMMONING:                
	case PAN_PIPE_OF_THE_SEWERS:
	case PAN_PIPE:*/
		res = do_play_instrument(obj);
		break;
	case MEDICAL_KIT:        
		if (Role_if(PM_HEALER)) can_use = TRUE;
		else if ((Role_if(PM_PRIEST) || Role_if(PM_MONK) ||
			Role_if(PM_UNDEAD_SLAYER) || Role_if(PM_SAMURAI)) &&
			!rn2(2)) can_use = TRUE;
		else if(!rn2(4)) can_use = TRUE;

		if (obj->cursed && rn2(3)) can_use = FALSE;
		if (obj->blessed && rn2(3)) can_use = TRUE;  

		makeknown(MEDICAL_KIT);
		if (obj->cobj) {
		    struct obj *otmp;
		    for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
			if (otmp->otyp == PILL)
			    break;
		    if (!otmp)
#if 0 /*JP*/
			You_cant("find any more pills in %s.", yname(obj));
#else
			pline("%sの中には錠剤がまったく見つからない．", yname(obj));
#endif
		    else if (!is_edible(otmp))
#if 0 /*JP*/
			You("find, but cannot eat, a white pill in %s.",
			  yname(obj));
#else
			You("%sの中にあった白い錠剤は服用できそうもない．",
			  yname(obj));
#endif
		    else {
			check_unpaid(obj);
			if (otmp->quan > 1L) {
			    otmp->quan--;
			    obj->owt = weight(obj);
			} else {
			    obj_extract_self(otmp);
			    obfree(otmp, (struct obj *)0);
			}
			/*
			 * Note that while white and pink pills share the
			 * same otyp value, they are quite different.
			 */
#if 0 /*JP*/
			You("take a white pill from %s and swallow it.",
#else
			You("%sから白い錠剤を取り出して飲み込んだ．",
#endif
				yname(obj));
			if (can_use) {
			    if (Sick) make_sick(0L, (char *) 0,TRUE ,SICK_ALL);
			    else if (Blinded > (long)(u.ucreamed+1))
				make_blinded(u.ucreamed ?
					(long)(u.ucreamed+1) : 0L, TRUE);
			    else if (HHallucination)
				make_hallucinated(0L, TRUE, 0L);
			    else if (Vomiting) make_vomiting(0L, TRUE);
			    else if (HConfusion) make_confused(0L, TRUE);
			    else if (HStun) make_stunned(0L, TRUE);
			    else if (u.uhp < u.uhpmax) {
				u.uhp += rn1(10,10);
				if (u.uhp > u.uhpmax) u.uhp = u.uhpmax;
#if 0 /*JP*/
				You_feel("better.");
#else
				You("よくなった気がする．");
#endif
				flags.botl = TRUE;
			    } else pline(nothing_happens);
			} else if (!rn2(3))
#if 0 /*JP*/
			    pline("Nothing seems to happen.");
#else
			    pline("何も起こらなかったようだ．");
#endif
			else if (!Sick)
#if 0 /*JP*/
			    make_sick(rn1(10,10), "bad pill", TRUE,
			      SICK_VOMITABLE);
#else
			    make_sick(rn1(10,10), "毒性のある錠剤", TRUE,
			      SICK_VOMITABLE);
#endif
			else {
#if 0 /*JP*/
			    You("seem to have made your condition worse!");
			    losehp(rn1(10,10), "a drug overdose", KILLED_BY);
#else
			    You("体調がますます悪くなった気がする！");
			    losehp(rn1(10,10), "薬の飲みすぎで", KILLED_BY);
#endif
			}
		    }
#if 0 /*JP*/
		} else You("seem to be out of medical supplies");
#else
		} else pline("医療用品は入っていないようだ．");
#endif
		break;
	case HORN_OF_PLENTY:	/* not a musical instrument */
		if (obj->spe > 0) {
		    struct obj *otmp;
		    const char *what;

		    consume_obj_charge(obj, TRUE);
		    if (!rn2(13)) {
			otmp = mkobj(POTION_CLASS, FALSE);
			/* KMH, balance patch -- rewritten */
			while ((otmp->otyp == POT_SICKNESS) ||
					objects[otmp->otyp].oc_magic)
			    otmp->otyp = rnd_class(POT_BOOZE, POT_WATER);
/*JP
			what = "A potion";
*/
			what = "薬";
		    } else {
			otmp = mkobj(FOOD_CLASS, FALSE);
			if (otmp->otyp == FOOD_RATION && !rn2(7))
			    otmp->otyp = LUMP_OF_ROYAL_JELLY;
/*JP
			what = "Some food";
*/
			what = "食べ物";
		    }
/*JP
		    pline("%s spills out.", what);
*/
		    pline("%sが飛び出てきた．", what);
		    otmp->blessed = obj->blessed;
		    otmp->cursed = obj->cursed;
		    otmp->owt = weight(otmp);
#if 0 /*JP*/
		    otmp = hold_another_object(otmp, u.uswallow ?
				       "Oops!  %s out of your reach!" :
					(Is_airlevel(&u.uz) ||
					 Is_waterlevel(&u.uz) ||
					 levl[u.ux][u.uy].typ < IRONBARS ||
					 levl[u.ux][u.uy].typ >= ICE) ?
					       "Oops!  %s away from you!" :
					       "Oops!  %s to the floor!",
					       The(aobjnam(otmp, "slip")),
					       (const char *)0);
#else
		    otmp = hold_another_object(otmp, u.uswallow ?
				       "おっと！%sを取りそこねた！" :
					(Is_airlevel(&u.uz) ||
					 Is_waterlevel(&u.uz) ||
					 levl[u.ux][u.uy].typ < IRONBARS ||
					 levl[u.ux][u.uy].typ >= ICE) ?
					       "おっと！%sはあなたを離れて漂っていった！" :
					       "おっと！%sを床に落とした！",
					       xname(otmp), (const char *)0);
#endif
		    makeknown(HORN_OF_PLENTY);
		} else
		    pline(nothing_happens);
		break;
	case LAND_MINE:
	case BEARTRAP:
		use_trap(obj);
		break;
	case FLINT:
	case LUCKSTONE:
	case LOADSTONE:
	case TOUCHSTONE:
	case HEALTHSTONE:
	case WHETSTONE:
		use_stone(obj);
		break;
#ifdef FIREARMS
	case ASSAULT_RIFLE:
		/* Switch between WP_MODE_SINGLE, WP_MODE_BURST and WP_MODE_AUTO */

		if (obj->altmode == WP_MODE_AUTO) {
			obj->altmode = WP_MODE_BURST;
		} else if (obj->altmode == WP_MODE_BURST) {
			obj->altmode = WP_MODE_SINGLE;
		} else {
			obj->altmode = WP_MODE_AUTO;
		}
		
#if 0 /*JP*/
		You("switch %s to %s mode.", yname(obj), 
			((obj->altmode == WP_MODE_SINGLE) ? "single shot" : 
			 ((obj->altmode == WP_MODE_BURST) ? "burst" :
			  "full automatic")));
#else
		You("%sのセレクターを%sモードに変更した．", yname(obj), 
			((obj->altmode == WP_MODE_SINGLE) ? "単射" : 
			 ((obj->altmode == WP_MODE_BURST) ? "制限点射" :
			  "連射")));
#endif
		break;	
	case AUTO_SHOTGUN:
	case SUBMACHINE_GUN:		
		if (obj->altmode == WP_MODE_AUTO) obj-> altmode = WP_MODE_SINGLE;
		else obj->altmode = WP_MODE_AUTO;
#if 0 /*JP*/
		You("switch %s to %s mode.", yname(obj), 
			(obj->altmode ? "semi-automatic" : "full automatic"));
#else
		You("%sのセレクターを%sモードに変更した．", yname(obj), 
			(obj->altmode ? "単射" : "連射"));
#endif
		break;
	case FRAG_GRENADE:
	case GAS_GRENADE:
		if (!obj->oarmed) {
#if 0 /*JP*/
			You("arm %s.", yname(obj));
#else
			You("%sの安全ピンを抜いた．", yname(obj));
#endif
			arm_bomb(obj, TRUE);
#if 0 /*JP*/
		} else pline("It's already armed!");
#else
		} else pline("既に秒読み段階に入っている！");
#endif
		break;
	case STICK_OF_DYNAMITE:
		light_cocktail(obj);
		break;
#endif
	default:
		/* KMH, balance patch -- polearms can strike at a distance */
		if (is_pole(obj)) {
			res = use_pole(obj);
			break;
		} else if (is_pick(obj) || is_axe(obj)) {
			res = use_pick_axe(obj);
			break;
		}
/*JP
		pline("Sorry, I don't know how to use that.");
*/
		pline("残念．使い方がわからない．");
	xit:
		nomul(0);
		return 0;
	}
	if (res && obj && obj->oartifact) arti_speak(obj);
	nomul(0);
	return res;
}

/* Keep track of unfixable troubles for purposes of messages saying you feel
 * great.
 */
int
unfixable_trouble_count(is_horn)
	boolean is_horn;
{
	int unfixable_trbl = 0;

	if (Stoned) unfixable_trbl++;
	if (Strangled) unfixable_trbl++;
	if (Wounded_legs
#ifdef STEED
		    && !u.usteed
#endif
				) unfixable_trbl++;
	if (Slimed) unfixable_trbl++;
	/* lycanthropy is not desirable, but it doesn't actually make you feel
	   bad */

	/* we'll assume that intrinsic stunning from being a bat/stalker
	   doesn't make you feel bad */
	if (!is_horn) {
	    if (Confusion) unfixable_trbl++;
	    if (Sick) unfixable_trbl++;
	    if (HHallucination) unfixable_trbl++;
	    if (Vomiting) unfixable_trbl++;
	    if (HStun) unfixable_trbl++;
	}
	return unfixable_trbl;
}

#endif /* OVLB */

/*apply.c*/
