//#include <my_global.h>
#include <stdio.h>
#include <mysql.h>

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

int main(int argc, char **argv){
  MYSQL *con = mysql_init(NULL);
  if (con == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }
  if (mysql_real_connect(con, "localhost", "root", "qwopaskl", NULL, 0, NULL, 0) == NULL){
    finish_with_error(con);
  }
  if (mysql_query(con, "CREATE DATABASE IF NOT EXISTS security")){
    finish_with_error(con);
  }
  
  if (mysql_query(con, "SELECT table_name FROM information_schema.tables WHERE table_schema = 'security' AND table_name = 'data';")) 
  {
      finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  
  if (result == NULL) 
  {
      finish_with_error(con);
  }
  
  printf ("Number of rows: %lu\n",
        (unsigned long) mysql_num_rows(result));
  
  mysql_close(con);
  exit(0);
}