/*
 * Router.h
 *
 *  Created on: Apr 28, 2014
 *      Author: Milin
 */
#include "CommonProject.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lockable_concepts.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/recursive_mutex.hpp>

using namespace boost;

class PendingTableRecord : public basic_lockable_adapter<recursive_mutex>
{

private:
		int RequestedContentID;
		int ContentRequestingHostID;
		int PortInterface;


		//Indicates the validity of the record based on its expiry
		bool isValid;

		//delay used for timer
		struct timespec tdelay_;

		pthread_t TimerThread;

public:


	PendingTableRecord(){}

	PendingTableRecord( int ReqContentID , int ContentReqHostID , int PortNum )
	{
		RequestedContentID  = ReqContentID;
		ContentRequestingHostID = ContentReqHostID;
		PortInterface = PortNum;;
		isValid = true;
		tdelay_.tv_nsec = (long int)((g_PENDING_TABLE_EXPIRY_TIME - (int)g_PENDING_TABLE_EXPIRY_TIME)*1e9);
		tdelay_.tv_sec =  (int)g_PENDING_TABLE_EXPIRY_TIME;

		//mx_ = new boost::mutex;

	}
	~PendingTableRecord(){}




	 int getReqContentID()
	 {
		 //boost::lock_guard<PendingTableRecord> guard(*this);
		 return RequestedContentID;

	 }

	 int getHostID()
	 {
		 //boost::lock_guard<PendingTableRecord> guard(*this);
		 return ContentRequestingHostID;

	 }

	 int getPortInterface()
	 {
		//boost::lock_guard<PendingTableRecord> guard(*this);
		 return PortInterface;

	 }

	 int getValidity()
	 {
	 	boost::lock_guard<PendingTableRecord> guard(*this);
	 	return isValid;

	 }

	 void makeInValid()
	 {
	 	boost::lock_guard<PendingTableRecord> guard(*this);
	 	isValid = false;

	 }
	 void makeValid()
	 {
		 boost::lock_guard<PendingTableRecord> guard(*this);
		 isValid = true;

	 }

	 void setContentID( int cID )
	 {
		 //boost::lock_guard<PendingTableRecord> guard(*this);
		 RequestedContentID = cID;

	 }

	 void setHostID( int HostID )
	 {
		// boost::lock_guard<PendingTableRecord> guard(*this);
		 ContentRequestingHostID = HostID;

	 }

	 void setPortInterface( int interface)
	 {
		// boost::lock_guard<PendingTableRecord> guard(*this);
		 PortInterface = interface;

	 }

	 static void* timerProc_RTR(void* arg)
	{
		PendingTableRecord *ptr = (PendingTableRecord*)arg;
		nanosleep(&(ptr->tdelay_), NULL);
		ptr->makeInValid();
		return NULL;

	}


	void startTimer()
	{
		//Start the Time to Expire thread for each record
		//boost::lock_guard<PendingTableRecord> guard(*this);

		int error = pthread_create(&TimerThread, NULL, &timerProc_RTR, this );
		if (error)
				throw "Pending Table Record Timer thread creation failed...";

	}

	 void stopTimer()
	 {
		// boost::lock_guard<PendingTableRecord> guard(*this);

		 pthread_cancel(TimerThread);
	 }

/*
	 boost::mutex getMutex()
	 {
		 return boost::ref(mx_);
	 }
*/



};

class RoutingTableRecord : public basic_lockable_adapter<boost::recursive_mutex>
{

private:


	int ContentID;
	int PortInterface;
	int NumOfHops;

	//Indicates the validity of the record based on its expiry
	bool isValid;

	//delay used for timer
	struct timespec tdelay_;

	pthread_t TimerThread;

public:


	RoutingTableRecord(){}
	RoutingTableRecord( int ReqContentID , int hops , int PortNum )
	{
		ContentID  = ReqContentID;
		NumOfHops = hops;
		PortInterface = PortNum;
		isValid = true;
		tdelay_.tv_nsec = (long int)((g_ROUTING_TABLE_EXPIRY_TIME - (int)g_ROUTING_TABLE_EXPIRY_TIME)*1e9);
		tdelay_.tv_sec =  (int)g_ROUTING_TABLE_EXPIRY_TIME;

		//mx_ = new boost::mutex;
	}

	~RoutingTableRecord(){}


	 int getContentID()
	 {
		 //boost::lock_guard<RoutingTableRecord> guard(*this);
		 return ContentID;

	 }

	 int getNumOfHops()
	 {
		// boost::lock_guard<RoutingTableRecord> guard(*this);
		 return NumOfHops;

	 }

	 int getPortInterface()
	 {
		 //boost::lock_guard<RoutingTableRecord> guard(*this);
		 return PortInterface;

	 }

	 int getValidity()
	 {
	 	boost::lock_guard<RoutingTableRecord> guard(*this);
	 	return isValid;

	 }

	 void makeInValid()
	 {
	 	boost::lock_guard<RoutingTableRecord> guard(*this);
	 	isValid = false;

	 }

	 void makeValid()
	 {
		 boost::lock_guard<RoutingTableRecord> guard(*this);
		 isValid = true;

	 }

	 void setContentID( int cID )
	 {
		// boost::lock_guard<RoutingTableRecord> guard(*this);
		 ContentID = cID;

	 }

	 void setNumOfHops( int hops )
	 {
		// boost::lock_guard<RoutingTableRecord> guard(*this);
		 NumOfHops = hops;

	 }

	 void setPortInterface( int interface)
	 {
		// boost::lock_guard<RoutingTableRecord> guard(*this);
		 PortInterface = interface;

	 }

	 static void* timerProc_PTR(void* arg)
	{
		RoutingTableRecord *rtr = (RoutingTableRecord*)arg;
		nanosleep(&(rtr->tdelay_), NULL);
		rtr->makeInValid();
		return NULL;

	}


	void startTimer()
	{
		//Start the Time to Expire thread for each record
		//boost::lock_guard<RoutingTableRecord> guard(*this);

		int error = pthread_create(&TimerThread, NULL, &timerProc_PTR, this );
		if (error)
				throw "Routing Table Record Timer thread creation failed...";

	}

	 void stopTimer()
	 {
		 //boost::lock_guard<RoutingTableRecord> guard(*this);

		 pthread_cancel(TimerThread);
	 }

	/* boost::mutex& getMutex()
	 {
		 return  boost::ref(mx_);
	 }
*/


};





class PendingTable
{

	private: std::vector<PendingTableRecord> Table;



public:

	PendingTable(){}


	void DeleteRecord( int RequestedContentID , int ContentRequestingHostID )
	{

			std::vector<PendingTableRecord>::iterator Iter;

			// Remove the Entry from the Pending Table
			for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
			{
				if(Iter->getReqContentID() == RequestedContentID && (Iter)->getHostID() == ContentRequestingHostID )
				{
					boost::lock_guard<PendingTableRecord> lock(*Iter);

					//Stop the timerthread of this record
						Iter->stopTimer();
						// make it Invalid.
						Iter->makeInValid();

					boost::lock_guard<PendingTableRecord> unlock(*Iter);
				}

			}



	}

	bool ReUseExistingInvalidRecord(int RequestedContentID, int ContentRequestingHostID, int incomingRxInterface)
	{
			std::vector<PendingTableRecord>::iterator Iter;

			for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
			{


						if((Iter)->getReqContentID() == RequestedContentID && (Iter)->getHostID() == ContentRequestingHostID)
						{

							boost::lock_guard<PendingTableRecord> lock(*Iter);


									cout<<"\n\t Pending Table Invalid Entry found :\n";
									cout<<"For Content-ID : "<<(Iter)->getReqContentID()<<endl;
									cout<<"For Host-ID : "<<(Iter)->getHostID()<<endl;
									// Making it valid again and Starting its timer.
									(Iter)->setPortInterface( incomingRxInterface );
									(Iter)->makeValid();
									(Iter)->startTimer();

							boost::lock_guard<PendingTableRecord> unlock(*Iter);

							return true;
						}

			}

			return false;


	}



	void InsertIntoPendingTable( int ContentID ,int hostID, int incomingPortInterface )
	{

		try
		{
			std::vector<PendingTableRecord>::iterator Iter;

			//Before Inserting into the Table , check id a similar ( all fields match ) record already
			//	exits wit an isValid == false; If yes , then just make its isValid = true and start its timer
			if ( ! ReUseExistingInvalidRecord( ContentID, hostID, incomingPortInterface )   )
			{

					PendingTableRecord ptr;
					ptr.setContentID(ContentID);
					ptr.setHostID(hostID);
					ptr.setPortInterface(incomingPortInterface);
					ptr.makeValid();

					//Push into the Table
					Table.push_back(ptr);
					//Start its timer
					Table.back().startTimer();

			}
		}
		catch( exception& e)
		{
			cout<<e.what()<<" PT-InsertIntoPendingTable"<<endl;

		}


	}

	bool UpdatePendingTableRecord(int contentID, int reqHostID, int incomingRxInterface)
	{


		std::vector<PendingTableRecord>::iterator Iter;

			for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
			{
				if((Iter)->getReqContentID() == contentID && (Iter)->getHostID() == reqHostID)
				{
					//lock the mutex
					boost::lock_guard<PendingTableRecord> lock(*Iter);

					if(Iter->getValidity() == true)
					{

							cout<<" Pending Request Table Valid (Content+Host) record found :\n";
							// Making it valid again and Starting its timer.
							cout<<" Updating the Interface"<<endl;
							(Iter)->setPortInterface( incomingRxInterface );
							cout<<" Resetting the Timer for the record"<<endl;
							(Iter)->stopTimer();
							(Iter)->startTimer();

							//unlock the mutex
							boost::lock_guard<PendingTableRecord> unlock(*Iter);
							return true;

					}

					//unlock the mutex
					boost::lock_guard<PendingTableRecord> unlock(*Iter);


				}

			}//for

			return false;


	}

	//returns the port interface for the a valid content record in the routing table
	int GetInterfaceFromPendingTable( int ContentID, int reqHostID)
	{

		//Check if the record for ContentID is present in the routing table.
		std::vector<PendingTableRecord>::iterator Iter;
		for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
		{
			//If Record is present
			if((Iter)->getReqContentID() == ContentID  && (Iter)->getHostID() == reqHostID )
			{
				//Accquire the lock on record's mutex
				boost::lock_guard<PendingTableRecord> lock(*Iter);

					if(Iter->getValidity() == true )
					{
						cout<<" Valid record found in the routing table"<<endl;

						//Unlock mutex of record
						boost::lock_guard<PendingTableRecord> unlock(*Iter);

						//return port
						return Iter->getPortInterface();

					}//if validity check

				//Unlock mutex of record
				boost::lock_guard<PendingTableRecord> unlock(*Iter);


			}//if

		}//for

		return -1;

	}



	void PrintPendingRequestTable(int id)
	{

		std::vector<PendingTableRecord>::iterator Iter;

		cout<<"--------------- PENDING TABLE @ ROUTER : "<< id  <<" --------------"<< endl;
		cout<<"Requested Content-ID\t\tRequesting Host-ID\t\tInterface"<<endl;

		for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
		{
			//Accquire the lock on record's mutex
			boost::lock_guard<PendingTableRecord> lock(*Iter);
			if( (Iter)->getValidity() == true)
			{
				//Printing
				cout<<"\t"<<(Iter)->getReqContentID()<<"\t\t\t\t";
				cout<<(Iter)->getHostID()<<"\t\t\t\t";
				cout<<(Iter)->getPortInterface()<<endl;

			}

			//Unlock mutex of record
			boost::lock_guard<PendingTableRecord> unlock(*Iter);
		}
		cout<<"--------------------------------------------------------------------"<< endl;

	}

};


class RoutingTable
{

	private :std::vector<RoutingTableRecord> Table;


public:

	RoutingTable(){}

	void DeleteRecord( int ContentID )
	{
		try{
			std::vector<RoutingTableRecord>::iterator Iter;

			// Remove the Entry from the Pending Table
			for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
			{
				if(Iter->getContentID() == ContentID )
				{
					boost::lock_guard<RoutingTableRecord> lock(*Iter);

					//Stop the timerthread of this record
						Iter->stopTimer();
						// make it Invalid.
						Iter->makeInValid();

					boost::lock_guard<RoutingTableRecord> unlock(*Iter);
				}

			}
		}
		catch( exception& e)
		{
			cout<<e.what()<<" PT-DeleteRecordOnExpiry"<<endl;

		}


	}

	bool ReUseExistingInvalidRecord(int ContentID, int hops , int incomingRxInterface)
	{
		try
		{
			std::vector<RoutingTableRecord>::iterator Iter;

			for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
			{

						if((Iter)->getContentID() == ContentID )
						{

							boost::lock_guard<RoutingTableRecord> lock(*Iter);


									cout<<"\n\t Routing Table Invalid Entry found :\n";
									cout<<"For Content-ID : "<<(Iter)->getContentID()<<endl;

									// Making it valid again and Starting its timer.
									(Iter)->setNumOfHops(hops);
									(Iter)->setPortInterface( incomingRxInterface );
									(Iter)->makeValid();
									(Iter)->startTimer();

							boost::lock_guard<RoutingTableRecord> unlock(*Iter);

							return true;
						}

			}

			return false;

		}
		catch( exception& e)
		{
			cout<<e.what()<<" PT-ReUseExistingInvalidRecord"<<endl;

		}
	}

	void InsertIntoRoutingTable( int ContentID ,int hops, int incomingPortInterface )
	{

		try
		{
			std::vector<RoutingTableRecord>::iterator Iter;

			//Before Inserting into the Table , check id a similar ( all fields match ) record already
			//	exits wit an isValid == false; If yes , then just make its isValid = true and start its timer
			if ( ! ReUseExistingInvalidRecord( ContentID, hops ,incomingPortInterface )   )
			{

				RoutingTableRecord rtr;
				rtr.setContentID(ContentID);
				rtr.setNumOfHops(hops);
				rtr.setPortInterface(incomingPortInterface);
				rtr.makeValid();

				//Push into the Table
				Table.push_back(rtr);
				//Start its timer
				Table.back().startTimer();

			}
		}
		catch( exception& e)
		{
			cout<<e.what()<<" PT-InsertIntoPendingTable"<<endl;

		}

	}


	//this function returns true if there was a record and if its updated/not updated(not required) as well.
	//returns false if not found the record and hence nothing was updated
	bool UpdateRoutingTable(int ContentID, int hops, int incomingRxInterface)
	{
		//Check if the record for ContentID is present in the routing table.
		std::vector<RoutingTableRecord>::iterator Iter;
		for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
		{
			//If Record is present
			if((Iter)->getContentID() == ContentID )
			{
				//Accquire the lock on record's mutex
				boost::lock_guard<RoutingTableRecord> lock(*Iter);

				if(Iter->getValidity() == true )
				{
					if( hops < Iter->getNumOfHops() )
					{
						 //If the existing hops is more than the arrived annuncement's hops
						 //then update the record's hop with this new hop
						 //also update the receiving interface in the routing table
						 //and reset the timer for this record

						cout<<" If the existing hops is more than the arrived annuncement's hops"  <<endl;

						(Iter)->setNumOfHops(hops);
						(Iter)->setPortInterface( incomingRxInterface );

						//Reseting Timer
						(Iter)->stopTimer();
						(Iter)->startTimer();

					}

					//found a valid record , updated/No need to update but broadcast needs to be done , so return true;
					//Unlock mutex of record
					 boost::lock_guard<RoutingTableRecord> unlock(*Iter);
					 return true;


				}//if validity check

				//Invalid record found , nothing to update here now.
				//Unlock mutex of record
				boost::lock_guard<RoutingTableRecord> unlock(*Iter);
				return false;

			}//if

		}//for

			//no update
			return false;

	}


	//returns the port interface for the a valid content record in the routing table
	int GetInterfaceFromRoutingTable( int contentID)
	{

		//Check if the record for ContentID is present in the routing table.
		std::vector<RoutingTableRecord>::iterator Iter;
		for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
		{
			//If Record is present
			if((Iter)->getContentID() == contentID )
			{
				//Accquire the lock on record's mutex
				boost::lock_guard<RoutingTableRecord> lock(*Iter);

					if(Iter->getValidity() == true )
					{
						cout<<" Valid record found in the routing table"<<endl;

						//Unlock mutex of record
						boost::lock_guard<RoutingTableRecord> unlock(*Iter);

						//return port
						return Iter->getPortInterface();

					}//if validity check

				//Unlock mutex of record
				boost::lock_guard<RoutingTableRecord> unlock(*Iter);


			}//if

		}//for

		return -1;

	}

	void PrintRoutingTable(int id)
	{
		std::vector<RoutingTableRecord>::iterator Iter;

		cout<<"--------------- ROUTING TABLE @ ROUTER : "<< id  <<" --------------"<< endl;
		cout<<"Content-ID\t\t#Hops\t\tInterface"<<endl;

		for( Iter = Table.begin(); Iter != Table.end(); Iter++ )
		{
			//Accquire the lock on record's mutex
			boost::lock_guard<RoutingTableRecord> lock(*Iter);

			if( (Iter)->getValidity() == true)
			{
				//Printing
				cout<<"\t"<<(Iter)->getContentID()<<"\t\t";
				cout<<(Iter)->getNumOfHops()<<"\t\t";
				cout<<(Iter)->getPortInterface()<<endl;

			}

			//Unlock mutex of record
			boost::lock_guard<RoutingTableRecord> unlock(*Iter);

		}
		cout<<"--------------------------------------------------------------------"<< endl;

	}

};



