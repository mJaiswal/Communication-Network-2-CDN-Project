/*This program requires FOUR command line arguments.

Synatx:   ./receiver ReceivingPort DestinatinAddress DestinationPort FileName

Variables: 
  Both of the following will be received in an initial packet "InfoPacket"
  - NPackets - Number of packets that will be received; extracted from header of InfoPacket
  - FName - Name of file that will be used to store all the packets being received; extracted from command line argument
*/


#include "common.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include "newport.h"
using namespace std;

class Packet;
class Port;
class Address;
class ReceiverPort;

int main(int argc, const char * argv[])
{



try {
  const char* hname = "localhost";
  //configure receiving port     
  Address * my_addr = new Address(hname, (short)(atoi(argv[1])));
  LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
  my_port->setAddress(my_addr);
  my_port->init();
  //FName = new char[strlen(argv[4])];
  //strcpy(FName, argv[4]);

  //configure a sending port to send ACK
  Address * my_tx_addr = new Address(hname, 3005);
  Address * dst_addr =  new Address(argv[2], (short)(atoi(argv[3])));
  mySendingPort *my_tx_port = new mySendingPort();
  my_tx_port->setAddress(my_tx_addr);
  my_tx_port->setRemoteAddress(dst_addr);
  my_tx_port->init();




  

  while(1)
  {
    Packet *p;
    p = my_port->receivePacket(); 
    if (p !=NULL)   {
      if(p->accessHeader()->getOctet(0) == 'A'){
        cout<< "Receiving packet, there is content "<<p->accessHeader()->getIntegerInfo(1)<<endl;
       

      }

  }  // k=p->accessHeader()->getIntegerInfo(1);
  }
} catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
}  

return 0;
}

