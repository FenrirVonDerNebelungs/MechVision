#pragma once
#ifndef COMCLIENT_H
#define COMCLIENT_H
#ifndef COM_H
#include "Com.h"
#endif

class ComClient :public Com {
public:
	ComClient();
	~ComClient();
	bool init(unsigned char msg[],
		Hex* hexAr,
		DrivePlane* DPlane,
		DriveLines* DLines,
		bool send_drive_txt = true,
		bool send_line_img = true);                 /*msg is what the client will sends
													it should be at least 14 chars long and will be filled with the
													header (4 chars), 2 chars for the width and 2 chars for the height,
													4 chars for the number of hexes, 2 chars for the number of msgs*/
	void release();
	int TransNext(unsigned char msg[]);/*puts next message in que to transmit in the array, return msg length*/
protected:
	/*not owned*/
	DrivePlane* m_DPlane;/*pointer to drive plane*/
	s_DrivePlate* m_plates;/*pointer to plates gotten from drive plane*/
	DriveLines* m_DLines;/*pointer to object that finds drive lines*/


	void reset();
	int sendSteering(unsigned char msg[]);/*sends the current steering info 2 chars for non decimal, 2 chars for decimal, and 2 of these four  char sequences making dist and angle*/
	int sendPlate(s_DrivePlate* plates, int Plate_i, unsigned char msg[]); /*returns message length*/
};



#endif
