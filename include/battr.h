/*-------------------------------------------------------*/
/* battr.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : Board Attribution				 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_BATTR_H_
#define	_BATTR_H_


/* ----------------------------------------------------- */
/* Board Attribution : flags in BRD.battr		 */
/* ----------------------------------------------------- */


#define BRD_NOZAP	0x01	/* 不可 zap */
#define BRD_NOTRAN	0x02	/* 不轉信 */
#define BRD_NOCOUNT	0x04	/* 不計文章發表篇數 */
#define BRD_NOSTAT	0x08	/* 不納入熱門話題統計 */
#define BRD_NOVOTE	0x10	/* 不公佈投票結果於 [record] 板 */
#define BRD_ANONYMOUS	0x20	/* 匿名看板 */
#define BRD_NOSCORE	0x40	/* 不評分看板 */
#define BRD_NOPHONETIC	0x80	/* 反注音文看板 */

#define BRD_LOCAL  0x00000100 /* 不收站外信 */
#define BRD_A10 0x00000200
#define BRD_A11 0x00000400
#define BRD_A12 0x00000800
#define BRD_A13 0x00001000
#define BRD_A14 0x00002000
#define BRD_A15 0x00004000
#define BRD_A16 0x00008000

#define BRD_A17 0x00010000
#define BRD_A18 0x00020000
#define BRD_A19 0x00040000
#define BRD_A20 0x00080000
#define BRD_A21 0x00100000
#define BRD_A22 0x00200000
#define BRD_A23 0x00400000
#define BRD_A24 0x00800000

#define BRD_A25 0x01000000
#define BRD_A26 0x02000000
#define BRD_A27 0x04000000
#define BRD_A28 0x08000000
#define BRD_A29 0x10000000
#define BRD_A30 0x20000000
#define BRD_A31 0x40000000
#define BRD_A32 0x80000000
/* ----------------------------------------------------- */
/* 各種旗標的中文意義					 */
/* ----------------------------------------------------- */


#define NUMBATTRS 32	

#define STR_BATTR	"zTcsvA%PL-----------------------"	
/* itoc: 新增旗標的時候別忘了改這裡啊 */


#ifdef _ADMIN_C_
static char *battr_tbl[NUMBATTRS] =
{
  "不可 Zap",			/* BRD_NOZAP */
  "不轉信出去",			/* BRD_NOTRAN */
  "不記錄篇數",			/* BRD_NOCOUNT */
  "不做熱門話題統計",		/* BRD_NOSTAT */
  "不公開投票結果",		/* BRD_NOVOTE */
  "匿名看板",			/* BRD_ANONYMOUS */
  "不評分看板",			/* BRD_NOSCORE */
  "不使用注音文",		/* BRD_NOPHONETIC */
	"不收站外信",			/* BRD_LOCAL */	
	"保留",			/* BRD_A10 */ 
	"保留",			/* BRD_A11 */
	"保留",			/* BRD_A12 */
	"保留",			/* BRD_A13 */
	"保留",			/* BRD_A14 */
	"保留",			/* BRD_A15 */
	"保留",			/* BRD_A16 */
	"保留",			/* BRD_A17 */
	"保留",			/* BRD_A18 */
	"保留",			/* BRD_A19 */
	"保留",			/* BRD_A20 */
	"保留",			/* BRD_A21 */
	"保留",			/* BRD_A22 */
	"保留",			/* BRD_A23 */
	"保留",			/* BRD_A24 */
	"保留",			/* BRD_A25 */
	"保留",			/* BRD_A26 */
	"保留",			/* BRD_A27 */
	"保留",			/* BRD_A28 */
	"保留",			/* BRD_A29 */
	"保留",			/* BRD_A30 */
	"保留",			/* BRD_A31 */
	"保留"			/* BRD_A32 */
};

#endif

#endif				/* _BATTR_H_ */
