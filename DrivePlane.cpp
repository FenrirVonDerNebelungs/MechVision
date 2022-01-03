#include "DrivePlane.h"

unsigned char DrivePlane::convertLines(s_DrivePlate& dp) {
	dp.n_lines = 0;
	for (int i = 0; i < dp.screen_n_lines; i++) {
		s_line* linep = dp.screen_lines[i];
		s_line& planeLine = dp.lines[dp.n_lines];
		bool lineAdded = false;
		planeLine.n = 0;
		for (int j = 0; j < linep->n; j++) {
			if (screenLineCoordToPlaneCoord(linep->pts[j].loc, planeLine.pts[planeLine.n].loc)) {
				lineAdded = true;
				copyLinePts(linep->pts[j], planeLine.pts[planeLine.n]);
				planeLine.n++;
				if (planeLine.n >= dp.maxLinePts)
					break;
			}
			else if (lineAdded) /*if line exits allowed range it is not allowed to enter again*/
				break;
		}
		if (lineAdded) {
			dp.n_lines++;
		}
	}
	return ECODE_OK;
}


void copyLinePts(const s_linePoint& p1, s_linePoint& p2) {
	p2.o = p1.o;
	p2.lunai = p1.lunai;
	p2.hexi = p1.hexi;
	p2.linei = p1.linei;
	p2.v = p1.v;
	p2.perp = p1.perp;
}