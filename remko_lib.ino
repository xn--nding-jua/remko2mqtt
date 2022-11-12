#define InvertCommunication 1
#define BitTime 550 // microseconds
#define cmdlength 21  // bytes

uint8_t remko_powerstate[]={0,0,0,0};
uint8_t remko_opmode[]={0,0,0,0};
uint8_t remko_setpoint[]={0,0,0,0};
uint8_t remko_followme[]={0,0,0,0};

// Setup Remko-communication
static unsigned long lastBitSentTime = 0;
uint8_t bitcounter_txd[] = {0,0,0,0}; // bitstream stopped
uint8_t remko_sendcmd_state[] = {0,0,0,0}; // current state of the statemachine

// storage for the current command for each device as we support sending data in parallel
byte cmd_txd[4][21] = {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};

// definition of the supported commands
const byte remko_cmd[17][21] = {{0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEE, 0xAE, 0xAE, 0xAA, 0xBB, 0xEE, 0xFE}, //turn on
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xBA, 0xBB, 0xEB, 0xEE, 0xAA, 0xAE, 0xEE, 0xAE, 0xAA, 0xEA, 0xEE, 0xEE, 0xFE}, // turn off
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAA, 0xBB, 0xBB, 0xBB, 0xBB, 0xAA, 0xEA, 0xBA, 0xEA, 0xAE, 0xEB, 0xEE, 0xFE}, // auto
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xAA, 0xBA, 0xEB, 0xEE, 0xEE, 0xFE}, // cool
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAA, 0xBB, 0xBB, 0xBB, 0xBB, 0xAA, 0xEA, 0xEA, 0xEA, 0xAE, 0xBB, 0xEE, 0xFE}, // dry
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xBA, 0xAB, 0xBB, 0xAE, 0xEE, 0xFE}, // heat
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xEE, 0xEA, 0xEE, 0xAE, 0xEE, 0xFE}, // 17°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xBB, 0xAB, 0xBB, 0xAB, 0xEE, 0xFE}, // 18°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xEE, 0xAE, 0xEE, 0xAA, 0xEE, 0xFE}, // 19°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xBA, 0xAB, 0xBB, 0xAE, 0xEE, 0xFE}, // 20°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xEB, 0xAE, 0xAE, 0xAE, 0xEE, 0xFE}, // 21°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBB, 0xBB, 0xBA, 0xAA, 0xEE, 0xFE}, // 22°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xEE, 0xAE, 0xAE, 0xAB, 0xEE, 0xFE}, // 23°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBA, 0xAB, 0xEB, 0xAE, 0xEE, 0xFE}, // 24°C
                               {0x0, 0xFF, 0xAE, 0xBB, 0xEB, 0xEA, 0xAA, 0xEB, 0xBA, 0xAE, 0xEA, 0xBA, 0xEE, 0xEE, 0xBA, 0xBA, 0xBB, 0xAE, 0xBB, 0xEA, 0xFE}, // followme on
                               {0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBA, 0xAB, 0xEB, 0xAE, 0xEE, 0xFE}, // followme off
                               {0x0, 0xFF, 0xAE, 0xBB, 0xAE, 0xEB, 0xEA, 0xBA, 0xEE, 0xEE, 0xBA, 0xAE, 0xEA, 0xBA, 0xAE, 0xAB, 0xEB, 0xBA, 0xEE, 0xBA, 0xFE}}; // toggle led

void remko_txd_init() {
  for (int device=0;device<RemkoTxDevices;device++){
    pinMode(RemkoTxPin[device], OUTPUT);
    #ifdef InvertCommunication
      digitalWrite(RemkoTxPin[device], LOW);
    #else
      digitalWrite(RemkoTxPin[device], HIGH);
    #endif
  }
}

void remko_publishmqtt() {
  #if RemkoTxDevices >= 1
    mqttclient.publish(mqtt_topic_dev0_powerstate, String(remko_powerstate[0]).c_str());  
    mqttclient.publish(mqtt_topic_dev0_opmode, String(remko_opmode[0]).c_str());  
    mqttclient.publish(mqtt_topic_dev0_setpoint, String(remko_setpoint[0]).c_str());  
    mqttclient.publish(mqtt_topic_dev0_followme, String(remko_followme[0]).c_str());  
  #endif
  #if RemkoTxDevices >= 2
    mqttclient.publish(mqtt_topic_dev1_powerstate, String(remko_powerstate[1]).c_str());  
    mqttclient.publish(mqtt_topic_dev1_opmode, String(remko_opmode[1]).c_str());  
    mqttclient.publish(mqtt_topic_dev1_setpoint, String(remko_setpoint[1]).c_str());  
    mqttclient.publish(mqtt_topic_dev1_followme, String(remko_followme[1]).c_str());  
  #endif
  #if RemkoTxDevices >= 3
    mqttclient.publish(mqtt_topic_dev2_powerstate, String(remko_powerstate[2]).c_str());  
    mqttclient.publish(mqtt_topic_dev2_opmode, String(remko_opmode[2]).c_str());  
    mqttclient.publish(mqtt_topic_dev2_setpoint, String(remko_setpoint[2]).c_str());  
    mqttclient.publish(mqtt_topic_dev2_followme, String(remko_followme[2]).c_str());  
  #endif
  #if RemkoTxDevices >= 4
    mqttclient.publish(mqtt_topic_dev3_powerstate, String(remko_powerstate[3]).c_str());  
    mqttclient.publish(mqtt_topic_dev3_opmode, String(remko_opmode[3]).c_str());  
    mqttclient.publish(mqtt_topic_dev3_setpoint, String(remko_setpoint[3]).c_str());  
    mqttclient.publish(mqtt_topic_dev3_followme, String(remko_followme[3]).c_str());  
  #endif
}

void remko_txd_step() {
  if (micros() - lastBitSentTime >= BitTime) {
    lastBitSentTime+=BitTime;
	
  	for (int device=0; device<RemkoTxDevices; device++) {
        remko_txd_sendbit(device);
  	}
  }
}

void remko_txd_sendbit(uint8_t device) {
  // this state-machine has five states: 0=standby, 1=send command, 2&3=waitstates, 4=send command
  if ((remko_sendcmd_state[device]==1) || (remko_sendcmd_state[device]==4)) {
    if (bitcounter_txd[device]<cmdlength*8) {
      #ifdef InvertCommunication
        digitalWrite(RemkoTxPin[device], !bitRead(cmd_txd[device][bitcounter_txd[device]/8], bitcounter_txd[device]-(bitcounter_txd[device]/8)*8));
      #else
        digitalWrite(RemkoTxPin[device], bitRead(cmd_txd[device][bitcounter_txd[device]/8], bitcounter_txd[device]-(bitcounter_txd[device]/8)*8));
      #endif
      bitcounter_txd[device]+=1;
    }else{
      // we reached end of transmission
      #ifdef InvertCommunication
        digitalWrite(RemkoTxPin[device], LOW);
      #else
        digitalWrite(RemkoTxPin[device], HIGH);
      #endif

      if (remko_sendcmd_state[device]==1) {
        // pause for 2 bits and then send same command second time
        remko_sendcmd_state[device]=2; // enter first wait-state
        bitcounter_txd[device]=0;
      }else if (remko_sendcmd_state[device]==4) {
        // stop transmission and reset bitcounter_txd
        remko_sendcmd_state[device]=0;
        bitcounter_txd[device]=0;
      }
    }
  }else if (remko_sendcmd_state[device]>0) {
    remko_sendcmd_state[device]++; // increase to second wait-state and then to second command
  }
}

void remko_txd_sendcmd(uint8_t device, uint8_t cmdtype, uint8_t value) {
  // load command
  
  switch (cmdtype) {
    case 0: // powerstate
      if (value==1) {
        memcpy(cmd_txd[device], remko_cmd[0], cmdlength);
        remko_powerstate[device]=1;
      }else{
        memcpy(cmd_txd[device], remko_cmd[1], cmdlength);
        remko_powerstate[device]=0;
      }
      break;
    case 1: // opmode
      switch(value) {
        case 0:
          memcpy(cmd_txd[device], remko_cmd[2], cmdlength);
          break;
        case 1:
          memcpy(cmd_txd[device], remko_cmd[3], cmdlength);
          break;
        case 2:
          memcpy(cmd_txd[device], remko_cmd[4], cmdlength);
          break;
        case 3:
          memcpy(cmd_txd[device], remko_cmd[5], cmdlength);
          break;
      }
      remko_opmode[device]=value;
      break;
    case 2: // setpoint
      switch(value) {
        case 17:
          memcpy(cmd_txd[device], remko_cmd[6], cmdlength);
          break;
        case 18:
          memcpy(cmd_txd[device], remko_cmd[7], cmdlength);
          break;
        case 19:
          memcpy(cmd_txd[device], remko_cmd[8], cmdlength);
          break;
        case 20:
          memcpy(cmd_txd[device], remko_cmd[9], cmdlength);
          break;
        case 21:
          memcpy(cmd_txd[device], remko_cmd[10], cmdlength);
          break;
        case 22:
          memcpy(cmd_txd[device], remko_cmd[11], cmdlength);
          break;
        case 23:
          memcpy(cmd_txd[device], remko_cmd[12], cmdlength);
          break;
        case 24:
          memcpy(cmd_txd[device], remko_cmd[13], cmdlength);
          break;
      }
      remko_setpoint[device]=value;
      break;
    case 3: // followme
      if (value==1) {
        memcpy(cmd_txd[device], remko_cmd[14], cmdlength);
        remko_followme[device]=1;
      }else{
        memcpy(cmd_txd[device], remko_cmd[15], cmdlength);
        remko_followme[device]=0;
      }
      break;
    case 4: // led
      memcpy(cmd_txd[device], remko_cmd[16], cmdlength);
      break;
  }

  // start bitstream-transmission
  bitcounter_txd[device] = 0;
  remko_sendcmd_state[device] = 1;
}
