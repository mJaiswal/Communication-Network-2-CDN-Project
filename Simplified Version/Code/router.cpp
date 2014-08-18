
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
#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>



using namespace std;

//MessageQs for the Rx Thread-Main() communication
boost::lockfree::queue<Message*>messageQueue(g_MESSAGE_Q_CAPACITY);



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
  bool isvalid;



};

class PRTrecord{
  public:
    int cID;
    int hID;
    int interface;
    int ttl_val;
    bool isvalid;


};





//list<message> MessageQueue;
  list<RTrecord> RoutingTable;
  list<PRTrecord> PendingRequestTable;



pthread_mutex_t mutex_rt,mutex_prt;


struct sharedStruct{

  LossyReceivingPort *my_recv_port;
  int routerID;
  int interface_num;

};



void *timer_function(void *arg){


	int rt_delete_count =0;
	int prt_delete_count =0;

	bool erase = false;

  while(1){

	 	 sleep(10);

        pthread_mutex_lock(&mutex_rt);

        rt_delete_count = RoutingTable.size();

	while(rt_delete_count>0)
	{
			erase = false;

			list<RTrecord>::iterator rtindex = RoutingTable.begin();
			for( rtindex = RoutingTable.begin(); rtindex != RoutingTable.end(); ++rtindex ){


			  //RTrecord record_test = *rtindex;
			  rtindex->ttl_rt = rtindex->ttl_rt- 10;


			  if (rtindex->ttl_rt <=0){


				 erase = true;
				 break;

			  }

			  if(erase == false)
				  break;

			}//for

			rt_delete_count--;

			//Deleteing
			if(erase)
			{
				RoutingTable.erase(rtindex );

			}
			else
				break;

	  }


        pthread_mutex_unlock(&mutex_rt);




        pthread_mutex_lock(&mutex_prt);

        prt_delete_count = PendingRequestTable.size();

		while(prt_delete_count>0)
		{
			erase = false;
			list<PRTrecord>::iterator prtindex = PendingRequestTable.begin();
				for( prtindex = PendingRequestTable.begin(); prtindex != PendingRequestTable.end(); ++prtindex ){


				  //PRTrecord record_test_prt = *prtindex;


					prtindex->ttl_val = prtindex->ttl_val -10;


				if (prtindex->ttl_val<=0){


					 erase = true;
					 break;

				  }

				  if(erase == false)
					  break;

				}//for

				prt_delete_count--;

				//Deleteing
				if(erase)
				{
					PendingRequestTable.erase(prtindex );

				}
				else
					break;

		  }//while


        pthread_mutex_unlock(&mutex_prt);


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



//Rx Threads
void RxThreadFunc( void *arg )
{
		struct sharedStruct *shst = (struct sharedStruct *)arg;

	    LossyReceivingPort *my_recv_port = shst->my_recv_port;

	    int routerID = shst->routerID;

	    int interface = shst->interface_num;


			cout<<"\n Thread Created at Interface : "<< interface <<endl;
			Message *message = new Message(interface);
			Packet *recvPkt;

				//Keep Receiving Packet on the RxPort
				while(1)
				{


					recvPkt = my_recv_port->receivePacket();

					if( recvPkt != NULL)
					{
						message->_recvPacket = *recvPkt;

						//Put it in the messageQueue.

						if( ! messageQueue.push(message) )
						{
							perror("\n\n PSH FAILED");
						}

					}
				}


}




void printPRT(int rID, list<PRTrecord> PendingRequestTable_local){


    cout<<"-------------- PENDING REQUEST TABLE @ ROUTER : "<< rID <<" --------------"<< endl;
    cout<<"Requested Content-ID\t\tRequesting Host-ID\t\tInterface"<<endl;

    list<PRTrecord>::iterator prtindex = PendingRequestTable_local.begin();
    for( prtindex = PendingRequestTable_local.begin(); prtindex != PendingRequestTable_local.end(); ++prtindex )
    {

        PRTrecord record_test_prt = *prtindex;
        cout<<record_test_prt.cID<<"\t\t\t";
        cout<<record_test_prt.hID<<"\t\t\t";
        //cout<<record_test_prt.ttl_val<<"\t\t\t";
        cout<<record_test_prt.interface<<endl;

    }

}


void printRT(int rID, list<RTrecord> RoutingTable_local){



    cout<<"-------------- ROUTING TABLE @ ROUTER : "<< rID  <<" --------------"<< endl;
    cout<<"Content-ID\t\t Hops \t\t Interface"<<endl;

    list<RTrecord>::iterator rtindex = RoutingTable_local.begin();

    for( rtindex = RoutingTable_local.begin(); rtindex != RoutingTable_local.end(); ++rtindex )
    {


        RTrecord record_test_rt = *rtindex;
        cout<<record_test_rt.cID<<"\t";
        cout<<record_test_rt.num_hops<<"\t";
        //cout<<record_test_rt.ttl_rt<<"\t\t\t";
        cout<<record_test_rt.interface<<endl;

    }

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
  vector<mySendingPort*> MySendingPorts;


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

  cout<<"Router : " << RouterID << " is configured successfully.\n"<<endl;



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


//      srtuct_arg->SendingPorts = MySendingPorts;
     // pthread_create(&thread[index],NULL,RxThreadFunc,struct_arg);
      boost::thread(RxThreadFunc,struct_arg  );



  }
  	   //Timer Thread
       pthread_create(&thread1,NULL,timer_function, NULL);

       //For MessageQueues
       	Message *p_message = new Message();

       	int interface;

       	//lOCAL cache
       	list<RTrecord> RoutingTable_local;
       	list<PRTrecord> PendingRequestTable_local;

       while(1)
       {


    	   //	cout<<"\n Making Local Cache - PRT \n";
    	       	    pthread_mutex_lock(&mutex_prt);

    	   			   list<PRTrecord>::iterator prtindex = PendingRequestTable.begin();
    	   			   while(prtindex != PendingRequestTable.end()){

    	   				  // PRTrecord record_test_prt = *prtindex;

    	   				   PendingRequestTable_local.push_back(*prtindex);

    	   				   prtindex++;

    	   			   }

    	   				cout<<endl;
    	   				printPRT(RouterID, PendingRequestTable);
    	   				cout<<endl;

    	   		   pthread_mutex_unlock(&mutex_prt);

    	   		 //  cout<<"\n Made Local Cache - PRT \n";


    	   		  // cout<<"\n Making Local Cache - RT \n";
    	   		   pthread_mutex_lock(&mutex_rt);

    	   		   list<RTrecord>::iterator rtindex = RoutingTable.begin();
    	   		   while(rtindex != RoutingTable.end()){

    	   			   //RTrecord record_test_rt = *rtindex;

    	   			   RoutingTable_local.push_back(*rtindex);

    	   			   rtindex++;

    	   		   }


    	   			  cout<<endl;

    	   			  printRT(RouterID, RoutingTable);

    	   			  cout<<endl;

    	   		   pthread_mutex_unlock(&mutex_rt);

    	   		  // cout<<"\n Made Local Cache - RT \n";

    	   		   sleep(2);
    	   //Read or Pop Out from MessageQueue
    	   		if(! messageQueue.pop(p_message))
    	   		{
    	   			continue;
    	   		}


    	   //	cout<<"\n Popped Message from Queue \n";
    	   	//interface = p_message->_RxInterface_Num;



		// cout<<"\n Switching based on packet received. \n";
    	  switch(p_message->_recvPacket.accessHeader()->getOctet(PACKET_TYPE_POS))
		  {



			case REQUEST:
			{
			  cout<<"Processing Request Packet......."<<endl;
			  //sleep(0.5);

			int ContentID = (int)p_message->_recvPacket.accessHeader()->getOctet(CONTENTID_POS);
			int HostID = (int)p_message->_recvPacket.accessHeader()->getOctet(HOSTID_OR_HOPS_POS);

			cout<<ContentID<<endl;
			cout<<HostID<<endl;



				bool test_flag = false;
				PRTrecord record_test_prt;


				//cout << "check for content "<< ContentID << "iterator " <<endl;
				list<PRTrecord>::iterator prtindex = PendingRequestTable_local.begin();
				while(prtindex != PendingRequestTable_local.end()){

					record_test_prt = *prtindex;

					if((record_test_prt.cID == ContentID)){
						  if((record_test_prt.hID == HostID)){

							//  cout<<"Content found "<<endl;
							  test_flag = true;

							  //Also reset its ttl
							  record_test_prt.ttl_val = g_PENDING_TABLE_EXPIRY_TIME;

							  break;
						  }
					}

					prtindex++;
				}



				if(!test_flag){
					//cout<<endl<<"Content not found "<<endl<<endl;
					record_test_prt.cID = ContentID;
					record_test_prt.hID = HostID;
					record_test_prt.ttl_val = g_PENDING_TABLE_EXPIRY_TIME;
					record_test_prt.interface = p_message->_RxInterface_Num;//store the interface

					//PendingRequestTable.push_back(record_test_prt);

				}

				//cout<<endl<<"check in routing table"<<endl<<endl;

					int interface_send = -1;//isContentPresentinRT(ContentID, RoutingTable_local);

					list<RTrecord>::iterator rtindex = RoutingTable_local.begin();
					while(rtindex != RoutingTable_local.end()){

					  if((rtindex->cID == ContentID)){

						  interface_send = rtindex->interface;

						  break;
					   }
					  rtindex++;
					}


					//cout<<endl<<"after check in routing table"<<endl<<endl;

					//cout<<"the interface is "<<interface_send<<endl<<endl;
					if( interface_send!= (-1)){

						  cout<<endl<<"Forwarding Request Through "<<interface_send<<endl;
						  MySendingPorts.at(interface_send)->sendPacket(&p_message->_recvPacket);

					}
					else
						cout<<endl<<"Returned Interface : " << interface_send <<"No valid entry found in the Routing Table "<<interface_send<<endl;



					pthread_mutex_lock(&mutex_prt);
					//cout<<"\n Accq PRT mutex \n";
						if(!test_flag)
							PendingRequestTable.push_back(record_test_prt);
						else
						{
							//cout<<"\n Checking pending table \n";
							list<PRTrecord>::iterator prtindex_ = PendingRequestTable.begin();
							while(prtindex_ != PendingRequestTable.end()){

							 if(prtindex_->cID == record_test_prt.cID && prtindex_->hID == record_test_prt.hID   )
							 {
								// cout<<"\n Update Pending Table"<<endl;
								 prtindex_->ttl_val = g_PENDING_TABLE_EXPIRY_TIME;//record_test_prt.ttl_val;
								 break;
							 }

							 prtindex_++;
							 //cout<<"\n Incremented prtindex_++"<<endl;


							}

						}//else

					 pthread_mutex_unlock(&mutex_prt);

					  cout<<"\n\n Updated Pending Request Table.\n\n";


			  break;
			}

			case RESPONSE:
			{
			  cout<<"Processing Response Packet......."<<endl;



			int ContentID = (int)p_message->_recvPacket.accessHeader()->getOctet(CONTENTID_POS);
			int HostID = (int)p_message->_recvPacket.accessHeader()->getOctet(HOSTID_OR_HOPS_POS);

			PRTrecord record_test_prt;
			bool isChange = false;




				list<PRTrecord>::iterator prtindex_req = PendingRequestTable_local.begin();

				while(prtindex_req != PendingRequestTable_local.end()){

				  record_test_prt = *prtindex_req;

				  if((record_test_prt.cID == ContentID && record_test_prt.hID == HostID )){


						MySendingPorts.at(record_test_prt.interface)->sendPacket(&p_message->_recvPacket);

						//pthread_mutex_lock(&mutex_prt);

						//PendingRequestTable.erase(prtindex_req);
						cout<<endl<<"response packet forwarded through interface "<< record_test_prt.interface<<endl;

						//pthread_mutex_unlock(&mutex_prt);

						//Remove this record from Pending Table
						//cout<<"Remove this request record from Pending Table"<<endl;
						break;

					}

					prtindex_req++;
				  }


					pthread_mutex_lock(&mutex_prt);
					//cout<<"\n Accq PRT mutex for Response\n";
					bool erase_response = false;
						 list<PRTrecord>::iterator prtindex_ = PendingRequestTable.begin();
					 while(prtindex_ != PendingRequestTable.end()){

						 if(prtindex_->cID == record_test_prt.cID && prtindex_->hID == record_test_prt.hID   )
						 {
							// cout<<"\n Erase from PRT flag set\n";
							 erase_response = true;

							 break;
						 }


						 prtindex_++;
						// cout<<"\n incremented prtindex_++ response \n";

					 }
					 if(erase_response)
					 {
						 //PendingRequestTable.erase(prtindex_);
						// cout<<"\n ERASED from PRT (response)\n";
					 }



					 pthread_mutex_unlock(&mutex_prt);

					  cout<<"\n\n Updated Pending Request Table after response.\n\n";




			  break;

			}

			case ANNOUNCEMENT:
			{

				  cout<<"Processing Announcement Packet......."<<endl;



				int ContentID = (int)p_message->_recvPacket.accessHeader()->getOctet(CONTENTID_POS);
				int hops = (int)p_message->_recvPacket.accessHeader()->getOctet(HOSTID_OR_HOPS_POS);


				bool flag = false;
				bool isBroacast = false;
				RTrecord record_test_rt;


				 list<RTrecord>::iterator rtindex = RoutingTable_local.begin();

				//cout<<"Processing Announcement Packet started"<<endl;

				//cout<<"CHecking in LOCAL RT ( announcement )"<<endl;

				  while(rtindex != RoutingTable_local.end()){


						if((rtindex->cID == ContentID)){

						  flag =true;

						  //Also check for Hop count
						  if( hops <= rtindex->num_hops )
						  {
							  rtindex->num_hops = hops;
							  rtindex->interface = p_message->_RxInterface_Num;
							  isBroacast = true;
						  }

						  //Put the ttl value again
						  rtindex->ttl_rt = g_ROUTING_TABLE_EXPIRY_TIME;

						  record_test_rt = *rtindex;
						  break;

						}
						//cout<<"\n incrementing rtindex++ in LOCAL RT ( announcement ) \n";
						rtindex++;

				  }

				 // cout<<"Processing Announcement Packet halfway"<<endl;

				  if(!flag){


					  record_test_rt.cID = ContentID;
					  record_test_rt.num_hops = hops;
					  record_test_rt.ttl_rt = g_ROUTING_TABLE_EXPIRY_TIME;
					  record_test_rt.interface = p_message->_RxInterface_Num;//store the interface

					  isBroacast = true;

					//	cout<<"Processing Announcement Packet pushed"<<endl;

				  }



			  //Router received announcement packet

					p_message->_recvPacket.accessHeader()->setOctet(((unsigned char)(hops+1)),HOSTID_OR_HOPS_POS);

					if(isBroacast)
					{
						for(int index=0; index < num_RX_TX_RemoteInterfaces; index++)
						{

							//cout<<"\n Interface "<< p_message->_RxInterface_Num << " num_RX_TX_RemoteInterfaces "<<num_RX_TX_RemoteInterfaces << " index "<< index <<endl;
						  if(index != p_message->_RxInterface_Num )
							{

							  MySendingPorts.at(index)->sendPacket( &p_message->_recvPacket);
							  cout<<" Broadcasting announcement through interface   "<< index <<endl;
							}

						}

						cout<<" Done announcement...."<<endl;
					}

				  pthread_mutex_lock(&mutex_rt);

				//  cout<<"\n Accq mutex RT for announcement \n";

				  bool erase = false;
				  if(!flag){
					 // cout<<"\n New Entry PUSHED into routing table \n";
					  RoutingTable.push_back(record_test_rt);
				  }
				  else
				  {
					//  cout<<"\n Check in routing table \n";
					 list<RTrecord>::iterator rtindex_ = RoutingTable.begin();
					 while(rtindex_ != RoutingTable.end()){

						 if( rtindex_->cID ==   record_test_rt.cID )
						 {
							 erase = true;

							 break;
						 }

						// cout<<"\n incremented rtindex_++ ( announcement ) \n";
						 rtindex_++;

					 }

					 if(erase){
						RoutingTable.erase(rtindex_);
					//	cout<<"\n ERASED from Routing Table \n";
					  	RoutingTable.push_back(record_test_rt);
					 // 	cout<<"\n ERASED and PUSHED into routing table \n";

					 }
				  }


				 pthread_mutex_unlock(&mutex_rt);

				//  cout<<"\n\n Updated Routing Table.\n\n";

				 cout<<" Finished processing Announcement."<<endl;
			  break;
			}//case

			default:
			  break;

		  }// end of switch



		   RoutingTable_local.clear();
		   PendingRequestTable_local.clear();

       }//end of while(1)






}
