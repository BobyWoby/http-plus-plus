#include  "../include/html.h"
#include <iostream>


std::string html::from_file(std::string filepath){
    std::ifstream file(filepath);
    if(!file){
        std::cout << "couldn't  open file!";
    }
    
    std::string buf, out;
    while(getline(file, buf)){
        out += buf;
    };
    return out;
}
