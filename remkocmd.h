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
