// CompactModel.h
#pragma once

#include "Instance.h"

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

using namespace scip;

/**
 * @brief The compact model formulation for the VRP as written in Book S. 233
 *
 * @param _scipCM pointer to the scip environment for the compact model
 *
 * @param _sol pointer to the solution of the compact model
 *
 * @param _ins pointer to the instance
 *
 * @param _var pointer to various SCIP-variables
 *
 * @param _cons pointer to various SCIP-constraints
 *
 * @note  * CompactModel is a class that implements the compact model formulation for the Vehicle Routing Problem (VRP)
 * as written in Book S. 233. It has a constructor that takes an Instance pointer as an argument, and a destructor to
 * free up memory. It also has methods to solve the problem, display the solution, and set SCIP parameters. The class
 * contains private variables that are pointers to SCIP environment, solution, instance, and various SCIP variables and
 * constraints. The variables include X_ijm (which is equal to 1 if in Tour m it would be navigated directly from i to
 * j), Y_im (which is equal to 1 if i is in tour m), and Z_i (which is a real-valued auxiliary variable to avoid short
 * cycles). The constraints include capacity constraints (each tour m cannot exceed the capacity b), leaving constraints
 * (each place i of the tour m must be left exactly once), arriving constraints (each place i of the tour m must be
 * arrived exactly once), one tour per destination constraint (each place i without depot must be in exactly one tour
 * m), short cycle avoidance constraints, and no self-tour constraints (forbid in every tour m to drive from i to i).
 */
class CompactModel
{

public:
   // constructor
   CompactModel(Instance* ins);

   // destructor
   ~CompactModel(); 
   
   // solve the problem
   void solve();

   // display the solution
   void displaySolution();

   // set all optional SCIP-Parameters
   void setSCIPParameters();

private:
   SCIP*     _scipCM; // pointer to the scip environment for the compact model
   SCIP_SOL* _sol;    // pointer to the solution of the compact model

   Instance* _ins; // pointer to the instance

   // variables
   vector<vector<vector<SCIP_VAR*>>> _var_X; // X_ijm:  =1, if in Tour m it would be navigated directly from i to j
   vector<vector<SCIP_VAR*>>         _var_Y; // Y_im:   =1, if i is in tour m
   vector<SCIP_VAR*>                 _var_Z; // Z_i:    real-valued auxiliary variable to avoid short cycles

   // constraints
   vector<SCIP_CONS*>         _cons_capacity; // (m) Each tour m can not exceed the capacity b (11.5)
   vector<vector<SCIP_CONS*>> _cons_leaving;  // (i, m) Each place i of the tour m must be left exactly once (11.6)
   vector<vector<SCIP_CONS*>> _cons_arriving; // (i, m) Each place i of the tour m must be arrived exactly once (11.7)
   vector<SCIP_CONS*> _cons_oneTourPerDest;   // (i) Each place i (without depot) must be in exactly one tour m (11.8)
   vector<vector<SCIP_CONS*>> _cons_shortCycles; //(i, j) Avoid short cycles
   vector<vector<SCIP_CONS*>> _cons_noSelfTour;  // (i, m) Forbid in every tour m to drive from i to i
};
