// Master.h
#pragma once

#include "Instance.h"

// scip includes
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

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

   // constraints
   vector<SCIP_CONS*> _cons_onePatternPerItem;    // (i) Each item i must be in exactly one pattern p

   // solve the problem void solve();
   void solve();

   // display the solution
   void displaySolution();

   // set all optional SCIP-Parameters
   void setSCIPParameters();
};