#include "RoachNet_Server.h"
RoachNet_Server::RoachNet_Server() {
	;
}
RoachNet_Server::~RoachNet_Server() {
	;
}
bool RoachNet_Server::init(const unsigned char msg[], int msg_len) {
	m_comm = (Com*)new ComServer;
	bool msgGood = ((ComServer*)m_comm)->init(msg, msg_len);
	if (!msgGood) {
		release();
		return false;
	}
	m_imgFrameBase = ((ComServer*)m_comm)->getImg();
	m_frame_width = m_imgFrameBase->getWidth();
	m_frame_height = m_imgFrameBase->getHeight();
	return true;
}
void RoachNet_Server::release() {
	m_imgFrameBase = NULL;
	if (m_comm != NULL) {
		m_comm->release();
		delete m_comm;
		m_comm = NULL;
	}
}

bool RoachNet_Server::exFrame(const unsigned char msg[], int msg_len) {
	bool goodmsg = ((ComServer*)m_comm)->transNext(msg, msg_len);
	if (!goodmsg) {
		std::cout << "---------- : --------- \n";
		return false;
	}
	unsigned char* img_dat = m_imgFrameBase->getImg();
	Size frameSize(m_frame_width, m_frame_height);
	Mat frame(frameSize, CV_8UC3, (void*)img_dat);
	imshow("RoachNet", frame);
	float dist = getSteerDist();
	float Ang = getSteerAng();
	std::cout << "d " << dist << " : " << "a " << Ang << "\n";
	return true;
}