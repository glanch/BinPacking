#include "BranchConsHdlr.h"

/**
 * @brief Create a Branch Ctr object
 *
 * @param scip
 * @param cons
 * @param items
 * @param type
 * @param node
 *
 * @note create a branching constraint in the SCIP optimization solver. It begins by getting the address of the
 * constraint handler, then creates and initializes a structure SCIP_CONSDATA to store data related to the branching
 * constraint. Finally, it creates the branching constraint and defines its behavior via functions/methods scip_prop,
 * scip_active and scip_deactive. The function returns a SCIP_RETCODE indicating success or failure.
 *
 * @return SCIP_RETCODE
 */
SCIP_RETCODE
createBranchCtr(SCIP* scip, SCIP_CONS** cons, pair<int, int> items, CONSTYPE type, SCIP_NODE* node)
{
   // 1. get the address of the constraint handler
   SCIP_CONSHDLR* conshdlr = NULL;
   conshdlr                = SCIPfindConshdlr(scip, "BranchConsHdlr");
   if( conshdlr == NULL )
   {
      SCIPerrorMessage("I haven't found handler ConsBranchHdlr\n");
      return SCIP_PLUGINNOTFOUND;
   }

   // 2. we create and initialize a structure SCIP_CONSDATA to store the data related to the branching constraint
   SCIP_CONSDATA* consdata;
   createConsdata(scip, &consdata, items, type, node);

   // 3. we create the branching constraint (locally), its behavior will be defined via the functions/methods scip_prop,
   // scip_active and scip_deactive
   SCIPcreateCons(scip,
                  cons,
                  "ctrBranching",
                  conshdlr,
                  consdata,
                  FALSE,
                  FALSE,
                  FALSE,
                  FALSE,
                  TRUE,
                  TRUE,
                  FALSE,
                  FALSE,
                  FALSE,
                  TRUE);

   return SCIP_OKAY;
}

/**
 * @brief Create a Consdata object
 *
 * @param scip
 * @param consdata
 * @param items
 * @param type
 * @param node
 *
 * @note  allocates memory and stores data associated with a branching constraint. It takes in parameters such as
 * items, type, and node, and stores them in the consdata structure. It also initializes the number of propagated
 * variables, number of propagations, and whether or not the constraint has been propagated to 0 or false respectively.
 * Finally, it returns SCIP_OKAY to indicate that the operation was successful.
 *
 * @return SCIP_RETCODE
 */
SCIP_RETCODE
createConsdata(SCIP* scip, SCIP_CONSDATA** consdata, pair<int, int> items, CONSTYPE type, SCIP_NODE* node)
{
   SCIP_CALL(SCIPallocBlockMemory(scip, consdata));

   // info about the branching parameters of that constraint
   (*consdata)->item1 = items.first;
   (*consdata)->item2 = items.second;
   (*consdata)->_type = type;
   (*consdata)->_node = node;

   // info about the constraint itself
   (*consdata)->_nPropagatedVars = 0;
   (*consdata)->_nPropagations   = 0;
   (*consdata)->_propagated      = FALSE;

   return SCIP_OKAY;
}

/**
 * @brief propagate the constraint (it is here where the constraint becomes "effective")
 *
 * @param scip
 * @param conshdlr
 * @param conss
 * @param nconss
 * @param nusefulconss
 * @param nmarkedconss
 * @param proptiming
 * @param result
 *
 * @note This code is a function that is part of the SCIP library. It is used to propagate branching constraints to the
 * master problem variables. The function takes in a SCIP pointer, a constraint handler pointer, an array of
 * constraints, the number of constraints, the number of useful constraints, the number of marked constraints, a
 * propagation timing parameter and a result pointer. It then iterates through all the constraints and obtains their
 * associated data. If the constraint has not been propagated yet, it increases its number of propagations and checks if
 * there are new master problem variables to which it needs to be applied. Depending on the type of constraint (separate
 * or together), it fixes those variables to 0 if they contain both items or one item respectively.
 * Finally, it sets the result pointer accordingly and returns SCIP_OKAY.
 * @return SCIP_RETCODE
 */
SCIP_RETCODE BranchConsHdlr::scip_prop(SCIP*           scip,
                                       SCIP_CONSHDLR*  conshdlr,
                                       SCIP_CONS**     conss,
                                       int             nconss,
                                       int             nusefulconss,
                                       int             nmarkedconss,
                                       SCIP_PROPTIMING proptiming,
                                       SCIP_RESULT*    result)
{
   *result = SCIP_DIDNOTFIND;

   //----------------------------------------------------
   // 1. obtain the consdata associated at the last constraint
   SCIP_CONSDATA* consdata;

   for( int c = 0; c < nconss; ++c ) // go through all constraints
   {
      consdata = SCIPconsGetData(conss[c]); // get the Data

      if( !consdata->_propagated ) // if the constraint has not been propagated
      {
         consdata->_nPropagations++; // increase the number of propagations

         // If by now the master problem contains variables to which the constraint has not been applied, we now
         // apply it to those variable. This can be the case if, after the last time the constraint has been propagated
         // to those master variables existent at that time, new master variables have been added by the pricer.

         if( consdata->_nPropagatedVars < _pbMaster->_Patterns.size() )
         { // We do have new master problem variables to which the constraint has to be propagated!
            pair<int, int> items = pair<int, int>(consdata->item1, consdata->item2);
            bool           ok    = true; // If fixing to 0 leads to a contradiction, we cut off this node.

            switch( consdata->_type )
            {
            case SEPARATE:
            {
               // disable all master variables containing both items
               ok = fixToZeroIfTogether(scip, items, consdata->_nPropagatedVars);
            }
            case TOGETHER:
            {
               // disable all master variables containing exactly one of the items
               ok = fixToZeroIfNotTogether(scip, items, consdata->_nPropagatedVars);
            }
            }

            // We remember the number of variables to which we have already propagated the
            // branching constraint.
            consdata->_nPropagatedVars = _pbMaster->_Patterns.size();

            if( ok )
            {
               *result               = SCIP_REDUCEDDOM;
               consdata->_propagated = TRUE;
            }
            else
               *result = SCIP_CUTOFF;
         }
      }
   }
   return SCIP_OKAY;
}

/**
 * @brief activate the constraint
 *
 * @param scip
 * @param conshdlr
 * @param cons
 *
 * @note This code is part of a BranchConsHdlr class and is used to activate a branching constraint. It first retrieves
 * the data associated with the constraint, then prints out information about the constraint. It then adds the branching
 * constraint to a list and to every subproblem in a Subproblem_mip list. Finally, it activates the branching constraint
 * for every node in the subtree of the current node.
 *
 * @return SCIP_RETCODE
 */
SCIP_RETCODE BranchConsHdlr::scip_active(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS* cons)
{
   SCIP_CONSDATA* consdata;
   consdata = SCIPconsGetData(cons);

   cout << "active ctr : (" << consdata->item1 << "," << consdata->item2 << ") : " << ( int )(consdata->_type)
        << "  node " << SCIPnodeGetNumber(consdata->_node) << endl;

   // Add the branching constraint to the list
   _pbMaster->_cons_branching.push_back(consdata);

   // Add the branching constraint to subproblem
   _pbPricer->Subproblem_mip->addBranching(consdata);

   // Each time a node of the subtree of the current node is processed, we activate
   // the branching constraint of that node.

   SCIPrepropagateNode(scip, consdata->_node);

   return SCIP_OKAY;
}
/**
 * @brief The branching constraint has to be deactivated as we leave this node and all its subtrees.
 *
 * @param scip
 * @param conshdlr
 * @param cons
 *
 * @note deactivate a branching constraint. It first drops the last element from a vector, which works like a stack.
 * Then it deletes the branching constraint for each subproblem in the Subproblem_mip vector. Finally, it returns
 * SCIP_OKAY to indicate that the operation was successful.
 *
 * @return SCIP_RETCODE
 */
SCIP_RETCODE BranchConsHdlr::scip_deactive(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS* cons)
{
   // We drop the last one. Works like a stack, but to be able loop through, we use a vector.
   _pbMaster->_cons_branching.pop_back();

   // delete the branching constraint of subproblem
   _pbPricer->Subproblem_mip->deleteLastBranching();
   return SCIP_OKAY;
}

// transforms constraint data into data belonging to the transformed problem
SCIP_RETCODE
BranchConsHdlr::scip_trans(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS* sourcecons, SCIP_CONS** targetcons)
{
   SCIPcreateCons(scip,
                  targetcons,
                  SCIPconsGetName(sourcecons),
                  conshdlr,
                  0,
                  SCIPconsIsInitial(sourcecons),
                  SCIPconsIsSeparated(sourcecons),
                  SCIPconsIsEnforced(sourcecons),
                  SCIPconsIsChecked(sourcecons),
                  SCIPconsIsPropagated(sourcecons),
                  SCIPconsIsLocal(sourcecons),
                  SCIPconsIsModifiable(sourcecons),
                  SCIPconsIsDynamic(sourcecons),
                  SCIPconsIsRemovable(sourcecons),
                  SCIPconsIsStickingAtNode(sourcecons));

   return SCIP_OKAY;
}

// this method has to be called to free the memory of "consdata" (if there is any)
SCIP_RETCODE BranchConsHdlr::scip_delete(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS* cons, SCIP_CONSDATA** consdata)
{
   SCIPfreeBlockMemory(scip, consdata);
   return SCIP_OKAY;
}

/**
 * @brief fix all variables to 0, if they contain both items
 *
 * @param scip
 * @param items
 *
 * @note fix the master variables corresponding to Patterns containing item1 and item2 together. It
 * iterates through the _var_lambda array and checks if the stable set associated with each variable contains both
 * items. If it does, it fixes the variable to 0. Finally, it returns a boolean value indicating whether or not
 * fixing the variables to 0 made the problem infeasible.
 *
 * @return !infeasible
 */
bool BranchConsHdlr::fixToZeroIfTogether(SCIP* scip, pair<int, int> items, int alreadyPropagated)
{

   SCIP_Bool fixed;
   SCIP_Bool infeasible = 0;

   for( int p = alreadyPropagated; p < _pbMaster->_Patterns.size(); p++ )
   {
      // if the pattern p contains both item1 and item2, we fix the corresponding lambda to 0
      if( _pbMaster->_Patterns[p]->containsBOTH(items.first, items.second) )
      {
         SCIPfixVar(scip,
                    _pbMaster->_var_lambda[_pbMaster->_Patterns[p]->LambdaPatternIndex],
                    0,
                    &infeasible,
                    &fixed); // fix to 0
      }
   }

   return (!infeasible); // If fixing to 0 makes the problem infeasible, we can cut off the node
}

/**
 * @brief fix all variables to 0, if they contain both items
 *
 * @param scip
 * @param items
 *
 * @note fix the master variables corresponding to Patterns containing item1 and item2 together. It
 * iterates through the _var_lambda array and checks if the stable set associated with each variable contains exactly
 * one of the items. If it does, it fixes the variable to 0. Finally, it returns a boolean value indicating
 * whether or not fixing the variables to 0 made the problem infeasible.
 *
 * @return !infeasible
 */
bool BranchConsHdlr::fixToZeroIfNotTogether(SCIP* scip, pair<int, int> items, int alreadyPropagated)
{

   SCIP_Bool fixed;
   SCIP_Bool infeasible = 0;

   for( int p = alreadyPropagated; p < _pbMaster->_Patterns.size(); p++ )
   {
      // if the Pattern p contains item1 and item2, we fix the corresponding lambda to 0
      if( _pbMaster->_Patterns[p]->containsONE(items.first, items.second) )
      {
         int m     = _pbMaster->_Patterns[p]->sub_m;
         int lambda_index = _pbMaster->_Patterns[p]->LambdaPatternIndex;
         SCIPfixVar(scip, _pbMaster->_var_lambda[lambda_index], 0, &infeasible, &fixed); // fix to 0
      }
   }

   return (!infeasible); // If fixing to 0 makes the problem infeasible, we can cut off the node
}
