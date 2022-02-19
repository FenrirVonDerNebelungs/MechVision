//#include <opencv2/opencv.hpp>
#ifndef ROACHNET_CLIENT_H
#include "RoachNet_Client.h"
#endif
#ifndef ROACHNET_SERVER_H
#include "RoachNet_Server.h"
#endif

const unsigned int frame_msg_len = 16384;/*2^14*/
const unsigned int loop_n = 2;

int testRoachFeed();

int main() {
	return testRoachFeed();
}
int testRoachFeed() {
	RoachNet_Client rnClient;
	unsigned char init_msg[14];
	if (!rnClient.init_vid(init_msg))
		return 1;

	RoachNet_Server rnServer;
	rnServer.init(init_msg, 14);

	/*loop*/
	unsigned char frame_msg[frame_msg_len];
	for (int icnt = 0; icnt < loop_n; icnt++) {
		int client_ret = -1;
		bool server_ret = true;
		do {
			int client_ret = rnClient.TransNext(frame_msg);
			if (client_ret > 0)
				server_ret = rnServer.RecvNext(frame_msg, client_ret);
			else
				rnServer.update();
		} while (server_ret);
	}

	rnServer.release();
	rnClient.release_vid();
	return 0;
}
