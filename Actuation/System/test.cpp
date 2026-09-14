#include "SysFunctionsLinux.hpp"

using namespace std;

int main(){
    SysfunctionsLinux& instance = SysfunctionsLinux::getInstance();

    cout << instance.closeApp("firefox", false) << endl;
}