#include <Ethernet.h>

// client incoming message variables
#define CLIENT_BUF_SIZE 28
#define SERVER_PORT 9000
#define MAX_CALLBACKS 4

// Define a callback function
typedef void(*func_ptr_t)(EthernetClient &client, const char *request);

class WebServer {
private:
  IPAddress server_ip;
  EthernetServer *server;
  byte mac[6] = {0x8E, 0x19, 0xA7, 0x54, 0xD5, 0x97};
  char client_buffer[CLIENT_BUF_SIZE + 1] = {0};
  char registered_urls[MAX_CALLBACKS][CLIENT_BUF_SIZE + 1];
  func_ptr_t registered_callbacks[MAX_CALLBACKS];
  int callback_counter = 0;

public:
  WebServer() {
    this->server_ip = IPAddress(192, 168, 1, 177);
    this->server = new EthernetServer(SERVER_PORT);
  }

  ~WebServer() {
    delete this->server;
  }

  //
  // Initialise the Ethernet board
  //
  void init_board() {
    // You can use Ethernet.init(pin) to configure the CS pin
    Ethernet.init(10);  // Most Arduino shields
    // start the Ethernet connection and the server:
    Ethernet.begin(this->mac, this->server_ip);
  }

  //
  // Bind the server to its port and listen for clients
  //
  void begin() {
    this->server->begin();
  }

  void get_server_ip_as_string(char buf[], bool add_port = true) {
    get_ip_as_string(this->server_ip, buf, SERVER_PORT, add_port);
  }

  static void get_client_ip_as_string(EthernetClient &client, char buf[]) {
    get_ip_as_string(client.remoteIP(), buf, 0, false);
  }

  static inline void get_ip_as_string(IPAddress ip, char buf[], int port, bool add_port = true) {
    if (add_port)
      sprintf(buf, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], SERVER_PORT);
    else
      sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  }

  EthernetClient available() {
    return this->server->available();
  }

  //
  // Processes one line at a time from the client request
  //
  bool get_request_line(EthernetClient &client) {
    client_buffer[0] = '\0';

    if (client.connected() && client.available()) {
      for (int i = 0; i < CLIENT_BUF_SIZE; i++) {
        char c = client.read();
        // If there are no more characters then quit
        if (c == -1)
          return true;

        client_buffer[i] = c;
        if (c == '\n') {
          client_buffer[i + 1] = '\0';
          // If i<=1 then end of request detected
          return (i <= 1);
        }
      }
    }
    // No end of request detected
    return false;
  }

  //
  // Closes the client connection
  //
  void close_client_connection(EthernetClient &client) {
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }

  //
  // Process client request data
  //
  void process_client(EthernetClient &client, Relays &relays, const byte temps[2]) {
    while (!get_request_line(client)) {
      for (int i = 0; i < callback_counter; i++) {

        // Try to find a match for teh callback URL
        if (!strncmp(client_buffer, registered_urls[i], strlen(registered_urls[i]))) {
          // Clear out the client buffer as we don't need the rest of the data
          while (client.read() != -1);

          // Make a call to the callback function
          registered_callbacks[i](client, client_buffer);
          return;
        }
      }
    }
  }

  //
  // Sends a standard 200 OK client header.
  //
  static void send_client_header(
          EthernetClient &client,
          int status_code = 200,
          const char status_text[] = "OK",
          int refresh = 0
  ) {
    // send a standard http response header
    client.print("HTTP/1.1 ");
    client.print(status_code);
    client.print(" ");
    client.println(status_text);
    client.println("Content-Type: text/html\nConnection: close");
    if (refresh) {
      client.print("Refresh: ");
      client.println(refresh);  // refresh the page automatically every n sec
    }
    client.println();
  }

  void register_callback(const char *url_path, func_ptr_t callback) {
    if (callback_counter == MAX_CALLBACKS) {
      return;
    }

    strcpy(registered_urls[callback_counter], url_path);
    registered_callbacks[callback_counter] = callback;
    callback_counter++;
  }

};
