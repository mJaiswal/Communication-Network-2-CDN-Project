/*This program requires FOUR command line arguments.
ynatx:   ./sender DestinationHost DestinationPort FileToBeSent ReceivingPort

Variables:
  - FileSize - Contains the size of the file to be sent in bytes.
  - NPackets - Number of packets that will be sent; computed based on FileSize and PayloadSize of 1494 bytes
  - ReadBytes - At any point of time, it contains the number of bytes of data that has been read from the file
  - SFName - Name of file to be sent; extracted from command line argument
*/

#include "common.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>  
#include <list> 
#include <math.h>  
#include "newport.h"
#include <string>  
#include <algorithm>
#include <typeinfo>
#include <sstream>  
  #include <vector>
#include "CommonProject.h"
using namespace std;

class Packet;
class Port;
class Address;
class SendingPort;
pthread_mutex_t lock;

list<int> list1;

void SaveFileAs(const char* Filename, vector<char> *Data)
{
   ofstream OutputFile(Filename) ;

   OutputFile.write(&Data->at(0) ,Data->size());

   OutputFile.close();
}

struct cShared{
  mySendingPort *my_send_port;
  LossyReceivingPort *my_recv_port;
  
};

void *rxthread(void *arg){

struct cShared *sh = (struct cShared *)arg;
//char *msg= (char *)msg1;
mySendingPort *my_send_port1 = sh-> my_send_port;
LossyReceivingPort *my_recv_port1 = sh-> my_recv_port;

  

Packet *pData;
int var2,var1=1;
char *varc1 = "0";
while(1){
    var1=1;
    cout<<"entered while(1) loop"<<endl;
    while(var1){
        cout<<"entered while(var1) loop"<<endl;
        pData = my_recv_port1->receivePacket();
           // cout<<"checking for ack"<<endl;
           if (pData != NULL)
             {
                cout<<"valid data"<<endl;
                var2 = (int)pData->accessHeader()->getOctet(1);
                //*varc1= pData->accessHeader()->getOctet(0);
                var1 = 0;
                cout<<"valid data after header retrieval "<<endl;
             }
      }
      if(pData->accessHeader()->getOctet(0)==REQUEST){         //RESPONDING TO A REQUEST FOR CONTENT

        ifstream myfile;
        //char contentName[8];


        std::stringstream ss;
        string filename1,filename;
        ss << var2;
        filename1= ss.str();

        filename=filename1+".txt";
//        strcat(filename,".txt");
        cout<<"about to read data "<< filename<<endl;


        ifstream file(filename.c_str());
        vector<char>Data;  // used to store all the data from the file.

        //Check if the File is successfully opened or not.
        if(!file.is_open())
        {
           cout<<"\n Could Not open File"<<endl;
        }
   
        //Get the file data
        while (file.good())          // loop while extraction from file is possible
        {
          Data.push_back(file.get());       // get character from file

        }

        file.close();
    //SaveFileAs("3.txt", &Data);




        Packet *my_packet = new Packet();

        PacketHdr *hdr = my_packet->accessHeader();
        hdr->setOctet(RESPONSE,0);
        my_packet->fillPayload( Data.size() , &Data.at(0));

        cout<<"sending data pkt "<<endl;
        //my_send_port1->setACKflag(true);
        my_send_port1->sendPacket(my_packet);

        delete my_packet;
      }else if(pData->accessHeader()->getOctet(0)==RESPONSE){        //I GET DATA
        //PUT DATA IN FILE
        
        
        ofstream myfile;


        std::stringstream ss;
        string filename1,filename;
        ss << var2;
        filename1= ss.str();

        filename=filename1+".txt";

        //filename.c_str()
        myfile.open (filename.c_str());
        myfile << pData->getPayload();
        myfile.close();

        cout<<"entered data response "<< pData->getPayload()<<" in location "<<filename<<endl;

        my_send_port1->setACKflag(true);
        my_send_port1->timer_.stopTimer();        

      }
    }
  

return 0;

}



void *bcast(void *arg){

struct cShared *sh = (struct cShared *)arg;
//char *msg= (char *)msg1;
mySendingPort *my_send_port1 = sh-> my_send_port;

while(1){
  Packet *my_packet = new Packet();
  PacketHdr *hdr = my_packet->accessHeader();
  hdr->setOctet(ANNOUNCEMENT,0);

  cout<<"into thread"<<endl;
  list<int>::iterator p1 = list1.begin();
  while(p1 != list1.end()){
      
      hdr->setOctet((unsigned char)(*p1),1);
      cout<<"sending pkt "<<*p1<<endl;
      //my_send_port1->setACKflag(true);
      my_send_port1->sendPacket(my_packet);
      
      p1++;
      
  }

//  sem_wait(&mutex);
  pthread_mutex_lock(&lock);
 // cout<<msg<<endl;
  pthread_mutex_unlock(&lock);
//  sem_post(&mutex);
  sleep(10);
  }
return 0;

}



int main(int argc, const char * argv[])
{
int  ret1, ret2, cID;
char a[10],b[10],c[10];
char *str1 = a, *str2 = b, *pch = c;

char *msg1 = "Broadcast thread";

pthread_t thread1,thread2;




try {

  const char* hname = "localhost";       
  Address * my_addr = new Address(hname, 3000);
  Address * dst_addr =  new Address(argv[1], (short)(atoi(argv[2])));
  mySendingPort *my_port = new mySendingPort();
  my_port->setAddress(my_addr);
  my_port->setRemoteAddress(dst_addr);
  my_port->init();

//configure receiving port to listen to ACK frames
  Address * my_rx_addr = new Address(hname, (short)(atoi(argv[4])));
  LossyReceivingPort * my_rx_port = new LossyReceivingPort(0.2);
  my_rx_port->setAddress(my_rx_addr);
  my_rx_port->init();


//storingthe arguments to be passed
  struct cShared *sh;
  sh = (struct cShared*)malloc(sizeof(struct cShared));
  sh->my_send_port = my_port;
  sh->my_recv_port = my_rx_port;


  ret1 = pthread_create(&thread1,NULL,bcast,sh);
  ret2 = pthread_create(&thread2,NULL,rxthread,sh);         //starting the rx/tx thread


  while(1){ 
    cout<<"waiting for command\n";
    
//    sleep(2);
    cout<<"enter operation\n";
    cin>>str1;

    cout<<"enter cid\n";
    cin>>cID;
    //cout<<"str1 "<<str1<<"\n";

    int i=0 ;
//    pch=strtok(str1," ");
    strncpy(str2,str1,3);

    /*
    cout<<"str2 "<<str2<<" "<<strlen(str2)<<"\n";
    while(i<1){
      cout<<pch<<"\n";
      pch=strtok(NULL,"\0");
      
      i++;  
    }
    */

    //cout<<"length of pch : "<<strlen(pch)<<"\n";

    int var1=cID;//atoi(pch);

    list<int>::iterator findIter = std::find(list1.begin(), list1.end(), var1);    //ITERATES THROUGH LIST TO CHECK IF CID EXISTS

    //cout<<"the value : "<<*findIter<<"\n";


    if(strncmp(str2,"PUT",3)==0){
      cout<<"putting data\n";
      if(!(*findIter)){                                                            //PUTS CID INTO LIST IF IT DOESNT ALREADY EXIST
        list1.push_back(var1);
      }


        std::stringstream ss;
        string filename1,filename;
        ss << var1;
        filename1= ss.str();
        //itoa (var1,filename,10);;//+".txt";
        filename=filename1+".txt";
//        strcat(filename,".txt");
        cout<<"about to read data "<< filename<<endl;


ifstream file(filename.c_str());
        vector<char>Data;  // used to store all the data from the file.

   //Check if the File is successfully opened or not.
   if(!file.is_open())
   {
     cout<<"\n Could Not open File"<<endl;
   }

   //Get the file data
   while (file.good())          // loop while extraction from file is possible
     {
       Data.push_back(file.get());       // get character from file

     }

    //SaveFileAs("3.txt", &Data);
    file.close();



    }else if(strncmp(str2,"DEL",3)==0){
      cout<<"deleting data\n";
      list1.remove(var1);

    }else if(strncmp(str2,"GET",3)==0){
      cout<<"getting data\n";

      ////////////////////////////////////////////
      //creation of ARQ thread and the various handling goes here... 

      if(*findIter)

        cout<< "You have the data already!!!";

      else{
        Packet * data_req_packet;
        data_req_packet = new Packet();
        //data_req_packet->setPayloadSize(Pksize);
        PacketHdr *hdr1 = data_req_packet->accessHeader();
        hdr1->setOctet(REQUEST,0);
        hdr1->setOctet((unsigned char)var1,1);
        
        //data_req_packet->fillPayload(strlen(SFName),SFName);
        my_port->sendPacket(data_req_packet);
        cout<<endl<<"data_req Packet Sent!!"<<endl;
        
        my_port->lastPkt_ = data_req_packet; 
        my_port->setACKflag(false);
        //schedule retransmit
        my_port->timer_.startTimer(2.5);
      ////////////////////////////////////////////
      }

    }
    else{
      cout<<"INVALID ENTRY\n";
    }

    list<int>::iterator p = list1.begin();
    while(p != list1.end()){
        //str2 = p;
        cout << *p << " : "<<"\n";
        p++;
      
    }

  }


    
} catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
  }  

 return 0;
}

