#ifndef DBO_MYSQL_H
#define DBO_MYSQL_H

#include <cda/c_define.h>
#include <malloc.h>
#include <cda/c_string.h>
#include <mysql/mysql.h>
#include "dbo_base.h"

typedef struct dbo_mysql_link
{
	MYSQL *sql_link;
}DBO_MYSQL_LINK;

typedef struct dbo_mysql_data
{
	MYSQL_RES *sql_data;
}DBO_MYSQL_RESULT;

typedef struct dbo_mysql_row
{
	MYSQL_ROW sql_row;
}DBO_MYSQL_ROW;

void dbo_Init();

DBO_MYSQL_LINK* dbo_mysql_Connect(DBO_MYSQL_LINK *link,CHAR_ *user_name,CHAR_ *user_password,CHAR_ *server_name,CHAR_ *database_name);

DBO_MYSQL_RESULT* dbo_mysql_Query(DBO_MYSQL_RESULT *result_data,CHAR_ *sql_command,DBO_MYSQL_LINK *link);

BOOL_ dbo_mysql_Exec(DBO_MYSQL_LINK *tag_db,CHAR_ *sql_command);

DBO_MYSQL_ROW* dbo_mysql_Fetch(DBO_MYSQL_ROW *result_row,DBO_MYSQL_RESULT *tag_data);

DBO_MYSQL_ROW* dbo_mysql_Fetch_Ex(DBO_MYSQL_ROW *result_row,UINT_ row,DBO_MYSQL_LINK *link);

C_STRING dbo_mysql_Get(C_STRING *tag_string,UINT_ field,DBO_MYSQL_ROW *tag_row);

void dbo_Free_mysql_result(DBO_MYSQL_RESULT *tag_data);

void dbo_mysql_Drop(DBO_MYSQL_LINK *tag_db);

void dbo_End();

#endif
