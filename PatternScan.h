#pragma once
#ifndef PATTERNSCAN_H
#define PATTERNSCAN_H
#ifndef PATTERNNODE_H
#include "PatternNode.h"
#endif
#ifndef NNODEBASE_H
#include "NNodeBase.h"
#endif
#ifndef PATTERNLUNA4_H
#include "PatternLuna4.h"
#endif
#ifndef NNETCOLWHEEL_H
#include "NNetColwheel.h"
#endif

struct s_luna4fs {
	int   lineNodei;
	float edgef;/* the largest result from the 6 half moon patterns*/
	float f[PATTERNLUNA4_NUM];/*the first 6 are from different half moons, index 6 is the full moon index 7 is the black moon*/
};
class PatternScan {
public:
	PatternScan();
	~PatternScan();
	unsigned char init(HexStack* stack);
	void          release();

	unsigned char eval();

	inline HexStack* getHexStack() { return m_hexStack; }
	inline int getNFootNodes() { return m_nfootNodes; }
	inline s_fNode** getFootNodes() { return m_footNodes; }
	inline s_luna4fs* getEdgefs() { return m_edgefs; }
protected:
	/*not owned*/
	HexStack* m_hexStack;
	/*         */
	s_fNode** m_footNodes;
	int       m_nfootNodes;

	int              m_nedgefs;
	s_luna4fs*       m_edgefs;/*each luna pattern contain six orientations corresponding to the 6 orientions of the edge finding 'luna' pattern*/
						      /*the result of these are stored in the first 6 f's*/

	/*owned*/
	NNetTestColwheel* m_colWheel;/*used to evaluate the values for the foot nodes*/
	PatternLuna4* m_patternLuna; /*half moon pattern that is used to scan to look for edges*
	/*     */

	unsigned char attachFootNodes();
	unsigned char evalFootNodes();
	unsigned char putLunaNodeIntoFs(s_pNode* node);
	unsigned char putdummyIntoFs();
};
#endif

