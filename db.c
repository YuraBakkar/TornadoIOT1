//#include <my_global.h>
#include <stdio.h>
#include <mysql.h>

int main(int argc, char **argv){
  MYSQL *con = mysql_init(NULL);
  if (con == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }
  if (mysql_real_connect(con, "localhost", "root", "qwopaskl", NULL, 0, NULL, 0) == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
  }
  if (mysql_query(con, "CREATE DATABASE IF NOT EXISTS security")){
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }
  mysql_close(con);
  exit(0);
}