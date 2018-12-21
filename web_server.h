#include <Ethernet.h>

// client incomming message variables
#define CLIENT_BUF_SIZE 129

class WebServer{
private:
	IPAddress server_ip;
	EthernetServer* server;
	byte mac[6] = {0x8E, 0x19, 0xA7, 0x54, 0xD5, 0x97};
	char client_buffer[CLIENT_BUF_SIZE + 1] = {0};
	
public:
	WebServer(){
		this->server_ip = IPAddress(192, 168, 1, 177);
		this->server = new EthernetServer((uint16_t)80);
	}
	
	~WebServer(){
		delete this->server;
	}

	//
	// Initialise the Ethernet board
	//
	void init_board(){
		// You can use Ethernet.init(pin) to configure the CS pin
		Ethernet.init(10);  // Most Arduino shields
		// start the Ethernet connection and the server:
		Ethernet.begin(this->mac, this->server_ip);
	}

	//
	// Bind the server to its port and listen for clients
	//
	void begin(){
		this->server->begin();
	}
	
	void get_server_ip_as_string(char buf[], bool add_port = true) {
		IPAddress ip = this->server_ip;
		if(add_port)
			sprintf(buf, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], 80);
		else
			sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	}
	
	void get_client_ip_as_string(EthernetClient& client, char buf[]) {
		IPAddress ip = client.remoteIP();
		sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	}
	
	EthernetClient available(){
		return this->server->available();
	}		
		
	//
	// Processes one line at a time from the client request
	//
	bool get_request_line(EthernetClient& client) {
		client_buffer[0] = '\0';

		if (client.connected() && client.available()) {
			for (int i = 0; i < CLIENT_BUF_SIZE; i++) {
				char c = client.read();
				// If there are no more characters then quit
				if (c == -1)
					return true;

				client_buffer[i] = c;
				if (c == '\n') {
					if (i <= 1) {
						// End of request detected
						client_buffer[i + 1] = '\0';
						return true;
					}
					// No end of request detected
					client_buffer[i + 1] = '\0';
					return false;
				}
			}
		}
		// No end of request detected
		return false;
	}


	//
	// Closes the client connection
	//
	void close_client_connection(EthernetClient& client) {
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
		client.stop();
		Serial.println("Client disconnected");
	}

	//
	// Process client request data
	//
	void process_client(EthernetClient& client, Relays& relays, const byte temps[2]) {
	  while (!get_request_line(client)) {
			Serial.print(client_buffer);
			if (!strcmp(client_buffer, "GET /temperatures/ HTTP/1.1\r\n")) {
				// flush the buffer
				while (client.read() != -1);
				send_client_data(client, temps);
				return;
			}
			if (!strcmp(client_buffer, "GET /api/v1/status/ HTTP/1.1\r\n")) {
				Serial.println("Sending client API JSON response.");
				// flush the buffer
				while (client.read() != -1);
				
				send_client_header(client);
				char data[33] = {0};

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
				return;
			}
			if (!strcmp(client_buffer, "GET /favicon.ico HTTP/1.1\r\n")) {
				// flush the buffer
				while (client.read() != -1);
				client.println("HTTP/1.1 404 NOT FOUND");
				client.println("Content-Type: text/html");
				client.println("Connection: close");  // the connection will be closed after completion of the response
				return;
			}
		}
	}
	
	//
	// Sends a standard 200 OK client header.
	//
	void send_client_header(
			EthernetClient& client, 
			int refresh=0, 
			int status_code=200, 
			const char status_text[]="OK"
		) {
		// send a standard http response header
		client.print("HTTP/1.1 ");
		client.print(status_code);
		client.print(" ");
		client.println(status_text);
		client.println("Content-Type: text/html");
		client.println("Connection: close");  // the connection will be closed after completion of the response
		if (refresh) {
			client.print("Refresh: ");
			client.println(refresh);  // refresh the page automatically every n sec
		}
		client.println();
	}
		
	//
	// Sends temperature data as a HTML page
	//
	void send_client_data(EthernetClient& client, const byte temps[2]) {
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
};