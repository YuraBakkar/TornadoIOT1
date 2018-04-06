//#include <my_global.h>
#include <stdio.h>
#include <mysql.h>
#include <wiringPi.h>
#include <time.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>

#define SIZE 256

time_t rawtime, time1, time2;
struct tm * timeinfo;
struct tm tm1;
struct tm tm2;
int openDoor[4], callCount;
char buffer[SIZE];
char phone1[SIZE];
char phone2[SIZE];
char doorName1[SIZE];
char doorName2[SIZE];
char doorName3[SIZE];
char doorName4[SIZE];
char timeString1[9];
char timeString2[9];

int fd;  // COM File descriptor

int lastAlarmTime = -1;
int replyDelay = 30;//in minutes
int callDelay = 30;//in seconds

int alarmDoor = 0;

MYSQL *con;

char smsMessage[]={"alarm - "};

void saveAlarmDB(int d, struct tm *t){
  char b[256];
  sprintf(b, "INSERT INTO log VALUES(null,%d,'%d-%02d-%02d %02d:%02d:%02d')",d,2000+t->tm_year,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
  //printf(stdout,"%s\n",b);
  if (mysql_query(con, b)) {
    finish_with_error(con);
  }
}

int checkReply(struct tm *t){
  int t1 = t->tm_hour*60+t->tm_min;
  if ((lastAlarmTime==-1)||(lastAlarmTime+replyDelay<t1)){
    lastAlarmTime = t1;
    return 1;
  }
  else
    return 0;
}

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

void initCOM(){
  fd = open("/dev/ttyS3", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
  {
    perror("open_port: Unable to open /dev/ttyUSB0 - ");
  }
  else
    fcntl(fd, F_SETFL, FNDELAY);

  printf ( "In Open port fd = %i\n", fd); 

  // Read the configureation of the port

  struct termios options;
  tcgetattr( fd, &options );

  /* SEt Baud Rate */

  cfsetispeed( &options, B115200 );
  cfsetospeed( &options, B115200 );

  //I don't know what this is exactly

  options.c_cflag |= ( CLOCAL | CREAD );

  // Set the Charactor size

  options.c_cflag &= ~CSIZE; /* Mask the character size bits */
  options.c_cflag |= CS8;    /* Select 8 data bits */

  // Set parity - No Parity (8N1)

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  // Disable Software Flow control

  options.c_iflag &= ~(IXON | IXOFF | IXANY);

  // Chose raw (not processed) output

  options.c_oflag &= ~OPOST;

  if ( tcsetattr( fd, TCSANOW, &options ) == -1 )
    printf ("1Error with tcsetattr = %s\n", strerror ( errno ) );
  else
    printf ( "%s\n", "tcsetattr succeed" );

  fcntl(fd, F_SETFL, FNDELAY);

  char smsCommand0[]={"ATE1\r"};
  usleep(100000);
  int n = write(fd, smsCommand0, strlen(smsCommand0));

}

void sendSMS(int d, int p){
  char smsCommand0[]={"AT+CSCS=\"GSM\"\r"};
  char smsCommand1[]={"AT+CMGF=1\r"};
  char smsCommand2[]={"AT+CMGS=\""};
  char smsCommand2End[]={"\"\r"};
  char b[SIZE];
  usleep(100000);
  int n = write(fd, smsCommand0, strlen(smsCommand0));
    
  if (n < 0)
    fputs("sms failed!\n", stderr);
  else {
    usleep(100000);
    write(fd, smsCommand1, strlen(smsCommand1));
    if (n < 0)
      fputs("sms failed!\n", stderr);
    else {
      strcpy(b,smsCommand2);
      if (p==1)
        strcat(b,phone1);
      else
        strcat(b,phone2);
      strcat(b,smsCommand2End);
      usleep(100000);
      n = write(fd, b, strlen(b));
      if (n < 0)
        fputs("sms failed!\n", stderr);
      else {
        strcpy(b,smsMessage);
        switch(d){
          case 1: strcat(b,doorName1); break;
          case 2: strcat(b,doorName2); break;
          case 3: strcat(b,doorName3); break;
          case 4: strcat(b,doorName4); break;
        }
        strcat(b,"\x1A");
        usleep(100000);
        n = write(fd, b, strlen(b));
        if (n < 0)
          fputs("sms failed!\n", stderr);
        else
          fprintf(stdout,"SMS send succeed\n");
      }
    }
  }
}

void callPhone(int p){
  char callBegin[]={"atd"};
  char callEnd[]={";\r"};
  char b[SIZE];
  strcpy(b,callBegin);  
  if (p==1){
    strcat(b,phone1);
  }
  else {
    strcat(b, phone2);
  }
  strcat(b,callEnd);
  int n = write(fd, b, strlen(b));
  if (n < 0)
    fputs("call failed!\n", stderr);
  else
    fprintf(stdout,"Call succeed n  = %i\n", n );
  //sleep(5);
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
    saveAlarmDB(d, timeinfo);
    if(checkReply(timeinfo))
      alarmDoor = d;
    /*if (checkReply(timeinfo)){
      {
        if (strlen(phone1)){
          //sendSMS(d,1);
          sleep(2);
          callPhone(1);//sendSMS(d,1);
          usleep(callDelay*1000000);
        }
        if (strlen(phone2)){
          sendSMS(d,2);
          sleep(2);
          callPhone(2);//sendSMS(d,1);
          usleep(callDelay*1000000);
        }
      }
    }*/
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
  wiringPiISR (2, INT_EDGE_BOTH, &myInterrupt3);
  wiringPiISR (3, INT_EDGE_BOTH, &myInterrupt4);
  //pinMode(0,INPUT);
  openDoor[0] = digitalRead(0);
  openDoor[1] = digitalRead(1);
  openDoor[2] = digitalRead(2);
  openDoor[3] = digitalRead(3);
  fprintf(stdout,"1-%d; 2-%d; 3-%d; 4-%d\n\n",openDoor[0],openDoor[1],openDoor[2],openDoor[3]);
  //pullUpDnControl(0,PUD_UP);
  //pullUpDnControl(1,PUD_UP);
  //pullUpDnControl(2,PUD_UP);
  //pullUpDnControl(3,PUD_UP);
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
    if (mysql_query(con, "CREATE TABLE log(id INT NOT NULL AUTO_INCREMENT, door INT, dat DATETIME, PRIMARY KEY (ID));"))
      finish_with_error(con);
    if (mysql_query(con, "CREATE TABLE params(Phone1 TEXT, Phone2 TEXT, Count INT, Time1 TIME, Time2 TIME, Door1 TEXT, Door2 TEXT, Door3 TEXT, Door4 TEXT);"))
      finish_with_error(con);
    if (mysql_query(con, "INSERT INTO params VALUES ('+380667906811','',1,'08:00:00','20:00:00','door1','door2','door3','door4');"))
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
    strcpy(doorName1, row[5]);
    strcpy(doorName2, row[6]);
    strcpy(doorName3, row[7]);
    strcpy(doorName4, row[8]);    
    fprintf(stdout,"ph1=%s, ph2=%s, count=%d, t1=%d:%d:%d, t2=%d:%d:%d\nd1=%s, d2=%s, d3=%s, d4=%s\n",phone1,phone2,callCount,tm1.tm_hour,tm1.tm_min,tm1.tm_sec,tm2.tm_hour,tm2.tm_min,tm2.tm_sec,doorName1,doorName2,doorName3,doorName4);
  }
  
  fflush(stdout);
  
  mysql_free_result(result);
}

void close_db(MYSQL *con){
  mysql_close(con);
}

int main(int argc, char **argv){
  /*MYSQL **/con = mysql_init(NULL);
  int n;
  char buf[1000]={"\0"};
  
  init_db(con);
  init_controller();
  initCOM();
  
  /*if (!openDoor[0])
    checkDoors(1);
  if (!openDoor[1])
    checkDoors(2);
  if (!openDoor[2])
    checkDoors(3);
  if (!openDoor[3])
    checkDoors(4);*/
  
  while(1){
    n = read( fd, buf, sizeof(buf) );

    if(n>0)
    {   
      printf("%s", buf);    
      fflush(stdout);
    }
    if (alarmDoor){
      /*if ( openDoor[d-1] )*/{
        if (strlen(phone1)){
          //sendSMS(d,1);
          sleep(2);
          callPhone(1);//sendSMS(d,1);
          usleep(callDelay*1000000);
        }
        if (strlen(phone2)){
          sendSMS(alarmDoor,2);
          sleep(2);
          callPhone(2);//sendSMS(d,1);
          usleep(callDelay*1000000);
        }
        alarmDoor = 0;
      }
    }
    //delay(100);
  }
  
  close_db(con);
  close( fd );
  return 0;
}
