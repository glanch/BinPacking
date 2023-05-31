#include "BranchConsHdlr.h"
#include "BranchRule.h"
#include "CompactModel.h"
#include "Master.h"
#include "Pricer.h"

/**
 * @brief main-function
 *
 * @return int
 *
 * @note This code creates an instance of a problem, reads in the data from a file, displays the data, creates a compact
 * model of the instance, solves the model, and displays the solution. The instance name is hardcoded as
 * "../data/Ins_01.bpp". The main function takes two parameters: argc (the number of arguments) and argv (an array of
 * strings containing the arguments). The function returns 0 upon completion.
 */
int main()
{
   // #####################################################################################################################
   //  Settings
   string InstanceName = "../data/bpa/u100_00.bpa";

   // #####################################################################################################################
   //  read and display the instance
   Instance* ins = new Instance();
   ins->readBPA(InstanceName);
   ins->display();

#ifdef SOLVE_COMPACT
   // #####################################################################################################################
   //  create compact problem

   // Use the Constructor to create the instance "compMod" of the class CompactModel
   CompactModel* compMod = new CompactModel(ins);

   // call the function "solve" to solve the model and the function "display" to show the solution
   compMod->solve();
   compMod->displaySolution();

   // after we are finished, we free the memory of compact model
   delete compMod;
#endif

   // proceed with CG
   Master* pbMaster = new Master(ins);

   //==========================================
   // create and activate pricer_BPP_exact_mip

   MyPricer* pricer_BPP_exact_mip = new MyPricer(pbMaster,
                                                 "BPP_exact_mip",             // name of the pricer
                                                 "Simple Bin Packing Pricer", // short description of the pricer
                                                 0,                           // priority
                                                 TRUE);                       // delay

   SCIPincludeObjPricer(pbMaster->_scipRMP, //
                        pricer_BPP_exact_mip,
                        true);

   // activate pricer_BPP_exact_mip
   SCIPactivatePricer(pbMaster->_scipRMP, SCIPfindPricer(pbMaster->_scipRMP, pricer_BPP_exact_mip->_name));

   // include constraint handler to manage the branching constraints
   BranchConsHdlr* RyanFosterConstraints = new BranchConsHdlr(pbMaster, pricer_BPP_exact_mip);

   SCIPincludeObjConshdlr(pbMaster->_scipRMP, RyanFosterConstraints, true);

   //=======================================================
   // include the branching rule
   BranchRule* RyanFosterBranching =
      new BranchRule(pbMaster,
                     "RyanFoster",
                     "Child1: two items are in one bin together, Child2: two items are in different bins ",
                     500000,
                     -1,
                     1);

   SCIPincludeObjBranchrule(pbMaster->_scipRMP, RyanFosterBranching, true);

   //==========================================
   // solve the master problem

   pbMaster->solve();
   pbMaster->displaySolution();

   // delete all dynamicly created objects
   delete pbMaster;
   // we do not need to delete the pricer, cause of delete-object-flag == true at SCIPincludeObjPricer()
   delete ins;
}