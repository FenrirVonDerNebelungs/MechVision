#pragma once
#ifndef COMSERVER_H
#define COMSERVER_H
#ifndef COMCLIENT_H
#include "ComClient.h"
#endif

class ComClient : public Com {
public:
	bool init(const unsigned char msg[], int msg_len);/*msg is what the server recives from the initialized client
											 it should be at least 12 chars long
											 the server uses the msg to determine the width/height of the
											 display and generate the appropriate mesh of hexes and
											 check that the expected number of hexes matches from server to
											 client*/
	void release();

	bool renderPlate_server();
protected:
	bool recvPlate_server(const unsigned char msg[], int msg_len);
	s_DrivePlate m_plates[DRIVEPLANE_NUMLUNALINE];/*full or partially filled plates filled from client side data*/

};

#endif