#include "../networking/exceptions.hpp"
#include <iostream>

using namespace exceptions;
using namespace std;

int main() {
    cout << "Throwing and catching NetworkSetupError with error code 42.\n";
    
    bool error_catched = false;
    string error;

    try {
        throw NetworkSetupError(42);
    } catch (const NetworkSetupError& e) {
        error_catched = true;    
        error = e.what();
    } catch (const exception& e) {}
    
    string error_answer = "Can't setup network because of error with code: 42";
    if (error_catched) {
        if (error == error_answer) {
            cout << "Error catched!";
        } else {
            cout << "Expected: " << error_answer << "\nGot: " << error;
        }
    } else {
        cout << "Error not catched!";
    }

    cout << '\n';
}