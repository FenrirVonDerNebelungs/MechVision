#include "RoachNet_Client.h"
RoachNet_Client::RoachNet_Client() : m_vcap(NULL), m_deviceID(0), m_apiID(0),
m_hexLow(NULL), m_lunaLayer(NULL), m_lineFinder(NULL), m_drivePlane(NULL), m_driveLines(NULL),
m_vframe(NULL), m_imgRender(NULL)
{
	;
}
RoachNet_Client::~RoachNet_Client() {
	;
}
bool RoachNet_Client::init(unsigned char msg[],
	int deviceID,
	int apiID)
{
	m_deviceID = deviceID;
	m_apiID = apiID;
	m_vcap = new VideoCapture;
	m_vcap->open(m_deviceID, m_apiID);

	init_vision();/*initalize all the classes that do the image processing*/

	/*comms*/
	m_comm = (Com*)new ComClient;
	bool comOk = ((ComClient*)m_comm)->init(msg, m_hexLow, m_drivePlane, m_driveLines);
	if (!comOk) {
		release();
		return false;
	}
	m_data_ex_flag = false;/*set flag to indicate that the data is not ready to transmit*/
	return true;
}
void RoachNet_Client::release() {
	if (m_comm != NULL) {
		m_comm->release();
		delete m_comm;
		m_comm = NULL;
	}
	release_vision();
	if (m_vcap != NULL) {
		/*should close?*/
		delete m_vcap;
	}
}
bool RoachNet_Client::init_vid(unsigned char msg[])
{
	m_vcap = new VideoCapture("IOFiles/linesV.avi");
	if (!(m_vcap->isOpened())) {
		cout << "Error\n";
		return false;
	}
	init_vision();
	/*comms*/
	m_comm = (Com*)new ComClient;
	bool comOk = ((ComClient*)m_comm)->init(msg, m_hexLow, m_drivePlane, m_driveLines);
	if (!comOk) {
		release();
		return false;
	}
	return true;
}
void RoachNet_Client::release_vid() {
	release();
}
int RoachNet_Client::TransNext(unsigned char msg[]) {
	int msg_len = -1;
	/*find if transmission is taking place*/
	if (!m_data_ex_flag) {
		/*if transmission is not currently taking place, read in a new frame to transmit*/
		m_vcap->read(*m_vframe);
		if (m_vframe->empty())
			return false;
		unsigned char* frame_dat = m_vframe->data;
		m_imgRender->initNoOwn(frame_dat, m_frame_width, m_frame_height, 3L);
		exFrame_vision();
		m_data_ex_flag = true;
		return 0;/*next cycle will start transmitting*/
	}
	/*client transmits data but does not show image*/
	msg_len = ((ComClient*)m_comm)->TransNext(msg);
	if (msg_len < 0) {/*check to see if have just hit a blank transmission, caused by all data being transmitted*/
		end_transmission();/*if m_comm data flag is true then, 
						   releases imgRender, resets m_comm and sets the data_ex_flag to false*/
	}
	return msg_len;
}
bool RoachNet_Client::update() {
	m_vcap->read(*m_vframe);
	if (m_vframe->empty())
		return false;
	unsigned char* frame_dat = m_vframe->data;
	m_imgRender->initNoOwn(frame_dat, m_frame_width, m_frame_height, 3L);
	exFrame_vision();
	m_imgRender->release();
	return true;
}
void RoachNet_Client::end_transmission() {
	if ((ComClient*)m_comm->dataFlag()) {
		/*now that message is transmited the img can be released/reset*/
		m_imgRender->release();
		/*the old render image has been released and the com client should also be reset for next transmission*/
		((ComClient*)m_comm)->reset();
		m_data_ex_flag = false;/*data exchange(transmission) has stopped*/
	}
}

bool RoachNet_Client::init_vision() {
	m_frame_width = (int)m_vcap->get(CAP_PROP_FRAME_WIDTH);
	m_frame_height = (int)m_vcap->get(CAP_PROP_FRAME_HEIGHT);

	m_imgFrameBase = new Img;
	unsigned char err = m_imgFrameBase->init(m_frame_width, m_frame_height, 3L);
	if (Err(err)) {
		release();
		return false;
	}
	m_hexLow = new Hex;
	m_hexLow->Init(m_imgFrameBase);
	m_lunaLayer = new PatLunaLayer;
	m_lunaLayer->init(m_hexLow);
	m_lineFinder = new LineFinder;
	m_lineFinder->init(&(m_lunaLayer->getPlateLayer(0)));
	m_drivePlane = new DrivePlane;
	m_drivePlane->init(m_lineFinder);
	m_driveLines = new DriveLines;
	m_driveLines->init(m_drivePlane);

	m_vframe = new Mat;
	m_imgRender = new Img;/*will be initalized as a "no own" during the loop & released also */
	return true;
}
void RoachNet_Client::release_vision() {
	if (m_imgRender != NULL) {
		m_imgRender->release();
		delete m_imgRender;
		m_imgRender = NULL;
	}
	if (m_vframe != NULL) {
		delete m_vframe;
		m_vframe = NULL;
	}
	if (m_driveLines != NULL) {
		m_driveLines->release();
		delete m_driveLines;
		m_driveLines = NULL;
	}
	if (m_drivePlane != NULL) {
		m_drivePlane->release();
		delete m_drivePlane;
		m_drivePlane = NULL;
	}
	if (m_lineFinder != NULL) {
		m_lineFinder->release();
		delete m_lineFinder;
		m_lineFinder = NULL;
	}
	if (m_lunaLayer != NULL) {
		m_lunaLayer->release();
		delete m_lunaLayer;
		m_lunaLayer = NULL;
	}
	if (m_hexLow != NULL) {
		m_hexLow->Release();
		delete m_hexLow;
		m_hexLow = NULL;
	}
	if (m_imgFrameBase != NULL) {
		m_imgFrameBase->release();
		delete m_imgFrameBase;
		m_imgFrameBase = NULL;
	}
}
unsigned char RoachNet_Client::exFrame_vision() {
	m_hexLow->Update(m_imgRender);
	/*the rest of the functions are constructed so that they retrieve information from the previous object when updated*/
	m_lunaLayer->Update();
	m_lineFinder->spawn();
	m_drivePlane->update();

	return ECODE_OK;
}