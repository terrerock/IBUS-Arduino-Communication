In this project, Arduino Uno Revision 4 is used to receive serial IBUS 32-byte data packet transmitted from FlySky's RC Receiver (FS-IA10B).

Getting Started:

1. Connect FS-IA10B TX pin to Arduino RX pin 0 (Purple)
2. Connect FS_IA10B 5V pin (middle pin) to Arduino 5V (Red)
3. Connect FS_IA10B Ground pin to Arduino Ground pin (Blue)
4. Connect Arduino USB-C to PC USB-C or USB-A port
5. Launch Arduino IDE, download IBus.h, IBus.cpp, and Ibus_decoding.ino
6. Compile IBus.h, IBus.cpp files and upload executable to Arduino.  Open Arduino IDE Serial Monitor
7. Turn on FlySky Transmitter (Make sure IBUS is ENABLE on the Transmitter using Setup Menu)
8. You should see the following RC data on the Serial Monitor:                                                  
// HDR   Roll Pit Throt Yaw  Pot1 Pot2 Ch07 Ch08 Ch09 Ch10 Ch11 Ch12 Ch13 Ch14 Csum  Stat  Cnt Time usec  Delta

// 16416 1525 1544 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62437   1    32 25125330   7701  
// 16416 1525 1544 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62437   1    32 25133032   7702  
// 16416 1536 1538 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62687   1    32 25140728   7696  
// 16416 1536 1538 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62687   1    32 25148426   7698  
// 16416 1536 1538 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62687   1    32 25156129   7703  
// 16416 1544 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62428   1    32 25163829   7700  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25171528   7699  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25179229   7701  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25186928   7699  
// 16416 1546 1534 1000 1508 2000 2000 1000 1000 1000 1000 1500 1500 1500 1500 62426   1    32 25194628   7700  
   

![IMG_0805](https://github.com/terrerock/IBUS-Arduino-Comm/assets/126227459/47381785-271b-41db-90f7-3521b4029ac0)
