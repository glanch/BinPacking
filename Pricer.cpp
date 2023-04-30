#include "Pricer.h"

using namespace std;
using namespace scip;

/**
 * @brief Construct a new My Pricer:: My Pricer object
 *
 * @param pbMaster pointer to the Master object
 * @param p_name name of the pricer
 * @param p_desc description of the pricer
 * @param p_priority priority of the pricer
 * @param p_delay whether the LP is re-optimized each time a variable is added
 *
 * @note This is a constructor function for a class called "MyPricer". It takes in several parameters including a
 * pointer to a Master object (pbMaster), the name of the pricer (p_name), a description of the pricer (p_desc), a
 * priority
 * level (p_priority), and a boolean value indicating whether the LP is re-optimized each time a variable is added
 * (p_delay).
 * The function initializes the inherited ObjPricer object by calling its constructor. It then assigns the values of the
 * input parameters to the corresponding private member variables of the MyPricer object: _name, _desc, _pbMaster,
 * _scipRMP, and _ins.
 * The _scipRMP and _ins member variables are shortcuts to the scip-env of the master-problem and the instance,
 * respectively, both of which are obtained from the pbMaster object.
 * @return nothing
 */
MyPricer::MyPricer(Master* pbMaster, const char* p_name, const char* p_desc, int p_priority, SCIP_Bool p_delay)
      : ObjPricer(pbMaster->_scipRMP,
                  p_name,
                  p_desc,
                  p_priority,
                  p_delay) // TRUE : LP is re-optimized each time a variable is added
{
   _name     = p_name;              // store the name of the pricer
   _desc     = p_desc;              // store the description of the pricer
   _pbMaster = pbMaster;            // store the pointer to the master-problem
   _scipRMP  = _pbMaster->_scipRMP; // construct a shortcut for the pointer to the scip-env of the master-problem
   _ins      = _pbMaster->_ins;     // construct a shortcut for the pointer to the instance,
}

/**
 * @brief Destroy the My Pricer:: My Pricer object
 *
 */
MyPricer::~MyPricer()
{
   // nothing to do here
}

/**
 * @brief get the pointers to the variables and constraints of the transformed problem
 *
 * @param scip
 * @param pricer
 * @return SCIP_RETCODE
 *
 * @note This code is part of a SCIP plugin and it initializes the plugin's pricer. In particular, it obtains pointers
 * to the variables and constraints of the transformed problem.
 * First, the code tries to retrieve the transformed variables. However, since there are no variables in the transformed
 * problem yet, this part of the code is commented out.
 * Next, the code retrieves the transformed constraints. It does this by looping through each destination (except the
 * first one) and using the SCIPgetTransformedCons function to get the transformed constraint corresponding to the
 * onePlanPerDest constraint. Similarly, the code loops through each vehicle and retrieves the transformed constraint
 * corresponding to the onePlanPerVehicle constraint. Finally, the code returns SCIP_OKAY to indicate that the
 * initialization was successful. Overall, this code is responsible for retrieving the transformed constraints so that
 * the plugin can modify them as needed using the addNewVar() function.
 */
SCIP_RETCODE MyPricer::scip_init(SCIP* scip, SCIP_PRICER* pricer)
{
   // get transformed variables
   // we do not have any variables at this point in Time

   // ###########################################################################################################
   // get transformed constraints:
   // to get all transformed constraints, use the same loops, as in generation in the master-problem
   // ###########################################################################################################

   // get transformed onePatternPerItem-Constraints
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPgetTransformedCons(_scipRMP, _pbMaster->_cons_onePatternPerItem[i], &(_pbMaster->_cons_onePatternPerItem[i]));
   }

   return SCIP_OKAY;
}

/**
 * @brief perform pricing for dual and farkas combined with Flag isFarkas
 *
 * @param isFarkas perform farkas whether the master problem is LP-infeasible
 *
 * @note a function named "pricing" inside the class "MyPricer". The function takes a boolean value called "isFarkas" as
 * its input parameter. The function initializes and defines two vectors called "destination_pi" and "convex_pi" with
 * the length of "_ins->_nbDestinations" and "_ins->_nbVehicles", respectively. Then it applies a loop to define the
 * dual variables for each constraint. The loop uses SCIPgetDualfarkasLinear function if "isFarkas" is true, and
 * SCIPgetDualsolLinear if "isFarkas" is false. Next, it applies another loop to calculate the reduced costs and the
 * objective function coefficient for each newly generated variable, and add new variables to the model if their reduced
 * cost is negative. Lastly, it returns SCIP_SUCCESS.
 * @return SCIP_RESULT
 */
SCIP_RESULT MyPricer::pricing(const bool isFarkas)
{
   // Initialize and dimension the vectors, standard-value 0
   vector<SCIP_Real> pattern_pi(_ins->_nbItems,
                                0); // (i) Dual-/ of Farkas-values of the onePatternPerItem-constraint for every i

   //############################################################################################################
   // define the dual variables
   // we need one value for every constraint, so use the same loops as in the master-problem
   // is isFarkas == true, then use the farkas-multipliers, is isFarkas == FALSE, use the dual-variables
   // After getting a value, check, if the value is possible, for <= or >= - constraints, we know the sign, so we can
   // force them. We do this to avoid numerical-issues
   //############################################################################################################

   // define all dual-variables for the onePatternPerItem-constraints
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      pattern_pi[i] = isFarkas ? SCIPgetDualfarkasLinear(_scipRMP, _pbMaster->_cons_onePatternPerItem[i])
                               : SCIPgetDualsolLinear(_scipRMP, _pbMaster->_cons_onePatternPerItem[i]);
   }

   SCIP_Real         reducedCosts = 0; // reduced costs of the optimal solution for the subproblem
   SCIP_Real         patternCosts = 1; // objective Function coefficient of the potential new variable
   vector<SCIP_Bool> newPattern(_ins->_nbItems,
                                false); // (i) = 1 if item i is part of the optimal solution for the subproblem

   reducedCosts = generate_solve_Subproblem_MIP(pattern_pi, newPattern, isFarkas, patternCosts);

   if( SCIPisNegative(_scipRMP, reducedCosts) )
   {
      addNewVar(newPattern, patternCosts, reducedCosts);
      display_one_variable(newPattern, patternCosts, reducedCosts);
   }
   // since we are using an exact method to calculate the optimal reduced costs, we can always be
   // sure, that we found one, or that no one exists-> our result is always "SCIP_success"

   return SCIP_SUCCESS;
}

/**
 * @brief perform dual-pricing
 *
 * @param scip  an instance of the SCIP solver
 * @param pricer an instance of the SCIP pricer
 * @param lowerbound  a pointer to store the resulting lower bound
 * @param stopearly a pointer to signal whether to stop the pricing process early
 * @param result a pointer to store the SCIP result
 * @return SCIP_RETCODE
 *
 * @note calls the "pricing" method of the "MyPricer" class, passing in a boolean value of false, and stores the
 * resulting SCIP result in the "result" pointer.
 */
SCIP_RETCODE MyPricer::scip_redcost(SCIP*        scip,
                                    SCIP_PRICER* pricer,
                                    SCIP_Real*   lowerbound,
                                    SCIP_Bool*   stopearly,
                                    SCIP_RESULT* result)
{
   cout << "Dual-Pricing: ";
   // start dual-pricing with isFarkas-Flag = false
   *result = pricing(false);

   return SCIP_OKAY;
}

/**
 * @brief perform farkas-pricing
 *
 * @param scip  an instance of the SCIP solver
 * @param pricer an instance of the SCIP pricer
 * @param result a pointer to store the SCIP result
 * @return SCIP_RETCODE
 *
 * @note calls the "pricing" method of the "MyPricer" class, passing in a boolean value of true, and stores the
 * resulting SCIP result in the "result" pointer.
 */
SCIP_RETCODE MyPricer::scip_farkas(SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result)
{

   cout << "Farkas-Pricing: ";
   // start dual-pricing with isFarkas-Flag = false
   *result = pricing(true);

   return SCIP_OKAY;
}

/**
 * @brief add a new variable (a new possible Tour for the vehicle sub_m) to the master problem.
 *
 * @param newTour a vector of booleans representing the calculated tour
 * @param planCosts the costs for the calculated tour
 * @param sub_m the index of the vehicle
 *
 * @note  a function addNewVar in the MyPricer class that adds a new variable to the SCIP optimization model. The
 * variable represents a possible tour for a vehicle and is added to the master problem. The function creates a new
 * variable and assigns a unique name to it. It then sets the lower and upper bounds of the variable and its objective
 * value in the model. The function adds the newly created variable as a priced variable to the SCIP optimization model.
 * The function then adds coefficients to two constraints in the model: onePlanPerDest and onePlanPerVehicle. It
 * iterates through each destination and adds the corresponding coefficient in the onePlanPerDest constraint. It then
 * adds a coefficient of 1 in the onePlanPerVehicle constraint. The newly created variable is also added to a list of
 * lambdas in the _pbMaster object. Finally, the function writes the updated optimization model to a file for
 * inspection.
 */
void MyPricer::addNewVar(vector<SCIP_Bool>& newPattern, double& patternCosts, double& reducedCosts)
{
   // create the new variable
   SCIP_VAR* newVar;

   char var_name[255];

   int lambdaIndex = _pbMaster->_var_lambda.size(); // actual number of lambdas for each vehicle

   ( void )SCIPsnprintf(var_name, 255, "lambda_%d", lambdaIndex); // create name

   SCIPcreateVar(_scipRMP,                // scip-env
                 &newVar,                 // connect with the new variable
                 var_name,                // set name
                 0.0,                     // lower bound
                 SCIPinfinity(_scipRMP),  // upper bound
                 patternCosts,            // objective
                 SCIP_VARTYPE_CONTINUOUS, // continuous if only column-generation
                 false,
                 false,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);

   // add the new variable and resume the simplex-algorithm with the reducedCosts
   SCIPaddPricedVar(_scipRMP, newVar, -reducedCosts);

   //############################################################################################################
   // add coefficients to the constraints

   // onePatternPerItem constraint
   for( int i = 0; i < _ins->_nbItems; ++i ) // without 0
   {
      SCIP_Real coeff = newPattern[i];
      SCIPaddCoefLinear(_scipRMP, _pbMaster->_cons_onePatternPerItem[i], newVar, coeff);
   }

   _pbMaster->_var_lambda.push_back(newVar);

   char model_name[255];

   // TODO: find out why this is named trans_counter
   int trans_counter = _pbMaster->_var_lambda.size();

   ( void )SCIPsnprintf(model_name, 255, "TransMasterProblems/TransMaster_%d.lp", trans_counter);
   SCIPwriteTransProblem(_scipRMP, model_name, "lp", FALSE);
};

/**
 * @brief generate the SCIP subproblem with the method of the compact Model
 *
 * @param destination_pi dual-variables for the onePlanPerDest-constraints
 * @param convex_pi_m dual-variables for the onePlanPerVehicle-constraints
 * @param newPlan a vector of booleans representing the calculated tour
 * @param sub_m the index of the vehicle
 * @param isFarkas a boolean flag indicating if the variables are dual or farkas
 * @param planCosts the costs for the calculated tour
 * @return the reduced costs, or 0 if infeasible
 *
 * @note generates and solves a subproblem using SCIP for the Vehicle Routing Problem (VRP). The subproblem is built
 * from scratch and solved with a standard solver, using dual/farkas variables and a flag to build a different objective
 * function.
 * The inputs to the function are a vector of dual/farkas variables, a value for convex_pi_m, a reference to a vector of
 * booleans for the new plan, an integer for the subproblem, and a boolean flag indicating if the variables are dual or
 * farkas.
 * The outputs of the function are the optimal reduce costs, the corresponding plan (via the newPlan argument), and the
 * costs of this new plan (via the planCosts argument).
 * The function creates a new SCIP instance, includes default plugins, sets optional parameters, creates and adds
 * variables, creates and adds constraints, solves the subproblem, and releases constraints and variables after the
 * solving process.
 */
SCIP_Real MyPricer::generate_solve_Subproblem_MIP(vector<SCIP_Real>& pattern_pi,
                                                  vector<SCIP_Bool>& newPattern,
                                                  const bool&        isFarkas,
                                                  SCIP_Real&         patternCosts)
{
   SCIP_Real val = 0;

   // first generate the Subproblem with the method of the compact Model
   SCIP* _scipSP;

   SCIPcreate(&_scipSP);
   SCIPincludeDefaultPlugins(_scipSP);
   SCIPcreateProbBasic(_scipSP, "Subproblem BPP");

   // set all optional SCIPParameters
   SCIPsetIntParam(_scipSP, "display/verblevel", 0);
   SCIPsetBoolParam(_scipSP, "display/lpinfo", FALSE);

   // create Helping-dummy for the name of variables and constraints
   char var_cons_name[255];

   //############################################################################################################
   // create and add all Variables
   //############################################################################################################

   // add the binary variable X_i for all items i

   // set dimension for variable vector, with empty pointers
   _var_X.resize(_ins->_nbItems);

   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      // only if isFarkas == false, consider the costs of the Plan, which are equal to 1
      SCIP_Real obj_coeff = !isFarkas ? 1 : 0;

      SCIPsnprintf(var_cons_name, 255, "X_%d", i); // set name for debugging

      SCIPcreateVarBasic(_scipSP,
                         &_var_X[i],           // returns the address of the newly created variable
                         var_cons_name,        // name
                         0,                    // lower bound
                         1,                    // upper bound
                         obj_coeff,            // objective function coefficient
                         SCIP_VARTYPE_BINARY); // variable type

      SCIPaddVar(_scipSP, _var_X[i]); // add newVar to scip-env
   }

   // add dummy-variable to consider the constant term of 1 in the objective Function
   SCIPsnprintf(var_cons_name, 255, "cost_const");

   SCIPcreateVarBasic(_scipSP,
                      &_var_cost_const,         // returns the address of the newly created variable
                      var_cons_name,            // name
                      1,                        // lower bound
                      1,                        // upper bound
                      1,                        // objective function coefficient
                      SCIP_VARTYPE_CONTINUOUS); // variable type

   SCIPaddVar(_scipSP, _var_cost_const); // add newVar to scip-env

   // #########################################################################################
   // Add restrictions
   //##########################################################################################
   // restriction (13) in lecture handout
   // sum(i, w_i * X_i) <= b
   // is equal to -infty <= sum(i, w_i * X_i) <= b

   _con_capacity = nullptr;

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
      SCIPaddCoefLinear(_scipSP,        // scip-env
                        _con_capacity,  // constraint
                        _var_X[i],      // variable
                        pattern_pi[i]); // coefficient
   }
   SCIPaddCons(_scipSP, _con_capacity); // add constraint to the scip-env

   // dummy constraint
   // cost_const == 1
   // is equal to 1 <= Cost_const <= 1

   SCIPcreateConsBasicLinear(_scipSP,          // scip
                             &_con_cost_const,   // cons
                             "con_cost_const", // name
                             0,                // nvar
                             0,                // vars
                             0,                // coeffs
                             1,                // lhs
                             1);               // rhs

   // #################################################################################
   // solve the subproblem
   // #################################################################################
   SCIPwriteOrigProblem(_scipSP, "subProblem.lp", "lp", FALSE);

   SCIPsolve(_scipSP);

   SCIP_SOL* sol = SCIPgetBestSol(_scipSP);
   val           = SCIPgetSolOrigObj(_scipSP, sol);
   patternCosts  = 1; // pattern costs are equal to one because each pattern costs exactly one bin

   // store the plan
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      newPattern[i] = SCIPgetSolVal(_scipSP, sol, _var_X[i]) > 0.5;
   }

   //#####################################################################################################################
   // release constraints
   //#####################################################################################################################
   // Every constraint that we have generated and stored needs to be released. Thus, use the same for-loops as for
   // generating the constraints to ensure that you release everyone.

   SCIPreleaseCons(_scipSP, &_con_capacity);
   SCIPreleaseCons(_scipSP, &_con_cost_const);

   //#####################################################################################################################
   // release all variables
   //#####################################################################################################################
   // Releasing variables is done in the same way as releasing constraints. Use the same for-loops as for generating the
   // variables and ensure you get everyone.

   // release all X_i - variables
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPreleaseVar(_scipSP, &_var_X[i]);
   }

   // release dummy variable
   SCIPreleaseVar(_scipSP, &_var_cost_const);

   //#####################################################################################################################
   // release SCIP object
   //#####################################################################################################################
   // At the end release the SCIP object itself
   SCIPfree(&_scipSP);

   return val;
}

/** @brief
 * @param newTour
 * @param planCosts
 * @param reducedCosts
 * @param sub_m
 * @note print the results of a calculation to the console. It takes the input variables and prints them to the console,
 * along with some additional text. The function starts by printing the vehicle number and the reduced and plan costs
 * for that vehicle. It then iterates through the newTour boolean vector and prints the indices where the value is true.
 */
void MyPricer::display_one_variable(vector<SCIP_Bool>& newPattern, double& patternCosts, double& reducedCosts)
{
   cout << "Variable with reduced costs: " << reducedCosts << " and PlanCosts: " << patternCosts << endl
        << "with destinations: ";
   for( int i = 0; i < newPattern.size(); ++i )
   {
      if( newPattern[i] == true )
         cout << i << " ";
   }

   cout << endl;
}