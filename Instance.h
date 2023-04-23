#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

/**
 * @brief every external parameter-value and set for the BPP
 *
 * @param _nb Setsizes
 *
 * @param par parameters
 *
 * @note This class defines the parameters for a Bin Packing Problem (BPP). It induces two sets:
 * - set I: containing all item indices, starting with 0 and ending in _nbItems - 1,
 * - set J: containing all bin indices, starting with 0 and ending in _nbBins
 and two parameters: par_w and par_b. The read() function reads in a file with the parameter
 values, and the display() function displays the values.
 */
class Instance
{
public:
   int _nbItems; // Setsize of Items (i \in I)
   int _nbBins;  // Setsize of Bins (j \in J)

   // Parameters
   int            par_b; // b - capacity of a single bin
   vector<double> par_w; // w_i - weight of item i \in I

   void read(string nameFile); // function to read data from a file

   void display(); // function to display the data
};