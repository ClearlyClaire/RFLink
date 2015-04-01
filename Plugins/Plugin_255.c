//#######################################################################################################
//#################################### Plugin-255: MyPlugin   ############################################
//#######################################################################################################

/*********************************************************************************************\
* Functional description: This is an empty plugin to be used as a sample for your own plugin development.
*                         When you create a new plugin that can be useful for others, then please share it.
* 
* <Write a description here describing how the plugin works> 
*
* Author             : <name and email of the developer>
* Support            : http://sourceforge.net/projects/rflink/
* License            : This code is free for use in any open source project when this header is included.
*                      Usage of any parts of this code in a commercial application is prohibited!
***********************************************************************************************
* Technical description:
*
* External functions : <list here any external fuctions that might be used> 
*
* <Put a technical description here explaining how your plugin works and include protocol details>
* 
* Tips en remarks for developing a plugin:
* -  Memory is limited. Write your code compact and use every free RAM and PROGMEM wisely. Remember, Arduino is not a PC.
* -  Dont use long waitingloops as this could cause serious timing issues.
*    The code is time critical, try to detect quickly if your plugin needs to be processing a signal and if not, exit as soon as possible.
* -  Do not make Om uitwisselbaar te blijven met andere Nodo versies en Nodo gebruikers, geen aanpassingen aan de Nodo code maken.
* -  Do not use interrupt driven routines, this might have negative side effects on I2C, Serial and ethernet communications.
* -  Only use Globals as an exception en do not change values of globals in the base code.
* -  Put comments in your code to make clear what is happening.
*
* Plugin examples:
* - Digital temperature sensors (Like Dallas DS18B20)
* - Digital humidity sensores (Like DTH-11)
* - Measure 8 different analog inputs with 1 WiredIn using a LTC1380.
* - WiredOut output increase to 8, 16, 32, 64 different digital outputs using a 74HCT595
* - I2C plugin control via the Arduino SLC and SDA lines.
* - anything else you can think of.
*
* Using the Wired ports and additional IO ports using their name as defined in the code:
* PIN_WIRED_OUT_n = WiredOut port, n = port number starting at 1 
* PIN_WIRED_IN_n  = WiredOut port, n = port number starting at 1 
* PIN_IO_n        = Additional IO-port, n = port number starting at 1 
* For Arduino pinout numbers: see schematic or the declaration tab "Base"
* Do not use any other ports as they might have a specific meaning in future versions.
*
* When you are done with developing your plugin, then clean up the code to save memory and have maximal performance. 
\*********************************************************************************************/
// Each plugin has a unique ID. The plugin numbers are maintained by the project team. 
// When you have created a custom plugin, please share it! 

// Plugin 255 can be used as a sample to create your own plugin or at least to understand the usage.
// You can also copy  an existing plugin to a new plugin number and devlop from that.
// Keep in mind that when you create a new plugin, you have to include the plugin in plugin_01.c so that it is also 
// added whe nthe project is compiled. 
#define PLUGIN_NAME "MyPlugin"

#define PLUGIN_ID   255

// For clarity the plugin number is used in the function name 
boolean Plugin_255(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  // The plugin code is called from multiple places in the main code. 
  // The purpose of the call is for various reasons as listed below:
  //
  // PLUGIN_RAWSIGNAL_IN  => Processing of IR/RF signals
  // PLUGIN_COMMAND       => Command for executing an event
  // PLUGIN_MMI_IN        => Translate input from user/script to events
  // PLUGIN_MMI_OUT       => Translate event to user readable text
  // DEVIDE_ONCE_A_SECOND => ~ every second
  // PLUGIN_INIT          => One time only, after reboot
  // PLUGIN_EVENT_IN      => Right before processing of an incoming event
  // PLUGIN_SERIAL_IN     => When bytes from the seriele poort have been received
  // PLUGIN_ETHERNET_IN   => When bytes from the ethernet poort have been received
  
  switch(function)
    {    
    #ifdef PLUGIN_255_CORE

    case PLUGIN_INIT:
      {
      // This code is called only once after a reboot and before any event struct is processed.
      // if you do not have any initialization tasks, then leave out this case.
      Serial.println(F("*** debug: MyPlugin: PLUGIN_INIT")); //??? Debug
      break;
      }

    case PLUGIN_EVENT_IN:
      {
      // When an even is started, this case is called. This makes it possible to intercept
      // an event and do some alternative processing. 
      // 
      // Events can be originating from RF, IR, Serial, TelNet, HTTP, CLOCK, WIRED 
      // Only external events end up here. 
      // 
      // Event contains the data that is going to be processed by the main code. 
      // The data can be changed at will to change the handling of such an external event.
      // When event->Command is filled with 0, then event processing is terminated.  
      // 
      // When you do not use this function, then it is preferred to completely remove this case.
      Serial.println(F("*** debug: MyPlugin: PLUGIN_EVENT_IN"));
      break;      
      }

    case PLUGIN_ONCE_A_SECOND:
      {
      // This part of the code is approximately !! called once per second. 
      // This part of the plugin is, when called, part of a time critical routine and part of th emain loop.
      // Use it wisely to quickly reset a variable or so. You should not use long routines here.
      // As long as code within this case is executed, NO IR or RF events are received!
      // Event does not contain any valid data. You can fill the event struct though.
      // When at return success=true, then the modified data is processed as a regular event.
      // 
      // When you do not use this function, then it is preferred to completely remove this case.
      // Serial.println(F("*** debug: MyPlugin: PLUGIN_ONCE_A_SECOND"));
      break;
      }

    case PLUGIN_RAWSIGNAL_IN:
      {
      // Code in this case is executed whenever data comes in via RF or IR
      // The RawSignal buffer is filled with the detected pulses. 
      // -  The array RawSignal.Pulses[] contains all Mark and Space times in microseconden. starting at element [1].
      //    The array is filled when we get here. Element [0] contains a multiplication factor to recalculate the real microseconds.
      // -  RawSignal.Pulses contains the number of pulses. 
      // Element [1] contains the 'mark' of the start bit.
      //
      // Note: Make sure to not cause overflows on RAW_BUFFER_SIZE, otherwise there will for sure be crashes.
      // 
      // When you do not use this function, then it is preferred to completely remove this case.
      Serial.println(F("*** debug: MyPlugin: PLUGIN_RAWSIGNAL_IN"));
      break;
      }
      
    case PLUGIN_COMMAND:
      {
      // When the code gets here, the struct [event] is filled and ready to be processed.
      // When at exit the struct [event] is filled with another event, then this new event will be executed as a new event.
      // This can be useful when a variable is set and the value has to be porocessed further.
      // When you do not use this function, then it is preferred to completely remove this case.
      Serial.print(F("*** debug: MyPlugin: PLUGIN_COMMAND"));
      Serial.print(", Par1=");    Serial.print(event->Par1);
      Serial.print(", Par2=");    Serial.println(event->Par2);
      
      // Als a sample, variabele 5 is filled with 123.45
      byte Variable = 5;
      float Value   = 123.45;

      ClearEvent(event);                                      // Start with a default "clean" event. This removes any old data that might be in the struct   
      event->Type         = NODO_TYPE_COMMAND;                // The event is a command to be processed
      event->Command      = CMD_VARIABLE_SET;                 // Command "VariableSet"
      event->Par1         = Variable;                         // Par1 carries the variablenumber
      event->Par2         = float2ul(Value);                  // Par2 float2ul() puts a value in the wanted format.
      
      success=true;                                           // When true, a new event has been put in the event struct and is ready to be processed. 
                                                              // Note: a plugin can not call another plugin. There are No other limitations 
                                                              // and all events/commands can be used and processed.
      break;
      }      

    case PLUGIN_SERIAL_IN:
      {
      // As soon as there is any character coming in via the serial port, this part of the plugin is called.
      // This function can be used to process special serial information.
      // 
      // Both "event" and "string" have an invalid value en can not be used.
      // 
      // Note that the character is only available but not read yet. Reading and processing any characters should be done here.
      // Also note that when data has been pulled from the serial port via this function, it can not be read again via the normal
      // parts of this software.       
      // 
      // When you do not use this function, then it is preferred to completely remove this case.
      // From PLUGIN_INIT it is possible to reinitialize the baudrate.
      // The PLUGIN_SERIAL_IN can ONLY be used in 1 plugin. 
      
      Serial.println(F("*** debug: MyPlugin: PLUGIN_SERIAL_IN"));

      // With the sample below the input can be pulled in. 
      // In this sample the code is commented out as it would normally not be used in regular plugins and it could influence the entire program.
      // 
      // while(Serial.available())
      //   {
      //   Serial.print(F("Received="));
      //   Serial.println(Serial.read());
      //   }                

      break;      
      }
    #endif // CORE
    
    #if NODO_MEGA // Only relevant for a Nodo Mega as the Small does not support MMI
    case PLUGIN_MMI_IN:
      {
      Serial.print(F("*** debug: MyPlugin: PLUGIN_MMI_IN, string="));Serial.println(string); // Debug
      
      // When a command is supplied, this part of the code is used to parse the command and translate parameters into variables in the Nodo event struct
      // When parsing was successful, success should be set to true so that other parts of the Nodo code can take care of processing the command.      

      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      // With PLUGIN_MMI_IN the user input (string) is translated to a Nodo Event so that it can be handled internally.
      // At this point, "string" contains the complete command.
      
      if(GetArgv(string,TempStr,1)) {               // Check first argument, make sure it matches the plugin name.
        if(strcasecmp(TempStr,PLUGIN_NAME)==0) {    // Check if the command/event name matches the name of this plugin
          if(GetArgv(string,TempStr,2)) {           // Check second argument
            // Put your code to parse the first parameter here. 
            // The byte "Par1" en unsigned long "Par2" that are in the struct [event] can be used.
            if(GetArgv(string,TempStr,3)) {         // If wanted a third argument can be used (=second parameter)
              // Put your code to parse the second parameter here. 
              // When the last parameter is parsed, then fill the variable [success] with true 
              // so that further treatment of the event can take place.

              // Plugins can be treated as a command or an event. 
              event->Type    = NODO_TYPE_PLUGIN_COMMAND; // Use event->Type to let other parts know how this event/command has to be treated.
              event->Command = 255;                 // number of this plugin
              success=true;                         // If success is filled with true, then the command/event is accepted as valid.
              }
            }
          }
        }
      free(TempStr);
      break;
      }

    case PLUGIN_MMI_OUT:
      {
      // The code here turns an event with a unique ID in the struct [event] into a readable event for the user.
      // The result has to be placed in the string [string] 
      // Note that the total command length is not more than 25 characters.
      Serial.println(F("*** debug: MyPlugin: PLUGIN_MMI_OUT")); // Debug

      strcpy(string,PLUGIN_NAME);                   // Command 
      strcat(string," ");
      strcat(string,int2str(event->Par1));          // Parameter-1 (8-bit)
      strcat(string,",");
      strcat(string,int2str(event->Par2));          // Parameter-2 (32-bit)

      break;
      }
    #endif //MMI
    }
    
  return success;
  }
