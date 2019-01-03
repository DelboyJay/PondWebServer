/*
  Temperature Sensor Web Server
*/

#include <OneWire.h>
#include <DallasTemperature.h>

/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

class TempSensors{
private:
	OneWire* oneWire;
	DallasTemperature* sensors;

public:
	TempSensors(){
		// Data wire is plugged into pin 2 on the Arduino
		this->oneWire = new OneWire(ONE_WIRE_BUS);
		this->sensors = new DallasTemperature(this->oneWire);
		// Initialise the temperature sensors
		this->sensors->begin();
	}
	
	~TempSensors(){
		// Data wire is plugged into pin 2 on the Arduino
		delete this->oneWire;
		delete this->sensors;
	}
	
	//
	// Obtains the tempratures form the sensors
	//
	byte read_value(const int index){
		this->sensors->requestTemperatures(); // Send the command to get temperature readings
		return byte(sensors->getTempCByIndex(index));
	}
	
	//
	// Returns a list of tempratures from all the sensors
	//	
	void read_all(byte temperatures[2]){
		this->sensors->requestTemperatures(); // Send the command to get temperature readings
		for(int i=0;i<2;i++){
			temperatures[i] = byte(sensors->getTempCByIndex(i));
		}
	}
	
	//
	// Obtains the last temprature read from the specified sensor - fast read
	// NOTE: this does not reflect the actual temperature at this point in time.
	//
	byte last_read_value(const int index){
		return byte(sensors->getTempCByIndex(index));
	}

	//
	// Returns the last values read by the sensors.
	// NOTE: this does not reflect the actual temperature at this point in time.
	void last_read_all(byte temperatures[2]){
		for(int i=0;i<2;i++){
			temperatures[i] = byte(sensors->getTempCByIndex(i));
		}
	}
};
