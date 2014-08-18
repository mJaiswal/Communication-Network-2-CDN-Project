#include "CommonProject.h"

using namespace std;
//
void MakeRequestPacket(Packet * reqPacket, int cID , int hID)
{

	reqPacket->accessHeader()->setOctet(REQUEST,PACKET_TYPE_POS);
	reqPacket->accessHeader()->setOctet((unsigned char)cID,CONTENTID_POS);
	reqPacket->accessHeader()->setOctet((unsigned char)hID,HOSTID_OR_HOPS_POS);


}

void MakeResponsePacket(Packet* respPacket, int cID, int hID)
{
	respPacket->accessHeader()->setOctet(RESPONSE,PACKET_TYPE_POS);
	respPacket->accessHeader()->setOctet((unsigned char)cID,CONTENTID_POS);
	respPacket->accessHeader()->setOctet((unsigned char)hID,HOSTID_OR_HOPS_POS);

	//GEt the content data and update the Payload size and Payload

}

void MakeAnnouncementPacket(Packet* announcementPacket, int cID, int hopCount)
{
	announcementPacket->accessHeader()->setOctet(ANNOUNCEMENT,PACKET_TYPE_POS);
	announcementPacket->accessHeader()->setOctet((unsigned char)cID,CONTENTID_POS);
	announcementPacket->accessHeader()->setOctet((unsigned char)hopCount,HOSTID_OR_HOPS_POS);

}

void PrintRequestPacket(Packet* packet)
{


}


