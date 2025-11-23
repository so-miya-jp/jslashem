/*	SCCS Id: @(#)spell.c	3.4	2003/01/17	*/
/*	Copyright (c) M. Stephenson 1988			  */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, Shiganai Sakusha, 2004-2007
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"
#include "edog.h"

/* Are now ints */
static NEARDATA int delay;            /* moves left for this spell */
static NEARDATA int end_delay;        /* when to stop studying */
static NEARDATA struct obj *book;	/* last/current book being xscribed */

/* spellmenu arguments; 0 thru n-1 used as spl_book[] index when swapping */
#define SPELLMENU_CAST (-2)
#define SPELLMENU_VIEW (-1)

#define KEEN 		10000	/* memory increase reading the book */
#define CAST_BOOST 	  500	/* memory increase for successful casting */
#define MAX_KNOW 	70000	/* Absolute Max timeout */
#define MAX_CAN_STUDY 	60000	/* Can study while timeout is less than */

#define MAX_STUDY_TIME 	  300	/* Max time for one study session */
#define MAX_SPELL_STUDY    30	/* Uses before spellbook crumbles */

#define spellknow(spell)	spl_book[spell].sp_know 

#define incrnknow(spell)        spl_book[spell].sp_know = ((spl_book[spell].sp_know < 1) ? KEEN \
				 : ((spl_book[spell].sp_know + KEEN) > MAX_KNOW) ? MAX_KNOW \
				 : spl_book[spell].sp_know + KEEN)
#define boostknow(spell,boost)  spl_book[spell].sp_know = ((spl_book[spell].sp_know + boost > MAX_KNOW) ? MAX_KNOW \
				 : spl_book[spell].sp_know + boost)

#define spellev(spell)		spl_book[spell].sp_lev
#define spellid(spell)          spl_book[spell].sp_id
#define spellname(spell)	OBJ_NAME(objects[spellid(spell)])
#define spellet(spell)	\
	((char)((spell < 26) ? ('a' + spell) : \
	        (spell < 52) ? ('A' + spell - 26) : \
		(spell < 62) ? ('0' + spell - 52) : 0 ))

STATIC_DCL int FDECL(spell_let_to_idx, (CHAR_P));
STATIC_DCL boolean FDECL(cursed_book, (struct obj *bp));
STATIC_DCL boolean FDECL(confused_book, (struct obj *));
STATIC_DCL void FDECL(deadbook, (struct obj *));
STATIC_PTR int NDECL(learn);
STATIC_DCL void NDECL(do_reset_learn);
STATIC_DCL boolean FDECL(getspell, (int *));
STATIC_DCL boolean FDECL(dospellmenu, (const char *,int,int *));
STATIC_DCL int FDECL(percent_success, (int));
STATIC_DCL void NDECL(cast_protection);
STATIC_DCL void FDECL(spell_backfire, (int));
STATIC_DCL const char *FDECL(spelltypemnemonic, (int));
STATIC_DCL int FDECL(isqrt, (int));

/* The roles[] table lists the role-specific values for tuning
 * percent_success().
 *
 * Reasoning:
 *   splcaster, special:
 *	Arc are aware of magic through historical research
 *	Bar abhor magic (Conan finds it "interferes with his animal instincts")
 *	Cav are ignorant to magic
 *	Hea are very aware of healing magic through medical research
 *	Kni are moderately aware of healing from Paladin training
 *	Mon use magic to attack and defend in lieu of weapons and armor
 *	Pri are very aware of healing magic through theological research
 *	Ran avoid magic, preferring to fight unseen and unheard
 *	Rog are moderately aware of magic through trickery
 *	Sam have limited magical awareness, prefering meditation to conjuring
 *	Tou are aware of magic from all the great films they have seen
 *	Val have limited magical awareness, prefering fighting
 *	Wiz are trained mages
 *
 *	The arms penalty is lessened for trained fighters Bar, Kni, Ran,
 *	Sam, Val -
 *	the penalty is its metal interference, not encumbrance.
 *	The `spelspec' is a single spell which is fundamentally easier
 *	 for that role to cast.
 *
 *  spelspec, spelsbon:
 *	Arc map masters (SPE_MAGIC_MAPPING)
 *	Bar fugue/berserker (SPE_HASTE_SELF)
 *	Cav born to dig (SPE_DIG)
 *	Hea to heal (SPE_CURE_SICKNESS)
 *	Kni to turn back evil (SPE_TURN_UNDEAD)
 *	Mon to preserve their abilities (SPE_RESTORE_ABILITY)
 *	Pri to bless (SPE_REMOVE_CURSE)
 *	Ran to hide (SPE_INVISIBILITY)
 *	Rog to find loot (SPE_DETECT_TREASURE)
 *	Sam to be At One (SPE_CLAIRVOYANCE)
 *	Tou to smile (SPE_CHARM_MONSTER)
 *	Val control lightning (SPE_LIGHTNING)
 *	Wiz all really, but SPE_MAGIC_MISSILE is their party trick
 *	Yeo guard doors (SPE_KNOCK)
 *
 *	See percent_success() below for more comments.
 *
 *  uarmbon, uarmsbon, uarmhbon, uarmgbon, uarmfbon:
 *	Fighters find body armour & shield a little less limiting.
 *	Headgear, Gauntlets and Footwear are not role-specific (but
 *	still have an effect, except helm of brilliance, which is designed
 *	to permit magic-use).
 */

#define uarmhbon 4 /* Metal helmets interfere with the mind */
#define uarmgbon 6 /* Casting channels through the hands */
#define uarmfbon 2 /* All metal interferes to some degree */

/* since the spellbook itself doesn't blow up, don't say just "explodes" */
#if 0 /*JP*/
static const char explodes[] = "radiates explosive energy";
#endif

/* convert an alnum into a number in the range 0..61, or -1 if not an alnum */
STATIC_OVL int
spell_let_to_idx(ilet)
char ilet;
{
    int indx;

    indx = ilet - 'a';
    if (indx >= 0 && indx < 26) return indx;
    indx = ilet - 'A';
    if (indx >= 0 && indx < 26) return indx + 26;
    indx = ilet - '0';
    if (indx >= 0 && indx < 10) return indx + 52;
    return -1;
}

/* TRUE: book should be destroyed by caller */
STATIC_OVL boolean
cursed_book(bp)
	struct obj *bp;
{
	int lev = objects[bp->otyp].oc_level;

	switch(rn2(lev)) {
	case 0:
/*JP
		You_feel("a wrenching sensation.");
*/
		You("ねじられたような感覚をおぼえた．");
		tele();		/* teleport him */
		break;
	case 1:
/*JP
		You_feel("threatened.");
*/
		You("脅迫されているような気がした．");
		aggravate();
		break;
	case 2:
		/* [Tom] lowered this (used to be 100,250) */
		make_blinded(Blinded + rn1(50,25),TRUE);
		break;
	case 3:
		take_gold();
		break;
	case 4:
/*JP
		pline("These runes were just too much to comprehend.");
*/
		pline("このルーン文字を理解するのは困難だ．");
		make_confused(HConfusion + rn1(7,16),FALSE);
		break;
	case 5:
/*JP
		pline_The("book was coated with contact poison!");
*/
		pline("この本は接触型の毒で覆われている！");
		if (uarmg) {
		    if (uarmg->oerodeproof || !is_corrodeable(uarmg)) {
/*JP
			Your("gloves seem unaffected.");
*/
			pline("小手は影響を受けなかったようだ．");
		    } else if (uarmg->oeroded2 < MAX_ERODE) {
			if (uarmg->greased) {
/*JP
			    grease_protect(uarmg, "gloves", &youmonst);
*/
			    grease_protect(uarmg, "小手", &youmonst);
			} else {
#if 0 /*JP*/
			    Your("gloves corrode%s!",
				 uarmg->oeroded2+1 == MAX_ERODE ?
				 " completely" : uarmg->oeroded2 ?
				 " further" : "");
#else
			    pline("小手は%s腐食した！",
				  uarmg->oeroded2+1 == MAX_ERODE ?
				  "完全に" : uarmg->oeroded2 ?
				  "さらに" : "");
#endif
			    uarmg->oeroded2++;
			}
		    } else
#if 0 /*JP*/
			Your("gloves %s completely corroded.",
			     Blind ? "feel" : "look");
#else
			pline("小手は完全に腐食してしまっている%s．",
			     Blind ? "ようだ" : "ように見える");
#endif
		    break;
		}
		/* temp disable in_use; death should not destroy the book */
		bp->in_use = FALSE;
		losestr(Poison_resistance ? rn1(2,1) : rn1(4,3));
		losehp(rnd(Poison_resistance ? 6 : 10),
/*JP
		       "contact-poisoned spellbook", KILLED_BY_AN);
*/
		       "魔法書に塗られた接触型の毒で", KILLED_BY_AN);
		bp->in_use = TRUE;
		break;
	case 6:
		if(Antimagic) {
		    shieldeff(u.ux, u.uy);
/*JP
		    pline_The("book %s, but you are unharmed!", explodes);
*/
		    pline("本は強力なエネルギーを放出したが，あなたは傷つかない！");
		} else {
#if 0 /*JP*/
		    pline("As you read the book, it %s in your %s!",
			  explodes, body_part(FACE));
#else
		    pline("読んだとたん，本は強力なエネルギーをあなたの%sに放出した！",
			  body_part(FACE));
#endif
/*JP
		    losehp(2*rnd(10)+5, "exploding rune", KILLED_BY_AN);
*/
		    losehp(2*rnd(10)+5, "強力なルーン文字のエネルギーで", KILLED_BY_AN);
		}
		return TRUE;
	default:
		rndcurse();
		break;
	}
	return FALSE;
}

/* study while confused: returns TRUE if the book is destroyed */
STATIC_OVL boolean
confused_book(spellbook)
struct obj *spellbook;
{
	boolean gone = FALSE;

	if (!rn2(3) && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
	    spellbook->in_use = TRUE;	/* in case called from learn */
	    pline(
/*JP
	"Being confused you have difficulties in controlling your actions.");
*/
	  "混乱しているので，思うように動くことが難しい．");
	    display_nhwindow(WIN_MESSAGE, FALSE);
/*JP
	    You("accidentally tear the spellbook to pieces.");
*/
	    You("うっかり，魔法書を引きさいてしまった．");
	    if (!objects[spellbook->otyp].oc_name_known &&
		!objects[spellbook->otyp].oc_uname)
		docall(spellbook);
	    if (carried(spellbook)) useup(spellbook);
	    else useupf(spellbook, 1L);
	    gone = TRUE;
	} else {
#if 0 /*JP*/
	    You("find yourself reading the %s line over and over again.",
		spellbook == book ? "next" : "first");
#else
	    You("%sの行を何度も何度も繰り返し読んでいることに気づいた．",
		spellbook == book ? "次" : "最初");
#endif
	}
	return gone;
}

/* special effects for The Book of the Dead */
STATIC_OVL void
deadbook(book2)
struct obj *book2;
{
    struct monst *mtmp, *mtmp2;
    coord mm;

/*JP
    You("turn the pages of the Book of the Dead...");
*/
    You("死者の書のページをめくった．．．");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    /* KMH -- Need ->known to avoid "_a_ Book of the Dead" */
    book2->known = 1;
    if(invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
	register struct obj *otmp;
	register boolean arti1_primed = FALSE, arti2_primed = FALSE,
			 arti_cursed = FALSE;

	if(book2->cursed) {
/*JP
	    pline_The("runes appear scrambled.  You can't read them!");
*/
	    pline("ルーン文字がごちゃまぜに現れて，読むことができなかった！");
	    return;
	}

	if(!u.uhave.bell || !u.uhave.menorah) {
/*JP
	    pline("A chill runs down your %s.", body_part(SPINE));
*/
	    Your("%sに寒けが走った．", body_part(SPINE));
/*JP
	    if(!u.uhave.bell) You_hear("a faint chime...");
*/
	    if(!u.uhave.bell) You_hear("かすかなベルの音を聞いた．．．");
/*JP
	    if(!u.uhave.menorah) pline("Vlad's doppelganger is amused.");
*/
	    if(!u.uhave.menorah) pline("ヴラドの幻影があざ笑った．");
	    return;
	}

	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if(otmp->otyp == CANDELABRUM_OF_INVOCATION &&
	       otmp->spe == 7 && otmp->lamplit) {
		if(!otmp->cursed) arti1_primed = TRUE;
		else arti_cursed = TRUE;
	    }
	    if(otmp->otyp == BELL_OF_OPENING &&
	       (moves - otmp->age) < 5L) { /* you rang it recently */
		if(!otmp->cursed) arti2_primed = TRUE;
		else arti_cursed = TRUE;
	    }
	}

	if(arti_cursed) {
#if 0 /*JP*/
	    pline_The("invocation fails!");
	    pline("At least one of your artifacts is cursed...");
#else
	    pline("儀式に失敗した！");
	    pline("あなたの聖器の少なくともひとつが呪われていたようだ．．．");
#endif
	} else if(arti1_primed && arti2_primed) {
	    unsigned soon = (unsigned) d(2,6);	/* time til next intervene() */

	    /* successful invocation */
	    mkinvokearea();
	    u.uevent.invoked = 1;
	    /* in case you haven't killed the Wizard yet, behave as if
	       you just did */
	    u.uevent.udemigod = 1;	/* wizdead() */
	    if (!u.udg_cnt || u.udg_cnt > soon) u.udg_cnt = soon;
	} else {	/* at least one artifact not prepared properly */
/*JP
	    You("have a feeling that %s is amiss...", something);
*/
	    You("何かが間違っているような気がした．．．");
	    goto raise_dead;
	}
	return;
    }

    /* when not an invocation situation */
    if (book2->cursed) {
raise_dead:

/*JP
	You("raised the dead!");
*/
	You("死者を蘇らせた！");
	/* first maybe place a dangerous adversary */
	if (!rn2(3) && ((mtmp = makemon(&mons[PM_MASTER_LICH],
					u.ux, u.uy, NO_MINVENT)) != 0 ||
			(mtmp = makemon(&mons[PM_NALFESHNEE],
					u.ux, u.uy, NO_MINVENT)) != 0)) {
	    mtmp->mpeaceful = 0;
	    set_malign(mtmp);
	}
	/* next handle the affect on things you're carrying */
	(void) unturn_dead(&youmonst);
	/* last place some monsters around you */
	mm.x = u.ux;
	mm.y = u.uy;
	mkundead(&mm, TRUE, NO_MINVENT);
    } else if(book2->blessed) {
	for(mtmp = fmon; mtmp; mtmp = mtmp2) {
	    mtmp2 = mtmp->nmon;		/* tamedog() changes chain */
	    if (DEADMONSTER(mtmp)) continue;

	    if (is_undead(mtmp->data) && cansee(mtmp->mx, mtmp->my)) {
		mtmp->mpeaceful = TRUE;
		if(sgn(mtmp->data->maligntyp) == sgn(u.ualign.type)
		   && distu(mtmp->mx, mtmp->my) < 4)
		    if (mtmp->mtame) {
			if (mtmp->mtame < 20)
			    mtmp->mtame++;
		    } else
			(void) tamedog(mtmp, (struct obj *)0);
		else monflee(mtmp, 0, FALSE, TRUE);
	    }
	}
    } else {
	switch(rn2(3)) {
	case 0:
/*JP
	    Your("ancestors are annoyed with you!");
*/
	    Your("先祖たちはあなたにいらついた！");
	    break;
	case 1:
/*JP
	    pline_The("headstones in the cemetery begin to move!");
*/
	    pline("墓地の墓石が動きはじめた！");
	    break;
	default:
/*JP
	    pline("Oh my!  Your name appears in the book!");
*/
	    pline("なんてこったい！あなたの名前が本に書いてある！");
	}
    }
    return;
}

STATIC_PTR int
learn(VOID_ARGS)
{
	int i;
	short booktype;
	char splname[BUFSZ];
	boolean costly = TRUE;

	if (!book || !(carried(book) || 
		(book->where == OBJ_FLOOR && 
			book->ox == u.ux && book->oy == u.uy))) {
	    /* maybe it was stolen or polymorphed? */
	    do_reset_learn();
	    return(0);
	}
	/* JDS: lenses give 50% faster reading; 33% smaller read time */
	if (delay < end_delay && ublindf && ublindf->otyp == LENSES && rn2(2))
	    delay++;
	if (Confusion) {		/* became confused while learning */
	    (void) confused_book(book);
	    book = 0;			/* no longer studying */
	    nomul(delay - end_delay);	/* remaining delay is uninterrupted */
	    delay = end_delay;
	    return(0);
	}
	if (delay < end_delay) {    /* not if (delay++), so at end delay == 0 */
	    delay++;
	    return(1); /* still busy */
	}
	exercise(A_WIS, TRUE);		/* you're studying. */
	booktype = book->otyp;
	if(booktype == SPE_BOOK_OF_THE_DEAD) {
	    deadbook(book);
	    return(0);
	}

#if 0 /*JP*/
	Sprintf(splname, objects[booktype].oc_name_known ?
			"\"%s\"" : "the \"%s\" spell",
		OBJ_NAME(objects[booktype]));
#else
	Sprintf(splname, objects[booktype].oc_name_known ?
			"\"%s\"" : "\"%s\"",
		jtrns_obj('+', OBJ_NAME(objects[booktype])));
#endif
	for (i = 0; i < MAXSPELL; i++)  {
		if (spellid(i) == booktype)  {
			if (book->spestudied > MAX_SPELL_STUDY) {
/*JP
			    pline("This spellbook is too faint to be read anymore.");
*/
			    pline("この魔法書の文字は薄すぎてこれ以上読めない．");
			    book->otyp = booktype = SPE_BLANK_PAPER;
			} else if (spellknow(i) <= MAX_CAN_STUDY) {
/*JP
			    Your("knowledge of that spell is keener.");
*/
			    Your("%sに対する知識は研ぎすまされた．", splname);
			    incrnknow(i);
			    book->spestudied++;
			    if (end_delay) {
			    	boostknow(i,
				  end_delay * (book->spe > 0 ? 20 : 10));
				use_skill(spell_skilltype(book->otyp),
				  end_delay / (book->spe > 0 ? 10 : 20));
			    }
			    exercise(A_WIS, TRUE);      /* extra study */
			} else { /* MAX_CAN_STUDY < spellknow(i) <= MAX_SPELL_STUDY */
/*JP
			    You("know %s quite well already.", splname);
*/
			    You("すでに%sについて熟知している．", splname);
			    costly = FALSE;
			}
			/* make book become known even when spell is already
			   known, in case amnesia made you forget the book */
			makeknown((int)booktype);
			break;
		} else if (spellid(i) == NO_SPELL)  {
			spl_book[i].sp_id = booktype;
			spl_book[i].sp_lev = objects[booktype].oc_level;
			incrnknow(i);
			book->spestudied++;
#if 0 /*JP*/
			You("have keen knowledge of the spell.");
			You(i > 0 ? "add %s to your repertoire." : "learn %s.",
			    splname);
#else
			You("呪文に関する強烈な知識を得た．");
			You(i > 0 ? "%sをレパートリーに加えた．" : "%sを習得した．", splname);
#endif
			makeknown((int)booktype);
			break;
		}
	}
	if (i == MAXSPELL) impossible("Too many spells memorized!");

	if (book->cursed) {	/* maybe a demon cursed it */
	    if (cursed_book(book)) {
		if (carried(book)) useup(book);
		else useupf(book, 1L);
		book = 0;
		return 0;
	    }
	}
	if (costly) check_unpaid(book);
	book = 0;
	return(0);
}

int
study_book(spellbook)
register struct obj *spellbook;
{
	register int	 booktype = spellbook->otyp;
	register boolean confused = (Confusion != 0);
	boolean too_hard = FALSE;

	if (delay && !confused && spellbook == book &&
		    /* handle the sequence: start reading, get interrupted,
		       have book become erased somehow, resume reading it */
		    booktype != SPE_BLANK_PAPER) {
/*JP
		You("continue your efforts to memorize the spell.");
*/
		You("呪文の学習を再開した．");
	} else {
		/* KMH -- Simplified this code */
		if (booktype == SPE_BLANK_PAPER) {
/*JP
			pline("This spellbook is all blank.");
*/
			pline("この魔法書は真っ白だ．");
			makeknown(booktype);
			return(1);
		}
		if (spellbook->spe && confused) {
		    check_unpaid_usage(spellbook, TRUE);
		    consume_obj_charge(spellbook, FALSE);
#if 0 /*JP*/
		    pline_The("words on the page seem to glow faintly purple.");
		    You_cant("quite make them out.");
#else
		    pline_The("そのページに書かれている言葉は，うっすらと紫色に光っているように見える．");
		    You("それらを全く理解することができなかった．");
#endif
		    return 1;
		}

		switch (objects[booktype].oc_level) {
		 case 1:
		 case 2:
			delay = -objects[booktype].oc_delay;
			break;
		 case 3:
		 case 4:
			delay = -(objects[booktype].oc_level - 1) *
				objects[booktype].oc_delay;
			break;
		 case 5:
		 case 6:
			delay = -objects[booktype].oc_level *
				objects[booktype].oc_delay;
			break;
		 case 7:
			delay = -8 * objects[booktype].oc_delay;
			break;
		 default:
			impossible("Unknown spellbook level %d, book %d;",
				objects[booktype].oc_level, booktype);
			return 0;
		}

		/* Books are often wiser than their readers (Rus.) */
		spellbook->in_use = TRUE;
		if (!spellbook->blessed &&
		    spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
		    if (spellbook->cursed) {
			too_hard = TRUE;
		    } else {
			/* uncursed - chance to fail */
			int read_ability = ACURR(A_INT) + 4 + u.ulevel/2
			    - 2*objects[booktype].oc_level
			    + ((ublindf && ublindf->otyp == LENSES) ? 2 : 0);
			/* only wizards know if a spell is too difficult */
			if (Role_if(PM_WIZARD) && read_ability < 20 &&
			    !confused && !spellbook->spe) {
			    char qbuf[QBUFSZ];
			    Sprintf(qbuf,
#if 0 /*JP*/
		      "This spellbook is %sdifficult to comprehend. Continue?",
				    (read_ability < 12 ? "very " : ""));
#else
		      "この魔法書を理解するのは%s困難だ．続けますか？",
					(read_ability < 12 ? "とても" : ""));
#endif
			    if (yn(qbuf) != 'y') {
				spellbook->in_use = FALSE;
				return(1);
			    }
			}
			/* its up to random luck now */
			if (rnd(20) > read_ability) {
			    too_hard = TRUE;
			}
		    }
		}

		if (too_hard && (spellbook->cursed || !spellbook->spe)) {
		    boolean gone = cursed_book(spellbook);

		    nomul(delay);			/* study time */
		    delay = 0;
		    if(gone || !rn2(3)) {
/*JP
			if (!gone) pline_The("spellbook crumbles to dust!");
*/
			if (!gone) pline("魔法書は崩れて塵となった！");
			if (!objects[spellbook->otyp].oc_name_known &&
				!objects[spellbook->otyp].oc_uname)
			    docall(spellbook);
				if (carried(spellbook)) useup(spellbook);
				else useupf(spellbook, 1L);
		    } else
			spellbook->in_use = FALSE;
		    return(1);
		} else if (confused) {
		    if (!confused_book(spellbook)) {
			spellbook->in_use = FALSE;
		    }
		    nomul(delay);
		    delay = 0;
		    return(1);
		}
		spellbook->in_use = FALSE;

		/* The glowing words make studying easier */
		if (spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
		    delay *= 2;
		    if (spellbook->spe) {
			check_unpaid_usage(spellbook, TRUE);
			consume_obj_charge(spellbook, FALSE);
#if 0 /*JP*/
			pline_The("words on the page seem to glow faintly.");
#else
			pline("そのページの単語はうっすらと光っているように見えた．");
#endif
			if (!too_hard)
			    delay /= 3;
		    }
		}
		end_delay = 0;  /* Changed if multi != 0 */

#ifdef DEBUG
		pline("Delay: %i", delay);
#endif
		if (multi) {
			/* Count == practice reading :) */
	        	char qbuf[QBUFSZ];
	        	
	        	if (multi + 1 > MAX_STUDY_TIME) multi = MAX_STUDY_TIME - 1;
#if 0 /*JP*/
	        	Sprintf(qbuf, "Study for at least %i turns?", (multi+1));
#else
	        	Sprintf(qbuf, "少なくとも%iターンを消費しますが学びますか？", (multi+1));
#endif
			if (ynq(qbuf) != 'y') {
				multi = 0;
				return(1);
			}
			if ((--multi) > (-delay)) end_delay = multi + delay;
			multi = 0;
#ifdef DEBUG
			pline("end_delay: %i", end_delay);
#endif
		}

#if 0 /*JP*/
		You("begin to %s the runes.",
		    spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "recite" :
		    "memorize");
#else
		You("ルーン文字を%sしはじめた．",
		    spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "詠唱" :
		    "記憶");
#endif
	}

	book = spellbook;
/*JP
	set_occupation(learn, "studying", 0);
*/
	set_occupation(learn, "学ぶ", 0);
	return(1);
}

/* a spellbook has been destroyed or the character has changed levels;
   the stored address for the current book is no longer valid */
void
book_disappears(obj)
struct obj *obj;
{
	if (obj == book) book = (struct obj *)0;
}

/* renaming an object usually results in it having a different address;
   so the sequence start reading, get interrupted, name the book, resume
   reading would read the "new" book from scratch */
void
book_substitution(old_obj, new_obj)
struct obj *old_obj, *new_obj;
{
	if (old_obj == book) book = new_obj;
}

static void
do_reset_learn()
{
	stop_occupation();
}

/* called from moveloop() */
void
age_spells()
{
	int i;
	/*
	 * The time relative to the hero (a pass through move
	 * loop) causes all spell knowledge to be decremented.
	 * The hero's speed, rest status, conscious status etc.
	 * does not alter the loss of memory.
	 */
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++)
	    if (spellknow(i))
		decrnknow(i);
	return;
}

/*
 * Return TRUE if a spell was picked, with the spell index in the return
 * parameter.  Otherwise return FALSE.
 */
STATIC_OVL boolean
getspell(spell_no)
	int *spell_no;
{
	int nspells, idx;
	char ilet, lets[BUFSZ], qbuf[QBUFSZ];

	if (spellid(0) == NO_SPELL)  {
/*JP
	    You("don't know any spells right now.");
*/
	    You("今のところ何の呪文も知らない．");
	    return FALSE;
	}
	if (flags.menu_style == MENU_TRADITIONAL) {
	    /* we know there is at least 1 known spell */
	    for (nspells = 1; nspells < MAXSPELL
			    && spellid(nspells) != NO_SPELL; nspells++)
		continue;

	    if (nspells == 1)  Strcpy(lets, "a");
	    else if (nspells < 27)  Sprintf(lets, "a-%c", 'a' + nspells - 1);
	    else if (nspells == 27)  Sprintf(lets, "a-z A");
	    else if (nspells < 53)
		Sprintf(lets, "a-z A-%c", 'A' + nspells - 27);
	    else if (nspells == 53)  Sprintf(lets, "a-z A-Z 0");
	    else if (nspells < 62)
		Sprintf(lets, "a-z A-Z 0-%c", '0' + nspells - 53);
	    else  Sprintf(lets, "a-z A-Z 0-9");

	    for(;;)  {
/*JP
		Sprintf(qbuf, "Cast which spell? [%s ?]", lets);
*/
		Sprintf(qbuf, "どの呪文を唱える？[%s ?]", lets);
		if ((ilet = yn_function(qbuf, (char *)0, '\0')) == '?')
		    break;

		if (index(quitchars, ilet))
		    return FALSE;

		idx = spell_let_to_idx(ilet);
		if (idx >= 0 && idx < nspells) {
		    *spell_no = idx;
		    return TRUE;
		} else
/*JP
		    You("don't know that spell.");
*/
		    You("そんな呪文は知らない．");
	    }
	}
#if 0 /*JP*/
	return dospellmenu("Choose which spell to cast",
			   SPELLMENU_CAST, spell_no);
#else
	return dospellmenu("どの呪文を唱える？",
			   SPELLMENU_CAST, spell_no);
#endif
}

/* the 'Z' command -- cast a spell */
int
docast()
{
	int spell_no;

	if (getspell(&spell_no))
	    return spelleffects(spell_no, FALSE);
	return 0;
}

STATIC_OVL const char*
spelltypemnemonic(int skill)
{
	switch (skill) {
	    case P_ATTACK_SPELL:
/*JP
	        return " attack";
*/
		return "攻撃";
	    case P_HEALING_SPELL:
/*JP
		return "healing";
*/
		return "治癒";
	    case P_DIVINATION_SPELL:
/*JP
	        return " divine";
*/
		return "予知";
	    case P_ENCHANTMENT_SPELL:
/*JP
	        return "enchant";
*/
		return "補助";
		case P_PROTECTION_SPELL:
/*JP
	        return "protect";
*/
		return "護り";
	    case P_BODY_SPELL:
/*JP
	        return "   body";
*/
		return "肉体";
	    case P_MATTER_SPELL:
/*JP
	        return " matter";
*/
		return "物質";
	    default:
		impossible("Unknown spell skill, %d;", skill);
		return "";
	}
}

int
spell_skilltype(booktype)
int booktype;
{
	return (objects[booktype].oc_skill);
}

STATIC_OVL void
cast_protection()
{
	int loglev = 0;
	int l = u.ulevel;
	int natac = u.uac - u.uspellprot;
	int gain;

	/* loglev=log2(u.ulevel)+1 (1..5) */
	while (l) {
	    loglev++;
	    l /= 2;
	}

	/* The more u.uspellprot you already have, the less you get,
	 * and the better your natural ac, the less you get.
	 *
	 *	LEVEL AC    SPELLPROT from sucessive SPE_PROTECTION casts
	 *      1     10    0,  1,  2,  3,  4
	 *      1      0    0,  1,  2,  3
	 *      1    -10    0,  1,  2
	 *      2-3   10    0,  2,  4,  5,  6,  7,  8
	 *      2-3    0    0,  2,  4,  5,  6
	 *      2-3  -10    0,  2,  3,  4
	 *      4-7   10    0,  3,  6,  8,  9, 10, 11, 12
	 *      4-7    0    0,  3,  5,  7,  8,  9
	 *      4-7  -10    0,  3,  5,  6
	 *      7-15 -10    0,  3,  5,  6
	 *      8-15  10    0,  4,  7, 10, 12, 13, 14, 15, 16
	 *      8-15   0    0,  4,  7,  9, 10, 11, 12
	 *      8-15 -10    0,  4,  6,  7,  8
	 *     16-30  10    0,  5,  9, 12, 14, 16, 17, 18, 19, 20
	 *     16-30   0    0,  5,  9, 11, 13, 14, 15
	 *     16-30 -10    0,  5,  8,  9, 10
	 */
	gain = loglev - (int)u.uspellprot / (4 - min(3,(10 - natac)/10));

	if (gain > 0) {
	    if (!Blind) {
		const char *hgolden = hcolor(NH_GOLDEN);

		if (u.uspellprot)
/*JP
		    pline_The("%s haze around you becomes more dense.",
*/
		    pline("あなたのまわりの%s霞が濃くなった．",
			      hgolden);
		else
#if 0 /*JP*/
		    pline_The("%s around you begins to shimmer with %s haze.",
			/*[ what about being inside solid rock while polyd? ]*/
			(Underwater || Is_waterlevel(&u.uz)) ? "water" : "air",
			      an(hgolden));
#else
		    pline("あなたのまわり%sに%sもやがあらわれてキラキラと光った．",
			      (Underwater || Is_waterlevel(&u.uz)) ? "の水中" : "",
			      hgolden);
#endif
	    }
	    u.uspellprot += gain;
	    u.uspmtime =
		P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_EXPERT ? 20 : 10;
	    if (!u.usptime)
		u.usptime = u.uspmtime;
	    find_ac();
	} else {
/*JP
	    Your("skin feels warm for a moment.");
*/
	    Your("肌は一瞬ポカポカした．");
	}
}

/* attempting to cast a forgotten spell will cause disorientation */
STATIC_OVL void
spell_backfire(spell)
int spell;
{
    long duration = (long)((spellev(spell) + 1) * 3);	 /* 6..24 */

    /* prior to 3.4.1, the only effect was confusion; it still predominates */
    switch (rn2(10)) {
    case 0:
    case 1:
    case 2:
    case 3: make_confused(duration, FALSE);			/* 40% */
	    break;
    case 4:
    case 5:
    case 6: make_confused(2L * duration / 3L, FALSE);		/* 30% */
	    make_stunned(duration / 3L, FALSE);
	    break;
    case 7:
    case 8: make_stunned(2L * duration / 3L, FALSE);		/* 20% */
	    make_confused(duration / 3L, FALSE);
	    break;
    case 9: make_stunned(duration, FALSE);			/* 10% */
	    break;
    }
    return;
}

int
spelleffects(spell, atme)
int spell;
boolean atme;
{
	int energy, damage, chance, n, intell;
	int hungr;
	int skill, role_skill;
	boolean confused = (Confusion != 0);
	struct obj *pseudo;

	/*
	 * Find the skill the hero has in a spell type category.
	 * See spell_skilltype for categories.
	 */
	skill = spell_skilltype(spellid(spell));
	role_skill = P_SKILL(skill);

	/*
	 * Spell casting no longer affects knowledge of the spell. A
	 * decrement of spell knowledge is done every turn.
	 */
	if (spellknow(spell) <= 0) {
/*JP
	    Your("knowledge of this spell is twisted.");
*/
	    Your("この呪文に関する知識はよじれた．");
/*JP
	    pline("It invokes nightmarish images in your mind...");
*/
	    pline("それは悪夢を心に浮ばせた．．．");
	    spell_backfire(spell);
	    return(0);
	} else if (spellknow(spell) <= 100) {
/*JP
	    You("strain to recall the spell.");
*/
	    You("呪文を思い出すのに苦労した．");
	} else if (spellknow(spell) <= 1000) {
/*JP
	    Your("knowledge of this spell is growing faint.");
*/
	    Your("この呪文に関する記憶が薄れつつある．");
	}
	energy = (spellev(spell) * 5);    /* 5 <= energy <= 35 */

	if (u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD) {
/*JP
		You("are too hungry to cast that spell.");
*/
	        pline("腹が減りすぎて呪文を唱えられない．");
		return(0);
	} else if (ACURR(A_STR) < 4)  {
/*JP
		You("lack the strength to cast spells.");
*/
		pline("強さが足りないため呪文を唱えられない．");
		return(0);
	} else if(check_capacity(
/*JP
		"Your concentration falters while carrying so much stuff.")) {
*/
		"あなたは物を持ちすぎており集中することができない．")){
	    return (1);
	} else if (!freehand()) {
/*JP
		Your("arms are not free to cast!");
*/
		pline("呪文を唱えようにも腕の自由が効かない！");
		return (0);
	}

	if (u.uhave.amulet) {
/*JP
		You_feel("the amulet draining your energy away.");
*/
		pline("魔除けにあなたの魔力が吸いとられている気がした．");
		energy += rnd(2*energy);
	}
		if (spellid(spell) != SPE_DETECT_FOOD) {
		hungr = energy * 2;

			/* If hero is a wizard, their current intelligence
			 * (bonuses + temporary + current)
			 * affects hunger reduction in casting a spell.
			 * 1. int = 17-18 no reduction
			 * 2. int = 16    1/4 hungr
			 * 3. int = 15    1/2 hungr
			 * 4. int = 1-14  normal reduction
			 * The reason for this is:
			 * a) Intelligence affects the amount of exertion
			 * in thinking.
			 * b) Wizards have spent their life at magic and
			 * understand quite well how to cast spells.
			 */
			intell = acurr(A_INT);
			if (!Role_if(PM_WIZARD)) intell = 10;
			switch (intell) {
				case 25: case 24: case 23: case 22:
				case 21: case 20: case 19: case 18:
				case 17: hungr = 0; break;
				case 16: hungr /= 4; break;
				case 15: hungr /= 2; break;
			}
	}
	else
		hungr = 0;
			/* don't put player (quite) into fainting from
			 * casting a spell, particularly since they might
			 * not even be hungry at the beginning; however,
			 * this is low enough that they must eat before
			 * casting anything else except detect food
			 */
			if (hungr > u.uhunger-3)
				hungr = u.uhunger-3;
	if (energy > u.uen)  {
/*JP
		You("don't have enough energy to cast that spell.");
*/
		pline("呪文を唱えるだけの十分な魔力を持っていない．");
		/* WAC/ALI Experts can override with HP/hunger loss */
/*JP
		if ((role_skill >= P_SKILLED) && (yn("Continue?") == 'y')) {
*/
		if ((role_skill >= P_SKILLED) && (yn("続けますか？") == 'y')) {
			energy -= u.uen;
			hungr += energy * 2;
			if (hungr > u.uhunger - 1)
				hungr = u.uhunger - 1;
/*JP
			losehp(energy,"spellcasting exhaustion", KILLED_BY);
*/
			losehp(energy,"呪文詠唱による消耗で", KILLED_BY);
			if (role_skill < P_EXPERT) exercise(A_WIS, FALSE);
			energy = u.uen;
		} else
			return 0;
	}
	morehungry(hungr);

	chance = percent_success(spell);
	if (confused || (rnd(100) > chance)) {
/*JP
		pline("You fail to cast the spell correctly.");
*/
		You("呪文を正しく唱えることができなかった．");

#ifdef ALLEG_FX
                if (iflags.usealleg) alleg_aura(u.ux, u.uy, P_ATTACK_SPELL-1);
#endif

		u.uen -= (energy / 2);
		flags.botl = 1;
		return(1);
	}

	u.uen -= energy;
	
	flags.botl = 1;
	exercise(A_WIS, TRUE);

	/* pseudo is a temporary "false" object containing the spell stats. */
	pseudo = mksobj(spellid(spell), FALSE, FALSE);
	pseudo->blessed = pseudo->cursed = 0;
	pseudo->quan = 20L;			/* do not let useup get it */

	/* WAC -- If skilled enough,  will act like a blessed version */
	if (role_skill >= P_SKILLED)
		pseudo->blessed = 1;

#ifdef ALLEG_FX
        if (iflags.usealleg) alleg_aura(u.ux, u.uy, skill);
#endif
	switch(pseudo->otyp)  {
	/*
	 * At first spells act as expected.  As the hero increases in skill
	 * with the appropriate spell type, some spells increase in their
	 * effects, e.g. more damage, further distance, and so on, without
	 * additional cost to the spellcaster.
	 */
	case SPE_MAGIC_MISSILE:
	case SPE_FIREBALL:
	case SPE_CONE_OF_COLD:
	case SPE_LIGHTNING:
	case SPE_ACID_STREAM:
	case SPE_POISON_BLAST:
		if (tech_inuse(T_SIGIL_TEMPEST)) {
		    weffects(pseudo);
		    break;
		} /* else fall through... */
	/* these spells are all duplicates of wand effects */
	case SPE_FORCE_BOLT:
	case SPE_SLEEP:
	case SPE_KNOCK:
	case SPE_SLOW_MONSTER:
	case SPE_WIZARD_LOCK:
	case SPE_DIG:
	case SPE_TURN_UNDEAD:
	case SPE_POLYMORPH:
	case SPE_TELEPORT_AWAY:
	case SPE_CANCELLATION:
	case SPE_FINGER_OF_DEATH:
	case SPE_LIGHT:
	case SPE_DETECT_UNSEEN:
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
	case SPE_DRAIN_LIFE:
	case SPE_STONE_TO_FLESH:
		if (!(objects[pseudo->otyp].oc_dir == NODIR)) {
			if (atme) u.dx = u.dy = u.dz = 0;
			else if (!getdir((char *)0)) {
			    /* getdir cancelled, re-use previous direction */
/*JP
			    pline_The("magical energy is released!");
*/
			    pline("魔力が解放された！");
			}
			if(!u.dx && !u.dy && !u.dz) {
			    if ((damage = zapyourself(pseudo, TRUE)) != 0) {
#if 0 /*JP*/
				char buf[BUFSZ];
				Sprintf(buf, "zapped %sself with a spell", uhim());
				losehp(damage, buf, NO_KILLER_PREFIX);
#else
				losehp(damage, "自分自身の魔法を浴びて",
				       KILLED_BY);
#endif
			    }
			} else weffects(pseudo);
		} else weffects(pseudo);
		update_inventory();	/* spell may modify inventory */
		break;
	/* these are all duplicates of scroll effects */
	case SPE_REMOVE_CURSE:
	case SPE_CONFUSE_MONSTER:
	case SPE_DETECT_FOOD:
	case SPE_CAUSE_FEAR:
#if 0
		/* high skill yields effect equivalent to blessed scroll */
		if (role_skill >= P_SKILLED) pseudo->blessed = 1;
#endif
		/* fall through */
	case SPE_CHARM_MONSTER:
	case SPE_MAGIC_MAPPING:
	case SPE_CREATE_MONSTER:
	case SPE_IDENTIFY:
	case SPE_COMMAND_UNDEAD:                
	case SPE_SUMMON_UNDEAD:
		(void) seffects(pseudo);
		break;

	case SPE_ENCHANT_WEAPON:                
	case SPE_ENCHANT_ARMOR:
		if (role_skill >= P_EXPERT) n = 8;
		else if (role_skill >= P_SKILLED) n = 10;
		else if (role_skill >= P_BASIC) n = 12;
		else n = 14;	/* Unskilled or restricted */
		if (!rn2(n)) {
		    pseudo->blessed = 0;
		    (void) seffects(pseudo);
		} else
/*JP
		    Your("enchantment failed!");
*/
		    You("魔力付与に失敗した！");
		break;

	/* these are all duplicates of potion effects */
	case SPE_HASTE_SELF:
	case SPE_DETECT_TREASURE:
	case SPE_DETECT_MONSTERS:
	case SPE_LEVITATION:
	case SPE_RESTORE_ABILITY:
#if 0
		/* high skill yields effect equivalent to blessed potion */
		if (role_skill >= P_SKILLED) pseudo->blessed = 1;
#endif
		/* fall through */
	case SPE_INVISIBILITY:
		(void) peffects(pseudo);
		break;
	case SPE_CURE_BLINDNESS:
		healup(0, 0, FALSE, TRUE);
		break;
	case SPE_CURE_SICKNESS:
/*JP
		if (Sick) You("are no longer ill.");
*/
		if (Sick) Your("病気は直った．");
		if (Slimed) {
/*JP
		    pline_The("slime disappears!");
*/
		    pline("スライムは消え失せた！");
		    Slimed = 0;
		 /* flags.botl = 1; -- healup() handles this */
		}
		healup(0, 0, TRUE, FALSE);
		break;
	case SPE_CREATE_FAMILIAR:
		(void) make_familiar((struct obj *)0, u.ux, u.uy, FALSE);
		break;
	case SPE_CLAIRVOYANCE:
		if (!BClairvoyant)
		    do_vicinity_map();
		/* at present, only one thing blocks clairvoyance */
		else if (uarmh && uarmh->otyp == CORNUTHAUM)
/*JP
		    You("sense a pointy hat on top of your %s.",
*/
		    You("%sの上にとんがり帽子を発見した．",
			body_part(HEAD));
		break;
	case SPE_PROTECTION:
		cast_protection();
		break;
	case SPE_JUMPING:
		if (!jump(max(role_skill,1)))
			pline(nothing_happens);
		break;
	case SPE_RESIST_POISON:
		if(!(HPoison_resistance & INTRINSIC)) {
/*JP
			You("feel healthy ..... for the moment at least.");
*/
			You("健康になったような気がした．．．．．．さしあたって一時的に．");
			incr_itimeout(&HPoison_resistance, rn1(1000, 500) +
				spell_damage_bonus(spellid(spell))*100);
		} else pline(nothing_happens);	/* Already have as intrinsic */
		break;
	case SPE_RESIST_SLEEP:
		if(!(HSleep_resistance & INTRINSIC)) {
			if (Hallucination)
/*JP
				pline("Too much coffee!");
*/
				pline("コーヒーが多すぎる！");
			else
/*JP
				You("no longer feel tired.");
*/
				You("もはや疲れ知らずになったような気がした．");
			incr_itimeout(&HSleep_resistance, rn1(1000, 500) +
				spell_damage_bonus(spellid(spell))*100);
		} else pline(nothing_happens);	/* Already have as intrinsic */
		break;
	case SPE_ENDURE_COLD:
		if(!(HCold_resistance & INTRINSIC)) {
/*JP
			You("feel warmer.");
*/
			You("暖かくなってきたような気がした．");
			incr_itimeout(&HCold_resistance, rn1(1000, 500) +
				spell_damage_bonus(spellid(spell))*100);
		} else pline(nothing_happens);	/* Already have as intrinsic */
		break;
	case SPE_ENDURE_HEAT:
		if(!(HFire_resistance & INTRINSIC)) {
			if (Hallucination)
/*JP
				pline("Excellent! You feel, like, totally cool!");
*/
				pline("素晴らしい！ クールになったような気がする！");
			else
/*JP
				You("feel colder.");
*/
				You("寒くなってきたような気がした．");
			incr_itimeout(&HFire_resistance, rn1(1000, 500) +
				spell_damage_bonus(spellid(spell))*100);
		} else pline(nothing_happens);	/* Already have as intrinsic */
		break;
	case SPE_INSULATE:
		if(!(HShock_resistance & INTRINSIC)) {
			if (Hallucination)
/*JP
				pline("Bummer! You've been grounded!");
*/
				pline("よっしゃあ！ アース完了！");
			else
/*JP
				You("are not at all shocked by this feeling.");
*/
				You("全く電撃の影響を受けなくなった．");
			incr_itimeout(&HShock_resistance, rn1(1000, 500) +
				spell_damage_bonus(spellid(spell))*100);
		} else pline(nothing_happens);	/* Already have as intrinsic */
		break;
	case SPE_ENLIGHTEN: 
/*JP
		You("feel self-knowledgeable...");
*/
		You("自分自身が判るような気がした．．．");
		display_nhwindow(WIN_MESSAGE, FALSE);
		enlightenment(FALSE);
/*JP
		pline("The feeling subsides.");
*/
		pline("その感じはなくなった．");
		exercise(A_WIS, TRUE);
		break;

	/* WAC -- new spells */
	case SPE_FLAME_SPHERE:
	case SPE_FREEZE_SPHERE:
	{	register int cnt = 1;
		struct monst *mtmp;


		if (role_skill >= P_SKILLED) cnt += (role_skill - P_BASIC);
		while(cnt--) {
			mtmp = make_helper((pseudo->otyp == SPE_FLAME_SPHERE) ?
					PM_FLAMING_SPHERE : PM_FREEZING_SPHERE, u.ux, u.uy);
			if (!mtmp) continue;
			mtmp->mtame = 10;
			mtmp->mhpmax = mtmp->mhp = 1;
			mtmp->isspell = mtmp->uexp = TRUE;
		} /* end while... */
		break;
	}

	/* KMH -- new spells */
	case SPE_PASSWALL:
		if (!Passes_walls)
/*JP
			You_feel("ethereal.");
*/
			You("身体が非物質化したように感じた．");
		incr_itimeout(&HPasses_walls, rn1(100, 50));
		break;

	default:
		impossible("Unknown spell %d attempted.", spell);
		obfree(pseudo, (struct obj *)0);
		return(0);
	}

	/* gain skill for successful cast */
	use_skill(skill, spellev(spell));

	/* WAC successful casting increases solidity of knowledge */
	boostknow(spell,CAST_BOOST);

	obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
	return(1);
}


void
losespells()
{
	boolean confused = (Confusion != 0);
	int  n, nzap, i;

	book = 0;
	for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; n++)
		continue;
	if (n) {
		nzap = rnd(n) + confused ? 1 : 0;
		if (nzap > n) nzap = n;
		for (i = n - nzap; i < n; i++) {
		    spellid(i) = NO_SPELL;
		    exercise(A_WIS, FALSE);	/* ouch! */
		}
	}
}


/* the '+' command -- view known spells */
int
dovspell(VOID_ARGS)
{
	char qbuf[QBUFSZ];
	int splnum, othnum;
	struct spell spl_tmp;

	if (spellid(0) == NO_SPELL)
/*JP
	    You("don't know any spells right now.");
*/
	    You("呪文を知らない．");
	else {
#if 0 /*JP*/
	    while (dospellmenu("Currently known spells",
			       SPELLMENU_VIEW, &splnum)) {
		Sprintf(qbuf, "Reordering spells; swap '%s' with",
			spellname(splnum));
#else
	    while (dospellmenu("現在記憶している呪文一覧",
			       SPELLMENU_VIEW, &splnum)) {
		Sprintf(qbuf, "呪文の並び変え： '%s'と交換する呪文は？",
			jtrns_obj('+', spellname(splnum)));
#endif
		if (!dospellmenu(qbuf, splnum, &othnum)) break;

		spl_tmp = spl_book[splnum];
		spl_book[splnum] = spl_book[othnum];
		spl_book[othnum] = spl_tmp;
	    }
	}
	return 0;
}

STATIC_OVL boolean
dospellmenu(prompt, splaction, spell_no)
const char *prompt;
int splaction;	/* SPELLMENU_CAST, SPELLMENU_VIEW, or spl_book[] index */
int *spell_no;
{
	winid tmpwin;
	int i, n, how;
	char buf[BUFSZ];
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	/*
	 * The correct spacing of the columns depends on the
	 * following that (1) the font is monospaced and (2)
	 * that selection letters are pre-pended to the given
	 * string and are of the form "a - ".
	 *
	 * To do it right would require that we implement columns
	 * in the window-ports (say via a tab character).
	 */
	if (!iflags.menu_tab_sep)
/*JP
		Sprintf(buf, "%-20s     Level  %-12s Fail", "    Name", "Category");
*/
		Sprintf(buf, "%-28s     レベル %-6s 成功率", "    呪文名", "分類");
	else
/*JP
		Sprintf(buf, "Name\tLevel\tCategory\tFail");
*/
		Sprintf(buf, "呪文名\tレベル\t分類\t成功率");
	if (flags.menu_style == MENU_TRADITIONAL)
		Strcat(buf, iflags.menu_tab_sep ? "\tKey" : "  Key");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
		Sprintf(buf, iflags.menu_tab_sep ?
#if 0 /*JP*/
			"%s\t%-d%s\t%s\t%-d%%" : "%-20s  %2d%s   %-12s %3d%%",
			spellname(i), spellev(i),
			spellknow(i) ? " " : "*",
			spelltypemnemonic(spell_skilltype(spellid(i))),
			100 - percent_success(i));
#else
			"%s\t%-d%s\t%s\t%-d%%" : "%-28s  %2d%s   %-6s %3d%%",
			jtrns_obj('+', spellname(i)), spellev(i),
			spellknow(i) ? " " : "*",
			spelltypemnemonic(spell_skilltype(spellid(i))),
			percent_success(i));
#endif
		if (flags.menu_style == MENU_TRADITIONAL)
			Sprintf(eos(buf), iflags.menu_tab_sep ?
				"\t%c" : "%4c ", spellet(i) ? spellet(i) : ' ');

		any.a_int = i+1;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			 0, 0, ATR_NONE, buf,
			 (i == splaction) ? MENU_SELECTED : MENU_UNSELECTED);
	      }
	end_menu(tmpwin, prompt);

	how = PICK_ONE;
	if (splaction == SPELLMENU_VIEW && spellid(1) == NO_SPELL)
	    how = PICK_NONE;	/* only one spell => nothing to swap with */
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	if (n > 0) {
		*spell_no = selected[0].item.a_int - 1;
		/* menu selection for `PICK_ONE' does not
		   de-select any preselected entry */
		if (n > 1 && *spell_no == splaction)
		    *spell_no = selected[1].item.a_int - 1;
		free((genericptr_t)selected);
		/* default selection of preselected spell means that
		   user chose not to swap it with anything */
		if (*spell_no == splaction) return FALSE;
		return TRUE;
	} else if (splaction >= 0) {
	    /* explicit de-selection of preselected spell means that
	       user is still swapping but not for the current spell */
	    *spell_no = splaction;
	    return TRUE;
	}
	return FALSE;
}

/* Integer square root function without using floating point. */
STATIC_OVL int
isqrt(val)
int val;
{
    int rt = 0;
    int odd = 1;
    while(val >= odd) {
	val = val-odd;
	odd = odd+2;
	rt = rt + 1;
    }
    return rt;
}


STATIC_OVL int
percent_success(spell)
int spell;
{
	/* Intrinsic and learned ability are combined to calculate
	 * the probability of player's success at cast a given spell.
	 */
	int chance, splcaster, special, statused;
	int difficulty;
	int skill;

	splcaster = urole.spelbase;
	special = urole.spelheal;
	statused = ACURR(urole.spelstat);

	/* Calculate armor penalties */
	if (uarm && !(uarm->otyp == ROBE ||
		      uarm->otyp == ROBE_OF_POWER ||
		      uarm->otyp == ROBE_OF_PROTECTION)) 
	    splcaster += 5;

	/* Robes are body armour in SLASH'EM */
	if (uarm && is_metallic(uarm))
	    splcaster += /*(uarmc && uarmc->otyp == ROBE) ?
		urole.spelarmr/2 : */urole.spelarmr;
	else if (uarmc && uarmc->otyp == ROBE)
	    splcaster -= urole.spelarmr;
	if (uarms) splcaster += urole.spelshld;

	if (uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_BRILLIANCE)
		splcaster += uarmhbon;
	if (uarmg && is_metallic(uarmg)) splcaster += uarmgbon;
	if (uarmf && is_metallic(uarmf)) splcaster += uarmfbon;

	if (spellid(spell) == urole.spelspec)
		splcaster += urole.spelsbon;

	/* `healing spell' bonus */
	if (spell_skilltype(spellid(spell)) == P_HEALING_SPELL)
		splcaster += special;

	if (uarm && uarm->otyp == ROBE_OF_POWER) splcaster -= 3;
	if (splcaster < 5) splcaster = 5;
	if (splcaster > 20) splcaster = 20;

	/* Calculate learned ability */

	/* Players basic likelihood of being able to cast any spell
	 * is based of their `magic' statistic. (Int or Wis)
	 */
	chance = 11 * statused / 2;

	/*
	 * High level spells are harder.  Easier for higher level casters.
	 * The difficulty is based on the hero's level and their skill level
	 * in that spell type.
	 */
	skill = P_SKILL(spell_skilltype(spellid(spell)));
	skill = max(skill,P_UNSKILLED) - 1;	/* unskilled => 0 */
	difficulty= (spellev(spell)-1) * 4 - ((skill * 6) + (u.ulevel/3) + 1);

	if (difficulty > 0) {
		/* Player is too low level or unskilled. */
		chance -= isqrt(900 * difficulty + 2000);
	} else {
		/* Player is above level.  Learning continues, but the
		 * law of diminishing returns sets in quickly for
		 * low-level spells.  That is, a player quickly gains
		 * no advantage for raising level.
		 */
		int learning = 15 * -difficulty / spellev(spell);
		chance += learning > 20 ? 20 : learning;
	}

	/* Clamp the chance: >18 stat and advanced learning only help
	 * to a limit, while chances below "hopeless" only raise the
	 * specter of overflowing 16-bit ints (and permit wearing a
	 * shield to raise the chances :-).
	 */
	if (chance < 0) chance = 0;
	if (chance > 120) chance = 120;

	/* Wearing anything but a light shield makes it very awkward
	 * to cast a spell.  The penalty is not quite so bad for the
	 * player's class-specific spell.
	 */
	if (uarms && weight(uarms) > (int) objects[SMALL_SHIELD].oc_weight) {
		if (spellid(spell) == urole.spelspec) {
			chance /= 2;
		} else {
			chance /= 4;
		}
	}

	/* Finally, chance (based on player intell/wisdom and level) is
	 * combined with ability (based on player intrinsics and
	 * encumbrances).  No matter how intelligent/wise and advanced
	 * a player is, intrinsics and encumbrance can prevent casting;
	 * and no matter how able, learning is always required.
	 */
	chance = chance * (20-splcaster) / 15 - splcaster;

	/* Clamp to percentile */
	if (chance > 100) chance = 100;
	if (chance < 0) chance = 0;

	return chance;
}

/* Learn a spell during creation of the initial inventory */
void
initialspell(obj)
struct obj *obj;
{
	int i;

	for (i = 0; i < MAXSPELL; i++) {
	    if (spellid(i) == obj->otyp) {
	         pline("Error: Spell %s already known.",
	         		OBJ_NAME(objects[obj->otyp]));
	         return;
	    }
	    if (spellid(i) == NO_SPELL)  {
	        spl_book[i].sp_id = obj->otyp;
	        spl_book[i].sp_lev = objects[obj->otyp].oc_level;
	        incrnknow(i);
	        return;
	    }
	}
	impossible("Too many spells memorized!");
	return;
}

boolean
studyspell()
{
	/*Vars are for studying spells 'W', 'F', 'I', 'N'*/
	int spell_no;

	if (getspell(&spell_no)) {
		if (spellknow(spell_no) <= 0) {
/*JP
			You("are unable to focus your memory of the spell.");
*/
			You("その呪文を思い出せない．");
			return (FALSE);
		} else if (spellknow(spell_no) <= 1000) {
/*JP
			Your("focus and reinforce your memory of the spell.");
*/
			Your("その呪文に関する知識は強化された．");
			incrnknow(spell_no);
			exercise(A_WIS, TRUE);      /* extra study */
			return (TRUE);
		} else /* 1000 < spellknow(spell_no) <= 5000 */
/*JP
			You("know that spell quite well already.");
*/
			Your("その呪文に関する知識は充分だ．");
	}
	return (FALSE);
}

/*spell.c*/
