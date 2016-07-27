#include "include/common.h"
void printHtmlHeader()
{
	cgiHeaderContentType("text/html;charset=gbk");
}

void showError(char *msg)
{
	TMPL_varlist *varlist = 0;
	varlist = TMPL_add_var(varlist,"err_msg",msg,0);
	TMPL_write("../view/Error.html",0,0,varlist,cgiOut,cgiOut);
}
