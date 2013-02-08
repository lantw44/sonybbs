/*-------------------------------------------------------*/
/* util/give_paycheck.c ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 發給全站使用者支票                           */
/* create : 05/04/18                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#if 0

 給 itoc 一張 100 銀 200 金 的支票
 % ~/bin/give_paycheck 100 200 itoc

 給 sysop 一張 100 銀 0 金 的支票
 % ~/bin/give_paycheck 100 0 sysop

 給全站使用者各一張 50 銀 30 金 的支票
 % ~/bin/give_paycheck 50 30

 給全站使用者支票要執行一段時間，請耐心等候

#endif


#include "bbs.h"


int
main(argc, argv)
 int argc;
 char *argv[];
{
 int money, gold;
 char c, *str, buf[64];
 struct dirent *de;
 DIR *dirp;
 PAYCHECK paycheck;

 if (argc == 3 || argc == 4)
 {
   money = atoi(argv[1]);
   gold = atoi(argv[2]);
   if (money < 0 || gold < 0)
     printf("money and gold should not be negative.\n");
 }
 else
 {
   printf("Usage: %s money gold [userid]\n", argv[0]);
   return -1;
 }

 memset(&paycheck, 0, sizeof(PAYCHECK));
 time(&paycheck.tissue);
 paycheck.money = money;
 paycheck.gold = gold;
 //strcpy(paycheck.reason, "[發錢] 站長");
 strcpy(paycheck.reason, "乖乖站民有紅包");

 if (argc == 4)
 {
   chdir(BBSHOME);
   usr_fpath(buf, argv[3], FN_PAYCHECK);
   rec_add(buf, &paycheck, sizeof(PAYCHECK));
   return 0;
 }

 for (c = 'a'; c <= 'z'; c++)
 {
   sprintf(buf, BBSHOME "/usr/%c", c);
   chdir(buf);

   if (!(dirp = opendir(".")))
     continue;

   while (de = readdir(dirp))
   {
     str = de->d_name;
     if (*str <= ' ' || *str == '.')
       continue;

     sprintf(buf, "%s/" FN_PAYCHECK, str);
     rec_add(buf, &paycheck, sizeof(PAYCHECK));
   }

   closedir(dirp);
 }

 return 0;
}
