// Master.h
#pragma once

#include "Instance.h"

// scip includes
#include "objscip/objbenders.h"
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#include "Pattern.h"
using namespace scip;

/**
 * @brief a class to store all informations for the master problem to solve the BPP with column generation
 *
 * @param _scipRMP pointer to the scip environment for the restricted master-problem
 * @param _ins pointer to the instance
 * @param _var_lambda  decision-variables
 * @param _cons various SCIP constraints
 *
 */
class Master
{
public:
   Master(Instance* ins); // constructor

   ~Master(); // destructor

   SCIP*     _scipRMP; // pointer to the scip environment for the restricted master-problem
   Instance* _ins;     // pointer to the instance

   // Variables
   vector<SCIP_VAR*> _var_lambda; // lambda_p: a vector of all decision-variables lambda
   vector<Pattern*> _Patterns; // a vector of all patterns, corresponding to decision-variables lambda, ordered in time of added

   // constraints
   vector<SCIP_CONS*> _cons_onePatternPerItem; // (i) Each item i must be in exactly one pattern p
   vector<SCIP_CONSDATA*> _cons_branching; // branching constraints for R&F

   // solve the problem void solve();
   void solve();

   // display the solution
   void displaySolution();

   // set all optional SCIP-Parameters
   void setSCIPParameters();
};
