#include "dbo_base.h"

DBO_LINK* dbo_Connect(DBO_LINK *link,LINK_TYPE type,CHAR_ *user_name,CHAR_ *user_password,CHAR_ *server_name,CHAR_ *database_name)
{
	
};

DBO_RESULT* dbo_Query(DBO_RESULT *result_data,CHAR_ *sql_command,DBO_LINK *link)
{

};

BOOL_ dbo_Exec(DBO_LINK *tag_db,CHAR_ *sql_command)
{

};

DBO_ROW* dbo_Fetch(DBO_ROW *result_row,DBO_RESULT *tag_data)
{

};

DBO_ROW* dbo_Fetch_Ex(DBO_ROW *result_row,UINT_ row,DBO_LINK *link)
{

};

CHAR_* dbo_Get(UINT_ field,DBO_ROW *tag_row)
{

};

void dbo_Free_result(DBO_RESULT *tag_data)
{

};

void dbo_Drop(DBO_LINK *tag_db)
{

};