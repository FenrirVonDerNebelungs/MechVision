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
#ifndef ROACHNET_TRAINTF_H
#include "RoachNet_trainTF.h"
#endif
const unsigned int frame_msg_len = 16384;/*2^14*/
const unsigned int loop_n = 2000;

int testRoachFeed();
int debugStamp();
int debugStamp_rawImg();
int debugTrain();
int debugTrainTF();

int main() {
#ifdef STAMPEYE_DODEBUGIMG
	return debugStamp_rawImg();//testRoachFeed();
#else
	return debugTrainTF();//debugTrain();//debugStamp();
#endif
}
int debugTrainTF() {
	RoachNet_trainTF* trainTF = new RoachNet_trainTF();
	trainTF->init();

	trainTF->gen();

	if (trainTF != NULL) {
		trainTF->release();
		delete trainTF;
	}
	return 0;
}
#ifdef STAMPEYE_DODEBUGIMG
int debugStamp_rawImg() {
	int frame_width_dummy = 640;
	int frame_height_dummy = 480;
	Img dummyImg;
	dummyImg.init(frame_width_dummy, frame_height_dummy, 3L);
	Hex HexLow;
	HexLow.Init(&dummyImg);
	PatternLuna lunaPat;
	lunaPat.init();
	StampEye* stampEy = new StampEye;
	stampEy->init(&lunaPat, &HexLow);
	stampEy->spawn();

	bool loop = true;

	for (int i_stamps = 0; i_stamps < stampEy->numStamps(); i_stamps++) {
		s_eyeStamp& curStamp = stampEy->getEyeStamp(i_stamps);
		for (int i_smearStamp = 0; i_smearStamp < curStamp.n; i_smearStamp++) {
			Img* cur_debugImg = curStamp.imgs[i_smearStamp];
			if (cur_debugImg->getWidth() > 0.f) {
				unsigned char* dispImgDat = cur_debugImg->getImg();
				Size frameSize(cur_debugImg->getWidth(), cur_debugImg->getHeight());
				Mat rendFrame(frameSize, CV_8UC3, (void*)dispImgDat);
				std::cout << "rad: " << curStamp.radius[i_smearStamp] << "  ang: " << curStamp.ang[i_smearStamp]
					<< "  center_ang: " << curStamp.center_ang[i_smearStamp] << "  opening_ang: " << curStamp.opening_ang[i_smearStamp]
					<< "\n";
				do {
					imshow("test stamp", rendFrame);
					char c = (char)waitKey(25);
					if (c == 27)
						break;
				} while (true);
			}
		}
	}

	stampEy->release();
	delete stampEy;
	stampEy = NULL;
	lunaPat.release();
	HexLow.Release();
	dummyImg.release();

	return 0;
}
#endif
int debugTrain() {
	int frame_width = 640;
	int frame_height = 480;
	Img dummyImg;
	dummyImg.init(frame_width, frame_height, 3L);
	Hex HexLow;
	HexLow.Init(&dummyImg);
	PatternLuna lunaPat;
	lunaPat.init();
	StampEye* stampEy = new StampEye;
	stampEy->init(&lunaPat, &HexLow);
	stampEy->spawn();
	HexEye* NNetsPreTrained = new HexEye;
	stampEy->initNNets(NNetsPreTrained);
	EyeNetTrain* preTrain = new EyeNetTrain;
	preTrain->init(stampEy);
	int numNets = NNetsPreTrained->getNEyes();
#ifdef NNETTRAIN_DEBUG
	stampEy->setupForStampi(2);
	preTrain->run(NNetsPreTrained->getEyePtr(2));
	preTrain->writeDump();
#endif
	preTrain->release();
	delete preTrain;
	stampEy->releaseNNets(NNetsPreTrained);
	delete NNetsPreTrained;
	stampEy->release();
	delete stampEy;
	stampEy = NULL;
	lunaPat.release();
	HexLow.Release();
	dummyImg.release();

	return 0;
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
	StampEye* stampEy = new StampEye;
	stampEy->init(&lunaPat, &HexLow);
	stampEy->spawn();
	DrawHexImg hexImg;
	hexImg.Init(&HexLow, stampEy);

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
	stampEy->release();
	delete stampEy;
	stampEy = NULL;
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
