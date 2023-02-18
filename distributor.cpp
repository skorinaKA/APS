#include "distributor.h"

Distributor::Distributor(Statistic& stat) :
	size_(stat.getExecAmount())
{
    for (int i = 0; i < size_; i++)
    {
        Executor exec(i, stat.getAlpha(), stat.getBeta());
        execVector_.push_back(exec);
    }
}

Executor& Distributor::getExecuter(size_t num)
{
    if (num < size_)
    {
        return execVector_[num];
    }
}
