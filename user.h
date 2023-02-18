#ifndef USER_H
#define USER_H

#include "request.h"

#include <random>

class User
{
public:
    User(int number, double speed);

    int getUserNumber();
    int getGeneratedTasks();
    double getRequestTime();
    requestPointer generation();

private:
    int userNumber_;
    int generatedTasks_ = 0;
    double sourceSpeed_ = 0.0;
    double newRequestTime_ = 0.0;
};

#endif
