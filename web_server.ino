/*
  Temperature Sensor Web Server
*/

#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ethernet.h>
#include "relays.h"

Relays relays;

// Setup the LCD display
LiquidCrystal_I2C lcd(0x3F, 20, 4);

/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
short temp_counter=0;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x8E, 0x19, 0xA7, 0x54, 0xD5, 0x97
};
IPAddress ip(192, 168, 1, 177);
int server_port = 80;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(server_port);

// client incomming message variables 
#define CLIENT_BUF_SIZE 129
char client_buffer[CLIENT_BUF_SIZE+1]={0};

//
// Return an IP address as a string with optional port
//
void get_ip_as_string(char* buf, IPAddress ip, int port, bool add_port = true) {
  if(add_port)
    sprintf(buf, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], port);
  else
    sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

//
// Sends a standard 200 OK client header.
//
void send_client_header(EthernetClient& client, int refresh=0){
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  if(refresh){
    client.print("Refresh: ");  
    client.println(refresh);  // refresh the page automatically every n sec
  }
  client.println();  
}

//
// Sends temperature data as a HTML page
//
void send_client_data(EthernetClient& client, const byte temps[2]){
  send_client_header(client, 5);
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<h1>Temperature sensors</h1>");
  client.print("Temp1: ");
  client.print(temps[0]);
  client.println("<br />");
  client.print("Temp2: ");
  client.print(temps[1]);
  client.println("<br />");
  client.println("</html>");
}

//
// Obtains the tempratures form the sensors
//
void get_temperatures(byte temperatures[2]){
  if(temp_counter==0)
    sensors.requestTemperatures(); // Send the command to get temperature readings
  for(int index=0;index<2;index++){
    temperatures[index] = byte(sensors.getTempCByIndex(index));
  }
  temp_counter++;
  if(temp_counter==50)
    temp_counter=0;
}

//
// Formats the tesmprature values as a string
//
void get_temperature_string(char* buf, const byte temps[2]){
  sprintf(buf, "T1: %d   T2: %d     ", (int)temps[0], (int)temps[1]);
}

//
// Shows a dot tick on the LCD display in the top-right corner
//
byte tick=0;
void show_tick(){
  const char* tick_chars = " .";
  lcd.setCursor(19, 0);
  lcd.print(tick_chars[tick++]);
  if(tick==2) tick=0;
}

//
// Processes one line at a time from the client request
//
bool get_request_line(EthernetClient& client){
  client_buffer[0] = '\0';
    
  if(client.connected() && client.available()) {
    for(int i=0; i<CLIENT_BUF_SIZE;i++){
      char c = client.read();
      // If there are no more characters then quit
      if(c==-1) 
        return true;
        
      client_buffer[i] = c;
      if (c == '\n') {
        if(i<=1){
          // End of request detected
          client_buffer[i+1] = '\0';
          return true;
        }
        // No end of request detected
        client_buffer[i+1] = '\0';
        return false;
      }
    }
  }  
  // No end of request detected
  return false;
}

//
// Process client request data
//
void process_client(EthernetClient& client, const byte temps[2]) {
  while(!get_request_line(client)){
    Serial.print(client_buffer);
    if (!strcmp(client_buffer, "GET /temperatures/ HTTP/1.1\r\n")){    
      // flush the buffer
      while(client.read() != -1);
      send_client_data(client, temps);
      return;
    }
    if(!strcmp(client_buffer, "GET /api/v1/status/ HTTP/1.1\r\n")){
      Serial.println("Sending client API JSON response.");
      // flush the buffer
      while(client.read() != -1);
      send_client_header(client);
      char data[33]={0};
      
      client.print("{");
      for(int i=0;i<2;i++){
        sprintf(data, "\"temp1\":%i,");
        client.print(data);
      }
      for(int i=0;i<6;i++){
        sprintf(
          data, 
          "\"relay1\":{\"name\":\"%s\", \"state\":%i}",
          relays.name(i), relays.get_state(i)
        );
        client.print(data);
        if(i!=5)
          client.print(",");
      }
      client.print("}");
      return;
    }
    if(!strcmp(client_buffer, "GET /favicon.ico HTTP/1.1\r\n")){
      // flush the buffer
      while(client.read() != -1);
      client.println("HTTP/1.1 404 NOT FOUND");
      client.println("Content-Type: text/html");
      client.println("Connection: close");  // the connection will be closed after completion of the response
      return;       
    }
  }
}

//
// Closes the client connection
//
void close_client_connection(EthernetClient& client){
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
  Serial.println("Client disconnected");
}

//
// Main Setup function
//
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 
  // setup the lcd display
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Initialise the temperature sensors
  sensors.begin();

  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    const char* buf="Ethernet shield was not found.";
    Serial.println(buf);
    lcd.setCursor(0, 0);
    lcd.print(buf);
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    const char* buf="Ethernet cable is not connected.";
    Serial.println(buf);
    lcd.setCursor(0, 0);
    lcd.print(buf);
  }

  // start the server
  server.begin();
  char buf[22]={'='};
  get_ip_as_string(buf, ip, server_port);
  lcd.setCursor(0, 0);
  lcd.print(buf);
  Serial.println("Setup complete...");
}

//
// Main Loop function
//
void loop() {
  show_tick();

  byte temps[2] = {0, 0};
  get_temperatures(temps);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    char remote_ip[22]={'='};
    get_ip_as_string(remote_ip, client.remoteIP(), 80, false);
    Serial.print("Remote IP: ");
    Serial.println(remote_ip);
    lcd.setCursor(0,1);
    lcd.print(remote_ip);
    process_client(client, temps);
    close_client_connection(client);
  }

  char buf[22]={0};
  get_temperature_string(buf, temps);
  lcd.setCursor(0, 2);
  lcd.print(buf);
  //Serial.println(msg);
}
