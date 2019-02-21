
#include "Camera_Exp.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

CAMERA cam;
char ssid[] = "WAYUPONG_WIFI";      //  your network SSID (name)
char pass[] = "59172310183";   // your network password

// www.howsmyssl.com root certificate authority, to verify the server
// change it to your server root CA
// SHA1 fingerprint is broken now!

const char* test_root_ca= \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
    "-----END CERTIFICATE-----\n" ;

// You can use x.509 client certificates if you want
//const char* test_client_key = "";   //to verify the client
//const char* test_client_cert = "";  //to verify the client

WiFiServer server(80);

String http_header = "HTTP/1.1 200 OK\r\n";
String http_stream = "Content-type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n\r\n";
String http_jpg = "Content-type: image/jpg\r\n\r\n";
String http_boundary = "--123456789000000000000987654321\r\n";
WiFiClient client;

#define button D1     // switch input Active Low

void setup()
{
  Serial.begin(115200);

  pinMode(button,INPUT_PULLUP);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  cam.setFrameSize(CAMERA_FS_VGA);
  esp_err_t err = cam.init();
  if (err != ESP_OK)
  {
    Serial.println("Camera init failed with error =" + String( err));
    return;
  }

  Serial.println("Open Web Browser");
  Serial.print("Get for single frame --> http://");//http://192.168.0.102/capture
  Serial.print(WiFi.localIP());
  Serial.println("/capture");
  
  Serial.print("Stream  --> http://");//http://192.168.0.102/capture
  Serial.print(WiFi.localIP());
  Serial.println("/stream");


}

void loop()
{

  if(!digitalRead(button)) {
    Serial.println("Button press..");
    readQrCode();    
  }
  
  String httpreq;
  client = server.available();
  if (client)
  {
    Serial.println("New Client.");
    String httpreq = "";
    while (client.connected())
    {
      if (client.available())
      {
        String httpreq_line = client.readStringUntil('\n');
        httpreq += httpreq_line;
        if (httpreq_line == "\r")
        {
          if (httpreq.indexOf("GET /stream") != -1)
          {
                Serial.println("Stream");
                stream();
          }
          if (httpreq.indexOf("GET /capture") != -1)
          {
                Serial.println("Capture");
                capture();
          }
          
          httpreq = "";
          client.stop();
        }

      }
    }

  }
}

void stream()
{
  client.print(http_header);
  client.print(http_stream);  
  while (client.connected())
  {
    esp_err_t err;
    err = cam.capture();
    if (err != ESP_OK)
    {
      Serial.println("Camera capture failed with error =" + String(err));
      return;
    }
    client.print(http_jpg);
    client.write(cam.getfb(),cam.getSize());
    client.print(http_boundary);
  }
}
void capture()
{
   esp_err_t err;
    err = cam.capture();
    if (err != ESP_OK)
    {
      Serial.println("Camera capture failed with error =" + String(err));
      return;
    }
   client.print(http_header);
   client.print(http_jpg);
   client.write(cam.getfb(),cam.getSize());
   
}

void readQrCode() {
  esp_err_t err;
  err = cam.capture();
  if (err != ESP_OK) {
    Serial.println("Camera capture failed with error =" + String(err));
    return;
  }
    
  client.setCACert(test_root_ca);
  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    String url = "/w/decode";
    Serial.print("requesting URL: ");
    Serial.println(url);
  
    String start_request = "";
    start_request = start_request +
    "------WebKitFormBoundaryieIsmGgKag7ktdTw\r\n" +
     "Content-Disposition: form-data; name=\"f\"; filename=\"line128x128.jpg\"\r\n" +
     "Content-Type: image/jpeg\r\n\r\n";
     
    String end_request = "";
    end_request = end_request + "\r\n------WebKitFormBoundaryieIsmGgKag7ktdTw--\r\n";
    
    uint16_t full_length;
    full_length = start_request.length() + cam.getSize() + end_request.length(); //f.size() // file size
    Serial.println("full_length -> " + String(full_length));
  
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: " + String(host) );
    client.println("Connection: keep-alive");
    client.println("Content-Length: " + String(full_length, DEC));
    client.println("Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryieIsmGgKag7ktdTw");
    client.println("User-Agent: BuildFailureDetectorESP8266");
    client.println();
    
    client.print(start_request);
    Serial.println(headerUrl + start_request);
  
    client.write(cam.getfb(), cam.getSize());

    client.println(end_request);
    Serial.println(end_request);
         
    Serial.println("request sent");
     
    String content = "";
  
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") 
      {
        Serial.println("headers received");
        break;
      }
    }

    Serial.println("Content==========");
    Serial.println();
    
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
      content += c;
    }
      
    Serial.println(content);
    Serial.println();
    Serial.println("End content==========");
    Serial.println("closing connection");

    if(content.length() == 0) {
      Serial.println("Data Error");
      return;
    }

    String strSearch = "Parsed Result</td><td><pre>";
    int first = content.indexOf(strSearch);
    int second = content.indexOf("</pre>", first + 1);
    Serial.println("The index is " + String(first) + " " + String(second) );

    if(first < 0) {
      Serial.println("QRCode not foud");
      return;
    }
  
    String result = content.substring(first + strSearch.length(), second);
    Serial.println("result: " + result);
  
  }
}
