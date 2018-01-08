#include "bci.h"


#define UNPACK_OK 1
#define BAD_MESSAGE 0



// ###################################
// ### values read from controller ###
// ###################################
// Number of output variables
#define OUTVAR_NUM 4
// Number of controlled motors
#define NUM_OF_MOTORS 4

//number of auxiliary varibles
#define NUM_OF_AUX 1

//output varibles
#define POS    0
#define CUR    1
#define SPD    2
#define VOL    3
#define AUX    4

#define POS_1  0
#define POS_2  1
#define POS_3  2
#define POS_4  3

#define CUR_1  0
#define CUR_2  1
#define CUR_3  2
#define CUR_4  3

#define SPD_1  0
#define SPD_2  1
#define SPD_3  2
#define SPD_4  3

#define VOL_1  0
#define VOL_2  1
#define VOL_3  2
#define VOL_4  3

#define AUX_1  0



// ####################################
// ### values written to controller ###
// ####################################

//number of input variables
#define INVAR_NUM 4

#define ENABLE   3
#define SETPOINT 0
#define P_GAIN   1
#define I_GAIN   2

#define ENABLE_1  0
#define ENABLE_2  1


#define SETPOINT_1  0
#define SETPOINT_2  1


#define P_GAIN_1  0
#define P_GAIN_2  1


#define I_GAIN_1  0
#define I_GAIN_2  1





// #######################################
// ### CANOpen protocol implementation ###
// #######################################

int unpackMessage(BCI_ts_CanMsg msg, int *valueID1, int *valueID2, int *value1, int *value2);


int packMessage(BCI_ts_CanMsg *msg, int controller, int valueID1, int value1, int value2);