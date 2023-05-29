#pragma once

#include "objscip/objbranchrule.h"
#include "objscip/objscip.h"
#include "scip/scip.h"
#include "scip/scipdefplugins.h"

#include <algorithm>
#include <iostream>

#include "BranchConsHdlr.h"
#include "Master.h"
#include "Pattern.h"

using namespace std;
using namespace scip;

//==============================================
// the used to create the child node of the current node of the B&B tree
// in case of a fractional solution to the LP relaxation, after the
// column generation process has terminated

class BranchRule : public ObjBranchrule
{

public:
   Master*   _pbMaster;
   Instance* _ins;

   const char* _name; // name of the Branchrule
   const char* _desc; // short description of the Branchrule

   BranchRule(Master*     pbMaster,
              const char* p_name,
              const char* p_desc,
              int         priority,
              int         maxDepth,
              double      maxBoundDist);

   ~BranchRule() {}

   virtual SCIP_RETCODE
   scip_execlp(SCIP*            scip,         /**< SCIP data structure */
               SCIP_BRANCHRULE* branchrule,   /**< the branching rule itself */
               SCIP_Bool        allowaddcons, /**< should adding constraints be allowed to avoid a branching? */
               SCIP_RESULT*     result        /**< pointer to store the result of the branching call */
               ) override;

private:
   /**
    * @brief get all patterns, which correspond to lambdas with fractional values
    *
    * @return vector<Pattern*> Pattern with fractional Lambda values
    */
   vector<Pattern*> getFractionalVars();

   /**
    * @brief Get the Branching items object
    *
    * @note Use the first items i and j that one can find
    *
    * @return pair<int, int> items (i, j)
    */
   pair<int, int> getBranchingItems(vector<Pattern*> varFrac);

   SCIP* _scipRMP; // pointer to the scip-env of the master-problem

   /**
    * @brief Check if there is a fractional variable, which contains only items i and not j
    *
    * @param item1 item i
    * @param item2 item j
    *
    * @note if there is a fractional variable, which contains only item i (item1) and not j (item2), return true
    *
    * @return bool
    */
   bool existAlone(int item1, int item2, vector<Pattern*> varFrac);
};
