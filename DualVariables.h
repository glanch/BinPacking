#pragma once

#include "Instance.h"
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

using namespace std;
using namespace scip;

// to store the Dual-/Farkas-Values of one Interation of the Column generation-Process
struct DualVariables
{
   DualVariables(Instance* _ins) { onePatternPerItem_pi.resize(_ins->_nbItems, 0); }
   vector<SCIP_Real> onePatternPerItem_pi; // (i) Dual-/ of Farkas-values of the onePatternPerItem-constraint
};