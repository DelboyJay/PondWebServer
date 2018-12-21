//
// Class that deals with relays on specific pins
//
class Relays{
private:
  const byte pins[6]={ 4, 5, 6, 7, 8, 9 };
  // default first relay as on for the pond pump
  byte states[6]={1,0,0,0,0,0};
  const char* names[6]={
    "Pond pump",
    "LED Lights",
    "Rockery Lights",
    "",
    "",
    "",
  };
  
public:
  Relays(){
    // setup pins
    for(int index=0;index<sizeof(pins);index++){
      pinMode(pins[index], OUTPUT);
      this->set_state(index, this->states[index]);
    }
  };

  void set_state(const int index, const bool value){
    // relays are active zero so flip the request
    this->states[index] = (byte)value;
    digitalWrite(pins[index], (int)!value);        
  };

  bool get_state(const int index){
    return this->states[index];
  };

  const char* name(int index){
    return names[index];
  }

  const int count(){
    return sizeof(pins);
  };
};
