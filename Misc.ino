/*********************************************************************************************\
 * Convert HEX or DEC tring to unsigned long HEX, DEC
\*********************************************************************************************/
unsigned long str2int(char *string) {
  return(strtoul(string,NULL,0));  
}
/*********************************************************************************************\
 * Convert string to command code
\*********************************************************************************************/
int str2cmd(char *command) {
    if(strcasecmp(command,"ON") == 0) return VALUE_ON;
    if(strcasecmp(command,"OFF") == 0) return VALUE_OFF;
    if(strcasecmp(command,"ALLON") == 0) return VALUE_ALLON;
    if(strcasecmp(command,"ALLOFF") == 0) return VALUE_ALLOFF;
    return false;
}
/********************************************************************************************\
 * Convert unsigned long to float long through memory
\*********************************************************************************************/
float ul2float(unsigned long ul) {
    float f;
    memcpy(&f, &ul,4);
    return f;
}
/*********************************************************************************************/
void PrintHex8(uint8_t *data, uint8_t length) { // prints 8-bit data in hex
  char tmp[length*2+1];
  byte first ;
  int j=0;
  for (uint8_t i=0; i<length; i++) {
    first = (data[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    j++;

    first = (data[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39; 
    else tmp[j] = first;
    j++;
  }
  tmp[length*2] = 0;
  Serial.print(tmp);
}
/*********************************************************************************************/

