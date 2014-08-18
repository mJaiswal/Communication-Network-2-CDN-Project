/*
 * Router.h
 *
 *  Created on: Apr 28, 2014
 *      Author: Milin
 */
#include "CommonProject.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/recursive_mutex.hpp>



class PendingTableRecord
{

private:
		int RequestedContentID;
		int ContentRequestingHostID;
		int PortInterface;


		//Indicates the validity of the record based on its expiry
		bool isValid;



		pthread_t TimerThread;

		//boost::recursive_mutex  mx_;

public:

	//delay used for timer
	struct timespec tdelay_;

	PendingTableRecord(){}// mx_(new boost::recursive_mutex) {}

	PendingTableRecord( int ReqContentID , int ContentReqHostID , int PortNum )
	{
		RequestedContentID  = ReqContentID;
		ContentRequestingHostID = ContentReqHostID;
		PortInterface = PortNum;;
		isValid = true;
		//tdelay_.tv_nsec = (long int)((g_PENDING_TABLE_EXPIRY_TIME - (int)g_PENDING_TABLE_EXPIRY_TIME)*1e9);
		//tdelay_.tv_sec =  (int)g_PENDING_TABLE_EXPIRY_TIME;


	}
	~PendingTableRecord(){}




	 int getReqContentID()
	 {
		 //boost::lock_guard<boost::mutex> lock(mx_);
		 return RequestedContentID;

	 }

	 int getHostID()
	 {
		 //boost::lock_guard<boost::mutex> lock(mx_);
		 return ContentRequestingHostID;

	 }

	 int getPortInterface()
	 {
		//boost::lock_guard<boost::mutex> lock(mx_);
		 return PortInterface;

	 }

	 int getValidity()
	 {
		//mx_->lock();
		return isValid;
		//mx_->unlock();

	 }

	 void makeInValid()
	 {
		//mx_.lock();
		isValid = false;
		//mx_.unlock();

	 }

	 void makeValid()
	 {
		// mx_.lock();
		 isValid = true;
		// mx_.unlock();

	 }


	 void setContentID( int cID )
	 {
		 //boost::lock_guard<boost::mutex> lock(mx_);
		 RequestedContentID = cID;

	 }

	 void setHostID( int HostID )
	 {
		// boost::lock_guard<boost::mutex> lock(mx_);
		 ContentRequestingHostID = HostID;

	 }

	 void setPortInterface( int interface)
	 {
		// boost::lock_guard<boost::mutex> lock(mx_);
		 PortInterface = interface;

	 }

	 static void* timerProc_RTR(void* arg)
	{
		PendingTableRecord *ptr = (PendingTableRecord*)arg;
		sleep(g_PENDING_TABLE_EXPIRY_TIME);
		ptr->makeInValid();
		cout<<"\n\n Timer Expired for Content "<< ptr->getReqContentID() <<" + "<< ptr->getHostID() <<" - Deleted the pending table record..."<<endl;
		return NULL;

	}


	void startTimer()
	{
		//Start the Time to Expire thread for each record
		//boost::lock_guard<boost::mutex> lock(mx_);

		int error = pthread_create(&TimerThread, NULL, &timerProc_RTR, this );
		if (error)
				throw "Pending Table Record Timer thread creation failed...";

	}

	 void stopTimer()
	 {
		// mx_.lock();

		 pthread_cancel(TimerThread);

		// mx_.unlock();
	 }


	 void Lock()
	 {
		// mx_->lock();
	 }

	 void Unlock()
	 {
		// mx_->unlock();

	 }




};

class RoutingTableRecord
{

private:


	int ContentID;
	int PortInterface;
	int NumOfHops;

	//Indicates the validity of the record based on its expiry
	bool isValid;




	pthread_t TimerThread;

	//boost::recursive_mutex  mx_ ;

public:

	//delay used for timer
	struct timespec tdelay_;


	RoutingTableRecord() {}//mx_(new boost::recursive_mutex) {}
	RoutingTableRecord( int ReqContentID , int hops , int PortNum )
	{
		ContentID  = ReqContentID;
		NumOfHops = hops;
		PortInterface = PortNum;
		isValid = true;
		//tdelay_.tv_nsec = (long int)((g_ROUTING_TABLE_EXPIRY_TIME - (int)g_ROUTING_TABLE_EXPIRY_TIME)*1e9);
		//tdelay_.tv_sec =  (int)g_ROUTING_TABLE_EXPIRY_TIME;

	}

	~RoutingTableRecord(){}


	 int getContentID()
	 {
		 //boost::lock_guard<boost::mutex> lock(mx_);
		 return ContentID;

	 }

	 int getNumOfHops()
	 {
		// boost::lock_guard<boost::mutex> lock(mx_);
		 return NumOfHops;

	 }

	 int getPortInterface()
	 {
		 //boost::lock_guard<boost::mutex> lock(mx_);
		 return PortInterface;

	 }

	 int getValidity()
	 {
		//mx_->lock();
		return isValid;
		//mx_->unlock();

	 }

	 void makeInValid()
	 {
		//mx_.lock();
		isValid = false;
		//mx_.unlock();

	 }

	 void makeValid()
	 {
		// mx_.lock();
		 isValid = true;
		// mx_.unlock();

	 }

	 void setContentID( int cID )
	 {
		// boost::lock_guard<boost::mutex> lock(mx_);
		 ContentID = cID;

	 }

	 void setNumOfHops( int hops )
	 {
		// boost::lock_guard<boost::mutex> lock(mx_);
		 NumOfHops = hops;

	 }

	 void setPortInterface( int interface)
	 {
		// boost::lock_guard<boost::mutex> lock(mx_);
		 PortInterface = interface;

	 }



	 void Lock()
	 {
		// mx_.lock();
	 }

	 void Unlock()
	 {
		// mx_.unlock();

	 }

		void startTimer()
		{

		}

		 void stopTimer()
		 {
			// mx_->lock();

			 pthread_cancel(TimerThread);

			// mx_->unlock();
		 }

};





class PendingTable
{

	private: std::vector<PendingTableRecord> Table;



public:

	PendingTable(){}


	void DeleteRecord( int RequestedContentID , int ContentRequestingHostID )
	{
			//Copy into a local vector


			std::vector<PendingTableRecord>::iterator Iter;

			// Remove the Entry from the Pending Table
			for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
			{
				if(Iter->getReqContentID() == RequestedContentID && (Iter)->getHostID() == ContentRequestingHostID )
				{
						//Iter->Lock();


					//Stop the timerthread of this record
						Iter->stopTimer();
						// make it Invalid.
						Iter->makeInValid();

						//Iter->Unlock();

						break;
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

							//Iter->Lock();


									cout<<"\n\t Pending Table Invalid Entry found :\n";
									cout<<"For Content-ID : "<<(Iter)->getReqContentID()<<endl;
									cout<<"For Host-ID : "<<(Iter)->getHostID()<<endl;
									// Making it valid again and Starting its timer.
									(Iter)->setPortInterface( incomingRxInterface );
									(Iter)->makeValid();
									(Iter)->startTimer();

							//Iter->Unlock();

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
				cout<<" Making a new routing table record"<<endl;
				PendingTableRecord *ptr = new PendingTableRecord();

				//Push into the Table
				Table.push_back(*ptr);

				Table.back().setContentID(ContentID);
				Table.back().setHostID(hostID);
				Table.back().setPortInterface(incomingPortInterface);
				Table.back().makeValid();
				Table.back().startTimer();



				cout<<" Added into routing table Content-ID : "<< Table.back().getReqContentID() <<" and Validity : "<< Table.back().getValidity() << endl;


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
					//Iter->Lock();

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
							//Iter->Unlock();
							return true;

					}

					//unlock the mutex
					//Iter->Unlock();
					break;

				}//if

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
				//Iter->Lock();

					if(Iter->getValidity() == true )
					{
						cout<<" Valid record found in the routing table"<<endl;

						//Unlock mutex of record
						//Iter->Unlock();

						//return port
						return Iter->getPortInterface();

					}//if validity check

				//Unlock mutex of record
					//Iter->Unlock();
					break;


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
			//Iter->Lock();
			if( (Iter)->getValidity() == true)
			{
				//Printing
				cout<<"\t"<<(Iter)->getReqContentID()<<"\t\t\t\t";
				cout<<(Iter)->getHostID()<<"\t\t\t\t";
				cout<<(Iter)->getPortInterface()<<endl;

			}

			//Unlock mutex of record
			//Iter->Unlock();
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
					//Iter->Lock();

					//Stop the timerthread of this record
						Iter->stopTimer();
						// make it Invalid.
						Iter->makeInValid();

					//Unlock
					//Iter->Unlock();

					break;
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

			std::vector<RoutingTableRecord>::iterator Iter;

			for( Iter = Table.begin(); Iter != Table.end(); ++Iter )
			{

						if((Iter)->getContentID() == ContentID )
						{

							//Iter->Lock();

								if(Iter->getValidity() == false)
								{
									cout<<"\n\t Routing Table Invalid Entry found :\n";
									cout<<"For Content-ID : "<<(Iter)->getContentID()<<endl;

									// Making it valid again and Starting its timer.
									(Iter)->setNumOfHops(hops);
									(Iter)->setPortInterface( incomingRxInterface );
									(Iter)->makeValid();
									(Iter)->startTimer();
								}
							//Unlock
							//Iter->Unlock();

							return true;
						}

			}

			return false;


	}

	void InsertIntoRoutingTable( int ContentID ,int hops, int incomingPortInterface )
	{

		try
		{
			std::vector<RoutingTableRecord>::iterator Iter;

			//Before Inserting into the Table , check id a similar ( all fields match ) record already
			//	exits wit an isValid == false; If yes , then just make its isValid = true and start its timer
			cout<<" Check for an existing invalid entry"<<endl;
			if ( ! ReUseExistingInvalidRecord( ContentID, hops ,incomingPortInterface )   )
			{
				cout<<" Making a new routing table record"<<endl;
				RoutingTableRecord *rtr = new RoutingTableRecord();

				//Push into the Table
				Table.push_back(*rtr);

				Table.back().setContentID(ContentID);
				Table.back().setNumOfHops(hops);
				Table.back().setPortInterface(incomingPortInterface);
				Table.back().makeValid();
				Table.back().startTimer();



				cout<<" Added into routing table Content-ID : "<< Table.back().getContentID() <<" and Validity : "<< Table.back().getValidity() << endl;

				//getchar();

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
				//Iter->Lock();

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
						cout<<"Resetting the timer .....";
						(Iter)->stopTimer();
						(Iter)->startTimer();


					}

					//found a valid record , updated/No need to update but broadcast needs to be done , so return true;
					return true;

				}//if validity check
				break;
				//Invalid record found , nothing to update here now.
				//Unlock mutex of record
				//Iter->Unlock();

			}//if content matches

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
				//Iter->Lock();

					if(Iter->getValidity() == true )
					{
						cout<<" Valid record found in the routing table"<<endl;

						//Unlock mutex of record
						//Iter->Unlock();

						//return port
						return Iter->getPortInterface();

					}//if validity check

				//Unlock mutex of record
				//Iter->Unlock();
				break;


			}//if

		}//for
		cout<<" No sending interface found in the routing table."<<endl;
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
			//Iter->Lock();

			if( (Iter)->getValidity() == true)
			{
				//Printing
				cout<<"\t"<<(Iter)->getContentID()<<"\t\t";
				cout<<(Iter)->getNumOfHops()<<"\t\t";
				cout<<(Iter)->getPortInterface()<<endl;

			}

			//Unlock mutex of record
			//Iter->Unlock();

		}
		cout<<"--------------------------------------------------------------------"<< endl;

	}

};



