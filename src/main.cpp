#include "su.hpp"
#include <iostream>
//base theta  =  θ

int main(int argc, char **argv){
    if (argc > 2){
      std::cerr << argv[1] << "file.su \n";
	    return EXIT_FAILURE;
    }

   if (argc == 2){
     Su::runFile(argv[1]);
   }else{
	   Su::runPrompt();
   }
return EXIT_SUCCESS;
}
