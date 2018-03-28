//#include <my_global.h>
#include <stdio.h>
#include <mysql.h>
#include <wiringPi.h>
#include <time.h>

#define SIZE 256

time_t rawtime, time1, time2;
struct tm * timeinfo;
struct tm tm1;
struct tm tm2;
int openDoor[4], callCount;
char buffer[SIZE];
char phone1[SIZE];
char phone2[SIZE];
char timeString1[9];
char timeString2[9];

int checkTime(struct tm *t){
  //time ( &rawtime );
  //timeinfo = localtime ( &rawtime );
  //printf("%d:%d:%d\n",t->tm_hour,t->tm_min,t->tm_sec);
  if ((t->tm_hour<tm1.tm_hour) || 
      ((t->tm_hour==tm1.tm_hour) && (t->tm_min<tm1.tm_min)) || 
      ((t->tm_hour==tm1.tm_hour) && (t->tm_min==tm1.tm_min) && (t->tm_sec<=tm1.tm_sec)) ||
      (t->tm_hour>tm2.tm_hour) || 
      ((t->tm_hour==tm2.tm_hour) && (t->tm_min>tm2.tm_min)) || 
      ((t->tm_hour==tm2.tm_hour) && (t->tm_min==tm2.tm_min) && (t->tm_sec>=tm2.tm_sec))
      )
    return 1;
  else
    return 0;
}

void checkDoors(int d){
  openDoor[d-1] = !openDoor[d-1];
  if ( openDoor[d-1]  )  
    fprintf(stdout,"Door %d is opened...",d);
  else
    fprintf(stdout,"Door %d is closed...",d);
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  //printf("%d:%d:%d\n",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
  if (checkTime(timeinfo)){
    fprintf (stdout,"time=%s\n", asctime(timeinfo));
  }
  fflush(stdout);
}

void myInterrupt1 (void) {
  checkDoors(1);
}

void myInterrupt2 (void) {
  checkDoors(2);
}

void myInterrupt3 (void) {
  checkDoors(3);
}
void myInterrupt4 (void) {
  checkDoors(4);
}

void init_controller(){
  //openDoor = false;
  wiringPiSetup() ;
  //pullUpDnControl(1,0);
  wiringPiISR (0, INT_EDGE_BOTH, &myInterrupt1);
  wiringPiISR (1, INT_EDGE_BOTH, &myInterrupt2);
  wiringPiISR (3, INT_EDGE_BOTH, &myInterrupt3);
  wiringPiISR (4, INT_EDGE_BOTH, &myInterrupt4);
  //pinMode(0,INPUT);
  openDoor[0] = digitalRead(0);
  openDoor[1] = digitalRead(1);
  openDoor[2] = digitalRead(2);
  openDoor[3] = digitalRead(3);
  fprintf(stdout,"1-%d; 2-%d; 3-%d; 4-%d\n\n",openDoor[0],openDoor[1],openDoor[2],openDoor[3]);
  pullUpDnControl(0,PUD_UP);
  pullUpDnControl(1,PUD_UP);
  pullUpDnControl(2,PUD_UP);
  pullUpDnControl(3,PUD_UP);
}

void finish_with_error(MYSQL *con){
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

void init_db(MYSQL *con){
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
  
  if (mysql_query(con, "SELECT table_name FROM information_schema.tables WHERE table_schema = 'security' AND table_name = 'log';")){
    finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  
  if (result == NULL){
      finish_with_error(con);
  }
  
  int nr = mysql_num_rows(result);

  if (mysql_query(con, "use security;")){
    finish_with_error(con);
  }
  
  if (nr == 0){
    if (mysql_query(con, "CREATE TABLE log(Id INT, Door INT, Dat DATETIME);"))
      finish_with_error(con);
    if (mysql_query(con, "CREATE TABLE params(Phone1 TEXT, Phone2 TEXT, Count INT, Time1 TIME, Time2 TIME);"))
      finish_with_error(con);
    if (mysql_query(con, "INSERT INTO params VALUES ('+380667906811','',1,'08:00:00','20:00:00');"))
      finish_with_error(con);
  }
  if (mysql_query(con, "SELECT * FROM params;"))
    finish_with_error(con);
  
  mysql_free_result(result);
  result = mysql_store_result(con);
  
  if (result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  
  while ((row = mysql_fetch_row(result))) 
  { 
    strcpy(phone1, row[0]);
    strcpy(phone2, row[1]);
    callCount = atoi(row[2]);
    strptime(row[3], "%H:%M:%S", &tm1);
    //time1 = mktime(&tm1);
    strptime(row[4], "%H:%M:%S", &tm2);
    //time2 = mktime(&tm2);
    
    printf("ph1=%s, ph2=%s, count=%d, t1=%d:%d:%d, t2=%d:%d:%d\n",phone1,phone2,callCount,tm1.tm_hour,tm1.tm_min,tm1.tm_sec,tm2.tm_hour,tm2.tm_min,tm2.tm_sec);
  }
  
  mysql_free_result(result);
}

void close_db(MYSQL *con){
  mysql_close(con);
}

int main(int argc, char **argv){
  MYSQL *con = mysql_init(NULL);
  init_db(con);
  init_controller();
  
  while(1){
    //a = digitalRead(0);
    //printf("a=%d\n",a);
    delay(100);
  }
  
  close_db(con);
  return 0;
}
