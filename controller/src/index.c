#include "include/common.h"

void setSession(sqlite3 *db,int userid);
int getSession(sqlite3 *db);
void touchSession();
void createUUID();

int cgiMain()
{
	char action[256] = {0};

	touchSession();
	if(cgiFormString("action",action,sizeof(action)) != cgiFormSuccess)
	{
		printHtmlHeader();
		showError("Please login");
		return -1;
	}
	if(strcmp(action,"login") == 0)
	{
		printHtmlHeader();
		TMPL_write("../view/Login.html",0,0,0,cgiOut,cgiOut);
		return 0;
	}
	else if(strcmp(action,"LoginSubmit") == 0)
	{

		char username[256]={0},password[256]={0},sql[1024]={0};
		int i,nCol,id=0;
		char **array;sqlite3 *db = NULL;

		if(cgiFormString("username",username,sizeof(username)) != cgiFormSuccess)
		{
			TMPL_varlist *varlist = 0;
			varlist = TMPL_add_var(varlist,"need_username","/* username must be filled */",0);
			printHtmlHeader();
			TMPL_write("../view/Login.html",0,0,varlist,cgiOut,cgiOut);
			return -1;
		}
		if(cgiFormString("password",password,sizeof(password)) != cgiFormSuccess)
		{
			TMPL_varlist *varlist = 0;
			varlist = TMPL_add_var(varlist,"need_password","/* password must be filled */",0);
			printHtmlHeader();
			TMPL_write("../view/Login.html",0,0,varlist,cgiOut,cgiOut);
			return -1;
		}
		if(sqlite3_open("../user.sqlite",&db) != SQLITE_OK)
		{
			printHtmlHeader();
			showError(sqlite3_errmsg(db));
			return -1;
		}
		sprintf(sql,"select ID from T_users where UserName='%s' and Password='%s';",username,password);
		if(executeWithQuery(db,&array,&nCol,sql) == -1)
		{
			printHtmlHeader();
			showError(sqlite3_errmsg(db));
			if(db != NULL)
				sqlite3_close(db);
			return -1;
		}
		i=0;
		while(array[i+nCol] != NULL)
		{
			id = atoi(array[i+nCol]);
			setSession(db,id);
			i+=nCol;
		}
		sqlite3_free_table(array);
		//说明用户名和密码不正确
		if(i == 0)
		{
			TMPL_varlist *varlist = 0;

			varlist = TMPL_add_var(varlist,"check_exist","username,password missmatch,please retry",0);
			printHtmlHeader();
			TMPL_write("../view/Login.html",0,0,varlist,cgiOut,cgiOut);
			return -1;
		}
		else if(i>0)
		{
			cgiHeaderLocation("index.cgi?action=Main");		
		}
		if(db != NULL)
			sqlite3_close(db);
	}
	else if(strcmp(action,"Main") == 0)
	{
		char sql[1024]={0},username[256]={0};
		sqlite3 *db = NULL;
		int i,nCol;
		char **array;
		TMPL_varlist *varlist = 0;
		if(sqlite3_open("../user.sqlite",&db) != SQLITE_OK)
		{
			printHtmlHeader();
			showError(sqlite3_errmsg(db));
			return -1;
		}
		int userid = getSession(db);

		if(userid == -1)
		{
			cgiHeaderLocation("index.cgi?action=login");
		}
		else
		{
			sprintf(sql,"select UserName from T_users where ID=%d;",userid);
			if(executeWithQuery(db,&array,&nCol,sql) == -1)
			{
				printHtmlHeader();
				showError(sqlite3_errmsg(db));
				return -1;
			}
			i=0;
			while(array[i+nCol] != NULL)
			{
				sprintf(username,"%s",array[i+nCol]);
				i+=nCol;
			}
			sqlite3_free_table(array);

			varlist = TMPL_add_var(varlist,"user",username,0);
			printHtmlHeader();
			TMPL_write("../view/Main.html",0,0,varlist,cgiOut,cgiOut);
		}

		if(db != NULL)
			sqlite3_close(db);

		return 0;
	}
	else if(strcmp(action,"logout") == 0)
	{
		char sessionid[512]={0};
		if(cgiCookieString("SessionId",sessionid,sizeof(sessionid)) == cgiFormSuccess)
		{
			cgiHeaderCookieSetString("SessionId",sessionid,0,"",HOST);
			cgiHeaderLocation("index.cgi?action=login");
		}
		else
		{
			return -1;
		}
	}
	else
	{
		printHtmlHeader();
		showError("wrong action");
		return -1;
	}


	return 0;
}

void setSession(sqlite3 *db,int userid)
{
	int needCreate = 0;
	int i,nCol,count=0;
	char **array;
	char sql[1024]={0};
	char sessionId[512]={0};

	if(cgiCookieString("SessionId",sessionId,sizeof(sessionId)) == cgiFormSuccess)
	{
		sprintf(sql,"select count(*) from T_sessions;");
		if(executeWithQuery(db,&array,&nCol,sql) == -1)
		{
			printHtmlHeader();
			showError(sqlite3_errmsg(db));
			return;
		}
		i=0;
		while(array[i+nCol] != NULL)
		{
			count = atoi(array[i+nCol]);
			i+=nCol;
		}
		sqlite3_free_table(array);
		
		if(count > 0)
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,
					"update T_sessions set UserId=%d,LastUpdateTime=datetime('now','localtime') where ID='%s';",
					userid,sessionId);
			if(executeNoQuery(db,sql) == -1)
			{
				printHtmlHeader();
				showError(sqlite3_errmsg(db));
				//showError(sql);
				return;
			}
		}
		else
		{
			needCreate = 1;
		}
	}
	else
	{
		needCreate = 1;
	}
	if(needCreate)
	{
		createUUID(sessionId);
		cgiHeaderCookieSetString("SessionId",sessionId,10*60,"",HOST);
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into T_sessions(ID,UserId,LastUpdateTime) values('%s',%d,datetime('now','localtime'));",
				sessionId,userid);
		if(executeNoQuery(db,sql) == -1)
		{
			printHtmlHeader();
			showError(sqlite3_errmsg(db));
			//showError(sql);
			return;
		}
	}
}

int getSession(sqlite3 *db)
{
	char sessionId[512]={0};
	char sql[1024]={0};
	int i,nCol,userid=-1;
	char **array;

	if(cgiCookieString("SessionId",sessionId,sizeof(sessionId)) == cgiFormSuccess)
	{
		sprintf(sql,"select UserId from T_sessions where ID='%s';",
				sessionId);
		if(executeWithQuery(db,&array,&nCol,sql) == -1)
		{
			printHtmlHeader();
			showError(sqlite3_errmsg(db));
			//showError(sql);
			return -1;
		}
		i=0;
		while(array[i+nCol] != NULL)
		{
			userid = atoi(array[i+nCol]);
			i+=nCol;
		}
		sqlite3_free_table(array);
		return userid;
	}
	else
	{
		return -1;
	}
}
void touchSession()
{
	char sessionid[256]={0};
	char sql[1024]={0};
	sqlite3 *conn = NULL;

	if(sqlite3_open("../user.sqlite",&conn) != SQLITE_OK)
	{
		printHtmlHeader();
		showError(sqlite3_errmsg(conn));
		if(conn != NULL)
			sqlite3_close(conn);
		return;
	}
	strcat(sql,"delete from T_sessions where strftime('%s','now','localtime')-strftime('%s',T_sessions.LastUpdateTime)>10*60;");
	if(executeNoQuery(conn,sql) == -1)
	{
		printHtmlHeader();
		showError(sqlite3_errmsg(conn));
		if(conn != NULL)
			sqlite3_close(conn);
		return;
	}
	if(cgiCookieString("SessionId",sessionid,sizeof(sessionid)) == cgiFormSuccess)
	{
		memset(sql,0,sizeof(sql));
		sprintf(sql,
				"update T_sessions set LastUpdateTime=datetime('now','localtime') where ID='%s';",
				sessionid);
		if(executeNoQuery(conn,sql) == -1)
		{
			printHtmlHeader();
			showError(sqlite3_errmsg(conn));
			if(conn != NULL)
				sqlite3_close(conn);
			return;
		}
		cgiHeaderCookieSetString("SessionId",sessionid,10*60,"",HOST);
	}
	if(conn != NULL)
		sqlite3_close(conn);
}
void createUUID(char *uuid)
{
	int i;
	uuid_t uu;
	char tmp[10];

	uuid_generate(uu);
	for(i=0; i<16; i++)
	{
		sprintf(tmp,"%x",uu[i]);
		strcat(uuid,tmp);
		memset(tmp,0,sizeof(tmp));
	}
}
