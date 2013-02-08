/*-------------------------------------------------------*/
/* util/changeperm.c	( NTHU CS MapleBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* target : 去除每位使用者更改ID的權限                   */
/* author : chensc.bbs@sony.tfcis.org                    */
/* create : 06/09/03                                     */
/* update :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

int 
main()
{
	printf("Be SURE to change all user's perm, continue? y/n : ");
	if(getchar() !='y')
		exit(1);

  int i,fd;
	SCHEMA *usr;
	struct stat st;

  chdir(BBSHOME);

  if ((fd = open(FN_SCHEMA, O_RDONLY)) < 0)
  {
    printf("ERROR at open file\n");
    exit(1);
  }

  fstat(fd, &st);
  usr = (SCHEMA *) malloc(st.st_size);
  read(fd, usr, st.st_size);
  close(fd);

	fd = st.st_size / sizeof(SCHEMA);

  for (i = 0; i < fd; i++)
  {
    ACCT cuser;
    char fpath[64];

    usr_fpath(fpath, usr[i].userid, FN_ACCT);
    if (rec_get(fpath, &cuser, sizeof(cuser), 0) < 0)
    {
      printf("%s: read error (maybe no such id?)\n", usr[i].userid);
      continue;
    }

		if (HAS_PERM(PERM_CHANGEID))
	    cuser.userlevel ^= PERM_CHANGEID;

    if (rec_put(fpath, &cuser, sizeof(cuser), 0, NULL) < 0)
      printf("%s: write error\n", usr[i].userid);

		printf("%s : done !\n",usr[i].userid);
  }
}
