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


#define BRD_NOZAP	0x01	/* ���i zap */
#define BRD_NOTRAN	0x02	/* ����H */
#define BRD_NOCOUNT	0x04	/* ���p�峹�o��g�� */
#define BRD_NOSTAT	0x08	/* ���ǤJ�������D�έp */
#define BRD_NOVOTE	0x10	/* �����G�벼���G�� [record] �O */
#define BRD_ANONYMOUS	0x20	/* �ΦW�ݪO */
#define BRD_NOSCORE	0x40	/* �������ݪO */
#define BRD_NOPHONETIC	0x80	/* �Ϫ`����ݪO */

#define BRD_LOCAL  0x00000100 /* �������~�H */
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
/* �U�غX�Ъ�����N�q					 */
/* ----------------------------------------------------- */


#define NUMBATTRS 32	

#define STR_BATTR	"zTcsvA%PL-----------------------"	
/* itoc: �s�W�X�Ъ��ɭԧO�ѤF��o�̰� */


#ifdef _ADMIN_C_
static char *battr_tbl[NUMBATTRS] =
{
  "���i Zap",			/* BRD_NOZAP */
  "����H�X�h",			/* BRD_NOTRAN */
  "���O���g��",			/* BRD_NOCOUNT */
  "�����������D�έp",		/* BRD_NOSTAT */
  "�����}�벼���G",		/* BRD_NOVOTE */
  "�ΦW�ݪO",			/* BRD_ANONYMOUS */
  "�������ݪO",			/* BRD_NOSCORE */
  "���ϥΪ`����",		/* BRD_NOPHONETIC */
	"�������~�H",			/* BRD_LOCAL */	
	"�O�d",			/* BRD_A10 */ 
	"�O�d",			/* BRD_A11 */
	"�O�d",			/* BRD_A12 */
	"�O�d",			/* BRD_A13 */
	"�O�d",			/* BRD_A14 */
	"�O�d",			/* BRD_A15 */
	"�O�d",			/* BRD_A16 */
	"�O�d",			/* BRD_A17 */
	"�O�d",			/* BRD_A18 */
	"�O�d",			/* BRD_A19 */
	"�O�d",			/* BRD_A20 */
	"�O�d",			/* BRD_A21 */
	"�O�d",			/* BRD_A22 */
	"�O�d",			/* BRD_A23 */
	"�O�d",			/* BRD_A24 */
	"�O�d",			/* BRD_A25 */
	"�O�d",			/* BRD_A26 */
	"�O�d",			/* BRD_A27 */
	"�O�d",			/* BRD_A28 */
	"�O�d",			/* BRD_A29 */
	"�O�d",			/* BRD_A30 */
	"�O�d",			/* BRD_A31 */
	"�O�d"			/* BRD_A32 */
};

#endif

#endif				/* _BATTR_H_ */
