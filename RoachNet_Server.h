#pragma once
#ifndef ROACHNET_SERVER_H
#define ROACHNET_SERVER_H

#ifndef ROACHNET_H
#include "RoachNet.h"
#endif


class RoachNet_Server : public RoachNet {
public:
	RoachNet_Server();
	~RoachNet_Server();
	bool init(const unsigned char msg[], int msg_len); /*initializes server with 1st message from client*/
	void release();/*end of program function for server*/
	bool TransNext(const unsigned char msg[], int msg_len);/*called each loop*/
protected:
	inline float getSteerDist() { return (((ComServer*)m_comm)->getSteer()).dist; };
	inline float getSteerAng() { return (((ComServer*)m_comm)->getSteer()).ang; };
	inline bool getSteerActive() { return (((ComServer*)m_comm)->getSteer()).steerActive; }
};

#endif
