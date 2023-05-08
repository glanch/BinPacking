/* SubProblem.h */

#include "DualVariables.h"
#include "Instance.h"
#include "Master.h"

using namespace std;
using namespace scip;

class SubProblemMIP
{
public:
   // constructor
   SubProblemMIP(Instance* ins);

   // destructor
   ~SubProblemMIP();

   // to store one optimal solution of the Subproblem
   struct solution
   {
      SCIP_Real         reducedCosts;
      SCIP_Real         BinPatternCost;
      vector<SCIP_Bool> BinPattern;
   };

   // update the objective Function
   void updateObjFunc(DualVariables* duals, const bool isFarkas);

   // solve the problem
   solution solve();

   SCIP*     _scipSP; // pointer to the scip-env of the subproblem
   Instance* _ins;    // pointer to the instance
   // variables
   vector<SCIP_VAR*> _var_X;          // X_i:  =1, if item i is packed into bin / knapsack, =0 otherwise
   SCIP_VAR*         _var_cost_const; // dummy variable to consider a constant term in the Objective-Function

   // constraints
   SCIP_CONS* _con_capacity;   // The capacity of the bin / knapsack is not exceeded
};