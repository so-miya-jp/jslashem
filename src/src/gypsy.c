/*** gypsy.c ***/

/*
**	Japanese version Copyright
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, 2004-2008
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"
#include "egyp.h"
#include "qtext.h"


/* To do:
 *	fortune_lev()
 *	Fourtunes for suited cards
 *	On-line help
 */


/*** Money-related functions ***/

static void
gypsy_charge (mtmp, amount)
	struct monst *mtmp;
	long amount;
{
#ifdef GOLDOBJ
	struct obj *gypgold;
#endif

	/* Take from credit first */
	if (amount > EGYP(mtmp)->credit) {
		/* Do in several steps, for broken compilers */
		amount -= EGYP(mtmp)->credit;
		EGYP(mtmp)->credit = 0;
#ifdef GOLDOBJ
		money2mon(mtmp, amount);
#else
		u.ugold -= amount;
#endif
		flags.botl = 1;
	} else
		EGYP(mtmp)->credit -= amount;

	/* The gypsy never carries cash; it might get stolen! */
#ifdef GOLDOBJ
	gypgold = findgold(mtmp->minvent);
	if (gypgold)
		m_useup(mtmp, gypgold);
#endif
	return;
}

static boolean
gypsy_offer (mtmp, cost, txt)
	struct monst *mtmp;
	long cost;
	char *txt;
{
#ifdef GOLDOBJ
	long umoney;
	umoney = money_cnt(invent);
#endif
#if 0 /*JP*/
	verbalize("For %ld credit I will %s!", cost, txt);
#else
	verbalize("%sには%ldの預け金が必要よ！", txt, cost);
#endif
	if (EGYP(mtmp)->credit >= cost) {
#if 0 /*JP*/
		if (yn("Accept this offer?") == 'y') {
#else
		if (yn("この申し出を受けますか？") == 'y') {
#endif
			EGYP(mtmp)->credit -= cost;
			return (TRUE);
		}
#ifndef GOLDOBJ
	} else if (EGYP(mtmp)->credit + u.ugold >= cost)
#if 0 /*JP*/
		verbalize("What a pity that I can't accept gold!");
#else
		verbalize("わたしは金貨は受け取らないの．おあいにくさま！");
#endif
#else
	} else if (EGYP(mtmp)->credit + umoney >= cost)
#if 0 /*JP*/
		verbalize("What a pity that I can't accept money!");
#else
		verbalize("わたしはお金は受け取らないの．おあいにくさま！");
#endif
#endif
		/* Maybe you could try gambling some of it for credit... */
	else
#if 0 /*JP*/
		verbalize("What a pity that you don't have enough!");
#else
		verbalize("あなたはそれだけの預け金を持ってないわ．おあいにくさま！");
#endif
	return (FALSE);
}

static long
gypsy_bet (mtmp, minimum)
	struct monst *mtmp;
	long minimum;
{
	char prompt[BUFSZ], buf[BUFSZ];
	long bet = 0L;
#ifdef GOLDOBJ
	long umoney;
	umoney = money_cnt(invent);
#endif

	if (minimum > EGYP(mtmp)->credit + 
#ifndef GOLDOBJ
													u.ugold) {
#else
 													umoney) {		
#endif
#if 0 /*JP*/
		You("don't have enough money for the minimum bet.");
#else
		You("賭け事をするための最低限の金額すら持っていない．");
#endif
		return (0L);
	}

	/* Prompt for an amount */
#if 0 /*JP*/
	Sprintf(prompt, "Bet how much (%ld to %ld)?", minimum,
#else
	Sprintf(prompt, "幾ら賭ける？ (%ld ～ %ld)", minimum,
#endif
			EGYP(mtmp)->credit + 
#ifndef GOLDOBJ
													u.ugold);
#else
													umoney);													
#endif
	getlin(prompt, buf);
	(void) sscanf(buf, "%ld", &bet);

	/* Validate the amount */
	if (bet == 0L) {
#if 0 /*JP*/
		pline("Never mind.");
#else
		pline("まぁ，気にしちゃ駄目よ．");
#endif
		return (0L);
	}
	if (bet < minimum) {
#if 0 /*JP*/
		You("must bet at least %ld.", minimum);
#else
		You("最低でも%ldは賭けなければならない．", minimum);
#endif
		return (0L);
	}
	if (bet > EGYP(mtmp)->credit +
#ifndef GOLDOBJ
								u.ugold) {
#else
								umoney) {												
#endif
#if 0 /*JP*/
		You("don't have that much money to bet!");
#else
		pline("賭けをするにはお金が足りない！");
#endif
		return (0L);
	}
	return (bet);
}


/*** Card-related functions ***/

static const char *suits[CARD_SUITS] =
#if 0 /*JP*/
{ "swords", "wands",     "shields",  "rings" };          /* Special */
#else
{ "剣",     "杖",        "盾",       "指輪" };           /* Special */
#endif
/* swords    wands/rods  roses/cups  pentacles/disks/coins  Tarot */
/* spade     bastoni     coppe       denari                 Italian */
/* swords    batons      cups        coins                  (translated) */
/* spades    clubs       hearts      diamonds               French */


static const char *ranks[CARD_RANKS] =
#if 0 /*JP*/
{ "ace", "2", "3", "4", "5", "6", "7", "8", "9", "10",
   /*none*/       "jack",       "queen", "king" }; /* French */
/* page/princess  knight/prince  queen    king        Tarot */
#else
{ "エース", "２", "３", "４", "５", "６", "７", "８", "９", "１０",
   /*none*/       "ジャック",   "クイーン", "キング" }; /* French */
#endif


static const char *trumps[CARD_TRUMPS] =
#if 0 /*JP*/
{	"the Fool",               /* This is NOT a Joker */
	"the Magician",           /* same as the Magus */
	"the High Priestess",     /* sometimes placed after the Emperor */
#if 0
	"the Empress",            /* not included here */
	"the Emperor",            /* not included here */
#endif
	"the Oracle",             /* same as the Hierophant */
	"the Lovers",
	"the Chariot",
	"Strength",               /* sometimes Adjustment */
	"the Hermit",
	"the Wheel of Fortune",   /* sometimes Fortune */
	"Justice",                /* sometimes Lust */
	"Punishment",             /* replaces the Hanged Man */
	"the Devil",              /* normally #15 */
	"Sorcery",                /* replaces Art or Temperance */
	"Death",                  /* swapped with the Devil so it remains #13 */
	"the Tower",              /* really! */
	"the Star",
	"the Moon",
	"the Sun",
	"Judgement",              /* sometimes Aeon */
	"Infinity"                /* replaces the World or the Universe */
#else
{	"愚者",              /* This is NOT a Joker */
	"魔術師",            /* same as the Magus */
	"女司祭長",          /* sometimes placed after the Emperor */
#if 0
	"女帝",              /* not included here */
	"皇帝",              /* not included here */
#endif
	"神託",              /* same as the Hierophant */
	"恋人",
	"戦車",
	"力",                /* sometimes Adjustment */
	"隠者",
	"運命の輪",          /* sometimes Fortune */
	"正義",              /* sometimes Lust */
	"囚人",              /* replaces the Hanged Man */
	"悪魔",              /* normally #15 */
	"魔術",              /* replaces Art or Temperance */
	"死神",              /* swapped with the Devil so it remains #13 */
	"塔",                /* really! */
	"星",
	"月",
	"太陽",
	"審判",              /* sometimes Aeon */
	"無限"               /* replaces the World or the Universe */
#endif
};


static void
card_shuffle (mtmp)
	struct monst *mtmp;
{
	xchar *cards = &EGYP(mtmp)->cards[0];
	int i, j, k;


#if 0 /*JP*/
	pline("%s shuffles the cards.", Monnam(mtmp));
#else
	pline("%sはカードをシャッフルした．", Monnam(mtmp));
#endif
	for (i = 0; i < CARD_TOTAL; i++)
		/* Initialize the value */
		cards[i] = i;
	for (i = 0; i < CARD_TOTAL; i++) {
		/* Swap this value with another randomly chosen one */
		j = rn2(CARD_TOTAL);
		k = cards[j];
		cards[j] = cards[i];
		cards[i] = k;
	}
	EGYP(mtmp)->top = CARD_TOTAL;
}

static xchar
card_draw (mtmp)
	struct monst *mtmp;
{
	if (EGYP(mtmp)->top <= 0)
		/* The deck is empty */
		return (-1);
	return (EGYP(mtmp)->cards[--EGYP(mtmp)->top]);
}

static void
card_name (num, buf)
	xchar num;
	char *buf;
{
	int r, s;


	if (!buf) return;
	if (Hallucination) num = rn2(CARD_TOTAL);
	if (num < 0 || num >= CARD_TOTAL) {
		/* Invalid card */
		impossible("no such card %d", num);
#if 0 /*JP*/
		Strcpy(buf, "a card");
#else
		Strcpy(buf, "カード");
#endif
	} else if (card_istrump(num)) {
		/* Handle trump cards */
		r = card_trump(num);
		if (!r)
#if 0 /*JP*/
			Sprintf(buf, "the zero of trumps (%s)", trumps[r]);
#else
			Sprintf(buf, "ゼロ番の切り札『%s』", trumps[r]);
#endif
		else
#if 0 /*JP*/
			Sprintf(buf, "the %d of trumps (%s)", r, trumps[r]);
#else
			Sprintf(buf, "第%d番の切り札『%s』", r, trumps[r]);
#endif
	} else {
		/* Handle suited cards */
		r = card_rank(num);
		s = card_suit(num);
#if 0 /*JP*/
		Sprintf(buf, "the %s of %s", ranks[r], suits[s]);
#else
		Sprintf(buf, "%sの%s", suits[s], ranks[r]);
#endif
	}
	return;
}


/*** Fortunes ***/

#define FORTUNE_COST	50			/* Cost to play */

static short birthstones[12] =
{
	/* Jan */  GARNET,      /* Feb */  AMETHYST,
	/* Mar */  AQUAMARINE,  /* Apr */  DIAMOND,
	/* May */  EMERALD,     /* Jun */  OPAL,
	/* Jul */  RUBY,        /* Aug */  CHRYSOBERYL,
	/* Sep */  SAPPHIRE,    /* Oct */  BLACK_OPAL,
	/* Nov */  TOPAZ,       /* Dec */  TURQUOISE
};


static void
fortune_lev (mtmp, name, txt)
	struct monst *mtmp;
	char *name, *txt;
{
	/*** FIXME -- still very buggy ***/
/*	d_level *lev;*/
	schar dep;


	dep = lev_by_name(name);
	if (!dep) {
		/* Perhaps the level doesn't exist? */
#if 0 /*JP*/
		verbalize("The vision is hazy.");
#else
		verbalize("幻像が霞んでいて見えないわ．");
#endif
		return;
	}

	if (dep == depth(&u.uz))
#if 0 /*JP*/
		verbalize("I see %s here.", txt);
#else
		verbalize("そこには%sが見えるわね．", txt);
#endif
	else {
#if 0 /*JP*/
		verbalize("I see %s on level %d.", txt, (int)dep);
#else
		verbalize("%d階に%sが見えるわね．", (int)dep, txt);
#endif
/*		if (gypsy_offer(mtmp, 5000L, "teleport you there"))
			;*/
	}
	return;
}

static void
fortune (mtmp)
	struct monst *mtmp;
{
	xchar card;
	char buf[BUFSZ];
	short otyp;
	struct obj *otmp;


	/* Shuffle the deck, if neccessary, and draw a card */
	gypsy_charge(mtmp, FORTUNE_COST);
	if (EGYP(mtmp)->top <= 0)
		card_shuffle(mtmp);
	card = card_draw(mtmp);
#ifdef WIZARD
	if (wizard) {
		long t = -1;

#if 0 /*JP*/
		getlin("Which trump?", buf);
#else
		getlin("どのカードを選ぶ？", buf);
#endif
		(void) sscanf(buf, "%ld", &t);
		if (t >= 0) card = t + CARD_SUITED;
	}
#endif
	card_name(card, buf);
#if 0 /*JP*/
	verbalize("You have drawn %s.", buf);
#else
	verbalize("あなたは%sを引いたわ．", buf);
#endif

	if (card_istrump(card))
		switch (card_trump(card)) {
		case 0:	/* the Fool */
			adjattrib(A_WIS, -1, 0);
			change_luck(-3);
			break;
		case 1:	/* the Magician */
			if (u.uevent.udemigod)
				resurrect();
			else
#if 0 /*JP*/
				fortune_lev(mtmp, "fakewiz1",
					"an entrance to the Wizard's tower");
#else
				fortune_lev(mtmp, "fakewiz1",
					"魔法使いの塔の入口");
#endif
				/*fortune_lev(mtmp, &portal_level);*/
			break;
		case 2: /* the High Priestess */
			if (u.uhave.amulet)
#if 0 /*JP*/
				verbalize("I see a high altar in the heavens.");
#else
				verbalize("天国に主祭壇が見えるわね．");
#endif
				/* Can only get there by ascending... */
			else
#if 0 /*JP*/
				verbalize("I see a high altar on level %d.",
#else
				verbalize("%d階に主祭壇が見えるわね．",
#endif
						depth(&sanctum_level));
				/* Can only get there by invocation... */
			break;
		case 3: /* the Oracle */
#if 0 /*JP*/
			fortune_lev(mtmp, "oracle", "the Oracle");
#else
			fortune_lev(mtmp, "oracle", "神託所");
#endif
			/*fortune_lev(mtmp, &oracle_level);*/
			break;
		case 4: /* the Lovers */
			makemon(&mons[flags.female ? PM_INCUBUS : PM_SUCCUBUS],
				u.ux, u.uy, 0);
			break;
		case 5: /* the Chariot */
#if 0 /*JP*/
			if (gypsy_offer(mtmp, 5000L,
					"teleport you to a level of your choosing")) {
#else
			if (gypsy_offer(mtmp, 5000L,
					"あなたを望む階に瞬間移動させる")) {
#endif
				incr_itimeout(&HTeleport_control, 1);
				level_tele();
			}
			break;
		case 6: /* Strength */
			adjattrib(A_STR, 1, 0);
			incr_itimeout(&HHalf_physical_damage, rn1(500, 500));
			break;
		case 7: /* the Hermit */
#if 0 /*JP*/
			You_feel("like hiding!");
#else
			You("隠れたいと思った！");
#endif
			incr_itimeout(&HTeleportation, rn1(300, 300));
			incr_itimeout(&HInvis, rn1(500, 500));
			newsym(u.ux, u.uy);
			break;
		case 8: /* the Wheel of Fortune */
			if (Hallucination)
#if 0 /*JP*/
				pline("Where is Vanna?");
#else
				/* アメリカでの国民的ゲーム番組"Wheel Of Fortune"では
				 * Vanna Whiteさんが司会者をしているらしい．
				 */
				pline("ヴァンナはどこだい？");
#endif
			else
#if 0 /*JP*/
				You_feel("lucky!");
#else
				pline("ラッキー！");
#endif
			if (u.uluck < 0)
				u.uluck = 0;
			else
				change_luck(3);
			break;
		case 9: /* Justice */
			makemon(&mons[PM_ERINYS], u.ux, u.uy, 0);
			break;
		case 10: /* Punishment */
			if (!Punished)
				punish((struct obj *)0);
			else
				rndcurse();
			break;
		case 11: /* the Devil */
			summon_minion(A_NONE, TRUE);
			break;
		case 12: /* Sorcery */
			adjattrib(urole.spelstat, 1, 0);
			incr_itimeout(&HHalf_spell_damage, rn1(500, 500));
			break;
		case 13: /* Death */
			if (nonliving(youmonst.data) || is_demon(youmonst.data) 
					|| Antimagic)
				shieldeff(u.ux, u.uy);
			else if(Hallucination)
#if 0 /*JP*/
				You("have an out of body experience.");
#else
				You("幽体離脱を体験した．");
#endif
			else  {
				killer_format = KILLED_BY;
#if 0 /*JP*/
				killer = "the card of Death";
#else
				killer = "死神のカードで";
#endif
				done(DIED);
			}
			break;
		case 14: /* the Tower */
#if 0 /*JP*/
			fortune_lev(mtmp, "vlad\'s tower", "Vlad the Impaler");
#else
			fortune_lev(mtmp, "vlad\'s tower", "ヴラド侯の塔");
#endif
			/* fortune_lev(mtmp, &vlad_level); */
			break;
		case 15: /* the Star */
			otyp = birthstones[getmonth()];
			makeknown(otyp);
			if ((otmp = mksobj(otyp, TRUE, FALSE)) != (struct obj *)0) {
#if 0 /*JP*/
				pline("%s reaches behind your %s and pulls out %s.",
						Monnam(mtmp), body_part(HEAD), doname(otmp));
#else
				pline("%sはあなたの%sの後ろに手を伸ばし，%sを引っ張り出した．",
						Monnam(mtmp), body_part(HEAD), doname(otmp));
#endif
				if (pickup_object(otmp, otmp->quan, FALSE) <= 0) {
					obj_extract_self(otmp);
					place_object(otmp, u.ux, u.uy);
					newsym(u.ux, u.uy);
				}
			}
			break;
		case 16: /* the Moon */
			/* Reset the old moonphase */
			if (flags.moonphase == FULL_MOON)
				change_luck(-1);

			/* Set the new moonphase */
			flags.moonphase = phase_of_the_moon();
			switch (flags.moonphase) {
				case NEW_MOON:
#if 0 /*JP*/
					pline("Be careful!  New moon tonight.");
#else
					pline("注意なさい！今晩は新月だから．");
#endif
					break;
				case 1:	case 2:	case 3:
#if 0 /*JP*/
					pline_The("moon is waxing tonight.");
#else
					pline("月は今宵も満ちつつあるわ．");
#endif
					break;
				case FULL_MOON:
#if 0 /*JP*/
					You("are lucky!  Full moon tonight.");
#else
					pline("ラッキーね！今晩は満月よ．");
#endif
					change_luck(1);
					break;
				case 5:	case 6:	case 7:
#if 0 /*JP*/
					pline_The("moon is waning tonight.");
#else
					pline("月は今宵も欠けつつあるわ．");
#endif
					break;
				default:
					impossible("wierd moonphase %d", flags.moonphase);
					break;
			}
			break;
		case 17: /* the Sun */
			if (midnight())
#if 0 /*JP*/
				verbalize("It is the witching hour.  Beware of the undead!");
#else
				verbalize("今は魔女がはびこる真夜中．アンデッドに気をつけなさい！");
#endif
			else if (night())
#if 0 /*JP*/
				verbalize("It is nighttime.  Beware of creatures of the night!");
#else
				verbalize("今は夜の支配する時． 夜の獣に気をつけて！");
#endif
			else
#if 0 /*JP*/
				verbalize("It is daytime.  Shouldn't you be working?");
#else
				verbalize("今は昼よ．働いているべきじゃないの？");
#endif
			break;
		case 18: /* Judgement */
#if 0 /*JP*/
			fortune_lev(mtmp, "portal to quest",
				"a portal to a quest");
#else
			fortune_lev(mtmp, "portal to quest",
				"故郷");
#endif
			/* fortune_lev(mtmp, &quest_level); */
			break;
		case 19: /* Infinity */
			if (mtmp->mcan) {
#if 0 /*JP*/
				verbalize("I wish I wasn't here!");
#else
				verbalize("わたしはここに居ないことを望むわ！");
#endif
				mongone(mtmp);
#if 0 /*JP*/
			} else if (gypsy_offer(mtmp, 10000L, "grant you a wish")) {
#else
			} else if (gypsy_offer(mtmp, 10000L, "願いを叶える")) {
#endif
				mtmp->mcan = TRUE;
				makewish();
			}
			break;
		default:
			impossible("unknown trump %d", card_trump(card));
			break;
		}	/* End trumps */
	else
		/* Suited card */
		com_pager(QT_GYPSY + card);

	return;
}


/*** Three-card monte ***/

#define MONTE_COST	1			/* Minimum bet */
#define MONTE_MAX	10			/* Maximum value of monteluck */


static void
monte (mtmp)
	struct monst *mtmp;
{
	long bet, n;
	char buf[BUFSZ];
	winid win;
	anything any;
	menu_item *selected;
	int delta;


	/* Get the bet */
	bet = gypsy_bet(mtmp, MONTE_COST);
	if (!bet) return;

	/* Shuffle and pick */
	if (flags.verbose)
#if 0 /*JP*/
		pline("%s places three cards and rearranges them.", Monnam(mtmp));
#else
		pline("%sはカードを3枚，並べて置いた．", Monnam(mtmp));
#endif
	any.a_void = 0;	/* zero out all bits */
	win = create_nhwindow(NHW_MENU);
	start_menu(win);
	any.a_char = 'l';
#if 0 /*JP*/
	add_menu(win, NO_GLYPH, &any , 'l', 0, ATR_NONE,
			"Left card", MENU_UNSELECTED);
#else
	add_menu(win, NO_GLYPH, &any , 'l', 0, ATR_NONE,
			"左のカード", MENU_UNSELECTED);
#endif
	any.a_char = 'c';
#if 0 /*JP*/
	add_menu(win, NO_GLYPH, &any , 'c', 0, ATR_NONE,
			"Center card", MENU_UNSELECTED);
#else
	add_menu(win, NO_GLYPH, &any , 'c', 0, ATR_NONE,
			"中央のカード", MENU_UNSELECTED);
#endif
	any.a_char = 'r';
#if 0 /*JP*/
	add_menu(win, NO_GLYPH, &any , 'r', 0, ATR_NONE,
			"Right card", MENU_UNSELECTED);
#else
	add_menu(win, NO_GLYPH, &any , 'r', 0, ATR_NONE,
			"右のカード", MENU_UNSELECTED);
#endif
#if 0 /*JP*/
	end_menu(win, "Pick a card:");
#else
	end_menu(win, "カードを選んでください：");
#endif
	while (select_menu(win, PICK_ONE, &selected) != 1) ;
	destroy_nhwindow(win);

	/* Calculate the change in odds for next time */
	/* Start out easy, but get harder once the player is suckered */
	delta = rnl(4) - 3;	/* Luck helps */
	if (u.umontelast == selected[0].item.a_char)
		/* Only suckers keep picking the same card */
		delta++;
	u.umontelast = selected[0].item.a_char;
	free((genericptr_t)selected);
	for (n = bet; n > 0; n /= 10L)
		/* Penalize big bets */
		delta++;
/*	pline("luck = %d; delta = %d", u.umonteluck, delta);*/

	/* Did we win? */
	if (u.umonteluck <= rn2(MONTE_MAX)) {
		if (u.umonteluck == 0)
#if 0 /*JP*/
			verbalize("You win!  Wasn't that easy?");
#else
			verbalize("あなたの勝ち！ そんなに甘くなかったでしょ？");
#endif
		else
#if 0 /*JP*/
			verbalize("You win!");
#else
			verbalize("あなたの勝ち！");
#endif
		EGYP(mtmp)->credit += bet;

		/* Make it harder for next time */
		if (delta > 0) u.umonteluck += delta;
		if (u.umonteluck > MONTE_MAX) u.umonteluck = MONTE_MAX;
	} else {
		card_name(rn1(2, 1), buf);
#if 0 /*JP*/
		verbalize("Sorry, you picked %s.  Try again.", buf);
#else
		verbalize("ごめんね，あなたが選んだカードは%sよ．やりなおしてちょうだい．", buf);
#endif
		gypsy_charge(mtmp, bet);

		/* Make it a little easier for next time */
		if (delta < 0) u.umonteluck += delta;
		if (u.umonteluck < 0) u.umonteluck = 0;
	}
	return;
}


/*** Ninety-nine ***/

#define NINETYNINE_COST		1	/* Minimum bet */
#define NINETYNINE_HAND		3	/* Number of cards in hand */
#define NINETYNINE_GOAL		99	/* Limit of the total */

static boolean
nn_playable (card, total)
	xchar card;
	int total;
{
	if (card_istrump(card))
		/* The fool always loses; other trumps are always playable */
		return (card != CARD_SUITED);
	switch (card_rank(card)+1) {
		case 11:	/* Jack */
		case 12:	/* Queen */
			return (total >= 10);
		case 13:	/* King */
			return (TRUE);
		default:	/* Ace through 10 */
			return ((total + card_rank(card) + 1) <= NINETYNINE_GOAL);
	}
}

static int
nn_play (card, total)
	xchar card;
	int total;
{
	if (card_istrump(card)) {
		if (card == CARD_SUITED)
			/* The Fool always loses */
			return (NINETYNINE_GOAL+1);
		else
			/* Other trumps leave the total unchanged */
			return (total);
	}
	switch (card_rank(card)+1) {
		case 11:	/* Jack */
		case 12:	/* Queen */
			return (total - 10);
		case 13:	/* King */
			return (NINETYNINE_GOAL);
		default:	/* Ace through 10 */
			return (total + card_rank(card) + 1);
	}
}

static int
nn_pref (card)
	xchar card;
{
	/* Computer's preferences for playing cards:
	 * 3.  Get rid of Ace through 10 whenever we can.  Highest priority.
	 * 2.  King will challenge the player.  High priority.
	 * 1.  Jack and queen may help us, or the hero.  Low priority. 
	 * 0.  Trumps can always be played (except the fool).  Lowest priority.
	 */
	if (card_istrump(card))
		/* The fool always loses; other trumps are always playable */
		return (0);
	switch (card_rank(card)+1) {
		case 11:	/* Jack */
		case 12:	/* Queen */
			return (1);
		case 13:	/* King */
			return (2);
		default:	/* Ace through 10 */
			return (3);
	}
}


static void
ninetynine (mtmp)
	struct monst *mtmp;
{
	long bet;
	int i, n, which, total = 0;
	xchar uhand[NINETYNINE_HAND], ghand[NINETYNINE_HAND];
	char buf[BUFSZ];
	winid win;
	anything any;
	menu_item *selected;


	/* Get the bet */
	bet = gypsy_bet(mtmp, NINETYNINE_COST);
	if (!bet) return;

	/* Shuffle the deck and deal */
	card_shuffle(mtmp);
	for (i = 0; i < NINETYNINE_HAND; i++) {
		uhand[i] = card_draw(mtmp);
		ghand[i] = card_draw(mtmp);
	}

	while (1) {
		/* Let the user pick a card */
		any.a_void = 0;	/* zero out all bits */
		win = create_nhwindow(NHW_MENU);
		start_menu(win);
		for (i = 0; i < NINETYNINE_HAND; i++) {
			any.a_int = (nn_playable(uhand[i], total) ? i+1 : 0);
			card_name(uhand[i], buf);
			add_menu(win, NO_GLYPH, &any , 0, 0, ATR_NONE,
					buf, MENU_UNSELECTED);
		}
		any.a_int = NINETYNINE_HAND + 1;
#if 0 /*JP*/
		add_menu(win, NO_GLYPH, &any , 'q', 0, ATR_NONE,
				"Forfeit", MENU_UNSELECTED);
#else
		add_menu(win, NO_GLYPH, &any , 'q', 0, ATR_NONE,
				"罰金を払う", MENU_UNSELECTED);
#endif
#if 0 /*JP*/
		end_menu(win, "Play a card:");
#else
		end_menu(win, "カード遊び：");
#endif
		while (select_menu(win, PICK_ONE, &selected) != 1) ;
		destroy_nhwindow(win);

		/* Play the card */
		which = selected[0].item.a_int-1;
		free((genericptr_t)selected);
		if (which >= NINETYNINE_HAND) {
#if 0 /*JP*/
			You("forfeit.");
#else
			You("掛け金を支払った．");
#endif
			gypsy_charge(mtmp, bet);
			return;
		}
		card_name(uhand[which], buf);
		total = nn_play(uhand[which], total);
#if 0 /*JP*/
		You("play %s for a total of %d.", buf, total);
#else
		pline("%sを出したので場の合計は%dだ．", buf, total);
#endif
		if (total < 0 || total > NINETYNINE_GOAL) {
#if 0 /*JP*/
			You("lose!");
#else
			You("負けてしまった！");
#endif
			gypsy_charge(mtmp, bet);
			return;
		}

		/* Draw a new card */
		uhand[which] = card_draw(mtmp);
		if (uhand[which] < 0) {
#if 0 /*JP*/
			pline_The("deck is empty.  You win!");
#else
			pline_The("山のカードがなくなった．あなたの勝ち！");
#endif
			EGYP(mtmp)->credit += bet;
			return;
		}

		/* Let the gypsy pick a card */
		n = 0;
		for (i = 0; i < NINETYNINE_HAND; i++)
			if (nn_playable(ghand[i], total)) {
				/* The card is playable, but is it the best? */
				if (!n++ || nn_pref(ghand[i]) > nn_pref(ghand[which]))
					which = i;
			}
		if (!n) {
			/* No playable cards */
#if 0 /*JP*/
			pline("%s forfeits.  You win!", Monnam(mtmp));
#else
			pline("%sは掛け金を支払った．あなたの勝ち！", Monnam(mtmp));
#endif
			EGYP(mtmp)->credit += bet;
			return;
		}

		/* Play the card */
		card_name(ghand[which], buf);
		total = nn_play(ghand[which], total);
#if 0 /*JP*/
		pline("%s plays %s for a total of %d.", Monnam(mtmp), buf, total);
#else
		pline("%sは%sを出したので場の合計は%dだ．", Monnam(mtmp), buf, total);
#endif

		/* Draw a new card */
		ghand[which] = card_draw(mtmp);
		if (ghand[which] < 0) {
#if 0 /*JP*/
			pline_The("deck is empty.  You win!");
#else
			pline_The("山のカードがなくなった．あなたの勝ち！");
#endif
			EGYP(mtmp)->credit += bet;
			return;
		}
	}

	return;
}



/*** Pawn gems ***/

STATIC_OVL NEARDATA const char pawnables[] = { ALLOW_COUNT, GEM_CLASS, 0 };

static void
pawn (mtmp)
	struct monst *mtmp;
{
	struct obj *otmp;
	long value;


	/* Prompt for an item */
	otmp = getobj((const char *)pawnables, "pawn");

	/* Is the item valid? */
	if (!otmp) return;
	if (!objects[otmp->otyp].oc_name_known) {
		/* Reject unknown objects */
#if 0 /*JP*/
		verbalize("Is this merchandise authentic?");
#else
		verbalize("この品は本物かしら？");
#endif
		return;
	}
	if (otmp->otyp < DILITHIUM_CRYSTAL || otmp->otyp > LAST_GEM) {
		/* Reject glass */
#if 0 /*JP*/
		verbalize("Don\'t bother with that junk!");
#else
		verbalize("ガラクタはお断りよ！");
#endif
		return;
	}

	/* Give the credit */
	value = otmp->quan * objects[otmp->otyp].oc_cost;
#if 0 /*JP*/
	pline("%s gives you %ld zorkmid%s credit.", Monnam(mtmp),
			value, plur(value));
#else
	pline("%sはあなたに%ld%s分の預け金を配当した．", Monnam(mtmp),
			value, currency(value));
#endif
	EGYP(mtmp)->credit += value;

	/* Gypsies don't keep merchandise; it could get stolen! */
	otmp->quan = 1L;
	useup(otmp);
	return;
}


/*** Yendorian Tarocchi ***/

#define TAROCCHI_COST	500		/* Cost to play */
#define TAROCCHI_HAND	10		/* Number of cards in hand */

static void
tarocchi (mtmp)
	struct monst *mtmp;
{
	int turn;

	/* Shuffle the deck and deal */
	gypsy_charge(mtmp, TAROCCHI_COST);
	card_shuffle(mtmp);

	/* Play the given number of turns */
	for (turn = TAROCCHI_HAND; turn > 0; turn--) {
	}

	return;
}


/*** Monster-related functions ***/

void
gypsy_init (mtmp)
	struct monst *mtmp;
{
	mtmp->isgyp = TRUE;
	mtmp->mpeaceful = TRUE;
	mtmp->msleeping = 0;
	mtmp->mtrapseen = ~0;	/* traps are known */
	EGYP(mtmp)->credit = 0L;
	EGYP(mtmp)->top = 0;
	return;
}


void
gypsy_chat (mtmp)
	struct monst *mtmp;
{
	long money;
	winid win;
	anything any;
	menu_item *selected;
#ifdef GOLDOBJ
	long umoney;
#endif
	int n;

#ifdef GOLDOBJ
	umoney = money_cnt(invent);
#endif

	/* Sanity checks */
	if (!mtmp || !mtmp->mpeaceful || !mtmp->isgyp ||
			!humanoid(mtmp->data))
		return;

	/* Add up your available money */
#if 0 /*JP*/
	You("have %ld zorkmid%s credit and are carrying %ld zorkmid%s.",
			EGYP(mtmp)->credit, plur(EGYP(mtmp)->credit),
#ifndef GOLDOBJ
			u.ugold, plur(u.ugold));
#else
			umoney, plur(umoney));			
#endif
#else
	You("%ld%s分の預け金と%ld%sの現金を持っている．",
			EGYP(mtmp)->credit, currency(EGYP(mtmp)->credit),
#ifndef GOLDOBJ
			u.ugold, currency(u.ugold));
#else
			umoney, currency(umoney));
#endif
#endif
	money = EGYP(mtmp)->credit +
#ifndef GOLDOBJ
											u.ugold;
#else
											umoney;
#endif

	/* Create the menu */
	any.a_void = 0;	/* zero out all bits */
	win = create_nhwindow(NHW_MENU);
	start_menu(win);

	/* Fortune */
	any.a_char = 'f';
	if (money >= FORTUNE_COST)
		add_menu(win, NO_GLYPH, &any , 'f', 0, ATR_NONE,
#if 0 /*JP*/
				"Read your fortune", MENU_UNSELECTED);
#else
				"運勢を占う", MENU_UNSELECTED);
#endif

	/* Three-card monte */
	any.a_char = 'm';
	if (money >= MONTE_COST)
		add_menu(win, NO_GLYPH, &any , 'm', 0, ATR_NONE,
#if 0 /*JP*/
				"Three-card monte", MENU_UNSELECTED);
#else
				"スリー・カード・モンテ", MENU_UNSELECTED);
#endif

	/* Ninety-nine */
	any.a_char = 'n';
	if (money >= NINETYNINE_COST)
		add_menu(win, NO_GLYPH, &any , 'n', 0, ATR_NONE,
#if 0 /*JP*/
				"Ninety-nine", MENU_UNSELECTED);
#else
				"ナインティ・ナイン", MENU_UNSELECTED);
#endif

	/* Pawn gems (always available) */
	any.a_char = 'p';
	add_menu(win, NO_GLYPH, &any , 'p', 0, ATR_NONE,
#if 0 /*JP*/
			"Pawn gems", MENU_UNSELECTED);
#else
			"宝石を質に入れる", MENU_UNSELECTED);
#endif

	/* Yendorian Tarocchi */
	any.a_char = 't';
/*	if (money >= TAROCCHI_COST)
		add_menu(win, NO_GLYPH, &any , 't', 0, ATR_NONE,
				"Yendorian Tarocchi", MENU_UNSELECTED);*/

	/* Help */
	any.a_char = '?';
		add_menu(win, NO_GLYPH, &any , '?', 0, ATR_NONE,
#if 0 /*JP*/
				"Help", MENU_UNSELECTED);

#else
				"ヘルプ", MENU_UNSELECTED);

#endif
	/* Display the menu */
#if 0 /*JP*/
	end_menu(win, "Play which game?");
#else
	end_menu(win, "どのゲームを選ぶ？");
#endif
	n = select_menu(win, PICK_ONE, &selected);
	destroy_nhwindow(win);
	if (n > 0) {
	    switch (selected[0].item.a_char) {
		case 'f':
			fortune(mtmp);
			break;
		case 'm':
			monte(mtmp);
			break;
		case 'n':
			ninetynine(mtmp);
			break;
		case 'p':
			pawn(mtmp);
			break;
		case 't':
			tarocchi(mtmp);
			break;
		case '?':
#if 0 /*JP*/
			display_file_area(FILE_AREA_SHARE, "gypsy.txt", TRUE);
#else
			display_file_area(FILE_AREA_SHARE, "jgypsy.txt", TRUE);
#endif
			break;
	    }
	    free((genericptr_t)selected);
	}

	return;
}


