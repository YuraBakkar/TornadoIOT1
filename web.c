#include <iostream>
#include "fcgio.h"
#include <mysql.h>
#include <stdlib.h>

#define SIZE 256

using namespace std;

MYSQL *con;

char doorName1[SIZE];
char doorName2[SIZE];
char doorName3[SIZE];
char doorName4[SIZE];

void finish_with_error(MYSQL *con){
  cout << mysql_error(con);
  mysql_close(con);
  exit(1);        
}

int main(void) {
    // Backup the stdio streambufs
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();
	
	//////////////////////////////////////////////////////////////////////////
	con = mysql_init(NULL);
	if (con == NULL){
		cout << mysql_error(con);
		exit(1);
	}
	if (mysql_real_connect(con, "localhost", "root", "qwopaskl", NULL, 0, NULL, 0) == NULL){
		finish_with_error(con);
	}
	if (mysql_query(con, "use security;")){
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
    strcpy(doorName1, row[5]);
    strcpy(doorName2, row[6]);
    strcpy(doorName3, row[7]);
    strcpy(doorName4, row[8]);    
  }

	//////////////////////////////////////////////////////////////////////////
	

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0) {
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);

        cout << "Content-type: text/html\r\n"
             << "\r\n"
             << "<html>\n"
             << "  <head>\n"
             << "    <title>Hello, World!</title>\n"
             << "  </head>\n"
             << "  <body>\n"
             << "    <h1>Hello!</h1>\n<table>";
		if (mysql_query(con, "SELECT * FROM log WHERE dat+INTERVAL 7 DAY>NOW();"))
		finish_with_error(con);
  
		MYSQL_RES *result = mysql_store_result(con);
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(result))) 
		{ 
			cout << "<tr>";
			cout << "<td>";
			int doors = stoi(row[1]);
			switch (doors) {
				case 1:
					cout << doorName1;
					break;
				case 2:
					cout << doorName2;
					break;
				case 3:
					cout << doorName3;
					break;
				case 4:
					cout << doorName4;
					break;
			}
			cout << "</td><td>" << row[2] << "</td>"; 
			cout << "</tr>\n";
		}
		cout << "</table></body>\n"
             << "</html>\n";

        // Note: the fcgi_streambuf destructor will auto flush
    }

	mysql_close(con);
	
    // restore stdio streambufs
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);

    return 0;
}