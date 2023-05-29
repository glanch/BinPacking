#pragma once
/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

using namespace std;
using namespace scip;

/**
 * @brief Stores one Pattern
 *
 * @note six public members: reducedCosts, PatternCosts, PatternIncidence, includedItems, sub_m and display(). It also
 * has two methods: containsBOTH() and containsONE(). The containsBOTH(item1, item2) method takes a pair of integers as an argument
 * and returns true if the stable set contains both item1 and item2, false otherwise. The containsONE(item1, item2)
 * method takes a pair of integers as an argument and returns true if the stable set contains item1 but not
 * item2 or vice versa.
 */
class Pattern
{
public:
   SCIP_Real         reducedCosts;
   SCIP_Real         PatternCosts;
   vector<SCIP_Bool> PatternIncidence;          // binary vector of the packing pattern
   vector<int>       includedItems; // integer vector of the Pattern
   int               sub_m;
   int               LambdaPatternIndex;

   void display() {}

   // return true if the Pattern contains both item1 AND item2, false otherwise
   bool containsBOTH(int item1, int item2)
   {
      return PatternIncidence[item1] && PatternIncidence[item2];
   }

   // return true if the Pattern contains item1 but not item1 or vice versa
   bool containsONE(int item1, int item2)
   {
      return (PatternIncidence[item1] && !PatternIncidence[item2]) ||
             (!PatternIncidence[item1] && PatternIncidence[item2]);
   }
};