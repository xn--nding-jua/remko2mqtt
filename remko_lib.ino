#define TxBitTime 545 // microseconds
#define RxBitTime 545 // microseconds
#define cmdlength 21  // bytes
//#define UseInterrupts // use interrupts for detecting the edges of the signal (Caution: not working well with WiFi-module)

uint8_t remko_debuglevel = 0; // 0=disable, 1=send Rx command
struct remko_state {
  uint8_t powerState;
  uint8_t opMode;
  uint8_t setPoint;
};
struct remko_state remko_states[4] = {{.powerState=0, .opMode=0, .setPoint=0},
                               {.powerState=0, .opMode=0, .setPoint=0}, 
                               {.powerState=0, .opMode=0, .setPoint=0},
                               {.powerState=0, .opMode=0, .setPoint=0}};

// Setup Remko-communication
static unsigned long lastBitSentTime = 0;
uint8_t bitcounter_txd[] = {0,0,0,0}; // bitstream stopped
uint8_t remko_sendcmd_state[] = {0,0,0,0}; // current state of the statemachine

uint8_t bitcounter_rxd[] = {0,0,0,0}; // search for beginning
unsigned long lastBitReadTime[] = {0,0,0,0};
#ifdef UseInterrupts
  volatile unsigned long HighBitTime[] = {0,0,0,0};
  volatile unsigned long LowBitTime[] = {0,0,0,0};
#else
  unsigned long HighBitTime[] = {0,0,0,0};
  unsigned long LowBitTime[] = {0,0,0,0};
  #ifdef RemkoInvertRxD
    uint8_t lastPinState[] = {0,0,0,0};
  #else
    uint8_t lastPinState[] = {1,1,1,1};
  #endif
#endif

// storage for the current command for each device as we support sending data in parallel
byte cmd_txd[4][cmdlength] = {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};

static byte cmd_rxd[4][cmdlength] = {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};

// definition of the supported commands
#define NumberOfSupportedCmds 17
const byte remko_cmd[NumberOfSupportedCmds][cmdlength] = {
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xBA, 0xBB, 0xeb, 0xEE, 0xAA, 0xAE, 0xEE, 0xAE, 0xAA, 0xEA, 0xEE, 0xEE, 0xFE}, // PowerOff
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xFE}, // 17 Cool, FanAuto
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xAB, 0xBA, 0xBB, 0xEE, 0xEE, 0xFE}, // 18
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xAE, 0xEA, 0xAE, 0xEE, 0xEE, 0xFE}, // 19
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xAA, 0xBA, 0xeb, 0xEE, 0xEE, 0xFE}, // 20
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xAB, 0xEA, 0xEA, 0xEE, 0xEE, 0xFE}, // 21
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBB, 0xAA, 0xAB, 0xEE, 0xEE, 0xFE}, // 22
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xAE, 0xEA, 0xBA, 0xEE, 0xEE, 0xFE}, // 23
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xAA, 0xBA, 0xEE, 0xEE, 0xEE, 0xFE}, // 24
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xEE, 0xEA, 0xEE, 0xAE, 0xEE, 0xFE}, // 17 Heat, FanAuto
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xAA, 0xBB, 0xAB, 0xBB, 0xAB, 0xEE, 0xFE}, // 18
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xEE, 0xAE, 0xEE, 0xAA, 0xEE, 0xFE}, // 19
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEA, 0xBA, 0xAB, 0xBB, 0xAE, 0xEE, 0xFE}, // 20
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xeb, 0xAE, 0xAE, 0xAE, 0xEE, 0xFE}, // 21
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBB, 0xBB, 0xBA, 0xAA, 0xEE, 0xFE}, // 22
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xEE, 0xAE, 0xAE, 0xAB, 0xEE, 0xFE}, // 23
  {0x00, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xeb, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xBA, 0xBA, 0xAB, 0xeb, 0xAE, 0xEE, 0xFE}}; // 24

void remko_initmqtt() {
  // not connected so try to connect
  mqttclient.connect(mqtt_id);

  // subscribe to the desired topics
  mqttclient.subscribe(mqtt_topic_set_debuglevel);
  mqttclient.subscribe(mqtt_topic_restart);

  #if RemkoTxDevices >= 1
    mqttclient.subscribe(mqtt_topic_dev0_set_poweroff);
    mqttclient.subscribe(mqtt_topic_dev0_set_opmode);
    mqttclient.subscribe(mqtt_topic_dev0_set_setpoint);
  #endif
  #if RemkoTxDevices >= 2
    mqttclient.subscribe(mqtt_topic_dev1_set_poweroff);
    mqttclient.subscribe(mqtt_topic_dev1_set_opmode);
    mqttclient.subscribe(mqtt_topic_dev1_set_setpoint);
  #endif
  #if RemkoTxDevices >= 3
    mqttclient.subscribe(mqtt_topic_dev2_set_poweroff);
    mqttclient.subscribe(mqtt_topic_dev2_set_opmode);
    mqttclient.subscribe(mqtt_topic_dev2_set_setpoint);
  #endif
  #if RemkoTxDevices >= 4
    mqttclient.subscribe(mqtt_topic_dev3_set_poweroff);
    mqttclient.subscribe(mqtt_topic_dev3_set_opmode);
    mqttclient.subscribe(mqtt_topic_dev3_set_setpoint);
  #endif
}

void remko_publishmqtt() {
  #if RemkoTxDevices >= 1
    mqttclient.publish(mqtt_topic_dev0_powerstate, String(remko_states[0].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev0_opmode, String(remko_states[0].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev0_setpoint, String(remko_states[0].setPoint).c_str());  
  #endif
  #if RemkoTxDevices >= 2
    mqttclient.publish(mqtt_topic_dev1_powerstate, String(remko_states[1].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev1_opmode, String(remko_states[1].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev1_setpoint, String(remko_states[1].setPoint).c_str());  
  #endif
  #if RemkoTxDevices >= 3
    mqttclient.publish(mqtt_topic_dev2_powerstate, String(remko_states[2].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev2_opmode, String(remko_states[2].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev2_setpoint, String(remko_states[2].setPoint).c_str());  
  #endif
  #if RemkoTxDevices >= 4
    mqttclient.publish(mqtt_topic_dev3_powerstate, String(remko_states[3].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev3_opmode, String(remko_states[3].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev3_setpoint, String(remko_states[3].setPoint).c_str());  
  #endif
}

// ******* Code for sending Remko-commands ********

void remko_txd_init() {
  for (int device=0;device<RemkoTxDevices;device++) {
    pinMode(RemkoTxPin[device], OUTPUT);
    #ifdef RemkoInvertTxD
      digitalWrite(RemkoTxPin[device], LOW);
    #else
      digitalWrite(RemkoTxPin[device], HIGH);
    #endif
  }
}

void remko_txd_step() {
  if (micros() - lastBitSentTime >= TxBitTime) {
    lastBitSentTime+=TxBitTime;
	
  	for (int device=0; device<RemkoTxDevices; device++) {
        remko_txd_sendbit(device);
  	}
  }
}

void remko_txd_sendbit(uint8_t device) {
  // this state-machine has five states: 0=standby, 1=send command, 2&3=waitstates, 4=send command
  if ((remko_sendcmd_state[device]==1) || (remko_sendcmd_state[device]==4)) {
    if (bitcounter_txd[device]<cmdlength*8) {
      #ifdef RemkoInvertTxD
        digitalWrite(RemkoTxPin[device], !bitRead(cmd_txd[device][bitcounter_txd[device]/8], bitcounter_txd[device]-(bitcounter_txd[device]/8)*8));
      #else
        digitalWrite(RemkoTxPin[device], bitRead(cmd_txd[device][bitcounter_txd[device]/8], bitcounter_txd[device]-(bitcounter_txd[device]/8)*8));
      #endif
      bitcounter_txd[device]+=1;
    }else{
      // we reached end of transmission
      #ifdef RemkoInvertTxD
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

void remko_txd_processmqtt(char* topic, uint8_t value) {
  // the following code is not nice but it is working. Feel free to write better code :)

  if (strcmp(topic, mqtt_topic_set_debuglevel)==0) {
    remko_debuglevel=value;    
  }

  if (strcmp(topic, mqtt_topic_restart)==0) {
    ESP.restart();
  }

  #if RemkoTxDevices >= 1
    if (strcmp(topic, mqtt_topic_dev0_set_poweroff)==0) {
      remko_txd_sendcmd(0, 0, 0);
    }
    if (strcmp(topic, mqtt_topic_dev0_set_opmode)==0) {
      remko_txd_sendcmd(0, 1, value);
    }
    if (strcmp(topic, mqtt_topic_dev0_set_setpoint)==0) {
      remko_txd_sendcmd(0, 2, value);
    }
  #endif
  
  #if RemkoTxDevices >= 2
    if (strcmp(topic, mqtt_topic_dev1_set_poweroff)==0) {
      remko_txd_sendcmd(1, 0, 0);
    }
    if (strcmp(topic, mqtt_topic_dev1_set_opmode)==0) {
      remko_txd_sendcmd(1, 1, value);
    }
    if (strcmp(topic, mqtt_topic_dev1_set_setpoint)==0) {
      remko_txd_sendcmd(1, 2, value);
    }
  #endif
  
  #if RemkoTxDevices >= 3
    if (strcmp(topic, mqtt_topic_dev2_set_poweroff)==0) {
      remko_txd_sendcmd(2, 0, 0);
    }
    if (strcmp(topic, mqtt_topic_dev2_set_opmode)==0) {
      remko_txd_sendcmd(2, 1, value);
    }
    if (strcmp(topic, mqtt_topic_dev2_set_setpoint)==0) {
      remko_txd_sendcmd(2, 2, value);
    }
  #endif
  
  #if RemkoTxDevices >= 4
    if (strcmp(topic, mqtt_topic_dev3_set_poweroff)==0) {
      remko_txd_sendcmd(3, 0, 0);
    }
    if (strcmp(topic, mqtt_topic_dev3_set_opmode)==0) {
      remko_txd_sendcmd(3, 1, value);
    }
    if (strcmp(topic, mqtt_topic_dev3_set_setpoint)==0) {
      remko_txd_sendcmd(3, 2, value);
    }
  #endif
}

bool remko_txd_sendcmd(uint8_t device, uint8_t cmdtype, uint8_t value) {
  if (remko_sendcmd_state[device]>0) {
    // a command is sending at the moment
    // as we have no command-queue we have to reject this command
    return false;
  }
  
  // load command
  switch (cmdtype) {
    case 0: // poweroff
      memcpy(cmd_txd[device], remko_cmd[0], cmdlength);
      remko_states[device].powerState=0;
      break;
    case 1: // opmode
      // store the desired opMode. if device is running send command to device
      remko_states[device].opMode=value; // 0=Cool, 1=Heat
	  
      if (remko_states[device].powerState==0) {
        // device is offline -> dont send command to device, so exit function
        return true;
        break;
      }
      // device is online, fall-through switch-case and send selected setpoint to device with new opMode-setting
    case 2: // setpoint
      if ((value>=17) && (value<=24)) {
        switch (remko_states[device].opMode) {
          case 0: // Cool
            memcpy(cmd_txd[device], remko_cmd[value-17+1], cmdlength); // !!!Caution: array-elements are sorted, so that value fits the correct array-index!!!
            remko_states[device].setPoint=value;
            break;
          case 1: // Heat
            memcpy(cmd_txd[device], remko_cmd[value-17+9], cmdlength); // !!!Caution: array-elements are sorted, so that value fits the correct array-index!!!
            remko_states[device].setPoint=value;
            break;
          default:
            // unsupported command
            return false;
        }
      }
      break;
    default:
      // unsupported command
      return false;
  }

  // start bitstream-transmission
  bitcounter_txd[device] = 0;
  remko_sendcmd_state[device] = 1;
  
  return true;
}

// ******* Code for receiving Remko-commands ********

#ifdef UseInterrupts
  #if RemkoRxDevices >= 1
  void IRAM_ATTR remko_rxd_isr_detecthighbits0() {
    remko_rxd_isr_detecthighbits(0);
  }
  void IRAM_ATTR remko_rxd_isr_detectlowbits0() {
    remko_rxd_isr_detectlowbits(0);
  }
  #endif
  #if RemkoRxDevices >= 2
  void IRAM_ATTR remko_rxd_isr_detecthighbits1() {
    remko_rxd_isr_detecthighbits(1);
  }
  void IRAM_ATTR remko_rxd_isr_detectlowbits1() {
    remko_rxd_isr_detectlowbits(1);
  }
  #endif
  #if RemkoRxDevices >= 3
  void IRAM_ATTR remko_rxd_isr_detecthighbits2() {
    remko_rxd_isr_detecthighbits(2);
  }
  void IRAM_ATTR remko_rxd_isr_detectlowbits2() {
    remko_rxd_isr_detectlowbits(2);
  }
  #endif
  #if RemkoRxDevices >= 4
  void IRAM_ATTR remko_rxd_isr_detecthighbits3() {
    remko_rxd_isr_detecthighbits(3);
  }
  void IRAM_ATTR remko_rxd_isr_detectlowbits3() {
    remko_rxd_isr_detectlowbits(3);
  }
  #endif
  
  void remko_rxd_isr_detecthighbits(uint8_t device) {
    detachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]));
    HighBitTime[device]=micros(); // set highbit flag
  }
  
  void remko_rxd_isr_detectlowbits(uint8_t device) {
    detachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]));
    LowBitTime[device]=micros(); // set lowbit flag
  }
  
  // create array to isr-routine to use for-loops to attach/detach isr
  #if RemkoRxDevices == 4
    void (*isr_table_detecthighbits[4])(void) = { remko_rxd_isr_detecthighbits0, remko_rxd_isr_detecthighbits1, remko_rxd_isr_detecthighbits2, remko_rxd_isr_detecthighbits3};
  #elif RemkoRxDevices == 3
    void (*isr_table_detecthighbits[3])(void) = { remko_rxd_isr_detecthighbits0, remko_rxd_isr_detecthighbits1, remko_rxd_isr_detecthighbits2};
  #elif RemkoRxDevices == 2
    void (*isr_table_detecthighbits[2])(void) = { remko_rxd_isr_detecthighbits0, remko_rxd_isr_detecthighbits1};
  #elif RemkoRxDevices == 1
    void (*isr_table_detecthighbits[1])(void) = { remko_rxd_isr_detecthighbits0};
  #endif
  #if RemkoRxDevices == 4
    void (*isr_table_detectlowbits[4])(void) = { remko_rxd_isr_detectlowbits0, remko_rxd_isr_detectlowbits1, remko_rxd_isr_detectlowbits2, remko_rxd_isr_detectlowbits3};
  #elif RemkoRxDevices == 3
    void (*isr_table_detectlowbits[3])(void) = { remko_rxd_isr_detectlowbits0, remko_rxd_isr_detectlowbits1, remko_rxd_isr_detectlowbits2};
  #elif RemkoRxDevices == 2
    void (*isr_table_detectlowbits[2])(void) = { remko_rxd_isr_detectlowbits0, remko_rxd_isr_detectlowbits1};
  #elif RemkoRxDevices == 1
    void (*isr_table_detectlowbits[1])(void) = { remko_rxd_isr_detectlowbits0};
  #endif
#endif

void remko_rxd_init() {
  for (int device=0;device<RemkoRxDevices;device++){
    pinMode(RemkoRxPin[device], INPUT);
    remko_rxd_reset(device);
  }
}

void remko_rxd_reset(uint8_t device) {
  // reattach interrupt for next command
  bitcounter_rxd[device]=0;

  #ifdef UseInterrupts
    #ifdef RemkoInvertRxD
      attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table_detecthighbits[device]), RISING);
    #else
      attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table_detecthighbits[device]), FALLING);
    #endif
  #else
    #ifdef RemkoInvertRxD
      lastPinState[device]=0;
    #else
      lastPinState[device]=1;
     #endif
  #endif
}

bool remko_rxd_processcmd(uint8_t device) {
  if (remko_debuglevel>=1) {
    // Send received command via MQTT
    String hexstring = "";
    hexstring += String(device);
    hexstring += ':';
    for(int i=0; i<cmdlength; i++) {
      if(cmd_rxd[device][i] < 0x10) {
        hexstring += '0';
      }
      hexstring += String(cmd_rxd[device][i], HEX);
    }
    mqttclient.publish(mqtt_topic_debug, hexstring.c_str());  
  }

  // check header to be 0x0 0xFF 0xAE 0xBB
  if ((cmd_rxd[device][0] != 0x00) || (cmd_rxd[device][1] != 0xFF) || (cmd_rxd[device][2] != 0xAE) ||
      (cmd_rxd[device][3] != 0xBB) || (cmd_rxd[device][20] != 0xFE)) {
    // at least one byte of the header or the ending has unexpected content -> abort processing the command
    return false;
  }

  // check payload: we are only supporting 0xAE, 0xBA , 0xBB, 0xEA, 0xEB and 0xEE
  // 00ffaebb aeebeabaeeeebaaeeabaaeabebbaeeba fe
  for (int i=4; i<20; i++) {
    if (!((cmd_rxd[device][i] == 0xAA) || (cmd_rxd[device][i] == 0xAB) || (cmd_rxd[device][i] == 0xAE) ||
        (cmd_rxd[device][i] == 0xBA) || (cmd_rxd[device][i] == 0xBB) || (cmd_rxd[device][i] == 0xEA) ||
        (cmd_rxd[device][i] == 0xEB) || (cmd_rxd[device][i] == 0xEE))) {
      // unexpected byte -> abort processing the command
      return false;
    }
  }
  
  // compare cmd_rxd[device] and remko_cmd to find the received command-type
  int8_t received_cmd_index = -1;
  for (int i=0; i<NumberOfSupportedCmds; i++) {
    if (memcmp(cmd_rxd[device], remko_cmd[i], cmdlength) == 0) {
      // we found a matching command
      received_cmd_index = i;
      break;
    }
  }

  if (received_cmd_index==0) {
    // PowerOff
    remko_states[device].powerState=0;
  }else if ((received_cmd_index>=1) && (received_cmd_index<=8)){
    // SetPoint Cool
    remko_states[device].powerState=1;
    remko_states[device].setPoint=received_cmd_index+16;
    remko_states[device].opMode=0;
  }else if ((received_cmd_index>=9) && (received_cmd_index<=16)){
    // SetPoint Heat
    remko_states[device].powerState=1;
    remko_states[device].setPoint=received_cmd_index+8;
    remko_states[device].opMode=1;
  }

  // send new values to MQTT-server
  if (mqttclient.connected()) {
    remko_publishmqtt();
  }
  
  // send the new command to output, if desired
  #ifdef MirrorRxD2TxD
    if (remko_debuglevel<2) {
      // copy the received command to the output-buffer
      // this will work with unknown commands, too
      memcpy(cmd_txd[device], cmd_rxd[device], cmdlength);
      // start bitstream-transmission
      bitcounter_txd[device] = 0;
      remko_sendcmd_state[device] = 1;
    }
  #endif

  return true;
}

void remko_rxd_readhighbits(uint8_t device) {
  // read received high-bits
  if (bitcounter_rxd[device]==0) {
    // nothing to do - we are at the beginning of the transmission
  }else{
    // check the received bits
    uint8_t numberOfHighBits = round((HighBitTime[device] - lastBitReadTime[device])/(float)RxBitTime);

    // write detected bits to command-array and increase bitcounter
    uint8_t cmdindex = 0;
    for (uint8_t i=bitcounter_rxd[device]; i<(bitcounter_rxd[device]+numberOfHighBits); i++) {
      cmdindex = i/8;
      if (cmdindex<cmdlength) {
        bitSet(cmd_rxd[device][cmdindex], (i-cmdindex*8));
      }
    }
    bitcounter_rxd[device]+=numberOfHighBits;
  }

  // prepare everything for next bits and reattach interrupt
  lastBitReadTime[device]=micros();
  #ifdef UseInterrupts
    #ifdef RemkoInvertRxD
      attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table_detectlowbits[device]), FALLING);
    #else
      attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table_detectlowbits[device]), RISING);
    #endif
  #endif
}

void remko_rxd_readlowbits(uint8_t device) {
  // check the received bits
  uint8_t numberOfLowBits = round((LowBitTime[device] - lastBitReadTime[device])/(float)RxBitTime);

  if (bitcounter_rxd[device]==0) {
    // searching for beginning
    if (numberOfLowBits==8) {
      // we found the beginning
      bitcounter_rxd[device]=8;
      cmd_rxd[device][0]=0;
    }
  }else{
    // write detected bits to command-array and increase bitcounter
    uint8_t cmdindex = 0;
    for (uint8_t i=bitcounter_rxd[device]; i<(bitcounter_rxd[device]+numberOfLowBits); i++) {
      cmdindex = i/8;
      if (cmdindex<cmdlength) {
        bitClear(cmd_rxd[device][cmdindex], (i-cmdindex*8));
      }
    }
    bitcounter_rxd[device]+=numberOfLowBits;
  }

  // check if we have reached the end of transmission
  if (bitcounter_rxd[device]<cmdlength*8) {
    // prepare everything for next bits and reattach interrupt
    lastBitReadTime[device]=micros();
    #ifdef UseInterrupts
      #ifdef RemkoInvertRxD
        attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table_detecthighbits[device]), RISING);
      #else
        attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table_detecthighbits[device]), FALLING);
      #endif
    #endif
  }
}

#ifndef UseInterrupts
void remko_rxd_detectbits(uint8_t device) {
  // check for changing GPIO. Signal has 1800 baud and should be slow enough for sufficient resolution
  if (!digitalRead(RemkoRxPin[device]) && (lastPinState[device]==1)) {
    // detected a falling edge
    lastPinState[device]=0;
    #ifdef RemkoInvertRxD
      LowBitTime[device]=micros(); // set lowbit flag
    #else
      HighBitTime[device]=micros(); // set highbit flag
    #endif
  }else if (digitalRead(RemkoRxPin[device]) && (lastPinState[device]==0)) {
    // detected a rising edge
    lastPinState[device]=1;
    #ifdef RemkoInvertRxD
      HighBitTime[device]=micros(); // set highbit flag
    #else
      LowBitTime[device]=micros(); // set lowbit flag
    #endif
  }
}
#endif

void remko_rxd_step() {
  // read individual bits and evaluate received commands
  for (int device=0; device<RemkoRxDevices; device++) {
    #ifndef UseInterrupts
      remko_rxd_detectbits(device);
    #endif

    // read received high-bits
    if (HighBitTime[device]>0) {
      remko_rxd_readhighbits(device);
      HighBitTime[device]=0; // reset flag
    }
    
    // read received low-bits
    if (LowBitTime[device]>0) {
      remko_rxd_readlowbits(device);
      LowBitTime[device]=0; // reset flag
    }

    // check if all expected bits have been received
    if (bitcounter_rxd[device] >= cmdlength*8) {
      // we received a new command
      remko_rxd_processcmd(device);

      // reset to receive new commands
      remko_rxd_reset(device);
    }
  }
}
