#include "CompactModel.h"

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
   //#####################################################################################################################
   // Settings
   string InstanceName = "../data/Ins_01.bpp";

   //#####################################################################################################################
   // read and display the instance
   Instance* ins = new Instance();
   ins->read(InstanceName);
   ins->display();

   //#####################################################################################################################
   // create compact problem

   // Use the Constructor to create the instance "compMod" of the class CompactModel
   CompactModel* compMod = new CompactModel(ins);

   // call the function "solve" to solve the model and the function "display" to show the solution
   compMod->solve();
   compMod->displaySolution();

   // after we are finished, we free the memory
   delete compMod;
   delete ins;

   return 0;
}