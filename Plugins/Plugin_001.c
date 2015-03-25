//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                          Plugin-01: Long Packet Translation and debugging                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of long packets that actually contain multiple RF packets 
 * Usually caused by a very short delay between RF packets 
 * 
 * Auteur             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Versie             : 18-01-2015, Versie 1.0, P.K.Tonkes: Eerste versie
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Technical information:
 *
 * The first part of this plugin outputs all the received pulses from RF packets, use this plugin to 
 * find signals from new devices. Even if you do not know what to do with the data yourself you might 
 * want to share your data so others can analyse it and add support for missong protocols.
 * 
 * The second part of this plugin takes care of oversized RF packets that might need to be cut into 
 * smaller RF packets. Usually the oversized packets are due to a very short delay between re-transmits
 * so that multiple packets are seen as one long packet
 *
 * ELRO HOMEEASY HE842
 * The HE842 remote control sends 3 different protocols.
 * 8 x Method 1 - 28 bit code (58 pulses)
 * 1 x method 2 - HE compatible (116 pulses) See Plugin 15 & 16.
 * -------------------
 * ELRO FLAMINGO FA500
 * The FA500R remote control sends 3 different protocols.
 * 4 x Method 1 - 28 bit code (58 pulses)
 * 6 x method 2 - AC compatible (130 pulses)
 * 5 x method 3 - 24/12 bit code (24 pulses)
 * It appears that the FA500S switch is only capable to react to the first method.
 * Nodo can only distinguish the 3rd method properly. Method 1 and 2 have to be pulled apart first.
 * This plugin will split the received signal so that method 1 or 2 can be used.
 * In this case we force method 1 so that the remote control can be used with the FA500 switches. 
 * Method 3: 20;60;DEBUG;Pulses=24;Pulses(uSec)=325,800,275,800,825,225,275,800,275,825,275,800,825,225,275,800,825,225,275,800,275,800,275;
 * 20;1F;DEBUG;Pulses=364;Pulses(uSec)=225,4650,200,875,200,875,800,250,200,875,200,875,800,250,200,875,200,875,825,250,825,250,200,875,825,250,200,875,200,875,825,250,800,250,200,875,200,875,200,875,800,250,800,225,200,875,200,875,825,250,825,250,200,875,800,250,200,875,200,4650,200,875,200,875,825,250,200,875,200,875,800,250,200,875,200,875,825,250,800,250,200,875,800,250,200,875,200,875,825,225,825,250,200,875,200,875,200,875,825,250,800,250,200,875,200,875,825,250,800,225,200,875,800,250,200,875,200,4650,200,875,200,875,800,250,200,875,200,875,825,250,200,875,200,875,800,250,825,250,200,875,800,250,200,875,200,875,800,225,825,250,200,875,200,875,200,875,800,250,800,250,200,875,200,875,800,250,825,225,200,875,800,250,200,875,200,4650,200,875,200,875,800,250,200,875,200,875,800,250,200,875,200,875,825,250,800,250,200,875,825,250,200,875,200,875,800,225,800,250,200,875,200,875,200,875,800,250,825,250,200,875,200,875,800,250,800,250,200,875,825,250,200,875,150,2550,150,175,150,1200,125,175,150,1200,125,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,150,1200,150,1175,150,200,150,175,150,1200,150,175,150,1175,150,175,150,1200,150,1175,150,200,150,1175,150,200,150,175,150,1175,150,1175,150,200,150,1175,150,200,125,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,150,1175,150,175,125,1200,150,175,125,1200,150,175,150,1200,150,175,150,1200,150,175,150,1200,150,175,150,1200,150,175,150,1200,150,1175,150,175,150;
 * -------------------
 * BYRON & LIDL doorbells
 * The doorbells send the same packet a number of times shortly after each other. 
 * We just grab one packet out of the lot and kill the remaining ones. 
 \*********************************************************************************************/
#define PLUGIN_ID 1
#define PLUGIN_NAME "RawSignalAnalyze"

#define FA500RM3_PulseLength    26
#define FA500RM1_PulseLength    58
#define PLUGIN_001_EVENT      "CONVERT"

boolean Plugin_001(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;        // always false as we only do conversion of long packets and no actual processing, the converted packet must be processed by other plugins
  byte FAconversiontype=1;      // 0=FA500R to Method 2
                                // 1=FA500R to Method 1
  byte HEconversiontype=0;      // 0=No conversion, 1=conversion to Elro 58 pulse protocol (same as FA500R Method 1)
  
  switch(function)
    {
    #ifdef PLUGIN_001_CORE

    case PLUGIN_RAWSIGNAL_IN:
      {
      int i,j;
      char buffer[14]=""; 
      // ==========================================================================
      // DEBUG
      // ==========================================================================
      if (RFDebug==true) {
         if(RawSignal.Number<8)return false;     // make sure the packet is long enough to have a meaning 
         // ----------------------------------
         // Output
         // ----------------------------------
         sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( buffer );
         // ----------------------------------
         Serial.print(F("DEBUG;Pulses="));      // debug data
         Serial.print(RawSignal.Number);
         Serial.print(F(";Pulses(uSec)="));      
         for(i=1;i<RawSignal.Number;i++) {
            Serial.print(RawSignal.Pulses[i]*RawSignal.Multiply); 
            if (i < RawSignal.Number-1) Serial.write(',');       
         }
         Serial.println(";");
      }      
      // ==========================================================================
      // Beginning of Signal translation for HomeEasy HE842
      // ==========================================================================
      if (RawSignal.Number > 460 && RawSignal.Number < 470) {
         if (HEconversiontype==0) {          // Reject the entire packet 
            RawSignal.Number=0;                 
            success=true;
         } else {                            // Convert to Elro Method 1 (same as FA500 Method 1)
            int pos1=RawSignal.Number - 58;
            if (RawSignal.Pulses[pos1]*RawSignal.Multiply > 4000) {
               for (i=0;i<58;i++){
                   RawSignal.Pulses[1+i]=RawSignal.Pulses[pos1+1+i];
               }
               RawSignal.Number=58;
               break;
            }
         }
      } 
      // ==========================================================================
      // End of Signal translation HomeEasy HE842
      // ==========================================================================      
      // ==========================================================================
      // Beginning of Signal translation for Flamingo FA500R
      // ==========================================================================
      if (RawSignal.Number > 330 && RawSignal.Number < 370) {
         int pos1=RawSignal.Number - 130;
         int pos2=RawSignal.Number - 130 - 58;
         int pos3=RawSignal.Number - 18;
         if (RawSignal.Pulses[pos1]*RawSignal.Multiply > 2000 && RawSignal.Pulses[pos2]*RawSignal.Multiply > 4000) {
            if (FAconversiontype==0) {              // Convert to Flamingo FA500R Method 2
                for (i=0;i<130;i++){ 
                    RawSignal.Pulses[3+i]=RawSignal.Pulses[pos1+1+i];
                }
                RawSignal.Number=132;
                break;
            } else 
            if (FAconversiontype==1) {              // Convert to Flamingo FA500R Method 1
                for (i=0;i<58;i++){
                    RawSignal.Pulses[1+i]=RawSignal.Pulses[pos2+1+i];
                }
                RawSignal.Pulses[0]=RawSignal.Pulses[pos3];  // Trick: use the on/off command from the method 2 (newkaku) packet and pass via Pulses[0]
                RawSignal.Number=58;
                break;
            }
         }
      }
      // ==========================================================================
      // End of Signal Translation
      // ==========================================================================      
      // ==========================================================================
      // Beginning of Signal translation for Auriol
      // ==========================================================================
      if (RawSignal.Number == 511) {
         int pos1=74;
         int pos2=74+74;
         int pos3=74+74+74;
         if (RawSignal.Pulses[pos1]*RawSignal.Multiply > 3400 && RawSignal.Pulses[pos2]*RawSignal.Multiply > 3400 && RawSignal.Pulses[pos3]*RawSignal.Multiply > 3400) {
            RawSignal.Pulses[pos1]=0;
            RawSignal.Number=74;
            break;
         }
      }
      // ==========================================================================
      // End of Signal Translation
      // ==========================================================================      
      // ==========================================================================
      // Beginning of Signal translation for Byron & Lidl Doorbells
      // ==========================================================================
      if (RawSignal.Number == 511) {
         for (j=2;j<RawSignal.Number;j++) {
             if (RawSignal.Pulses[j]*RawSignal.Multiply > 2500) {  // input is going to fast skip to where new part is going to start
                if (j+26 > 511) break; 
                if (RawSignal.Pulses[j+26]*RawSignal.Multiply > 2500) { // first long delay found, make sure we have another at the right position               
                    int pos1=j+1;
                    for (i=0;i<26;i++){
                        RawSignal.Pulses[1+i]=RawSignal.Pulses[pos1+i]; // new filling of the pulses
                    }
                    RawSignal.Number=26;
                }
                break;
             } else 
             if (RawSignal.Pulses[j]*RawSignal.Multiply > 1750) {  // input is going to fast skip to where new part is going to start
                if (j+81 > 511) break;
                if (RawSignal.Pulses[j+1]*RawSignal.Multiply > 1000 && RawSignal.Pulses[j+2]*RawSignal.Multiply > 1000 &&
                    RawSignal.Pulses[j+3]*RawSignal.Multiply > 1000 && RawSignal.Pulses[j+4]*RawSignal.Multiply > 1000 && 
                    RawSignal.Pulses[j+5]*RawSignal.Multiply > 1000 && RawSignal.Pulses[j+6]*RawSignal.Multiply > 1000 &&
                    RawSignal.Pulses[j+7]*RawSignal.Multiply > 1000 && RawSignal.Pulses[j+8]*RawSignal.Multiply > 1000 ) {  //input is going to fast skip to where new part is going to start

                    int pos1=j;
                    for (i=0;i<90;i++){
                        RawSignal.Pulses[1+i]=RawSignal.Pulses[pos1+1+i]; // new filling of the pulses
                    }
                    RawSignal.Number=90;
                    break;
                }  
             }
         }
      } 
      // ==========================================================================
      // End of Signal translation
      // ==========================================================================      
      if (RawSignal.Number > 290) {   // unknown and unsupported long packet (284 is the max. pulse length used at the cresta/tfa plugin)
         RawSignal.Number=0;          // no need to show this to any of the plugins for processing 
         success=true;                // set to abort
      }                               // as there is no support for it anyway 
      break;
      }
    #endif //PLUGIN_CORE_001
    }      
  return success;
}


