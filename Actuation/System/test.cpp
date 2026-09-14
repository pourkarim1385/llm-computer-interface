#include "SysFunctionsLinux.hpp"

using namespace std;

int main(){
    SysfunctionsLinux& instance = SysfunctionsLinux::getInstance();

    cout << instance.launchProgram("code") << endl;
}