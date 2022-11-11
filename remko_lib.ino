#define InvertCommunication 1
#define BitTime 550

uint8_t remko_powerstate[]={0,0,0,0};
uint8_t remko_opmode[]={0,0,0,0};
uint8_t remko_setpoint[]={0,0,0,0};
uint8_t remko_followme[]={0,0,0,0};

// Setup Remko-communication
static unsigned long lastBitRefreshTime = 0;
uint8_t bitcounter[] = {0,0,0,0}; // bitstream stopped
uint8_t remko_cmd_state[] = {0,0,0,0}; // current state of the statemachine
#define cmdlength 21
// storage for the current command for each device as we support sending data in parallel
byte cmd0[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
byte cmd1[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
byte cmd2[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
byte cmd3[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

// definition of the supported commands
const byte cmd_turnon[] =      {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEE, 0xAE, 0xAE, 0xAA, 0xBB, 0xEE, 0xFE};
const byte cmd_turnoff[] =     {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xBA, 0xBB, 0xEB, 0xEE, 0xAA, 0xAE, 0xEE, 0xAE, 0xAA, 0xEA, 0xEE, 0xEE, 0xFE};
const byte cmd_auto[] =        {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAA, 0xBB, 0xBB, 0xBB, 0xBB, 0xAA, 0xEA, 0xBA, 0xEA, 0xAE, 0xEB, 0xEE, 0xFE};
const byte cmd_cool[] =        {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xAA, 0xBA, 0xEB, 0xEE, 0xEE, 0xFE};
const byte cmd_dry[] =         {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAA, 0xBB, 0xBB, 0xBB, 0xBB, 0xAA, 0xEA, 0xEA, 0xEA, 0xAE, 0xBB, 0xEE, 0xFE};
const byte cmd_heat[] =        {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xBA, 0xAB, 0xBB, 0xAE, 0xEE, 0xFE};
const byte cmd_17[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xEE, 0xEA, 0xEE, 0xAE, 0xEE, 0xFE};
const byte cmd_18[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xBB, 0xAB, 0xBB, 0xAB, 0xEE, 0xFE};
const byte cmd_19[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xEE, 0xAE, 0xEE, 0xAA, 0xEE, 0xFE};
const byte cmd_20[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xBA, 0xAB, 0xBB, 0xAE, 0xEE, 0xFE};
const byte cmd_21[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xEB, 0xAE, 0xAE, 0xAE, 0xEE, 0xFE};
const byte cmd_22[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBB, 0xBB, 0xBA, 0xAA, 0xEE, 0xFE};
const byte cmd_23[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xEE, 0xAE, 0xAE, 0xAB, 0xEE, 0xFE};
const byte cmd_24[] =          {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBA, 0xAB, 0xEB, 0xAE, 0xEE, 0xFE};
const byte cmd_followmeon[] =  {0x0, 0xFF, 0xAE, 0xBB, 0xEB, 0xEA, 0xAA, 0xEB, 0xBA, 0xAE, 0xEA, 0xBA, 0xEE, 0xEE, 0xBA, 0xBA, 0xBB, 0xAE, 0xBB, 0xEA, 0xFE};
const byte cmd_followmeoff[] = {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBA, 0xAB, 0xEB, 0xAE, 0xEE, 0xFE};
const byte cmd_led[] =         {0x0, 0xFF, 0xAE, 0xBB, 0xAE, 0xEB, 0xEA, 0xBA, 0xEE, 0xEE, 0xBA, 0xAE, 0xEA, 0xBA, 0xAE, 0xAB, 0xEB, 0xBA, 0xEE, 0xBA, 0xFE};

void remko_comm_init() {
  #ifdef RemkoDevice0
    pinMode(RemkoDevice0, OUTPUT);
    #ifdef InvertCommunication
      digitalWrite(RemkoDevice0, LOW);
    #else
      digitalWrite(RemkoDevice0, HIGH);
    #endif
  #endif
  #ifdef RemkoDevice1
    pinMode(RemkoDevice1, OUTPUT);
    #ifdef InvertCommunication
      digitalWrite(RemkoDevice1, LOW);
    #else
      digitalWrite(RemkoDevice1, HIGH);
    #endif
  #endif
  #ifdef RemkoDevice2
    pinMode(RemkoDevice2, OUTPUT);
    #ifdef InvertCommunication
      digitalWrite(RemkoDevice2, LOW);
    #else
      digitalWrite(RemkoDevice2, HIGH);
    #endif
  #endif
  #ifdef RemkoDevice3
    pinMode(RemkoDevice3, OUTPUT);
    #ifdef InvertCommunication
      digitalWrite(RemkoDevice3, LOW);
    #else
      digitalWrite(RemkoDevice3, HIGH);
    #endif
  #endif
}

void remko_comm_publishmqtt() {
  #ifdef RemkoDevice0
    mqttclient.publish(mqtt_topic_dev0_powerstate, String(remko_powerstate[0]).c_str());  
    mqttclient.publish(mqtt_topic_dev0_opmode, String(remko_opmode[0]).c_str());  
    mqttclient.publish(mqtt_topic_dev0_setpoint, String(remko_setpoint[0]).c_str());  
    mqttclient.publish(mqtt_topic_dev0_followme, String(remko_followme[0]).c_str());  
  #endif
  #ifdef RemkoDevice1
    mqttclient.publish(mqtt_topic_dev1_powerstate, String(remko_powerstate[1]).c_str());  
    mqttclient.publish(mqtt_topic_dev1_opmode, String(remko_opmode[1]).c_str());  
    mqttclient.publish(mqtt_topic_dev1_setpoint, String(remko_setpoint[1]).c_str());  
    mqttclient.publish(mqtt_topic_dev1_followme, String(remko_followme[1]).c_str());  
  #endif
  #ifdef RemkoDevice2
    mqttclient.publish(mqtt_topic_dev2_powerstate, String(remko_powerstate[2]).c_str());  
    mqttclient.publish(mqtt_topic_dev2_opmode, String(remko_opmode[2]).c_str());  
    mqttclient.publish(mqtt_topic_dev2_setpoint, String(remko_setpoint[2]).c_str());  
    mqttclient.publish(mqtt_topic_dev2_followme, String(remko_followme[2]).c_str());  
  #endif
  #ifdef RemkoDevice3
    mqttclient.publish(mqtt_topic_dev3_powerstate, String(remko_powerstate[3]).c_str());  
    mqttclient.publish(mqtt_topic_dev3_opmode, String(remko_opmode[3]).c_str());  
    mqttclient.publish(mqtt_topic_dev3_setpoint, String(remko_setpoint[3]).c_str());  
    mqttclient.publish(mqtt_topic_dev3_followme, String(remko_followme[3]).c_str());  
  #endif
}

void remko_comm_step() {
  if (micros() - lastBitRefreshTime >= BitTime) {
    lastBitRefreshTime+=BitTime;
    #ifdef RemkoDevice0
      remko_comm_sendbit(0, RemkoDevice0, cmd0);
    #endif
    #ifdef RemkoDevice1
      remko_comm_sendbit(1, RemkoDevice1, cmd1);
    #endif
    #ifdef RemkoDevice2
      remko_comm_sendbit(2, RemkoDevice2, cmd2);
    #endif
    #ifdef RemkoDevice3
      remko_comm_sendbit(3, RemkoDevice3, cmd3);
    #endif
  }
}

void remko_comm_sendbit(uint8_t device, uint8_t pin, byte *cmd) {
  // this state-machine has five states: 0=standby, 1=send command, 2&3=waitstates, 4=send command
  if ((remko_cmd_state[device]==1) || (remko_cmd_state[device]==4)) {
    if (bitcounter[device]<cmdlength*8) { // here we can use cmd0 safely as the length will not change between devices
      #ifdef InvertCommunication
        digitalWrite(pin, !bitRead(*(cmd + bitcounter[device]/8), bitcounter[device]-(bitcounter[device]/8)*8));
      #else
        digitalWrite(pin, bitRead(*(cmd + bitcounter[device]/8), bitcounter[device]-(bitcounter[device]/8)*8));
      #endif
      bitcounter[device]+=1;
    }else{
      // we reached end of transmission
      #ifdef InvertCommunication
        digitalWrite(pin, LOW);
      #else
        digitalWrite(pin, HIGH);
      #endif

      if (remko_cmd_state[device]==1) {
        // pause for 2 bits and then send same command second time
        remko_cmd_state[device]=2; // enter first wait-state
        bitcounter[device]=0;
      }else if (remko_cmd_state[device]==4) {
        // stop transmission and reset bitcounter
        remko_cmd_state[device]=0;
        bitcounter[device]=0;
      }
    }
  }else if (remko_cmd_state[device]>0) {
    remko_cmd_state[device]++; // increase to second wait-state and then to second command
  }
}

void remko_comm_sendcmd(uint8_t device, uint8_t cmdtype, uint8_t value) {
  // load command
  byte *cmd;
  switch (device) {
    case 0:
      cmd=cmd0;
      break;
    case 1:
      cmd=cmd1;
      break;
    case 2:
      cmd=cmd2;
      break;
    case 3:
      cmd=cmd3;
      break;
  }
  
  switch (cmdtype) {
    case 0: // powerstate
      if (value==1) {
        memcpy(cmd, cmd_turnon, cmdlength);
        remko_powerstate[device]=1;
      }else{
        memcpy(cmd, cmd_turnoff, cmdlength);
        remko_powerstate[device]=0;
      }
      break;
    case 1: // opmode
      switch(value) {
        case 0:
          memcpy(cmd, cmd_auto, cmdlength);
          break;
        case 1:
          memcpy(cmd, cmd_cool, cmdlength);
          break;
        case 2:
          memcpy(cmd, cmd_dry, cmdlength);
          break;
        case 3:
          memcpy(cmd, cmd_heat, cmdlength);
          break;
      }
      remko_opmode[device]=value;
      break;
    case 2: // setpoint
      switch(value) {
        case 17:
          memcpy(cmd, cmd_17, cmdlength);
          break;
        case 18:
          memcpy(cmd, cmd_18, cmdlength);
          break;
        case 19:
          memcpy(cmd, cmd_19, cmdlength);
          break;
        case 20:
          memcpy(cmd, cmd_20, cmdlength);
          break;
        case 21:
          memcpy(cmd, cmd_21, cmdlength);
          break;
        case 22:
          memcpy(cmd, cmd_22, cmdlength);
          break;
        case 23:
          memcpy(cmd, cmd_23, cmdlength);
          break;
        case 24:
          memcpy(cmd, cmd_24, cmdlength);
          break;
      }
      remko_setpoint[device]=value;
      break;
    case 3: // followme
      if (value==1) {
        memcpy(cmd, cmd_followmeon, cmdlength);
        remko_followme[device]=1;
      }else{
        memcpy(cmd, cmd_followmeoff, cmdlength);
        remko_followme[device]=0;
      }
      break;
    case 4: // led
      memcpy(cmd, cmd_led, cmdlength);
      break;
  }

  // start bitstream-transmission
  bitcounter[device] = 0;
  remko_cmd_state[device] = 1;
}
