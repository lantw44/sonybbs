/*-------------------------------------------------------*/
/* util/camera.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �إ� [�ʺA�ݪO] cache			 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


static char *list[] = 		/* src/include/struct.h */
{
  "opening.0",			/* FILM_OPENING0 */
  "opening.1",			/* FILM_OPENING1 */
  "opening.2",			/* FILM_OPENING2 */
  "goodbye", 			/* FILM_GOODBYE  */
  "notify",			/* FILM_NOTIFY   */
  "mquota",			/* FILM_MQUOTA   */
  "mailover",			/* FILM_MAILOVER */
  "mgemover",			/* FILM_MGEMOVER */
  "birthday",			/* FILM_BIRTHDAY */
  "apply",			/* FILM_APPLY    */
  "justify",			/* FILM_JUSTIFY  */
  "re-reg",			/* FILM_REREG    */
  "e-mail",			/* FILM_EMAIL    */
  "newuser",			/* FILM_NEWUSER  */
  "tryout",			/* FILM_TRYOUT   */
  "post",			/* FILM_POST     */
  NULL				/* FILM_MOVIE    */
};


static void
str_strip(str)		/* itoc.060417: �N�ʺA�ݪO�C�C���e���t�b SCR_WIDTH */
  char *str;
{
  int ch, ansi, len;

  /* �Y�ʺA�ݪO���@�C���e�׶W�L SCR_WIDTH�A�|��ܤG�C�A�y���ƪ����~ (�D�n�O�I�q������)
     �ҥH�N���ܧ�W�L SCR_WIDTH �������R�� (�b�����Ҽ{�e�ù�) */
  /* �Y���C���� \033*s �� \033*n�A��ܥX�ӷ|����A�ҥH�n�S�O�B�z */

  ansi = len = 0;
  while (ch = *str++)
  {
    if (ch == '\n')
    {
      break;
    }
    else if (ch == '\033')
    {
      ansi = 1;
    }
    else if (ansi)
    {
      if (ch == '*')	/* KEY_ESC + * + s/n �q�X ID/username�A�Ҽ{�̤j���� */
      {
	ansi = 0;
	len += BMAX(IDLEN, UNLEN) - 1;
	if (len > SCR_WIDTH)
	{
	  *str = '\0';
	  break;
	}
      }
      else if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
	ansi = 0;
    }
    else
    {
      if (++len > SCR_WIDTH)
      {
	*str = '\0';
	break;
      }
    }
  }
}


static FCACHE image;
static int number;	/* �ثe�w mirror �X�g�F */
static int total;	/* �ثe�w mirror �X byte �F */


static int		/* 1:���\ 0:�w�W�L�g�Ʃήe�q */
mirror(fpath, line)
  char *fpath;
  int line;		/* 0:�t�Τ��A������C��  !=0:�ʺA�ݪO�Aline �C */
{
  int size, i;
  char buf[FILM_SIZ];
  char tmp[ANSILINELEN];
  struct stat st;
  FILE *fp;

  /* �Y�w�g�W�L�̤j�g�ơA�h���A�~�� mirror */
  if (number >= MOVIE_MAX - 1)
    return 0;

  if (stat(fpath, &st))
    size = -1;
  else
    size = st.st_size;

  if (size > 0 && size < FILM_SIZ && (fp = fopen(fpath, "r")))
  {
    size = i = 0;
    while (fgets(tmp, ANSILINELEN, fp))
    {
      str_strip(tmp);

      strcpy(buf + size, tmp);
      size += strlen(tmp);
  
      if (line)
      {
	/* �ʺA�ݪO�A�̦h line �C */
	if (++i >= line)
	  break;
      }
    }
    fclose(fp);

    if (i != line)	
    {
      /* �ʺA�ݪO�A�Y���� line �C�A�n�� line �C */
      for (; i < line; i++)
      {
	buf[size] = '\n';
	size++;
      }
      buf[size] = '\0';
    }
  }

  if (size <= 0 || size >= FILM_SIZ)
  {
    if (line)		/* �p�G�O �ʺA�ݪO/�I�q ���ɮסA�N�� mirror */
      return 1;

    /* �p�G�O�t�Τ��X�����ܡA�n�ɤW�h */
    sprintf(buf, "�Чi�D�����ɮ� %s �򥢩άO�L�j", fpath);
    size = strlen(buf);
  }

  size++;	/* Thor.980804: +1 �N������ '\0' �]��J */

  i = total + size;
  if (i >= MOVIE_SIZE)	/* �Y�[�J�o�g�|�W�L�����e�q�A�h�� mirror */
    return 0;

  memcpy(image.film + total, buf, size);
  image.shot[++number] = total = i;

  return 1;
}


static void
do_gem(folder)		/* itoc.011105: ��ݪO/��ذϪ��峹���i movie */
  char *folder;		/* index ���| */
{
  char fpath[64];
  FILE *fp;
  HDR hdr;

  if (fp = fopen(folder, "r"))
  {
    while (fread(&hdr, sizeof(HDR), 1, fp) == 1)
    {
      if (hdr.xmode & (GEM_RESTRICT | GEM_RESERVED | GEM_BOARD))	/* ����šB�ݪO ����J movie �� */
	continue;

      hdr_fpath(fpath, folder, &hdr);

      if (hdr.xmode & GEM_FOLDER)	/* �J����v�h�j��i�h�� movie */
      {
	do_gem(fpath);
      }
      else				/* plain text */
      {
	if (!mirror(fpath, MOVIE_LINES))
	  break;
      }
    }
    fclose(fp);
  }
}


static void
lunar_calendar(key, now, ptime)	/* itoc.050528: �Ѷ����A���� */
  char *key;
  time_t *now;
  struct tm *ptime;
{
#if 0	/* Table ���N�q */

  (1) "," �u�O���j�A��K���H�\Ū�Ӥw
  (2) �e 12 �� byte ���O�N��A�� 1-12 �묰�j��άO�p��C"L":�j��T�Q�ѡA"-":�p��G�Q�E��
  (3) �� 14 �� byte �N���~�A��|��C"X":�L�|��A"123456789:;<" ���O�N��A�� 1-12 �O�|��
  (4) �� 16-20 �� bytes �N���~�A��s�~�O���ѡC�Ҧp "02:15" ��ܶ���G��Q����O�A��s�~

#endif

  #define TABLE_INITAIL_YEAR	1970
  #define TABLE_FINAL_YEAR	2037

  char Table[TABLE_FINAL_YEAR - TABLE_INITAIL_YEAR + 1][21] = 
  {
    "L--L-LL-LL-L,X,02:06",	/* 1970 ���~ */
    "-L--LL-LLL-L,5,01:27",	/* 1971 �ަ~ */
    "-L--L-L-LL-L,X,02:15",	/* 1972 ���~ */
    "L-L--L--LL-L,X,02:03",	/* 1973 ���~ */
    "LL-L-L--LL-L,4,01:23",	/* 1974 ��~ */
    "LL-L--L--L-L,X,02:11",	/* 1975 �ߦ~ */
    "LL-L-L-L-L-L,8,01:31",	/* 1976 �s�~ */
    "L-LL-L-L-L--,X,02:18",	/* 1977 �D�~ */
    "L-LL-LL-L-L-,X,02:07",	/* 1978 ���~ */
    "L--L-L-LL-L-,6,01:28",	/* 1979 �Ϧ~ */
    "L--L-L-LL-LL,X,02:16",	/* 1980 �U�~ */
    "-L--L--LL-LL,X,02:05",	/* 1981 ���~ */
    "L-L-L--L-LLL,4,01:25",	/* 1982 ���~ */
    "L-L--L--L-LL,X,02:13",	/* 1983 �ަ~ */
    "L-LL--L--LLL,:,02:02",	/* 1984 ���~ */
    "-LL-L-L--L-L,X,02:20",	/* 1985 ���~ */
    "-LL-LL-L-L--,X,02:09",	/* 1986 ��~ */
    "L-L-LLLL-L--,6,01:29",	/* 1987 �ߦ~ */
    "L-L-L-LL-LL-,X,02:17",	/* 1988 �s�~ */
    "L--L-L-L-LLL,X,02:06",	/* 1989 �D�~ */
    "-L--L-L-LLLL,5,01:27",	/* 1990 ���~ */
    "-L--L--L-LLL,X,02:15",	/* 1991 �Ϧ~ */
    "-LL--L--L-LL,X,02:04",	/* 1992 �U�~ */
    "-LLL-L--L-L-,3,01:23",	/* 1993 ���~ */
    "LLL-L-L--L-L,X,02:10",	/* 1994 ���~ */
    "-LL-L-LL-L-L,8,01:31",	/* 1995 �ަ~ */
    "-L-LL-L-LL--,X,02:19",	/* 1996 ���~ */
    "L-L-L-LL-LL-,X,02:07",	/* 1997 ���~ */
    "L--L-LL-LL-L,5,01:28",	/* 1998 ��~ */
    "L--L--L-LLL-,X,02:16",	/* 1999 �ߦ~ */
    "LL--L--L-LL-,X,02:05",	/* 2000 �s�~ */
    "LL-LL--L-L-L,4,01:24",	/* 2001 �D�~ */
    "LL-L-L--L-L-,X,02:12",	/* 2002 ���~ */
    "LL-LL-L--L-L,X,02:01",	/* 2003 �Ϧ~ */
    "-LLL-L-L-L-L,2,01:22",	/* 2004 �U�~ */
    "-L-L-LL-L-L-,X,02:09",	/* 2005 ���~ */
    "L-L-L-LLL-LL,7,01:29",	/* 2006 ���~ */
    "--L--L-LLL-L,X,02:18",	/* 2007 �ަ~ */
    "L--L--L-LL-L,X,02:07",	/* 2008 ���~ */
    "LL--L-L-L-LL,5,01:26",	/* 2009 ���~ */
    "L-L-L--L-L-L,X,02:14",	/* 2010 ��~ */
    "L-LL-L--L-L-,X,02:03",	/* 2011 �ߦ~ */
    "L-LLL-L-L-L-,4,01:23",	/* 2012 �s�~ */
    "L-L-LL-L-L-L,X,02:10",	/* 2013 �D�~ */
    "-L-L-L-LLL-L,9,01:31",	/* 2014 ���~ */
    "-L--L-LLL-L-,X,02:19",	/* 2015 �Ϧ~ */
    "L-L--L-LL-LL,X,02:08",	/* 2016 �U�~ */
    "-L-L---L-LLL,6,01:28",	/* 2017 ���~ */
    "-L-L--L-L-LL,X,02:16",	/* 2018 ���~ */
    "L-L-L--L--LL,X,02:05",	/* 2019 �ަ~ */
    "-LLLL--L-L-L,4,01:25",	/* 2020 ���~ */
    "-LL-L-L-L-L-,X,02:12",	/* 2021 ���~ */
    "L-L-LL-L-L-L,X,02:01",	/* 2022 ��~ */
    "-L-LL-LL-L-L,2,01:22",	/* 2023 �ߦ~ */
    "-L--L-LL-LL-,X,02:10",	/* 2024 �s�~ */
    "L-L--LL-LLL-,6,01:29",	/* 2025 �D�~ */
    "L-L--L--LLL-,X,02:17",	/* 2026 ���~ */
    "LL-L--L--LL-,X,02:06",	/* 2027 �Ϧ~ */
    "LLL-L-L--LL-,5,01:26",	/* 2028 �U�~ */
    "LL-L-L-L--LL,X,02:13",	/* 2029 ���~ */
    "-L-LL-L-L-L-,X,02:03",	/* 2030 ���~ */
    "-LLL-LL-L-L-,3,01:23",	/* 2031 �ަ~ */
    "L--L-LL-LL-L,X,02:11",	/* 2032 ���~ */
    "-L--L-L-LLLL,;,01:31",	/* 2033 ���~ */
    "-L--L-L-LL-L,X,02:19",	/* 2034 ��~ */
    "L-L--L--LL-L,X,02:08",	/* 2035 �ߦ~ */
    "LL-L----L-LL,6,01:28",	/* 2036 �s�~ */
    "LL-L--L--L-L,X,02:15",	/* 2037 �D�~ */
#if 0
    "LL-L-L-L--L-,X,02:04",	/* 2038 ���~ */
    "LL-LLL-L-L--,5,01:24",	/* 2039 �Ϧ~ */
    "L-LL-L-LL-L-,X,02:12",	/* 2040 �U�~ */
    "-L-L-LL-LL-L,X,02:01",	/* 2041 ���~ */
    "-L-L-L-LL-LL,2,01:22",	/* 2042 ���~ */
    "-L--L--LL-LL,X,02:10",	/* 2043 �ަ~ */
    "L-L--L-L-LLL,7,01:30",	/* 2044 ���~ */
    "L-L--L--L-LL,X,02:17",	/* 2045 ���~ */
    "L-L-L-L--L-L,X,02:06",	/* 2046 ��~ */
    "L-LL--L--L-L,5,01:26",	/* 2047 �ߦ~ */
    "-LL-LL-L--L-,X,02:14",	/* 2048 �s�~ */
    "L-L-LL-LL-L-,X,02:02",	/* 2049 �D�~ */
    "-L--L-LL-LL-,3,01:23",	/* 2050 ���~ */
    "L--L--LL-LLL,X,02:11",	/* 2051 �Ϧ~ */
    "-L--L--LLLLL,8,02:01",	/* 2052 �U�~ */
    "-L--L--L-LLL,X,02:19",	/* 2053 ���~ */
    "-LL--L--L-LL,X,02:08",	/* 2054 ���~ */
    "-LL-L---L-L-,6,01:28",	/* 2055 �ަ~ */
    "LLL-L-L--L-L,X,02:15",	/* 2056 ���~ */
    "-LL-L-LL--L-,X,02:04",	/* 2057 ���~ */
    "L-L--LL-LL--,4,01:24",	/* 2058 ��~ */
    "L-L-L-L-LLL-,X,02:12",	/* 2059 �ߦ~ */
    "L--L--L-LLL-,X,02:02",	/* 2060 �s�~ */
    "LL-L--L-LLL-,3,01:21",	/* 2061 �D�~ */
    "LL--L--L-LL-,X,02:09",	/* 2062 ���~ */
    "LL-L-L-L-L-L,7,01:29",	/* 2063 �Ϧ~ */
    "LL-L-L--L-L-,X,02:17",	/* 2064 �U�~ */
    "LL-LL-L--L-L,X,02:05",	/* 2065 ���~ */
    "-L-LLL-L-L-L,5,01:26",	/* 2066 ���~ */
    "-L-L-LL-L-L-,X,02:14",	/* 2067 �ަ~ */
    "L-L--LL-LL-L,X,02:03",	/* 2068 ���~ */
    "-L-L-L-LLL-L,4,01:23",	/* 2069 ���~ */
    "-L-L--L-LL-L,X,02:11",	/* 2070 ��~ */
    "L-L-L--LL-LL,8,01:31",	/* 2071 �ߦ~ */
    "L-L-L--L-L-L,X,02:19",	/* 2072 �s�~ */
    "L-LL-L--L-L-,X,02:07",	/* 2073 �D�~ */
    "L-LL-LL-L-L-,6,01:27",	/* 2074 ���~ */
    "L-L-LL-L-L-L,X,02:15",	/* 2075 �Ϧ~ */
    "-L-L-L-LL-L-,X,02:05",	/* 2076 �U�~ */
    "L-L-L-LLL-L-,4,01:24",	/* 2077 ���~ */
    "L-L--L-LL-LL,X,02:12",	/* 2078 ���~ */
    "-L-L--L-L-LL,X,02:02",	/* 2079 �ަ~ */
    "L-LL--L--LLL,3,01:22",	/* 2080 ���~ */
    "-LL-L--L--LL,X,02:09",	/* 2081 ���~ */
    "-LLL--LL--LL,7,01:29",	/* 2082 ��~ */
    "-LL-L-L-L-L-,X,02:17",	/* 2083 �ߦ~ */
    "L-L-LL-L-L-L,X,02:06",	/* 2084 �s�~ */
    "-L--L-LL-L-L,5,01:26",	/* 2085 �D�~ */
    "-L--L-LL-LL-,X,02:14",	/* 2086 ���~ */
    "L-L--L-L-LLL,X,02:03",	/* 2087 �Ϧ~ */
    "-L-L-L--LLL-,4,01:24",	/* 2088 �U�~ */
    "LL-L--L--LL-,X,02:10",	/* 2089 ���~ */
    "LLL-L--L-LL-,8,01:30",	/* 2090 ���~ */
    "LL-L-L-L--L-,X,02:18",	/* 2091 �ަ~ */
    "LL-LL-L-L-L-,X,02:07",	/* 2092 ���~ */
    "-LL-L-L-L-L-,6,01:27",	/* 2093 ���~ */
    "-L-L-LL-LL-L,X,02:15",	/* 2094 ��~ */
    "-L--L-L-LLL-,X,02:05",	/* 2095 �ߦ~ */
    "L-L-L--LLL-L,4,01:25",	/* 2096 �s�~ */
    "L-L--L--LL-L,X,02:12",	/* 2097 �D�~ */
    "LL-L---L-L-L,X,02:01",	/* 2098 ���~ */
    "LLLL--L--L-L,2,01:21",	/* 2099 �Ϧ~ */
    "LL-L-L-L--L-,X,02:09",	/* 2100 �U�~ */
#endif
  };

  char year[21];

  time_t nyd;	/* ���~���A��s�~ */
  struct tm ntime;

  int i;
  int Mon, Day;
  int leap;	/* 0:���~�L�|�� */

  /* ����X���ѬO�A����@�~ */

  memcpy(&ntime, ptime, sizeof(ntime));

  for (i = TABLE_FINAL_YEAR - TABLE_INITAIL_YEAR; i >= 0; i--)
  {
    strcpy(year, Table[i]);
    ntime.tm_year = TABLE_INITAIL_YEAR - 1900 + i;
    ntime.tm_mday = atoi(year + 18);
    ntime.tm_mon = atoi(year + 15) - 1;
    nyd = mktime(&ntime);

    if (*now >= nyd)
      break;
  }

  /* �A�q�A�䥿���@�}�l�ƨ줵�� */

  leap = (year[13] == 'X') ? 0 : 1;

  Mon = Day = 1;
  for (i = (*now - nyd) / 86400; i > 0; i--)
  {
    if (++Day > (year[Mon - 1] == 'L' ? 30: 29))
    {
      Mon++;
      Day = 1;

      if (leap == 2)
      {
	leap = 0;
	Mon--;
	year[Mon - 1] = '-';	/* �|�륲�p�� */
      }
      else if (year[13] == Mon + '0')
      {
	if (leap == 1)		/* �U��O�|�� */
	  leap++;
      }
    }
  }

  sprintf(key, "%02d:%02d", Mon, Day);
}


static char *
do_today()
{
  FILE *fp;
  char buf[80], *ptr1, *ptr2, *ptr3, *today;
  char key1[6];			/* mm/dd: ���� mm��dd�� */
  char key2[6];			/* mm/#A: ���� mm�몺��#�ӬP��A */
  char key3[6];			/* MM\DD: �A�� MM��DD�� */
  time_t now;
  struct tm *ptime;
  static char feast[64];

  time(&now);
  ptime = localtime(&now);
  sprintf(key1, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
  sprintf(key2, "%02d/%d%c", ptime->tm_mon + 1, (ptime->tm_mday - 1) / 7 + 1, ptime->tm_wday + 'A');
  lunar_calendar(key3, &now, ptime);

  today = image.today;
  sprintf(today, "%s %.2s", key1, "��@�G�T�|����" + (ptime->tm_wday << 1));

  if (fp = fopen(FN_ETC_FEAST, "r"))
  {
    while (fgets(buf, 80, fp))
    {
      if (buf[0] == '#')
	continue;

      if ((ptr1 = strtok(buf, " \t\n")) && (ptr2 = strtok(NULL, " \t\n")))
      {
	if (!strcmp(ptr1, key1) || !strcmp(ptr1, key2) || !strcmp(ptr1, key3))
	{
	  str_ncpy(today, ptr2, sizeof(image.today));

	  if (ptr3 = strtok(NULL, " \t\n"))
	    sprintf(feast, "etc/feasts/%s", ptr3);
	  if (!dashf(feast))
	    feast[0] = '\0';

	  break;
	}
      }
    }
    fclose(fp);
  }

  return feast;
}


int
main()
{
  int i;
  char *fname, *str, *feast, fpath[64];
  FCACHE *fshm;

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  number = total = 0;

  /* --------------------------------------------------- */
  /* ���Ѹ`��					 	 */
  /* --------------------------------------------------- */

  feast = do_today();

  /* --------------------------------------------------- */
  /* ���J�`�Ϊ����� help			 	 */
  /* --------------------------------------------------- */

  strcpy(fpath, "gem/@/@");
  fname = fpath + 7;

  for (i = 0; str = list[i]; i++)
  {
    if (i >= FILM_OPENING0 && i <= FILM_OPENING2 && feast[0])	/* �Y�O�`��A�}�Y�e���θӸ`�骺�e�� */
    {
      mirror(feast, 0);
    }
    else
    {
      strcpy(fname, str);
      mirror(fpath, 0);
    }
  }

  /* itoc.����: ��o�̥H��A���Ӥw�� FILM_MOVIE �g */

  /* --------------------------------------------------- */
  /* ���J�ʺA�ݪO					 */
  /* --------------------------------------------------- */

  /* itoc.����: �ʺA�ݪO���I�q���X�p�u�� MOVIE_MAX - FILM_MOVIE - 1 �g�~�|�Q���i movie */

  sprintf(fpath, "gem/brd/%s/@/@note", BN_CAMERA);	/* �ʺA�ݪO���s���ɮצW�����R�W�� @note */
  do_gem(fpath);					/* �� [note] ��ذϦ��i movie */

#ifdef HAVE_SONG_CAMERA
  brd_fpath(fpath, BN_KTV, FN_DIR);
  do_gem(fpath);					/* ��Q�I���q���i movie */
#endif

  /* --------------------------------------------------- */
  /* resolve shared memory				 */
  /* --------------------------------------------------- */

  image.shot[0] = number;	/* �`�@���X�� */

  fshm = (FCACHE *) shm_new(FILMSHM_KEY, sizeof(FCACHE));
  memcpy(fshm, &image, sizeof(FCACHE));

  /* printf("%d/%d films, %d/%d bytes\n", number, MOVIE_MAX, total, MOVIE_SIZE); */

  exit(0);
}
