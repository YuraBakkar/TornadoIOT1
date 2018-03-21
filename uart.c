#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
  int fd;  // File descriptor
  int n,i;
  char buf[1000]={"\0"}; 
  char com[]={"at+cfun=1\r"};
  char com1[]={"atd+380667906811;\r"};       
  fd = open_port();

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


  // Write some stuff !!!

  n = write(fd, com, strlen(com));
  if (n < 0)
    fputs("write() of 4 bytes failed!\n", stderr);
  else
    printf ("Write succeed n  = %i\n", n );
  sleep(1);
  n = write(fd, com1, strlen(com1));
  if (n < 0)
    fputs("write() of 4 bytes failed!\n", stderr);
  else
    printf ("Write succeed n  = %i\n", n );

  n=0;
  i=0;
  while (1) 
  {
    n = read( fd, buf, sizeof(buf) );

    if(n>0)
    {   
      printf("%s", buf);    
      fflush(stdout);
    }
    //   i=i+1;
  }

  close( fd );
  return 0;
}           

int open_port(void)
{
  int fd; /* File descriptor for the port */

  fd = open("/dev/ttyS3", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
  {
    perror("open_port: Unable to open /dev/ttyUSB0 - ");
  }
  else
    fcntl(fd, F_SETFL, FNDELAY);

  printf ( "In Open port fd = %i\n", fd); 
  return (fd);
}