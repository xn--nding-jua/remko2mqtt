#define BitTime 550 // microseconds
#define cmdlength 21  // bytes

uint8_t remko_powerstate[]={0,0,0,0};
uint8_t remko_opmode[]={0,0,0,0};
uint8_t remko_setpoint[]={0,0,0,0};
uint8_t remko_followme[]={0,0,0,0};

// Setup Remko-communication
static unsigned long lastBitSentTime = 0;
static unsigned long lastBitReadTime[] = {0,0,0,0};
uint8_t bitcounter_txd[] = {0,0,0,0}; // bitstream stopped
uint8_t bitcounter_rxd[] = {0,0,0,0}; // bitstream stopped
uint8_t remko_sendcmd_state[] = {0,0,0,0}; // current state of the statemachine
uint8_t remko_readcmd_state[] = {0,0,0,0}; // current state of the statemachine

// storage for the current command for each device as we support sending data in parallel
byte cmd_txd[4][21] = {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
                      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};

byte cmd_rxd[4][21] = {{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
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

// ******* Code for receiving Remko-commands ********

#if RemkoRxDevices >= 1
void ICACHE_RAM_ATTR remko_rxd_isr0() {
  // we detected a start of communication (falling edge)

  // disable interrupt
  detachInterrupt(digitalPinToInterrupt(RemkoRxPin[0]));
    
  // preload with half-bittime (=275us) to read bit in the middle
  lastBitReadTime[0]=micros()-(BitTime/2);

  // start statemachine for reading
  bitcounter_rxd[0]=0;
  remko_readcmd_state[0] = 1; // start reading command
}
#endif

#if RemkoRxDevices >= 2
void ICACHE_RAM_ATTR remko_rxd_isr1() {
  // we detected a start of communication (falling edge)

  // disable interrupt
  detachInterrupt(digitalPinToInterrupt(RemkoRxPin[1]));
    
  // preload with half-bittime (=275us) to read bit in the middle
  lastBitReadTime[1]=micros()-(BitTime/2);

  // start statemachine for reading
  bitcounter_rxd[1]=0;
  remko_readcmd_state[1] = 1; // start reading command
}
#endif

#if RemkoRxDevices >= 3
void ICACHE_RAM_ATTR remko_rxd_isr2() {
  // we detected a start of communication (falling edge)

  // disable interrupt
  detachInterrupt(digitalPinToInterrupt(RemkoRxPin[2]));
    
  // preload with half-bittime (=275us) to read bit in the middle
  lastBitReadTime[2]=micros()-(BitTime/2);

  // start statemachine for reading
  bitcounter_rxd[2]=0;
  remko_readcmd_state[2] = 1; // start reading command
}
#endif

#if RemkoRxDevices >= 4
void ICACHE_RAM_ATTR remko_rxd_isr3() {
  // we detected a start of communication (falling edge)

  // disable interrupt
  detachInterrupt(digitalPinToInterrupt(RemkoRxPin[3]));
    
  // preload with half-bittime (=275us) to read bit in the middle
  lastBitReadTime[3]=micros()-(BitTime/2);

  // start statemachine for reading
  bitcounter_rxd[3]=0;
  remko_readcmd_state[3] = 1; // start reading command
}
#endif

// create array to isr-routine to use for-loops to attach/detach isr
#if RemkoRxDevices == 4
  void (*isr_table[4])(void) = { remko_rxd_isr0, remko_rxd_isr1, remko_rxd_isr2, remko_rxd_isr3};
#elif RemkoRxDevices == 3
  void (*isr_table[4])(void) = { remko_rxd_isr0, remko_rxd_isr1, remko_rxd_isr2, 0};
#elif RemkoRxDevices == 2
  void (*isr_table[4])(void) = { remko_rxd_isr0, remko_rxd_isr1, 0, 0};
#elif RemkoRxDevices == 1
  void (*isr_table[4])(void) = { remko_rxd_isr0, 0, 0, 0};
#else
  void (*isr_table[4])(void) = { 0, 0, 0, 0};
#endif

void remko_rxd_init() {
  for (int device=0;device<RemkoRxDevices;device++){
    remko_rxd_reset(device);
  }
}

void remko_rxd_reset(uint8_t device) {
  // reattach interrupt for next command
  bitcounter_rxd[device]=0;
  remko_readcmd_state[device]=0; // reset statemachine for next command
  #ifdef RemkoInvertRxD
    attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table[device]), RISING);
  #else
    attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table[device]), FALLING);
  #endif
}

void remko_rxd_step() {
  // read individual bits and evaluate received commands
  for (int device=0; device<RemkoRxDevices; device++) {
    if (micros() - lastBitReadTime[device] >= BitTime) {
      if (remko_readcmd_state[device] == 1) {
        // statemachine is running
        lastBitReadTime[device]+=BitTime; // jump one full bit to the right for next bit
        remko_rxd_readbit(device); // read current bit
      }else if (remko_readcmd_state[device] > 1) {
        // wait 21 bytes (= ignoring second command)
        bitcounter_rxd[device]++;
  
        if (bitcounter_rxd[device]>cmdlength*8) {
          // end of second commmand
          remko_rxd_reset(device);
        }
      }
    }
    
    if (remko_readcmd_state[device] == 2) {
      // we received a new command
      remko_readcmd_state[device] = 3; // acknowledge new command, stop statemachine

      // compare cmd_rxd[device] and remko_cmd to find the received command-type
      int8_t received_cmd_index = -1;
      for (int i=0; i<17; i++) {
        if (memcmp(cmd_rxd[device], remko_cmd, cmdlength) == 0) {
          // we found a matching command
          received_cmd_index = i;
          break;
        }
      }

      switch (received_cmd_index) {
        case 0: // TurnOn
          remko_powerstate[device]=1;
          break;
        case 1: // TurnOff
          remko_powerstate[device]=0;
          break;
        case 2: // Auto
          remko_opmode[device]=0;
          break;
        case 3: // Cool
          remko_opmode[device]=1;
          break;
        case 4: // Dry
          remko_opmode[device]=2;
          break;
        case 5: // Heat
          remko_opmode[device]=3;
          break;
        case 6: // 17
          remko_setpoint[device]=17;
          break;
        case 7: // 18
          remko_setpoint[device]=18;
          break;
        case 8: // 19
          remko_setpoint[device]=19;
          break;
        case 9: // 20
          remko_setpoint[device]=20;
          break;
        case 10: // 21
          remko_setpoint[device]=21;
          break;
        case 11: // 22
          remko_setpoint[device]=22;
          break;
        case 12: // 23
          remko_setpoint[device]=23;
          break;
        case 13: // 24
          remko_setpoint[device]=24;
          break;
        case 14: // FollowMe on
          remko_followme[device]=1;
          break;
        case 15: // FollowMe off
          remko_followme[device]=0;
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
    }
  }
}

void remko_rxd_readbit(uint8_t device) {
  // read current bit from GPIO

  if (bitcounter_rxd[device]<cmdlength*8) {
    // read current bit and write it to cmd
    #ifdef RemkoInvertRxD
      bitWrite(cmd_rxd[device][bitcounter_rxd[device]/8], (bitcounter_rxd[device]-(bitcounter_rxd[device]/8)*8), !digitalRead(RemkoRxPin[device]));
    #else
      bitWrite(cmd_rxd[device][bitcounter_rxd[device]/8], (bitcounter_rxd[device]-(bitcounter_rxd[device]/8)*8), digitalRead(RemkoRxPin[device]));
    #endif
    // check for first two bytes. Should be 0x00 and 0xFF
    if (((bitcounter_rxd[device]==7) && (cmd_rxd[device][0]!=0)) || ((bitcounter_rxd[device]==15) && (cmd_rxd[device][1]!=255))) {
      // unexpected first or second byte
      remko_rxd_reset(device);
      return;
    }

    // increase bitcounter_rxd[device] for next bit    
    bitcounter_rxd[device]++;
  }else{
    // we reached end of the current command
    remko_readcmd_state[device]=2;
    bitcounter_rxd[device]=0;
  }
}
