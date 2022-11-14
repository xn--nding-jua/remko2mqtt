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
