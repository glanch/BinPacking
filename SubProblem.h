#pragma once

#include "DualVariables.h"
#include "Instance.h"
#include "Master.h"
#include "Pattern.h"
#include "SCIP_ConsData.h"


using namespace std;
using namespace scip;

class SubProblemMIP
{
public:
   SubProblemMIP(Instance* ins);
   ~SubProblemMIP();

   // update the objective Function
   void updateObjFunc(DualVariables* duals, const bool isFarkas);

   // solve the problem
   Pattern* solve();

   SCIP*     _scipSP; // pointer to the scip-env of the subproblem
   Instance* _ins;    // pointer to the instance
   // variables
   vector<SCIP_VAR*> _var_X;          // X_i:  =1, if item i is packed into bin / knapsack, =0 otherwise
   SCIP_VAR*         _var_cost_const; // dummy variable to consider a constant term in the Objective-Function

   // constraints
   SCIP_CONS* _con_capacity;   // The capacity of the bin / knapsack is not exceeded

   vector<SCIP_CONS*> _cons_branching;   // ryan-and-foster branching constraints

   void addBranching(SCIP_ConsData* consData); // add a branching constraint
   void deleteLastBranching();  
};