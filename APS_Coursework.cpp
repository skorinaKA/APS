#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <conio.h>
#include <limits.h>
#include <cmath>
#include "easy_plot.hpp"

#include "server.h"
#include "buffer.h"
#include "distributor.h"

#pragma warning(disable : 4996)

static Statistic globalStat(0, 0, 0, 0, 0.0, 0.0, 0.0);
static int step_;
static int maxstep_;
static bool next;
static bool back;

int requestAmount_;
int userAmount_;
int bufferSize_;
int execAmount_;
double lambda_;
double alpha_;
double beta_;

ep::WindowSpec wave1;
ep::WindowSpec wave2;
ep::WindowSpec wave3;

void run(bool mode);

void table(std::vector<std::vector<std::string>>& data, int width, int hight);

void waveform(int argc, char* argv[]);

void fillDataStat(std::vector<std::vector<std::string>>& data);

void fillDataExecUsage(std::vector<std::vector<std::string>>& data);

void stepMode();

int main(int argc, char* argv[])
{
    std::cout << "Choose working mode: 0 - auto, 1 - step, 2 - wave 3 - test: ";
    int choose;
    std::cin >> choose;
    if (choose == 0)
    {
        run(false);
        std::vector<std::vector<std::string>> data(globalStat.getUserAmount() + 1, std::vector<std::string>(8));
        fillDataStat(data);
        table(data, 8, globalStat.getUserAmount() + 1);

        std::cout << "\n\n\n";

        std::vector<std::vector<std::string>> data1(globalStat.getExecAmount() + 1, std::vector<std::string>(2));
        fillDataExecUsage(data1);
        table(data1, 2, globalStat.getExecAmount() + 1);
    }
    else if (choose == 1)
    {
        run(false);
        step_ = 0;
        next = true;
        back = false;
        bool statusMode = true;
        bool statusStep = false;
        while (statusMode)
        {
            statusStep = false;
            system("cls");
            std::cout << "\n     " << char(17) << "    : step back\n     " << char(16) 
                      << "    : step forward\n  [space] : jump to X step\n    " 
                      << char(30) << " " << char(31) << "   : exit\n"
                      << "_______________________________";
            stepMode();
            while (!statusStep)
            {
                int button = getch();
                if (button == 224 || button == 0)
                {
                    int button = getch();
                    switch (button)
                    {
                    case 75:
                        if (back)
                        {
                            step_--;
                            statusStep = true;
                        }
                        break;
                    case 77:
                        if (next)
                        {
                            step_++;
                            statusStep = true;
                        }
                        break;
                    default:
                        statusMode = false;
                        statusStep = true;
                    }
                }
                else if (button == 32)
                {
                    std::size_t st = 0;
                    std::cout << "\n\n Jump to ";
                    std::cin >> st;
                    if (!std::cin)
                    {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
                        printf("\033[A\r\033[K\033[A\033[A");
                        continue;
                    }
                    step_ = (st > globalStat.getStepVector().size()) ? globalStat.getStepVector().size() - 1 : st - 1;
                    step_ = (step_ < 0) ? 0 : step_;
                    statusStep = true;
                }
            }
        }
    }
    else if (choose == 2)
    {
        run(false);
        step_ = 0;
        maxstep_ = step_;
        next = true;
        back = false;
        bool statusMode = true;
        bool statusStep = false;
        while (statusMode)
        {
            statusStep = false;
            system("cls");
            std::cout 
                <<"\n[space] : jump to X step\n    "
                << char(30) << " " << char(31) << "   : exit\n"
                << "steps: " << maxstep_ << "\n"
                << "_______________________________";
            waveform(argc, argv);
            while (!statusStep)
            {
                int button = getch();
                if (button == 224 || button == 0)
                {
                        statusMode = false;
                        statusStep = true;
                }
                else if (button == 32)
                {
                    std::size_t st = 0;
                    std::cout << "\n\n Jump to ";
                    std::cin >> st;
                    if (!std::cin)
                    {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
                        printf("\033[A\r\033[K\033[A\033[A");
                        continue;
                    }
                    maxstep_ = (st > globalStat.getStepVector().size()) ? globalStat.getStepVector().size() - 1 : st - 1;
                    maxstep_ = (maxstep_ < 0) ? 0 : maxstep_;
                    statusStep = true;
                }
            }
        }
    }
    else if (choose == 3)
    {
        int buffer = 10;
        int exec = 15;
        requestAmount_ = 1000;
        userAmount_ = 10;
        alpha_ = 0.5;
        beta_ = 0.6;
        lambda_ = 0.9;
        for (alpha_ = 0.0; alpha_ < 1; alpha_ += 0.1)
        {
            beta_ = alpha_ + 0.1;
            std::cout << "\nalpha = " << alpha_ << ", beta = " << beta_ << "\n__________________________";
            for (int i = 1; i <= buffer; i++)
            {
                for (int j = 1; j <= exec; j++)
                {
                    bufferSize_ = i;
                    execAmount_ = j;
                    run(true);
                    double rejProb = 0.0;
                    double usingProb = 0.0;
                    for (int k = 0; k < i; k++)
                    {
                        rejProb += globalStat.getRejectProbality(k);
                    }
                    rejProb /= i;
                    for (int k = 0; k < j; k++)
                    {
                        usingProb += globalStat.getExecUsage(k);
                    }
                    usingProb /= j;
                    if (abs(rejProb - 0.1) <= 0.03 && abs(usingProb - 0.9) < 0.03)
                    {
                        std::cout << "\nOptimal value: buffer size = " << i << ", executors count = " << j << "\nRej. prob. = " << rejProb << ", Using prob. = " << usingProb << "\nPrice = " << i * 10000 + j * (1000 / alpha_);
                    }
                }
            }
            std::cout << "\n__________________________\n";
        }
    }
    return 0;
}

void run(bool mode)
{
    int requestAmount;
    int userAmount;
    int bufferSize;
    int execAmount;
    double lambda;
    double alpha;
    double beta;
    if (!mode)
    {
        std::cout << "Enter start values:\n";
        std::cout << "  Request amount: ";
        std::cin >> requestAmount;
        std::cout << "  User amount: ";
        std::cin >> userAmount;
        std::cout << "  Buffer size: ";
        std::cin >> bufferSize;
        std::cout << "  Count of executers: ";
        std::cin >> execAmount;
        std::cout << "  Lambda: ";
        std::cin >> lambda;
        std::cout << "  Alpha: ";
        std::cin >> alpha;
        std::cout << "  Beta: ";
        std::cin >> beta;
    }
    else
    {
        requestAmount = requestAmount_;
        userAmount = userAmount_;
        bufferSize = bufferSize_;
        execAmount = execAmount_;
        lambda = lambda_;
        alpha = alpha_;
        beta = beta_;
    }

    Statistic statistic(requestAmount, userAmount, bufferSize, execAmount, lambda, alpha, beta);

    Server server(statistic);
    Buffer buffer(statistic);
    Distributor distributor(statistic);

    int requestNumber = 0;
    int deviceNumber = 0;
    bool isRejected = false;
    step step;
    step.userStateVector = std::vector<requestPointer>(statistic.getUserAmount(), nullptr);
    step.bufferStateVector = std::vector<requestPointer>(statistic.getBufferSize(), nullptr);
    step.execStateVector = std::vector<requestPointer>(statistic.getExecAmount(), nullptr);
    step.stepNumber = 0;

    while (statistic.getProcessed() + statistic.getRejected() != statistic.getRequestAmount())
    {
        if (requestNumber != statistic.getRequestAmount())
        {
            server.getRequest(requestNumber)->setSerialNumber(requestNumber + 1);
            requestPointer req = server.getRequest(requestNumber, step);
            statistic.addStep(step);
            statistic.addUserTime(step.time);
            statistic.addUserStep(step);

            for (int i = 0; i < step.userStateVector.size(); i++)
            {
                step.userStateVector[i] = nullptr;
            }
            isRejected = buffer.addRequest(req, step);
            if (!isRejected)
            {
                statistic.incrementRejected(server.getRequest(requestNumber)->getUserNumber());
            }
            statistic.addBufferTime(step.time);
            statistic.addBufferStep(step);
            statistic.addStep(step);

            for (int i = 0; i < statistic.getExecAmount(); i++)
            {
                if (distributor.getExecuter(i).isAvaliable(server.getRequest(requestNumber)->getStartTime(), statistic, step))
                {
                    distributor.getExecuter(i).getRequest(buffer.getRequest(step), step);
                    statistic.addExecTime(step.time);
                    statistic.addExecStep(step);
                    statistic.addStep(step);

                    break;
                }
            }

            requestNumber++;
        }
        else {
            for (int i = 0; i < statistic.getExecAmount() && !buffer.isEmpty(); i++)
            {
                if (distributor.getExecuter(i).isAvaliable(statistic, step))
                {
                    distributor.getExecuter(i).getRequest(buffer.getRequest(step), step);
                    statistic.addExecTime(step.time);
                    statistic.addExecStep(step);
                    statistic.addStep(step);
                    break;
                }
            }

            if (buffer.isEmpty())
            {
                distributor.getExecuter(deviceNumber).isAvaliable(statistic, step);
                deviceNumber = deviceNumber != statistic.getExecAmount() - 1 ? deviceNumber + 1 : 0;
            }
        }
    }
    double endTime = 0.0;
    for (int i = 0; i < statistic.getExecAmount(); i++)
    {
        if (endTime < distributor.getExecuter(i).getTaskStartTime())
        {
            endTime = distributor.getExecuter(i).getTaskStartTime();
        }
    }
    statistic.setTotalSimulationTime(endTime);
    globalStat = statistic;
}

void table(std::vector<std::vector<std::string>>& data, int width, int hight)
{
    // Variables for building a table
//-----------------------------------------
    char DOUBLE_UPPER_LEFT_CORNER(201);
    char DOUBLE_UPPER_RIGHT_CORNER(187);
    char DOUBLE_LOVER_LEFT_CORNER(200);
    char DOUBLE_LOVER_RIGHT_CORNER(188);

    char DOUBLE_HORIZONTAL_LINE(205);
    char DOUBLE_VERTICAL_LINE(186);

    char SINGLE_HORIZONTAL_LINE(196);

    char DOUBLE_TO_DOUBLE_LEFT(204);
    char DOUBLE_TO_DOUBLE_RIGHT(185);
    char DOUBLE_TO_DOUBLE_UP(203);
    char DOUBLE_TO_DOUBLE_DOWN(202);
    char DOUBLE_TO_DOUBLE_ALL(206);

    char DOUBLE_TO_SINGLE_LEFT(199);
    char DOUBLE_TO_SINGLE_RIGHT(182);
    char DOUBLE_TO_SINGLE_ALL(215);
    //-----------------------------------------

    int tableWidth = 0;
    std::vector<size_t> columnWidth(width);
        

    for (int i = 0; i < width; i++)
    {
        int max = 0;
        for (int j = 0; j < hight; j++)
        {
            if ((max - int(data[j][i].size())) <= 1)
            {
                max = data[j][i].size() + 2;
            }
        }
        columnWidth[i] = max;
    }

    for (int i = 0; i < width; i++)
    {
        if (i == 0)
        {
            std::cout << " " << DOUBLE_UPPER_LEFT_CORNER << std::setfill(DOUBLE_HORIZONTAL_LINE) << std::setw(columnWidth[i] + 3) << DOUBLE_TO_DOUBLE_UP;
        }
        else if (i == width - 1)
        {
            std::cout << std::setw(columnWidth[i] + 3) << DOUBLE_UPPER_RIGHT_CORNER << "\n";
        }
        else
        {
            std::cout << std::setw(columnWidth[i] + 3) << DOUBLE_TO_DOUBLE_UP;
        }

    }
    for (int i = 0; i < width; i++)
    {
        if (i == 0)
        {
            std::cout << std::setfill(' ') << " " << DOUBLE_VERTICAL_LINE;
        }
        int size = columnWidth[i] - data[0][i].size();
        int left = round(size / 2);
        int right = size - left;
        std::cout << std::setw(left+1) << " " << data[0][i] << std::setw(right + 1) << " " << DOUBLE_VERTICAL_LINE;
        if (i == width - 1)
        {
            std::cout << "\n";
        }
    }
    for (int i = 0; i < width; i++)
    {
        if (i == 0)
        {
            std::cout << " " << DOUBLE_TO_DOUBLE_LEFT << std::setfill(DOUBLE_HORIZONTAL_LINE) << std::setw(columnWidth[i] + 3) << DOUBLE_TO_DOUBLE_ALL;
        }
        else if (i == width - 1)
        {
            std::cout << std::setw(columnWidth[i] + 3) << DOUBLE_TO_DOUBLE_RIGHT << "\n";
        }
        else
        {
            std::cout << std::setw(columnWidth[i] + 3) << DOUBLE_TO_DOUBLE_ALL;
        }

    }

    for (int i = 0; i < hight - 1; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int size = columnWidth[j] - data[i + 1][j].size();
            int left = round(size / 2);
            int right = size - left;
            if (j == 0)
            {
                std::cout << " " << DOUBLE_VERTICAL_LINE << std::setfill(' ');
            }
            std::cout << std::setw(left + 1) << " " << data[i + 1][j] << std::setw(right + 1) << " " << DOUBLE_VERTICAL_LINE;
            if (j == width - 1)
            {
                std::cout << "\n";
            }
        }
        if (i == hight - 2)
        {
            for (int j = 0; j < width; j++)
            {
                if (j == 0)
                {
                    std::cout << " " << DOUBLE_LOVER_LEFT_CORNER << std::setfill(DOUBLE_HORIZONTAL_LINE) << std::setw(columnWidth[j] + 3) << DOUBLE_TO_DOUBLE_DOWN;
                }
                else if (j == width - 1)
                {
                    std::cout << std::setw(columnWidth[j] + 3) << DOUBLE_LOVER_RIGHT_CORNER << "\n";
                }
                else
                {
                    std::cout << std::setw(columnWidth[j] + 3) << DOUBLE_TO_DOUBLE_DOWN;
                }
            }
        }
        else
        {
            for (int j = 0; j < width; j++)
            {
                if (j == 0)
                {
                    std::cout << " " << DOUBLE_TO_SINGLE_LEFT << std::setfill(SINGLE_HORIZONTAL_LINE) << std::setw(columnWidth[j] + 3) << DOUBLE_TO_SINGLE_ALL;
                }
                else if (j == width - 1)
                {
                    std::cout << std::setw(columnWidth[j] + 3) << DOUBLE_TO_SINGLE_RIGHT << "\n";
                }
                else
                {
                    std::cout << std::setw(columnWidth[j] + 3) << DOUBLE_TO_SINGLE_ALL;
                }
            }
        }
    }
}

void fillDataStat(std::vector<std::vector<std::string>>& data)
{
    std::vector<std::string> headers = { "       ", " Generated ", " Processed ", " Rejected ", " Reject probability ", " Time in system ", " Waiting dispersion ", " Servicing dispersion " };
    data[0] = headers;
    for (int i = 1; i <= globalStat.getUserAmount(); i++)
    {
        std::string userStr = " User #" + std::to_string(i);
        userStr.append(" ");
        data[i][0] = userStr;

        int generated = globalStat.getProcessed(i - 1) + globalStat.getRejected(i - 1);
        data[i][1] = std::to_string(generated);

        int proc = globalStat.getProcessed(i - 1);
        data[i][2] = std::to_string(proc);

        int rej = globalStat.getRejected(i - 1);
        data[i][3] = std::to_string(rej);

        double rejectProb = globalStat.getRejectProbality(i - 1);
        data[i][4] = std::to_string(round(rejectProb * 10000) / 10000).substr(0, 6);

        double timeInSys = globalStat.getSimulationTime(i - 1);
        data[i][5] = std::to_string(round(timeInSys * 10000) / 10000).substr(0, 6);

        double waitTime = globalStat.getWaitingTimeDispersion(i - 1);
        data[i][6] = std::to_string(round(waitTime * 10000) / 10000).substr(0, 6);

        double servTime = globalStat.getServicingTimeDispersion(i - 1);
        data[i][7] = std::to_string(round(servTime * 10000) / 10000).substr(0, 6);
    }
}

void fillDataExecUsage(std::vector<std::vector<std::string>>& data)
{
    std::vector<std::string> headers = { " Executors ", " Executor usage "};
    data[0] = headers;

    for (int i = 1; i <= globalStat.getExecAmount(); i++)
    {
        std::string userStr = " Device #" + std::to_string(i);
        userStr.append(" ");
        data[i][0] = userStr;

        double usage = globalStat.getExecUsage(i - 1);
        //int j = i;
        //while (usage > 1.0)
        //{
        //    usage = globalStat.getExecUsage(j) + 0.0013;
        //    j++;
        //}
        data[i][1] = std::to_string(round(usage * 10000) / 10000).substr(0, 6);
    }
}

void stepMode()
{
    step step = globalStat.getStepVector()[step_];

    std::string actionText = step.actionString;

    double time = step.time;

    int stepNum = step_ + 1;

    std::cout << "\n Step - " << stepNum << "/" << globalStat.getStepVector().size();
    std::cout << "\n Time - " << std::fixed << std::setprecision(5) << time;
    std::cout << "\n Step action: " << actionText << "\n\n";

    std::vector<std::vector<std::string>> data(2, std::vector<std::string>((globalStat.getUserAmount() + 1)));

    for (int i = 0; i <= globalStat.getBufferSize(); i++)
    {
        if (i == 0)
        {
            data[0][i] = "User Number:";
            data[1][i] = "Last generated request:";
            continue;
        }
        std::string buffer = "User #";
        buffer.append(std::to_string(i));
        std::string curObj;

        if (step.userStateVector[i - 1] == nullptr)
        {
            curObj = "-";
        }
        else {
            curObj = std::to_string(step.userStateVector[i - 1]->getUserNumber() + 1) + "." + std::to_string(step.userStateVector[i - 1]->getRequestNumber());
        }

        data[0][i] = buffer;
        data[1][i] = curObj;
    }
    table(data, globalStat.getUserAmount() + 1, 2);
    std::cout << "\n\n";

    std::vector<std::vector<std::string>> data1(3, std::vector<std::string>((globalStat.getBufferSize()+1)));

    for (int i = 0; i <= globalStat.getBufferSize(); i++)
    {
        if (i == 0)
        {
            data1[0][i] = "Buffer Number:";
            data1[1][i] = "Status:";
            data1[2][i] = "Current request:";
            continue;
        }
        std::string buffer = "Buffer #";
        buffer.append(std::to_string(i));
        std::string status;
        std::string curObj;

        if (step.bufferStateVector[i-1] == nullptr)
        {
            status = "Free";
            curObj = "-";
        }
        else {
            status = "Busy"; 
            curObj = std::to_string(step.bufferStateVector[i-1]->getUserNumber() + 1) + "." + std::to_string(step.bufferStateVector[i - 1]->getRequestNumber());
        }

        data1[0][i] = buffer;
        data1[1][i] = status;
        data1[2][i] = curObj;
    }
    table(data1, globalStat.getBufferSize() + 1, 3);
    std::cout << "\n\n";

    std::vector<std::vector<std::string>> data2(3, std::vector<std::string>((globalStat.getExecAmount() + 1)));

    for (int i = 0; i <= globalStat.getExecAmount(); i++)
    {
        if (i == 0)
        {
            data2[0][i] = "Executor Number:";
            data2[1][i] = "Status:";
            data2[2][i] = "Current request:";
            continue;
        }
        std::string executor = "Executor #";
        executor.append(std::to_string(i));
        std::string status;
        std::string curObj;

        if (step.execStateVector[i-1] == nullptr)
        {
            status = "Free";
            curObj = "-";
        }
        else {
            status = "Busy";
            curObj = std::to_string(step.execStateVector[i - 1]->getUserNumber() + 1) + "." + std::to_string(step.execStateVector[i - 1]->getRequestNumber());
        }

        data2[0][i] = executor;
        data2[1][i] = status;
        data2[2][i] = curObj;
    }
    table(data2, globalStat.getExecAmount() + 1, 3);

    if (step_ == globalStat.getStepVector().size() - 1)
    {
        next = false;
    }
    else
    {
        next = true;
    }

    if (step_ == 0)
    {
        back = false;
    }
    else
    {
        back = true;
    }
}

void waveform(int argc, char* argv[])
{
    ep::init(&argc, argv);

    int countUsers = globalStat.getUserAmount();
    int countBuffer = globalStat.getBufferSize();
    int countExec = globalStat.getExecAmount();

    wave1.is_zero_x_line = true;
    wave1.is_zero_y_line = true;

    wave1.height = (20 + countUsers) * 10;
    wave1.width = 1200;

    wave2.is_zero_x_line = true;
    wave2.is_zero_y_line = true;

    wave2.height = (20 + countBuffer) * 10;
    wave2.width = 1200;

    wave3.is_zero_x_line = true;
    wave3.is_zero_y_line = true;

    wave3.height = (20 + countExec) * 10;
    wave3.width = 1200;

    std::vector<std::vector<double>> wavesUsers(countUsers);
    std::vector<std::vector<double>> wavesBuffer(countBuffer);
    std::vector<std::vector<double>> wavesExec(countExec);

    std::vector<std::vector<double>> timeUsers(countUsers);
    std::vector<std::vector<double>> timeBuffer(countBuffer);
    std::vector<std::vector<double>> timeExec(countExec);

    std::vector<ep::LineSpec> line_style1, line_style2, line_style3;

    for (int i = 0; i < countUsers; i++)
    {
        int k = 0;
        for (int j = step_; j < maxstep_ +2 && k < globalStat.getUserStepVect().size() - 1; ++j) {
            step step = globalStat.getUserStepVect()[k];
            if (j == 0)
            {
                wavesUsers[i].push_back(0 + i * 2);
                timeUsers[i].push_back(0.0);
            }
            if (j == maxstep_ + 1)
            {
                wavesUsers[i].push_back(0 + i * 2);
                timeUsers[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                break;
            }
            if (step.stepNumber > maxstep_ + 1)
            {
                wavesUsers[i].push_back(0 + i * 2);
                timeUsers[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                break;
            }
            if (step.userStateVector[i] != nullptr) {
                wavesUsers[i].push_back(0 + i * 2);
                wavesUsers[i].push_back(1 + i * 2);
                wavesUsers[i].push_back(0 + i * 2);
                timeUsers[i].push_back(globalStat.getUserStepVect()[k].time);
                timeUsers[i].push_back(globalStat.getUserStepVect()[k].time);
                timeUsers[i].push_back(globalStat.getUserStepVect()[k].time);
                k++;
            }
            else {
                wavesUsers[i].push_back(0 + i * 2);
                timeUsers[i].push_back(globalStat.getUserStepVect()[k].time);
                k++;
            }
            
        }
        line_style1.push_back(ep::LineSpec(1, 0, 1));
    }

    for (int i = 0; i < countBuffer; i++)
    {
        int k = 0;
        requestPointer prevReq = nullptr;
        requestPointer prevBuff = globalStat.getBufferStepVect()[k].bufferStateVector[i];
        for (int j = step_; j < maxstep_ + 1 && k < globalStat.getBufferStepVect().size() - 1; ++j) {
            if (j == 0)
            {
                wavesBuffer[i].push_back(0 + i * 2);
                timeBuffer[i].push_back(0.0);
            }
            step step = globalStat.getBufferStepVect()[k];
            if (maxstep_ + 1 == 1)
            {
                wavesBuffer[i].push_back(0 + i * 2);
                timeBuffer[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                continue;
            }
            if (step.stepNumber > maxstep_ + 1)
            {
                wavesBuffer[i].push_back(wavesBuffer[i][wavesBuffer[i].size()-1]);
                wavesBuffer[i].push_back(0 + i * 2);
                wavesBuffer[i].push_back(0 + i * 2);
                timeBuffer[i].push_back(timeBuffer[i][timeBuffer[i].size() - 1]);
                timeBuffer[i].push_back(timeBuffer[i][timeBuffer[i].size() - 1]);
                timeBuffer[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                break;
            }
            if (step.bufferStateVector[i] != nullptr && step.bufferStateVector[i] != prevBuff && prevBuff != nullptr)
            {
                wavesBuffer[i].push_back(1 + i * 2);
                wavesBuffer[i].push_back(0 + i * 2);
                wavesBuffer[i].push_back(1 + i * 2);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                if (globalStat.getBufferStepVect()[k + 1].bufferStateVector[i] == nullptr)
                {
                    wavesBuffer[i].push_back(0 + i * 2);
                    timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                    prevReq = globalStat.getStepVector()[j].bufferStateVector[i];
                    prevBuff = nullptr;
                    k++;
                    continue;
                }
            }
            else if (step.bufferStateVector[i] == nullptr && prevBuff != nullptr)
            {
                wavesBuffer[i].push_back(1 + i * 2);
                wavesBuffer[i].push_back(0 + i * 2);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
            }
            else if (step.bufferStateVector[i] == prevBuff && prevBuff != nullptr)
            {
                if (k == 0)
                {
                    wavesBuffer[i].push_back(0 + i * 2);
                    timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                }
                wavesBuffer[i].push_back(1 + i * 2);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                if (globalStat.getBufferStepVect()[k + 1].bufferStateVector[i] == nullptr)
                {
                    wavesBuffer[i].push_back(0 + i * 2);
                    timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                    prevReq = globalStat.getStepVector()[j].bufferStateVector[i];
                    prevBuff = nullptr;
                    k++;
                    continue;
                }
            }
            else if (step.bufferStateVector[i] != nullptr && prevBuff == nullptr) {
                wavesBuffer[i].push_back(0 + i * 2);
                wavesBuffer[i].push_back(1 + i * 2);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                if (globalStat.getBufferStepVect()[k + 1].bufferStateVector[i] == nullptr)
                {
                    wavesBuffer[i].push_back(0 + i * 2);
                    timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
                    prevReq = globalStat.getStepVector()[j].bufferStateVector[i];
                    prevBuff = nullptr;
                    k++;
                    continue;
                }
            }
            else
            {
                wavesBuffer[i].push_back(0 + i * 2);
                timeBuffer[i].push_back(globalStat.getBufferStepVect()[k].time);
            }
            prevReq = globalStat.getStepVector()[j].bufferStateVector[i];
            prevBuff = step.bufferStateVector[i];
            k++;
        }
        line_style2.push_back(ep::LineSpec(1, 0, 1));
    }

    for (int i = 0; i < countExec; i++) {
        int k = 0;
        requestPointer prevReq = nullptr;
        requestPointer prevExec = globalStat.getExecStepVect()[k].execStateVector[i];
        for (int j = step_; j < maxstep_ + 1; ++j) {
            if (j == 0)
            {
                wavesExec[i].push_back(0 + i * 2);
                timeExec[i].push_back(0.0);
            }
            if (k > globalStat.getExecStepVect().size()-1)
            {
                wavesExec[i].push_back(wavesExec[i][wavesExec[i].size() - 1]);
                wavesExec[i].push_back(0 + i * 2);
                wavesExec[i].push_back(0 + i * 2);
                timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                timeExec[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                break;
            }
            step step = globalStat.getExecStepVect()[k];
            if (maxstep_ + 1 == 1)
            {
                wavesExec[i].push_back(0 + i * 2);
                timeExec[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                continue;
            }
            if (j == globalStat.getStepVector().size()-1)
            {
                wavesExec[i].push_back(wavesExec[i][wavesExec[i].size() - 1]);
                wavesExec[i].push_back(0 + i * 2);
                wavesExec[i].push_back(0 + i * 2);
                timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                timeExec[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                break;
            }
            if (globalStat.getStepVector()[j].stepNumber != step.stepNumber)
            {
                if (j == maxstep_)
                {
                    wavesExec[i].push_back(wavesExec[i][wavesExec[i].size() - 1]);
                    wavesExec[i].push_back(0 + i * 2);
                    wavesExec[i].push_back(0 + i * 2);
                    timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                    timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                    timeExec[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                    break;
                }
                continue;
            }
            if (step.execStateVector[i] != nullptr && step.execStateVector[i] != prevExec && prevExec != nullptr)
            {
                wavesExec[i].push_back(1 + i * 2);
                wavesExec[i].push_back(0 + i * 2);
                wavesExec[i].push_back(1 + i * 2);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                if (k < globalStat.getExecStepVect().size() - 1 && globalStat.getExecStepVect()[k + 1].execStateVector[i] == nullptr)
                {
                    wavesExec[i].push_back(0 + i * 2);
                    timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                    prevReq = globalStat.getStepVector()[j].execStateVector[i];
                    prevExec = nullptr;
                    k++;
                    continue;
                }
            }
            else if (step.execStateVector[i] == nullptr && prevExec != nullptr)
            {
                wavesExec[i].push_back(1 + i * 2);
                wavesExec[i].push_back(0 + i * 2);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
            }
            else if (step.execStateVector[i] == prevExec && prevExec != nullptr)
            {
                if (k == 0)
                {
                    wavesExec[i].push_back(0 + i * 2);
                    timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                }
                wavesExec[i].push_back(1 + i * 2);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                //if (globalStat.getExecStepVect()[k + 1].execStateVector[i] == nullptr)
                //{
                //    wavesExec[i].push_back(0 + i * 2);
                //    timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                //    prevReq = globalStat.getStepVector()[j].execStateVector[i];
                //    prevExec = nullptr;
                //    k++;
                //    continue;
                //}
            }
            else if (step.execStateVector[i] != nullptr && prevExec == nullptr) {
                wavesExec[i].push_back(0 + i * 2);
                wavesExec[i].push_back(1 + i * 2);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                if (globalStat.getExecStepVect()[k + 1].execStateVector[i] == nullptr)
                {
                    wavesExec[i].push_back(0 + i * 2);
                    timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
                    prevReq = globalStat.getStepVector()[j].execStateVector[i];
                    prevExec = nullptr;
                    k++;
                    continue;
                }
            }
            else
            {
                wavesExec[i].push_back(0 + i * 2);
                timeExec[i].push_back(globalStat.getExecStepVect()[k].time);
            }
            prevReq = globalStat.getStepVector()[j].execStateVector[i];
            prevExec = step.execStateVector[i];
            k++;
            if (j == maxstep_)
            {
                wavesExec[i].push_back(wavesExec[i][wavesExec[i].size() - 1]);
                wavesExec[i].push_back(0 + i * 2);
                wavesExec[i].push_back(0 + i * 2);
                timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                timeExec[i].push_back(timeUsers[0][timeUsers[0].size() - 2]);
                timeExec[i].push_back(globalStat.getUserStepVect()[globalStat.getUserStepVect().size() - 1].time);
                break;
            }
        }
        line_style3.push_back(ep::LineSpec(1, 0, 1));
    }
    
    ep::plot2<double>("Users", ::wave1, timeUsers, wavesUsers, line_style1);
    ep::plot2<double>("Buffer", ::wave2, timeBuffer, wavesBuffer, line_style2);
    ep::plot2<double>("Executors", ::wave3, timeExec, wavesExec, line_style3);

    if (maxstep_ == globalStat.getStepVector().size() - 1)
    {
        next = false;
    }
    else
    {
        next = true;
    }

    if (maxstep_ == 0)
    {
        back = false;
    }
    else
    {
        back = true;
    }
}