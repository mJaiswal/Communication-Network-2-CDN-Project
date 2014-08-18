/*
 * Common.h
 *
 *  Created on: Apr 26, 2014
 *      Author: Milin
 */

#include "common.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <time.h>
//#include <pthread.h>


//
#define g_PENDING_TABLE_EXPIRY_TIME 			35 // 10 seconds

#define g_ROUTING_TABLE_EXPIRY_TIME				25 //

#define g_LOSS_PROBABLITY				0

#define g_MESSAGE_Q_CAPACITY				700

#define g_MAX_HOP_COUNT					10

enum HeaderFieldPositions
{
	PACKET_TYPE_POS = 0,
	CONTENTID_POS = 1,
	HOSTID_OR_HOPS_POS = 2

};

enum PacketTypes
{
	REQUEST = 0x00,
	RESPONSE = 0x01,
	ANNOUNCEMENT = 0x02

};

enum ConfigFilePositions
{
	ROUTER_NUM_POS = 0,
	NUM_TX_RX_REMOTE_INTERFACES_POS =1

};

enum HostCmds
{
	GET =1,
	DEL =2,
	PUT =3
};


class Message
{
	public:
	Packet _recvPacket;
	int _RxInterface_Num;

	Message(){}
	Message(int _Interface_Num)
	{
		_RxInterface_Num = _Interface_Num;
	}

};

void MakeRequestPacket(Packet * , int cID , int hID);
void MakeResponsePacket(Packet*, int cID, int hID);
void MakeAnnouncementPacket(Packet* , int cID, int hopCount);


void PrintRequestPacket(Packet*);
void PrintResponsePacket(Packet*);
void PrintAnnouncementPacket(Packet*);


