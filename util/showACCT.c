/*-------------------------------------------------------*/
/* util/showACCT.c      ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 顯示使用者資料                               */
/* create : 01/07/16                                     */
/* update :                                              */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


//#undef	SHOW_PASSWORD		/* 顯示密碼 (很耗時間) */



//#ifdef SHOW_PASSWORD

#define GUESS_LEN	8		/* 只測三碼(含)以下的密碼組合 (最多是 PSWDLEN) */

static char *
_bitmsg(str, level)
  char *str;
  int level;
{
  int cc;
  int num;
  static char msg[33];

  num = 0;
  while (cc = *str)
  {
    msg[num] = (level & 1) ? cc : '-';
    level >>= 1;
    str++;
    num++;
  }
  msg[num] = 0;

  return msg;
}


static inline void
showACCT(acct)
  ACCT *acct;
{
  char msg1[40], msg2[40], msg3[40], msg4[40], msg5[40], msg6[40];

  strcpy(msg1, _bitmsg(STR_PERM, acct->userlevel));
  strcpy(msg2, _bitmsg(STR_UFO, acct->ufo));
  strcpy(msg3, Btime(&(acct->firstlogin)));
  strcpy(msg4, Btime(&(acct->lastlogin)));
  strcpy(msg5, Btime(&(acct->tcheck)));
  strcpy(msg6, Btime(&(acct->tvalid)));

  printf("> ------------------------------------------------------------------------------------------ \n"
    "編號: %-15d [ID]: %-15s 姓名: %-15s 暱稱: %-15s \n" 
    "權限: %-37s 設定: %-37s \n" 
    "簽名: %-37d 性別: %-15.2s \n" 
    "銀幣: %-15d 金幣: %-15d 生日: %02d/%02d/%02d \n" 
    "上站: %-15d 文章: %-15d 發信: %-15d \n" 
    "首次: %-37s 上次: %-30s \n" 
    "檢查: %-37s 通過: %-30s \n" 
    "登入: %-30s \n" 
    "信箱: %-60s \n", 
    acct->userno, acct->userid, acct->realname, acct->username, 
    msg1, msg2,
    acct->signature, "？♂♀" + (acct->sex << 1),
    acct->money, acct->gold, acct->year, acct->month, acct->day, 
    acct->numlogins, acct->numposts, acct->numemails, 
    msg3, msg4, 
    msg5, msg6, 
    acct->lasthost, 
    acct->email);

//#ifdef SHOW_PASSWORD
	printf("%s\n", acct->passwd);
//#endif
}


int
main(argc, argv)
  int argc;
  char **argv;
{
  int i;

  if (argc < 2)
  {
    printf("Usage: %s UserID1 [UserID2] ...\n", argv[0]);
    return -1;
  }

  chdir(BBSHOME);

  for (i = 1; i < argc; i++)
  {
    ACCT acct;
    char fpath[64];

    usr_fpath(fpath, argv[i], FN_ACCT);
    if (rec_get(fpath, &acct, sizeof(ACCT), 0) < 0)
    {
      printf("%s: read error (maybe no such id?)\n", argv[i]);
      continue;
    }

    showACCT(&acct);
  }
  printf("> ------------------------------------------------------------------------------------------ \n");

  return 0;
}
