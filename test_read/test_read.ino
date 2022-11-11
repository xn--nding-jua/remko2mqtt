static const unsigned long BIT_TIME = 550; // microseconds
static unsigned long lastBitRefreshTime = 0;
uint8_t bitcounter = 0; // bitstream stopped
uint8_t remko_cmd_state = 0; // current state of the statemachine
#define cmdlength 21

byte cmd_a[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
byte cmd_b[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

void ICACHE_RAM_ATTR GPIO_D1_ISR() {
  // we detected a start of communication (falling edge)

  // disable interrupt
  detachInterrupt(digitalPinToInterrupt(D1));
  lastBitRefreshTime=micros()+(BIT_TIME/2); // preload with half-bittime (=275us) to read bit in the middle
  // start statemachine for reading
  remko_cmd_state = 1;
}

void remko_comm_readbit() {
  // read current bit from GPIO
  
  if (bitcounter<cmdlength*8) {
    // read current bit and write it to cmd
    if (remko_cmd_state==1) {
      // read first command
      bitWrite(cmd_a[bitcounter/8], (bitcounter-(bitcounter/8)*8), digitalRead(D1));
    }else{
      // read second command
      bitWrite(cmd_b[bitcounter/8], (bitcounter-(bitcounter/8)*8), digitalRead(D1));
    }
    bitcounter+=1;
  }else{
    // we reached end of the current command

    if (remko_cmd_state==1) {
      remko_cmd_state=2; // set statemachine to next command
    }else{
      remko_cmd_state=3; // stop statemachine and indicate, that we received two commands
    }
    
    bitcounter=0; // reset bitcounter
    // reattach interrupt for next command
    attachInterrupt(digitalPinToInterrupt(D1), GPIO_D1_ISR, FALLING);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(D1, INPUT);
  attachInterrupt(digitalPinToInterrupt(D1), GPIO_D1_ISR, FALLING);
}

void loop() {
  if ((remko_cmd_state>0) && (micros() - lastBitRefreshTime >= BIT_TIME)) {
    // statemachine is running
    lastBitRefreshTime+=BIT_TIME; // jump one full bit to the right for next bit
    remko_comm_readbit(); // read current bit
  }

  if (remko_cmd_state==3) {
    // we received a new command

	remko_cmd_state=0; // reset statemachine for next command

    Serial.println("We received some data:");
    Serial.println("CMD_A:");
    for (int i=0; i<cmdlength; i++) {
      Serial.write(cmd_a[i]);
    }
    Serial.println("CMD_B:");
    for (int i=0; i<cmdlength; i++) {
      Serial.write(cmd_b[i]);
    }
    Serial.println("Done");
  }
}
