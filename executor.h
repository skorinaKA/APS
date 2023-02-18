#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "Request.h"
#include "Statistic.h"
#include "step.h"

#include <random>
#include <memory>
#include <iostream>

class Executor
{
public:
    Executor(int execNumber, double a, double b);

    void getRequest(const requestPointer newRequest, step& step);

    bool isAvaliable(Statistic& statistic, step& step);
    bool isAvaliable(double lastRequestTime, Statistic& statistic, step& step);
    void fillStatistic(Statistic& statistic, step& step);
    int getExecutorNumber() const;
    double getTaskStartTime() const;

private:
    double a_;
    double b_;
    double taskStartTime_ = 0;
    double taskEndTime_ = 0;
    const int execNumber_;
    requestPointer currentRequest_;
};

#endif
