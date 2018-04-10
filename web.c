#include <iostream>
#include "fcgio.h"
#include <mysql.h>

using namespace std;

MYSQL *con;

void finish_with_error(MYSQL *con){
  fprintf(stderr, "%s\n", mysql_error(con));
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
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
	}
	if (mysql_real_connect(con, "localhost", "root", "qwopaskl", NULL, 0, NULL, 0) == NULL){
		finish_with_error(con);
	}
	if (mysql_query(con, "use security;")){
		finish_with_error(con);
	}
	if (mysql_query(con, "SELECT * FROM log WHERE dat+INTERVAL 7 DAY>NOW();"))
    finish_with_error(con);
  
	mysql_free_result(result);
	MYSQL_RES *result = mysql_store_result(con);
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
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(result))) 
		{ 
			cout << "<tr>";
			cout << "<td>" << row[1] << "</td><td>" << row[2] << "</td>"; 
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