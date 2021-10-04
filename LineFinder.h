#pragma once
#ifndef LINEFINDER_H
#define LINEFINDER_H

#ifndef PATTERNLUNA_H
#include "PatternLuna.h"
#endif

#define LINEFINDERMAXLINES 500
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
    bool* f;
};

class LineFinder : public Base 
{
public:
    LineFinder();
    ~LineFinder();

    unsigned char init(
        s_PlateLayer* plateLayer,
        float minTrigo = 0.3f,//0.3f,
        float mino = 0.1f,
        int minLineSegPts = 8,//6,
        int dLine = 1,//3,
        float loopBackDist = 100.f,
        int nloopBackScan = 5
    );
    void release();

    unsigned char spawn();

    inline s_line& getLine(int i) { return m_lines[i]; }
    inline s_line* getLines() { return m_lines; }
    inline int getNLines() { return m_n; }

protected:
    float m_minTrigo;/*min o to start a line*/
    float m_mino;/*min o to continue a line*/
    int   m_minLineSegPts;/*should be set to greater than three*/
    int   m_dLine;/*spacing between hexes for actual line saved needs to be at least 1*/
    float m_loopBackDist;/*if points are closer than this distance near the end line is considered a loop back*/
    int   m_nloopBackScan; /*number of final points to scan in loop back check*/
    long  m_numHex;/*Number of hexes on plate, maximum number of line points*/

    /*owned*/
    bool* m_in_line;/*points are in line and should not be added to line during search along line*/
    bool* m_covered;/*mask over hexes line & surrounding that should not be used to start line search again*/
    s_line       m_lines[LINEFINDERMAXLINES];
    int          m_n;
    /*owned scratch*/
    s_linePoint* m_lineSegR;
    int m_numLineSegR;
    s_linePoint* m_lineSegL;
    int m_numLineSegL;
    s_line m_scratchLine;
    /*             */
    /*not owned*/
    s_PlateLayer* m_plateLayer;
    /*         */

    /* utility */
    s_2pt m_lunaVecs[6];/*unit vectors pointing from low to high for each luna pattern*/
    s_2pt m_lunaVecPerp[6];/*unit vectors that would be the 'x' in a rh coord system to the lunaVecs 'y'*/
    /*         */
    /*init     */
    void setConstVectors();
    /*         */

    void reset();
    unsigned char spawnLine(int start_hexi, int& ret_hexi);

    int findLineStart(int start_hexi, int& lunaHighest);
    unsigned char addLinePoint(int lunai, int hexi, s_linePoint linePts[], int& nPts);

    unsigned char scanNextLink(int webCircleStart_i, long  hex_i, int lunai, int& retHexi, int& retlunai);
    inline void blackout(long hex_i) { m_in_line[hex_i] = true; }
    unsigned char blackoutSurrounding(long hex_i);

    unsigned char mergeSegs(s_linePoint lineR[], int nlineR, s_linePoint lineL[], int nlineL, s_line& newLine);
    unsigned char formLine(s_line& denseLine, s_line& newLine);
 

    unsigned char setVectors(s_linePoint& prePt, s_linePoint& postPt, s_linePoint& pt);/*set vectors for point using pre & post info*/
};

#endif