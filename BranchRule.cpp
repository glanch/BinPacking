
#include "BranchRule.h"

BranchRule::BranchRule(Master*     pbMaster,
                       const char* p_name,
                       const char* p_desc,
                       int         priority,
                       int         maxDepth,
                       double      maxBoundDist)
      : ObjBranchrule(pbMaster->_scipRMP, p_name, p_desc, priority, maxDepth, maxBoundDist)
{
   _name     = p_name;              // store the name of the pricer
   _desc     = p_desc;              // store the description of the pricer
   _pbMaster = pbMaster;            // store the pointer to the master-problem
   _scipRMP  = _pbMaster->_scipRMP; // construct a shortcut for the pointer to the scip-env of the master-problem
   _ins      = _pbMaster->_ins;     // construct a shortcut for the pointer to the instance,
}

//==========================================================================
// branching execution method for fractional LP solutions
//

//==========================================================================
SCIP_RETCODE
BranchRule::scip_execlp(SCIP* scip, SCIP_BRANCHRULE* branchrule, SCIP_Bool allowaddcons, SCIP_RESULT* result)
{

   cout << "--------------------- branching --------------------------"
        << "\n";
   cout << "branching for node " << SCIPnodeGetNumber(SCIPgetCurrentNode(scip)) << "\n";

   if( SCIPgetBestSol(scip) )
      cout << "objective function value of best integer solution = " << SCIPgetSolOrigObj(scip, SCIPgetBestSol(scip))
           << "\n";
   else
      cout << "still no integer solution"
           << "\n";

   cout << "global LB = " << SCIPgetLowerbound(scip) << "\n";
   cout << "local LB = " << SCIPgetLocalLowerbound(scip) << "\n";

   *result = SCIP_DIDNOTRUN;

   //

   //==============================================================================
   // 1. we search for two items i and j associated to two patterns P1 and P2
   //    so that i is in the intersection of P1 and P2, i.e, i is in BOTH P1 and P2,
   //    whereas j is in P1, but NOT in P2. In addition, the variables related to
   //    P1 and to P2 must both have fractional values.
   //==========================================================================

   auto varFrac = getFractionalVars(); // get all Patterns which correspond to lambdas with fractional values

   if( varFrac.empty() )
      cout << "BranchRule::scip_execlp :there are no fractioned variables -> should we go on with the branching ?"
           << "\n";

   // For each pair (i,j) of items that are both in the Pattern of a fractional variable, we try to find out
   // whether there is another fractional variable for which the corresponding Pattern contains i but not j, or j but
   // not i. As soon as two such items i and j and the Pattern P1 and P2 have been found, we know how to
   // branch and stop the search. Note: We could use more elaborate ways to find the pair (i,j) to branch on!

   auto branchingItems = getBranchingItems(varFrac);
   assert(branchingItems.first != -1 && branchingItems.second != -1);

   cout << "we branch on items i=" << branchingItems.first << " and j=" << branchingItems.second << "\n";

   //============================================================================
   //   2. we create the two child nodes and the associated branching constraints
   //============================================================================

   SCIP_NODE* nodeTogether;  // in this node we will enforce that i and j are either
                             // both in the Pattern or both not in the Pattern
   SCIP_NODE* nodeSeparated; // in this node we enforce that at most one of the
                             // items i and j can be part of the Pattern,
                             // but not both

   SCIP_CONS* consTogether;  // ctr associated to node nodeTogether
   SCIP_CONS* consSeparated; // ctr associated to node nodeSeparated

   // 2.1 we create the child node (here their are two child nodes ... )
   SCIPcreateChild(scip, &nodeTogether, 0.0, SCIPgetLocalTransEstimate(scip));
   SCIPcreateChild(scip, &nodeSeparated, 0.0, SCIPgetLocalTransEstimate(scip));

   cout << "creation of 2 nodes : Together ( node " << SCIPnodeGetNumber(nodeTogether) << ") - separated (node "
        << SCIPnodeGetNumber(nodeSeparated) << ")"
        << "\n";

   // 2.2 we create the branching constraints
   createBranchCtr(scip, &consTogether, branchingItems, TOGETHER, nodeTogether);
   createBranchCtr(scip, &consSeparated, branchingItems, SEPARATE, nodeSeparated);

   // 2.3 we attach the constraints to the respective child nodes
   SCIPaddConsNode(scip, nodeTogether, consTogether, NULL);
   SCIPaddConsNode(scip, nodeSeparated, consSeparated, NULL);

   // 2.4 we free the memory
   SCIPreleaseCons(scip, &consTogether);
   SCIPreleaseCons(scip, &consSeparated);

   *result = SCIP_BRANCHED;

   return SCIP_OKAY;
}

/**
 * Checks whether the given `items` pair exists alone in the `varFrac` vector.
 *
 * @param items the pair of items to check if it exists alone in `varFrac`
 * @param varFrac the vector to search for the given `items` pair
 *
 * @return true if the `items` pair exists alone in the `varFrac` vector, false otherwise
 */

bool BranchRule::existAlone(int item1, int item2, vector<Pattern*> varFrac)
{
   for( auto p : varFrac )
   {
      if( p->containsONE(item1, item2) )
         return true;
   }
   return false;
};

/**
 * Returns a vector of pairs containing the indices (m, p) of all fractional variables
 * in the given 2D vector of SCIP_VAR pointers. A variable is considered
 * fractional if it's exact value is > 0 and < 1.
 * This is achieved by relaying floating point error handling to SCIP
 *
 * @param _lambda_var a 2D vector of SCIP_VAR pointers
 *
 * @return vector of pairs where each pair contains the indices of a fractional variable
 */

vector<Pattern*> BranchRule::getFractionalVars()
{
   vector<Pattern*> varFrac;

   for( auto p : _pbMaster->_Patterns )
   {
      double val = SCIPgetVarSol(_scipRMP, _pbMaster->_var_lambda[p->LambdaPatternIndex]);

      if( !SCIPisIntegral(_scipRMP, val) ) // > 0 and < 1, i.e., fractional
      {
         varFrac.push_back(p);
      }
   }

   return varFrac;
};

/**
 * Returns the branching items for a given vector of fractional variables.
 *
 * @param varFrac the vector of fractional variables to check for branching items
 *
 * @return a pair of integers representing the branching items, or (-1, -1) if none exist
 */

pair<int, int> BranchRule::getBranchingItems(vector<Pattern*> varFrac)
{
   pair<int, int> branchingItems = pair<int, int>(-1, -1);
   for( auto p : varFrac )
   {

      for( auto i : p->includedItems )
      {
         for( auto j : p->includedItems )
         {
            if( i != j )
            {
               // if i exists without j in the Pattern associated with a fractional variable, we keep this pair for
               // the branching
               if( existAlone(i, j, varFrac) )
               {
                  branchingItems.first  = i;
                  branchingItems.second = j;
                  return branchingItems;
               }
            }
         }
      }
   }

   return branchingItems;
}
