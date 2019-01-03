/*
  Temperature Sensor Web Server
*/

#include "relays.h"
#include "temp_sensors.h"
#include "web_server.h"

#define WATCHDOG_PIN 7

Relays relays;
TempSensors sensors;
WebServer server;

bool blink_value = false;
const char *temp_names[] = {"water", "air"};
byte watchdog_counter = 0;
unsigned long loop_counter = 0;
unsigned long valid_requests = 0;
unsigned long requests = 0;

void favicon_callback(EthernetClient &client, const char *request) {
  WebServer::send_client_header(client, 404, "NOT FOUND");
  valid_requests++;
}

void api_status_callback(EthernetClient &client, const char *request) {
  byte temps[2] = {0, 0};
  sensors.last_read_all(temps);

  WebServer::send_client_header(client);

  char data[48] = {0};
  client.print("{\"temperatures\":{");
  for (int i = 0; i < 2; i++) {
    sprintf(data, "\"%s\": %i", temp_names[i], temps[i]);
    client.print(data);
    if (i != 1)
      client.print(",");
  }
  client.print("},\"relays\":{");
  for (int i = 0; i < MAX_RELAYS; i++) {
    sprintf(
            data,
            "\"%s\":%i",
            relays.name(i), relays.get_state(i)
    );
    client.print(data);
    if (i != MAX_RELAYS - 1)
      client.print(",");
  }
  client.print("}, \"system_info\":{ ");
  unsigned long ticks = millis();
  sprintf(data, "\"ticks_ms\":%lu,", ticks);
  client.print(data);
  sprintf(data, "\"loops\":%lu, ", loop_counter);
  client.print(data);
  sprintf(data, "\"loop_time_ms\":%lu, ", (unsigned long) (ticks / loop_counter));
  client.print(data);

  valid_requests++;
  client.print("\"requests\": {");
  sprintf(data, "\"valid\":%lu, ", (unsigned long) valid_requests);
  client.print(data);
  sprintf(data, "\"invalid\":%lu, ", (unsigned long) (requests - valid_requests));
  client.print(data);
  sprintf(data, "\"total\":%lu ", (unsigned long) requests);
  client.print(data);
  client.print("} } }");
}

void temperatures_callback(EthernetClient &client, const char *request) {
  byte temps[2] = {0, 0};
  sensors.last_read_all(temps);

  WebServer::send_client_header(client, 200, "OK", 5);
  client.println("<!DOCTYPE HTML>\n<html>\n<h1>Temperature sensors</h1>\nWater: ");
  client.print(temps[0]);
  client.println("<br />Air: ");
  client.print(temps[1]);
  client.println("<br /></html>");
  valid_requests++;
}

void api_relays_callback(EthernetClient &client, const char *request) {
  char buf[7] = {0};
  const char *ptr = request + 20;
  for (int i = 0; i < MAX_RELAYS; i++) {
    sprintf(buf, "%i/on/", i);
    if (!strncmp(ptr, buf, 5)) {
      relays.set_state(i, 1);
      WebServer::send_client_header(client);
      valid_requests++;
      return;
    }
    sprintf(buf, "%i/off/", i);
    if (!strncmp(ptr, buf, 6)) {
      relays.set_state(i, 0);
      WebServer::send_client_header(client);
      valid_requests++;
      return;
    }
  }
}

//
// Main Setup function
//
void setup() { 
  server.init_board();

  pinMode(WATCHDOG_PIN, OUTPUT);
  digitalWrite(WATCHDOG_PIN, 1);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    bool state = false;
    while (true) {
      delay(1000); // do nothing, no point running without Ethernet hardware
      relays.set_state(1, state);
      state = !state;
    }

  }

  // start the server
  server.begin();
  server.register_callback("GET /api/v1/status/ ", &api_status_callback);
  server.register_callback("GET /temperatures/ ", &temperatures_callback);
  server.register_callback("GET /favicon.ico ", &favicon_callback);
  server.register_callback("POST /api/v1/relays/", &api_relays_callback);

  // test the relays
  relays.test();
}

//
// Main Loop function
//
void loop() {
  byte temps[2] = {0, 0};
  sensors.read_all(temps);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    requests++;
    server.process_client(client, relays, temps);
    server.close_client_connection(client);
  }
  watchdog_counter++;
  watchdog_counter %= 20;
  digitalWrite(WATCHDOG_PIN, watchdog_counter < 10);
  loop_counter++; 
}
