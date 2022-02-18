#pragma once
#ifndef COMSERVER_H
#define COMSERVER_H
#ifndef COMCLIENT_H
#include "ComClient.h"
#endif
struct s_ComSteer{
	bool steerActive;
	float dist;
	float ang;
};
namespace n_ComSteer {
	void zero(s_ComSteer& s);
}
class ComServer : public Com {
public:
	ComServer();
	~ComServer();
	bool init(const unsigned char msg[], int msg_len);/*msg is what the server recives from the initialized client
											 it should be at least 12 chars long
											 the server uses the msg to determine the width/height of the
											 display and generate the appropriate mesh of hexes and
											 check that the expected number of hexes matches from server to
											 client*/
	void release();

	bool transNext(const unsigned char msg[], int msg_len);
	inline Img* getImg() { return m_img; }
	inline s_ComSteer& getSteer() { return m_steer; }
protected:
	/*owned*/
	Img* m_img;
	s_DrivePlate m_plates[DRIVEPLANE_NUMLUNALINE];/*full or partially filled plates filled from client side data*/
	s_ComSteer m_steer;

	unsigned char initDrivePlates();
	void releaseDrivePlates();

	void reset();
	bool recvSteering(const unsigned char msg[], int msg_len);
	bool recvPlate(const unsigned char msg[], int msg_len);

	s_rgb convertPlateCharToRGB(int plate_i, const unsigned char ch);
	s_rgb genLunaCol(int lunai);
};

#endif