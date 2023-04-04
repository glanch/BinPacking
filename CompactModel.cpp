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
 * parameters. It then creates and adds all variables to the model, including binary variables X_ijm, Y_im, and
 * continuous variable Z_i. Finally, it adds all restrictions to the model and writes the final LP-model into a file.
 */
CompactModel::CompactModel(Instance* ins)
{
   _ins = ins; // initialize the private pointer variable to the instance object

   // create a SCIP environment and load all defaults
   SCIPcreate(&_scipCM);
   SCIPincludeDefaultPlugins(_scipCM);

   // create an empty model
   SCIPcreateProbBasic(_scipCM, "Compact Model VRP");

   // set the objective sense to minimize (not mandatory, default is minimize)
   SCIPsetObjsense(_scipCM, SCIP_OBJSENSE_MINIMIZE);

   // call the created function set all optional SCIPParameters
   setSCIPParameters();

   // create helping-dummy for the name of variables and constraints
   char var_cons_name[255];

   //#####################################################################################################################
   // Create and add all variables
   //#####################################################################################################################
   //
   // To create a variable, we first scale our vector to the right size with the resize()-function. If we
   // have a 0-dimensional variable, we do not need to resize. If we have a one-dimensional variable, we resize the
   // vector once. If we have multiple dimensions, we use nested for-loops. In the lowest layer, we set the
   // standard-value to an empty pointer (nullptr). This avoids undefined behavior.
   //
   // The resizing ensured that every variable we want to create has a place for its pointer. In the second step,
   // we than create the variables using nested for-loops and the SCIPcreateVarBasic()-function.
   // In this function, we define the variable-space (such as binary, integer and continuos) and the bounds.
   // In addition, we define the parts of our objective-function in the SCIPcreateVarBasic()-function:
   // For each variable, we set the Objective-Function-coefficients. This means, that we do not need
   // to define the objective function separately.
   //
   // After we created our variable, we need to add them to our Scip-env.

   //#####################################################################################################################
   // binary variable X_ijm

   // set all dimensions for X_ijm, with empty pointers
   _var_X.resize(_ins->_nbDestinations);
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      _var_X[i].resize(_ins->_nbDestinations);
      for( int j = 0; j < _ins->_nbDestinations; ++j )
      {
         _var_X[i][j].resize(_ins->_nbVehicles,
                             nullptr); // set the standard-value to nullptr
      }
   }

   // create and add the variable X_ijm to the model
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int j = 0; j < _ins->_nbDestinations; ++j )
      {
         for( int m = 0; m < _ins->_nbVehicles; ++m )
         {
            SCIPsnprintf(var_cons_name, 255, "X_%d_%d_%d", i, j, m); // set name for debugging

            SCIPcreateVarBasic(_scipCM,              //
                               &_var_X[i][j][m],     // returns the address of the newly created variable
                               var_cons_name,        // name
                               0,                    // lower bound
                               1,                    // upper bound
                               _ins->par_c[i][j],    // objective function coefficient
                               SCIP_VARTYPE_BINARY); // variable type

            SCIPaddVar(_scipCM, _var_X[i][j][m]); // add var to scip-env
         }
      }
   }

   //#####################################################################################################################
   // binary variable Y_im

   // set all dimensions for Y_im, with empty pointers
   _var_Y.resize(_ins->_nbDestinations);
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      _var_Y[i].resize(_ins->_nbVehicles, nullptr);
   }

   // create and add the variable Y_im to the model
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPsnprintf(var_cons_name, 255, "Y_%d_%d", i, m); // set name

         SCIPcreateVarBasic(_scipCM,
                            &_var_Y[i][m],        // returns the address of the newly created variable
                            var_cons_name,        // name
                            0,                    // lower bound
                            1,                    // upper bound
                            0,                    // objective function coefficient
                            SCIP_VARTYPE_BINARY); // variable type

         SCIPaddVar(_scipCM, _var_Y[i][m]); // add var to scip-env
      }
   }

   //#####################################################################################################################
   // add the continuos variable Z_i

   _var_Z.resize(_ins->_nbDestinations,
                 nullptr); // set one dimension for Z_i (one Z for each i)

   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      SCIPsnprintf(var_cons_name, 255, "Z_%d", i);

      SCIPcreateVarBasic(_scipCM,
                         &_var_Z[i],               // returns the address of the newly created variable
                         var_cons_name,            // name
                         -SCIPinfinity(_scipCM),   // lower bound
                         SCIPinfinity(_scipCM),    // upper bound
                         0,                        // objective function coefficient
                         SCIP_VARTYPE_CONTINUOUS); // variable type

      SCIPaddVar(_scipCM, _var_Z[i]); // add var to scip-env
   }

   //#####################################################################################################################
   // Add restrictions
   //#####################################################################################################################
   //
   // To add an restriction, we follow the method as for adding variables. First, we need to resize the correspondng
   // vector for the pointers. Afterwards, we add each constraint. For constraints that are set for various indices, we
   // use for-loops.
   //
   // For SCIP, every constraint must be formulated in the following way: lhs <= term <= rhs,
   // "lhs" is the lowerbound of this constraint, and "rhs" is the upperbound. An example for the "rhs" is an
   // capacity-restriction. Every variable with their coefficient is added to the "term". Assume we have a constraint
   // with a parameter b. We have three common contraint-types:
   // term >= b : we set lhs = b,         rhs = infinity
   // term <= b : we set lhs = -infinity, rhs = b
   // term = b  : we set lhs = b,         rhs = b
   //
   // We obtain our parameters from our included instance _ins->par. As always, if we need to address one specific-value
   // from a vector, we use [] to indicate the location.
   // Variables are added with the SCIPaddCoefLinear() function.
   // If we want to add a sum of variables to a contraint, we use a for-loop over these variables and add them with
   // their coefficients with SCIPaddCoefLinear().
   // After every coefficient is added, we add the constraint to our model with SCIPaddCons().

   //#####################################################################################################################
   // restriction 11.5 in Book, but with b_m, because symmetry is a special case

   // sum(i in I, w_i * Y_im ) <= b_m for all m
   // is equal to:
   // -inf <= sum(i in I, w_i * Y_im ) <= b_m for all m

   // set all dimension for constraint with empty pointer
   _cons_capacity.resize(_ins->_nbVehicles, nullptr);

   for( int m = 0; m < _ins->_nbVehicles; ++m )
   {
      SCIPsnprintf(var_cons_name, 255, "capacity_%d", m); // set constraint name for debugging

      SCIPcreateConsBasicLinear(_scipCM,                // scip
                                &_cons_capacity[m],     // cons
                                var_cons_name,          // name
                                0,                      // number of variables
                                0,                      // vars
                                0,                      // coeffs
                                -SCIPinfinity(_scipCM), // lhs
                                _ins->par_b[m]);        // rhs

      for( int i = 0; i < _ins->_nbDestinations; ++i ) // sum over all i
      {
         SCIPaddCoefLinear(_scipCM,           // scip-env
                           _cons_capacity[m], // constraint
                           _var_Y[i][m],      // variable
                           _ins->par_w[i]);   // coefficient
      }
      SCIPaddCons(_scipCM, _cons_capacity[m]); // add constraint to the scip-env
   }

   //#####################################################################################################################
   // restriction 11.6 in Book

   // sum(j, X_ijm) = Y_im for all i, m
   // is equal to:
   // 0 <= sum(j, X_ijm) - Y_im <= 0 for all i, m

   // set all dimensions
   _cons_leaving.resize(_ins->_nbDestinations);
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      _cons_leaving[i].resize(_ins->_nbVehicles);
   }

   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPsnprintf(var_cons_name, 255, "leaving_%d_%d", i, m);

         SCIPcreateConsBasicLinear(_scipCM,              // scip
                                   &_cons_leaving[i][m], // cons
                                   var_cons_name,        // name
                                   0,                    // nvar
                                   0,                    // vars
                                   0,                    // coeffs
                                   0,                    // lhs
                                   0);                   // rhs

         for( int j = 0; j < _ins->_nbDestinations; ++j ) // sum over all j
         {
            SCIPaddCoefLinear(_scipCM, _cons_leaving[i][m], _var_X[i][j][m], 1);
         }
         SCIPaddCoefLinear(_scipCM, _cons_leaving[i][m], _var_Y[i][m], -1);

         SCIPaddCons(_scipCM, _cons_leaving[i][m]);
      }
   }

   //#####################################################################################################################
   // restriction 11.7 in Book

   // sum(i, X_ijm) = Y_jm for all j, m
   // is equal to:
   // 0 <= sum(i, X_ijm) - Y_jm <= 0 for all j, m

   // set all dimensions
   _cons_arriving.resize(_ins->_nbDestinations);
   for( int j = 0; j < _ins->_nbDestinations; ++j )
   {
      _cons_arriving[j].resize(_ins->_nbVehicles);
   }

   for( int j = 0; j < _ins->_nbDestinations; ++j )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPsnprintf(var_cons_name, 255, "arriving_%d_%d", j, m);

         SCIPcreateConsBasicLinear(_scipCM,               // scip
                                   &_cons_arriving[j][m], // cons
                                   var_cons_name,         // name
                                   0,                     // nvar
                                   0,                     // vars
                                   0,                     // coeffs
                                   0,                     // lhs
                                   0);                    // rhs

         for( int i = 0; i < _ins->_nbDestinations; ++i ) // sum over all i
         {
            SCIPaddCoefLinear(_scipCM, _cons_arriving[j][m], _var_X[i][j][m], 1);
         }
         SCIPaddCoefLinear(_scipCM, _cons_arriving[j][m], _var_Y[j][m], -1);

         SCIPaddCons(_scipCM, _cons_arriving[j][m]);
      }
   }

   //#####################################################################################################################
   // restriction 11.8 from Book

   // sum(m, Y_im) = 1 for all i
   // is equal to:
   // 1 <= sum(m, Y_im) <= 1 for all i

   _cons_oneTourPerDest.resize(_ins->_nbDestinations);

   for( int i = 1; i < _ins->_nbDestinations; ++i ) // without 0!!!!
   {
      SCIPsnprintf(var_cons_name, 255, "oneTourPerDest_%d", i);

      SCIPcreateConsBasicLinear(_scipCM,                  // scip
                                &_cons_oneTourPerDest[i], // cons
                                var_cons_name,            // name
                                0,                        // nvar
                                0,                        // vars
                                0,                        // coeffs
                                1,                        // lhs
                                1);                       // rhs

      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPaddCoefLinear(_scipCM, _cons_oneTourPerDest[i], _var_Y[i][m], 1);
      }
      SCIPaddCons(_scipCM, _cons_oneTourPerDest[i]);
   }

   //#####################################################################################################################
   // restriction 11.9 from Book

   // Z_i - Z_j + I * sum(m, X_ijm) <= I - 1 for all i,j | (i > 0 & j > 0 &(i!=j))
   // is equal to:
   // -inf <= Z_i - Z_j + sum(m, I * X_ijm) <= I - 1 for all i,j | (i > 0 & j > 0 &(i!=j))

   _cons_shortCycles.resize(_ins->_nbDestinations);
   for( int i = 1; i < _ins->_nbDestinations; i++ )
   {
      _cons_shortCycles[i].resize(_ins->_nbDestinations);
   }

   for( int i = 1; i < _ins->_nbDestinations; i++ )
   {
      for( int j = 1; j < _ins->_nbDestinations; j++ )
      {
         if( i != j )
         {

            SCIPsnprintf(var_cons_name, 255, "ShortCycle_%d_%d_", i, j);

            SCIPcreateConsBasicLinear(_scipCM,                    // scip
                                      &_cons_shortCycles[i][j],   // cons
                                      var_cons_name,              // name
                                      0,                          // nvar
                                      0,                          // vars
                                      0,                          // coeffs
                                      -SCIPinfinity(_scipCM),     // lhs
                                      _ins->_nbDestinations - 1); // rhs

            SCIPaddCoefLinear(_scipCM, _cons_shortCycles[i][j], _var_Z[i], 1);
            SCIPaddCoefLinear(_scipCM, _cons_shortCycles[i][j], _var_Z[j], -1);

            for( int m = 0; m < _ins->_nbVehicles; m++ ) // sum over all m
            {
               SCIPaddCoefLinear(_scipCM, _cons_shortCycles[i][j], _var_X[i][j][m], _ins->_nbDestinations);
            }

            SCIPaddCons(_scipCM, _cons_shortCycles[i][j]);
         }
      }
   }

   //#####################################################################################################################
   // restriction 11.10 from Book

   // X_iim = 0 for all i, m
   // is equal to:
   // 0 <= X_iim <= 0 for all i, m

   _cons_noSelfTour.resize(_ins->_nbDestinations);
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      _cons_noSelfTour[i].resize(_ins->_nbVehicles);
   }

   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPsnprintf(var_cons_name, 255, "noSelfTours_%d_%d", i, m);

         SCIPcreateConsBasicLinear(_scipCM,                 // scip
                                   &_cons_noSelfTour[i][m], // cons
                                   var_cons_name,           // name
                                   0,                       // nvar
                                   0,                       // vars
                                   0,                       // coeffs
                                   0,                       // lhs
                                   0);                      // rhs

         SCIPaddCoefLinear(_scipCM, _cons_noSelfTour[i][m], _var_X[i][i][m], 1);

         SCIPaddCons(_scipCM, _cons_noSelfTour[i][m]);
      }
   }

   //#####################################################################################################################
   // Generate LP file
   //#####################################################################################################################

   // Generate a file to show the LP-Program, that is build. "FALSE" = we get our specific choosen names.
   SCIPwriteOrigProblem(_scipCM, "compact_model_vrp.lp", "lp", FALSE);
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

   // release all NoSelfTour-constraints
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPreleaseCons(_scipCM, &_cons_noSelfTour[i][m]);
      }
   }

   // release all shortCycles-constraints
   for( int i = 1; i < _ins->_nbDestinations; i++ )
   {
      for( int j = 1; j < _ins->_nbDestinations; j++ )
      {
         if( i != j )
         {
            SCIPreleaseCons(_scipCM, &_cons_shortCycles[i][j]);
         }
      }
   }

   // release all oneTourPerDest-constraints
   for( int i = 1; i < _ins->_nbDestinations; ++i ) // without 0!!!!
   {
      SCIPreleaseCons(_scipCM, &_cons_oneTourPerDest[i]);
   }

   // release all Arriving-constraints
   for( int j = 0; j < _ins->_nbDestinations; ++j )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPreleaseCons(_scipCM, &_cons_arriving[j][m]);
      }
   }

   // release all Leaving-constraints
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPreleaseCons(_scipCM, &_cons_leaving[i][m]);
      }
   }

   // release all capacity-constraints
   for( int m = 0; m < _ins->_nbVehicles; ++m )
   {
      SCIPreleaseCons(_scipCM, &_cons_capacity[m]);
   }

   //#####################################################################################################################
   // release all variables
   //#####################################################################################################################
   // Releasing variables is done in the same way as releasing constraints. Use the same for-loops as for generating the
   // variables and ensure you get everyone.

   // release all X_ijm - variables
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int j = 0; j < _ins->_nbDestinations; ++j )
      {
         for( int m = 0; m < _ins->_nbVehicles; ++m )
         {
            SCIPreleaseVar(_scipCM, &_var_X[i][j][m]);
         }
      }
   }

   // release all Y_im - variables
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      for( int m = 0; m < _ins->_nbVehicles; ++m )
      {
         SCIPreleaseVar(_scipCM, &_var_Y[i][m]);
      }
   }

   // release all Z_i - variables
   for( int i = 0; i < _ins->_nbDestinations; ++i )
   {
      SCIPreleaseVar(_scipCM, &_var_Z[i]);
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
