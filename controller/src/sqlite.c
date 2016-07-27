#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

int executeNoQuery(sqlite3 *db,const char *sql)
{
	sqlite3_stmt *pstmt = NULL;

	if(sqlite3_prepare_v2(db,sql,strlen(sql),&pstmt,NULL) != SQLITE_OK)
	{
		if(pstmt != NULL)
			sqlite3_finalize(pstmt);
		return -1;
	}
	if(sqlite3_step(pstmt) != SQLITE_DONE)
	{
		sqlite3_finalize(pstmt);
		return -1;
	}
	sqlite3_finalize(pstmt);
	return 0;
}

int executeWithQuery(sqlite3 *db,char ***result,int *col,const char *sql)
{
	int ret,row;

	ret = sqlite3_get_table(db,sql,result,&row,col,NULL);
	if(ret != SQLITE_OK)
	{
		return -1;
	}
	(*result)[(row+1)*(*col)] = NULL;
	return 0;
}
