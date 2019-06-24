/*-------------------------------------------------------*/
/* util/camera.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : 建立 [動態看板] cache			 */
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
str_strip(str)		/* itoc.060417: 將動態看板每列的寬度掐在 SCR_WIDTH */
  char *str;
{
  int ch, ansi, len;

  /* 若動態看板有一列的寬度超過 SCR_WIDTH，會顯示二列，造成排版錯誤 (主要是點歌的部分)
     所以就乾脆把超過 SCR_WIDTH 的部分刪除 (在此不考慮寬螢幕) */
  /* 若本列中有 \033*s 或 \033*n，顯示出來會更長，所以要特別處理 */

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
      if (ch == '*')	/* KEY_ESC + * + s/n 秀出 ID/username，考慮最大長度 */
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
static int number;	/* 目前已 mirror 幾篇了 */
static int total;	/* 目前已 mirror 幾 byte 了 */


static int		/* 1:成功 0:已超過篇數或容量 */
mirror(fpath, line)
  char *fpath;
  int line;		/* 0:系統文件，不限制列數  !=0:動態看板，line 列 */
{
  int size, i;
  char buf[FILM_SIZ];
  char tmp[ANSILINELEN];
  struct stat st;
  FILE *fp;

  /* 若已經超過最大篇數，則不再繼續 mirror */
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
	/* 動態看板，最多 line 列 */
	if (++i >= line)
	  break;
      }
    }
    fclose(fp);

    if (i != line)	
    {
      /* 動態看板，若不到 line 列，要填滿 line 列 */
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
    if (line)		/* 如果是 動態看板/點歌 缺檔案，就不 mirror */
      return 1;

    /* 如果是系統文件出錯的話，要補上去 */
    sprintf(buf, "請告訴站長檔案 %s 遺失或是過大", fpath);
    size = strlen(buf);
  }

  size++;	/* Thor.980804: +1 將結尾的 '\0' 也算入 */

  i = total + size;
  if (i >= MOVIE_SIZE)	/* 若加入這篇會超過全部容量，則不 mirror */
    return 0;

  memcpy(image.film + total, buf, size);
  image.shot[++number] = total = i;

  return 1;
}


static void
do_gem(folder)		/* itoc.011105: 把看板/精華區的文章收進 movie */
  char *folder;		/* index 路徑 */
{
  char fpath[64];
  FILE *fp;
  HDR hdr;

  if (fp = fopen(folder, "r"))
  {
    while (fread(&hdr, sizeof(HDR), 1, fp) == 1)
    {
      if (hdr.xmode & (GEM_RESTRICT | GEM_RESERVED | GEM_BOARD))	/* 限制級、看板 不放入 movie 中 */
	continue;

      hdr_fpath(fpath, folder, &hdr);

      if (hdr.xmode & GEM_FOLDER)	/* 遇到卷宗則迴圈進去收 movie */
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
lunar_calendar(key, now, ptime)	/* itoc.050528: 由陽曆算農曆日期 */
  char *key;
  time_t *now;
  struct tm *ptime;
{
#if 0	/* Table 的意義 */

  (1) "," 只是分隔，方便給人閱讀而已
  (2) 前 12 個 byte 分別代表農曆 1-12 月為大月或是小月。"L":大月三十天，"-":小月二十九天
  (3) 第 14 個 byte 代表今年農曆閏月。"X":無閏月，"123456789:;<" 分別代表農曆 1-12 是閏月
  (4) 第 16-20 個 bytes 代表今年農曆新年是哪天。例如 "02:15" 表示陽曆二月十五日是農曆新年

#endif

  #define TABLE_INITAIL_YEAR	1970
  #define TABLE_FINAL_YEAR	2037

  char Table[TABLE_FINAL_YEAR - TABLE_INITAIL_YEAR + 1][21] = 
  {
    "L--L-LL-LL-L,X,02:06",	/* 1970 狗年 */
    "-L--LL-LLL-L,5,01:27",	/* 1971 豬年 */
    "-L--L-L-LL-L,X,02:15",	/* 1972 鼠年 */
    "L-L--L--LL-L,X,02:03",	/* 1973 牛年 */
    "LL-L-L--LL-L,4,01:23",	/* 1974 虎年 */
    "LL-L--L--L-L,X,02:11",	/* 1975 兔年 */
    "LL-L-L-L-L-L,8,01:31",	/* 1976 龍年 */
    "L-LL-L-L-L--,X,02:18",	/* 1977 蛇年 */
    "L-LL-LL-L-L-,X,02:07",	/* 1978 馬年 */
    "L--L-L-LL-L-,6,01:28",	/* 1979 羊年 */
    "L--L-L-LL-LL,X,02:16",	/* 1980 猴年 */
    "-L--L--LL-LL,X,02:05",	/* 1981 雞年 */
    "L-L-L--L-LLL,4,01:25",	/* 1982 狗年 */
    "L-L--L--L-LL,X,02:13",	/* 1983 豬年 */
    "L-LL--L--LLL,:,02:02",	/* 1984 鼠年 */
    "-LL-L-L--L-L,X,02:20",	/* 1985 牛年 */
    "-LL-LL-L-L--,X,02:09",	/* 1986 虎年 */
    "L-L-LLLL-L--,6,01:29",	/* 1987 兔年 */
    "L-L-L-LL-LL-,X,02:17",	/* 1988 龍年 */
    "L--L-L-L-LLL,X,02:06",	/* 1989 蛇年 */
    "-L--L-L-LLLL,5,01:27",	/* 1990 馬年 */
    "-L--L--L-LLL,X,02:15",	/* 1991 羊年 */
    "-LL--L--L-LL,X,02:04",	/* 1992 猴年 */
    "-LLL-L--L-L-,3,01:23",	/* 1993 雞年 */
    "LLL-L-L--L-L,X,02:10",	/* 1994 狗年 */
    "-LL-L-LL-L-L,8,01:31",	/* 1995 豬年 */
    "-L-LL-L-LL--,X,02:19",	/* 1996 鼠年 */
    "L-L-L-LL-LL-,X,02:07",	/* 1997 牛年 */
    "L--L-LL-LL-L,5,01:28",	/* 1998 虎年 */
    "L--L--L-LLL-,X,02:16",	/* 1999 兔年 */
    "LL--L--L-LL-,X,02:05",	/* 2000 龍年 */
    "LL-LL--L-L-L,4,01:24",	/* 2001 蛇年 */
    "LL-L-L--L-L-,X,02:12",	/* 2002 馬年 */
    "LL-LL-L--L-L,X,02:01",	/* 2003 羊年 */
    "-LLL-L-L-L-L,2,01:22",	/* 2004 猴年 */
    "-L-L-LL-L-L-,X,02:09",	/* 2005 雞年 */
    "L-L-L-LLL-LL,7,01:29",	/* 2006 狗年 */
    "--L--L-LLL-L,X,02:18",	/* 2007 豬年 */
    "L--L--L-LL-L,X,02:07",	/* 2008 鼠年 */
    "LL--L-L-L-LL,5,01:26",	/* 2009 牛年 */
    "L-L-L--L-L-L,X,02:14",	/* 2010 虎年 */
    "L-LL-L--L-L-,X,02:03",	/* 2011 兔年 */
    "L-LLL-L-L-L-,4,01:23",	/* 2012 龍年 */
    "L-L-LL-L-L-L,X,02:10",	/* 2013 蛇年 */
    "-L-L-L-LLL-L,9,01:31",	/* 2014 馬年 */
    "-L--L-LLL-L-,X,02:19",	/* 2015 羊年 */
    "L-L--L-LL-LL,X,02:08",	/* 2016 猴年 */
    "-L-L---L-LLL,6,01:28",	/* 2017 雞年 */
    "-L-L--L-L-LL,X,02:16",	/* 2018 狗年 */
    "L-L-L--L--LL,X,02:05",	/* 2019 豬年 */
    "-LLLL--L-L-L,4,01:25",	/* 2020 鼠年 */
    "-LL-L-L-L-L-,X,02:12",	/* 2021 牛年 */
    "L-L-LL-L-L-L,X,02:01",	/* 2022 虎年 */
    "-L-LL-LL-L-L,2,01:22",	/* 2023 兔年 */
    "-L--L-LL-LL-,X,02:10",	/* 2024 龍年 */
    "L-L--LL-LLL-,6,01:29",	/* 2025 蛇年 */
    "L-L--L--LLL-,X,02:17",	/* 2026 馬年 */
    "LL-L--L--LL-,X,02:06",	/* 2027 羊年 */
    "LLL-L-L--LL-,5,01:26",	/* 2028 猴年 */
    "LL-L-L-L--LL,X,02:13",	/* 2029 雞年 */
    "-L-LL-L-L-L-,X,02:03",	/* 2030 狗年 */
    "-LLL-LL-L-L-,3,01:23",	/* 2031 豬年 */
    "L--L-LL-LL-L,X,02:11",	/* 2032 鼠年 */
    "-L--L-L-LLLL,;,01:31",	/* 2033 牛年 */
    "-L--L-L-LL-L,X,02:19",	/* 2034 虎年 */
    "L-L--L--LL-L,X,02:08",	/* 2035 兔年 */
    "LL-L----L-LL,6,01:28",	/* 2036 龍年 */
    "LL-L--L--L-L,X,02:15",	/* 2037 蛇年 */
#if 0
    "LL-L-L-L--L-,X,02:04",	/* 2038 馬年 */
    "LL-LLL-L-L--,5,01:24",	/* 2039 羊年 */
    "L-LL-L-LL-L-,X,02:12",	/* 2040 猴年 */
    "-L-L-LL-LL-L,X,02:01",	/* 2041 雞年 */
    "-L-L-L-LL-LL,2,01:22",	/* 2042 狗年 */
    "-L--L--LL-LL,X,02:10",	/* 2043 豬年 */
    "L-L--L-L-LLL,7,01:30",	/* 2044 鼠年 */
    "L-L--L--L-LL,X,02:17",	/* 2045 牛年 */
    "L-L-L-L--L-L,X,02:06",	/* 2046 虎年 */
    "L-LL--L--L-L,5,01:26",	/* 2047 兔年 */
    "-LL-LL-L--L-,X,02:14",	/* 2048 龍年 */
    "L-L-LL-LL-L-,X,02:02",	/* 2049 蛇年 */
    "-L--L-LL-LL-,3,01:23",	/* 2050 馬年 */
    "L--L--LL-LLL,X,02:11",	/* 2051 羊年 */
    "-L--L--LLLLL,8,02:01",	/* 2052 猴年 */
    "-L--L--L-LLL,X,02:19",	/* 2053 雞年 */
    "-LL--L--L-LL,X,02:08",	/* 2054 狗年 */
    "-LL-L---L-L-,6,01:28",	/* 2055 豬年 */
    "LLL-L-L--L-L,X,02:15",	/* 2056 鼠年 */
    "-LL-L-LL--L-,X,02:04",	/* 2057 牛年 */
    "L-L--LL-LL--,4,01:24",	/* 2058 虎年 */
    "L-L-L-L-LLL-,X,02:12",	/* 2059 兔年 */
    "L--L--L-LLL-,X,02:02",	/* 2060 龍年 */
    "LL-L--L-LLL-,3,01:21",	/* 2061 蛇年 */
    "LL--L--L-LL-,X,02:09",	/* 2062 馬年 */
    "LL-L-L-L-L-L,7,01:29",	/* 2063 羊年 */
    "LL-L-L--L-L-,X,02:17",	/* 2064 猴年 */
    "LL-LL-L--L-L,X,02:05",	/* 2065 雞年 */
    "-L-LLL-L-L-L,5,01:26",	/* 2066 狗年 */
    "-L-L-LL-L-L-,X,02:14",	/* 2067 豬年 */
    "L-L--LL-LL-L,X,02:03",	/* 2068 鼠年 */
    "-L-L-L-LLL-L,4,01:23",	/* 2069 牛年 */
    "-L-L--L-LL-L,X,02:11",	/* 2070 虎年 */
    "L-L-L--LL-LL,8,01:31",	/* 2071 兔年 */
    "L-L-L--L-L-L,X,02:19",	/* 2072 龍年 */
    "L-LL-L--L-L-,X,02:07",	/* 2073 蛇年 */
    "L-LL-LL-L-L-,6,01:27",	/* 2074 馬年 */
    "L-L-LL-L-L-L,X,02:15",	/* 2075 羊年 */
    "-L-L-L-LL-L-,X,02:05",	/* 2076 猴年 */
    "L-L-L-LLL-L-,4,01:24",	/* 2077 雞年 */
    "L-L--L-LL-LL,X,02:12",	/* 2078 狗年 */
    "-L-L--L-L-LL,X,02:02",	/* 2079 豬年 */
    "L-LL--L--LLL,3,01:22",	/* 2080 鼠年 */
    "-LL-L--L--LL,X,02:09",	/* 2081 牛年 */
    "-LLL--LL--LL,7,01:29",	/* 2082 虎年 */
    "-LL-L-L-L-L-,X,02:17",	/* 2083 兔年 */
    "L-L-LL-L-L-L,X,02:06",	/* 2084 龍年 */
    "-L--L-LL-L-L,5,01:26",	/* 2085 蛇年 */
    "-L--L-LL-LL-,X,02:14",	/* 2086 馬年 */
    "L-L--L-L-LLL,X,02:03",	/* 2087 羊年 */
    "-L-L-L--LLL-,4,01:24",	/* 2088 猴年 */
    "LL-L--L--LL-,X,02:10",	/* 2089 雞年 */
    "LLL-L--L-LL-,8,01:30",	/* 2090 狗年 */
    "LL-L-L-L--L-,X,02:18",	/* 2091 豬年 */
    "LL-LL-L-L-L-,X,02:07",	/* 2092 鼠年 */
    "-LL-L-L-L-L-,6,01:27",	/* 2093 牛年 */
    "-L-L-LL-LL-L,X,02:15",	/* 2094 虎年 */
    "-L--L-L-LLL-,X,02:05",	/* 2095 兔年 */
    "L-L-L--LLL-L,4,01:25",	/* 2096 龍年 */
    "L-L--L--LL-L,X,02:12",	/* 2097 蛇年 */
    "LL-L---L-L-L,X,02:01",	/* 2098 馬年 */
    "LLLL--L--L-L,2,01:21",	/* 2099 羊年 */
    "LL-L-L-L--L-,X,02:09",	/* 2100 猴年 */
#endif
  };

  char year[21];

  time_t nyd;	/* 今年的農曆新年 */
  struct tm ntime;

  int i;
  int Mon, Day;
  int leap;	/* 0:本年無閏月 */

  /* 先找出今天是農曆哪一年 */

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

  /* 再從農曆正月初一開始數到今天 */

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
	year[Mon - 1] = '-';	/* 閏月必小月 */
      }
      else if (year[13] == Mon + '0')
      {
	if (leap == 1)		/* 下月是閏月 */
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
  char key1[6];			/* mm/dd: 陽曆 mm月dd日 */
  char key2[6];			/* mm/#A: 陽曆 mm月的第#個星期A */
  char key3[6];			/* MM\DD: 農曆 MM月DD日 */
  time_t now;
  struct tm *ptime;
  static char feast[64];

  time(&now);
  ptime = localtime(&now);
  sprintf(key1, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
  sprintf(key2, "%02d/%d%c", ptime->tm_mon + 1, (ptime->tm_mday - 1) / 7 + 1, ptime->tm_wday + 'A');
  lunar_calendar(key3, &now, ptime);

  today = image.today;
  sprintf(today, "%s %.2s", key1, "日一二三四五六" + (ptime->tm_wday << 1));

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
  /* 今天節日					 	 */
  /* --------------------------------------------------- */

  feast = do_today();

  /* --------------------------------------------------- */
  /* 載入常用的文件及 help			 	 */
  /* --------------------------------------------------- */

  strcpy(fpath, "gem/@/@");
  fname = fpath + 7;

  for (i = 0; str = list[i]; i++)
  {
    if (i >= FILM_OPENING0 && i <= FILM_OPENING2 && feast[0])	/* 若是節日，開頭畫面用該節日的畫面 */
    {
      mirror(feast, 0);
    }
    else
    {
      strcpy(fname, str);
      mirror(fpath, 0);
    }
  }

  /* itoc.註解: 到這裡以後，應該已有 FILM_MOVIE 篇 */

  /* --------------------------------------------------- */
  /* 載入動態看板					 */
  /* --------------------------------------------------- */

  /* itoc.註解: 動態看板及點歌本合計只有 MOVIE_MAX - FILM_MOVIE - 1 篇才會被收進 movie */

  sprintf(fpath, "gem/brd/%s/@/@note", BN_CAMERA);	/* 動態看板的群組檔案名稱應命名為 @note */
  do_gem(fpath);					/* 把 [note] 精華區收進 movie */

#ifdef HAVE_SONG_CAMERA
  brd_fpath(fpath, BN_KTV, FN_DIR);
  do_gem(fpath);					/* 把被點的歌收進 movie */
#endif

  /* --------------------------------------------------- */
  /* resolve shared memory				 */
  /* --------------------------------------------------- */

  image.shot[0] = number;	/* 總共有幾片 */

  fshm = (FCACHE *) shm_new(FILMSHM_KEY, sizeof(FCACHE));
  memcpy(fshm, &image, sizeof(FCACHE));

  /* printf("%d/%d films, %d/%d bytes\n", number, MOVIE_MAX, total, MOVIE_SIZE); */

  exit(0);
}
