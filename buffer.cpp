#include "Buffer.h"

Buffer::Buffer(Statistic& stat) :
    bufferSize_(stat.getBufferSize()),
    freeSpace_(stat.getBufferSize())
{
    requestVector_ = std::vector<requestPointer>(stat.getBufferSize());
    requestVector_.reserve(stat.getBufferSize());
}

bool Buffer::addRequest(const requestPointer newRequest, step& step)
{
    if (freeSpace_ != 0)
    {
        int iterNum = 0;
        int i = tail_;
        while (iterNum != bufferSize_)
        {
            if (requestVector_[i] == nullptr)
            {
                tail_ = i;
                break;
            }
            if (i == bufferSize_ - 1)
            {
                i = 0;
            }
            else
            {
                i++;
            }
            iterNum++;
        }
        requestVector_[tail_] = newRequest;
        step.time = newRequest->getStartTime() + 0.00001;
        step.bufferStateVector[tail_] = newRequest;
        step.actionString = "Request " + std::to_string(newRequest->getUserNumber() + 1) + "." + std::to_string(newRequest->getRequestNumber()) + " generated and added to buffer.";
        step.stepNumber++;
        if (tail_ == bufferSize_ - 1)
        {
            tail_ = 0;
        }
        else
        {
            tail_++;
        }
        minusFreeSpace();
        return true;
    }
    else
    {
        double time = (double)INT_MAX;
        int oldNum = INT_MAX;
        for (int i = 0; i != bufferSize_; i++)
        {
            if (requestVector_[i]->getStartTime() < time)
            {
                time = requestVector_[i]->getStartTime();
                oldNum = i;
            }
        }
        if (head_ == oldNum)
        {
            head_ = (head_ == bufferSize_ - 1) ? 0 : head_ + 1;
        }
        requestVector_[oldNum] = newRequest;
        step.time = newRequest->getStartTime();
        step.bufferStateVector[oldNum] = newRequest;
        step.actionString = "Request" + std::to_string(newRequest->getUserNumber() + 1) + "." + std::to_string(newRequest->getRequestNumber()) + " generated and added to buffer. The oldest order was knocked out";
        tail_ = oldNum;
        return false;
    }
    
    return false;
}

void Buffer::minusFreeSpace()
{
    freeSpace_--;
}

bool Buffer::isEmpty()
{
    return freeSpace_ == bufferSize_;
}

bool Buffer::isFull()
{
    return freeSpace_ == 0;
}

int Buffer::getBufferSize() const
{
    return bufferSize_;
}

int Buffer::getFreeSpace() const
{
    return freeSpace_;
}

requestPointer Buffer::getRequest(step& step)
{
    requestPointer req = nullptr;
    if (!isEmpty())
    {
        req = requestVector_[head_];
        step.bufferStateVector[head_] = nullptr;
        requestVector_[head_] = nullptr;
        if (head_ == bufferSize_ - 1)
        {
            head_ = 0;
        }
        else
        {
            head_++;
        }
        freeSpace_++;
    }

    return req;
}
