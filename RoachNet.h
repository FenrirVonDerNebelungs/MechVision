#pragma once
#ifndef ROACHNET_H
#define ROACHNET_H

#include <opencv2/opencv.hpp>
#ifndef COMSERVER_H
#include "ComServer.h"
#endif

#ifndef PATLUNALAYER_H
#include "PatLunaLayer.h"
#endif
using namespace std;
using namespace cv;

/*there are two classes that inherit from this: 
RoachNet Client which runs the vision on the robot
&
RoachNet Server which is designed to receive transmissions from the robot and display them
*/
class RoachNet : public Base{
public:
	RoachNet();
	~RoachNet();
	/*frame functions are executed once per loop, loop is intended when not used for test purposes to be
	   run by the python script that calls this program*/
	virtual bool exFrame() { return false; }
protected:
	bool m_data_ex_flag;/*transmission is happening don't input video or render*/
	int m_frame_width;
	int m_frame_height;
	/*the image is owned by RoachNet_Client, but part of the ComClient for RoachNet_Server*/
	Img* m_imgFrameBase; /*image that is setup to be passed into hexLow and used to generate the base hex structure*/
	/*client comm functions*/
	Com* m_comm;
};


#endif


