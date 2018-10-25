/*
 created : 16 Dec 2014 by Graeme Pinnock
 modified: 17,22,23,28,30 Dec 2014 
           21             Jan 2015
           by Graeme Pinnock
*/

//Required for Ethernet Shield
#include <SPI.h>
#include <Ethernet.h>

// Mac address below is mapped to "arduino1" by my DHCP server.
// It is the mac supplied with the hardware.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xC0, 0x2F };

// Initialize the Ethernet server library - port 32000
EthernetServer server(32000);

// run once
void setup() {
  // start the Ethernet connection (This can take 30 seconds!) :
  if (Ethernet.begin(mac) == 0) {
//    Serial.println("DHCP Failed");
    // no point in carrying on, so do nothing forevermore:
    // IDEA: Flash Status LED to show error 
    for(;;)
      ;
  }

  // Start the TCP/IP server:
  server.begin();
  // announce IP address the server is running on
//  Serial.print("ServerIP ");
//  Serial.println(Ethernet.localIP());

  // Open serial communications 
  Serial.begin(57600);

  // Clear serial buffer
  clearSerialBuffer() ;  
}

void loop() {
  // wait for clients to connect
  EthernetClient client = server.available();
  
  if (client) {
//    Serial.print("<- new client ->");
//    ++reqcount ;
//    Serial.println (reqcount) ;

    String commandLine = "" ;

    // while the client is connected to us
    while (client.connected()) {
      // and there is data in the buffer
      if (client.available()) {
        // read the data one character at a time
        char c = client.read();
        // and echo it to the serial monitor
//        Serial.write(c);
        // so you can send a reply
        if (c == '\n' ) {
          // send a response 
          if (commandLine == "SSS") {
            client.println ("Volts:12.0 Temp:24.6C");
          } else if (commandLine == "SSU") {
            client.print   ("Arduino2 PINGed Arduino1 which replied : ") ;
            client.println(getSerialResponse("PING")) ;
          } else {            
            client.print   ("Arduino2 passed: '");
            client.print   (commandLine);
            client.print   ("' to Arduino1, which responded: '");
            client.print   (getSerialResponse(commandLine)) ;
            client.println ("'");
          }
          commandLine = "" ;
          break ;
        }
        commandLine += c ;
      }
    }
    // give client time to receive the data
    delay(500);
    // close the client connection:
    client.stop();
    // Serial.println("<- Server stopped Client ->");
  }
}
String getSerialResponse(String commandLine) {
   Serial.println (commandLine);
   // send data only when you receive data:
     delay (500) ;
     String cmdResponse = "" ;
     while (Serial.available()) {
          char s = Serial.read();
          if (s == '\n' ) {
            return cmdResponse ;
          }  
          cmdResponse += s ;
     }
     return "Timeout" ;
}     

void clearSerialBuffer() {
     while (Serial.available()) {
          char s = Serial.read();
     }
}     
