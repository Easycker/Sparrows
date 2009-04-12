#include "dbo_mysql.h"

DBO_MYSQL_LINK* dbo_mysql_Connect(DBO_MYSQL_LINK *link,CHAR_ *user_name,CHAR_ *user_password,CHAR_ *server_name,CHAR_ *database_name)
{
	char *user_name_ansi;
	char *user_password_ansi;
	char *server_name_ansi;
	char *database_name_ansi;

	#ifndef WITHOUT_WIDECHAR_SUPPORT_
	user_name_ansi=array_Create(sizeof(char));
	user_password_ansi=array_Create(sizeof(char));
	server_name_ansi=array_Create(sizeof(char));
	database_name_ansi=array_Create(sizeof(char));

	string_Widetoansi(&user_name_ansi,user_name);
	string_Widetoansi(&user_password_ansi,user_password);
	string_Widetoansi(&server_name_ansi,server_name);
	string_Widetoansi(&database_name_ansi,database_name);
	#else
	user_name_ansi=user_name;
	user_password_ansi=user_name;
	server_name_ansi=server_name;
	database_name_ansi=database_name;
	#endif

	link->sql_link=mysql_init(NULL);
	mysql_real_connect(link->sql_link,server_name_ansi,user_name_ansi,user_password_ansi,database_name_ansi,0,NULL,CLIENT_MULTI_STATEMENTS);

	#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(user_name_ansi);
	array_Drop(user_password_ansi);
	array_Drop(server_name_ansi);
	array_Drop(database_name_ansi);
	#endif

	return link;
};

DBO_MYSQL_RESULT* dbo_mysql_Query(DBO_MYSQL_RESULT *result_data,CHAR_ *sql_command,DBO_MYSQL_LINK *link)
{
	char *sql_command_ansi;
	
	#ifndef WITHOUT_WIDECHAR_SUPPORT_
	sql_command_ansi=array_Create(sizeof(char));
	string_Widetochar(&sql_command_ansi,sql_command);
	#else
	sql_command_ansi=sql_command;
	#endif
	
	if(mysql_query(link->sql_link,sql_command_ansi)!=0)return NULL;
	result_data->sql_data=mysql_store_result(link->sql_link);

	#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(sql_command_ansi);
	#endif

	return result_data;
};

BOOL_ dbo_mysql_Exec(DBO_MYSQL_LINK *tag_db,CHAR_ *sql_command)
{
	char *sql_command_ansi;

	#ifndef WITHOUT_WIDECHAR_SUPPORT_
	sql_command_ansi=array_Create(sizeof(char));
	string_Widetochar(&sql_command_ansi,sql_command);
	#else
	sql_command_ansi=sql_command;
	#endif

	if(mysql_query(tag_db->sql_link,sql_command_ansi)!=0)return FALSE_;

	#ifndef WITHOUT_WIDECHAR_SUPPORT_
	array_Drop(sql_command_ansi);
	#endif

	return TRUE_;
};

DBO_MYSQL_ROW* dbo_mysql_Fetch(DBO_MYSQL_ROW *result_row,DBO_MYSQL_RESULT *tag_data)
{
	result_row->sql_row=mysql_fetch_row(tag_data->sql_data);
	return result_row;
};

DBO_MYSQL_ROW* dbo_mysql_Fetch_Ex(DBO_MYSQL_ROW *result_row,UINT_ row,DBO_MYSQL_LINK *link)
{
	/*not yet*/
};

C_STRING dbo_mysql_Get(C_STRING *tag_string,UINT_ field,DBO_MYSQL_ROW *tag_row)
{
	#ifndef WITHOUT_WIDECHAR_SUPPORT_
	string_Ansitowide(tag_string,tag_row->sql_row[field]);
	#else
	string_Set(tag_string,tag_row->sql_row[field]);
	#endif
	return *tag_string;
};

void dbo_Free_mysql_result(DBO_MYSQL_RESULT *tag_data)
{
	mysql_free_result(tag_data->sql_data);
};

void dbo_mysql_Drop(DBO_MYSQL_LINK *tag_db)
{

};