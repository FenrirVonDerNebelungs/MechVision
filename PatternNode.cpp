#include "stdafx.h"
#include "PatternNode.h"


s_bNode::s_bNode() :f(0.f) { ; }
s_bNode::~s_bNode() { ; }

s_fNode::s_fNode() : shex(NULL), x(0.f), y(0.f) { ; }
s_fNode::~s_fNode() { ; }

s_pNode::s_pNode() : nodes(NULL), w(NULL), N(0) { ; }
s_pNode::~s_pNode() { ; }

s_edgeNode::s_edgeNode() : uPerp_x(0.f), uPerp_y(0.f) { ; }
s_edgeNode::~s_edgeNode() { ; }