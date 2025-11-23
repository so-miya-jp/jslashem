Fri Jul 24 2008  Masaki Miyaso  <so-miya@sourceforge.jp>
	* 報告バグの修正：#14614 呪われた複数重なっている松明のバグ
	* JNetHackの修正取込：#17806 3.4.3-0.10でのメッセージ不具合
	* jrumor.falおよびjrumor.truの翻訳を改訂
	* NetHack本家のバグ対応：C343-17 ゾーンに変化して人間の姿に戻った
	  後、プレイヤーがそこにはない落し穴にはまることがある。
	* 単語の翻訳修正
	  shape changer : 変化怪物 → 変身怪物
	  potion of sparkling : 火花が飛ぶ薬 → 泡立つ薬
	  sparkling water : 泡立つ水 → 泡立つ水
	  （potion of bubbly : 泡だっている薬
	    potion of fizzy : 発泡している薬
	    は変更しない）
	  potion of blood-red : 血のような赤色をした薬 → 血のように赤い薬
	  deep gnome : 神秘的なノーム → ディープノーム
	  quadruped(シンボル) : 四足動物 → 四つ足動物
	  quantum mechanic(シンボル) : 量子場 → 量子力学
	* 単語の翻訳の不統一を修正．多いほうに統一しました．
	  door：ドア，扉 → 扉
	  trap door：トラップドア，落とし扉，落し扉 → 落し扉
	  teleport：テレポート，瞬間移動 → 瞬間移動
	  number pad：ナンバーパッド，テンキー → テンキー
	* jdata.baseの翻訳修正
	  アバソール，エアドレ・ファインヤ，アレアックス，類人猿，アルコン，
	  アーケン石，足利尊氏，アズフィンクス，アソール，アテナ，ビホルダー，
	  瓶，バグベアー，クロマティック・ドラゴン，チカトリス，クトゥルフ，
	  *デヴァ，プラネター，ソーラー，影の狼，霧の狼，ドゥマソイン，毒
	  トカゲ，コモドドラゴン，ディープノーム，緑のガラス片，アークバグ，
	  氷の魔術師，カマダン，レーテ，リッチ，マングラー，モラディン，モ
	  ンバット，ン・カイ，多相グモ，ピクシー，ピロリスク，ルッゴ，魔法，
	  トーリンを翻訳．
	  ストームブリンガーを引用．
	  他いくつかの見出し語などを修正．
	* jcmdhelp，jhelp，jhh，jopthelpにおける訳語のぶれを統一．

Sun Aug 3 2008  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F3-J0.2.1リリース
	* 報告バグの修正：#12892 J0.2.0でのメッセージ周りの不具合
	* 報告バグの修正：#12893 光の聖槍を下賜された際の警告メッセージ
	* MinGW gcc-3.4.2でのコンパイルができるようにした．
	* NetHack brass #080325で実装された2バイトコードの擦れ処理を
	  JNetHack 3.4.3-0.10経由で取り込み．いくつかの修正を入れました．
	* JNetHackの修正取込：#11905 床の長いメッセージを読むと落ちる
	* JNetHackの修正取込：#11906 日本語で願えないものがある
	* JNetHackの修正取込：#12285 幻覚時の詳細表示
	* #11906関連で「アエスキュラピスの杖」も願えるように．
	* 野戦ライフルのburst modeの訳語を三点射から制限点射に変更．
	* 自動散弾銃等のsemi-automatic modeの訳語を半自動から単射に変更．
	* 自動散弾銃等のfull automatic modeの訳語を全自動から連射に変更．
	* クエストの文章を改訳
	* その他細かい文章の訳修正
	* 単語の翻訳修正
	  scroll of create monster：怪物を作る巻物 → 怪物を造る巻物
	  spellbook of create familiar：造魔の魔法書 → 使い魔を造る魔法書
	  wand of create horde：大群の杖 → 大群を造る杖
	  runed wand：ルーン文字の書かれた杖 → 神秘的な杖
	  worthless piece of * glass：＊ガラス → ＊ガラス片
	  burbling blob：おしゃべりブロッブ → 泡立つブロッブ
	  static blob：静かなブロッブ → 静電のブロッブ
	  water hulk：ウオーターハルク → ウォーターハルク
	  iron bars：鉄棒 → 鉄格子

Sat Mar 1 2008  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F3-J0.2.0リリース
	* 報告バグの修正：#11351 メッセージ周りの不整合・未訳部分
	* 本家修正先取り
		-Fixed bug 1612361: Strange behaviour when selling medical kit
		-Refix bug 1612361: Strange behaviour when selling medical kit
		                    (Pat Rankin)
		-Fixed bug 1681811: Ring of levitation - "Lev" status changes
		                    late
	* JNetHackの修正取込：wince用のコードを一部反映
	* JNetHackの修正取込：#9942 jrumors.fal
	* JNetHackの修正取込：訳語のrefine：joracles.txt, jquest.txt,
	                      wizard.c
	* JNetHackの修正取込：#9875 墓石のメッセージ
	* JNetHackの修正取込：#10527 クエスト文章の誤り
	* JNetHackの修正取込：#11611 変化制御を持つ状態で獣人形態が解除さ
	                      れるときの選択メッセージ
	* jrumors.truとjrumors.falをhack@holicの記述を参考に修正．
	* MSDOS版においてVIDEOSHADEが有効になっているとき(デフォルト)，既
	  知かつ視界外の廊下や部屋の表示色が灰色表示になっていなかった問題
	  の修正．
	* MSDOS版においてNO_TERMとVIDEOSHADEが両方有効になっているとき，
	 videoshadeオプションが無効になってしまっていた問題の修正．
	* MSDOS版においてオプションエラーが発生したとき，エラーが画面上に
	  出力されない問題を修正．
	* 落とし穴やくもの巣にあるアイテムを蹴ったときのメッセージを修正．
	* undeadの訳語が「アンデット」になっていた部分を修正．
	* Lord Carnarvonの訳語を「考古学者『カーナボン』伯爵」から「『カ
	  ーナボン』卿」に変更．
	* High Flame Mageの訳語を「炎の高位魔術師」から「炎魔術師の長」
	  に変更．
	* High Ice Mageの訳語を「氷の高位魔術師」から「氷魔術師の長」
	  に変更．
	* Orionの訳語を「勇者『オリオン』」から「『オリオン』」に変更．
	* chieftainの訳語を「首領」から「族長」に変更．
	* attendantの訳語を「随行員」から「助手」に変更．
	* igniterの訳語を「点火者」から「火種者」に変更．
	* embalmerの訳語を「ミイラ製作者」から「ミイラ職人」に変更．
	* Embalmerの訳語を「遺体化粧人」から「ミイラ職人」に変更．
	* apprenticeの訳語を「実習生」から「見習い」に変更．
	* Shan Lai Chingの訳語を「山雷精」から「山海経」に変更．
	* The Commonsの訳語を「平民」から「庶民院」に変更．
	* rutterkinの訳語を「ルッターキン」から「ラターキン」に変更．
	* nupperiboの訳語を「のっぺらぼう」から「ヌプパーレボ」に変更．
	* guardian nagaの訳語を「番兵ナーガ」から「守護のナーガ」に変更．
	* pit viperの訳語を「穴ヘビ」から「マムシ」に変更．
	* rhinovirusの訳語を「鼻風邪」から「ライノウィルス」に変更．
	* lion-dogの訳語を「ライオン犬」から「獅子・狛犬」に変更．
	* Luggageの訳語を「ルッゲージ」から「＜荷物＞」に変更．
	* one-eyed one-horned flying purple people eaterの訳語を「一目一手
	  空飛ぶ紫人食類」から「一目一角空飛ぶ紫人食類」に変更．
	* kegの訳語を「ケッグ」から「ビア樽」に変更．
	* obsidianの訳語を「黒燿石」から「黒耀石」に変更．
	* ring of obsidianの訳語を「黒曜石の指輪」から「黒耀石の指輪」に変
	  更．
	* dilithium crystalの訳語を「ディリジウムの結晶」から「ダイリチウ
	  ムの結晶」に変更．
	* 巻き物の未識別名をJNetHack独自のものから原文のアルファベットの
	  ものに変更．
	* Astral Planeの訳語を天上界に統一．
	* burnedの訳語を「傷ついた」から「焦げた」に変更．
	* 特性「人目を盗む能力」の訳語を「隠密行動」に変更．
	* 日本語化されたNetHack brassを参考にした訳語一括修正．
	* クエストの文章を一括改訂．

Sun Jan 21 2007  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F3-J0.1.5リリース
	* ベースをSLASH'EM-0.0.7E7F3にした．
		-fixed bug 1549137: Technique cutthroat is effective to
		                    no-head monster (Michael DiPietro)
		-fixed bug 1288590: Potion of hallucination cannot be
		                    identified by drinking
		-fixed bug 1482524: alchemy with gain level asymmetric
		                    (Michael DiPietro)
		-fixed bug 1546379: Fedora & charisma (Michael DiPietro)
		-Actioned feature request 1504905: Add french translation of
		 manual page
		-fixed bug 1624174: koala bugs steed
		-fixed bug 1539448: rotten wraith corpses GPF
		-fixed bug 1521400: Dungeon collapsed returning to full level
		-fixed bug 1211513: Upgraded objects retain names, even
		                    artifact names
		-fixed bug 1554633: lichen, grown on a dead pet corpse,
		                    inherits pet's name
		-fixed bug 1521471: Medical kit contents can leak
		-fixed bug 1609542: Reaper + eyes of overworld
		-fixed bug 1503394: Conduct= is not always written to logfile
		-fixed bug 1558274: Light source weapons not working properly
		-fixed bug 1602002: dmonsfree when using travel command &
		                    burdened
	* dat/history と比べて dat/jhistory の内容が古かったので翻訳しなお
	  した．
	* JNetHackの修正取込：#9435 do.c の訳語について
	* JNetHackの修正取込：#9436 店の商品を店の外から蹴り壊した時のメッ
	                      セージ
	* JNetHackの修正取込：訳語のrefine

Sat Nov 11 2006  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F2-J0.1.4リリース
	* Visual Studio 2005 Express Editionでコンパイル可能なように修正．
	* 報告バグの修正：#8490 X11 port での Status 表示がおかしい
	* 報告バグの修正：#8887 こするものを持っていないときに#rubしたとき
	* JNetHackの修正取込：#8886 投げ矢の罠にかかったとき
	* JNetHackの修正取込：#9078 ノルンのセリフで「娘」が「妹」になって
	  いる
	* JNetHackの修正取込：#9148 盲目攻撃を受けたときのメッセージ
	* "lock"の訳語を違和感ない程度に「錠」になるように修正．
	* WIN32においてキャラ名にいわゆる全角文字を使ったとき，セーブファ
	  イル名が化けていた問題に対処．
	* その他細かいところを修正．
	* 本家修正先取り
		-inserted missing #ifdef resulting in compilation of pure ansi
		 tty version for DOS failure.
		-fixed bug 1461660: Misaligned door
		-fixed bug 1449238: #youpoly takes a turn when denied
		-fixed bug 1432473: help for Luggage shows help for Lug/Lugh
		-fixed bug 1428520: Poisoned ID'd weapon gives description
		                    as well as ID
		-fixed bug 1375292: help for wombat shows text for bat
		-fixed bug 1375286: pummel strikes nothing, inside ochre jelly
		-fixed bug 1355891: Cannot wish for multiple potions of
		                    vampire blood
		-fixed bug 1314114: Shopkeepers cant see Shirts through
		                    invisible armour
		-fixed bug 1304954: You can Name the Sword of Balance
		-fixed bug 1223533: Typo error in the Install.X11 file
		-fixed bug 1407275: Wrong weight of medical kits
		-fixed bug 1213658: Death message missing with gas spore
		                    explosion when blind
		-fixed bug 1291161: #force command doesn't work on locked box
		-fixed bug 1223632: #force on shop doors
		-fixed bug 879247: gender & alignment select dont focus on
		                   "random" button
		-fixed bug 943553: Eating off floor while levitating
		-fixed bug 1432411: Knew the colour of a potion thrown on me,
		                    when blind
		-fixed bug 1449159: DEFAULT_WINDOW_SYS wrong if only X11
		                    enabled
		-fixed bug 1400802: wax golems don't emit light
		-fixed bug 1586216: USE_REGEX_MATCH fails on Solaris 10
		-fixed bug 1569015: Redundant code in percent_success()
		-fixed bug 1583978: Grammer error in broken celibacy message
		-fixed bug 1444749: character wearing invisible blindfold
		                    can't see

Sat Apl 08 2006  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F2-J0.1.3リリース
	* 報告バグの修正：#8117 神の名前が表示されない
	* 報告バグの修正：#8194 透明な物体を可視の薬に浸した際のメッセージ
	* 報告バグの修正：#8210 表記揺れ：ヴラド侯/候
	* 「あなたは～」が不適切ないくつかのメッセージを修正．
	* 魔法エネルギーを表す"energy"の訳語を「魔力」に統一．
	* "knapsack"の訳語を「ナップザック」に統一．
	* その他幾つかの訳語を修正．
	* "！"を表示すべき部分で表示してない場合があったため修正．
	* 燃えやすい素材のゴーレムに変身中に燃やされたときの訳文抜け対応
	* JNetHackの修正取込：rename polearms
	* JNetHackの修正取込：jtrnsobj.datのrefine
	* JNetHackの修正取込：#7524 メッセージ修正案色々
	* JNetHackの修正取込：#7740 日本語でアーティファクト名がつけられな
	  い
	* JNetHackの修正取込：#7817 盲目時に指輪を流し台に落とした時のメッ
	  セージ
	* JNetHackの修正取込：#7820 アスクレピオスの杖
	* JNetHackの修正取込：#7860 the Tomb of the Toltec Kings
	* JNetHackの修正取込：#7962 結びつけられた紐を置こうとしたとき
	* JNetHackの修正取込：#7966 Queen Beruthiel

Tue Dec 11 2005  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F2-J0.1.2リリース
	* ベースをSLASH'EM-0.0.7E7F2にした．
	  -fixed bug 215062: no black color
	                     Extends videocolors option,
	                     see Guidebook for details.
	  -fixed bug 1179557: Found egg marked (laid by you)
	  -fixed bug 1206099: Torches are not extinguished with rust traps
	  -fixed bug 1237780: gtk window port fails to build with gcc-4.0
	  -fixed bug 1244468: different spitting venom when polymorph
	  -fixed bug 1294907: 'Raise Zombies' technique cannot raise dwarf
	                      zombies! (Xeno)
	  -fixed bug 1355972: Monsters Don't gain Drain resistance due to
	                      worn items! (Xeno)
	* JNetHackの修正取込：#7413 乗馬中にリッチに鞍を呪われたとき
	* JNetHackの修正取込：#7524 メッセージ修正案色々(一部)
	* JNetHackの修正取込：#7570 ダイアモンド or ダイヤモンド ?
	* JNetHackの修正取込：#7571 中身を指定した缶と卵を日本語で願う
	* JNetHackの修正取込：#7701 ジンの出現場所がないとき

Mon Oct 03 2005  Masaki Miyaso  <so-miya@sourceforge.jp>
	* 本家修正先取り：ttycolorsに関するrefine
	* JNetHackの修正取込：#7034 シャツの上からシャツを着ようとした時

Sun Sep 18 2005  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F1-J0.1.1リリース
	* 店のサービス"Weapon-works"の訳を"武器関係"から"武器加工"に．
	* 店のサービス"Armor-works"の訳を"防具関係"から"防具加工"に．
	* DJGPPでコンパイルするためのバッチファイル等を修正．
	* オプション変更の和訳追加．
	* 「透明な～」「吸い尽くした～」等のSLASH'EM独自のアイテムの日本
	  語形容詞が願いに使えなかった問題を修正．
	* 数詞に関する修正．
	* 本家修正先取り
	  - bug 933587: One-Eyed Sam referred to as "it"
	  - bug 934078: Mold "wiggle" out of bag
	  - bug 988827: Monster wearing CoI left invisible corpse
	  - bug 1104713: Pet and Frozen food stores can charge
	  - bug 1223222: Female Bulls??
	  - bug 1223226: Some creatures are missing horns
	  - bug 1231766: Food shown in Medical Kits
	  - bug 1232707: engraving with lightsabers
	  - bug 1239041: engrave dialog doesn't show lightsabers
	  - bug 1238060: grammatical error when burning through a door
	                 (Benjamin Schieder)
	  - bug 1240794: Pets don't grow up properly
	  - bug 1240799: Steed ridden onto poly trap polymorphs
	                 permanently
	  - bug 1243904: chameleon as statue trap
	  - bug 1244470: shopkeeper don't have shotgun
	* JNetHackの修正取込：jdata.baseの改行位置の統一．
	* JNetHackの修正取込：#6437 DOS版3.4.3-0.6，0.5で文字化け
	* JNetHackの修正取込：#6437 鍵のかかった宝箱を斧でこじ開けて壊し
	  たとき
	* JNetHackの修正取込：コードrefine
	* JNetHackの修正取込：訳語refine．motalの訳語を「定命の者」に統
	  一，他．
	* JNetHackの修正取込：#6556 rumor/true.txt 誤字
	* JNetHackの修正取込：「#6580 店の損失を補填したとき」に関連する
	  mhisを使用している部分でおかしな部分を修正．
	* JNetHackの修正取込：#6567 アイテムを盗んだ時のメッセージ
	* JNetHackの修正取込：#6641 ｢透明の薬｣が願えない

Mon Jul 5 2005  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7F1-J0.1.0リリース
	* ベースをSLASH'EM-0.0.7E7F1にした．
	  - bug 924811: Fire vampires leave corpses
	  - bug 1028948: casting stone to flesh on a gargoyle
	  - bug 1157414: Do not "loose" faith
	  - bug 1185759: Shopkeeper "looks at corpse" through wall
	  - bug 1193070: Engraving with a wand of draining keeps
	                 illiterate conduct (Nico Kist)
	  - bug 1197822: necromancer zombie's hp
	  - bug 1205563: suppress_alert still says NetHack
	  - bug 1212903: Int and Wis stats not correct (in obscure case)
	  - bug 1213881: Lieutenants lack Firearms
	* オプションヘルプ/オプション変更の和訳．
	* 死因メッセージの修正．
	* 報告バグの修正：#6143 Mingw + CC1_SJにおける英和テーブルの文
	  字化け対応
	* /win/X11にJNetHackパッチを中途半端に当てたときの残骸が残ってい
	  たので整理．SLASH'EMとNetHackでは微妙に違うのでパッチがあたっ
	  ていませんでした．X11LARGETILE機能を完全に削除しました．
	* 本家修正先取り：Remove redundant code.
	* JNetHackの修正取込：jdata.baseの見出し単語の一部を追加変更．
	* JNetHackの修正取込：jrumors.falの修正．
	* JNetHackの修正取込：動詞語尾変化のrefine．
	* JNetHackの修正取込：水中でつるはしや斧を振り回したときのメッセ
	  ージの修正
	* JNetHackの修正取込：日本語でアエスキュラピスの杖を願うとランダ
	  ムな杖が出てくる問題の修正．
	* JNetHackの修正取込：モンスターに殺された時の墓碑銘の修正．

Sun May 15 2005  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7-J0.0.13リリース
	* GTKモードで日本語が使えるように修正．
	* WindowsNTのGCC用Makefileを修正．
	* 報告バグの修正：#5765 鍵の掛かった箱を武器を用いてこじあけるこ
	  とができない問題を修正．
	* ブラックマーケットでモンスターがレベルテレポートに失敗したとき
	  のメッセージの未訳を修正．
	* 技能名一覧に6文字未満の技名のみが存在している場合でも表示が乱
	  れないように修正．
	* 薬箱を使用した技能を発動させたときのアイテムの和訳を和訳テーブ
	  ルを通して表示するようにした．
	* 報告バグの修正：#5673 knock'em deadの和訳を修正した．
	* 本家修正先取り：「魅了の魔法」を使用したり「怪物を飼いならす巻
	  物」を読んだときに「アンデッドを使役する魔法」が同時に発動した
	  り，怪物に飲み込まれているときに前述の動作や「アンデッドを使役
	  する魔法」を使用すると虐殺まで発動してしまう問題に対処．
	* JNetHackの修正取込：jquest.txt読込時の一行あたりの最大行数を80
	  文字から256文字に修正．
	* JNetHackの修正取込：getobjおよびggetobj関数をtrans_verb関数を
	  用いるように修正．

Mon Feb 21 2005  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSLASH'EM-0.0.7E7-J0.0.12リリース
	* ベースをSLASH'EM-0.0.7E7にした．それに伴いいくつかの和訳を追加．
	* 技能の日本語名称をjtrnsobj.datに記述するように修正．
	* ジプシーによる占いの実現が出来ないときのメッセージを修正．
	* JNetHackの修正取込：jdata.baseの見出し単語の一部を追加変更．
	* JNetHackの修正取込：ジョークモンスターの訳語を一部変更．
	* JNetHackの修正取込：gold pieceの訳語を金貨に統一．
	* JNetHackの修正取込： "/"コマンドで画面上のシンボルを調べる時，
	  アイテムや地形の場合でも表示されるように修正．

Mon Jan 10 2005  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSlash'EM-0.0.7E6F3-J0.0.11リリース
	* 落し扉や穴に浮遊や飛行などの落ちない状況であっても落ちてしまっ
	  ていた問題を修正．
	* 鏡に関するメッセージの修正．
	* 溶岩に岩を落としたときのメッセージの修正．
	* 怪物がひどく傷んだ武器で攻撃し，それが劣化しそうになったときの
	  メッセージの修正．
	* 光のともった光源を鞄等にしまうときの灯を消した旨のメッセージの
	  修正．
	* 解放の印を唱えたときのメッセージの修正．
	* ペットの吸血鬼が死体の血を吸っているのを目撃したときのメッセー
	  ジを修正．
	* 変化したアイテムを能力回復の薬につけたときのメッセージを修正．
	* いくつかの句点と感嘆符の取り違えを修正．
	* 幻覚中に自分に対して鏡を使ったときのメッセージを修正．
	* 本家1066530の修正取込：投擲アイテムを拾ったときまとまらない問題
	  を修正．
	* JNetHackの修正取込：紐で結ばれたモンスターにさらに紐を結ぼうと
	  したときのメッセージをJNetHackと同一に修正．
	* JNetHackの修正取込：アイテム別の数詞の選択を関数に変更．
	* JNetHackの修正取込：量子力学者の攻撃で視界内の怪物が飛ばされた
	  ときのメッセージの修正．
	* JNetHackの修正取込：目隠し中に呪われたタオルを使ったときのメッ
	  セージを修正．
	* JNetHackの修正取込：いくつかの訳語のリファイン．
	* JNetHackの修正取込：日本語でデータベースが呼び出せない場合があ
	  る問題を修正．
	* コマンド一覧(jhh)を修正．
	* ゲームのオプション一覧(長文)(jopthelp)を修正．
	* Slash'EMの簡単な歴史(jhistory)をメニュー名称と共に修正．
	* ゲームおよびコマンドの解説(jhelp)を修正．
	* このキーが何を意味するかの説明(jcmdhelp)を修正．
	* Wizard用ヘルプ(jwizhelp)を修正．
	* 神託の和訳を修正．
	* 正しい噂の和訳ファイルを一部修正．
	* jGuidebook.mnの未訳部分を翻訳．jGuidebook.txtの再生成．
	* いくつかのファイルのコメント方法を修正．

Sat Nov 20 2004  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSlash'EM-0.0.7E6F3-J0.0.10リリース
	* SLASHEM-0.0.7E6F3ベースに移行．
	* JNetHackの修正取込："a disintegration blast"の訳語が「死の光線
	  」になっていたので「分解の息」に修正．
	* コアラの攻撃メッセージのを「なごんだ」から「落ち着いた」に修正．
	  また，ペット等がコアラに攻撃された場合のメッセージの訳抜けを修
	  正．
	* JNetHackの修正取込：モンスター同士の戦闘，罠で死んだモンスター
	  が死んだとき，"r* in peace"と表示しているところの訳文を修正．
	* JNetHackの修正取込：スリングで石を撃ったときの数詞を追加．
	* JNetHackの修正取込：紐で結ばれたモンスターにさらに紐を結ぼうと
	  したときのメッセージを修正．
	* JnetHackの修正取込：所持数以上の数を指定して品物を置こうとした
	  時のメッセージを修正．

Fri Oct 31 2004  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSlash'EM-0.0.7E6F2-J0.0.9リリース
	* jdata.baseの見出し語を修正．
	* jdata.baseのpiercerの訳文を修正．
	* 使用中の特殊技能の表記が"使用可"になっていたため，これを"使用中"
	  に修正．
	* 使えなくなった特殊技能の表記を"リミット"から"限界"に変更．
	* アイテム名の返却関数の日本語化作業における修正ミスで，人形の名
	  称を表示していなかった問題を修正．
	* 魔法のろうそくなど，いくつかの道具の表示数詞を補正．
	* ユーカリの葉など，いくつかの食料において表示数詞を分けて表示す
	  るように変更．
	* ヴェクナの手の素アイテムである"severed hand"の訳文を「重い手」
	  から「切断された手」に修正．
	* JNetHackの修正取込：大地の巻物で柔らかい兜をかぶった怪物を巻き
	  込んだとき，怪物の兜と間違えてプレイヤーの兜の名称を表示してし
	  まう問題を修正．
	* JNetHackの修正取込：コカトリスの死体に触れて石になったときの死
	  亡メッセージを修正．
	* JNetHackの修正取込：句読点および！マーク抜けの修正．
	* JNetHackの修正取込：モンスター名「曇の霧」から「雲の霧」に変更．
	* JNetHackの修正取込：スキル名「乗馬」から「騎乗」に，「クロスボ
	  ウ」から「クロスボゥ」に変更．
	* JNetHackの修正取込：罠の名前「岩落しの罠」から「落石の罠」に，
	  「落岩の罠」から「巨岩の罠」に変更．
	* コアラの攻撃メッセージの訳抜けの修正．
	* マストドンのjdata.base修正．
	* JNetHackの修正取込：怪物同士での毒攻撃における防御メッセージの
	  修正．
	* 魔法選択アルファベットを切り替えるときの魔法名の訳抜け修正．
	* JNetHackの修正取込：精霊界でちょっと距離の離れた高僧をlook up
	  で見たときのモンスター名が未訳だったのを修正．
	* モンスターがモンスターに酸で攻撃されたときのメッセージの修正．
	* 反射した光線を反射し返されたときのメッセージの修正．
	* JNetHackの修正取込：Windowsのttyモードで存在しないオプションを
	  default.nhに指定したときに一般保護エラーになる問題の修正．

Fri Aug 27 2004  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSlash'EM-0.0.7E6F2-J0.0.8リリース
	* JNetHack-3.4.3-0.4のリリースに伴い0.3との差分を適用．
	* 特殊技能「武器落し」で相手の武器を奪ったときに落ちる危険性があ
	  った問題を修正．
	* 炎の魔術師のクエスト情報に一部誤りがあり，これを修正．
	* ドッグフードを食べたときの最後のメッセージを変更．
	* ドッペルゲンガーが怪物に変身するときのメッセージを微妙に変更．
	* 泉でエクスカリバーを生成したときのメッセージを変更．
	* ナイトメアレベルや衛士クエストの名のついたアイテムの名称を翻訳．
	* 飲み込み怪物に飲み込まれて死んだときの啓蒙メッセージを修正．
	* 蜘蛛人間が変身した時のメッセージを翻訳．
	* "body spells"と"protection spells"の翻訳を"肉体"と"護り"に統一．
	* "Magic Memory Vault"の翻訳を「魔法のメモリー倉庫」に統一．
	* "gas spore"の和訳を「包子ガス」から「胞子ガス」に変更．
	* 大アルカナ"Judgement"の誤訳を修正（『正義』→『審判』）．
	* ドラゴンがブレスを吐いたときのメッセージを修正．
	* drains (life)の翻訳を「生命力吸収」に統一．
	* JNetHackの修正取込：heavy iron ballの翻訳を「重い鉄球」に統一．
	* 店のサービスのアイテム選択メッセージを修正．
	* 本家のバグである，鉄球を引きずった状態でディスプレーサービース
	  トと位置交換したときにplace_object: obj not freeで例外となって
	  しまう問題を仮修正．
	* 特殊技能「魔力収集」の終了メッセージを修正．
	* 翻訳編集時のミスでヒツジの鳴き声が表示されなくなっていたのを修
	  正．
	* jdata.baseの検索ユニークモンスター名称が『』に対応していなかっ
	  たので対応．
	* JNetHackの修正取込：氷の渦に変化して怪物を飲み込んでいる時のメ
	  ッセージを修正．
	* JNetHackの修正取込：ワルキューレクエストに訪れたときのメッセー
	  ジを修正．

Sat Jul 31 2004  Masaki Miyaso  <so-miya@sourceforge.jp>
	* JSlash'EM-0.0.7E6F2-J0.0.7リリース
	* 起動時に自動リカバーするオプションを有効化
	* クエスト中のメッセージやコメント行が80文字を超えてしまい，クエ
	  ストネメシス等のメッセージ表示に不具合が出ていた問題を修正．
	* 句読点，クエスチョン，エクスクラメーションの統一．
	* 和訳テーブルに"Azog the Orc King"の訳を追加．
	* 手榴弾やガス弾を使用したときのメッセージ，売り物を使用したとき
	  のメッセージ，使用中のアイテム状態の表示を修正した．
	* J0.0.6から入れたYou_hearにて，文字列領域の未初期化バグを修正．
	* JNetHackの修正取込：変化する怪物を日本語で入力できるようにする．
	* 啓蒙時の「能力維持の属性」，「今の姿から変化できない属性」の表
	  示形式を変更．
	* 「美しい 水のニンフ」等，モンスターの修飾語の後にスペースが入
	  る問題を修正．

Sun Jul 25 2004  Masaki Miyaso  <so-miya@sourceforge.jp>

	* JSlash'EM-0.0.7E6F2-J0.0.6リリース
	* カーソルによる位置指定時のヘルプ「背景シンボルによる移動」の訳
	  の修正．
	* UNIXのMakefileにおけるjGuidebookとjgypsy.txtに関する処理の記述
	  抜けの修正．
	* 衝撃の魔法等での怪物への命中時のメッセージの未訳を修正．
	* ジプシーのスリーカード勝利時のメッセージの修正．
	* asid potionの和訳が「酸の」になっていたためこれを修正．
	* 特殊技能「武器落とし」を学習したときのメッセージの修正．
	* 冒険者の攻撃を怪物が防具によって防いだときのメッセージの修正．
	* 怪物の攻撃が怪物の防具によって防がれたときのメッセージの修正．
	* 店の扉を破壊したときのメッセージの修正．
	* 敵対的な吸血鬼との会話文の修正．
	* 盗賊の特殊技能「クリティカル」の未訳の修正．
	* 特殊技能を全く持っていない状態での特殊技能を使用しようとしたと
	  きのメッセージの未訳を修正．
	* 床の文字や墓石を発見したときの補助メッセージの修正．
	* 毒吹きの魔法や毒の息が冒険者に命中したときのメッセージの修正．
	* 名前の付いたモンスターにその名前を継承したモルドが生えたときの
	  メッセージにて，名前のみではなくモンスター名(モルド)も表示する
	  ように修正．
	* ユニコーンが宝石を受け取ったときのメッセージの修正．
	* bullの和訳名の牡牛の後ろについていた半角スペースを除去．
	* 呪われた軽量化の鞄の中で未識別なアイテムが消滅したときのメッセ
	  ージを修正．
	* アンデッドを使役する魔法書が登場したことによる魔法書一覧のズレ
	  の修正．
	* 指輪の強化値の前に入っている半角スペースを削除．武器，防具等の
	  強化値の前には半角スペースは入っていないため，統一した．
	* vコマンドによるバージョン情報にて日本語版のバージョン情報を出す
	  ように修正．
	* モンスター名にて和訳変換テーブルを通していないものがいくつかあ
	  ったため，これを修正．
	  - 怪物の装備しているコカトリスの死体をガントレットを装備して
	    いない状態で 鞭で奪ったとき
	  - ババウに凝視されて死んだとき
	  - 飲み込み攻撃を持つ怪物に変身して爆発する怪物を飲み込んで，
	    飲み込んだ怪物が爆発してそのダメージで死んだとき
	  - アンデットスレイヤーで，変化制御能力なしでアンデッドに変化
	    しそうになったとき
	  - 怪物に変化しているときに，敵対的な吸血鬼に話し掛けたとき
	  - 紫ワームに変身して緑スライムを飲み込んだとき
	  - 金庫の番人に魔法のメモリー倉庫の外で殺されたときの倉庫に建
	    てられる墓のメッセージに含まれる冒険者の変身していた怪物名
	* 指がぬるぬるしていたときの啓蒙のメッセージの修正．
	* 犬や猫の缶詰を食べて反感を買ったときのメッセージの修正．
	* JNetHackの修正取込：まとまったアイテムを重さの関係上分けて持ち
	  上げたときのメッセージの修正．
	* JNetHackの修正取込：/コマンド等での水中・溶岩中のアイテム表示
	  の修正．
	* JNetHackの修正取込：墓場のある階における幻覚時のメッセージの修
	  正．
	* 冒険者を飲み込んでいる怪物を石化させたときのメッセージの修正．
	* You_hearを使用している箇所における和訳の不具合の修正．
	* 飛び道具を投げられて死んだときの墓の死因表示の不具合の修正．

Sat Jul 10 2004  Masaki Miyaso  <so-miya@sourceforge.jp>

	* JSlash'EM-0.0.7E6F2-J0.0.5リリース
	* JNetHackの修正取込：幻覚時の薬を混ぜたときのメッセージ修正
	* JNetHackの修正取込：考古学者クエストの床文字の和訳
	* ユニークモンスターの名称統一：今回は仮に固有名のみ『』で囲む
	  ことにしました．
	* 職業「郷士」を「衛士」に変更．
	* 衛士のクエストの会話にて，「ビーフェザー」を「ビーフィーター」
	  に変更．
	* アイテム，モンスターの和訳変換テーブルを大幅修正．
	* 「常闇の海」フロアの床メッセージを修正．
	* SLASH'EMで追加された獣人の性別名称が表示されるようにした．
	* 啓蒙表示を修正：消化不良の指輪装備時．
	* 簡易啓蒙表示にて職業と性別を翻訳．
	* 過去に食べたことのあるモンスターと同じシンボルのモンスターの
	  肉の缶詰を食べたときの記憶に関するメッセージの和訳変換ミスの
	  修正．
	* 死因「裏切られて死んだ」が抜けていたことによる死因・脱出メッセ
	  ージのずれていたのを修正．
	* 冒険者が吸血鬼に血を吸われたときのメッセージを翻訳．
	* 名前のついたアイテム／未識別の聖器の名前が表示されていなかった
	  のを修正．
	* 透明なアイテム，霞んだアイテムの修飾語を翻訳．
	* 能力回復の薬を飲んだときのメッセージを修正．
	* テレパシーを得たときのメッセージを修正．
	* "mage"の和訳が統一されていなかったので「魔術師」に統一．
	* 職業「修行僧」の称号「木の習い手」を「風の習い手」に修正．
	* ドッグフード等を食べたときの吐き気の連鎖メッセージにて「あなた
	  は」が重複していたのを修正．
	* ゲーム起動時のコピーライトを追加．

Sun Jul  4 2004  Masaki Miyaso  <so-miya@sourceforge.jp>

	* JSlash'EM-0.0.7E6F2-J0.0.4リリース
	* 弾丸の数詞を「発分の」にした
	* 銃器の数詞を「丁の」にした
	* defsyms[].explanationの訳抜け修正
	* 銃器のセレクター選択時のバグを修正
	* UNIXの場合の設定ファイルを，.jslashemrc，.jnethackrcの順に読み
	  込むように修正．
	* steel bootsの訳語「鋼鉄の靴」を追加．
	* 店の鑑定サービスのメッセージを修正．
	* ペットが金貨拾うと落ちるバグを修正．

Fri Jul  2 2004  Masaki Miyaso  <so-miya@sourceforge.jp>

	* JSlash'EM-0.0.7E6F2-J0.0.3リリース
	* Makefile.mscとMakefile.bccにてMSWINタイルモードでコンパイルでき
	  るようにした．
	* jquest.txtのSlash'EM独自部分の翻訳が完了した．
	* 翻訳修正

Mon Jun 28 2004  Masaki Miyaso  <so-miya@sourceforge.jp>

	* JSlash'EM-0.0.7E6F2-J0.0.2リリース
	* Slash'EM独自の未訳部分を翻訳
	* Makefile.mscを使用できるように修正
	* 翻訳追加修正

Wed Jun 23 2004  Masaki Miyaso  <so-miya@sourceforge.jp>

	* JSlash'EM-0.0.7E6F2-J0.0.1暫定リリース
	* JNetHack-3.4.3-0.3をSlash'EM-0.0.7E6F2に適用
	* Makefile.bccを使用できるように修正
	* 翻訳追加修正
