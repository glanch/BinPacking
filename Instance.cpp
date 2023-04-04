#include "Instance.h"

/**
 * @brief read data from a .vrp-file
 *
 * @param nameFile path to vrp.file
 *
 * @note This function reads data from a .vrp-file and stores it in the instance. It takes in a string nameFile which is
 * the path to the vrp.file. The function uses an ifstream to open the file and checks if it is valid. If not, an error
 * message is printed. The function then reads each line of the file and stores it in a string line. A character
 * par_name is set to 0 and ss (an istringstream) reads the first character of the line. A switch statement is used
 * to determine which parameter to store the data in based on the first character of each line. The
 * _nbDestinations-parameter is used to resize the par_c matrix and par_w-vector to fit the number of destinations. The
 * data is then stored in vectors par_w, par_c, and par_b depending on what type of data it is. Every line is loaded
 * into ss and with >> we can get the next character from ss and delete it from ss. Finally, the file stream is closed.
 */
void Instance::read(string nameFile)
{
   ifstream infile(nameFile);
   if( !infile ) //! warning, if file-path is wrong, pay attention, the code will not stop!
      cout << "Instance::read : file not found"
           << "\n";

   string line; // a string, which stores a single line

   while( getline(infile, line) ) // do this for every line in the file, if there is any
   {
      istringstream ss(line); // ss is a string stream of the line, which makes it possible to get different
                              // strings separated by spaces
                              
      char par_name = {0};    // the first character of the line, which indicates the parameter, initialized with 0

      ss >> par_name; // get the first char of the line

      switch( par_name ) // Instead of using multiple if-statements, we use a switch statement: if(par_name == '')
      {
      case 'I': // read number of destinations
      {
         ss >> _nbDestinations;

         // as par_w and par_c depend on the number of destinations, we are resizing them
         par_w.resize(_nbDestinations, 0);

         par_c.resize(_nbDestinations);
         for( int i = 0; i < _nbDestinations; ++i )
         {
            par_c[i].resize(_nbDestinations, 0);
         }
         break; // jump to the end of the switch-statement
      }

      case 'M': // read number of tours
      {
         ss >> _nbVehicles;

         par_b.resize(_nbVehicles, 0);
         break;
      }

      case 'b': // read b : capacity-parameter
      {
         int m, val = 0;
         ss >> m >> val;
         par_b[m] = val;
         break;
      }

      case 'c': // read c : costs to drive from destination i to destination j
      {
         int i, j, val = 0;
         ss >> i >> j >> val;
         par_c[i][j] = val;
         break;
      }

      case 'w': // read w : capacity use for destination i
      {
         int i, val = 0;
         ss >> i >> val;
         par_w[i] = val;
         break;
      }

         // if no of the key-chars is at the beginning, ignore the whole line and do nothing
      }
   }
   infile.close();
}

/**
 * @brief function to display the instance-data
 *
 * @note This function is used to display the instance-data of a Vehicle Routing Problem. It prints out the number of
 destinations (I) and tours (M), as well as the costs c_ij to drive from i to j, capacity b_m for every tour m, and
 quantity w_i for every destination i.
 */
void Instance::display()
{
   cout << "Instance: \n";

   cout << "Number of Destinations I: " << _nbDestinations << "\n";
   cout << "Number of Tours M: " << _nbVehicles << "\n";

   cout << "Costs c_ij to drive from i to j: \n";
   for( int i = 0; i < _nbDestinations; ++i )
   {
      for( int j = 0; j < _nbDestinations; ++j )
      {
         cout << i << " " << j << ": " << par_c[i][j] << "\n";
      }
   }

   cout << "capacity b_m:  \n";
   for( int m = 0; m < _nbVehicles; ++m )
   {
      cout << m << ": " << par_b[m] << "\n";
   }
   cout << "quantity w_i for every destination i: \n";
   for( int i = 0; i < _nbDestinations; ++i )
   {
      cout << i << ": " << par_w[i] << "\n";
   }
}
