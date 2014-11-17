#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#include <uaclientsdk.h>

static const int PARAMETERCOUNT = 6;

// Module Input
enum ModuleMode
{
    ModuleModeInit = 0,
    ModuleModeAuto = 1,
    ModuleModeManual = 2,
    ModuleModeReset = 3
};

enum ModuleConnectionStatus
{
    ModuleConnectionDisconnected = 0,
    ModuleConnectionConnected = 1
};
struct ParameterInput
{
    UaString string; // ......1 (e.g. 14000-1)
    OpcUa_Double value; // ......0 (e.g. 14000-0)

    ParameterInput(): value(0) { string = "";};
};

struct ParameterInputArray
{
    ParameterInput paramInput[PARAMETERCOUNT];
};

struct SkillInput
{
    OpcUa_Boolean execute; // ....1 (e.g. 14001)
    ParameterInput parameterInput[6]; // ....1. (e.g. 1400-1-x)

    SkillInput(): execute(false) {};
};

struct ModuleInput
{
    OpcUa_Boolean connectionTestInput; // 1000
    OpcUa_Boolean emergencyStop; // 1100
    OpcUa_Int16 moduleMode; // 1200
    OpcUa_Double positionInput; // 1300
    SkillInput skillInput[16]; // 1400..1409

    ModuleInput(): connectionTestInput(false), emergencyStop(false),
        positionInput(0), moduleMode(-1) { };
};

// Module Output
struct ParameterOutput
{
    OpcUa_Double defaultParameter; // 22XY1
    OpcUa_Boolean dummy;  // 22XY2
    OpcUa_UInt32 id;  // 22XY3
    OpcUa_Double maxValue;  // 22XY4
    OpcUa_Double minValue;  // 22XY5
    UaString name;  // 22XY6
    OpcUa_Boolean required;  // 22XY7
    UaString unit;  // 22XY8
};

struct SkillOutput
{
    OpcUa_Boolean activated; // 22X1
    OpcUa_Boolean busy; // 22X2
    OpcUa_Boolean done; // 22X3
    OpcUa_Boolean dummy; // 22X4
    OpcUa_Boolean error; // 22X5
    OpcUa_UInt32 id; // 22X6
    UaString name; // 22X7
    ParameterOutput parameterOutput[6]; // 22XY.
    OpcUa_Boolean ready; // 22X9
};

struct StateValue //TODO
{
    OpcUa_Boolean dummy; // 23Z1
    OpcUa_UInt32 value; // 23Z2
    UaString name; // 23Z3
    UaString description; // 23Z4
    UaString unit; // 24Z5
};

struct ModuleOutput
{
    OpcUa_Boolean connected; // 200
    OpcUa_Boolean connectionTestOutput; // 201
    UaString currentTaskDescription; // 202
    OpcUa_Boolean dummy; // 203
    OpcUa_Boolean error; // 204
    UaString errorDescription; // 205
    OpcUa_UInt32 errorId; // 206
    OpcUa_UInt32 id; // 207
    UaString ip; // 208
    OpcUa_Boolean idle; // 209
    UaString name; // 210
    OpcUa_Boolean positionSensor; // 211
    OpcUa_Double positionOutput; // 212
    SkillOutput skillOutput[16]; // 22..
    StateValue stateValue[11]; // 23.. // TODO!
};

struct Parameter
{
    OpcUa_Boolean dummy;
    OpcUa_UInt32 id;
    UaString name;
    UaString unit;
    OpcUa_Boolean required;
    OpcUa_Double defaultParameter;
    OpcUa_Double minvalue;
    OpcUa_Double maxValue;
    OpcUa_Double value;
    UaString stringValue;
};

struct Skill
{
    UaString assignedModuleName;
    OpcUa_UInt32 assignedModuleId;
    OpcUa_Double assignedModulePosition;
    OpcUa_Boolean dummy;
    OpcUa_UInt32 id;
    UaString name;
    Parameter parameter[11];
    OpcUa_Int32 state;
};

enum TaskState
{
    TaskUnassigned = 0,
    TaskAssigned = 1,
    TaskWaitingForSkillReady = 2,
    TaskInProgress = 3,
    TaskDone = 4,
    TaskAborted = 5,
    TaskError = 6
};

struct ProductionTask
{
    OpcUa_Int32 taskNumberInStructure;
    OpcUa_Boolean dummy;
    UaString name;
    OpcUa_Int32 taskId;
    OpcUa_Int32 recipeId;
    UaString timestamp;
    OpcUa_Int32 taskState;
    Skill skill[51];
};

struct taskSkillQueue
{
    int skillNumberInTask;
    int skillId;
    int skillState;
};

struct skillModuleList
{
    int moduleNumber;
    int skillId;
    int skillPos;
};

struct matchedSkill
{
    int moduleNumber;
    int moduleSkillState;
    bool moduleSkillReady;
    int skillId;
    int taskSkillState;
    int skillPosition;
    bool blocked;
    matchedSkill() : moduleNumber(0), moduleSkillState(-1), moduleSkillReady(false),
        skillId(-1), taskSkillState(-1), skillPosition(0), blocked(false) {};
};

struct ManualModuleParameter
{
    OpcUa_Int32 id;
    UaString name;
    UaString stringValue;
    UaString unit;
    OpcUa_Double value;
    ManualModuleParameter(): id(0), value(0) { UaString voidString = UaString(""); name = voidString; stringValue = voidString; unit = voidString;};
};

struct ManualModuleData
{
    OpcUa_Boolean iExecute;
    OpcUa_Boolean oBusy;
    OpcUa_Boolean oDone;
    OpcUa_Boolean oError;
    OpcUa_Boolean oReady;
    OpcUa_UInt16 oErrorId;
    OpcUa_Double oPosition;
    UaString iSkilldescription;
    OpcUa_UInt16 iSkillId;
    OpcUa_UInt16 iTaskId;
    ManualModuleParameter iParameter[PARAMETERCOUNT];

    ManualModuleData() : iExecute(false), oBusy(false), oDone(false), oError(false), oErrorId(0),
        oPosition(0),
        iSkillId(0),
        iTaskId(0) { iSkilldescription = UaString("0"); };
};

#endif // DATASTRUCTURES_H