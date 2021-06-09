#pragma once
#ifndef LINEFINDER_H
#define LINEFINDER_H

#ifndef PATTERNSCAN_H
#include "PatternScan.h"
#endif

#define LINEFINDERMAXLINES 100
struct s_linePoint {
    int lunai;/*dominate maski at this point*/
    int hexi;/*index of hex this point sits on*/
    int linei;/*index of line in linefinder that this point belongs to*/
    s_2pt v;/*vector parallel to direction of line*/
    s_2pt perp;/*points from  low val to high*/
    s_2pt loc;/*location of point*/
};

struct s_line {
    int n;
    s_linePoint* pts;
};
class LineFinder :
    public Base
{
public:
    LineFinder();
    ~LineFinder();
    unsigned char init(
        HexStack* stack,
        PatternScan* patScan,
        int stackLevel = 0,
        float minTrigf = 0.3f,
        float minf = 0.1f,
        int minLineSegPts = 6,
        int dLine = 3,
        float loopBackDist=100.f,
        int nloopBackScan = 5
    );
    void release();

    unsigned char spawn();

    inline int getStackLevel() { return m_stackLevel; }
    inline s_line& getLine(int i) { return m_lines[i]; }
    inline s_line* getLines() { return m_lines; }
    inline int getNLines() { return m_n; }
protected:
    int   m_stackLevel;
    float m_minTrigf;/*min f to start a line*/
    float m_minf;/*min f to continue a line*/
    int   m_minLineSegPts;/*should be set to greater than three*/
    int   m_dLine;/*spacing between hexes for actual line saved needs to be at least 1*/
    float m_loopBackDist;/*if points are closer than this distance near the end line is considered a loop back*/
    int   m_nloopBackScan; /*number of final points to scan in loop back check*/

    int   m_numHex;/*total number of base hexes retrieved from the stack*/

    /*owned*/
    bool* m_in_line;
    bool* m_covered;/*mask over hexes that should not be searched again*/
    s_line       m_lines[LINEFINDERMAXLINES];
    int          m_n;
    /*     */
    /*not owned*/
    HexStack*   m_stack;
    s_hex* m_hexes;
    s_luna4fs*  m_luna4s;
    /*         */
    /*utility*/
    s_2pt m_lunaVecs[6];/*unit vectors pointing from low to high for each luna pattern*/
    s_2pt m_lunaVecPerp[6];/*unit vectors that would be the 'x' in a rh coord system to the lunaVecs 'y'*/
    s_2pt m_v60U;/*vector at a 60 deg angle off of straight pointing to the right*/
    /*       */
    /* init   */
    void setConstVectors();
    void genConsUsides(s_2pt vecs[]);
    /*       */

    void reset();
    unsigned char spawnLine(int start_hexi, int& ret_hexi);

    int findLineStart(int start_hexi);
    unsigned char addLinePoint(int lunai, int hexi, s_linePoint linePts[], int& nPts);
    inline void blackout(s_hex& pthex) { m_in_line[pthex.thisLink] = true; }
    unsigned char blackoutSurrounding(s_hex& pthex);
    unsigned char scanNextLink(int webCircleStart_i, s_hex& pthex, int lunai, int& retHexi, int& retlunai);
    unsigned char mergeSegs(s_linePoint lineR[], int nlineR, s_linePoint lineL[], int nlineL, s_line& newLine);
    
    unsigned char formLine(s_line& denseLine, s_line& newLine);
    int needsSplit(s_line& newLine);
    unsigned char splitLine(int i_corner, s_line& fullline, s_line& newLine);
    unsigned char setVectors(s_linePoint& prePt, s_linePoint& postPt, s_linePoint& pt);/*set vectors for point using pre & post info*/

    s_2pt getVectorInLunaU(int reli0, int reli1);
};

#endif
