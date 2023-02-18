#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include "executor.h"
#include "statistic.h"

#include <random>

class Distributor
{
public:
    Distributor(Statistic& stat);

    Executor& getExecuter(size_t num);

private:
    int size_;
    std::vector<Executor> execVector_;
};

#endif
