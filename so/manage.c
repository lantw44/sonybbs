/*-------------------------------------------------------*/
/* manage.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 看板管理				 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;


#ifdef HAVE_TERMINATOR
/* ----------------------------------------------------- */
/* 站長功能 : 拂楓落葉斬				 */
/* ----------------------------------------------------- */


extern char xo_pool[];


#define MSG_TERMINATOR	"《拂楓落葉斬》"

int
post_terminator(xo)		/* Thor.980521: 終極文章刪除大法 */
  XO *xo;
{
  int mode, type;
  HDR *hdr;
  char keyOwner[80], keyTitle[TTLEN + 1], buf[80];

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_FOOT;

  mode = vans(MSG_TERMINATOR "刪除 (1)本文作者 (2)本文標題 (3)自定？[Q] ") - '0';

  if (mode == 1)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyOwner, hdr->owner);
  }
  else if (mode == 2)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyTitle, str_ttl(hdr->title));		/* 拿掉 Re: */
  }
  else if (mode == 3)
  {
    if (!vget(b_lines, 0, "作者：", keyOwner, 73, DOECHO))
      mode ^= 1;
    if (!vget(b_lines, 0, "標題：", keyTitle, TTLEN + 1, DOECHO))
      mode ^= 2;
  }
  else
  {
    return XO_FOOT;
  }

  type = vans(MSG_TERMINATOR "刪除 (1)轉信板 (2)非轉信板 (3)所有看板？[Q] ");
  if (type < '1' || type > '3')
    return XO_FOOT;

  sprintf(buf, "刪除%s：%.35s 於%s板，確定嗎(Y/N)？[N] ", 
    mode == 1 ? "作者" : mode == 2 ? "標題" : "條件", 
    mode == 1 ? keyOwner : mode == 2 ? keyTitle : "自定", 
    type == '1' ? "轉信" : type == '2' ? "非轉信" : "所有看");

  if (vans(buf) == 'y')
  {
    BRD *bhdr, *head, *tail;
    char tmpboard[BNLEN + 1];

    /* Thor.980616: 記下 currboard，以便復原 */
    strcpy(tmpboard, currboard);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do				/* 至少有 note 一板 */
    {
      int fdr, fsize, xmode;
      FILE *fpw;
      char fpath[64], fnew[64], fold[64];
      HDR *hdr;

      xmode = head->battr;
      if ((type == '1' && (xmode & BRD_NOTRAN)) || (type == '2' && !(xmode & BRD_NOTRAN)))
	continue;

      /* Thor.980616: 更改 currboard，以 cancel post */
      strcpy(currboard, head->brdname);

      sprintf(buf, MSG_TERMINATOR "看板：%s \033[5m...\033[m", currboard);
      outz(buf);
      refresh();

      brd_fpath(fpath, currboard, fn_dir);

      if ((fdr = open(fpath, O_RDONLY)) < 0)
	continue;

      if (!(fpw = f_new(fpath, fnew)))
      {
	close(fdr);
	continue;
      }

      fsize = 0;
      mgets(-1);
      while (hdr = mread(fdr, sizeof(HDR)))
      {
	xmode = hdr->xmode;

	if ((xmode & POST_MARKED) || 
	  ((mode & 1) && strcmp(keyOwner, hdr->owner)) ||
	  ((mode & 2) && strcmp(keyTitle, str_ttl(hdr->title))))
	{
	  if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
	  {
	    fclose(fpw);
	    close(fdr);
	    goto contWhileOuter;
	  }
	  fsize++;
	}
	else
	{
	  /* 若為看板就連線砍信 */

	  cancel_post(hdr);
	  hdr_fpath(fold, fpath, hdr);
	  unlink(fold);
	}
      }
      close(fdr);
      fclose(fpw);

      sprintf(fold, "%s.o", fpath);
      rename(fpath, fold);
      if (fsize)
	rename(fnew, fpath);
      else
  contWhileOuter:
	unlink(fnew);
    } while (++head < tail);

    /* 還原 currboard */
    strcpy(currboard, tmpboard);
    return XO_LOAD;
  }

  return XO_FOOT;
}
#endif	/* HAVE_TERMINATOR */


/* ----------------------------------------------------- */
/* 板主功能 : 修改板名					 */
/* ----------------------------------------------------- */


static int
post_brdtitle(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  /* itoc.註解: 其實呼叫 brd_title(bno) 就可以了，沒差，蠻幹一下好了 :p */
  if (vans("是否修改中文板名敘述(Y/N)？[N] ") == 'y')
  {
    vget(b_lines, 0, "看板主題：", newbrd.title, BTLEN + 1, GCARRY);

    if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
    {
      memcpy(oldbrd, &newbrd, sizeof(BRD));
      rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
      return XO_HEAD;	/* itoc.011125: 要重繪中文板名 */
    }
  }

  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* 板主功能 : 修改進板畫面				 */
/* ----------------------------------------------------- */


static int
post_memo_edit(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  mode = vans("進板畫面 (D)刪除 (E)修改 (Q)取消？[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, fn_note);

    if (mode == 'd')
    {
      unlink(fpath);
      return XO_FOOT;
    }

    if (vedit(fpath, 0))	/* Thor.981020: 注意被talk的問題 */
      vmsg(msg_cancel);
    return XO_HEAD;
  }
  return XO_FOOT;
}

#ifdef POST_PREFIX
/* ----------------------------------------------------- */
/* 板主功能 : 修改發文類別                               */
/* ----------------------------------------------------- */


static int
post_prefix_edit(xo)
  XO *xo;
{
#define NUM_PREFIX      9
  int i;
  FILE *fp;
  char fpath[64], buf[20], prefix[NUM_PREFIX][20], *menu[NUM_PREFIX + 3];
  char *prefix_def[NUM_PREFIX] =   /* 預設的類別 */
  {
    "[公告]", "[測試]", "[閒聊]", "[灌水]", "[無聊]",
	 	"[打混]", "[紀念]", "[創作]", "[文件]"
  };
	char modified=0; /* lantw44: 判斷有無修改過 */

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  i = vans("類別 (D)刪除 (E)修改 (Q)取消？ [E] ");

  if (i == 'q')
    return XO_FOOT;

  brd_fpath(fpath, currboard, "prefix");

  if (i == 'd')
  {
    unlink(fpath);
    return XO_FOOT;
  }

  i = 0;

  if (fp = fopen(fpath, "r"))
  {
    for (; i < NUM_PREFIX; i++)
    {
      //if (fscanf(fp, "%10s", buf) != 1)
			if (fgets(buf, 11, fp) == NULL)
        break;
      buf[strlen(buf)-1]='\0'; /* lantw44:吃掉最後一個\n */
			sprintf(prefix[i], "%d.%s", i + 1, buf);
    }
    fclose(fp);
  }

  /* 填滿至 NUM_PREFIX 個 */
  for (; i < NUM_PREFIX; i++)
    sprintf(prefix[i], "%d.%s", i + 1, prefix_def[i]);

  menu[0] = "10";
  for (i = 1; i <= NUM_PREFIX; i++)
    menu[i] = prefix[i - 1];
  menu[NUM_PREFIX + 1] = "0.離開";
  menu[NUM_PREFIX + 2] = NULL;

  do
  {
    /* 在 popupmenu 裡面按 左鍵 離開 */
    i = pans(3, 20, "文章類別", menu) - '0';
    if (i >= 1 && i <= NUM_PREFIX)
    {
      strcpy(buf, prefix[i - 1] + 2);
      if (vget(b_lines, 0, "類別：", buf, 10, GCARRY)){
				if(strcmp(prefix[i - 1] + 2,buf)){
					modified=1;
				}
        strcpy(prefix[i - 1] + 2, buf);
			}
    }
  } while (i);

	if(!modified || vans("確定要儲存檔案嗎(Y/N)？[Y] ") == 'n'){
		vmsg("原封不動");
		return XO_FOOT;
	}

  if (fp = fopen(fpath, "w"))
  {
    for (i = 0; i < NUM_PREFIX; i++)
      fprintf(fp, "%s\n", prefix[i] + 2);
    fclose(fp);
		vmsg("更新完畢");
  }else{
		vmsg("檔案寫入失敗");
	}

  return XO_FOOT;
}
#endif      /* POST_PREFIX */
/* ----------------------------------------------------- */
/* 板主功能 : 修改發文綱領                               */
/* ----------------------------------------------------- */


static int
post_postlaw_edit(xo)       /* 板主自定文章發表綱領 */
  XO *xo;
{
  int mode;
  char fpath[64];

  mode = vans("文章發表綱領 (D)刪除 (E)修改 (Q)取消？[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, FN_POSTLAW);

    if (mode == 'd')
    {
      unlink(fpath);
      return XO_FOOT;
    }
    if (vedit(fpath, 0))      /* Thor.981020: 注意被talk的問題 */
      vmsg(msg_cancel);
    return XO_HEAD;
  }
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* 板主功能 : 看板屬性					 */
/* ----------------------------------------------------- */


#ifdef HAVE_SCORE
static int
post_battr_noscore(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("開放評分 (1)允許\ (2)不許\ (Q)取消？[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_NOSCORE;
    break;
  case '2':
    newbrd.battr |= BRD_NOSCORE;
    break;
  default:
    return XO_FOOT;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_FOOT;
}
#endif	/* HAVE_SCORE */


#ifdef ANTI_PHONETIC
static int
post_battr_nophonetic(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("1)可以用注音文 2)不可用注音文 [Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_NOPHONETIC;
    break;
  case '2':
    newbrd.battr |= BRD_NOPHONETIC;
    break;
  default:
    return XO_FOOT;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_FOOT;
}
#endif      /* ANTI_PHONETIC */
/* ----------------------------------------------------- */
/* 板主功能 : 修改板主名單				 */
/* ----------------------------------------------------- */


static int
post_changeBM(xo)
  XO *xo;
{
  char buf[80], userid[IDLEN + 2], *blist;
  BRD *oldbrd, newbrd;
  ACCT acct;
  int BMlen, len;

  oldbrd = bshm->bcache + currbno;

  if (strcmp(oldbrd->class, "個人") &&
  	  strcmp(oldbrd->class, "社團") &&
	    strcmp(oldbrd->class, "團體"))
  {
	  vmsg ("公開板不給改喲");
	  return XO_FOOT;
	}
	
  blist = oldbrd->BM;
  if (is_bm(blist, cuser.userid) != 1)	/* 只有正板主可以設定板主名單 */
    return XO_FOOT;

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  move(3, 0);
  clrtobot();

  move(8, 0);
  prints("目前板主為 %s\n請輸入新的板主名單，或按 [Return] 不改", oldbrd->BM);

  strcpy(buf, oldbrd->BM);
  BMlen = strlen(buf);

  while (vget(10, 0, "請輸入副板主，結束請按 Enter，清掉所有副板主請打「無」：", userid, IDLEN + 1, DOECHO))
  {
    if (!strcmp(userid, "無"))
    {
      strcpy(buf, cuser.userid);
      BMlen = strlen(buf);
    }
    else if (is_bm(buf, userid))	/* 刪除舊有的板主 */
    {
      len = strlen(userid);
      if (!str_cmp(cuser.userid, userid))
      {
	vmsg("不可以將自己移出板主名單");
	continue;
      }
      else if (!str_cmp(buf + BMlen - len, userid))	/* 名單上最後一位，ID 後面不接 '/' */
      {
	buf[BMlen - len - 1] = '\0';			/* 刪除 ID 及前面的 '/' */
	len++;
      }
      else						/* ID 後面會接 '/' */
      {
	str_lower(userid, userid);
	strcat(userid, "/");
	len++;
	blist = str_str(buf, userid);
	strcpy(blist, blist + len);
      }
      BMlen -= len;
    }
    else if (acct_load(&acct, userid) >= 0 && !is_bm(buf, userid))	/* 輸入新板主 */
    {
      len = strlen(userid) + 1;	/* '/' + userid */
      if (BMlen + len > BMLEN)
      {
	vmsg("板主名單過長，無法將這 ID 設為板主");
	continue;
      }
      sprintf(buf + BMlen, "/%s", acct.userid);
      BMlen += len;

      acct_setperm(&acct, PERM_BM, 0);
    }
    move(8, 0);
    prints("目前板主為 %s", buf);
    clrtoeol();
  }
  strcpy(newbrd.BM, buf);

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);

    sprintf(currBM, "板主：%s", newbrd.BM);
    return XO_HEAD;	/* 要重繪檔頭的板主 */
  }

  return XO_BODY;
}


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* 板主功能 : 看板權限					 */
/* ----------------------------------------------------- */


static int
post_brdlevel(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("1)公開看板 2)秘密看板 3)好友看板？[Q] "))
  {
  case '1':				/* 公開看板 */
    newbrd.readlevel = 0;
    newbrd.postlevel = PERM_POST;
    newbrd.battr &= ~(BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '2':				/* 秘密看板 */
    newbrd.readlevel = PERM_SYSOP;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '3':				/* 好友看板 */
    newbrd.readlevel = PERM_BOARD;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  default:
    return XO_FOOT;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_FOOT;
}
#endif	/* HAVE_MODERATED_BOARD */


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* 板友名單：moderated board				 */
/* ----------------------------------------------------- */


static void
bpal_cache(fpath)
  char *fpath;
{
  BPAL *bpal;

  bpal = bshm->pcache + currbno;
  bpal->pal_max = image_pal(fpath, bpal->pal_spool);
}


extern XZ xz[];


static int
XoBM(xo)
  XO *xo;
{
  XO *xt;
  char fpath[64];

  brd_fpath(fpath, currboard, fn_pal);
  xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
  xt->key = PALTYPE_BPAL;
  xover(XZ_PAL);		/* Thor: 進xover前, pal_xo 一定要 ready */

  /* build userno image to speed up, maybe upgreade to shm */

  bpal_cache(fpath);

  free(xt);

  return XO_INIT;
}
#endif	/* HAVE_MODERATED_BOARD */

static int
post_usies(xo)
  XO *xo;
{
  char fpath[64];

  brd_fpath(fpath, currboard, "usies");
  if (more(fpath, (char *) -1) >= 0 &&
    vans("請問是否刪除這些看板閱\讀記錄(Y/N)？[N] ") == 'y')
    unlink(fpath);

  return XO_HEAD;
}

static int post_extern(xo)
	XO* xo;
{
	char buf[80];
  BRD *oldbrd, newbrd;
  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));
	sprintf(buf,"是否將傳送到 %s.brd@%s 的文章張貼於本看板(Y/N)？[Q] ",oldbrd->brdname,MYHOSTNAME);
	switch(vans(buf)){
		case 'n':
			newbrd.battr |= BRD_LOCAL;
			break;
		case 'y':
			newbrd.battr &= ~(BRD_LOCAL);
			break;
		default:
			return XO_FOOT;
	}
  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }
	return XO_FOOT;
}

static int post_lock(xo)
  XO* xo;
{
  BRD *brd;
  brd = bshm->bcache + currbno;
  char fpath[64];
  brd_fpath(fpath, brd->brdname, fn_lock);
  brd_editlock(fpath);
  return XO_FOOT;
}

/* ----------------------------------------------------- */
/* 板主選單						 */
/* ----------------------------------------------------- */


int
post_manage(xo)
  XO *xo;
{
#ifdef POPUP_ANSWER
  char *menu[] = 
  {
    "BQ",
    "BTitle  修改看板主題",
    "WMemo   編輯進板畫面",
#ifdef POST_PREFIX
    "RPrefix 編輯文章類別",
#endif
    "PostLaw 編輯發文綱領",
    "Manager 增減副板主",
    "ExtPost 設定站外寄信發文",
    "ZLock   管理看板鎖定狀態",
    "Usies   觀察看板閱\讀記錄",
#  ifdef HAVE_SCORE
    "Score   設定可否評分",
#  endif
#  ifdef ANTI_PHONETIC
    "Juyinwen注音文設定",
#  endif
#  ifdef HAVE_MODERATED_BOARD
    "Level   公開/好友/秘密",
    "OPal    板友名單",
#  endif
    NULL
  };
#else
  char *menu = "◎ 板主選單 (B)主題 (W)進板 (R)類別 (P)綱領 (M)副板 (E)收信 (Z)鎖定 (U)記錄"
#  ifdef HAVE_SCORE
    " (S)評分"
#  endif
#  ifdef ANTI_PHONETIC
    " (P)注音"
#  endif
#  ifdef HAVE_MODERATED_BOARD
    " (L)權限 (O)板友"
#  endif
    "？[Q] ";
#endif

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

#ifdef POPUP_ANSWER
  switch (pans(3, 20, "板主選單", menu))
#else
  switch (vans(menu))
#endif
  {
  case 'b':
    return post_brdtitle(xo);

  case 'w':
    return post_memo_edit(xo);
#ifdef POST_PREFIX
  case 'r':
    return post_prefix_edit(xo);
#endif
  case 'p':
    return post_postlaw_edit(xo);

  case 'm':
    return post_changeBM(xo);

	case 'e':
		return post_extern(xo);

  case 'z':
    return post_lock(xo);

  case 'u':
    return post_usies(xo);

#ifdef HAVE_SCORE
  case 's':
    return post_battr_noscore(xo);
#endif
#ifdef ANTI_PHONETIC
  case 'j':
    return post_battr_nophonetic(xo);
#endif
#ifdef HAVE_MODERATED_BOARD
  case 'l':
    return post_brdlevel(xo);

  case 'o':
    return XoBM(xo);
#endif
  }

  return XO_FOOT;
}

