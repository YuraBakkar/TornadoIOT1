#include <stdio.h>
#include <wiringPi.h>
#include <time.h>

time_t rawtime;
struct tm * timeinfo;
int openDoor, openDoor1, openDoor2, openDoor3;

void myInterrupt (void) {
  openDoor = !openDoor;
  if ( openDoor  )  
    printf("Door 1 is opened...");
  else
    printf("Door 1 is closed...");
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf ("time=%s\n", asctime(timeinfo));
}

void myInterrupt1 (void) {
  openDoor1 = !openDoor1;
  if ( openDoor1  )  
    printf("Door 2 is opened...");
  else
    printf("Door 2 is closed...");
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf ("time=%s\n", asctime(timeinfo));

}

void myInterrupt2 (void) {
  openDoor2 = !openDoor2;
  if ( openDoor2  )  
    printf("Door 3 is opened...");
  else
    printf("Door 3 is closed...");
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf ("time=%s\n", asctime(timeinfo));

}
void myInterrupt3 (void) {
  openDoor3 = !openDoor3;
  if ( openDoor3  )  
    printf("Door 4 is opened...");
  else
    printf("Door 4 is closed...");
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf ("time=%s\n", asctime(timeinfo));

}

int main (void)
{
  //openDoor = false;
  wiringPiSetup() ;
  //pullUpDnControl(1,0);
  wiringPiISR (0, INT_EDGE_BOTH, &myInterrupt);
  wiringPiISR (1, INT_EDGE_BOTH, &myInterrupt1);
  wiringPiISR (3, INT_EDGE_BOTH, &myInterrupt2);
  wiringPiISR (4, INT_EDGE_BOTH, &myInterrupt3);
  //pinMode(0,INPUT);
  openDoor = digitalRead(0);
  openDoor1 = digitalRead(1);
  openDoor2 = digitalRead(2);
  openDoor3 = digitalRead(3);
  printf("1-%d; 2-%d; 3-%d; 4-%d\n\n",openDoor,openDoor1,openDoor2,openDoor3);
  pullUpDnControl(0,PUD_UP);
  pullUpDnControl(1,PUD_UP);
  pullUpDnControl(2,PUD_UP);
  pullUpDnControl(3,PUD_UP);
  while(1)
  {
    //a = digitalRead(0);
    //printf("a=%d\n",a);
    delay(100);
  }
  return 0 ;
}
