#include "unistd.h"

/* Project headers */
#include "Log.h"
#include "Scheduler/include/SchedulerInterface.h"
#include "CommonTypes.h"
#include "TB2820.h"
#include "HardwareRecord/TB2820Record.h"

#define TB2820_PROC_VERSION_MAJOR_NUMBER 2U
#define TB2820_PROC_VERSION_MINOR_NUMBER 0U
#define TB2820_PROC_VERSION_REVISION_NUMBER 0U

static void ShowBoardInfo(short boardNum)
{
    if (boardNum <= 0)
    {
    printf("Invalid board number: %d\n", boardNum);
    return;
}

TB_Version hwVersion;
TB_Version fwVersion;
TB2820_Handler *handler = nullptr;
TB2820_Open(&handler, boardNum - 1);
if (TB2820_GetHardwareVersion(handler, &hwVersion) != RETURN_OK ||
    TB2820_GetFirmwareVersion(handler, &fwVersion) != RETURN_OK)
{
    printf("Failed to get version info for board: %d\n", boardNum);
    return;
}

printf("BoardType: TB2820\n"
       "BoardNumber: %d\n"
       "HardwareVersion: %x.%x\n"
       "FirmwareVersion: %x.%x.%x\n",
       boardNum,
       hwVersion.major, hwVersion.minor,
       fwVersion.major, fwVersion.minor, fwVersion.revision);
TB2820_Close(&handler);

void ShowHelp()
{
    printf("\nhrrtp-tb2820-io - An I/O program of HIRAIN Real-Time Simulation Engine\n\n");
    printf("Syntax:\n");
    printf(" hrrtp-tb2820-io [option]\n\n");
    printf("Options:\n");
    printf(" -b Start with specified board number N in simulator.\n");
    printf(" -d Start in debug mode, does not depended on tasksched, main loop is 1ms.\n");
    printf(" -v Show version number.\n");
    printf(" -h Show this help.\n");
    printf(" -i Show this board information.\n\n");
}

int main(int argc, char *argv[])
{
    short boardNumber = 0;
    bool debugMode = false;
    int opt = 0;
while ((opt = getopt(argc, argv, "vb:dhi")) != EOF)
{
    switch (opt)
    {
    case 'b':
        boardNumber = atoi(optarg);
        break;
    case 'd':
        debugMode = true;
        break;
    case 'v':
        printf("hrrtp-tb2820-io version %u.%u.%u\n",
               TB2820_PROC_VERSION_MAJOR_NUMBER,
               TB2820_PROC_VERSION_MINOR_NUMBER,
               TB2820_PROC_VERSION_REVISION_NUMBER);
        printf("hrrtp-tb2820-io use interface version 3.1.x\n");
        TB2820_PrintVersion();
        exit(EXIT_SUCCESS);
    case 'h':
        ShowHelp();
        exit(EXIT_SUCCESS);
    case 'i':
        ShowBoardInfo(boardNumber);
        exit(EXIT_SUCCESS);
    default:
        break;
    }
}

HR_InitializeFileLogger();

if (boardNumber < 1)
{
    HR_Fatal("Task cannot start with invalid board number %d!", boardNumber);
    exit(EXIT_FAILURE);
}

HR_Info("Task start with board number %d.", boardNumber);

auto hardwarerecord = std::make_shared<TB2820Record>();
std::shared_ptr<IoTask> task(new TB2820(boardNumber, hardwarerecord));
try
{
    task->Initialize("/home/SimulationFile/hardwarerecord.db");
    HR_Info("Task initialize complete.");

    std::function<void()> scheduling;
    if (debugMode)
    {
        scheduling = []() { WaitNextFrame(); };
    }
    else
    {
        scheduling = []() { usleep(1000); };
    }

    task->Run(scheduling);
    task->Terminate();
}
catch (std::exception &e)
{
    HR_Fatal("Task abnormal exit, exception message is \"%s\".", e.what());
    try
    {
        task->Terminate();
    }
    catch (const std::exception &ex)
    {
        HR_Fatal("Task abnormal terminate, exception message is \"%s\".", ex.what());
    }
}

return EXIT_SUCCESS;