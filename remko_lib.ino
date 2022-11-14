#define TxBitTime 550 // microseconds
#define RxBitTime 550 // microseconds
#define cmdlength 21  // bytes
//#define UseInterrupts // use interrupts for detecting the edges of the signal (Caution: not working well with WiFi-module)

struct remko_state {
  uint8_t powerState;
  uint8_t opMode;
  uint8_t setPoint;
  uint8_t followMe;
};
struct remko_state remko_states[4] = {{.powerState=0, .opMode=0, .setPoint=0, .followMe=0},
                               {.powerState=0, .opMode=0, .setPoint=0, .followMe=0}, 
                               {.powerState=0, .opMode=0, .setPoint=0, .followMe=0},
                               {.powerState=0, .opMode=0, .setPoint=0, .followMe=0}};
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
const byte remko_cmd[NumberOfSupportedCmds][cmdlength] = {{0x0, 0xFF, 0xAE, 0xBB, 0xBA, 0xBA, 0xBA, 0xEB, 0xAE, 0xBB, 0xBB, 0xBB, 0xAE, 0xAA, 0xEE, 0xAE, 0xAE, 0xAA, 0xBB, 0xEE, 0xFE}, //turn on
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

void remko_publishmqtt() {
  #if RemkoTxDevices >= 1
    mqttclient.publish(mqtt_topic_dev0_powerstate, String(remko_states[0].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev0_opmode, String(remko_states[0].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev0_setpoint, String(remko_states[0].setPoint).c_str());  
    mqttclient.publish(mqtt_topic_dev0_followme, String(remko_states[0].followMe).c_str());  
  #endif
  #if RemkoTxDevices >= 2
    mqttclient.publish(mqtt_topic_dev1_powerstate, String(remko_states[1].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev1_opmode, String(remko_states[1].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev1_setpoint, String(remko_states[1].setPoint).c_str());  
    mqttclient.publish(mqtt_topic_dev1_followme, String(remko_states[1].followMe).c_str());  
  #endif
  #if RemkoTxDevices >= 3
    mqttclient.publish(mqtt_topic_dev2_powerstate, String(remko_states[2].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev2_opmode, String(remko_states[2].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev2_setpoint, String(remko_states[2].setPoint).c_str());  
    mqttclient.publish(mqtt_topic_dev2_followme, String(remko_states[2].followMe).c_str());  
  #endif
  #if RemkoTxDevices >= 4
    mqttclient.publish(mqtt_topic_dev3_powerstate, String(remko_states[3].powerState).c_str());  
    mqttclient.publish(mqtt_topic_dev3_opmode, String(remko_states[3].opMode).c_str());  
    mqttclient.publish(mqtt_topic_dev3_setpoint, String(remko_states[3].setPoint).c_str());  
    mqttclient.publish(mqtt_topic_dev3_followme, String(remko_states[3].followMe).c_str());  
  #endif
}

// ******* Code for sending Remko-commands ********

void remko_txd_init() {
  for (int device=0;device<RemkoTxDevices;device++){
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

void remko_txd_sendcmd(uint8_t device, uint8_t cmdtype, uint8_t value) {
  if (remko_sendcmd_state[device]>0) {
    // a command is sending at the moment
    // as we have no command-queue we have to reject this command
    return;
  }
  
  // load command
  switch (cmdtype) {
    case 0: // powerstate
      if (value==1) {
        memcpy(cmd_txd[device], remko_cmd[0], cmdlength);
        remko_states[device].powerState=1;
      }else{
        memcpy(cmd_txd[device], remko_cmd[1], cmdlength);
        remko_states[device].powerState=0;
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
      remko_states[device].opMode=value;
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
      remko_states[device].setPoint=value;
      break;
    case 3: // followme
      if (value==1) {
        memcpy(cmd_txd[device], remko_cmd[14], cmdlength);
        remko_states[device].followMe=1;
      }else{
        memcpy(cmd_txd[device], remko_cmd[15], cmdlength);
        remko_states[device].followMe=0;
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

boolean remko_rxd_processcmd(uint8_t device) {
  // check header to be 0x0 0xFF 0xAE 0xBB
  if ((cmd_rxd[device][0] != 0x00) || (cmd_rxd[device][1] != 0xFF) || (cmd_rxd[device][2] != 0xAE) || (cmd_rxd[device][3] != 0xBB)) {
    // at least one byte of the header has unexpected content -> abort processing the command
    return false;
  }
  
  // compare cmd_rxd[device] and remko_cmd to find the received command-type
  int8_t received_cmd_index = -1;
  for (int i=0; i<NumberOfSupportedCmds; i++) {
    if (memcmp(cmd_rxd[device], remko_cmd, cmdlength) == 0) {
      // we found a matching command
      received_cmd_index = i;
      break;
    }
  }
  
  switch (received_cmd_index) {
    case 0: // TurnOn
      remko_states[device].powerState=1;
      break;
    case 1: // TurnOff
      remko_states[device].powerState=0;
      break;
    case 2: // Auto
      remko_states[device].opMode=0;
      break;
    case 3: // Cool
      remko_states[device].opMode=1;
      break;
    case 4: // Dry
      remko_states[device].opMode=2;
      break;
    case 5: // Heat
      remko_states[device].opMode=3;
      break;
    case 6: // 17
      remko_states[device].setPoint=17;
      break;
    case 7: // 18
      remko_states[device].setPoint=18;
      break;
    case 8: // 19
      remko_states[device].setPoint=19;
      break;
    case 9: // 20
      remko_states[device].setPoint=20;
      break;
    case 10: // 21
      remko_states[device].setPoint=21;
      break;
    case 11: // 22
      remko_states[device].setPoint=22;
      break;
    case 12: // 23
      remko_states[device].setPoint=23;
      break;
    case 13: // 24
      remko_states[device].setPoint=24;
      break;
    case 14: // FollowMe on
      remko_states[device].followMe=1;
      break;
    case 15: // FollowMe off
      remko_states[device].followMe=0;
      break;
    case 16: // Toggle LED
      break;
    default: // unknown command
      break;
  }
  
  // send the new command to output, if desired
  #ifdef MirrorRxD2TxD
    // copy the received command to the output-buffer
    // this will work with unknown commands, too
    memcpy(cmd_txd[device], cmd_rxd[device], cmdlength);
    // start bitstream-transmission
    bitcounter_txd[device] = 0;
    remko_sendcmd_state[device] = 1;
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
