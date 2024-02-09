#include <iostream>
#include "Proxy.h"
#include <fstream>

int main() {

    //std::ofstream file;
    //file.open("logs.txt");
    //Proxy serwer1("0.0.0.0", 8081);
    //Proxy serwer = serwer1;
    //Proxy serwer("0.0.0.0", 8080);
    //serwer.startListen();
    Proxy proxy("0.0.0.0", 8081);
    proxy.startListen();
    //std::cout << "Hello" << std::endl;
    return 0;
}
