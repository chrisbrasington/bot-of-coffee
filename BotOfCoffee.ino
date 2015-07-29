#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Time.h>
#include <String.h>
#include <Twitter.h>

// Enter a MAC address for your controller below.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

unsigned int localPort = 8888;      // local port to listen for UDP packets

// time server
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 
int timeZoneOffSet = 5;

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Your Token to Tweet (get it from http://arduino-tweet.appspot.com/)
Twitter twitter("");

// 10 second refresh interval
int delayInterval = 10000;

// DD/MM/YYYY HH:MM:SS AM/PM
String getTimeString() {
  String time = "";
  time += month();
  time += "/";
  time += day();
  time += "/";
  time += year();
  time += " ";
  
  unsigned long epoch = now();
  
  // hours
  time += (((epoch  % 86400L) / 3600)+timeZoneOffSet)%12;
  time += ":";
  
  // minutes
  if ( ((epoch % 3600) / 60) < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    time += "0";
  }
  time += (epoch  % 3600) / 60;
  time += ":";
  
  // seconds
  if ( (epoch % 60) < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    time += "0";
  }
  time += epoch %60;
  
  if(isAM()) {
    time += " AM";
  }
  else {
    time += " PM";
  }
  
  return time;
}

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}

bool tweet(String message){
  Serial.println("Tweeting message: ");
  Serial.println(message);

  message += " " + getTimeString();

  if (twitter.post(message.c_str())) {
    // Specify &Serial to output received response to Serial.
    // If no output is required, you can just omit the argument, e.g.
    // int status = twitter.wait();
    int status = twitter.wait(&Serial);
    if (status == 200) {
      Serial.println("OK.");
      return true;
    } else {
      Serial.print("failed : code ");
      Serial.println(status);
    }
  } else {
    Serial.println("connection failed.");
  }
  return false;
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  Udp.begin(localPort);

  sendNTPpacket(timeServer); // send an NTP packet to a time server

  // wait to see if a reply is available
  // set current time
  delay(1000);
  if ( Udp.parsePacket() ) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    setTime(epoch);
  }

  // initial tweet of reboot
  tweet("I'm being programmed! @BotOfCoffee has rebooted.");
}

void loop()
{
  // serial print current time
  Serial.print("Current Time is ");
  Serial.println(getTimeString());

  // wait ten seconds before asking for the time again
  delay(delayInterval);
}






