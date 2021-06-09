#include "stdafx.h"
#include "NNodeBase.h"

s_nNode::s_nNode() :nodes(NULL), w(NULL), N(0), f(-1.f), hnodes(NULL), hN(0) { ; }
s_nNode::~s_nNode() { 
	;
}

s_nWrapperNode::s_nWrapperNode() : shex(NULL) { ; }
s_nWrapperNode::~s_nWrapperNode() { 
	;
}
