#include "Instance.h"

/**
 * @brief read data from a .bpp-file
 *
 * @param nameFile path to bpp.file
 *
 * @note This function reads data from a .bpp-file and stores it in the instance. It takes in a string nameFile which is
 * the path to the bpp.file. The function uses an ifstream to open the file and checks if it is valid. If not, an error
 * message is printed. The function then reads each line of the file and stores it in a string line. A character
 * par_name is set to 0 and ss (an istringstream) reads the first character of the line. A switch statement is used
 * to determine which parameter to store the data in based on the first character of each line. The
 * _nbItems-parameter is used to resize the par_w vector to fit the number of items. The _nbBins-parameter is used to
 * specify the upper bound of available bins. In our case it is equivalent to _nbItems. The data is then stored in
 * vector par_w (weights of items) and scalar par_b (capacity of a single bin) depending on what type of data it is.
 * Every line is loaded into ss and with >> we can get the next character from ss and delete it from ss. Finally, the
 * file stream is closed.
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

      char par_name = {0}; // the first character of the line, which indicates the parameter, initialized with 0

      ss >> par_name; // get the first char of the line

      switch( par_name ) // Instead of using multiple if-statements, we use a switch statement: if(par_name == '')
      {
      case 'I': // read number of items and therefore bins
      {
         int parameter;
         ss >> parameter;

         // set _nbItems and _nbBins
         _nbItems = parameter;
         _nbBins  = parameter;

         // as par_w depends on the number of items, we are resizing them
         par_w.resize(_nbItems, 0);

         break; // jump to the end of the switch-statement
      }

      case 'b': // read b : bin capacity parameter
      {
         int val = 0;
         ss >> val;
         par_b = val;
         break;
      }

      case 'w': // read w_i: weight for item i
      {
         int item, weight = 0;
         ss >> item >> weight;
         par_w[item] = weight;
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
 * @note This function is used to display the instance-data of a Bin Packing Problem. It prints out the number of
 items (I) and bins (J), as well as the weights w_i of item and the capacity b of a single bin.
 */
void Instance::display()
{
   cout << "Instance: "<< endl;

   cout << "Number of Items I: " << _nbItems << endl;
   cout << "Weights of item i: " << endl;
   for( int i = 0; i < _nbItems; ++i )
   {
      cout << i << ": " << par_w[i] << endl;
   }

   cout << "Capacity of a single bin: " << par_b << endl;
}

