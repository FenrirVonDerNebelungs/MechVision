#include <opencv2/opencv.hpp>

#ifndef HEX_H
#include "Hex.h"
#endif
#ifndef PATLUNALAYER_H
#include "PatLunaLayer.h"
#endif
#ifndef DRIVEPLANE_H
#include "DrivePlane.h"
#endif
#ifndef DRAWHEXIMG_H
#include "DrawHexImg.h"
#endif


using namespace std;
using namespace cv;

int runTest0();
int testPi();
int testLive();
int main() {
	int dummyMarker0;
	int* pdummyMark0 = &dummyMarker0;

	return runTest0();
}
int testLive() {
	VideoCapture cap;
	Mat frame;
	int deviceID = 0;
	int apiID = cv::CAP_ANY;
	cap.open(deviceID, apiID);
	if (!cap.isOpened()) {
		cout << "Error\n";
		return -1;
	}
	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);

	Img MVImg;

	Hex HexLow;
	Img dummyImg;/*dummy image is just used to set dimensions for the first pass generation of the hexes*/
	dummyImg.init(frame_width, frame_height, 3L);
	HexLow.Init(&dummyImg);
	PatLunaLayer patLunLay;
	patLunLay.init(&HexLow);
	LineFinder* findLines = new LineFinder;
	findLines->init(&patLunLay.getPlateLayer(0));
	DrivePlane* driveP = new DrivePlane;
	driveP->init(findLines);

	DrawHexImg hexDrawLines;
	hexDrawLines.Init((HexBase*)&HexLow);

	for (int icnt = 0; icnt < 1000; icnt++) {
		cap.read(frame);
		if (frame.empty()) {
			cerr << "ERROR blank frame grabbed\n";
			break;
		}
		unsigned char* frmdat = frame.data;
		MVImg.initNoOwn(frmdat, frame_width, frame_height, 3L);
		HexLow.Update(&MVImg);

		patLunLay.Update();
		findLines->spawn();
		driveP->update();
		hexDrawLines.renderLineImg(findLines);

		unsigned char* MVImgdat = hexDrawLines.getHexedImg()->getImg();//doCol ? hexBaseDraw.getHexedImg()->getImg() : hexDraw.getHexedImg()->getImg();//MVImg.getImg();
		Size frameSize(frame_width, frame_height);
		Mat rendFrame(frameSize, CV_8UC3, (void*)MVImgdat);

		imshow("TestVid", rendFrame);
		char c = (char)waitKey(25);
		if (c == 27)
			break;
		MVImg.release();
	}
	hexDrawLines.Release();
	driveP->release();
	findLines->release();
	patLunLay.release();
	HexLow.Release();
	dummyImg.release();
	delete driveP;
	delete findLines;

	destroyAllWindows();
	return 0;
}
int testPi() {
	VideoCapture cap("IOFiles/linesV.avi");
	if (!cap.isOpened()) {
		cout << "Error\n";
		return -1;
	}
	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);

	Img MVImg;

	Hex HexLow;
	Img dummyImg;/*dummy image is just used to set dimensions for the first pass generation of the hexes*/
	dummyImg.init(frame_width, frame_height, 3L);
	HexLow.Init(&dummyImg);
	PatLunaLayer patLunLay;
	patLunLay.init(&HexLow);
	LineFinder* findLines = new LineFinder;
	findLines->init(&patLunLay.getPlateLayer(0));
	DrivePlane* driveP = new DrivePlane;
	driveP->init(findLines);

	DrawHexImg hexDrawLines;
	hexDrawLines.Init((HexBase*)&HexLow);
	DrawHexImg hexDrawDriveP;
	hexDrawDriveP.Init(driveP->getHexPlate(0), HexLow.getHexMaskPlus());

	int cnt = 0;
	bool doCol = false;
	while (cnt < 2000) {//000) {
		Mat frame0;
		cap >> frame0;
		if (frame0.empty())
			break;

		unsigned char* frmdat = frame0.data;
		MVImg.initNoOwn(frmdat, frame_width, frame_height, 3L);
		HexLow.Update(&MVImg);

		patLunLay.Update();
		findLines->spawn();
		driveP->update();
		hexDrawLines.renderLineImg(findLines);
		hexDrawDriveP.Run();

		unsigned char* MVImgdat = hexDrawDriveP.getHexedImg()->getImg();//hexDrawLines.getHexedImg()->getImg();////doCol ? hexBaseDraw.getHexedImg()->getImg() : hexDraw.getHexedImg()->getImg();//MVImg.getImg();
		//unsigned char* MVImgdat = doCol ? hexBaseDraw.getHexedImg()->getImg() : hexDraw.getHexedImg()->getImg();//MVImg.getImg();
		Size frameSize(frame_width, frame_height);
		Mat rendFrame(frameSize, CV_8UC3, (void*)MVImgdat);

		imshow("TestVid", rendFrame);
		char c = (char)waitKey(25);
		if (c == 27)
			break;
		MVImg.release();

		cnt++;
	}
	hexDrawDriveP.Release();
	hexDrawLines.Release();
	driveP->release();
	findLines->release();
	patLunLay.release();
	HexLow.Release();
	dummyImg.release();
	delete driveP;
	delete findLines;

	cout << "Hello with OpenCV\n";
	destroyAllWindows();
	return 0;
}
int runTest0() {
	VideoCapture cap("IOFiles/linesV.avi");
	if (!cap.isOpened()) {
		cout << "Error\n";
		return -1;
	}
	int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);

	Img MVImg;
	Hex HexLow;
	Img dummyImg;/*dummy image is just used to set dimensions for the first pass generation of the hexes*/
	dummyImg.init(frame_width, frame_height, 3L);
	HexLow.Init(&dummyImg);

	PatLunaLayer patLunLay;
	patLunLay.init(&HexLow);
	LineFinder findLines;
	findLines.init(&patLunLay.getPlateLayer(0));
	DrivePlane driveP;
	driveP.init(&findLines);


	DrawHexImg hexDraw;
	hexDraw.Init((HexBase*)&HexLow, &(patLunLay.getPlate(0, 2))); //patLunLay.getColPlate(0));//&(patLunLay.getPlate(0,0)));//((HexBase*)&HexLow);
	DrawHexImg hexBaseDraw;
	hexBaseDraw.Init((HexBase*)&HexLow, (patLunLay.getColPlate(0)));
	DrawHexImg hexDrawLines;
	hexDrawLines.Init((HexBase*)&HexLow);
	DrawHexImg hexDrawDriveP;
	hexDrawDriveP.Init(driveP.getHexPlate(0), HexLow.getHexMaskPlus());


	/*debug*/
	//hexDraw.setHexes(colLay.getBaseHexes());
	/*     */

	int cnt = 0;
	bool doCol = false;
	while (cnt<2000){//000) {
		Mat frame0;
		cap >> frame0;
		if (frame0.empty())
			break;
		
		unsigned char* frmdat = frame0.data;
		MVImg.initNoOwn(frmdat, frame_width, frame_height, 3L);
		HexLow.Update(&MVImg);
		//colLay.Update();

		patLunLay.Update();
		findLines.spawn();
		driveP.update();
		hexDraw.Run();
		hexBaseDraw.Run();// renderHexImg();
		hexDrawLines.renderLineImg(&findLines);
		hexDrawDriveP.drawDrivePlates(driveP.getPlates(), DRIVEPLANE_NUMLUNALINE);

		unsigned char* MVImgdat = hexDrawDriveP.getHexedImg()->getImg();//doCol ? hexBaseDraw.getHexedImg()->getImg() : hexDraw.getHexedImg()->getImg();//MVImg.getImg();
		Size frameSize(frame_width, frame_height);
		Mat rendFrame(frameSize, CV_8UC3, (void*)MVImgdat);

		imshow("TestVid", rendFrame);
		char c=(char)waitKey(25);
		if (c == 27)
			break;
		MVImg.release();
		
		cnt++;
	}
	hexDrawDriveP.Release();
	hexDrawLines.Release();
	hexBaseDraw.Release();
	hexDraw.Release();
	driveP.release();
	findLines.release();
	patLunLay.release();

	HexLow.Release();
	dummyImg.release();

	cap.release();

 	cout << "Hello with OpenCV\n";
 	destroyAllWindows();
	return 0;
}