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
        ,
        _name(p_name), _desc(p_desc), _pbMaster(pbMaster), _scipRMP(pbMaster->_scipRMP), _ins(pbMaster->_ins)
{
   // Initialize dual variables object
   // it ensures correct initialization of all it's memmbers
   DualValues = new DualVariables(_ins);

   // Initialize
   Subproblem_mip = new SubProblemMIP(_ins);
}

/**
 * @brief Destroy the My Pricer:: My Pricer object
 *
 */
MyPricer::~MyPricer()
{
   // destroy dualvalues
   delete DualValues;

   // destroy subproblem
   delete Subproblem_mip;
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
 * Next, the code retrieves the transformed constraints. It does this by looping through each item
 * and using the SCIPgetTransformedCons function to get the transformed constraint corresponding to the
 * onePatternPerItem constraint. Finally, the code returns SCIP_OKAY to indicate that the
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
 * @return SCIP_RESULT
 */
SCIP_RESULT MyPricer::pricing(const bool isFarkas)
{
   // ############################################################################################################
   //  define the dual variables
   //  we need one value for every constraint, so use the same loops as in the master-problem
   //  is isFarkas == true, then use the farkas-multipliers, is isFarkas == false, use the dual-variables
   //  After getting a value, check, if the value is possible, for <= or >= - constraints, we know the sign, so we can
   //  force them. We do this to avoid numerical-issues
   // ############################################################################################################

   // define all dual-variables for the onePatternPerItem-constraints
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      DualValues->onePatternPerItem_pi[i] =
         isFarkas ? SCIPgetDualfarkasLinear(_scipRMP, _pbMaster->_cons_onePatternPerItem[i])
                  : SCIPgetDualsolLinear(_scipRMP,
                                         _pbMaster->_cons_onePatternPerItem[i]); // choose the appropiate method call
                                                                                 // for retrieving dual variables
   }

   Subproblem_mip->updateObjFunc(DualValues,
                                 isFarkas); // update objective function according to new dual variable values
   Pattern* solution = Subproblem_mip->solve(); // and now, solve subproblem

   // if reduced costs are (sufficiently) negative, add the solution
   if( SCIPisNegative(_scipRMP, solution->reducedCosts + 0.001) )
   {
      addNewVar(solution);
   } else { // the solution is not needed any longer, delete it
      delete solution;
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

   cout << endl;
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

   cout << endl;

   return SCIP_OKAY;
}

/**
 * @brief add a new variable (a new possible pattern for a bin) to the master problem.
 *
 * @param solution a pointer to a solution of the subproblem
 *
 * @note  a function addNewVar in the MyPricer class that adds a new variable to the SCIP optimization model. The
 * variable represents a packing pattern of a bin and is added to the master problem. The function creates a new
 * variable and assigns a unique name to it. It then sets the lower and upper bounds of the variable and its objective
 * value in the model. The function adds the newly created variable as a priced variable to the SCIP optimization model.
 * The function then adds coefficients to the one (non-dummy) constraint in the model: onePatternPerItem. It
 * iterates through each item and adds the corresponding coefficient in the onePatternPerItem constraint. The newly
 * created variable is also added to a list of lambdas in the _pbMaster object. Finally, the function writes the updated
 * optimization model to a file for inspection.
 */
void MyPricer::addNewVar(Pattern* solution)
{
   // create the new variable
   SCIP_VAR* newVar;

   char var_name[255];

   int lambdaIndex =
      _pbMaster->_var_lambda.size(); // actual number of lambda variables corresponds to the next free zero-based index

   ( void )SCIPsnprintf(var_name, 255, "lambda_%d", lambdaIndex); // create name

   SCIPcreateVar(_scipRMP,                 // scip-env
                 &newVar,                  // connect with the new variable
                 var_name,                 // set name
                 0.0,                      // lower bound
                 1,   // upper bound
                 solution->PatternCosts, // objective
                 SCIP_VARTYPE_BINARY,  // discrete since we are using discretization
                 false,
                 false,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);

   // add the new variable and resume the simplex-algorithm with the reducedCosts
   SCIPaddPricedVar(_scipRMP, newVar, -solution->reducedCosts);

   // ############################################################################################################
   //  add coefficients to the constraints

   // onePatternPerItem constraint
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIP_Real coeff = solution->PatternIncidence[i];
      SCIPaddCoefLinear(_scipRMP, _pbMaster->_cons_onePatternPerItem[i], newVar, coeff);
   }
   
   // current variable index
   int trans_counter = _pbMaster->_var_lambda.size();

   // add variable to book keeping
   _pbMaster->_var_lambda.push_back(newVar);

   // set pattern lambda index after "adding" lambda variable
   solution->LambdaPatternIndex = trans_counter;

   // add corresponding pattern to book keeping
   _pbMaster->_Patterns.push_back(solution);

   char model_name[255];
   ( void )SCIPsnprintf(model_name, 255, "TransMasterProblems/TransMaster_%d.lp", trans_counter);
   SCIPwriteTransProblem(_scipRMP, model_name, "lp", FALSE);
};

/** @brief
 * @param solution The solution that should be printed
 * @note print the results of a calculation to the console. It takes the solution of SubProblem::solve() and prints it
 * to the console, along with some additional text. The function prints the reduced
 * and newly generated packing pattern costs. It then iterates through the PatternIncidence boolean vector and prints the
 * indices where the value is true, thus displaying whether an item is part of the new pattern.
 */
void MyPricer::display_one_variable(Pattern* solution)
{
   cout << "Variable / new pattern with reduced costs: " << solution->reducedCosts
        << " and PatternCosts: " << solution->PatternCosts << endl
        << "with items: ";
   for( int i = 0; i < solution->PatternIncidence.size(); ++i )
   {
      if( solution->PatternIncidence[i] == true )
         cout << i << " ";
   }

   cout << endl;
}