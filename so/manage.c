/*-------------------------------------------------------*/
/* manage.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �ݪO�޲z				 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;


#ifdef HAVE_TERMINATOR
/* ----------------------------------------------------- */
/* �����\�� : �ط�������				 */
/* ----------------------------------------------------- */


extern char xo_pool[];


#define MSG_TERMINATOR	"�m�ط������١n"

int
post_terminator(xo)		/* Thor.980521: �׷��峹�R���j�k */
  XO *xo;
{
  int mode, type;
  HDR *hdr;
  char keyOwner[80], keyTitle[TTLEN + 1], buf[80];

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_FOOT;

  mode = vans(MSG_TERMINATOR "�R�� (1)����@�� (2)������D (3)�۩w�H[Q] ") - '0';

  if (mode == 1)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyOwner, hdr->owner);
  }
  else if (mode == 2)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyTitle, str_ttl(hdr->title));		/* ���� Re: */
  }
  else if (mode == 3)
  {
    if (!vget(b_lines, 0, "�@�̡G", keyOwner, 73, DOECHO))
      mode ^= 1;
    if (!vget(b_lines, 0, "���D�G", keyTitle, TTLEN + 1, DOECHO))
      mode ^= 2;
  }
  else
  {
    return XO_FOOT;
  }

  type = vans(MSG_TERMINATOR "�R�� (1)��H�O (2)�D��H�O (3)�Ҧ��ݪO�H[Q] ");
  if (type < '1' || type > '3')
    return XO_FOOT;

  sprintf(buf, "�R��%s�G%.35s ��%s�O�A�T�w��(Y/N)�H[N] ", 
    mode == 1 ? "�@��" : mode == 2 ? "���D" : "����", 
    mode == 1 ? keyOwner : mode == 2 ? keyTitle : "�۩w", 
    type == '1' ? "��H" : type == '2' ? "�D��H" : "�Ҧ���");

  if (vans(buf) == 'y')
  {
    BRD *bhdr, *head, *tail;
    char tmpboard[BNLEN + 1];

    /* Thor.980616: �O�U currboard�A�H�K�_�� */
    strcpy(tmpboard, currboard);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do				/* �ܤ֦� note �@�O */
    {
      int fdr, fsize, xmode;
      FILE *fpw;
      char fpath[64], fnew[64], fold[64];
      HDR *hdr;

      xmode = head->battr;
      if ((type == '1' && (xmode & BRD_NOTRAN)) || (type == '2' && !(xmode & BRD_NOTRAN)))
	continue;

      /* Thor.980616: ��� currboard�A�H cancel post */
      strcpy(currboard, head->brdname);

      sprintf(buf, MSG_TERMINATOR "�ݪO�G%s \033[5m...\033[m", currboard);
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
	  /* �Y���ݪO�N�s�u��H */

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

    /* �٭� currboard */
    strcpy(currboard, tmpboard);
    return XO_LOAD;
  }

  return XO_FOOT;
}
#endif	/* HAVE_TERMINATOR */


/* ----------------------------------------------------- */
/* �O�D�\�� : �ק�O�W					 */
/* ----------------------------------------------------- */


static int
post_brdtitle(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  /* itoc.����: ���I�s brd_title(bno) �N�i�H�F�A�S�t�A�Z�F�@�U�n�F :p */
  if (vans("�O�_�ק襤��O�W�ԭz(Y/N)�H[N] ") == 'y')
  {
    vget(b_lines, 0, "�ݪO�D�D�G", newbrd.title, BTLEN + 1, GCARRY);

    if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
    {
      memcpy(oldbrd, &newbrd, sizeof(BRD));
      rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
      return XO_HEAD;	/* itoc.011125: �n��ø����O�W */
    }
  }

  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* �O�D�\�� : �ק�i�O�e��				 */
/* ----------------------------------------------------- */


static int
post_memo_edit(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  mode = vans("�i�O�e�� (D)�R�� (E)�ק� (Q)�����H[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, fn_note);

    if (mode == 'd')
    {
      unlink(fpath);
      return XO_FOOT;
    }

    if (vedit(fpath, 0))	/* Thor.981020: �`�N�Qtalk�����D */
      vmsg(msg_cancel);
    return XO_HEAD;
  }
  return XO_FOOT;
}

#ifdef POST_PREFIX
/* ----------------------------------------------------- */
/* �O�D�\�� : �ק�o�����O                               */
/* ----------------------------------------------------- */


static int
post_prefix_edit(xo)
  XO *xo;
{
#define NUM_PREFIX      9
  int i;
  FILE *fp;
  char fpath[64], buf[20], prefix[NUM_PREFIX][20], *menu[NUM_PREFIX + 3];
  char *prefix_def[NUM_PREFIX] =   /* �w�]�����O */
  {
    "[���i]", "[����]", "[����]", "[���]", "[�L��]",
	 	"[���V]", "[����]", "[�Ч@]", "[���]"
  };
	char modified=0; /* lantw44: �P�_���L�ק�L */

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  i = vans("���O (D)�R�� (E)�ק� (Q)�����H [E] ");

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
      buf[strlen(buf)-1]='\0'; /* lantw44:�Y���̫�@��\n */
			sprintf(prefix[i], "%d.%s", i + 1, buf);
    }
    fclose(fp);
  }

  /* �񺡦� NUM_PREFIX �� */
  for (; i < NUM_PREFIX; i++)
    sprintf(prefix[i], "%d.%s", i + 1, prefix_def[i]);

  menu[0] = "10";
  for (i = 1; i <= NUM_PREFIX; i++)
    menu[i] = prefix[i - 1];
  menu[NUM_PREFIX + 1] = "0.���}";
  menu[NUM_PREFIX + 2] = NULL;

  do
  {
    /* �b popupmenu �̭��� ���� ���} */
    i = pans(3, 20, "�峹���O", menu) - '0';
    if (i >= 1 && i <= NUM_PREFIX)
    {
      strcpy(buf, prefix[i - 1] + 2);
      if (vget(b_lines, 0, "���O�G", buf, 10, GCARRY)){
				if(strcmp(prefix[i - 1] + 2,buf)){
					modified=1;
				}
        strcpy(prefix[i - 1] + 2, buf);
			}
    }
  } while (i);

	if(!modified || vans("�T�w�n�x�s�ɮ׶�(Y/N)�H[Y] ") == 'n'){
		vmsg("��ʤ���");
		return XO_FOOT;
	}

  if (fp = fopen(fpath, "w"))
  {
    for (i = 0; i < NUM_PREFIX; i++)
      fprintf(fp, "%s\n", prefix[i] + 2);
    fclose(fp);
		vmsg("��s����");
  }else{
		vmsg("�ɮ׼g�J����");
	}

  return XO_FOOT;
}
#endif      /* POST_PREFIX */
/* ----------------------------------------------------- */
/* �O�D�\�� : �ק�o�����                               */
/* ----------------------------------------------------- */


static int
post_postlaw_edit(xo)       /* �O�D�۩w�峹�o����� */
  XO *xo;
{
  int mode;
  char fpath[64];

  mode = vans("�峹�o����� (D)�R�� (E)�ק� (Q)�����H[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, FN_POSTLAW);

    if (mode == 'd')
    {
      unlink(fpath);
      return XO_FOOT;
    }
    if (vedit(fpath, 0))      /* Thor.981020: �`�N�Qtalk�����D */
      vmsg(msg_cancel);
    return XO_HEAD;
  }
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* �O�D�\�� : �ݪO�ݩ�					 */
/* ----------------------------------------------------- */


#ifdef HAVE_SCORE
static int
post_battr_noscore(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("�}����� (1)���\\ (2)���\\ (Q)�����H[Q] "))
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

  switch (vans("1)�i�H�Ϊ`���� 2)���i�Ϊ`���� [Q] "))
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
/* �O�D�\�� : �ק�O�D�W��				 */
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

  if (strcmp(oldbrd->class, "�ӤH") &&
  	  strcmp(oldbrd->class, "����") &&
	    strcmp(oldbrd->class, "����"))
  {
	  vmsg ("���}�O�������");
	  return XO_FOOT;
	}
	
  blist = oldbrd->BM;
  if (is_bm(blist, cuser.userid) != 1)	/* �u�����O�D�i�H�]�w�O�D�W�� */
    return XO_FOOT;

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  move(3, 0);
  clrtobot();

  move(8, 0);
  prints("�ثe�O�D�� %s\n�п�J�s���O�D�W��A�Ϋ� [Return] ����", oldbrd->BM);

  strcpy(buf, oldbrd->BM);
  BMlen = strlen(buf);

  while (vget(10, 0, "�п�J�ƪO�D�A�����Ы� Enter�A�M���Ҧ��ƪO�D�Х��u�L�v�G", userid, IDLEN + 1, DOECHO))
  {
    if (!strcmp(userid, "�L"))
    {
      strcpy(buf, cuser.userid);
      BMlen = strlen(buf);
    }
    else if (is_bm(buf, userid))	/* �R���¦����O�D */
    {
      len = strlen(userid);
      if (!str_cmp(cuser.userid, userid))
      {
	vmsg("���i�H�N�ۤv���X�O�D�W��");
	continue;
      }
      else if (!str_cmp(buf + BMlen - len, userid))	/* �W��W�̫�@��AID �᭱���� '/' */
      {
	buf[BMlen - len - 1] = '\0';			/* �R�� ID �Ϋe���� '/' */
	len++;
      }
      else						/* ID �᭱�|�� '/' */
      {
	str_lower(userid, userid);
	strcat(userid, "/");
	len++;
	blist = str_str(buf, userid);
	strcpy(blist, blist + len);
      }
      BMlen -= len;
    }
    else if (acct_load(&acct, userid) >= 0 && !is_bm(buf, userid))	/* ��J�s�O�D */
    {
      len = strlen(userid) + 1;	/* '/' + userid */
      if (BMlen + len > BMLEN)
      {
	vmsg("�O�D�W��L���A�L�k�N�o ID �]���O�D");
	continue;
      }
      sprintf(buf + BMlen, "/%s", acct.userid);
      BMlen += len;

      acct_setperm(&acct, PERM_BM, 0);
    }
    move(8, 0);
    prints("�ثe�O�D�� %s", buf);
    clrtoeol();
  }
  strcpy(newbrd.BM, buf);

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);

    sprintf(currBM, "�O�D�G%s", newbrd.BM);
    return XO_HEAD;	/* �n��ø���Y���O�D */
  }

  return XO_BODY;
}


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* �O�D�\�� : �ݪO�v��					 */
/* ----------------------------------------------------- */


static int
post_brdlevel(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("1)���}�ݪO 2)���K�ݪO 3)�n�ͬݪO�H[Q] "))
  {
  case '1':				/* ���}�ݪO */
    newbrd.readlevel = 0;
    newbrd.postlevel = PERM_POST;
    newbrd.battr &= ~(BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '2':				/* ���K�ݪO */
    newbrd.readlevel = PERM_SYSOP;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '3':				/* �n�ͬݪO */
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
/* �O�ͦW��Gmoderated board				 */
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
  xover(XZ_PAL);		/* Thor: �ixover�e, pal_xo �@�w�n ready */

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
    vans("�аݬO�_�R���o�ǬݪO�\\Ū�O��(Y/N)�H[N] ") == 'y')
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
	sprintf(buf,"�O�_�N�ǰe�� %s.brd@%s ���峹�i�K�󥻬ݪO(Y/N)�H[Q] ",oldbrd->brdname,MYHOSTNAME);
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
/* �O�D���						 */
/* ----------------------------------------------------- */


int
post_manage(xo)
  XO *xo;
{
#ifdef POPUP_ANSWER
  char *menu[] = 
  {
    "BQ",
    "BTitle  �ק�ݪO�D�D",
    "WMemo   �s��i�O�e��",
#ifdef POST_PREFIX
    "RPrefix �s��峹���O",
#endif
    "PostLaw �s��o�����",
    "Manager �W��ƪO�D",
    "ExtPost �]�w���~�H�H�o��",
    "ZLock   �޲z�ݪO��w���A",
    "Usies   �[��ݪO�\\Ū�O��",
#  ifdef HAVE_SCORE
    "Score   �]�w�i�_����",
#  endif
#  ifdef ANTI_PHONETIC
    "Juyinwen�`����]�w",
#  endif
#  ifdef HAVE_MODERATED_BOARD
    "Level   ���}/�n��/���K",
    "OPal    �O�ͦW��",
#  endif
    NULL
  };
#else
  char *menu = "�� �O�D��� (B)�D�D (W)�i�O (R)���O (P)���� (M)�ƪO (E)���H (Z)��w (U)�O��"
#  ifdef HAVE_SCORE
    " (S)����"
#  endif
#  ifdef ANTI_PHONETIC
    " (P)�`��"
#  endif
#  ifdef HAVE_MODERATED_BOARD
    " (L)�v�� (O)�O��"
#  endif
    "�H[Q] ";
#endif

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

#ifdef POPUP_ANSWER
  switch (pans(3, 20, "�O�D���", menu))
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

