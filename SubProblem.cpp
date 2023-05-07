#include "SubProblem.h"

// constructor
SubProblemMIP::SubProblemMIP(Instance* ins) : _ins(ins)
{
   // first generate the Subproblem with the method of the compact Model
   SCIPcreate(&_scipSP);
   SCIPincludeDefaultPlugins(_scipSP);
   SCIPcreateProbBasic(_scipSP, "Subproblem BPP");

   // set all optional SCIPParameters
   SCIPsetIntParam(_scipSP, "display/verblevel", 0);
   SCIPsetBoolParam(_scipSP, "display/lpinfo", FALSE);

   // we do not care about solutions, if these have a not negative optimal objfunc-value
   SCIPsetObjlimit(_scipSP, -SCIPepsilon(_scipSP));

   // create Helping-dummy for the name of variables and constraints
   char var_cons_name[255];

   //############################################################################################################
   // create and add all Variables
   //
   // But all Objective-Function-values are 0!!!!!
   //############################################################################################################

   // add the binary variable X_i: item i is packed into knapsack for all items
   // set dimension for vector of variables: amount of items
   _var_X.resize(_ins->_nbItems);

   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPsnprintf(var_cons_name, 255, "X_%d", i); // set name for debugging

      SCIPcreateVarBasic(_scipSP,
                         &_var_X[i],           // returns the address of the newly created variable
                         var_cons_name,        // name
                         0,                    // lower bound
                         1,                    // upper bound
                         0,                    // objective function coefficient, currently 0, will be updated later
                         SCIP_VARTYPE_BINARY); // variable type

      SCIPaddVar(_scipSP, _var_X[i]); // add newVar to scip-env
   }

   //##################################################################################

   SCIPsnprintf(var_cons_name, 255, "cost_const");

   SCIPcreateVarBasic(_scipSP,
                      &(this->_var_cost_const), // returns the address of the newly created variable
                      var_cons_name,            // name
                      1,                        // lower bound
                      1,                        // upper bound
                      0,                        // objective function coefficient, currently 0, will be updated later
                      SCIP_VARTYPE_CONTINUOUS); // variable type

   SCIPaddVar(_scipSP, _var_cost_const); // add newVar to scip-env
   // #########################################################################################
   // Add restrictions
   //##########################################################################################
   // restriction (13) in lecture handout
   // sum(i, w_i * X_i) <= b
   // is equal to -infty <= sum(i, w_i * X_i) <= b

   _con_capacity = nullptr; // TODO: find out wht this does here and why it may be necessary

   SCIPsnprintf(var_cons_name, 255, "capacity"); // set constraint name for debugging

   SCIPcreateConsBasicLinear(_scipSP,                // scip
                             &_con_capacity,         // cons
                             "con_capacity",         // name
                             0,                      // nvar
                             0,                      // vars
                             0,                      // coeffs
                             -SCIPinfinity(_scipSP), // lhs
                             _ins->par_b);           // rhs

   for( int i = 0; i < _ins->_nbItems; ++i ) // sum over all items i and add pattern_pi[i]*X_i as a term
   {
      SCIPaddCoefLinear(_scipSP,         // scip-env
                        _con_capacity,   // constraint
                        _var_X[i],       // variable
                        _ins->par_w[i]); // coefficient
   }
   SCIPaddCons(_scipSP, _con_capacity); // add constraint to the scip-env

   //##########################################################################################
   // dummy constraint
   // cost_const == 1
   // is equal to 1 <= Cost_const <= 1

   SCIPcreateConsBasicLinear(_scipSP,          // scip
                             &_con_cost_const, // cons
                             "con_cost_const", // name
                             0,                // nvar
                             0,                // vars
                             0,                // coeffs
                             1,                // lhs
                             1);               // rhs
   SCIPaddCoefLinear(_scipSP,                  // scip-env
                     _con_cost_const,          // constraint
                     _var_cost_const,          // variable
                     1);                       // coefficient = 1
}

// destructor
SubProblemMIP::~SubProblemMIP()
{
   //#####################################################################################################################
   // release constraints
   //#####################################################################################################################

   // dummy constraint
   SCIPreleaseCons(_scipSP, &_con_cost_const);

   // capacity constraint
   SCIPreleaseCons(_scipSP, &_con_capacity);

   //#####################################################################################################################
   // release all variables
   //#####################################################################################################################
   // Releasing variables is done in the same way as releasing constraints. Use the same for-loops as for generating the
   // variables and ensure you get everyone.

   // release all X_i
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPreleaseVar(_scipSP, &_var_X[i]);
   }

   // release cost_const
   SCIPreleaseVar(_scipSP, &_var_cost_const);

   //#####################################################################################################################
   // release SCIP object
   //#####################################################################################################################
   // At the end release the SCIP object itself
   SCIPfree(&_scipSP);
}

// update the objective-function of the Subproblem according to the new DualVariables with SCIPchgVarObj()
void SubProblemMIP::updateObjFunc(DualVariables* duals, const bool isFarkas)
{
   {
      SCIPfreeTransform(_scipSP); // enable modifications

      // X_i variables
      for( int i = 0; i < _ins->_nbItems; ++i )
      { 
         // objective value equals negative value of corresponding dual variable
         SCIPchgVarObj(_scipSP, _var_X[i], -duals->onePatternPerItem_pi[i]);
      }

      // dummy variable cost const
      // objective value: 0 if farkas, 1 otherwise
      SCIPchgVarObj(_scipSP, _var_cost_const,  isFarkas ? 0 : 1);
   }
}

SubProblemMIP::solution SubProblemMIP::solve()
{
   // #################################################################################
   // solve the subproblem
   // #################################################################################
   solution sol;

   SCIPwriteOrigProblem(_scipSP, "subProblem.lp", "lp", FALSE);

   SCIPsolve(_scipSP);

   SCIP_SOL* scip_sol = SCIPgetBestSol(_scipSP);

   if( scip_sol == NULL )
   {
      sol.reducedCosts = 0;
      return sol;
   }

   sol.reducedCosts = SCIPgetSolOrigObj(_scipSP, scip_sol);
   sol.BinPatternCost    = 1; // the cost is equal to 1 since a new bin costs exactly 1 cost unit


   sol.BinPattern.resize(_ins->_nbItems, false);
   // store the pattern
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      // Item is part of pattern if corresponding variable in Pricing Sub Problem is bigger than 0.5
       sol.BinPattern[i] = SCIPgetSolVal(_scipSP, scip_sol, _var_X[i]) > 0.5; 
   }

   return sol;
}
