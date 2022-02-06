#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#define db_name "money_management.db"
#define user_db  "users"
#define transaction_db  "trans"
#define net_dues_db "net_dues"
#define strcpyALL(buf, offset, ...)  do{ \
    char *bp=(char*)(buf); /*so we can add to the end of a string*/ \
    const char *s, \
    *a[] = { __VA_ARGS__,NULL}, \
    **ss=a; \
    while((s=*ss++)) \
         while((*s)&&(++offset<(int)sizeof(buf))) \
            *bp++=*s++; \
    if (offset!=sizeof(buf))*bp=0; \
}while(0)

void swap(char **str1_ptr, char **str2_ptr) 
{ 
  char *temp = *str1_ptr; 
  *str1_ptr = *str2_ptr; 
  *str2_ptr = temp; 
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   // for(int i = 0; i<argc; i++) {
    if (strcmp(argv[2],"0")==0)  return 0;
   // }  
   for(int i = 0; i<argc; i++) {
      printf("%s = %s ", azColName[i], argv[i] ? argv[i] : "NULL");   
   } printf("\n");
   return 0;
}

void add_user(char *name) {
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB);
   char sql[256], *zErrMsg = 0; int len=0;
   strcpyALL(sql,len, "INSERT or IGNORE INTO users (NAME) VALUES(\'",name,"\');");
   int rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %d%s\n",rc ,zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "New user added! \n");
   }
   sqlite3_close(DB);
}

void add_tran_history(char *name1,char *name2, int amt,char* tableName) {
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB);
   char sql[256], *zErrMsg = 0;
   int len=0;
   char* amt_str; asprintf (&amt_str, "%d", amt);
   strcpyALL(sql,len, "INSERT or IGNORE INTO ",tableName," (Person1,Person2,Amt) VALUES (\"",name1,"\", \"",name2,"\" ,\"",amt_str,"\");");
   int rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %d%s\n",rc ,zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "New entry in %s! \n",tableName);
   }
   sqlite3_close(DB);
}

int user_exists(char *name) {
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB);
   char sql[256]; int len=0;
   strcpyALL(sql,len,"SELECT * FROM ",user_db," WHERE NAME=\'",name,"\';");
   struct sqlite3_stmt *selectstmt; 
   int result = sqlite3_prepare_v2(DB,sql, -1, &selectstmt, NULL);
   if (result==SQLITE_OK && sqlite3_step(selectstmt) == SQLITE_ROW) {
      sqlite3_finalize(selectstmt);
      sqlite3_close(DB);
      return 0;
   }
   sqlite3_finalize(selectstmt);
   sqlite3_close(DB);
   return 1;
}

int transation_exists(char *name1,char *name2){
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB);
   char sql[256]; int len=0;
   strcpyALL(sql,len,"SELECT * FROM net_dues WHERE (Person1=\'",name1,"\' AND Person2=\'",name2,"\') OR (Person1=\'",name2,"\' AND Person2=\'",name1,"\');");
   struct sqlite3_stmt *selectstmt; 
   int result = sqlite3_prepare_v2(DB,sql, -1, &selectstmt, NULL);
  
   if (result==SQLITE_OK && sqlite3_step(selectstmt) == SQLITE_ROW) {
      sqlite3_finalize(selectstmt);
      sqlite3_close(DB);
      return 0;
   }
   sqlite3_finalize(selectstmt);
   sqlite3_close(DB);
   return 1;  
}
void update_value(char* name1, char* name2, int amt) {
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB),len=0;
   char sql[256], *amt_str,*zErrMsg = 0; asprintf (&amt_str, "%d", amt);  
   strcpyALL(sql,len,"UPDATE net_dues SET Amt=Amt+",amt_str," WHERE (Person1=\'",name1,"\' AND Person2=\'",name2,"\');");
   int rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
      if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %d%s\n",rc ,zErrMsg);
      sqlite3_free(zErrMsg);
   }
   strcpyALL(sql,len,"UPDATE net_dues SET Amt=Amt-",amt_str," WHERE (Person1=\'",name2,"\' AND Person2=\'",name1,"\');");
   rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);   
      if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %d%s\n",rc ,zErrMsg);
      sqlite3_free(zErrMsg);
   }
   sqlite3_close(DB);
}

void add_entry(char *name1,char *name2, int amt) {
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB);
   char sql[256], *zErrMsg = 0;  int len=0;
   struct sqlite3_stmt *selectstmt; 
   if (amt<0) {
      amt*=-1;   swap(&name1,&name2);
   }
   add_tran_history(name1,name2,amt,transaction_db);
   if (user_exists(name1)==1) add_user(name1);
   if (user_exists(name2)==1) add_user(name2);


   if (transation_exists(name1,name2)==1) {
      add_tran_history(name1,name2,amt,net_dues_db);
   } else {
      update_value(name1,name2,amt);
   }
   sqlite3_close(DB);
}

void clean_table(char *tableName){
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB),len=0;
   char *zErrMsg = 0, sql[256];
   strcpyALL(sql,len,"DELETE FROM ",tableName,";");
   int rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %d%s\n",rc ,zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Table cleaned! \n");
   }   
   sqlite3_close(DB);   
}

void create_db(){
    char *zErrMsg = 0;
    sqlite3* DB;
    int rc;
    int exit = sqlite3_open(db_name, &DB);
    if (exit) {
        printf("Error open DB %s \n", sqlite3_errmsg(DB));
    }
    char* tran_table= "CREATE TABLE trans(" \
         "Person1   TEXT    NOT NULL, " \
         "Person2   TEXT    NOT NULL, " \
         "Amt INT NOT NULL);";
    rc = sqlite3_exec(DB, tran_table, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
      sqlite3_free(zErrMsg);
    }

    char* user_table= "CREATE TABLE users( NAME   TEXT    NOT NULL);";
    rc = sqlite3_exec(DB, user_table, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
      sqlite3_free(zErrMsg);
    }    

    char* net_table=  "CREATE TABLE net_dues(" \
         "Person1   TEXT    NOT NULL, " \
         "Person2   TEXT    NOT NULL, " \
         "Amt INT NOT NULL);";
    rc = sqlite3_exec(DB, net_table, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
      sqlite3_free(zErrMsg);
    }  
    sqlite3_close(DB);
}

char* query_table(char *tableName, char *user) {
   sqlite3* DB;  int exit = sqlite3_open(db_name, &DB),len=0;
   char *zErrMsg = 0, sql[256];
   strcpyALL(sql,len,"SELECT * FROM ",tableName," WHERE Person1=\'",user,"\' OR Person2=\'",user,"\';");
   struct sqlite3_stmt *statement; 
    if ( sqlite3_prepare_v2(DB, sql, -1, &statement, NULL)!= SQLITE_OK)
    {
      sqlite3_finalize(statement);
      sqlite3_close(DB);
      printf("Can't retrieve data: %s\n", sqlite3_errmsg(DB));
    }
   sqlite3_str *buffer = sqlite3_str_new(DB);
   while (sqlite3_step(statement) == SQLITE_ROW)
    {
      if (strcmp(sqlite3_column_text(statement, 0),user)==0) {
         sqlite3_str_appendf(buffer,"%s,%s\n",sqlite3_column_text(statement, 1),sqlite3_column_text(statement, 2));
      }  else {
         sqlite3_str_appendf(buffer,"%s,-%s\n",sqlite3_column_text(statement, 0),sqlite3_column_text(statement, 2));
      }
    }
   char *result = sqlite3_str_finish(buffer);
   // printf("%s ",result);
   sqlite3_finalize(statement);
   sqlite3_close(DB);
   return result;
}

// int main(){
//    create_db();    
//    add_user("aditya");
//    //  add_tran_history("ad","Ab",24,net_dues_db);
//    //  int a=query_user("aditya");
//    //  printf("%d",a);
//    add_entry("aditya","gupta",100);
//     // add_tran_history("ab","Av",31);
//    return 0;
// }