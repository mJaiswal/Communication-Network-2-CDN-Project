/*
 * Router.cpp
 *
 *  Created on: Apr 26, 2014
 *      Author: Milin
 */
#include "Router.h"




using namespace std;



//MessageQs for the Rx Thread-Main() communication
boost::lockfree::queue<Message*>messageQueue(g_MESSAGE_Q_CAPACITY);





//----------------------------------------------------------------------------------

void RxThreadFunc( LossyReceivingPort *Rx_port , int RxInterfaceNum )
{
		try
		{

			cout<<"\n Thread Created at Interface : "<< RxInterfaceNum <<endl;
			Message *message = new Message(RxInterfaceNum);
			Packet *recvPkt;

				//Keep Receiving Packet on the RxPort
				while(1)
				{


					recvPkt = Rx_port->receivePacket();

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

		catch(void*)
		{
			cout<<"Could not configure Receiving Interface"<<RxInterfaceNum <<endl;
			cout<<"Abort...";
			exit(0);
		}



	

}


bool HandleAnnouncementPacket ( RoutingTable *rt , Packet *recvPacket , int incomingRxInterface)
{


		int contentID 	= 	(int)recvPacket->accessHeader()->getOctet(CONTENTID_POS);
		int hops 	= 	(int) recvPacket->accessHeader()->getOctet(HOSTID_OR_HOPS_POS);

		//Check and update accordingly if the record for ContentID is present in the routing table.
		bool FoundorUpdated = rt->UpdateRoutingTable(contentID, hops, incomingRxInterface);

		if ( FoundorUpdated == false )
		{
			cout<<" Valid ContentD is not there , so add the entry in the routing table"  <<endl;
			rt->InsertIntoRoutingTable(contentID, hops, incomingRxInterface);

		}
			//Increment the received packet's hop count
			cout<<" Increment the received packet's hop count and put it in the received packet."  <<endl;
			recvPacket->accessHeader()->setOctet((unsigned char)(hops+1), HOSTID_OR_HOPS_POS );

			return true;//Broadcast has to be done


}

int HandleRequestPackets(RoutingTable *rt , PendingTable *pt, Packet *recvPacket , int incomingRxInterface)
{
	int contentID 	= 	(int)recvPacket->accessHeader()->getOctet(CONTENTID_POS);
	int reqHostID 	= 	(int) recvPacket->accessHeader()->getOctet(HOSTID_OR_HOPS_POS);

	cout<<" CHeck in Pending table for an existing request ( Content+Host)"<<endl;
	bool FoundandUpdate = pt->UpdatePendingTableRecord(contentID,reqHostID, incomingRxInterface);

	if( FoundandUpdate == false )
	{
		cout<<" No Record (Content+Host) found  in Pending Table"<<endl;
		cout<<" Add record (Content+Host) Pending Table"<<endl;
		pt->InsertIntoPendingTable( contentID, reqHostID, incomingRxInterface );
	}

	//Now look into the routing table to get the interface to send this packet
	cout<<" Now look into the routing table to get the interface for this Content-ID to forward this request packet" <<endl;
	return rt->GetInterfaceFromRoutingTable(contentID);

}








int main( int argc, const char * argv[]   )
{

	//Router ID
	int RouterID;
	int num_RX_TX_RemoteInterfaces;
	//the Rx Addressess
	vector<int> RxAddressess;
	//the Tx Addressess
	vector<int> TxAddressess;
	//the Tx Addressess
	vector<int> TxRemoteAddressess;
	//Rx Addr
	vector<Address*> RxAddr;
	//Tx Addr
	vector<Address*> TxAddr;
	// Tx Remote Addressess
	vector<Address*> TxRemoteAddr;
	// Tx Ports
	vector<mySendingPort*> MySendingPorts;
	//Rx Ports
	vector<LossyReceivingPort*> MyReceivingPorts;



	//Read From the Cfg File and put the values in the respective global variable . ---------------------------------

	int num =0;
	int index =0;
	vector<int>ConfigurationData;
	ifstream  file(argv[1]);
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
			RxAddr.push_back( new Address("localhost", (short)(num) ) );
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
		file.ignore(numeric_limits<streamsize>::max(), '\n');
		index++;

	}
	//close the file
	file.close();

	//----------------------------------------------------------------------------------------

	//Useless
	//MySendingPorts.push_back(new mySendingPort());
	//MyReceivingPorts.push_back(new LossyReceivingPort(g_LOSS_PROBABLITY));

	//Configure the Sending Ports
	for(int index =0; index < num_RX_TX_RemoteInterfaces; index++ )
	{
		MySendingPorts.push_back(new mySendingPort());
		MySendingPorts.back()->setAddress(TxAddr[index]);
		MySendingPorts.back()->setRemoteAddress(TxRemoteAddr[index]);
		MySendingPorts.back()->init();

	}
	//Configure the Receiving Ports
	for(int index =0; index < num_RX_TX_RemoteInterfaces; index++ )
	{
		MyReceivingPorts.push_back(new LossyReceivingPort(g_LOSS_PROBABLITY));
		MyReceivingPorts.back()->setAddress(RxAddr[index]);
		MyReceivingPorts.back()->init();

	}



	cout<<"----------------\tRouter Configuration Details:\t---------------"<<endl;
	cout<<"Receiving Interfaces\t\tSending Interfaces\t\tDestination Interfaces"<<endl;
	for(int index =0; index < num_RX_TX_RemoteInterfaces; index++ )
	{
		cout<<"\t"<<RxAddressess.at(index)<<"\t\t\t\t";
		cout<<TxAddressess.at(index)<<"\t\t\t\t";
		cout<<TxRemoteAddressess.at(index)<<"\t"<<endl;
	}

	cout<<"Router : " << RouterID << " is configured successfully. Enter to START\n"<<endl;





	//Set the Routing Table and Pending Table objects

	PendingTable *p_PendingTable = new PendingTable();
	RoutingTable *p_RoutingTable = new RoutingTable();

	PendingTable *p_PendingTable_local = new PendingTable();
	RoutingTable *p_RoutingTable_local = new RoutingTable();

	//Creating the Thread Group for Rx Threads.

	//boost::thread_group RxThreads;

	//Based of the number of Interfaces for RxPorts create RxThrads

	for(int num =0 ; num < num_RX_TX_RemoteInterfaces ; ++num )
	{
		cout<<"\n Value of Num : "<<num <<endl;

		boost::thread(RxThreadFunc, MyReceivingPorts.at(num), num );
		sleep(1);
		//RxThreads.create_thread( boost::bind( RxThreadFunc, boost::cref(MyReceivingPorts.at(num)), boost::cref(num) ));
	}

	//For MessageQueues
	Message *p_message = new Message();

	//Spin on the MessageQueue
	while(1)
	{
		cout<<"\n\n";
		p_RoutingTable->PrintRoutingTable(RouterID);
		cout<<"\n\n";
		p_PendingTable->PrintPendingRequestTable(RouterID );
		cout<<"\n\n";

		sleep(1);

		//Read or Pop Out from MessageQueue
		if(! messageQueue.pop(p_message))
		{
			continue;
		}



		switch(p_message->_recvPacket.accessHeader()->getOctet(PACKET_TYPE_POS))
		{

			case REQUEST:
			{
				cout<<"\n Processing Request Packet......."<<endl;


				int TxIndex_ = HandleRequestPackets(p_RoutingTable, p_PendingTable, &p_message->_recvPacket, p_message->_RxInterface_Num );


				if(TxIndex_ != -1)
				{
					cout<<" Forwarding request packet through interface "<< TxIndex_ <<" ..."<<endl;

					MySendingPorts.at(TxIndex_)->sendPacket( &p_message->_recvPacket);
				}
				else
					cout<<" Could not forward requested packet...  "<< TxIndex_ <<endl;

				break;
			}

			case RESPONSE:
			{
				cout<<"\n Processing Response Packet......."<<endl;



				int ContentID = (int)p_message->_recvPacket.accessHeader()->getOctet(CONTENTID_POS);
				int hostID  =  (int)p_message->_recvPacket.accessHeader()->getOctet(HOSTID_OR_HOPS_POS);

				int TxIndex = p_PendingTable->GetInterfaceFromPendingTable( ContentID, hostID);

				if(TxIndex != -1 )
				{

					//No need p_message->_recvPacket.accessHeader()->setOctet( Temp->ContentRequestingHostID, HOSTID_OR_HOPS_POS);

					cout<<" Sending response packet through interface "<< TxIndex <<" ..."<<endl;
					MySendingPorts.at(TxIndex)->sendPacket(&p_message->_recvPacket);

					cout<<" Deleting the record (content+host) from the pending table"<<endl;
					p_PendingTable->DeleteRecord(ContentID,hostID );
				}


				break;
			}

			case ANNOUNCEMENT:
			{

				cout<<"\n Processing Announcement Packet......."<<endl;

				//Router received announcement packet
				bool broadcast = HandleAnnouncementPacket( p_RoutingTable ,&p_message->_recvPacket, p_message->_RxInterface_Num  );

				if(broadcast == true)
				{
					//If Broadcast is true , send it thru all the Tx ports
					cout<<" Broadcast is true , sending it thru all the sending ports .... "<<endl;
					for(int index=0; index < num_RX_TX_RemoteInterfaces; index++)
					{
						MySendingPorts.at(index)->sendPacket( &p_message->_recvPacket);

					}

				}

				break;
			}


			default:
				break;


		}// end of switch


	}

	//All Operations Complete
	return 0;


}


