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
//#include "newport.h"
#include <string>  
#include <algorithm>
#include <typeinfo>
#include <sstream>  
#include <vector>
#include "CommonProject.h"
using namespace std;

bool content_resp;

class Packet;
class Port;
class Address;
class SendingPort;
pthread_mutex_t lock;

list<int> list1;
int content_number = -1;

const char* FNgenerator(int index , bool  flag){

    std::stringstream ss;
    string filename1,filename;
    ss << index;
    filename1= ss.str();
    if(flag)
      filename=filename1+".txt";
    else
      filename=filename1+"r.txt";
    return filename.c_str();


}

void SaveFileAs(const char* Filename, vector<char> *Data)
{
   ofstream OutputFile(Filename) ;

   OutputFile.write(&Data->at(0) ,Data->size());

   OutputFile.close();
}

struct cShared{
  mySendingPort *my_send_port;
  LossyReceivingPort *my_recv_port;
  int hostID;
  
};

void *rxthread(void *arg){

struct cShared *sh = (struct cShared *)arg;
//char *msg= (char *)msg1;
mySendingPort *my_send_port1 = sh-> my_send_port;
LossyReceivingPort *my_recv_port1 = sh-> my_recv_port;
int hID;


Packet *pData;
int var2,var1=1;



while(1){

    const char * FName;


    var1=1;
//    cout<<"entered while(1) loop"<<endl;
    while(var1){
//        cout<<"entered while(var1) loop"<<endl;
        pData = my_recv_port1->receivePacket();
           // cout<<"checking for ack"<<endl;
           if (pData != NULL)
             {
//                cout<<"valid data"<<endl;
                var2 = (int)pData->accessHeader()->getOctet(CONTENTID_POS);
                hID = (int)pData->accessHeader()->getOctet(HOSTID_OR_HOPS_POS);
                //cout<<"received packet of type : "<< (int)pData->accessHeader()->getOctet(PACKET_TYPE_POS);
                //*varc1= pData->accessHeader()->getOctet(0);
                var1 = 0;
//                cout<<"valid data after header retrieval "<<endl;
             }
      }
      if(pData->accessHeader()->getOctet(PACKET_TYPE_POS)==REQUEST){         //RESPONDING TO A REQUEST FOR CONTENT

        ifstream myfile;
        //char contentName[8];

        cout<<"Request for Content Received"<<endl;

        FName = FNgenerator(var2, true);
        


        ifstream file(FName);
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

        ifstream ReadFile;
        ReadFile.open(FName,ios::in|ios::ate);
        int FileSize = ReadFile.tellg();
        ReadFile.close(); 

        pthread_mutex_lock(&lock);

        Packet *my_packet = new Packet();

        MakeResponsePacket(my_packet, var2, hID);

        my_packet->setPayloadSize(FileSize);

        my_packet->fillPayload( Data.size() , &Data.at(0));

        cout<<endl<<"sending Content ......"<<endl;
        //my_send_port1->setACKflag(true);
        my_send_port1->sendPacket(my_packet);

        sleep(0.5);
        pthread_mutex_unlock(&lock);
      //  delete my_packet;
      }else if(pData->accessHeader()->getOctet(PACKET_TYPE_POS)==RESPONSE){        //I GET DATA
        //PUT DATA IN FILE
        
        if(var2 == content_number)
        {
            ofstream myfile;


            FName = FNgenerator(var2, false);

            //filename.c_str()
            myfile.open (FName);
            myfile << pData->getPayload();
            myfile.close();

            cout<<"\n Received data response "<< pData->getPayload()<<" and saved. Check in your folder :-) "<<endl;
            content_resp = true;
            my_send_port1->setACKflag(true);
            my_send_port1->timer_.stopTimer();

            //content_number = -1;
        }  

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

  pthread_mutex_lock(&lock);

  list<int>::iterator p1 = list1.begin();
  while(p1 != list1.end()){
      
      
      MakeAnnouncementPacket(my_packet, *p1, 0);


      cout<<"sending announcement for content "<<*p1<<endl;
      //my_send_port1->setACKflag(true);
      my_send_port1->sendPacket(my_packet);
      
      p1++;
      sleep(1);
  }

 /* cout<<"\n List values \n";
  list<int>::iterator p = list1.begin();
  while(p != list1.end()){
      //str2 = p;
      cout << *p << " : "<<"\n";
      p++;

  }*/

  pthread_mutex_unlock(&lock);

  sleep(BCAST_TIME);
  }
return 0;

}



int main(int argc, const char * argv[])
{
int  ret1, ret2, cID;


pthread_t thread1, thread2;



try {

  const char* hname = "localhost";       
  Address * my_addr = new Address(hname, (short)(atoi(argv[3])));     //tx port
  Address * dst_addr =  new Address(hname, (short)(atoi(argv[4])));   //remote port
  mySendingPort *my_port = new mySendingPort();
  my_port->setAddress(my_addr);
  my_port->setRemoteAddress(dst_addr);
  my_port->init();

//configure receiving port to listen to ACK frames
  Address * my_rx_addr = new Address(hname, (short)(atoi(argv[2])));  //rx port
  LossyReceivingPort * my_rx_port = new LossyReceivingPort(0);
  my_rx_port->setAddress(my_rx_addr);
  my_rx_port->init();

  int hostID = atoi(argv[1]);
  
  cout<<"setup done"<<endl;
//storingthe arguments to be passed
  struct cShared *sh;
  sh = (struct cShared*)malloc(sizeof(struct cShared));
  sh->my_send_port = my_port;
  sh->my_recv_port = my_rx_port;
  sh->hostID=hostID;

  pthread_mutex_init(&lock, NULL);


  ret1 = pthread_create(&thread1,NULL,bcast,sh);
  ret2 = pthread_create(&thread2,NULL,rxthread,sh);         //starting the rx/tx thread


  while(1){ 

    const char * FName;

    char a[10],b[10],c[10];
    char *str1 = a, *str2 = b;
    
//    sleep(2);
    cout<<"Enter Operation (GET / PUT / DEL)\n";
    cin>>str1;

    cout<<"Enter ContentID\n";
    cin>>cID;
    //cout<<"str1 "<<str1<<"\n";

    //int i=0 ;
//    pch=strtok(str1," ");
    strncpy(str2,str1,3);



    int var1=cID;//atoi(pch);



    //cout<<"the value : "<<*findIter<<"\n";


    if(strncmp(str2,"PUT",3)==0){
      //cout<<"putting data\n";



      FName = FNgenerator(var1, true);



      ifstream file(FName);
      vector<char>Data;  // used to store all the data from the file.

      //Check if the File is successfully opened or not.
      if(!file.is_open())
      {
      
          cout<<"\n Oops!! Please enter PUT and ContentID again."<<endl;
      
      }else {

    	  pthread_mutex_lock(&lock);

    	     list<int>::iterator findIter = std::find(list1.begin(), list1.end(), cID);    //ITERATES THROUGH LIST TO CHECK IF CID EXISTS
    	     if((*findIter)<=0){                                                            //PUTS CID INTO LIST IF IT DOESNT ALREADY EXIST
    	                //pthread_mutex_lock(&lock);

    	                list1.push_back(cID);

    	                //pthread_mutex_unlock(&lock);

    	              }

    	    pthread_mutex_unlock(&lock);


      }


      file.close();



    }else if(strncmp(str2,"DEL",3)==0){
      
        cout<<"deleting data\n";
        pthread_mutex_lock(&lock);

        list1.remove(var1);

        pthread_mutex_unlock(&lock);

    }else if(strncmp(str2,"GET",3)==0){
      

      cout<<"getting data\n";


//      if(*findIter)

//        cout<< "You have the data already!!!";

//      else{

                                                                        // REQUEST FOR PACKET BEING SENT HERE
        Packet * data_req_packet;
        data_req_packet = new Packet();
        content_number = var1;

        MakeRequestPacket( data_req_packet, var1 , hostID);
        content_resp = false;


        //data_req_packet->fillPayload(strlen(SFName),SFName);
            my_port->sendPacket(data_req_packet);
            printf("data_req Packet for content %X Sent!! \n\n", data_req_packet->accessHeader()->getOctet(CONTENTID_POS) );
        
            my_port->lastPkt_ = data_req_packet;
            my_port->setACKflag(false);
            my_port->timer_.startTimer(ARQ_TIME);


      ////////////////////////////////////////////
 //     }

    }
    else{
      cout<<"INVALID ENTRY for command. Re-Enter Command\n";
    }


  }


    
} catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
  }  

 return 0;
}

