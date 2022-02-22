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
	if (goodmsg) {
		return true;/*just receive message and go on*/
	}
	else {
		/*either transmission has failed, or the transmssion is over*/
		/*when the steering info is recived the data flag of the comm is set to true and 'false' is returned for the final message*/
		if (((ComServer*)m_comm)->dataFlag()) {
			/*if the data flag is true then render*/
			render();
			/*after the image is rendered the com server no longer needs to hold on to the data*/
			((ComServer*)m_comm)->reset();
		}
	}		
	return false;/*returns false to flag end of transmission beginning of new*/
}
bool RoachNet_Server::render() {
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