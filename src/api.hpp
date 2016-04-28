#ifndef API
#define API

#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "messages.hpp"

#define MAX_RESPONSE_SIZE 128
#define SOURCE 0x01

#define FT_ERROR -6
#define DEVICE_ERROR -7
#define FATAL_ERROR -8

#define MOVED_HOME_STATUS 3
#define MOVE_COMPLETED_STATUS 2
#define MOVE_STOPPED_STATUS 4

#define OTHER_MESSAGE 5
#define EMPTY 1

#define INVALID_DEST -10
#define INVALID_SOURCE -11
#define INVALID_CHANNEL -12

#define INVALID_PARAM_1 -15
#define INVALID_PARAM_2 -16
#define INVALID_PARAM_3 -17
#define INVALID_PARAM_4 -18
#define INVALID_PARAM_5 -19
                                   
static uint8_t DefaultDest(){
    return 0x50;
}

static uint8_t DefaultChanel8(){
    return 0x01;
}

static uint16_t DefaultChanel16(){
    return 0x01;
}

static uint8_t DeafultRate(){
    return 1;
}

static uint8_t DefaultStopMode(){
    return 0x02;
}

extern int OpenDevice(int index);

//----------------- Input buffer monitoring thread --------------------------------

extern int getInBuffDesc();

extern int monInit();

extern void monDestr();

//----------------- Device communication functions ---------------------------------

extern int CheckParams( uint8_t dest, int chanID);

extern int SendMessage(Message &message);

extern int CheckIncomingQueue(uint16_t *ret_msgID);

extern int EmptyIncomingQueue();

extern int GetResponseMess(uint16_t expected_msg, int size, uint8_t *mess );



namespace device_calls{
    
// ------------------------- Generic device calls ------------------------------

extern int Identify(uint8_t dest = DefaultDest());

extern int EnableChannel(uint8_t dest = DefaultDest(), uint8_t chanel = DefaultChanel8());

extern int DisableChannel(uint8_t dest = DefaultDest(), uint8_t chanel = DefaultChanel8());

extern int ChannelState(GetChannelState *info, uint8_t dest = DefaultDest(), uint8_t chanel = DefaultChanel8());

extern int DisconnectHW(uint8_t dest = DefaultDest());

extern int StartUpdateMess(uint8_t rate = DeafultRate(), uint8_t dest = DefaultDest());

extern int StopUpdateMess(uint8_t dest = DefaultDest());

extern int GetHwInfo(HwInfo *message, uint8_t dest = DefaultDest());

extern int GetBayUsed(GetRackBayUsed *message, uint8_t bayID, uint8_t dest = DefaultDest());

extern int GetHubUsed(GetHubBayUsed *message, uint8_t dest = DefaultDest());

//-------------------------- Motor control calls ------------------------------

extern int FlashProgYes(uint8_t dest = DefaultDest());

extern int FlashProgNo(uint8_t dest = DefaultDest());

extern int SetPositionCounter(int32_t pos, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetPositionCounter(GetPosCounter *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetEncoderCounter(int32_t count, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetEncoderCounter(GetEncCount *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetVelocityP(int32_t acc, int32_t maxVel, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetVelocityP(GetVelocityParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetJogP(uint16_t mode, int32_t stepSize, int32_t vel, int32_t acc, uint16_t stopMode, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetJogP(GetJogParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetPowerUsed(uint16_t rest_power, uint16_t move_power, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetPowerUsed(GetPowerParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetBacklashDist(uint32_t dist, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetBacklashDist(GetGeneralMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetRelativeMoveP(uint32_t dist, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetRelativeMoveP(GetRelativeMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetAbsoluteMoveP(uint32_t pos, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetAbsoluteMoveP(GetAbsoluteMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetHomingVel(uint32_t vel, int8_t dest = DefaultDest(),  uint16_t channel = DefaultChanel16());

extern int GetHomingVel(GetHomeParams *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetLimitSwitchConfig(uint16_t CwHwLim, uint16_t CCwHwLim, uint16_t CwSwLim, uint16_t CCwSwLim, uint16_t mode, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetLimitSwitchConfig(GetLimitSwitchParams *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int MoveToHome(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int StartSetRelativeMove(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int StartRelativeMove(int32_t dist, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int StartSetAbsoluteMove(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int StartAbsoluteMove(int32_t pos, uint8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int StartJogMove(uint8_t direction, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int StartSetVelocityMove( uint8_t direction, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int StopMovement(uint8_t stopMode = DefaultStopMode(), uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetAccelerationProfile(uint16_t index, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetAccelerationProfile(GetBowIndex *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetLedP(uint16_t mode, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());


extern int GetLedP(GetLedMode *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SetButtons(uint16_t mode, int32_t pos1, int32_t pos2, uint16_t timeout, int8_t dest = DefaultDest(), uint16_t channel = DefaultChanel16());

extern int GetButtonsInfo(GetButtonParams *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

// only requests for data, automatically stored in device info
extern int ReqStatus(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

// only requests for data, automatically s
extern int ReqDcStatus(uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int SendServerAlive(uint8_t dest = DefaultDest());

extern int GetStatBits(GetStatusBits *message ,uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int DisableEomMessages(uint8_t dest = DefaultDest());

extern int EnableEomMessages(uint8_t dest = DefaultDest());

extern int CreateTrigger(uint8_t mode, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());

extern int GetMotorTrigger(GetTrigger *message, uint8_t dest = DefaultDest(), uint8_t channel = DefaultChanel8());


} // namespace device_calls


#endif 