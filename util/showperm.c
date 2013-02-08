/*-------------------------------------------------------*/
/* util/showperm.c	( NTHU CS MapleBBS Ver 3.00 )        */
/*-------------------------------------------------------*/
/* target : 顯示擁有某種權限的使用者                     */
/* author : chensc.bbs@sony.tfcis.org                    */
/* create : 06/09/03                                     */
/* update :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

int 
main()
{
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
     // printf("%s: read error (maybe no such id?)\n", usr[i].userid);
      continue;
    }

		if(HAS_PERM(PERM_CHANGEID))
			printf("%s\n",usr[i].userid);

    if (rec_put(fpath, &cuser, sizeof(cuser), 0, NULL) < 0)
      printf("%s: write error\n", usr[i].userid);

  }
}
