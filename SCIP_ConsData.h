#pragma once

#include "Instance.h"
#include "Pattern.h"

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#include <stack>

using namespace scip;

enum ConsType
{
   SEPARATE = 0,
   TOGETHER = 1
};
typedef enum ConsType CONSTYPE;

// We will have to attach data to the branching constraint. To this end, we create a small structure
// SCIP_ConsData which allows us to store the data. We will then pass a pointer to this structure
// to the function SCIPcreateCons in the constraint handler dealing with Ryan & Foster branching.

struct SCIP_ConsData
{
   int item1; // item i used for branching
   int item2; // item j used for branching
   
   CONSTYPE       _type;        // type of branching: TOGETHER or SEPARATE

   int _nPropagatedVars;      // number of variables that existed, the last time, the related node was
                              // propagated, used to determine whether the constraint should be
                              // repropagated
   int        _nPropagations; // stores the number propagations runs of this constraint
   int        _propagated;    // has this constraint already been propagated?
   SCIP_NODE* _node;          // the node in the B&B-tree at which the cons is sticking
};
