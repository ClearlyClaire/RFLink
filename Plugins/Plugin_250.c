//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                              Plugin-250: Slave Nodo Translation plugin                            ##
//#######################################################################################################
/*********************************************************************************************\
* Description: This plugin translates data received from slave nodo sensors to protocol packets
*              Supported are: temperature/humidity/UV and Barometric pressure slave nodo sensors.
*              In order to use this plugin your slave nodo's MUST use the correct variable numbers listed below.
*              Variable 11, 12 and 13 : Temperature sensor.
*              Variable 14            : Humidity.
*              Variable 15            : UV index (translated to 1-100%)
*              Variable 16            : Barometric pressure
* 
* Note: the Nodo slave number is used as sensor ID. So you can use as many slave nodo's with for example 
* humidity as you like. Just make sure each Nodo has a unique ID.
*
* No action is needed with this plugin. Just compile and make sure you use the correct variable numbers.
* Je hoeft niets te doen met deze plugin, gewoon mee compileren
* Uiteraard wel er voor zorgen dat de Nodo Slaves de correcte variable nummers gebruiken voor het type sensor dat je hebt
*
* Author             : R. ten Klooster
* Support            : http://sourceforge.net/projects/rflink/
* License            : This code is free for use in any open source project when this header is included.
*                      Usage of any parts of this code in a commercial application is prohibited!
*********************************************************************************************
* Changelog: v1.0 initial release
*********************************************************************************************
* Technische beschrijving:
*
* Compiled size      : x
* Externe funkties   : float2ul 
*
*/
#define PLUGIN_NAME "MyPlugin"
#define PLUGIN_ID   250

boolean Plugin_250(byte function, struct NodoEventStruct *event, char *string) {
  boolean success=false;    
  char buffer[12]="";    
  switch(function) {    
    #ifdef PLUGIN_250_CORE
    case PLUGIN_EVENT_IN:
      {
      // Event Command's kunnen het volgende zijn:   1: boot 2: sound 3: userevent 4: variable  
      // Variabele 11-13 : Temperatuur
      // Variabele 14 : Luchtvochtigheid   0 - 100
      // Variabele 15 : Lichtsterkte       0 - 1024 dus mappen naar 1-100 
      // Variabele 16 : Barometric pressure 
      if ((event->Command == 4) && ((event->Par1 > 10)  && (event->Par1 < 14)) && (event->Type == 1)){  
         // Variable 11 12 or 13 : emulate temperature sensor
         int temperature = 10 * ul2float(event->Par2);
         if (temperature <= 0) temperature=-temperature | 0x8000; // set high bit for negative temperatures
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( buffer );
         // ----------------------------------
         Serial.print("Slave;");                          // Label
         sprintf(buffer, "ID=%02x%02x;", event->Par1, event->SourceUnit); // ID    
         Serial.print( buffer );
         sprintf(buffer, "TEMP=%04x;", temperature);      // 2 byte value
         Serial.print( buffer );
         Serial.println();
         //==================================================================================
         success=true;
         ClearEvent(event);
      } else 
      if ((event->Command == 4) && (event->Par1 == 14) && (event->Type == 1)){  
         // Variable 14 : emulate humidity sensor
         int humidity = ul2float(event->Par2) + 0.5;      // add 0.5 to make sure it's rounded the way it should and assign as integer to remove decimal value 
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( buffer );
         // ----------------------------------
         Serial.print("Slave;");                          // Label
         sprintf(buffer, "ID=%02x%02x;", event->Par1, event->SourceUnit); // ID    
         Serial.print( buffer );
         sprintf(buffer, "HUM=%02d;", humidity);     
         Serial.print( buffer );
         Serial.println();
         //==================================================================================
         success=true;
         ClearEvent(event);
      }
      if ((event->Command == 4) && (event->Par1 == 15)){  
         // Variable 15 : UV sensor
         int light = ul2float(event->Par2);               // supplied is value between 0 and 1024
         light = map(light, 0,1024,1,100);                // Map value to 1 - 100
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( buffer );
         // ----------------------------------
         Serial.print("Slave;");                          // Label
         sprintf(buffer, "ID=%02x%02x;", event->Par1, event->SourceUnit); // ID    
         Serial.print( buffer );
         sprintf(buffer, "UV=%04x;", light);              // 2 byte value
         Serial.print( buffer );
         Serial.println();
         //==================================================================================
         success=true;
         ClearEvent(event);
      }  
      if ((event->Command == 4) && (event->Par1 == 16)){  
         // Variable 16 : Barometric pressure sensor
         int baro = ul2float(event->Par2);                    
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( buffer );
         // ----------------------------------
         Serial.print("Slave;");                          // Label
         sprintf(buffer, "ID=%02x%02x;", event->Par1, event->SourceUnit); // ID    
         Serial.print( buffer );
         sprintf(buffer, "BARO=%04x;", baro);             // 2 byte value
         Serial.print( buffer );
         Serial.println();
         //==================================================================================
         success=true;
         ClearEvent(event);
      }  
      break;      
      }
      #endif // CORE
    }
  return success;
}
