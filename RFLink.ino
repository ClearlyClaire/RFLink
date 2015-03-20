/*********************************************************************************************************************************

NNNNNNNN        NNNNNNNN                             d::::::d                       333333333333333           77777777777777777777
N:::::::N       N::::::N                             d::::::d                      3:::::::::::::::33         7::::::::::::::::::7
N::::::::N      N::::::N                             d::::::d                      3::::::33333::::::3        7::::::::::::::::::7
N:::::::::N     N::::::N                             d:::::d                       3333333     3:::::3        777777777777:::::::7
N::::::::::N    N::::::N   ooooooooooo       ddddddddd:::::d    ooooooooooo                    3:::::3                   7::::::7 
N:::::::::::N   N::::::N oo:::::::::::oo   dd::::::::::::::d  oo:::::::::::oo                  3:::::3                  7::::::7  
N:::::::N::::N  N::::::No:::::::::::::::o d::::::::::::::::d o:::::::::::::::o         33333333:::::3                  7::::::7   
N::::::N N::::N N::::::No:::::ooooo:::::od:::::::ddddd:::::d o:::::ooooo:::::o         3:::::::::::3                  7::::::7    
N::::::N  N::::N:::::::No::::o     o::::od::::::d    d:::::d o::::o     o::::o         33333333:::::3                7::::::7     
N::::::N   N:::::::::::No::::o     o::::od:::::d     d:::::d o::::o     o::::o                 3:::::3              7::::::7      
N::::::N    N::::::::::No::::o     o::::od:::::d     d:::::d o::::o     o::::o                 3:::::3             7::::::7       
N::::::N     N:::::::::No::::o     o::::od:::::d     d:::::d o::::o     o::::o                 3:::::3            7::::::7        
N::::::N      N::::::::No:::::ooooo:::::od::::::ddddd::::::ddo:::::ooooo:::::o     3333333     3:::::3           7::::::7         
N::::::N       N:::::::No:::::::::::::::o d:::::::::::::::::do:::::::::::::::o     3::::::33333::::::3 ......   7::::::7          
N::::::N        N::::::N oo:::::::::::oo   d:::::::::ddd::::d oo:::::::::::oo      3:::::::::::::::33  .::::.  7::::::7           
NNNNNNNN         NNNNNNN   ooooooooooo      ddddddddd   ddddd   ooooooooooo         333333333333333    ...... 77777777      

                                       Nodo RadioFrequencyLink aka Nodo RFLink Version 1.0
                                                      
/********************************************************************************************************************************
* Arduino project "Nodo RFLink" © Copyright 2015 StuntTeam - NodoRFLink V1.0
* This is a slightly modified version of Nodo 3.7
\********************************************************************************************************************************/

// ============================================================================================================================
// Supply the correct path to the RFLink files in the define below 
//
// Geef in onderstaande regel op waar de .ino bestanden zich bevinden die je nu geopend hebt.
// Geef het volledige pad op. Let op: Bij Linux/Unix zijn de namen case-sensitive.
// ============================================================================================================================

#define SKETCH_PATH D:\arduino\RFLink

/**************************************************************************************************************************\
* Arduino project "Nodo" © Copyright 2014 Paul Tonkes 
* 
* This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License 
* as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
* You received a copy of the GNU General Public License along with this program in file 'COPYING.TXT'.
*
* Voor toelichting op de licentievoorwaarden zie    : http://www.gnu.org/licenses
* Uitgebreide documentatie is te vinden op          : http://www.nodo-domotica.nl
* bugs kunnen worden gelogd op                      : https://code.google.com/p/arduino-nodo/
* Compiler voor deze programmacode te downloaden op : http://arduino.cc
* Voor vragen of suggesties, mail naar              : p.k.tonkes@gmail.com
* Compiler                                          : Arduino Compiler met minimaal versie 1.0.5
* Libraries                                         : EthernetNodo library vervangt de standaard Ethernet library!
\*************************************************************************************************************************/

// ============================================================================================================================
// IMPORTANT NOTE: This code only runs on an Arduino MEGA !!!!!!!!!!!!!!!!!!!!!!  It was designed that way for various reasons.
// ============================================================================================================================
// We kennen twee type Nodo's:
//
// Nodo-Mega:   Een Nodo op basis van een Arduino Mega met een ATMega1280 of 2560 processor. Deze Nodo heeft o.a. de mogelijkheid
//              tot ethernet communicatie.
//
// Nodo-Small:  Dit is een kleine Nodo die wordt vooral wordt gebruikt als satelliet in combinatie met een centrale Nodo-Mega.
//              Een Nodo-Small maakt gebruik van een Arduino met een ATMega328 processor. (Nano, Pro,Duemillanove, Uno, etc)                                                                                                                
//
// Bij gebruik van meerdere Nodo's kan voor iedere Nodo een eigen configutatie file worden aangemaakt. In deze configuratie files
// kan worden aangegeven welke plugins worden gebruikt en kunnen eventueel speciale instellingen worden opgegeven die mee
// gaan met het compileren van de code.
// Configuratie bestanden bevinden zich in de directory ../Config. In deze configuratiefiles kunnen settings worden opgegeven 
// die worden meegecompileerd of kunnen devices worden opgegeven waar de Nodo mee moet kunnen communiceren.
// Default zijn de volgende configuratie files gemaakt:
//
// Config_01.c => Deze is default bestemd voor een Nodo Mega met unitnummer 1.
//
// Alle regels gemarkeerd met een '//' worden niet meegecompilileerd.
// 
// LET OP:
//  
// -  Het unitnummer van de Nodo zal pas veranderen nadat de Nodo software voor het eerst wordt geinstalleerd of de Nodo het 
//    commando [Reset] uitvoert. Dus niet altijd na een compilatie!
// -  Indien gewenst kunnen de config files ook voor andere unitnummers worden aangemaakt (1..31)
// -  Gebruik altijd de in deze zip meegeleverde libraries voor I2C en Ethernet communicatie!
// ============================================================================================================================

