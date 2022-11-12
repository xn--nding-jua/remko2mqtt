#define InvertCommunication 1
#define BIT_TIME 550 // microseconds
#define cmdlength 21 // bytes

#define RemkoRxDevices 1 // define the numbers of devices you want to read (0...4)
uint8_t RemkoRxPin[] = {D1, 10, 0, 15}; // Enter the desired GPIO pins in this array

static unsigned long lastBitReadTime[] = {0,0,0,0};
uint8_t bitcounter_rxd[] = {0,0,0,0}; // bitstream stopped
uint8_t remko_readcmd_state[] = {0,0,0,0}; // current state of the statemachine

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

#if RemkoRxDevices >= 1
void ICACHE_RAM_ATTR remko_rxd_isr0() {
  // we detected a start of communication (falling edge)

  // disable interrupt
  detachInterrupt(digitalPinToInterrupt(RemkoRxPin[0]));
    
  // preload with half-bittime (=275us) to read bit in the middle
  lastBitReadTime[0]=micros()-(BIT_TIME/2);

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
  lastBitReadTime[1]=micros()-(BIT_TIME/2);

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
  lastBitReadTime[2]=micros()-(BIT_TIME/2);

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
  lastBitReadTime[3]=micros()-(BIT_TIME/2);

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

void remko_rxd_readbit(uint8_t device) {
  // read current bit from GPIO

  if (bitcounter_rxd[device]<cmdlength*8) {
    // read current bit and write it to cmd
    #ifdef InvertCommunication
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

void remko_txd_init() {
  for (int device=0;device<RemkoRxDevices;device++){
    remko_rxd_reset(device);
  }
}

void remko_rxd_reset(uint8_t device) {
  // reattach interrupt for next command
  bitcounter_rxd[device]=0;
  remko_readcmd_state[device]=0; // reset statemachine for next command
  #ifdef InvertCommunication
    attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table[device]), RISING);
  #else
    attachInterrupt(digitalPinToInterrupt(RemkoRxPin[device]), (*isr_table[device]), FALLING);
  #endif
}

void remko_txd_step() {
  for (int device=0; device<RemkoRxDevices; device++) {  
    if (micros() - lastBitReadTime[device] >= BIT_TIME) {
      if (remko_readcmd_state[device] == 1) {
        // statemachine is running
        lastBitReadTime[device]+=BIT_TIME; // jump one full bit to the right for next bit
        remko_rxd_readbit(device); // read current bit
      }else if (remko_readcmd_state[device] > 1) {
        // wait 21 bytes
        bitcounter_rxd[device]++;
  
        if (bitcounter_rxd[device]>cmdlength*8) {
          // end of second commmand
          remko_rxd_reset(device);
        }
      }
    }
    
    if (remko_readcmd_state[device] == 2) {
      remko_readcmd_state[device] = 3;

      // we received a new command

      // compare cmd_rxd[device] to remko_cmd to find the received command-type
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
          Serial.println("TurnOn");
          break;
        case 1: // TurnOff
          break;
        case 2: // Auto
          break;
        case 3: // Cool
          break;
        case 4: // Dry
          break;
        case 5: // Heat
          break;
        case 6: // 17
          break;
        case 7: // 18
          break;
        case 8: // 19
          break;
        case 9: // 20
          break;
        case 10: // 21
          break;
        case 11: // 22
          break;
        case 12: // 23
          break;
        case 13: // 24
          break;
        case 14: // FollowMe on
          break;
        case 15: // FollowMe off
          break;
        case 16: // Toggle LED
          break;
        default:
          // we received an unknown command
          Serial.println("Unkown command");
          for (int i=0; i<cmdlength; i++) {
            Serial.print(cmd_rxd[device][i], HEX);
            Serial.print(" ");
          }
          Serial.println("");
          break;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  remko_txd_init();
}

void loop() {
  remko_txd_step();
}
