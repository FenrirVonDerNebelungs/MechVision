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

bool RoachNet_Server::RecvNext(const unsigned char msg[], int msg_len) {
	bool goodmsg = ((ComServer*)m_comm)->recvNext(msg, msg_len);
	/* if goodmsg=true then still receiving data do nothing*/
	return goodmsg;/*if this returns false the the transmissions are comming too fast*/
}
bool RoachNet_Server::update() {
	unsigned char msg[1];
	((ComServer*)m_comm)->recvNext(msg, 0);/*check with dummy message to see if exactly at end of message*/
	/*if the above call resets the dataFlag to true then all messages were received and the data is ready for dump*/
	if (((ComServer*)m_comm)->dataFlag()) {
		/*data is ready to be dumped/rendered*/
		/*render the data*/
		unsigned char* img_dat = m_imgFrameBase->getImg();
		Size frameSize(m_frame_width, m_frame_height);
		Mat frame(frameSize, CV_8UC3, (void*)img_dat);
		imshow("RoachNet", frame);
		float dist = getSteerDist();
		float Ang = getSteerAng();
		if (getSteerActive())
			std::cout << "d " << dist << " : " << "a " << Ang << "\n";
		else
			std::cout << "---------- : --------- \n";
		/* return true at end of cycle */
		return true;
	}
	return false;
}