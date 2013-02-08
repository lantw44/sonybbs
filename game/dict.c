/*-------------------------------------------------------*/
/* target : Yahoo �u�W�r��                               */
/* create : 01/07/09                                     */
/* update : 06/01/16                                     */
/* author : statue.bbs@bbs.yzu.edu.tw                    */
/* change : kyo@cszone.tw                                */
/*-------------------------------------------------------*/

#if 0

 ���q
 http://tw.dictionary.yahoo.com/search?ei=big5&p=hello

 �P�q�r/�ϸq�r
 http://tw.dictionary.yahoo.com/search?ei=big5&p=hello&type=t

 �ܤƧ�
 http://tw.dictionary.yahoo.com/search?ei=big5&p=hello&type=v

#endif


#include "bbs.h"

#ifdef HAVE_NETTOOL

#define mouts(y,x,s)    { move(y, x); outs(s); }

#define HTTP_PORT       80
#define SERVER_yahoo    "tw.dictionary.yahoo.com"
#define CGI_yahoo       "/search?ei=big5&p="
#define CGI_yahoo2      "&type="
#define REF             "http://tw.dictionary.yahoo.com/"
#define Dict_Name       "Yahoo! �u�W�r��"

static void
url_encode(dst, src)        /* URL encoding */
 uschar *dst;       /* Thor.990331: �n src ���T���Ŷ� */
 uschar *src;
{
 for (; *src; src++)
 {
   if (*src == ' ')
     *dst++ = '+';
   else if (is_alnum(*src))
     *dst++ = *src;
   else
   {
     register cc = *src;
     *dst++ = '%';
     *dst++ = radix32[cc >> 4];
     *dst++ = radix32[cc & 0xf];
   }
 }
 *dst = '\0';
}


static int
write_file(type, sockfd, fp, ftmp)
 char type;
 int sockfd;
 FILE *fp;
 char *ftmp;
{
 static char pool[2048];
 int cc, i, j, fsize;
 char *xhead, *fimage;
 int show, start_show;
 int space;                /* �b html ���A�s�� space �u�|��@�� */

 char *start_str[] =
 {
   "d", "<div class=pcixin>",
   "d", "<div class=chinese>",
   "t", "<h4>",
   "v", "<h4>",
   NULL
 };

 char *stop_str[] =
 {
   "d", "</h2>",
   "A", "<h4>",
   "A", "</blockquote>",
   NULL
 };

 char *newline_str[] =      /* ���N����r�����Ÿ� */
 {
   "<br>",
   "</div>",
   "</li>",
   NULL
 };

 char *other_str[] =        /* ��L�r���Ÿ� */
 {
   "A", "&nbsp;", " ",
   "d", "<li><div class=pexplain>", "\033[m\n  \033[1;33m��\033[32m",
   "d", "<li>\n<div class=pexplain>", "\033[m\n  \033[1;33m��\033[32m",
   "d", "<li>", "\033[m\n  \033[1;33m��\033[32m",
   "d", "<div class=pexplain>", "\033[m\n    ",
   "d", "<div class=pcixin>", "\033[1;31m",
   "d", "<span id=dropdownid>", "\033[m      \033[1;37m",
   "d", "<div class=ptitle>", "\033[1;36m",
   "t", "<div class=ptitle>", "\033[m\n\n\033[1;36m",
   "t", "<div class=pexplain>", "\033[m\n",
   "v", "<div class=ptitle>", "\033[m\n\n\033[1;36m",
   "v", "<div class=pexplain>", "\033[m\n",
   NULL
 };

 FILE *fpw;

 fpw = fopen(ftmp, "w");

 while ((cc = read(sockfd, &pool, sizeof(pool))) > 0)
   fwrite(&pool, cc, 1, fpw);

 fclose(fpw);
 close(sockfd);

 sprintf(pool, "/usr/local/bin/autob5 -i utf8 -o big5 < %s > %s.big5",
   ftmp, ftmp);
 system(pool);
 unlink(ftmp);
 strcat(ftmp, ".big5");

 fimage = f_map(ftmp, &fsize);
 if (fimage == (char *) -1)
 {
   fclose(fp);
   vmsg("�ثe�L�k�}�� Yahoo �u�W�r��Ȧs��");
   return -1;
 }

 /* parser return message from web server */
 show = 1;
 start_show = 0;
 space = 0;

 for (xhead = fimage; *xhead; xhead++)
 {
   if (!start_show)
   {
     for (i = 0; start_str[i] != NULL; i += 2)
     {
       j = strlen(start_str[i + 1]);
       if ((start_str[i][0] == type || start_str[i][0] == 'A') &&
         str_ncmp(xhead, start_str[i + 1], j) == 0)
       {
         fputs("\033[m\n\n\033[1;31m", fp);
         start_show = 1;
         xhead += j;
         break;
       }
     }
   }
   else if (start_show)
   {
     for (i = 0; stop_str[i] != NULL; i += 2)
     {
       j = strlen(stop_str[i + 1]);
       if ((stop_str[i][0] == type || stop_str[i][0] == 'A') &&
         str_ncmp(xhead, stop_str[i + 1], j) == 0)
       {
         start_show = 0;
         xhead += j;
         break;
       }
     }
   }

   if (!start_show)
     continue;

   for (i = 0; newline_str[i] != NULL; i++)
   {
     j = strlen(newline_str[i]);
     if (str_ncmp(xhead, newline_str[i], j) == 0)
     {
       fputs("\033[m\n", fp);
       xhead += j;
       space = 0;
       break;
     }
   }


   for (i = 0; other_str[i] != NULL; i += 3)
   {
     j = strlen(other_str[i + 1]);
     if ((other_str[i][0] == type || other_str[i][0] == 'A') &&
       str_ncmp(xhead, other_str[i + 1], j) == 0)
     {
       fputs(other_str[i + 2], fp);
       xhead += j;
       space = 0;
       break;
     }
   }

   /* ���Ҳ��L */

   cc = *xhead;
   switch(cc)
   {
   case '<':
     show = 0;
     continue;
   case '>':
     show = 1;
     continue;
   case '\n':
   case '\r':
     continue;
   case ' ':
     if (space)
       continue;
     space = 1;
   }

   if (show)
     fputc(cc, fp);

   if (cc != ' ')
     space = 0;
 }
 fputc('\n', fp);

 munmap(fimage, fsize);
 unlink(ftmp);

 return 1;
}


static int
http_conn(server, word, type, s)
 char *server;
 char *word;
 char type;
 char *s;
{
 int sockfd;
 FILE *fp;
 char fname[64], ftmp[64], *str;

 if ((sockfd = dns_open(server, HTTP_PORT)) < 0)
 {
   vmsg("�L�k�P���A�����o�s���A�d�ߥ���");
   return 0;
 }
 else
 {
   mouts(22, 0, "���b�s�����A���A�еy��(�����N�����}).............");
   refresh();
 }
 write(sockfd, s, strlen(s));
 shutdown(sockfd, 1);

 sprintf(fname, "tmp/%s.yahoo_dict", cuser.userid);

 fp = fopen(fname, "w");
 sprintf(ftmp, "tmp/%s.yahoo_dict.tmp", cuser.userid);
 str = strchr(s + 4, ' ');

 if (str)
   *str = '\0';

 fprintf(fp, "%-24s\033[1;37;44m  " Dict_Name "  \033[m\n%s\n",
   "", msg_seperator);
 fprintf(fp, "�ӭ��s���Ghttp://%s%s\n\n", server, s + 4);
 fprintf(fp, "%s", word);

 outz("�^����Ƥ��A�еy��...");
 refresh();

 if (write_file(type, sockfd, fp, ftmp) >= 0)
   fprintf(fp, "\n\n%s\n%-26s\033[1;36;40mYahoo! \033[37m�u�W�r��\033[m\n",
     msg_seperator, "");

 fclose(fp);

 more(fname, (char *) -1);
 unlink(fname);
 return 0;
}


static void
yahoo_dict(word, type)
 char *word;
 char type;
{
 char sendform[512];
 char ue_word[90];

 url_encode(ue_word, word);

 if (type == 'd')
   sprintf(sendform, "GET " CGI_yahoo "%s HTTP/1.0\n\n", ue_word);
 else
   sprintf(sendform, "GET " CGI_yahoo "%s" CGI_yahoo2 "%c HTTP/1.0\n\n",
    ue_word, type);

 http_conn(SERVER_yahoo, word, type, sendform);
}


int
main_yahoo()
{
 char word[30];
 char *menu[] = {"DD",
     "D  ���q",
     "T  �P�q�r/�ϸq�r",
     "V  �ܤƧ�", NULL};

 while (1)
 {
   clear();
   move(0, 25);
   outs("\033[1;37;44m�� " Dict_Name " ��\033[m");
   move(3, 0);
   outs("���r��ӷ��� " Dict_Name "�C\n\n");
   outs("���}�G" REF);

   if (!vget(8, 0, "�d�ߦr�J�G", word, 30, DOECHO))
     break;

   yahoo_dict(word, pans(-1, -1, "�d�߿ﶵ", menu));
 }

 return 0;
}
#endif  /* HAVE_NETTOOL */