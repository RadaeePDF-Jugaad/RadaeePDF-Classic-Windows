#include "pch.h"
#include "DXScroller.h"
using namespace RDPDFLib::view;
float DXScroller::VISCOUS_FLUID_NORMALIZE;
float DXScroller::VISCOUS_FLUID_OFFSET;
float DXScroller::SPLINE_POSITION[NB_SAMPLES + 1];
float DXScroller::SPLINE_TIME[NB_SAMPLES + 1];
float DXScroller::DECELERATION_RATE = (float)(log(0.78) / log(0.9));
bool DXScroller::ms_init = false;
