#pragma once

#include "objscip/objscip.h"
#include "scip/cons_linear.h"
#include "scip/scip.h"

#include <iostream>

#include "Master.h"
#include "Pricer.h"
#include "SCIP_ConsData.h"

using namespace std;
using namespace scip;

//============================================================
// class used to manage the branching constraints related
// to the Ryan & Forster branching rule

class BranchConsHdlr : public ObjConshdlr
{
public:
   Master*   _pbMaster; // pointer to the master problem
   MyPricer* _pbPricer; // pointer to the Pricer

   // constructor
   BranchConsHdlr(Master* pbMaster, MyPricer* pbPricer)
         : ObjConshdlr(pbMaster->_scipRMP,
                       "BranchConsHdlr",
                       "stores the local branching decisions",
                       0,
                       0,
                       9999999,
                       -1,
                       1,
                       1,
                       0,
                       FALSE,
                       FALSE,
                       TRUE,
                       SCIP_PROPTIMING_BEFORELP,
                       SCIP_PRESOLTIMING_FAST | SCIP_PRESOLTIMING_EXHAUSTIVE)
   {
      _pbMaster = pbMaster;
      _pbPricer = pbPricer;
   }

   // destructor
   virtual ~BranchConsHdlr() {}

   virtual SCIP_RETCODE scip_trans(SCIP*          scip,       //**< SCIP data structure *
                                   SCIP_CONSHDLR* conshdlr,   //**< the constraint handler itself *
                                   SCIP_CONS*     sourcecons, //**< source constraint to transform *
                                   SCIP_CONS**    targetcons  //**< pointer to store created target constraint *
                                   ) override;

   // propagate the constraint (it is here where the constraint becomes "effective")
   virtual SCIP_RETCODE scip_prop(SCIP*           scip,
                                  SCIP_CONSHDLR*  conshdlr,
                                  SCIP_CONS**     conss,
                                  int             nconss,
                                  int             nusefulconss,
                                  int             nmarkedconss,
                                  SCIP_PROPTIMING proptiming,
                                  SCIP_RESULT*    result) override;

   // the constraint needs to be propagated (enforced)
   virtual SCIP_RETCODE scip_active(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS* cons) override;

   // the constraint no longer needs to be propagated (enforced)
   virtual SCIP_RETCODE scip_deactive(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS* cons) override;

   // optional : this method has to be called to free the memory of "consdata" (if there is any)
   virtual SCIP_RETCODE
   scip_delete(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS* cons, SCIP_CONSDATA** consdata) override;

   //================================================================================================
   // The next six function have to be redefined as they are purely virtual in the parent class,
   // but they are not being used here

   virtual SCIP_RETCODE scip_check(SCIP*          scip,
                                   SCIP_CONSHDLR* conshdlr,
                                   SCIP_CONS**    conss,
                                   int            nconss,
                                   SCIP_SOL*      sol,
                                   SCIP_Bool      checkintegrality,
                                   SCIP_Bool      checklprows,
                                   SCIP_Bool      printreason,
                                   SCIP_Bool      completely,
                                   SCIP_RESULT*   result) override
   {
      return SCIP_OKAY;
   }

   virtual SCIP_RETCODE scip_enfolp(SCIP*          scip,
                                    SCIP_CONSHDLR* conshdlr,
                                    SCIP_CONS**    conss,
                                    int            nconss,
                                    int            nusefulconss,
                                    SCIP_Bool      solinfeasible,
                                    SCIP_RESULT*   result) override
   {
      return SCIP_OKAY;
   }

   virtual SCIP_RETCODE scip_enfops(SCIP*          scip,
                                    SCIP_CONSHDLR* conshdlr,
                                    SCIP_CONS**    conss,
                                    int            nconss,
                                    int            nusefulconss,
                                    SCIP_Bool      solinfeasible,
                                    SCIP_Bool      objinfeasible,
                                    SCIP_RESULT*   result) override
   {
      return SCIP_OKAY;
   }

   virtual SCIP_RETCODE scip_lock(SCIP*          scip,
                                  SCIP_CONSHDLR* conshdlr,
                                  SCIP_CONS*     cons,
                                  SCIP_LOCKTYPE  locktype,
                                  int            nlockspos,
                                  int            nlocksneg) override
   {
      return SCIP_OKAY;
   }

   virtual SCIP_RETCODE scip_sepalp(SCIP*          scip,
                                    SCIP_CONSHDLR* conshdlr,
                                    SCIP_CONS**    conss,
                                    int            nconss,
                                    int            nusefulconss,
                                    SCIP_RESULT*   result) override
   {
      return SCIP_OKAY;
   }

   virtual SCIP_RETCODE scip_sepasol(SCIP*          scip,
                                     SCIP_CONSHDLR* conshdlr,
                                     SCIP_CONS**    conss,
                                     int            nconss,
                                     int            nusefulconss,
                                     SCIP_SOL*      sol,
                                     SCIP_RESULT*   result) override
   {
      return SCIP_OKAY;
   }

   //==============================================================================================
   // Fix to 0 the master variables corresponding to patterns containing item1 and item2
   // together. We consider all the variables, starting at alreadyPropagated.

   bool fixToZeroIfTogether(SCIP* scip, pair<int, int> items, int alreadyPropagated);

   // Fix to 0 the master variables corresponding to patterns containing either item1 or item2,
   // but not both. We consider all the variables, starting at alreadyPropagated.

   bool fixToZeroIfNotTogether(SCIP* scip, pair<int, int> items, int alreadyPropagated);
};

// We create a branching constraint and the associated constraint data. The pointer
// to that constraint is passed as a parameter.
SCIP_RETCODE
createBranchCtr(SCIP* scip, SCIP_CONS** cons, pair<int, int> items, CONSTYPE type, SCIP_NODE* node);

// Create the data structure with the consdata and initialize it the data passed as parameter
SCIP_RETCODE
createConsdata(SCIP* scip, SCIP_CONSDATA** consdata, pair<int, int> items, CONSTYPE type, SCIP_NODE* node);