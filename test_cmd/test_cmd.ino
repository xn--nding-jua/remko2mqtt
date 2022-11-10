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
byte cmd[] =                   {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

void setup() {
  Serial.begin(115200);
  pinMode(D4, OUTPUT);
}

static const unsigned long REFRESH_INTERVAL = 1000; // ms
static unsigned long lastRefreshTime = 0;

static const unsigned long BIT_TIME = 550; // microseconds
static unsigned long lastBitRefreshTime = 0;
uint8_t bitcounter = 0; // bitstream stopped
uint8_t remko_cmd_pass = 0; // bitstream stopped

void DoBitStreamStep() {
  if (remko_cmd_pass>0) {
    if (bitcounter<sizeof(cmd)*8) {
      digitalWrite(D4, bitRead(cmd[bitcounter/8], bitcounter-(bitcounter/8)*8));
      bitcounter+=1;
    }else{
      // we reached end of transmission
      digitalWrite(D4, HIGH);
      delayMicroseconds(BIT_TIME); // give last bit time

      if (remko_cmd_pass==1) {
        // send same command second time
        delayMicroseconds(BIT_TIME); // add additional bit
        remko_cmd_pass=2;
        bitcounter=0;
      }else{
        // stop transmission
        remko_cmd_pass=0;
        bitcounter=0;
      }
    }
  }
}

void SendRemkoCmd(uint8_t CmdType) {
  // load command
  switch (CmdType) {
    case 0:
      memcpy(cmd, cmd_turnon, sizeof(cmd));
      break;
    case 1:
      memcpy(cmd, cmd_turnoff, sizeof(cmd));
      break;
    case 2:
      memcpy(cmd, cmd_auto, sizeof(cmd));
      break;
    case 3:
      memcpy(cmd, cmd_cool, sizeof(cmd));
      break;
    case 4:
      memcpy(cmd, cmd_dry, sizeof(cmd));
      break;
    case 5:
      memcpy(cmd, cmd_heat, sizeof(cmd));
      break;
    case 6:
      memcpy(cmd, cmd_17, sizeof(cmd));
      break;
    case 7:
      memcpy(cmd, cmd_18, sizeof(cmd));
      break;
    case 8:
      memcpy(cmd, cmd_19, sizeof(cmd));
      break;
    case 9:
      memcpy(cmd, cmd_20, sizeof(cmd));
      break;
    case 10:
      memcpy(cmd, cmd_21, sizeof(cmd));
      break;
    case 11:
      memcpy(cmd, cmd_22, sizeof(cmd));
      break;
    case 12:
      memcpy(cmd, cmd_23, sizeof(cmd));
      break;
    case 13:
      memcpy(cmd, cmd_24, sizeof(cmd));
      break;
    case 14:
      memcpy(cmd, cmd_followmeon, sizeof(cmd));
      break;
    case 15:
      memcpy(cmd, cmd_followmeoff, sizeof(cmd));
      break;
    case 16:
      memcpy(cmd, cmd_led, sizeof(cmd));
      break;
  }

  // start bitstream-transmission
  bitcounter = 0;
  remko_cmd_pass = 1;
}

void loop() {
  if (micros() - lastBitRefreshTime >= BIT_TIME) {
    lastBitRefreshTime+=BIT_TIME;
    DoBitStreamStep();
  }
  
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
  {
    lastRefreshTime += REFRESH_INTERVAL;
    //SendRemkoCmd(0); // send cmd every 1s
  }

  while (Serial.available() > 0) {
    int desired_cmd = Serial.parseInt();
    if (Serial.read() == '\n') {
      SendRemkoCmd(desired_cmd);
    }
  }
}