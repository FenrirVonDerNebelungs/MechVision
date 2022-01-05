#include "DrivePlane.h"

unsigned char DrivePlane::init(
	LineFinder* lineF,
	float yPinHole_screenLowPt,
	float camera_d,
	float camera_y,
	float screen_y_horizion_offset,
	float screen_x_center_offset,
	float screenHYDim
) 
{
	m_cameraTrans = new CameraTrans;

	/*assume that all plates have the same dimension*/
	if (lineF->getPlateLayer()->n < 1)
		return ECODE_FAIL;
	s_hexPlate& dim_plate = lineF->getPlateLayer()->p[0];
	unsigned char errcode= m_cameraTrans->init(dim_plate, yPinHole_screenLowPt, camera_d, camera_y, screen_y_horizion_offset, screen_x_center_offset);
	if (Err(errcode))
		return ECODE_FAIL;
	m_screenClosestY = m_cameraTrans->getScreenLowPt();
	m_screenClosestY_Unit_d = m_cameraTrans->getCamera_d();

	if (screenHYDim >= 1.f) {
		m_screenHYDim = screenHYDim;
	}
	else
		m_screenHYDim = 5.f * m_cameraTrans->getCamera_d();


	for (int i = 0; i < DRIVEPLANE_NUMLUNALINE; i++) {
		m_plates[i].screen_lines = new s_line* [LINEFINDERMAXLINES];
		m_plates[i].screen_lines = 0;
		m_plates[i].maxLinePts = DRIVEPLANE_MAXPTS_X_HEIGHT_FAC * (int)dim_plate.m_height;
		for (int j = 0; j < LINEFINDERMAXLINES; j++) {
			m_plates[i].lines[j].n = 0;
			m_plates[i].lines[j].pts = new s_linePoint[m_plates[i].maxLinePts];
			m_plates[i].lines[j].f = new bool[m_plates[i].maxLinePts];
		}
		m_plates[i].n_lines = 0;
		initDrivePlateHexPlate(dim_plate, m_plates[i].p);
	}


}
unsigned char DrivePlane::initDrivePlateHexPlate(const s_hexPlate& dim_plate, s_hexPlate& p) {
	PatStruct::genPlateWSameWeb(dim_plate, p);
	for (long i = 0; i < p.m_nHex; i++) {
		p.m_fhex[i].o = 0.f;

	}
}
void DrivePlane::reset() {
	for (int i = 0; i < DRIVEPLANE_NUMLUNALINE; i++) {
		m_plates[i].screen_n_lines = 0;
		
	}
}
unsigned char DrivePlane::setPlateForwardSpan(float plateSpanH) {
	if (m_plates[0].p.m_height <= 0)
		return ECODE_FAIL;
	float spanInNormDim = plateSpanH / m_cameraTrans->getCamera_d();
	if (spanInNormDim <= 0.f)
		return ECODE_FAIL;
	m_plateDimToPix = ((float)m_plates[0].p.m_height)/spanInNormDim;
	return ECODE_OK;
}
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

bool DrivePlane::fillPlateHexSpotty(int plate_i, float scaleFac) {
	s_DrivePlate& dp = m_plates[plate_i];
	for (int i = 0; i < dp.n_lines; i++) {
		s_line& line = dp.lines[i];
		for (int pt_i = 0; pt_i < line.n; pt_i++) {
			s_linePoint& pt = line.pts[pt_i];
			s_2pt plateXY;
			if (LineLocToPlateLoc(scaleFac, pt.loc, plateXY)) {
				fillHex(plateXY, pt.o, dp.p);
			}
		}
	}
	return true;
}
bool DrivePlane::LineLocToPlateLoc(float scaleFac, const s_2pt& lineXY, s_2pt& plateXY) {
	plateXY.x1 = lineXY.x1 - m_screenClosestY_Unit_d;
	if (plateXY.x1 < 0.f)
		return false;
	plateXY.x0 = scaleFac * lineXY.x0;
	plateXY.x1 *= scaleFac;
	return true;
}
bool DrivePlane::fillHex(const s_2pt& plateXY, float line_o, s_hexPlate& p) {
	/*find which hex*/
	long hex_i = PatStruct::squarePlate_xyToHexi(p, plateXY);
	if (hex_i < 0)
		return false;
	p.m_fhex[hex_i].o = line_o;
	return true;
}

bool DrivePlane::fillPlateHex(int plate_i, float scaleFac) {
	s_DrivePlate& dp = m_plates[plate_i];
	for (int i = 0; i < dp.n_lines; i++) {
		s_line& line = dp.lines[i];
		int max_pt_i = line.n - 1;
		for (int pt_i = 0; pt_i < max_pt_i; pt_i++) {
			fillLineBetweenPts(scaleFac, line.pts[pt_i], line.pts[pt_i + 1], dp.p);
		}
	}
	return true;
}
bool DrivePlane::fillLineBetweenPts(float scaleFac, const s_linePoint& pt1, const s_linePoint& pt2, s_hexPlate& p) {
	s_2pt U;
	s_2pt perp;
	s_2pt pt1XY;
	s_2pt pt2XY;
	if (!LineLocToPlateLoc(scaleFac, pt1.loc, pt1XY))
		return false;
	if (!LineLocToPlateLoc(scaleFac, pt2.loc, pt2XY))
		return false;
	long hex_start_i = PatStruct::squarePlate_xyToHexi(p, pt1XY);
	if (hex_start_i < 0)
		return false;
	long hex_end_i = PatStruct::squarePlate_xyToHexi(p, pt2XY);
	if (hex_end_i < 0)
		return false;
	float dist = findUsForLinePts(p.m_Rhex, pt1XY, pt2XY, U, perp);
	if (dist < 0.f) {
		p.m_fhex[hex_start_i].o = pt1.o;
		p.m_fhex[hex_end_i].o = pt2.o;
	}
	else {
		int web_start_i = findWebArcStartForLine(p, U);
		long hex_i = hex_start_i;
		for (int cnt = 0; cnt < DRIVEPLANE_MAXLOOPCATCH; cnt++) {
			p.m_fhex[hex_i].o = pt1.o;/*might change this to an average later*/
			long next_hex_i = findNextHexToFill(p, hex_i, web_start_i, pt1XY, perp);
			if (next_hex_i < 0)
				break;
			if (next_hex_i == hex_end_i) {
				p.m_fhex[next_hex_i].o = pt2.o;
				break;
			}
			hex_i = next_hex_i;
		}
	}
	return true;
}
float DrivePlane::findUsForLinePts(float R, const s_2pt& pt1, const s_2pt& pt2, s_2pt& Uline, s_2pt& Uline_perp) {
	s_2pt V = vecMath::v12(pt1, pt2);
	float dist = vecMath::len(V);
	if (dist <= 2.0 * R)
		return -1.f;
	Uline.x0 = V.x0 / dist;
	Uline.x1 = V.x1 / dist;
	s_2pt perp = vecMath::perpUR(Uline);
	Uline_perp.x0 = perp.x0;
	Uline_perp.x1 = perp.x1;
	return dist;
}
long DrivePlane::findNextHexToFill(s_hexPlate& p, long hexi, int hexweb_start, const s_2pt& lineStart, const s_2pt& line_perp) {
	s_fNode& hex = p.m_fhex[hexi];
	long next_hex_i = -1;
	float dist_line_hex0 = DRIVEPLANE_MOREMAXDIM;
	float dist_line_hex1 = DRIVEPLANE_MOREMAXDIM;
	for (int i = 0; i < 3; i++) {
		int web_i = Math::loop(i + hexweb_start, 6);
		s_fNode* adj_hex = (s_fNode*)hex.web[web_i];
		if (adj_hex != NULL) {
			/*this may not be the fastest check all*/
			s_2pt hexXY = { adj_hex->x, adj_hex->y };
			dist_line_hex0 = distHexRough(lineStart, line_perp, hexXY);
			if (inHexRough(p, dist_line_hex0)) {
				if (i < 2) {
					int next_web_i = Math::loop(i + hexweb_start+ 1, 6);
					s_fNode* adj_next_hex = (s_fNode*)hex.web[next_web_i];
					if (adj_next_hex != NULL) {
						s_2pt next_hexXY = { adj_next_hex->x, adj_next_hex->y };
						dist_line_hex1 = distHexRough(lineStart, line_perp, next_hexXY);
						next_hex_i = (dist_line_hex1 < dist_line_hex0) ? adj_next_hex->thislink : adj_hex->thislink;
					}
					else
						next_hex_i = adj_hex->thislink;
				}
				else
					next_hex_i = adj_hex->thislink;
			}
		}
	}
	return next_hex_i;
}
int DrivePlane::findWebArcStartForLine(s_hexPlate& p, const s_2pt& Uline) {
	float max_dotp = 0.f;
	int max_i=-1;
	for (int i = 0; i < 6; i++) {
		float dot_p = vecMath::(Uline, p.m_hexU[i]);
		if (dot_p > max_dotp) {
			max_dotp = dot_p;
			max_i = i;
		}
	}
	int start_i = -1;
	if (max_i >=0) {
		start_i = Math::loop(max_i - 1, 6);
	}
	return start_i;
}
float DrivePlane::distHexRough(const s_2pt& lineStart, const s_2pt& line_perp, const s_2pt& hexXY) {
	s_2pt v_toHex = vecMath::v12(lineStart, hexXY);
	float d = fabsf(vecMath::dot(v_toHex, line_perp));
	return d;
}
void DrivePlane::copyLinePts(const s_linePoint& p1, s_linePoint& p2) {
	p2.o = p1.o;
	p2.lunai = p1.lunai;
	p2.hexi = p1.hexi;
	p2.linei = p1.linei;
	p2.v = p1.v;
	p2.perp = p1.perp;
}