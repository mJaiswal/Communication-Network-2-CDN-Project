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



class Packet;
class Port;
class Address;
class SendingPort;
pthread_mutex_t lock;

class RTrecord {

public:
  int cID;
  int interface;
  int num_hops;
  int ttl_rt;



};

class PRTrecord{
  public:
    int cID;
    int hID;
    int interface;
    int ttl_val;

};





//list<message> MessageQueue;
  list<RTrecord> RoutingTable;
  list<PRTrecord> PendingRequestTable;
  vector<mySendingPort*> MySendingPorts;


pthread_mutex_t mutex_rt,mutex_prt; 

void *timer_function(void *arg){

  while(1){
/*
        pthread_mutex_lock(&mutex_rt);

        RTrecord testrecord_RT;
        PRTrecord testrecord_PRT;


        list<RTrecord>::iterator rtindex = RoutingTable.begin();
        for( rtindex = RoutingTable.begin(); rtindex != RoutingTable.end(); ++rtindex ){


          //RTrecord record_test = *rtindex;
          rtindex->ttl_rt = rtindex->ttl_rt-5;


          if (rtindex->ttl_rt <=0){

              RoutingTable.erase(rtindex);

          }

        }


        pthread_mutex_unlock(&mutex_rt);

        pthread_mutex_lock(&mutex_prt);

        list<PRTrecord>::iterator prtindex = PendingRequestTable.begin();
        for( prtindex = PendingRequestTable.begin(); prtindex != PendingRequestTable.end(); ++prtindex ){


          //PRTrecord record_test_prt = *prtindex;


        	prtindex->ttl_val = prtindex->ttl_val -5;
          //PendingRequestTable.push_back(testrecord_PRT);

          if((prtindex->ttl_val <= 0)){
                    
              PendingRequestTable.erase(prtindex);

                
           }

        }      

        pthread_mutex_unlock(&mutex_prt);
*/
        sleep(5);
    }
      return NULL;

}




int isContentPresentinRT(int ContentID, list<RTrecord> RoutingTable_local){

      list<RTrecord>::iterator rtindex = RoutingTable_local.begin();
      while(rtindex != RoutingTable_local.end()){
        
        if((rtindex->cID == ContentID)){
                  
            return rtindex->interface;
              
         }

        rtindex++;
      }
      return -1;

}



struct sharedStruct{

  LossyReceivingPort *my_recv_port;
  int routerID;
  int interface_num;
  int num_interface;
  //vector<mySendingPort*> SendingPorts;
  
};



void printPRT(int rID, list<PRTrecord> PendingRequestTable_local){


    cout<<"-------------- PENDING REQUEST TABLE @ ROUTER : "<< rID <<" --------------"<< endl;
    cout<<"Requested Content-ID\t\tRequesting Host-ID\t\t\tInterface"<<endl;

    list<PRTrecord>::iterator prtindex = PendingRequestTable_local.begin();
    for( prtindex = PendingRequestTable_local.begin(); prtindex != PendingRequestTable_local.end(); ++prtindex )
    {
   
        PRTrecord record_test_prt = *prtindex;
        cout<<record_test_prt.cID<<"\t\t\t\t";
        cout<<record_test_prt.hID<<"\t\t\t";
        //cout<<record_test_prt.ttl_val<<"\t\t\t";
        cout<<record_test_prt.interface<<endl;

    }

}


void printRT(int rID, list<RTrecord> RoutingTable_local){



    cout<<"-------------- ROUTING TABLE @ ROUTER : "<< rID  <<" --------------"<< endl;
    cout<<"Content-ID\t\t\tInterface"<<endl;

    list<RTrecord>::iterator rtindex = RoutingTable_local.begin();

    for( rtindex = RoutingTable_local.begin(); rtindex != RoutingTable_local.end(); ++rtindex )
    {


        RTrecord record_test_rt = *rtindex;
        cout<<record_test_rt.cID<<"\t\t\t\t";
        //cout<<record_test_rt.num_hops<<"\t\t\t";
        //cout<<record_test_rt.ttl_rt<<"\t\t\t";
        cout<<record_test_rt.interface<<endl;

    }

}


void *rx_function(void *arg){

    struct sharedStruct *shst = (struct sharedStruct *)arg;
    //char *msg= (char *)msg1;
    LossyReceivingPort *my_recv_port = shst->my_recv_port;

    int routerID = shst->routerID;

    int interface = shst->interface_num;

    int num_RX_TX_RemoteInterfaces = shst->num_interface;

    Packet *_recvPacket;
    int var2,var1=1;


                    list<RTrecord> RoutingTable_local;
                    list<PRTrecord> PendingRequestTable_local;

//    cout<<endl<<endl<<"Host transmit and receive through interface "<<interface<<endl<<endl;


    while(1){

        _recvPacket = my_recv_port->receivePacket();

            if (_recvPacket != NULL)
            {

              cout<<"pkt received "<<endl;
//                var2 = (int)pData->accessHeader()->getOctet(CONTENTID_POS);


                    pthread_mutex_lock(&mutex_prt);

                    list<PRTrecord>::iterator prtindex = PendingRequestTable.begin();
                    while(prtindex != PendingRequestTable.end()){

                        PRTrecord record_test_prt = *prtindex;

                        PendingRequestTable_local.push_back(record_test_prt);

                        prtindex++;
                                        
                    }

                    pthread_mutex_unlock(&mutex_prt);



                    pthread_mutex_lock(&mutex_rt);

                    list<RTrecord>::iterator rtindex = RoutingTable.begin();
                    while(rtindex != RoutingTable.end()){

                        RTrecord record_test_rt = *rtindex;

                        RoutingTable_local.push_back(record_test_rt);

                        rtindex++;
                                        
                    }

                    pthread_mutex_unlock(&mutex_rt);


                    cout<<endl;
                    printPRT(routerID, PendingRequestTable_local);

                    cout<<endl;

               		printRT(routerID, RoutingTable_local);

               		cout<<endl;


              printf("\n\n Received packet of type %x\n\n",_recvPacket->accessHeader()->getOctet(PACKET_TYPE_POS));



              switch(_recvPacket->accessHeader()->getOctet(PACKET_TYPE_POS))
              {



                case REQUEST:
                {
                  cout<<"Processing Request Packet......."<<endl;
                  //sleep(0.5);

                int ContentID = (int)_recvPacket->accessHeader()->getOctet(CONTENTID_POS);
                int HostID = (int)_recvPacket->accessHeader()->getOctet(HOSTID_OR_HOPS_POS);

                cout<<ContentID<<endl;
                cout<<HostID<<endl;

                  int  interface_num = 0;

                    bool test_flag = false;
                    PRTrecord record_test_prt;


                    cout << "check for content "<< ContentID << "iterator " <<endl;
                    list<PRTrecord>::iterator prtindex = PendingRequestTable_local.begin();
                    while(prtindex != PendingRequestTable_local.end()){

                        record_test_prt = *prtindex;

                        if((record_test_prt.cID == ContentID)){
                              if((record_test_prt.hID == HostID)){
                                  
                            	  cout<<"Content found "<<endl;
                                  test_flag = true;

                                  //Also reset its ttl
                                  record_test_prt.ttl_val = g_PENDING_TABLE_EXPIRY_TIME;

                                  break;
                              }
                        }

                        prtindex++;
                    }
                    


                    if(!test_flag){
                    	cout<<endl<<"Content not found "<<endl<<endl;
                    	record_test_prt.cID = ContentID;
                    	record_test_prt.hID = HostID;
                    	record_test_prt.ttl_val = g_PENDING_TABLE_EXPIRY_TIME;
                    	record_test_prt.interface = interface;//store the interface

                        //PendingRequestTable.push_back(record_test_prt);

                    }
                    
                    cout<<endl<<"check in routing table"<<endl<<endl;

                        int interface_send = -1;//isContentPresentinRT(ContentID, RoutingTable_local);

                        list<RTrecord>::iterator rtindex = RoutingTable_local.begin();
                        while(rtindex != RoutingTable_local.end()){

                          if((rtindex->cID == ContentID)){

                        	  interface_send = rtindex->interface;

                        	  break;
                           }
                          rtindex++;
                        }


                        cout<<endl<<"after check in routing table"<<endl<<endl;

                        cout<<"the interface is "<<interface_send<<endl<<endl;
                        if( interface_send!= (-1)){

                              cout<<endl<<"Forwarding Request Through "<<interface_send<<endl;
                              MySendingPorts.at(interface_send)->sendPacket(_recvPacket);

                        }
                        else
                        	cout<<endl<<"Returned Interface : " << interface_send <<"No valid entry found in the Routing Table "<<interface_send<<endl;
                  

                        cout<<endl;
                        printPRT(routerID, PendingRequestTable_local);
                        cout<<endl;


                        pthread_mutex_lock(&mutex_prt);

                        if(!test_flag)
                        	PendingRequestTable.push_back(record_test_prt);
                        else
                        {
							 list<PRTrecord>::iterator prtindex_ = PendingRequestTable.begin();
						 	while(prtindex_ != PendingRequestTable.end()){

							 if(prtindex_->cID == record_test_prt.cID && prtindex_->hID == record_test_prt.hID   )
							 {

								 prtindex_->ttl_val = record_test_prt.ttl_val;
								 break;
							 }


							 prtindex_++;

						 	}

                        }//else


						 pthread_mutex_unlock(&mutex_prt);
						  cout<<"\n\n Updated Pending Request Table.\n\n";


                  break;
                }

                case RESPONSE:
                {
                  cout<<"Processing Response Packet......."<<endl;
                  


                int ContentID = (int)_recvPacket->accessHeader()->getOctet(CONTENTID_POS);
                int HostID = (int)_recvPacket->accessHeader()->getOctet(HOSTID_OR_HOPS_POS);

                PRTrecord record_test_prt;
                bool isChange = false;




                    list<PRTrecord>::iterator prtindex_req = PendingRequestTable_local.begin();

                    while(prtindex_req != PendingRequestTable_local.end()){

                      record_test_prt = *prtindex_req;

                      if((record_test_prt.cID == ContentID && record_test_prt.hID == HostID )){
                            

                            MySendingPorts.at(record_test_prt.interface)->sendPacket(_recvPacket);

                            //pthread_mutex_lock(&mutex_prt);

                            //PendingRequestTable.erase(prtindex_req);
                            cout<<endl<<"response packet forwarded through interface "<< record_test_prt.interface<<endl;

                            //pthread_mutex_unlock(&mutex_prt);

                            //Remove this record from Pending Table
                            cout<<"Remove this request record from Pending Table"<<endl;
                            break;

                        }

                        prtindex_req++;
                      }
                    
                    cout<<endl;
                    printPRT(routerID, PendingRequestTable_local);
                    cout<<endl;

                    	pthread_mutex_lock(&mutex_prt);

							 list<PRTrecord>::iterator prtindex_ = PendingRequestTable.begin();
						 while(prtindex_ != PendingRequestTable.end()){

							 if(prtindex_->cID == record_test_prt.cID && prtindex_->hID == record_test_prt.hID   )
							 {
								 PendingRequestTable.erase(prtindex_);

								 break;
							 }


							 prtindex_++;

						 }

						 pthread_mutex_unlock(&mutex_prt);




						  cout<<"\n\n Updated Pending Request Table.\n\n";




                  break;

                }

                case ANNOUNCEMENT:
                {

					  cout<<"Processing Announcement Packet......."<<endl;

					  sleep(0.5);

					int ContentID = (int)_recvPacket->accessHeader()->getOctet(CONTENTID_POS);
					int hops = (int)_recvPacket->accessHeader()->getOctet(HOSTID_OR_HOPS_POS);
					  //PrintRoutingTable(p_RoutingTable, RouterID);

					bool flag = false;
					bool isNew = false;

					RTrecord record_test_rt;
                

                 	 list<RTrecord>::iterator rtindex = RoutingTable_local.begin();

                  	cout<<"Processing Announcement Packet started"<<endl;

					  while(rtindex != RoutingTable_local.end()){

						  record_test_rt = *rtindex;

							if((record_test_rt.cID == ContentID)){

							  flag =true;

							  //Also check for Hop count
							  if( hops < record_test_rt.num_hops )
									   record_test_rt.num_hops = hops;

							  //Put the ttl value again
							  record_test_rt.ttl_rt = g_ROUTING_TABLE_EXPIRY_TIME;

							  break;

							}

							rtindex++;

					  }

					  cout<<"Processing Announcement Packet halfway"<<endl;

					  if(!flag){


						  record_test_rt.cID = ContentID;
						  record_test_rt.num_hops = hops;
						  record_test_rt.ttl_rt = g_ROUTING_TABLE_EXPIRY_TIME;
						  record_test_rt.interface = interface;//store the interface


							cout<<"Processing Announcement Packet pushed"<<endl;

					  }



                  //Router received announcement packet

						for(int index=0; index < num_RX_TX_RemoteInterfaces; index++)
						{
						  _recvPacket->accessHeader()->setOctet((unsigned char)(hops+1),HOSTID_OR_HOPS_POS);
									   cout<<"sending"<<endl;
							if(index != interface)
							{
							  MySendingPorts.at(index)->sendPacket( _recvPacket);
							  cout<<" Broadcasting announcement through all tx ports  "<<endl;
							}

						}

						cout<<"Done announcement...."<<endl;

						 cout<<endl;

						 printRT(routerID, RoutingTable_local);

						 cout<<endl;

					  pthread_mutex_lock(&mutex_rt);

					  if(!flag)
						  RoutingTable.push_back(record_test_rt);
					  else
					  {
						 list<RTrecord>::iterator rtindex_ = RoutingTable.begin();
						 while(rtindex_ != RoutingTable.end()){

							 if( rtindex_->cID ==   record_test_rt.cID )
							 {
								 //RoutingTable.erase(rtindex_);

								 rtindex_->num_hops = record_test_rt.num_hops;
								 rtindex_->ttl_rt = record_test_rt.ttl_rt;


								 //RoutingTable.push_back(record_test_rt);

								 break;
							 }
							 rtindex_++;

						 }
					  }



					 pthread_mutex_unlock(&mutex_rt);

					  cout<<"\n\n Updated Routing Table.\n\n";


                  break;
                }//case

                default:
                  break;

              }// end of switch        

          //Updating Everything Back;


			// Update Done






              cout<<endl<<endl;
			  //CLear local cache
              RoutingTable_local.clear();
              PendingRequestTable_local.clear();




          }

/*
                    pthread_mutex_lock(&mutex_prt);
                    

                    pthread_mutex_unlock(&mutex_prt);
                   


                    cout<<endl<<endl;

                    pthread_mutex_lock(&mutex_rt);



                    pthread_mutex_unlock(&mutex_rt);
*/



     }

     cout<<"exiting thread"<<endl;

return NULL;


}



int main(int argc, const char * argv[]){


  pthread_t thread[5], thread1;


  pthread_mutex_init(&mutex_rt, NULL);
  pthread_mutex_init(&mutex_prt, NULL);


  int num =0;
  int index =0;

  int RouterID;
  int num_RX_TX_RemoteInterfaces;
  //the Rx Addressess
  vector<int> RxAddressess;
  //the Tx Addressess
  vector<int> TxAddressess;
  //the Tx Addressess
  vector<int> TxRemoteAddressess;
  //Tx Addr
  vector<Address*> TxAddr;
  // Tx Remote Addressess
  vector<Address*> TxRemoteAddr;
  // Tx Ports

  ifstream file(argv[1]);
  while (file >> num)
  {
    if(index == ROUTER_NUM_POS )                      
      RouterID = num;

    else if(index == NUM_TX_RX_REMOTE_INTERFACES_POS)
    {

      num_RX_TX_RemoteInterfaces = num;


    }
    else if (index >=2 && index < (2+num_RX_TX_RemoteInterfaces) )
    {
      RxAddressess.push_back(num);

    }
    else if (index >=(2+num_RX_TX_RemoteInterfaces) && index < (2+ (2*num_RX_TX_RemoteInterfaces)) )
    {
      TxAddressess.push_back(num);
      TxAddr.push_back( new Address("localhost", (short)(num) ) );

    }
    else if (  index >=(2+ (2*num_RX_TX_RemoteInterfaces)) && index < ( 2 + (3*num_RX_TX_RemoteInterfaces) ) )
    {
      TxRemoteAddressess.push_back(num);
      TxRemoteAddr.push_back( new Address("localhost", (short)(num))  );
    }

    // ignore anything else on the line
    index++;

  }
  //close the file
  file.close();


  //Configure the Sending Ports
  for(int index =0; index < num_RX_TX_RemoteInterfaces; index++ )
  {
    MySendingPorts.push_back(new mySendingPort());
    MySendingPorts.at(index)->setAddress(TxAddr[index]);
    MySendingPorts.at(index)->setRemoteAddress(TxRemoteAddr[index]);
    MySendingPorts.at(index)->init();

  }



  cout<<"----------------\tRouter Configuration Details:\t---------------"<<endl;
  cout<<"Receiving Interfaces\t\tSending Interfaces\t\tDestination Interfaces"<<endl;
  for(int index =0; index < num_RX_TX_RemoteInterfaces; index++ )
  {
    cout<<RxAddressess.at(index)<<"\t\t\t\t";
    cout<<TxAddressess.at(index)<<"\t\t\t\t";
    cout<<TxRemoteAddressess.at(index)<<"\t\t\t\t"<<endl;
  }

  cout<<"Router : " << RouterID << " is configured successfully. Enter to START\n"<<endl;



  for (int index=0; index<num_RX_TX_RemoteInterfaces ;index++){

      Address * my_rx_addr = new Address("localhost", (short)(RxAddressess.at(index)));  //rx port
      LossyReceivingPort * my_rx_port = new LossyReceivingPort(0);
      my_rx_port->setAddress(my_rx_addr);
      my_rx_port->init();


      struct sharedStruct *struct_arg;
      struct_arg = (struct sharedStruct*)malloc(sizeof(struct sharedStruct));
      struct_arg->my_recv_port = my_rx_port;
      struct_arg->routerID = RouterID;
      struct_arg->interface_num = index;
      struct_arg->num_interface = num_RX_TX_RemoteInterfaces;

//      srtuct_arg->SendingPorts = MySendingPorts;
      pthread_create(&thread[index],NULL,rx_function,struct_arg);                 



  }

       pthread_create(&thread1,NULL,timer_function, NULL);


       while(1);

}



