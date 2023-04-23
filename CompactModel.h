// CompactModel.h
#pragma once

#include "Instance.h"

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

using namespace scip;

/**
 * @brief The compact model formulation for the BPP as given in the lecture handout "OR_II_Bin_Packing.pdf"
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
 * @note  * CompactModel is a class that implements the compact model formulation for the Bin Packing Problem (BPP)
 * as given in the lecture handout "OR_II_Bin_Packing.pdf". It has a constructor that takes an Instance pointer as an
 * argument, and a destructor to free up memory. It also has methods to solve the problem, display the solution, and set
 * SCIP parameters. The class contains private variables that are pointers to SCIP environment, solution, instance, and
 * various SCIP variables and constraints. The variables include X_ij (which is equal to 1 in a feasible solution if
 * item i \in I is placed in bin j \in J) and Y_j (which is equal to 1 in a feasible solution if bin j \in J is used).
 * The constraints include unique assignment constraints (every item i \in I is packed in exactly one bin) and capacity
 * and linking constraints (for every bin the sum of the weights of the packed items is less or equal to the bin
 * capacity b, also it ensures that variable Y_j = 0 implies X_ij = 0 for every item i and bin j).
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
   vector<vector<SCIP_VAR*>> _var_X; // X_ij: =1, if item i is placed in bin j
   vector<SCIP_VAR*>         _var_Y; // Y_i:  =1, if bin i is used

   // constraints
   vector<SCIP_CONS*> _cons_capacity_and_linking; // capacity and linking constraint for every bin: bin capacity is
                                                  // respected, item i can only be placed in bin j if bin j is used
                                                  // dimension: number of bins
   vector<SCIP_CONS*> _cons_unique_assignment;    // unique assignment constraint: every item i is placed in exactly one
                                                  // bin j
                                                  // dimension: number of items
};
