//#include <opencv2/opencv.hpp>
#ifndef ROACHNET_CLIENT_H
#include "RoachNet_Client.h"
#endif
#ifndef ROACHNET_SERVER_H
#include "RoachNet_Server.h"
#endif

const unsigned int frame_msg_len = 16384;/*2^14*/
const unsigned int loop_n = 2000;

int testRoachFeed();

int main() {
	return testRoachFeed();
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
