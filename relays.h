//
// Class that deals with relays on specific pins
//

#define MAX_RELAYS 3

class Relays {
private:
  const byte pins[MAX_RELAYS] = {3, 4, 5};
  // default first relay as on for the pond pump
  byte states[MAX_RELAYS] = {1, 0, 0};
  const char *names[MAX_RELAYS] = {
          "pond_pump",
          "waterfall_lights",
          "rockery_lights",
  };

public:
  Relays() {
    // setup pins
    for (int index = 0; index < sizeof(pins); index++) {
      pinMode(pins[index], OUTPUT);
      this->set_state(index, this->states[index]);
    }
  };

  void set_state(const int index, const bool value) {
    // relays are active zero so flip the request
    this->states[index] = (byte) value;
    digitalWrite(pins[index], (int) !value);
  };

  bool get_state(const int index) {
    return this->states[index];
  };

  const char *name(int index) {
    return names[index];
  }

  const int count() {
    return sizeof(pins);
  };

  void get_states(byte buf[MAX_RELAYS]){
    for(int i=0;i<MAX_RELAYS;i++)
      buf[i] = states[i];
  }

  void test(){
    byte buf[MAX_RELAYS]={0};
    this->get_states(buf);
    
    for (int index = 0; index < sizeof(pins); index++) {
      this->set_state(index, 1);
      delay(1000);      
    }    
    for (int index = 0; index < sizeof(pins); index++) {
      this->set_state(index, 0);
      delay(1000);      
    }    
    for (int index = 0; index < sizeof(pins); index++) {
      this->set_state(index, buf[index]);
    }      
  }
};
