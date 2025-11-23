/*	SCCS Id: @(#)sounds.c	3.4	2002/05/06	*/
/*	Copyright (c) 1989 Janet Walz, Mike Threepoint */
/* NetHack may be freely redistributed.  See license for details. */

/*
**	Japanese version Copyright
**	(c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000
**	For 3.4, Copyright (c) Kentaro Shirakata, 2002-2003
**	For SLASH'EM 0.0.7 Copyright (c) Masaki Miyaso, Shiganai Sakusha, 2004-2008
**	JNetHack may be freely redistributed.  See license for details. 
*/

#include "hack.h"
#include "edog.h"
#ifdef USER_SOUNDS
# ifdef USER_SOUNDS_REGEX
#include <regex.h>
# endif
#endif

/* Hmm.... in working on SHOUT I started thinking about things.
 * I think something like this should be set up:
 *  You_hear_mon(mon,loud, msg) - You_hear(msg); monnoise(mon,loud);
 *  monnoise(mon,loud) - wake_nearto(mon->mx,mon->my,mon->data->mlevel*loud)
 *				and stuff like that
 *  mon_say(mon,loud,msg) - verbalize(msg); sayeffects(mon,loud,msg);
 *  sayeffects(mon,loud,msg) - monnoise(mon,loud); + the pet stuff et al
 * In fact, I think will set this up, but as a diff, not actually modifying the
 * files.
 * If I knew something about branches I might do that.
 * But anyway, I should be working on petcommands now... maybe later...
 * -- JRN
 */

#ifdef OVLB

static int FDECL(domonnoise,(struct monst *));
static int NDECL(dochat);
static const char *FDECL(yelp_sound,(struct monst *));
static const char *FDECL(whimper_sound,(struct monst *));

#endif /* OVLB */

#ifdef OVL0

#ifdef DUMB
static int FDECL(mon_in_room, (struct monst *,int));

/* this easily could be a macro, but it might overtax dumb compilers */
static int
mon_in_room(mon, rmtyp)
struct monst *mon;
int rmtyp;
{
    int rno = levl[mon->mx][mon->my].roomno;

    return rooms[rno - ROOMOFFSET].rtype == rmtyp;
}
#else
/* JRN: converted above to macro */
# define mon_in_room(mon,rmtype) (rooms[ levl[(mon)->mx][(mon)->my].roomno \
					- ROOMOFFSET].rtype == (rmtype))
#endif

void
dosounds()
{
    register struct mkroom *sroom;
    register int hallu, vx, vy;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
    int xx;
#endif
    struct monst *mtmp;

    if (!flags.soundok || u.uswallow || Underwater) return;

    hallu = Hallucination ? 1 : 0;

    if (level.flags.nfountains && !rn2(400)) {
	static const char * const fountain_msg[4] = {
/*JP
		"bubbling water.",
*/
		"コポコポという水の音を聞いた．",
/*JP
		"water falling on coins.",
*/
		"降り注ぐ水の音を聞いた．",
/*JP
		"the splashing of a naiad.",
*/
		"泉の精が水浴びをする音を聞いた．",
/*JP
		"a soda fountain!",
*/
		"ソーダの泉がシューという音を聞いた！"
	};
	You_hear(fountain_msg[rn2(3)+hallu]);
    }
#ifdef SINK
    if (level.flags.nsinks && !rn2(300)) {
	static const char * const sink_msg[3] = {
/*JP
		"a slow drip.",
*/
		"水がぽたぽたと落ちる音を聞いた．",
/*JP
		"a gurgling noise.",
*/
		"がらがらという音を聞いた．",
/*JP
		"dishes being washed!",
*/
		"皿を洗う音を聞いた！",
	};
	You_hear(sink_msg[rn2(2)+hallu]);
    }
#endif
    if (level.flags.has_court && !rn2(200)) {
	static const char * const throne_msg[4] = {
/*JP
		"the tones of courtly conversation.",
*/
		"上品な話し声を聞いた．",
/*JP
		"a sceptre pounded in judgment.",
*/
		"裁判で笏を叩き鳴らされるのを聞いた．",
/*JP
		"Someone shouts \"Off with %s head!\"",
*/
		"だれかが「そのものの首を刎ねよ！」と叫ぶ声を聞いた．",
/*JP
		"Queen Beruthiel's cats!",
*/
		"ベルシエル王妃の猫の声を聞いた！",
	};
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->msleeping ||
			is_lord(mtmp->data) || is_prince(mtmp->data)) &&
		!is_animal(mtmp->data) &&
		mon_in_room(mtmp, COURT)) {
		/* finding one is enough, at least for now */
		int which = rn2(3)+hallu;

		if (which != 2) You_hear(throne_msg[which]);
		else		pline(throne_msg[2], uhis());
		return;
	    }
	}
    }
    if (level.flags.has_swamp && !rn2(200)) {
	static const char * const swamp_msg[3] = {
#if 0 /*JP*/
		"hear mosquitoes!",
		"smell marsh gas!",	/* so it's a smell...*/
		"hear Donald Duck!",
#else
		"蚊の羽音を聞いた！",
		"沼地の腐った匂いを嗅いだ！",	/* so it's a smell...*/
		"ドナルドダックの声を聞いた！",
#endif
	};
#if 0 /*JP*/
	You(swamp_msg[rn2(2)+hallu]);
#else
	{
		int c;
		c = rn2(2)+hallu;
		if(c == 1){
			You(swamp_msg[c]);
		} else {
			You_hear(swamp_msg[c]);
		}
	}
#endif
	return;
    }
    if (level.flags.spooky && !rn2(200)) {
	static const char *spooky_msg[24] = {
#if 0 /*JP*/
		"hear screaming in the distance!",
		"hear a faint whisper: \"Please leave your measurements for your custom-made coffin.\"",
		"hear a door creak ominously.",
		"hear hard breathing just a few steps behind you!",
		"hear dragging footsteps coming closer!",
		"hear anguished moaning and groaning coming out of the walls!",
		"hear mad giggling directly behind you!",
		"smell rotting corpses.",
		"smell chloroform!",
		"feel ice cold fingers stroking your neck.",
		"feel a ghostly touch caressing your face.",
		"feel somebody dancing on your grave.",
		"feel something breathing down your neck.",
		"feel as if the walls were closing in on you.",
		"just stepped on something squishy.",
		"hear a strong voice pronouncing: \"There can only be one!\"",
		"hear a voice booming all around you: \"Warning: self-destruction sequence activated!\"",
		"smell your mother-in-law's cooking!",
		"smell horse dung.",
		"hear someone shouting: \"Who ordered the burger?\"",
		"can faintly hear the Twilight Zone theme.",
		"hear an outraged customer complaining: \"I'll be back!\"",
		"hear someone praising your valor!",
		"hear someone singing: \"Jingle bells, jingle bells...\"",
	};
	You(spooky_msg[rn2(15)+hallu*9]);
#else
		"あなたのすぐそばから悲鳴が聞こえた！",
		"かすかなささやきが聞こえた：「寸法をとらせて頂けませんか？棺を特注する必要がありまして．」",
		"扉が無気味にきしむ音を聞いた．",
		"ほんの数歩ほど離れた場所から強い息遣いが聞こえてきた！",
		"暗闇から足を引きずる音が近づいてくるのが聞こえた！",
		"壁の向こうから苦悶のうめきとうなり声が聞こえてきた！",
		"あなたのすぐそばで狂ったようなくすくす笑いが聞こえた！",
		"腐った死体の匂いがする．",
		"あなたはクロロホルムの匂いをかいだ！",
		"あなたは氷の様に冷たい手が首筋をなでるのを感じた．",
		"あなたは幽霊の手があなたの顔をなで回すのを感じた．",
		"あなたは誰かがあなたの墓の上で踊っているように感じた．",
		"あなたは誰かが首筋に息を吹きかけているのを感じた．",
		"あなたは壁が迫ってくるように感じた．",
		"あ，何か踏んづけた．うわあ，ぐしゃぐしゃだ．．．",	/*"あなたは何かを踏んづけて，ぐしゃっとつぶした．",*/
		"「そこに行けるのは一人だけだ！」と，力強い声が発せられるのを聞いた．",
		"あなたの周囲で声が響いた：「警告：自殺シーケンス作動中！」",
		"義理の母の料理の香りがただよってきた！",
		"馬の糞の匂いがした．",
		"誰かが「ハンバーガーをご注文された方はいらっしゃいますか？」と叫ぶのを聞いた．",
		"トワイライトゾーンのテーマがかすかに流れているのが聞こえた．",
		"暴力的な奴が「I'll be back!」とつぶやくのを聞いた．",
		"どこからか，あなたを応援する声が聞こえた！",
		"「ジングルベル，ジングルベル．．．」という歌声が聞こえた．",
	};
	/* 日本語では "あなたは" の後に "匂い/香りがした" 等を入れる事が難しい事もあり,この際省略*/
	pline(spooky_msg[rn2(15)+hallu*9]);
#endif
	return;
    }
    if (level.flags.has_vault && !rn2(200)) {
	if (!(sroom = search_special(VAULT))) {
	    /* strange ... */
	    level.flags.has_vault = 0;
	    return;
	}
	if(gd_sound())
	    switch (rn2(2)+hallu) {
		case 1: {
		    boolean gold_in_vault = FALSE;

		    for (vx = sroom->lx;vx <= sroom->hx; vx++)
			for (vy = sroom->ly; vy <= sroom->hy; vy++)
			    if (g_at(vx, vy))
				gold_in_vault = TRUE;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
		    /* Bug in aztec assembler here. Workaround below */
		    xx = ROOM_INDEX(sroom) + ROOMOFFSET;
		    xx = (xx != vault_occupied(u.urooms));
		    if(xx)
#else
		    if (vault_occupied(u.urooms) !=
			 (ROOM_INDEX(sroom) + ROOMOFFSET))
#endif /* AZTEC_C_WORKAROUND */
		    {
			if (gold_in_vault)
#if 0 /*JP*/
			    You_hear(!hallu ? "someone counting money." :
				"the quarterback calling the play.");
#else
			    You_hear(!hallu ? "誰かがお金を数えている声を聞いた．" :
				"クォータバックが指示をする声を聞いた．");
#endif
			else
/*JP
			    You_hear("someone searching.");
*/
			    You_hear("誰かが捜索している音を聞いた．");
			break;
		    }
		    /* fall into... (yes, even for hallucination) */
		}
		case 0:
/*JP
		    You_hear("the footsteps of a guard on patrol.");
*/
		    You_hear("警備員のパトロールする音を聞いた．");
		    break;
		case 2:
/*JP
		    You_hear("Ebenezer Scrooge!");
*/
		    You_hear("こち亀の両さんの声を聞いた！");
		    break;
	    }
	return;
    }
    if (level.flags.has_beehive && !rn2(200)) {
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->data->mlet == S_ANT && is_flyer(mtmp->data)) &&
		mon_in_room(mtmp, BEEHIVE)) {
		switch (rn2(2)+hallu) {
		    case 0:
/*JP
			You_hear("a low buzzing.");
*/
			You_hear("ぶーんという羽音を聞いた．");
			break;
		    case 1:
/*JP
			You_hear("an angry drone.");
*/
			You_hear("興奮した雄バチの羽音を聞いた．");
			break;
		    case 2:
#if 0 /*JP*/
			You_hear("bees in your %sbonnet!",
			    uarmh ? "" : "(nonexistent) ");
#else
			You_hear("帽子%sの中にいるハチの羽音を聞いた！",
			    uarmh ? "" : "(被ってないけど)");
#endif
			break;
		}
		return;
	    }
	}
    }
    if (level.flags.has_morgue && !rn2(200)) {
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (is_undead(mtmp->data) &&
		mon_in_room(mtmp, MORGUE)) {
		switch (rn2(2)+hallu) {
		    case 0:
/*JP
			You("suddenly realize it is unnaturally quiet.");
*/
			You("突然，不自然なくらい静かなことに気づいた．");
			break;
		    case 1:
#if 0 /*JP*/
			pline_The("%s on the back of your %s stands up.",
				body_part(HAIR), body_part(NECK));
#else
			pline("あなたの%sのうしろの%sが逆立った．",
				body_part(NECK), body_part(HAIR));
#endif
			break;
		    case 2:
#if 0 /*JP*/
			pline_The("%s on your %s seems to stand up.",
				body_part(HAIR), body_part(HEAD));
#else
			pline("あなたの%sの%sは逆立った．",
				body_part(HEAD), body_part(HAIR));
#endif
			break;
		}
		return;
	    }
	}
    }
    if (level.flags.has_barracks && !rn2(200)) {
	static const char * const barracks_msg[4] = {
#if 0 /*JP*/
		"blades being honed.",
		"loud snoring.",
		"dice being thrown.",
		"General MacArthur!",
#else
		"刃物を研ぐ音を聞いた．",
		"大きないびきを聞いた．",
		"ダイスが振られる音を聞いた．",
		"マッカーサー将軍の声を聞いた！",
#endif
	};
	int count = 0;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (is_mercenary(mtmp->data) &&
#if 0		/* don't bother excluding these */
		!strstri(mtmp->data->mname, "watch") &&
		!strstri(mtmp->data->mname, "guard") &&
#endif
		mon_in_room(mtmp, BARRACKS) &&
		/* sleeping implies not-yet-disturbed (usually) */
		(mtmp->msleeping || ++count > 5)) {
		You_hear(barracks_msg[rn2(3)+hallu]);
		return;
	    }
	}
    }
    if (level.flags.has_zoo && !rn2(200)) {
	static const char * const zoo_msg[3] = {
#if 0 /*JP*/
		"a sound reminiscent of an elephant stepping on a peanut.",
		"a sound reminiscent of a seal barking.",
		"Doctor Doolittle!",
#else
		"象がピーナッツの上で踊るような音を聞いた．",
		"アシカが吠えるような音を聞いた．",
		"ドリトル先生の声を聞いた！",
#endif
	};
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->msleeping || is_animal(mtmp->data)) &&
		    mon_in_room(mtmp, ZOO)) {
		You_hear(zoo_msg[rn2(2)+hallu]);
		return;
	    }
	}
    }
    if (level.flags.has_shop && !rn2(200)) {
	if (!(sroom = search_special(ANY_SHOP))) {
	    /* strange... */
	    level.flags.has_shop = 0;
	    return;
	}
	if (tended_shop(sroom) &&
		!index(u.ushops, ROOM_INDEX(sroom) + ROOMOFFSET)) {
	    static const char * const shop_msg[3] = {
#if 0 /*JP*/
		    "someone cursing shoplifters.",
		    "the chime of a cash register.",
		    "Neiman and Marcus arguing!",
#else
		    "誰かが泥棒をののしる声を聞いた．",
		    "レジのチーンという音を聞いた．",
		    "イトーとヨーカドーの議論を聞いた！",
#endif
	    };
	    You_hear(shop_msg[rn2(2)+hallu]);
	}
	return;
    }
    if (Is_oracle_level(&u.uz) && !rn2(400)) {
	/* make sure the Oracle is still here */
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if (!DEADMONSTER(mtmp) && mtmp->data == &mons[PM_ORACLE])
		break;
	/* and don't produce silly effects when she's clearly visible */
	if (mtmp && (hallu || !canseemon(mtmp))) {
	    static const char * const ora_msg[5] = {
#if 0 /*JP*/
		    "a strange wind.",		/* Jupiter at Dodona */
		    "convulsive ravings.",	/* Apollo at Delphi */
		    "snoring snakes.",		/* AEsculapius at Epidaurus */
		    "someone say \"No more woodchucks!\"",
		    "a loud ZOT!"		/* both rec.humor.oracle */
#else
		"奇妙な風の音を聞いた．",
		"半狂乱の声を聞いた．",
		"蛇のいびきを聞いた．",
		"誰かが「もうウッドチャックはいらない！」と言っている声を聞いた．",
		"大きなＺＯＴを聞いた！"
#endif
	    };
	    /* KMH -- Give funny messages on Groundhog Day */
	    if (flags.groundhogday) hallu = 1;
	    You_hear(ora_msg[rn2(3)+hallu*2]);
	}
	return;
    }
#ifdef BLACKMARKET
    if (!Is_blackmarket(&u.uz) && at_dgn_entrance("One-eyed Sam's Market") &&
        !rn2(200)) {
      static const char *blkmar_msg[3] = {
#if 0 /*JP*/
        "You hear someone complaining about the prices.",
        "Somebody whispers: \"Food rations? Only 900 zorkmids.\"",
        "You feel like searching for more gold.",
#else
        "あなたは誰かが値段について愚痴を言っているのを聞いた．",
        "誰かのささやきが聞こえた：「食糧はたったの900ゴールドだ．」",
        "あなたは更に多くの金貨を嗅ぎつけたような気がした．",
#endif
      };
      pline(blkmar_msg[rn2(2)+hallu]);
    }
#endif /* BLACKMARKET */
}

#endif /* OVL0 */
#ifdef OVLB

static const char * const h_sounds[] = {
#if 0 /*JP*/
    "beep", "boing", "sing", "belche", "creak", "cough", "rattle",
    "ululate", "pop", "jingle", "sniffle", "tinkle", "eep"
#else
    "ピーッと鳴いた","ボインと跳ねた","歌った","キーキーと鳴いた",
    "せき込んだ","ゴロゴロ鳴った","ホーホー鳴いた","ポンと鳴いた",
    "ガランガランと鳴いた","クンクン鳴いた","チリンチリンと鳴いた",
    "イーッと鳴いた"
#endif
};

/* make the sounds of a pet in any level of distress */
/* (1 = "whimper", 2 = "yelp", 3 = "growl") */
void
pet_distress(mtmp, lev)
register struct monst *mtmp;
int lev;
{
    const char *verb;
    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
	return;
    /* presumably nearness and soundok checks have already been made */

    if (Hallucination)
	verb = h_sounds[rn2(SIZE(h_sounds))];
    else if (lev == 3)
	verb = growl_sound(mtmp);
    else if (lev == 2)
	verb = yelp_sound(mtmp);
    else if (lev == 1)
	verb = whimper_sound(mtmp);
    else
	panic("strange level of distress");

    if (verb) {
#if 0 /*JP*/
	pline("%s %s%c", Monnam(mtmp), vtense((char *)0, verb),
		lev>1?'!':'.');
#else
	pline("%sは%s%s", Monnam(mtmp), makeplural(verb),
		lev>1?"！":"．");
#endif
	if (flags.run) nomul(0);
	wake_nearto(mtmp->mx,mtmp->my,mtmp->data->mlevel*6*lev);
    }
}

/* the sounds of a seriously abused pet, including player attacking it */
/* in extern.h: #define growl(mon) pet_distess((mon),3) */

const char *
growl_sound(mtmp)
register struct monst *mtmp;
{
	const char *ret;

	switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_HISS:
/*JP
	    ret = "hiss";
*/
	    ret = "シャーッと鳴いた";
	    break;
	case MS_BARK:
	case MS_GROWL:
/*JP
	    ret = "growl";
*/
	    ret = "激しく吠えた";
	    break;
	case MS_ROAR:
/*JP
	    ret = "roar";
*/
	    ret = "吠えた";
	    break;
	case MS_BUZZ:
/*JP
	    ret = "buzz";
*/
	    ret = "羽音を立てた";
	    break;
	case MS_SQEEK:
/*JP
	    ret = "squeal";
*/
	    ret = "キーキー鳴いた";
	    break;
	case MS_SQAWK:
/*JP
	    ret = "screech";
*/
	    ret = "ガァガァ鳴いた";
	    break;
	case MS_NEIGH:
/*JP
	    ret = "neigh";
*/
	    ret = "いなないた";
	    break;
	case MS_WAIL:
/*JP
	    ret = "wail";
*/
	    ret = "悲痛な叫びをあげた";
	    break;
	case MS_SILENT:
/*JP
		ret = "commotion";
*/
		ret = "興奮した";
		break;
	case MS_PARROT:
/*JP
	    ret = "squaark";
*/
	    ret = "クァーックと鳴いた";
	    break;
	default:
/*JP
		ret = "scream";
*/
		ret = "金切り声をあげた";
	}
	return ret;
}

/* the sounds of mistreated pets */
/* in extern.h: #define yelp(mon) pet_distress((mon),2) */

static
const char *
yelp_sound(mtmp)
register struct monst *mtmp;
{
    const char *ret;

    switch(mtmp->data->msound) {
	case MS_MEW:
/*JP
	ret = "yowl";
*/
	    ret = "悲しく鳴いた";
	    break;
	case MS_BARK:
	case MS_GROWL:
/*JP
	ret = "yelp";
*/
	    ret = "キャンキャン鳴いた";
	    break;
	case MS_ROAR:
/*JP
	ret = "snarl";
*/
	    ret = "歯をむいてうなった";
	    break;
	case MS_SQEEK:
/*JP
	ret = "squeal";
*/
	    ret = "キーキー鳴いた";
	    break;
	case MS_SQAWK:
/*JP
	ret = "screak";
*/
	    ret = "ガァガァ鳴いた";
	    break;
	case MS_WAIL:
/*JP
	ret = "wail";
*/
	    ret = "悲痛な叫びをあげた";
	    break;
    default:
	ret = (const char*) 0;
    }
    return ret;
}

/* the sounds of distressed pets */
/* in extern.h: #define whimper(mon) pet_distress((mon),1) */

static
const char *
whimper_sound(mtmp)
register struct monst *mtmp;
{
    const char *ret;

    switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_GROWL:
/*JP
	ret = "whimper";
*/
	    ret = "クンクン鳴いた";
	    break;
	case MS_BARK:
/*JP
	ret = "whine";
*/
	    ret = "クーンと鳴いた";
	    break;
	case MS_SQEEK:
/*JP
	ret = "squeal";
*/
	    ret = "キーキー鳴いた";
	    break;
    default:
	ret = (const char *)0;
    }
    return ret;
}

/* pet makes "I'm hungry" noises */
void
beg(mtmp)
register struct monst *mtmp;
{
    if (mtmp->msleeping || !mtmp->mcanmove ||
	    !(carnivorous(mtmp->data) || herbivorous(mtmp->data)))
	return;

    /* presumably nearness and soundok checks have already been made */
    if (!is_silent(mtmp->data) && mtmp->data->msound <= MS_ANIMAL)
	(void) domonnoise(mtmp);
    else if (mtmp->data->msound >= MS_HUMANOID) {
	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);
/*JP
	verbalize("I'm hungry.");
*/
	verbalize("はらぺこだ．");
}
}

static int
domonnoise(mtmp)
register struct monst *mtmp;
{
    register const char *pline_msg = 0,	/* Monnam(mtmp) will be prepended */
			*verbl_msg = 0;	/* verbalize() */
    struct permonst *ptr = mtmp->data;
    char verbuf[BUFSZ];

    /* presumably nearness and sleep checks have already been made */
    if (!flags.soundok) return(0);
    if (is_silent(ptr)) return(0);

    /* Make sure its your role's quest quardian; adjust if not */
    if (ptr->msound == MS_GUARDIAN && ptr != &mons[urole.guardnum]) {
    	int mndx = monsndx(ptr);
    	ptr = &mons[genus(mndx,1)];
    }

    /* be sure to do this before talking; the monster might teleport away, in
     * which case we want to check its pre-teleport position
     */
    if (!canspotmon(mtmp))
	map_invisible(mtmp->mx, mtmp->my);

    switch (ptr->msound) {
	case MS_ORACLE:
	    return doconsult(mtmp);
	case MS_PRIEST:
	    priest_talk(mtmp);
	    break;
	case MS_LEADER:
	case MS_NEMESIS:
	case MS_GUARDIAN:
	    quest_chat(mtmp);
	    break;
	case MS_SELL: /* pitch, pay, total */
	    shk_chat(mtmp);
	    break;
	case MS_VAMPIRE:
	    {
	    /* vampire messages are varied by tameness, peacefulness, and time of night */
		boolean isnight = night();
		boolean kindred = maybe_polyd(u.umonnum == PM_VAMPIRE ||
				    u.umonnum == PM_VAMPIRE_LORD ||
				    u.umonnum == PM_VAMPIRE_MAGE,
				    Race_if(PM_VAMPIRE));
		boolean nightchild = (Upolyd && (u.umonnum == PM_WOLF ||
				       u.umonnum == PM_SHADOW_WOLF ||
				       u.umonnum == PM_MIST_WOLF ||
				       u.umonnum == PM_WINTER_WOLF ||
	    			       u.umonnum == PM_WINTER_WOLF_CUB));
#if 0 /*JP*/
		const char *racenoun = (flags.female && urace.individual.f) ?
					urace.individual.f : (urace.individual.m) ?
					urace.individual.m : urace.noun;
#else
		const char *racenoun = (flags.female) ? "あなた" : "おまえ" ;
#endif

		if (mtmp->mtame) {
			if (kindred) {
#if 0 /*JP*/
				Sprintf(verbuf, "Good %s to you Master%s",
					isnight ? "evening" : "day",
					isnight ? "!" : ".  Why do we not rest?");
#else
				Sprintf(verbuf, "ご主人様，%s%s",
					isnight ? "こんばんは" : "こんにちは",
					isnight ? "!" : "．お休みにならないので？");
#endif
				verbl_msg = verbuf;
		    	} else {
#if 0 /*JP*/
		    	    Sprintf(verbuf,"%s%s",
				nightchild ? "Child of the night, " : "",
				midnight() ?
					"I can stand this craving no longer!" :
				isnight ?
					"I beg you, help me satisfy this growing craving!" :
					"I find myself growing a little weary.");
#else
		    	    Sprintf(verbuf,"%s%s",
				nightchild ? "夜の仔よ，" : "",
				midnight() ?
					"私はこれ以上渇望を抑えられない！" :
				isnight ?
					"ふくれあがる渇望を満たすのを助けてくれないか？頼む！" :
					"私は少々疲れたようだ．");
#endif
				verbl_msg = verbuf;
			}
		} else if (mtmp->mpeaceful) {
			if (kindred && isnight) {
#if 0 /*JP*/
				Sprintf(verbuf, "Good feeding %s!",
	    				flags.female ? "sister" : "brother");
#else
				Sprintf(verbuf, "ごきげんよう！");
#endif
				verbl_msg = verbuf;
 			} else if (nightchild && isnight) {
#if 0 /*JP*/
				Sprintf(verbuf,
				    "How nice to hear you, child of the night!");
#else
				Sprintf(verbuf,
				    "夜の仔よ，元気そうだな！");
#endif
				verbl_msg = verbuf;
	    		} else
/*JP
		    		verbl_msg = "I only drink... potions.";
*/
				verbl_msg = "私は薬．．．しか飲まない．";
    	        } else {
			int vampindex;
	    		static const char * const vampmsg[] = {
			       /* These first two (0 and 1) are specially handled below */
#if 0 /*JP*/
	    			"I vant to suck your %s!",
	    			"I vill come after %s without regret!",
#else
				"お前の%sをよこせ！",
				"存分に%sを追撃させてもらおう！",
#endif
		    	       /* other famous vampire quotes can follow here if desired */
	    		};
			if (kindred)
#if 0 /*JP*/
			    verbl_msg = "This is my hunting ground that you dare to prowl!";
#else
			    verbl_msg = "おまえがうろついているこのあたりは私の狩場だ！";
#endif
			else if (youmonst.data == &mons[PM_SILVER_DRAGON] ||
				 youmonst.data == &mons[PM_BABY_SILVER_DRAGON]) {
			    /* Silver dragons are silver in color, not made of silver */
#if 0 /*JP*/
			    Sprintf(verbuf, "%s! Your silver sheen does not frighten me!",
					youmonst.data == &mons[PM_SILVER_DRAGON] ?
					"Fool" : "Young Fool");
			    verbl_msg = verbuf; 
#else
			    Sprintf(verbuf, "%s！おまえの銀の輝きなど怖くないぞ！",
					youmonst.data == &mons[PM_SILVER_DRAGON] ?
					"ばかめ" : "若造が");
			    verbl_msg = verbuf; 
#endif
			} else {
			    vampindex = rn2(SIZE(vampmsg));
			    if (vampindex == 0) {
				Sprintf(verbuf, vampmsg[vampindex], body_part(BLOOD));
	    			verbl_msg = verbuf;
			    } else if (vampindex == 1) {
				Sprintf(verbuf, vampmsg[vampindex],
#if 0 /*JP*/
					Upolyd ? an(mons[u.umonnum].mname) : an(racenoun));
#else
					Upolyd ? jtrns_mon(mons[u.umonnum].mname) : racenoun);
#endif
	    			verbl_msg = verbuf;
		    	    } else
			    	verbl_msg = vampmsg[vampindex];
			}
	        }
	    }
	    break;
	case MS_WERE:
	    if (flags.moonphase == FULL_MOON && (night() ^ !rn2(13))) {
#if 0 /*JP*/
		pline("%s throws back %s head and lets out a blood curdling %s!",
		      Monnam(mtmp), mhis(mtmp),
		      ptr == &mons[PM_HUMAN_WERERAT] ? "shriek" : "howl");
#else
		pline("%sは頭をのけぞらし背筋が凍るような%sをあげた！",
		      Monnam(mtmp),
		      ptr == &mons[PM_HUMAN_WERERAT] ? "金切り声" : "咆哮");
#endif
		wake_nearto(mtmp->mx, mtmp->my, 11*11);
	    } else
		pline_msg =
/*JP
		     "whispers inaudibly.  All you can make out is \"moon\".";
*/
		     "聞きとれないような声でささやいた．かろうじて『月』という言葉だけが聞きとれた．";
	    break;
	case MS_BARK:
	    if (flags.moonphase == FULL_MOON && night()) {
/*JP
		pline_msg = "howls.";
*/
		pline_msg = "吠えた．";
	    } else if (mtmp->mpeaceful) {
		if (mtmp->mtame &&
			(mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
			 moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5))
/*JP
		    pline_msg = "whines.";
*/
		    pline_msg = "クンクン鳴いた．";
		else if (mtmp->mtame && EDOG(mtmp)->hungrytime > moves + 1000)
/*JP
		    pline_msg = "yips.";
*/
		    pline_msg = "キャンキャン鳴いた．";
		else {
		    if (mtmp->data != &mons[PM_DINGO])	/* dingos do not actually bark */
/*JP
			    pline_msg = "barks.";
*/
			    pline_msg = "ワンワン吠えた．";
		}
	    } else {
/*JP
		pline_msg = "growls.";
*/
		pline_msg = "激しく吠えた．";
	    }
	    break;
	case MS_MEW:
	    if (mtmp->mtame) {
		if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
			mtmp->mtame < 5)
/*JP
		    pline_msg = "yowls.";
*/
		    pline_msg = "悲しく鳴いた．";
		else if (moves > EDOG(mtmp)->hungrytime)
/*JP
		    pline_msg = "meows.";
*/
		    pline_msg = "ニャーンと鳴いた．";
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
/*JP
		    pline_msg = "purrs.";
*/
		    pline_msg = "ゴロゴロと鳴いた．";
		else
/*JP
		    pline_msg = "mews.";
*/
		    pline_msg = "ニャーニャー鳴いた．";
		break;
	    } /* else FALLTHRU */
	case MS_GROWL:
/*JP
	    pline_msg = mtmp->mpeaceful ? "snarls." : "growls!";
*/
	    pline_msg = mtmp->mpeaceful ? "うなった．" : "激しく吠えた！";
	    break;
	case MS_ROAR:
/*JP
	    pline_msg = mtmp->mpeaceful ? "snarls." : "roars!";
*/
	    pline_msg = mtmp->mpeaceful ? "うなった．" : "とても激しく吠えた！";
	    break;
	case MS_SQEEK:
/*JP
	    pline_msg = "squeaks.";
*/
	    pline_msg = "キーキー鳴いた．";
	    break;
	case MS_PARROT:
	    switch (rn2(8)) {
		default:
		case 0:
/*JP
		    pline_msg = "squaarks louldly!";
*/
		    pline_msg = "うるさく鳴いた！";
		    break;
		case 1:
/*JP
		    pline_msg = "says 'Polly want a lembas wafer!'";
*/
		    pline_msg = "「ポリーはレンバスが欲しい！」と言った．";
		    break;
		case 2:
/*JP
		    pline_msg = "says 'Nobody expects the Spanish Inquisition!'";
*/
		    pline_msg = "「スペインの宗教裁判からは誰も逃れられない」と言った．";
		    break;
		case 3:
/*JP
		    pline_msg = "says 'Who's a good boy, then?'";
*/
		    pline_msg = "「いい子にしてたかな？」と言った．";
		    break;
		case 4:
/*JP
		    pline_msg = "says 'Show us yer knickers!'";
*/
			/*スラングか？*/
		    pline_msg = "「パンツ見せて！」と言った．";
		    break;
		case 5:
/*JP
		    pline_msg = "says 'You'll never make it!'";
*/
		    pline_msg = "「お前には出来ない！」と言った．";
		    break;
		case 6:
/*JP
		    pline_msg = "whistles suggestively!";
*/
		    pline_msg = "思わせぶりな笛の音でピーッと鳴いた！";
		    break;
		case 7:
/*JP
		    pline_msg = "says 'What sort of a sword do you call that!'";
*/
		    pline_msg = "「どんな種類の剣を呼ぶつもりだ！」と言った．";
		    break;
	    }
	    break;
	case MS_SQAWK:
	    if (ptr == &mons[PM_RAVEN] && !mtmp->mpeaceful)
/*JP
	    	verbl_msg = "Nevermore!";
*/
	    	verbl_msg = "もう二度と！";
	    else
/*JP
	    	pline_msg = "squawks.";
*/
		pline_msg = "ガァガァ鳴いた．";
	    break;
	case MS_HISS:
	    if (!mtmp->mpeaceful)
/*JP
		pline_msg = "hisses!";
*/
		pline_msg = "シーッと鳴いた！";
	    else return 0;	/* no sound */
	    break;
	case MS_BUZZ:
/*JP
	    pline_msg = mtmp->mpeaceful ? "drones." : "buzzes angrily.";
*/
	    pline_msg = mtmp->mpeaceful ? "低い羽音をたてている．" : "怒ったような羽音をたてた．";
	    break;
	case MS_GRUNT:
/*JP
	    pline_msg = "grunts.";
*/
	    pline_msg = "うなり声をあげた．";
	    break;
	case MS_NEIGH:
	    if (mtmp->mtame < 5)
/*JP
		pline_msg = "neighs.";
*/
		pline_msg = "荒々しくいなないた．";
	    else if (moves > EDOG(mtmp)->hungrytime)
/*JP
		pline_msg = "whinnies.";
*/
		pline_msg = "弱々しくいなないた．";
	    else
/*JP
		pline_msg = "whickers.";
*/
		pline_msg = "ヒヒーンといなないた．";
	    break;
	case MS_WAIL:
/*JP
	    pline_msg = "wails mournfully.";
*/
	    pline_msg = "悲しげに鳴いた．";
	    break;
	case MS_GURGLE:
/*JP
	    pline_msg = "gurgles.";
*/
	    pline_msg = "ごぼごぼと音を立てた．";
	    break;
	case MS_BURBLE:
/*JP
	    pline_msg = "burbles.";
*/
	    pline_msg = "ぶーぶくと鳴いた．";
	    break;
	case MS_SHRIEK:
/*JP
	    pline_msg = "shrieks.";
*/
	    pline_msg = "金切り声をあげた．";
	    aggravate();
	    break;
	case MS_IMITATE:
/*JP
	    pline_msg = "imitates you.";
*/
	    pline_msg = "あなたの真似をした．";
	    break;
	case MS_SHEEP:
/*JP
	    pline_msg = "baaaas.";
*/
	    pline_msg = "メェ～と鳴いた．";
	    break;
	case MS_CHICKEN:
/*JP
	    pline_msg = "clucks.";
*/
	    pline_msg = "コッコッと鳴いた．";
	    break;
	case MS_COW:
/*JP
	    pline_msg = "bellows.";
*/
	    pline_msg = "モーッと鳴いた．";
	    break;
	case MS_BONES:
/*JP
	    pline("%s rattles noisily.", Monnam(mtmp));
*/
	    pline("%sは骨をカタカタと鳴らした．",Monnam(mtmp));
/*JP
	    You("freeze for a moment.");
*/
	    You("一瞬凍りついた．");
	    nomul(-2);
	    nomovemsg = 0;
	    break;
	case MS_LAUGH:
	    {
		static const char * const laugh_msg[4] = {
/*JP
		    "giggles.", "chuckles.", "snickers.", "laughs.",
*/
		    "くすくす笑った．", "くすっすと笑った．", "ばかにしたように笑った．", "笑った．",
		};
		pline_msg = laugh_msg[rn2(4)];
	    }
	    break;
	case MS_MUMBLE:
/*JP
	    pline_msg = "mumbles incomprehensibly.";
*/
	    pline_msg = "不可解な言葉をつぶやいた．";
	    break;
	case MS_DJINNI:
	    if (mtmp->mtame) {
/*JP
		verbl_msg = "Sorry, I'm all out of wishes.";
*/
		verbl_msg = "ごめんよ，願いは品切れなんだ．";
	    } else if (mtmp->mpeaceful) {
		if (ptr == &mons[PM_WATER_DEMON])
/*JP
		    pline_msg = "gurgles.";
*/
		    pline_msg = "ゴボゴボゴボゴボ．";
		else
/*JP
		    verbl_msg = "I'm free!";
*/
		    verbl_msg = "私は自由だ！";
/*JP
	    } else verbl_msg = "This will teach you not to disturb me!";
*/
	    } else verbl_msg = "私を煩わす者がどうなるか知るがいい！";
	    break;
	case MS_BOAST:	/* giants */
	    if (!mtmp->mpeaceful) {
		switch (rn2(4)) {
#if 0 /*JP*/
		case 0: pline("%s boasts about %s gem collection.",
			      Monnam(mtmp), mhis(mtmp));
#else
		case 0: pline("%sは自分の宝石のコレクションを自慢した．",
			      Monnam(mtmp));
#endif
			break;
/*JP
		case 1: pline_msg = "complains about a diet of mutton.";
*/
		case 1: pline_msg = "毎日羊ばかり食べている事を愚痴った．";
			break;
/*JP
	       default: pline_msg = "shouts \"Fee Fie Foe Foo!\" and guffaws.";
*/
	       default: pline_msg = "『わっはっはっは！』とばか笑いした．";
			wake_nearto(mtmp->mx, mtmp->my, 7*7);
			break;
		}
		break;
	    }
	    /* else FALLTHRU */
	case MS_HUMANOID:
	    if (!mtmp->mpeaceful) {
		if (In_endgame(&u.uz) && is_mplayer(ptr)) {
		    mplayer_talk(mtmp);
		    break;
		} else return 0;	/* no sound */
	    }
	    /* Generic peaceful humanoid behaviour. */
	    if (mtmp->mflee)
/*JP
		pline_msg = "wants nothing to do with you.";
*/
		pline_msg = "あなたに関わりたくないようだ．";
	    else if (mtmp->mhp < mtmp->mhpmax/4)
/*JP
		pline_msg = "moans.";
*/
		pline_msg = "うめき声をあげた．";
	    else if (mtmp->mconf || mtmp->mstun)
/*JP
		verbl_msg = !rn2(3) ? "Huh?" : rn2(2) ? "What?" : "Eh?";
*/
		verbl_msg = !rn2(3) ? "へ？" : rn2(2) ? "何？" : "え？";
	    else if (!mtmp->mcansee)
/*JP
		verbl_msg = "I can't see!";
*/
		verbl_msg = "何も見えない！";
	    else if (mtmp->mtrapped) {
		struct trap *t = t_at(mtmp->mx, mtmp->my);

		if (t) t->tseen = 1;
/*JP
		verbl_msg = "I'm trapped!";
*/
	        verbl_msg = "罠にはまってしまった！";
	    } else if (mtmp->mhp < mtmp->mhpmax/2)
/*JP
		pline_msg = "asks for a potion of healing.";
*/
		pline_msg = "回復の薬を持ってないか尋ねた．";
	    else if (mtmp->mtame && !mtmp->isminion &&
						moves > EDOG(mtmp)->hungrytime)
/*JP
		verbl_msg = "I'm hungry.";
*/
		verbl_msg = "腹が減ったな．";
	    /* Specific monsters' interests */
	    else if (is_elf(ptr))
/*JP
		pline_msg = "curses orcs.";
*/
		pline_msg = "オークを呪った．";
	    else if (is_dwarf(ptr))
/*JP
		pline_msg = "talks about mining.";
*/
		pline_msg = "採掘について話した．";
	    else if (likes_magic(ptr))
/*JP
		pline_msg = "talks about spellcraft.";
*/
		pline_msg = "魔法技術について話した．";
	    else if (ptr->mlet == S_CENTAUR)
/*JP
		pline_msg = "discusses hunting.";
*/
		pline_msg = "猟について議論した．";
	    else switch (monsndx(ptr)) {
		case PM_HOBBIT:
		    pline_msg = (mtmp->mhpmax - mtmp->mhp >= 10) ?
#if 0 /*JP*/
				"complains about unpleasant dungeon conditions."
				: "asks you about the One Ring.";
#else
				"不愉快な迷宮の状態について不満を述べた．"
				: "「一つの指輪」について尋ねた．";
#endif
		    break;
#if 0	/* OBSOLETE */
		case PM_FARMER_MAGGOT:
#if 0 /*JP*/
			pline_msg = "mumbles something about Morgoth.";
#else
			pline_msg = "冥王『モルゴス』についてつぶやいた．";
#endif
			break;
#endif
		case PM_ARCHEOLOGIST:
/*JP
    pline_msg = "describes a recent article in \"Spelunker Today\" magazine.";
*/
		    pline_msg = "「日刊スペランカー」の最新の記事を執筆している．";
		    break;
#ifdef TOURIST
		case PM_TOURIST:
/*JP
		    verbl_msg = "Aloha.";
*/
		    verbl_msg = "アローハ．";
		    break;
#endif
		default:
/*JP
		    pline_msg = "discusses dungeon exploration.";
*/
		    pline_msg = "迷宮探検について議論した．";
		    break;
	    }
	    break;
	case MS_SEDUCE:
#ifdef SEDUCE
	    if (ptr->mlet != S_NYMPH &&
		could_seduce(mtmp, &youmonst, (struct attack *)0) == 1) {
			(void) doseduce(mtmp);
			break;
	    }
	    switch ((poly_gender() != (int) mtmp->female) ? rn2(3) : 0)
#else
	    switch ((poly_gender() == 0) ? rn2(3) : 0)
#endif
	    {
		case 2:
#if 0 /*JP*/
			verbl_msg = "Hello, sailor.";
#else
			switch(poly_gender()){
			case 0:	      
			  verbl_msg = "こんにちは．あらいい男ね．";
			break;
			case 1:
			  verbl_msg = "こんにちは，お嬢さん．";
			  break;
			default:
			  verbl_msg = "こんにちは．";
			  break;
			}
#endif
			break;
		case 1:
/*JP
			pline_msg = "comes on to you.";
*/
			pline_msg = "あなたのほうへすり寄ってきた．";
			break;
		default:
/*JP
			pline_msg = "cajoles you.";
*/
			pline_msg = "あなたをおだてた．";
	    }
	    break;
#ifdef KOPS
	case MS_ARREST:
	    if (mtmp->mpeaceful)
#if 0 /*JP*/
		verbalize("Just the facts, %s.",
		      flags.female ? "Ma'am" : "Sir");
#else
		verbalize("事実だけが知りたいんですよ，%s．",
		      flags.female ? "奥さん" : "旦那");
#endif
	    else {
		static const char * const arrest_msg[3] = {
#if 0 /*JP*/
		    "Anything you say can be used against you.",
		    "You're under arrest!",
		    "Stop in the name of the Law!",
#else
		    "おまえの供述はおまえに不利な証拠となることがある！",
		    "おまえを逮捕する！",
		    "法の名のもと直ちに中止せよ！",
#endif
		};
		verbl_msg = arrest_msg[rn2(3)];
	    }
	    break;
#endif
	case MS_BRIBE:
	    if (mtmp->mpeaceful && !mtmp->mtame) {
		(void) demon_talk(mtmp);
		break;
	    }
	    /* fall through */
	case MS_CUSS:
	    if (!mtmp->mpeaceful)
		cuss(mtmp);
	    break;
	case MS_GYPSY:	/* KMH */
		if (mtmp->mpeaceful) {
			gypsy_chat(mtmp);
			break;
		}
		/* fall through */
	case MS_SPELL:
	    /* deliberately vague, since it's not actually casting any spell */
/*JP
	    pline_msg = "seems to mutter a cantrip.";
*/
	    pline_msg = "ぶつぶつとつぶやいている．";
	    break;
	case MS_NURSE:
	    if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
		|| (u.twoweap && uswapwep && (uswapwep->oclass == WEAPON_CLASS
		|| is_weptool(uswapwep))))
/*JP
		verbl_msg = "Put that weapon away before you hurt someone!";
*/
		verbl_msg = "武器をおさめなさい！それは人を傷つけるものよ！";
	    else if (uarmc || uarm || uarmh || uarms || uarmg || uarmf)
		verbl_msg = Role_if(PM_HEALER) ?
#if 0 /*JP*/
			  "Doc, I can't help you unless you cooperate." :
			  "Please undress so I can examine you.";
#else
			  "先生，あなたの協力なしではどうしようもありませんわ．" :
			  "服を脱いでください．あなたを診察しますわ．";
#endif
#ifdef TOURIST
	    else if (uarmu)
/*JP
		verbl_msg = "Take off your shirt, please.";
*/
		verbl_msg = "シャツを脱いでください．";
#endif
/*JP
	    else verbl_msg = "Relax, this won't hurt a bit.";
*/
	    else verbl_msg = "おちついて．ちっとも痛くありませんよ．";
	    break;
	case MS_GUARD:
#ifndef GOLDOBJ
	    if (u.ugold)
#else
	    if (money_cnt(invent))
#endif
/*JP
		verbl_msg = "Please drop that gold and follow me.";
*/
		verbl_msg = "金を置いてついてきてください．";
	    else
/*JP
		verbl_msg = "Please follow me.";
*/
		verbl_msg = "ついてきてください．";
	    break;
	case MS_SOLDIER:
	    {
		static const char * const soldier_foe_msg[3] = {
#if 0 /*JP*/
		    "Resistance is useless!",
		    "You're dog meat!",
		    "Surrender!",
#else
		    "抵抗しても無駄だ！",
		    "犬に喰われちまえ！",
		    "降伏しろ！",
#endif
		},		  * const soldier_pax_msg[3] = {
#if 0 /*JP*/
		    "What lousy pay we're getting here!",
		    "The food's not fit for Orcs!",
		    "My feet hurt, I've been on them all day!",
#else
		    "ここの給料はスズメの涙なみだ！",
		    "こんな飯，オークでも喰わねぇぜ！",
		    "足が痛いな，一日中立ちっぱなしだ！",
#endif
		};
		verbl_msg = mtmp->mpeaceful ? soldier_pax_msg[rn2(3)]
					    : soldier_foe_msg[rn2(3)];
	    }
	    break;
	case MS_RIDER:
	    if (ptr == &mons[PM_DEATH] && !rn2(10))
/*JP
		pline_msg = "is busy reading a copy of Sandman #8.";
*/
		pline_msg = "Sandmanの8章を読むのに忙しい．";
/*JP
	    else verbl_msg = "Who do you think you are, War?";
*/
	    else verbl_msg = "自分が何者か考えたことがあるか，ウォーよ？";
	    break;
    }

/*JP
    if (pline_msg) pline("%s %s", Monnam(mtmp), pline_msg);
*/
    if (pline_msg) pline("%sは%s", Monnam(mtmp), pline_msg);
    else if (verbl_msg) verbalize(verbl_msg);
    return(1);
}


int
dotalk()
{
    int result;
    boolean save_soundok = flags.soundok;
    flags.soundok = 1;	/* always allow sounds while chatting */
    result = dochat();
    flags.soundok = save_soundok;
    return result;
}

static int
dochat()
{
    register struct monst *mtmp;
    register int tx,ty;
    struct obj *otmp;

    if (is_silent(youmonst.data)) {
/*JP
	pline("As %s, you cannot speak.", an(youmonst.data->mname));
*/
	pline("あなたは%sなので，話すことができない．", jtrns_mon_gen(youmonst.data->mname, flags.female));
	return(0);
    }
    if (Strangled) {
/*JP
	You_cant("speak.  You're choking!");
*/
	You("話せない．あなたは首を絞められている！");
	return(0);
    }
    if (u.uswallow) {
/*JP
	pline("They won't hear you out there.");
*/
	pline("外にいる誰も聞きいれなかった．");
	return(0);
    }
    if (Underwater) {
/*JP
	Your("speech is unintelligible underwater.");
*/
	pline("水面下では，あなたの話はろくに理解されない．");
	return(0);
    }

    if (!Blind && (otmp = shop_object(u.ux, u.uy)) != (struct obj *)0) {
	/* standing on something in a shop and chatting causes the shopkeeper
	   to describe the price(s).  This can inhibit other chatting inside
	   a shop, but that shouldn't matter much.  shop_object() returns an
	   object iff inside a shop and the shopkeeper is present and willing
	   (not angry) and able (not asleep) to speak and the position contains
	   any objects other than just gold.
	*/
	price_quote(otmp);
	return(1);
    }

/*JP
    if (!getdir("Talk to whom? (in what direction)")) {
*/
    if (!getdir("誰と話しますか？[方向を入れてね]")) {
	/* decided not to chat */
	return(0);
    }

#ifdef STEED
    if (u.usteed && u.dz > 0)
	return (domonnoise(u.usteed));
#endif
    if (u.dz) {
/*JP
	pline("They won't hear you %s there.", u.dz < 0 ? "up" : "down");
*/
	pline("%s向って話をしても意味がない．", u.dz < 0 ? "上へ" : "下へ");
	return(0);
    }

    if (u.dx == 0 && u.dy == 0) {
/*
 * Let's not include this.  It raises all sorts of questions: can you wear
 * 2 helmets, 2 amulets, 3 pairs of gloves or 6 rings as a marilith,
 * etc...  --KAA
	if (u.umonnum == PM_ETTIN) {
	    You("discover that your other head makes boring conversation.");
	    return(1);
	}
*/
/*JP
	pline("Talking to yourself is a bad habit for a dungeoneer.");
*/
	pline("迷宮探検者にとって一人言は悪い癖だ．");
	return(0);
    }

    tx = u.ux+u.dx; ty = u.uy+u.dy;
    mtmp = m_at(tx, ty);

    if (!mtmp || mtmp->mundetected ||
		mtmp->m_ap_type == M_AP_FURNITURE ||
		mtmp->m_ap_type == M_AP_OBJECT)
	return(0);

    /* sleeping monsters won't talk, except priests (who wake up) */
    if ((!mtmp->mcanmove || mtmp->msleeping) && !mtmp->ispriest) {
	/* If it is unseen, the player can't tell the difference between
	   not noticing him and just not existing, so skip the message. */
	if (canspotmon(mtmp))
/*JP
	    pline("%s seems not to notice you.", Monnam(mtmp));
*/
	    pline("%sはあなたに気がついていないようだ．", Monnam(mtmp));
	return(0);
    }

    /* if this monster is waiting for something, prod it into action */
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    if (mtmp->mtame && mtmp->meating) {
	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);
/*JP
	pline("%s is eating noisily.", Monnam(mtmp));
*/
	pline("%sはバリバリと物を食べている．", Monnam(mtmp));
	return (0);
    }

    return domonnoise(mtmp);
}

#ifdef USER_SOUNDS

extern void FDECL(play_usersound, (const char*, int));

typedef struct audio_mapping_rec {
#ifdef USER_SOUNDS_REGEX
	struct re_pattern_buffer regex;
#else
	char *pattern;
#endif
	char *filename;
	int volume;
	struct audio_mapping_rec *next;
} audio_mapping;

static audio_mapping *soundmap = 0;

char* sounddir = ".";

/* adds a sound file mapping, returns 0 on failure, 1 on success */
int
add_sound_mapping(mapping)
const char *mapping;
{
	char text[256];
	char filename[256];
	char filespec[256];
	int volume;

	if (sscanf(mapping, "MESG \"%255[^\"]\"%*[\t ]\"%255[^\"]\" %d",
		   text, filename, &volume) == 3) {
	    const char *err;
	    audio_mapping *new_map;

	    if (strlen(sounddir) + strlen(filename) > 254) {
		raw_print("sound file name too long");
		return 0;
	    }
	    Sprintf(filespec, "%s/%s", sounddir, filename);

	    if (can_read_file(filespec)) {
		new_map = (audio_mapping *)alloc(sizeof(audio_mapping));
#ifdef USER_SOUNDS_REGEX
		new_map->regex.translate = 0;
		new_map->regex.fastmap = 0;
		new_map->regex.buffer = 0;
		new_map->regex.allocated = 0;
		new_map->regex.regs_allocated = REGS_FIXED;
#else
		new_map->pattern = (char *)alloc(strlen(text) + 1);
		Strcpy(new_map->pattern, text);
#endif
		new_map->filename = strdup(filespec);
		new_map->volume = volume;
		new_map->next = soundmap;

#ifdef USER_SOUNDS_REGEX
		err = re_compile_pattern(text, strlen(text), &new_map->regex);
#else
		err = 0;
#endif
		if (err) {
		    raw_print(err);
		    free(new_map->filename);
		    free(new_map);
		    return 0;
		} else {
		    soundmap = new_map;
		}
	    } else {
		Sprintf(text, "cannot read %.243s", filespec);
		raw_print(text);
		return 0;
	    }
	} else {
	    raw_print("syntax error in SOUND");
	    return 0;
	}

	return 1;
}

void
play_sound_for_message(msg)
const char* msg;
{
	audio_mapping* cursor = soundmap;

	while (cursor) {
#ifdef USER_SOUNDS_REGEX
	    if (re_search(&cursor->regex, msg, strlen(msg), 0, 9999, 0) >= 0) {
#else
	    if (pmatch(cursor->pattern, msg)) {
#endif
		play_usersound(cursor->filename, cursor->volume);
	    }
	    cursor = cursor->next;
	}
}

#endif /* USER_SOUNDS */

#endif /* OVLB */

/*sounds.c*/
