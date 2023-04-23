// CompactModel.cpp  ???
#include "CompactModel.h"
#include <scip/scip_general.h>
#include <scip/scip_prob.h>

/**
 * @brief Construct a new Compact Model:: Compact Model object
 *
 * @param ins pointer to problem-instance
 *
 * @note This code is a constructor for the CompactModel class. It creates a SCIP environment and sets the specific
 * parameters. It then creates and adds all variables to the model, including binary variables X_ij and Y_i for items i,
 * bins j. Finally, it adds all restrictions to the model and writes the final LP-model into a file.
 */
CompactModel::CompactModel(Instance* ins)
{
   _ins = ins; // initialize the private pointer variable to the instance object

   // create a SCIP environment and load all defaults
   SCIPcreate(&_scipCM);
   SCIPincludeDefaultPlugins(_scipCM);

   // create an empty model
   SCIPcreateProbBasic(_scipCM, "Compact Model BPP");

   // set the objective sense to minimize (not mandatory, default is minimize)
   SCIPsetObjsense(_scipCM, SCIP_OBJSENSE_MINIMIZE);

   // call the created function set all optional SCIPParameters
   setSCIPParameters();

   // create helping-dummy for the name of variables and constraints
   char var_cons_name[255];

   //#####################################################################################################################
   // Create and add all variables
   //#####################################################################################################################

   // binary variable Y_j

   // set all dimensions for Y_i, with empty pointers
   _var_Y.resize(_ins->_nbBins);

   for( int j = 0; j < _ins->_nbBins; ++j )
   {
      SCIPsnprintf(var_cons_name, 255, "Y_%d", j); // set name for debugging

      SCIPcreateVarBasic(_scipCM,              //
                         &_var_Y[j],           // returns the address of the newly created variable
                         var_cons_name,        // name
                         0,                    // lower bound
                         1,                    // upper bound
                         1,                    // objective function coefficient, this is equal to 1 according to (1)
                         SCIP_VARTYPE_BINARY); // variable type

      SCIPaddVar(_scipCM, _var_Y[j]); // add var to scip-env
   }

   //#####################################################################################################################
   // binary variable X_ij

   // set all dimensions for X_ij, with empty pointers
   _var_X.resize(_ins->_nbItems); // first dimension of X_ij is equal to the amount of items in this instance

   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      _var_X[i].resize(_ins->_nbItems); // second dimension of X_ij is equal to the amount of bins in this instance
   }
   // create and add the variable Y_ij to the model
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      for( int j = 0; j < _ins->_nbBins; ++j )
      {
         SCIPsnprintf(var_cons_name, 255, "X_%d_%d", i, j); // set name

         SCIPcreateVarBasic(_scipCM,
                            &_var_X[i][j], // returns the address of the newly created variable
                            var_cons_name, // name
                            0,             // lower bound
                            1,             // upper bound
                            0, // objective function coefficient, this is equal to 0 because the variable does not
                               // appear in the objective (1)
                            SCIP_VARTYPE_BINARY); // variable type

         SCIPaddVar(_scipCM, _var_X[i][j]); // add var to scip-env
      }
   }
   //#####################################################################################################################
   // Add restrictions
   //#####################################################################################################################

   //#####################################################################################################################
   // restriction (2) in lecture handout: unique assignment constraints

   // sum(j in J, X_ij) = 1 for all i in I
   // is equal to:
   // 1 <= sum(i in I, X_ij) <= 1 for all i in I

   // set all dimension for constraint with empty pointer

   _cons_unique_assignment.resize(_ins->_nbItems); // dimension is equal to the number of items in theinstance

   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPsnprintf(var_cons_name, 255, "unique_assignment_%d", i);

      SCIPcreateConsBasicLinear(_scipCM,                     // scip
                                &_cons_unique_assignment[i], // cons
                                var_cons_name,               // name
                                0,                           // nvar
                                0,                           // vars
                                0,                           // coeffs
                                1,                           // lhs
                                1);                          // rhs

      for( int j = 0; j < _ins->_nbBins; ++j ) // sum over all bins j in J
      {
         SCIPaddCoefLinear(_scipCM, _cons_unique_assignment[i], _var_X[i][j], 1);
      }

      SCIPaddCons(_scipCM, _cons_unique_assignment[i]);
   }

   //#####################################################################################################################
   // restriction (3) in lecture handout: capacity and linking constraints

   // sum(i in I, w_i * X_ij) <= b * Y_j for all bins j in J
   // is equal to:
   // -infty <= sum(i, w_i * X_ij) - b * Y_j <= 0 for all items i in I, bins j in J

   // set all dimensions
   _cons_capacity_and_linking.resize(_ins->_nbBins,
                                     nullptr); // dimension is equal to the number of bins in this instance

   for( int j = 0; j < _ins->_nbBins; ++j )
   {
      SCIPsnprintf(var_cons_name, 255, "capacity_and_linking_%i", j); // set constraint name for debugging

      SCIPcreateConsBasicLinear(_scipCM,                        // scip
                                &_cons_capacity_and_linking[j], // cons
                                var_cons_name,                  // name
                                0,                              // number of variables
                                0,                              // vars
                                0,                              // coeffs
                                -SCIPinfinity(_scipCM),         // lhs
                                0);                             // rhs

      for( int i = 0; i < _ins->_nbItems; ++i ) // sum over all items i in I
      {
         SCIPaddCoefLinear(_scipCM,                       // scip-env
                           _cons_capacity_and_linking[j], // constraint
                           _var_X[i][j],                  // variable
                           _ins->par_w[i]);               // coefficient
      }
      SCIPaddCoefLinear(_scipCM, _cons_capacity_and_linking[j], _var_Y[j], -ins->par_b);
      SCIPaddCons(_scipCM, _cons_capacity_and_linking[j]); // add constraint to the scip-env
   }

   //#####################################################################################################################
   // Generate LP file
   //#####################################################################################################################

   // Generate a file to show the LP-Program, that is build. "FALSE" = we get our specific choosen names.
   SCIPwriteOrigProblem(_scipCM, "compact_model_bpp.lp", "lp", FALSE);
}

/**
 * @brief Destroy the Compact Model:: Compact Model object
 *
 * @note This is the destructor for the Compact Model class. It releases all constraints and variables associated with
 * the model, and then releases the SCIP object. It releases all NoSelfTour-constraints, shortCycles-constraints,
 * oneTourPerDest-constraints, Arriving-constraints, Leaving-constraints and capacity-constraints. It also releases all
 * X_ijm - variables, Y_im - variables and Z_i - variables. Finally it frees the SCIP object.
 * If you get a:
// "WARNING: Original variable <> not released when freeing SCIP problem <>"
// this is the place to look and check every constraint and variable (yes, also the constraints, it leads to the same
// warning).
 */
CompactModel::~CompactModel()
{
   //#####################################################################################################################
   // release constraints
   //#####################################################################################################################
   // Every constraint that we have generated and stored needs to be released. Thus, use the same for-loops as for
   // generating the constraints to ensure that you release everyone.

   // release all unique assignment constraints
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPreleaseCons(_scipCM, &_cons_unique_assignment[i]);
   }

   // release all capacity and linking constraints

   for( int j = 1; j < _ins->_nbBins; j++ )
   {

      SCIPreleaseCons(_scipCM, &_cons_capacity_and_linking[j]);
   }

   //#####################################################################################################################
   // release all variables
   //#####################################################################################################################

   // release all X_ij - variables

  for( int i = 0; i < _ins->_nbItems; ++i ) // sum over all i in I
   {                       
      for( int j = 0; j < _ins->_nbBins; ++j ) // sum over all j in J
      {
         SCIPreleaseVar(_scipCM, &_var_X[i][j]);
      }
   }

   // release all Y_j - variables
   for( int j = 0; j < _ins->_nbBins; ++j ) // sum over all bins j in J
   {
         SCIPreleaseVar(_scipCM, &_var_Y[j]);
   }

   //#####################################################################################################################
   // release SCIP object
   //#####################################################################################################################
   // At the end release the SCIP object itself
   SCIPfree(&_scipCM);
}

/**
 * @brief set optional SCIP parameters
 *
 * @note This function sets optional SCIP parameters for the CompactModel object. The parameters that are set are
 * "limits/time" to 1e+20 seconds, "limits/gap" to 0, "display/verblevel" to 4, and "display/lpinfo" to FALSE. For more
 * information on these parameters, please refer to the SCIP documentation at
 * https://www.scipopt.org/doc/html/PARAMETERS.php.
 */
void CompactModel::setSCIPParameters()
{
   SCIPsetRealParam(_scipCM, "limits/time", 1e+20);    // default 1e+20 s
   SCIPsetRealParam(_scipCM, "limits/gap", 0);         // default 0
   SCIPsetIntParam(_scipCM, "display/verblevel", 4);   // default 4
   SCIPsetBoolParam(_scipCM, "display/lpinfo", FALSE); // default FALSE
};

/**
 * @brief solve the compact model
 *
 * @note This function solves the compact model using SCIPsolve. It prints a message to the console indicating that it
 * is starting to solve the compact model.
 */
void CompactModel::solve()
{
   cout << "___________________________________________________________________________________________\n";
   cout << "start Solving compact Model: \n";
   SCIPsolve(_scipCM);
};

/**
 * @brief Display every Value of the variables in the optimal solution
 *
 * @note This function displays every value of the variables in the optimal solution of a CompactModel object. It takes
 * no parameters and returns nothing. It uses the SCIPprintBestSol() function from the SCIP library to print out the
 * values.
 */
void CompactModel::displaySolution() { SCIPprintBestSol(_scipCM, NULL, FALSE); };
