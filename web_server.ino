/*
  Temperature Sensor Web Server
*/

#include <LiquidCrystal_I2C.h>
#include "relays.h"
#include "temp_sensors.h"
#include "web_server.h"

Relays relays;
TempSensors sensors;
WebServer server;

// Setup the LCD display
LiquidCrystal_I2C lcd(0x3F, 20, 4);

//
// Shows a dot tick on the LCD display in the top-right corner
//
byte tick = 0;
void show_tick() {
  const char* tick_chars = " .";
  lcd.setCursor(19, 0);
  lcd.print(tick_chars[tick++]);
  if (tick == 2) tick = 0;
}

void favicon_callback(EthernetClient& client, const char* request){
  WebServer::send_client_header(client, 404, "NOT FOUND");
}

void api_status_callback(EthernetClient& client, const char* request){
	byte temps[2] = {0, 0};
  sensors.last_read_all(temps);
  
  WebServer::send_client_header(client);

  char data[48] = {0};
  client.print("{\"temperatures\":[");
  for (int i = 0; i < 2; i++) {
    sprintf(data, "%i", temps[i]);
    client.print(data);
    if (i != 1)
      client.print(",");
  }
  client.print("],\"relays\":[");
  for (int i = 0; i < 6; i++) {
    sprintf(
      data,
      "{\"name\":\"%s\", \"state\":%i}",
      relays.name(i), relays.get_state(i)
    );
    client.print(data);
    if (i != 5)
      client.print(",");
  }
  client.print("]}");
	Serial.println("***D1");
}

void temperatures_callback(EthernetClient& client, const char* request){
  byte temps[2] = {0, 0};
  sensors.last_read_all(temps);
  
  WebServer::send_client_header(client, 200, "OK", 5);
  client.println("<!DOCTYPE HTML>\n<html>\n<h1>Temperature sensors</h1>\nTemp1: ");
  client.print(temps[0]);
  client.println("<br />Temp2: ");
  client.print(temps[1]);
  client.println("<br /></html>");
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

  server.init_board();

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    const char* buf = "Ethernet shield was not found.";
    Serial.println(buf);
    lcd.setCursor(0, 0);
    lcd.print(buf);
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    const char* buf = "Ethernet cable is not connected.";
    Serial.println(buf);
    lcd.setCursor(0, 0);
    lcd.print(buf);
  }

  // start the server
  server.begin();
  char buf[22] = {'='};
  server.get_server_ip_as_string(buf);
  lcd.setCursor(0, 0);
  lcd.print(buf);

  server.register_callback("GET /favicon.ico ", &favicon_callback);
  server.register_callback("GET /api/v1/status/ ", &api_status_callback);
  server.register_callback("GET /temperatures/ ", &temperatures_callback);
  
}

//
// Main Loop function
//
void loop() {
  show_tick();

  byte temps[2] = {0, 0};
  sensors.read_all(temps);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    char remote_ip[22] = {'='};
    server.get_client_ip_as_string(client, remote_ip);
    lcd.setCursor(0, 1);
    lcd.print(remote_ip);
    server.process_client(client, relays, temps);
    server.close_client_connection(client);
  }

  char buf[22] = {0};
  sensors.get_temperature_string(buf, temps);
  lcd.setCursor(0, 2);
  lcd.print(buf);
}
