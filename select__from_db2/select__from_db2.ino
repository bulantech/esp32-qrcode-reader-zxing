
#include <WiFi.h>           // Use this for WiFi instead of Ethernet.h
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
IPAddress server_addr(27,254,172,48);  // IP of the MySQL *server* here
char user[] = "qrcode"; // MySQL user login username
char password[] = "@qrcode#"; // MySQL user login password
// WiFi card example
char ssid[] = "WAYUPONG_WIFI"; // your SSID
char pass[] = "59172310183";    // your SSID Password
String stringOne, stringTwo;

WiFiClient client;                 // Use this for WiFi instead of EthernetClient
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

const char QUERY_POP[] = "SELECT * FROM eermutio_qrcode.outqrcode ORDER BY Password;";
char query[128];



void setup() {
  Serial.begin(115200);
  stringOne = String("AB0007");
  while (!Serial); // wait for serial port to connect
    Serial.println("Connecting...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // print out info about the connection:
  Serial.println("\nConnected to network");
  Serial.print("My IP address is: ");
  Serial.println(WiFi.localIP());
  
  Serial.print("Connecting to SQL...  ");
  if (conn.connect(server_addr, 3306, user, password))
    Serial.println("OK.");
  else
    Serial.println("FAILED.");
  
}


void loop() 
{
  delay(1000);

  Serial.println("> Running SELECT with dynamically supplied parameter");

  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  sprintf(query, QUERY_POP, 9000000);

  cur_mem->execute(query);


  column_names *cols = cur_mem->get_columns();
  for (int f = 0; f < cols->num_fields; f++) {
    //Serial.print(cols->fields[f]->name);
    if (f < cols->num_fields-1) {
    //  Serial.print("         :        ");
    }
  }
  Serial.println();
  // Read the rows and print them
  row_values *row = NULL;
  do {
    row = cur_mem->get_next_row();
    if (row != NULL) {
      for (int f = 0; f < cols->num_fields; f++) {
        Serial.print(row->values[0]);
        stringTwo = String(row->values[0]);
      if (stringOne == stringTwo) {
           Serial.print("----------ok----------");
       }
       else
       {
         Serial.print("Fuckkkk");
       }
      }
            Serial.println();
    }
    
  } while (row != NULL);
  // Deleting the cursor also frees up memory used
  delete cur_mem;
  delay(5000);
  
}
