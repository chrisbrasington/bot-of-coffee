#include <SPI.h> // needed in Arduino 0019 or later
#include <Ethernet.h>
#include <Twitter.h>
#include <Time.h>
/*
#include <UdpBytewise.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008 
#if  UDP_TX_PACKET_MAX_SIZE <64 ||  UDP_RX_PACKET_MAX_SIZE < 64
#error : UDP packet size to small - modify UdpBytewise.h to set buffers to 64 bytes
#endif
*/

// Ethernet Shield Settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// If you don't specify the IP address, DHCP is used
byte ip[] = { 192, 168, 11, 250 };

// Your Token to Tweet (get it from http://arduino-tweet.appspot.com/)
Twitter twitter("");

// Message to post
//char msg[] = "Hello, World! I'm @BotOfCoffee Arduino!";

byte SNTP_server_IP[]    = { 192, 43, 244, 18}; // time.nist.gov
time_t prevDisplay = 0; // when the digital clock was displayed
const  long timeZoneOffset = 0L; // set this to the offset in seconds to your local time;

void setup()
{
  delay(1000);
  Ethernet.begin(mac, ip);

  Serial.begin(9600);
  Serial.println("connecting ...");
  
  Serial.println(year());
  
  /*
  if (twitter.post(msg)) {
    // Specify &Serial to output received response to Serial.
    // If no output is required, you can just omit the argument, e.g.
    // int status = twitter.wait();
    int status = twitter.wait(&Serial);
    if (status == 200) {
      Serial.println("OK.");
    } else {
      Serial.print("failed : code ");
      Serial.println(status);
    }
  } else {
    Serial.println("connection failed.");
  }
  */
  
  setSyncProvider(getNtpTime);
  while(timeStatus()== timeNotSet)   
     ; // wait until the time is set by the sync provider
}

void loop()
{
  if( now() != prevDisplay) //update the display only if the time has changed
  {
    prevDisplay = now();
    digitalClockDisplay();  
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

unsigned long getNtpTime()
{
  sendNTPpacket(SNTP_server_IP);
  delay(1000);
  if ( UdpBytewise.available() ) {
    for(int i=0; i < 40; i++)
       UdpBytewise.read(); // ignore every field except the time
    const unsigned long seventy_years = 2208988800UL + timeZoneOffset;        
    return getUlong() -  seventy_years;      
  }
  return 0; // return 0 if unable to get the time
}

unsigned long sendNTPpacket(byte *address)
{
  UdpBytewise.begin(123);
  UdpBytewise.beginPacket(address, 123);
  UdpBytewise.write(B11100011);   // LI, Version, Mode
  UdpBytewise.write(0);    // Stratum
  UdpBytewise.write(6);  // Polling Interval
  UdpBytewise.write(0xEC); // Peer Clock Precision
  write_n(0, 8);    // Root Delay & Root Dispersion
  UdpBytewise.write(49); 
  UdpBytewise.write(0x4E);
  UdpBytewise.write(49);
  UdpBytewise.write(52);
  write_n(0, 32); //Reference and time stamps  
  UdpBytewise.endPacket();   
}

unsigned long getUlong()
{
    unsigned long ulong = (unsigned long)UdpBytewise.read() << 24;
    ulong |= (unsigned long)UdpBytewise.read() << 16;
    ulong |= (unsigned long)UdpBytewise.read() << 8;
    ulong |= (unsigned long)UdpBytewise.read();
    return ulong;
}

void write_n(int what, int how_many)
{
  for( int i = 0; i < how_many; i++ )
    UdpBytewise.write(what);
}

