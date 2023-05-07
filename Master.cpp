#include "Master.h"
#include <scip/scip_cons.h>

/**
 * @brief Construct a new Master:: Master object
 *
 * @param ins pointer to a instance object
 *
 * @note This code defines the constructor for the Master class, which is part of a larger program for solving a BPP
 * using column generation.
 * The constructor takes an instance object (ins) as input and assigns it to a private pointer variable (_ins) in the
 * Master class.
 * It then creates a SCIP (Solving Constraint Integer Programs) environment and loads all the default plugins.
 * After that, it creates an empty problem using the SCIPcreateProb() function.
 * Next, it sets all optional SCIP parameters by calling the setSCIPParameters() function.
 * Following this, the function creates the name dummy variable (var_cons_name), resizes a vector (_var_lambda) for use
 * later, and creates one set of linear constraints: _cons_onePatternPerItem
 * The _cons_onePatternPerItem constraints ensure that each item is packed is packed exactly once.
 * Finally, the function writes the original LP program to a file using the SCIPwriteOrigProblem() function
 */
Master::Master(Instance* ins)
{
   _ins = ins; // initialize the private pointer variable to the instance object

   // create a SCIP environment and load all defaults
   SCIPcreate(&_scipRMP);
   SCIPincludeDefaultPlugins(_scipRMP);

   // create an empty problem
   SCIPcreateProb(_scipRMP, "master-problem BPP", 0, 0, 0, 0, 0, 0, 0);

   // set all optional SCIPParameters
   setSCIPParameters();

   // create Helping-dummy for the name of variables and constraints
   char var_cons_name[255];

   // add variables:
   // we have currently no variables, cause we are at the beginning of our columnGeneration-Process.
   _var_lambda.resize(0);

   // #####################################################################################
   // add restrictions with SCIPcreateConsLinear(), it needs to be modifiable:

   // ###################################################################################
   // onePatternPerItem
   // sum(p, lambda_p * a_i^p) = 1 for all i
   // is equal to:
   // 1 <= sum(p, lambda_p * a_i^p) <= 1 for all i
   // a_i^p is the coefficient in the pattern p in row i, i.e. for item i, which we produce by solving our pricing
   // subproblem

   _cons_onePatternPerItem.resize(_ins->_nbItems);

   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPsnprintf(var_cons_name, 255, "onePatternPerItem_%d", i);

      SCIPcreateConsLinear(_scipRMP,                    // scip
                           &_cons_onePatternPerItem[i], // cons
                           var_cons_name,               // name
                           0,                           // nvar
                           0,                           // vars
                           0,                           // coeffs
                           1,                           // lhs
                           1,                           // rhs
                           TRUE,                        // initial
                           FALSE,                       // separate
                           TRUE,                        // enforce
                           TRUE,                        // check
                           TRUE,                        // propagate
                           FALSE,                       // local
                           TRUE,                        // modifiable
                           FALSE,                       // dynamic
                           FALSE,                       // removable
                           FALSE);                      // stick at nodes

      // our term is empty, cause we have no Lambdas
      SCIPaddCons(_scipRMP, _cons_onePatternPerItem[i]);
   }

   // generate a file to show the LP-Program that is build. "FALSE" = we get our specific choosen names.
   SCIPwriteOrigProblem(_scipRMP, "original_RMP_bpp.lp", "lp", FALSE);
}

/**
 * @brief Destroy the Master:: Master object
 *
 * @note This code is a destructor for a class called "Master". It is responsible for releasing all constraints, variables and the
 * SCIP (Solving Constraint Integer Programming) environment associated with the class.
 * Specifically, the code uses a for loop to iterate over all items in the instance (referred to by the variable
 * "_ins") and their corresponding constraints. For each item, the corresponding onePatternPerItem constraint is freed by calling SCIPreleaseCons.
 * For each generated pattern, the corresponding lambda variables (stored in the vector "_var_lambda"),
 * SCIPreleaseVar is called to release it's memory from the SCIP environment.
 * Finally, SCIPfree is called on the SCIP environment itself, which frees any memory associated with the environment.
 */
Master::~Master()
{
   // release constraints
   for(int i = 0; i < _ins->_nbItems; i++) {
      SCIPreleaseCons(_scipRMP, &_cons_onePatternPerItem[i]);
   }

   // release variables
   for( int p = 0; p < _var_lambda.size(); p++)
   {
      SCIPreleaseVar(_scipRMP, &_var_lambda[p]);
   }

   SCIPfree(&_scipRMP);
}

// solve the problem
void Master::solve()
{
   cout << "___________________________________________________________________________________________\n";
   cout << "start Solving ColumnGeneration: \n";
   SCIPsolve(_scipRMP);
}

/**
 * @brief Display the solution of the master problem
 *
 * @note The code defines a member function called "displaySolution" of a class called "Master". This function utilizes
 * the SCIP optimization solver to print the best solution found by the solver. The SCIPprintBestSol function takes
 * three arguments: a pointer to a SCIP instance (in this case, _scipRMP), a pointer to a file stream for output (in
 * this case, NULL), and a Boolean value indicating whether to display the solution in verbose mode (in this case,
 * FALSE). The function does not return any value.
 */
void Master::displaySolution() { SCIPprintBestSol(_scipRMP, NULL, FALSE); }

/**
 * @brief Set the SCIP parameters
 *
 * @note This code defines a method named setSCIPParameters in the class Master. The method sets various parameters for
 * the SCIP optimization solver. These parameters include limits for time and gap, verbosity level, and display options.
 * Additionally, a file for the vbc-tool is specified so that the branch and bound tree can be visualized.
 * Some parameters are modified specifically for the pricing process. For column generation, the maxrestarts parameter
 * is set to 0 to avoid a known bug. For constraints containing priced variables, the rootredcost parameter is disabled,
 * while constraints that may not be respected during the pricing process are not added.
 * Finally, constraints are not separated to avoid adding ones that may be violated during the pricing process.
 */
void Master::setSCIPParameters()
{
   // for more information: https://www.scipopt.org/doc/html/PARAMETERS.php
   SCIPsetRealParam(_scipRMP, "limits/time", 1e+20);    // default 1e+20 s
   SCIPsetRealParam(_scipRMP, "limits/gap", 0);         // default 0
   SCIPsetIntParam(_scipRMP, "display/verblevel", 4);   // default 4
   SCIPsetBoolParam(_scipRMP, "display/lpinfo", FALSE); // default FALSE

   // write a file for vbc-tool, so that later the branch&bound tree can be visualized
   SCIPsetStringParam(_scipRMP, "visual/vbcfilename", "tree.vbc");

   // modify some parameters so that the pricing can work properly

   // http://scip.zib.de : "known bug : If one uses column generation and restarts, a solution that contains
   // variables that are only present in the transformed problem
   //(i.e., variables that were generated by a pricer) is not pulled back into the original space correctly,
   // since the priced variables have no original counterpart.Therefore, one should disable restarts by setting the
   // parameter "presolving/maxrestarts" to 0, if one uses a column generation approach."

   SCIPsetIntParam(_scipRMP, "presolving/maxrestarts", 0);

   // http://scip.zib.de : "If your pricer cannot cope with variable bounds other than 0 and infinity, you have to
   // mark all constraints containing priced variables as modifiable, and you may have to disable reduced cost
   // strengthening by setting propagating / rootredcost / freq to - 1."
   SCIPsetIntParam(_scipRMP, "propagating/rootredcost/freq", -1);

   // no separation to avoid that constraints are added which we cannot respect during the pricing process
   SCIPsetSeparating(_scipRMP, SCIP_PARAMSETTING_OFF, TRUE);
}
