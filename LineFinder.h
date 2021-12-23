#pragma once
#ifndef LINEFINDER_H
#define LINEFINDER_H

#ifndef PATTERNLUNA_H
#include "PatternLuna.h"
#endif

#define LINEFINDERMAXLINES 50
#define LINEFINDERMAXLUNADIFF 1
struct s_linePoint {
    float o;
    int lunai;/*dominate maski at this point*/
    long hexi;/*index of hex this point sits on*/
    int linei;/*index of line in linefinder that this point belongs to*/
    s_2pt v;/*vector parallel to direction of line*/
    s_2pt perp;/*points from  low val to high*/
    s_2pt loc;/*location of point*/
};

struct s_line {
    int n;
    s_linePoint* pts;
    bool* f;
    bool blacked;
    bool lowerHalf;
};

namespace n_line{
    inline float dist(const s_linePoint& p1, const s_linePoint& p2) { return vecMath::dist(p1.loc, p2.loc); }
    inline bool isIn(const s_linePoint& p1, const s_linePoint& p2, float d) { return d >= dist(p1, p2); }
    void copyPt(const s_linePoint& p1, s_linePoint& p2);
    void copyLines(const s_line& l1, s_line& l2);
}

class LineFinder : public Base 
{
public:
    LineFinder();
    ~LineFinder();

    unsigned char init(
        s_PlateLayer* plateLayer,
        long spawn_start_hexi = 6000,
        long min_scan_hexi=5000,
        float minTrigo = 0.3f,//0.3f,
        float mino = 0.1f,
        int minLineSegPts = 8,//6,
        int dLine = 1,//3,
        float maxNebDistFac = 3.f,
        long minMergeOverlap = 6,
        float mergeOverlap = 0.6 /*how much merge overlap before one of the lines is removed*/
    );
    void release();

    unsigned char spawn();

    inline s_line* getLine(int i) { return m_lines[i]; }
    inline s_line** getLines() { return m_lines; }
    inline s_line* getSingLunaLines() { return m_singLunaLines; }
    inline int getNSingLunaLines() { return m_n; }
    inline int getNLines() { return m_n_lines; }

protected:
    long m_spawn_start_hexi; /*where the search for new lines starts*/
    long m_min_scan_hexi;/*points below this will not be added to line, way of eliminating hi lines*/
    float m_minTrigo;/*min o to start a line*/
    float m_mino;/*min o to continue a line*/
    int   m_minLineSegPts;/*should be set to greater than three*/
    int   m_dLine;/*spacing between hexes for actual line saved needs to be at least 1*/

    float  m_maxNebDist;/*max distance points may be apart to be considered for merge neighbors*/
    long   m_minMergeOverlap;/*min number of points that lines must overlap to try to merge*/
    float  m_mergeOverlap;/*merge overlap needed to consider both lines the same*/

    long  m_numHex;/*Number of hexes on plate, maximum number of line points*/

    /*owned*/
    bool* m_in_line;/*points are in line and should not be added to line during search along line*/
    bool* m_covered;/*mask over hexes line & surrounding that should not be used to start line search again*/
    s_line       m_singLunaLines[LINEFINDERMAXLINES];
    s_line       m_mergedLines[LINEFINDERMAXLINES];
    s_line*      m_lines[LINEFINDERMAXLINES];/*these pointers point to lines in singLunaLines*/
    int          m_n;
    int          m_n_lines;
    /*owned scratch*/
    s_linePoint* m_lineSegR;
    int m_numLineSegR;
    s_linePoint* m_lineSegL;
    int m_numLineSegL;
    s_line m_scratchLine;
    s_line m_scratchLine1;
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

    int findLineStart(int start_hexi, int& lunaHighest, float& o);
    unsigned char addLinePoint(int lunai, int hexi, float o, s_linePoint linePts[], int& nPts);

    unsigned char scanNextLink(int webCircleStart_i, long  hex_i, int lunai, int& retHexi, int& retlunai, float& o);
    inline void blackout(long hex_i) { m_in_line[hex_i] = true; }
    unsigned char blackoutSurrounding(long hex_i);

    unsigned char mergeSegs(s_linePoint lineR[], int nlineR, s_linePoint lineL[], int nlineL, s_line& newLine);
    //unsigned char formLine(s_line& denseLine, s_line& newLine);
 
    unsigned char setVectorsForLine(s_line* lineptr);
    unsigned char setVectors(s_linePoint& pt);/*set vectors for point using pre & post info*/

    /*merge functions for lines that may change luna*/
    unsigned char mergeLunaLines();/*this should be run on the lines before they are spaced out*/
    bool doMergeLunaLines(const s_line& l, const s_line& c, long& l_i, long& c_i);

    unsigned char mergeLunaLinesForward(int l_i, int c_i, const s_line& l, const s_line& c, s_line& m, bool& selFirst);
    unsigned char mergeLunaLineToTail(const s_line& m, const s_line& c, long c_i, s_line& mm);

    bool lunaNeb(const s_linePoint& p1, const s_linePoint& p2);
    bool neb(const s_linePoint& p1, const s_linePoint& p2);
    bool overlapPts(const s_linePoint& p1, const s_linePoint& p2);

};

#endif