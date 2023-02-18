#include "user.h"

User::User(int number, double speed) :
    userNumber_(number),
    sourceSpeed_(speed)
{ }

int User::getUserNumber()
{
    return userNumber_;
}

int User::getGeneratedTasks()
{
    return generatedTasks_;
}

double User::getRequestTime()
{
    return newRequestTime_;
}

requestPointer User::generation()
{
    generatedTasks_++;
    newRequestTime_ += (-1 / sourceSpeed_ * log(static_cast<double>(rand()) / static_cast<double>(RAND_MAX)));
    return Request::getNewRequest(newRequestTime_, userNumber_, generatedTasks_);
}
