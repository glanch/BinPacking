#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

/**
 * @brief every external parameter-value and set for the VRP
 *
 * @param _nb Setsizes
 *
 * @param par parameters
 *
 * @note This class defines the parameters for a Vehicle Routing Problem (VRP). It has three sets: _nbDestinations,
 _nbVehicles, and three parameters: par_c, par_w, and par_b. The read() function reads in a file with the parameter
 values, and the display() function displays the values.
 */
class Instance
{
public:
   int _nbDestinations; // Setsize of Destinations (i,j \in I)
   int _nbVehicles;     // Setsize of Vehicles (m \in M)

   // Parameters
   vector<vector<double>> par_c; // d_ij - distance between i and j
   vector<double>         par_w; // w_i - demand at destination i
   vector<double>         par_b; // b_m - capacity of vehicle m

   void read(string nameFile); // function to read data from a file

   void display(); // function to display the data
};