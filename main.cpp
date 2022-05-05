//#include <opencv2/opencv.hpp>
#ifndef ROACHNET_CLIENT_H
#include "RoachNet_Client.h"
#endif
#ifndef ROACHNET_SERVER_H
#include "RoachNet_Server.h"
#endif
#ifndef DRAWHEXIMG_H
#include "DrawHexImg.h"
#endif

const unsigned int frame_msg_len = 16384;/*2^14*/
const unsigned int loop_n = 2000;

int testRoachFeed();
int debugStamp();

int main() {
	return debugStamp();//testRoachFeed();
}

int debugStamp() {
	int frame_width = 640;
	int frame_height = 480;
	Img dummyImg;
	dummyImg.init(frame_width, frame_height, 3L);
	Hex HexLow;
	HexLow.Init(&dummyImg);
	PatternLuna lunaPat;
	lunaPat.init();
	StampEye stampEy;
	stampEy.init(&lunaPat, 2, 12.f, 5.f, 1, 7, 6.0f, 3.f, &HexLow);
	stampEy.spawn();
	DrawHexImg hexImg;
	hexImg.Init(&HexLow, &stampEy);

	bool loop = true;

	do {
		if (hexImg.Run() != ECODE_OK)
			break;
		unsigned char* dispImgDat = hexImg.getHexedImg()->getImg();
		Size frameSize(frame_width, frame_height);
		Mat rendFrame(frameSize, CV_8UC3, (void*)dispImgDat);
		do {
			imshow("test stamp", rendFrame);
			char c = (char)waitKey(25);
			if (c == 27)
				break;
		} while (true);
	} while (loop);

	hexImg.Release();
	stampEy.release();
	lunaPat.release();
	HexLow.Release();
	dummyImg.release();

	return 0;
}

int testRoachFeed() {
	RoachNet_Client* rnClient;
	rnClient = new RoachNet_Client;
	unsigned char init_msg[14];
	if (!rnClient->init_vid(init_msg))
		return 1;

	RoachNet_Server rnServer;
	rnServer.init(init_msg, 14);

	/*loop*/
	unsigned char frame_msg[frame_msg_len];
	for (int icnt = 0; icnt < loop_n; icnt++) {
		int client_ret = -1;
		bool server_ret = true;
		do {
			int client_ret = rnClient->TransNext(frame_msg);
			if (client_ret > 0)
				server_ret = rnServer.RecvNext(frame_msg, client_ret);
		} while (server_ret);
		char c = (char)waitKey(25);
		if (c == 27)
			break;
	}
	rnClient->end_transmission();

	rnServer.release();
	rnClient->release_vid();
	if (rnClient != NULL)
		delete rnClient;
	return 0;
}
