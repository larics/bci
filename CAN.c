/*
 * sfuntmpl_basic.c: Basic 'C' template for a level 2 S-function.
 *
 * Copyright 1990-2013 The MathWorks, Inc.
 */


/*
 * You must specify the S_FUNCTION_NAME as the name of your S-function
 * (i.e. replace sfuntmpl_basic with the name of your S-function).
 */

#define S_FUNCTION_NAME  CAN
#define S_FUNCTION_LEVEL 2

/*
 * Need to include simstruc.h for the definition of the SimStruct and
 * its associated macro definitions.
 */
#include "simstruc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bci.h"
#include "kiwi.h"




BCI_BRD_HDL CANBoard;
int ret;
int flag = 0;

#define CAN0_ID 0x10
#define CAN1_ID 0x11


int BCI_Status2Str (UINT16 Status, char *StatusStr);
int BCI_ShowCANMsg (BCI_ts_CanMsg * CANMsg);
int BCI_CreateCANMsg (BCI_ts_CanMsg * CANMsg, UINT32 ID, UINT8 * Data,  // Databytes of the message
        UINT8 DLC,        // data length code (Bit 0..3)
        UINT8 MFF);       // Message frame format
void BCI_MDelay (unsigned int msec);


/* Error handling
 * --------------
 *
 * You should use the following technique to report errors encountered within
 * an S-function:
 *
 *       ssSetErrorStatus(S,"Error encountered due to ...");
 *       return;
 *
 * Note that the 2nd argument to ssSetErrorStatus must be persistent memory.
 * It cannot be a local variable. For example the following will cause
 * unpredictable errors:
 *
 *      mdlOutputs()
 *      {
 *         char msg[256];         {ILLEGAL: to fix use "static char msg[256];"}
 *         sprintf(msg,"Error due to %s", string);
 *         ssSetErrorStatus(S,msg);
 *         return;
 *      }
 *
 */

/*====================*
 * S-function methods *
 *====================*/

/* Function: mdlInitializeSizes ===============================================
 * Abstract:
 *    The sizes information is used by Simulink to determine the S-function
 *    block's characteristics (number of inputs, outputs, states, etc.).
 */
static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 0);  /* Number of expected parameters */
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        /* Return if number of expected != number of actual parameters */
        return;
    }
    
    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, OUTVAR_NUM*NUM_OF_MOTORS+NUM_OF_AUX);
    
    if (!ssSetNumInputPorts(S, INVAR_NUM)) return;
    ssSetInputPortWidth(S,  ENABLE,  NUM_OF_MOTORS);
    ssSetInputPortWidth(S,  SETPOINT,NUM_OF_MOTORS);
    ssSetInputPortWidth(S,  P_GAIN,  NUM_OF_MOTORS);
    ssSetInputPortWidth(S,  I_GAIN,  NUM_OF_MOTORS);
    
    ssSetInputPortRequiredContiguous(S,  ENABLE,  true); /*direct input signal access*/
    ssSetInputPortRequiredContiguous(S,  SETPOINT,true); /*direct input signal access*/
    ssSetInputPortRequiredContiguous(S,  P_GAIN,  true); /*direct input signal access*/
    ssSetInputPortRequiredContiguous(S,  I_GAIN,  true); /*direct input signal access*/
    
    /*
     * Set direct feedthrough flag (1=yes, 0=no).
     * A port has direct feedthrough if the input is used in either
     * the mdlOutputs or mdlGetTimeOfNextVarHit functions.
     */
    ssSetInputPortDirectFeedThrough(S,  ENABLE,  0);
    ssSetInputPortDirectFeedThrough(S,  SETPOINT,0);
    ssSetInputPortDirectFeedThrough(S,  P_GAIN,  0);
    ssSetInputPortDirectFeedThrough(S,  I_GAIN,  0);
    
    
    
    if (!ssSetNumOutputPorts(S, OUTVAR_NUM+NUM_OF_AUX)) return;
    ssSetOutputPortWidth(S,  POS, NUM_OF_MOTORS); //pozicija
    ssSetOutputPortWidth(S,  CUR, NUM_OF_MOTORS);  //struja
    ssSetOutputPortWidth(S,  SPD, NUM_OF_MOTORS);  //brzina
    ssSetOutputPortWidth(S,  VOL, NUM_OF_MOTORS);  //napon
    ssSetOutputPortWidth(S,  AUX, NUM_OF_AUX   );  //pomocna
    
    
    
    ssSetNumSampleTimes(S, 1);
    ssSetNumRWork(S, 0);
    ssSetNumIWork(S, 0);
    ssSetNumPWork(S, 0);
    ssSetNumModes(S, 0);
    ssSetNumNonsampledZCs(S, 0);
    
    /* Specify the sim state compliance to be same as a built-in block */
    ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);
    
    //Osigurava da se CAN blok izvodi sto prije moguce
    ssSetOptions(S, SS_OPTION_PLACE_ASAP);
}



/* Function: mdlInitializeSampleTimes =========================================
 * Abstract:
 *    This function is used to specify the sample time(s) for your
 *    S-function. You must register the same number of sample times as
 *    specified in ssSetNumSampleTimes.
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, 0.008);
    ssSetOffsetTime(S, 0, 0.0);
    
}



#define MDL_INITIALIZE_CONDITIONS   /* Change to #undef to remove function */
#if defined(MDL_INITIALIZE_CONDITIONS)
/* Function: mdlInitializeConditions ========================================
 * Abstract:
 *    In this function, you should initialize the continuous and discrete
 *    states for your S-function block.  The initial states are placed
 *    in the state vector, ssGetContStates(S) or ssGetRealDiscStates(S).
 *    You can also perform any other initialization activities that your
 *    S-function may require. Note, this routine will be called at the
 *    start of simulation and if it is present in an enabled subsystem
 *    configured to reset states, it will be call when the enabled subsystem
 *    restarts execution to reset the states.
 */
static void mdlInitializeConditions(SimStruct *S)
{
    int i = 0;
    real_T *Kiwi = ssGetRealDiscStates(S);
    for (i = 0; i < OUTVAR_NUM*NUM_OF_MOTORS+NUM_OF_AUX; i++)
        Kiwi[i] = 0;
}


#endif /* MDL_INITIALIZE_CONDITIONS */



#define MDL_START  /* Change to #undef to remove function */
#if defined(MDL_START)
/* Function: mdlStart =======================================================
 * Abstract:
 *    This function is called once at start of model execution. If you
 *    have states that should be initialized once, this is the place
 *    to do it.
 */
static void mdlStart(SimStruct *S)
{
    char DevFileName[64] = "/dev/can0";
    extern BCI_BRD_HDL CANBoard;
    extern int ret;
    ret = BCI_OpenBoard (&CANBoard, DevFileName);
    //BCI_GetBoardInfo (CANBoard, &BoardInfo);
    //ret = BCI_GetBoardInfo (CANBoard, &BoardInfo));
    //BCI_MDelay (1500);
    //BCI_ResetCan (CANBoard, Controller);
    //BCI_InitCan (CANBoard, Controller, BCI_125KB, 0);
    //BCI_ConfigRxQueue (CANBoard, Controller, BCI_POLL_MODE);
}
#endif /*  MDL_START */



/* Function: mdlOutputs =======================================================
 * Abstract:
 *    In this function, you compute the outputs of your S-function
 *    block.
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{
    extern int ret, flag;
    
    real_T *Kiwi = ssGetRealDiscStates(S);
    
    UINT8 Controller = 0;
    int valueID1, valueID2, value1, value2, i;
    BCI_ts_CanMsg msg_r;
    
    if (flag == 0) {
        BCI_InitCan (CANBoard, Controller, BCI_1000KB, 0);
        BCI_ConfigRxQueue (CANBoard, Controller, BCI_POLL_MODE);
        //BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, 10);
        //BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, 11);
        //BCI_RegisterRxId (CANBoard, Controller, BCI_MFF_29_DAT, 12);
        BCI_SetAccMask (CANBoard, Controller, BCI_11B_MASK, 0, BCI_ACC_ALL);
        //BCI_SetAccMask (CANBoard, Controller, BCI_29B_MASK, 0x0, BCI_ACC_ALL);
        BCI_StartCan (CANBoard, Controller);
    }
    flag = 1;
    
    //resetiraj pomocnu varijablu    
    Kiwi[AUX*NUM_OF_MOTORS+AUX_1]=0;
    //primi trenutna stanja
    while (BCI_ReceiveCanMsg(CANBoard, Controller, &msg_r, BCI_NO_WAIT) == BCI_OK)
    {
        unpackMessage(msg_r, &valueID1, &valueID2, &value1, &value2);
        //pomocna varijabla zapisuje 1 na potencije rednih brojeva dolaznih paketa
        Kiwi[AUX*NUM_OF_MOTORS+AUX_1]+=pow((double)10.0,valueID1)+pow((double)10.0,valueID2);
        if(valueID1<(OUTVAR_NUM*NUM_OF_MOTORS) && valueID2<(OUTVAR_NUM*NUM_OF_MOTORS))
        {
            Kiwi[valueID1] = value1;
            Kiwi[valueID2] = value2;
        }
    }
    
    
    //dohvati izlazne varijable
    real_T       *position = ssGetOutputPortSignal(S,POS);
    real_T       *current = ssGetOutputPortSignal(S,CUR);
    real_T       *speed = ssGetOutputPortSignal(S,SPD);
    real_T       *volatage = ssGetOutputPortSignal(S,VOL);
    real_T       *auxiliary = ssGetOutputPortSignal(S,AUX);
    
    //zapisi izlaze
    for(i=0;i<NUM_OF_MOTORS;i++){
    position[i] = Kiwi[POS*NUM_OF_MOTORS+i];    
    current[i] = Kiwi[CUR*NUM_OF_MOTORS+i];
    speed[i] = Kiwi[SPD*NUM_OF_MOTORS+i];  
    volatage[i] = Kiwi[VOL*NUM_OF_MOTORS+i];
    }
    auxiliary[AUX_1]=Kiwi[AUX*NUM_OF_MOTORS+AUX_1];
}



#define MDL_UPDATE  /* Change to #undef to remove function */
#if defined(MDL_UPDATE)
/* Function: mdlUpdate ======================================================
 * Abstract:
 *    This function is called once for every major integration time step.
 *    Discrete states are typically updated here, but this function is useful
 *    for performing any tasks that should only take place once per
 *    integration step.
 */
static void mdlUpdate(SimStruct *S, int_T tid)
{   
    UINT8 Controller = 0;
    BCI_ts_CanMsg msg_r, msg_s;
    int target,num_of_controllers;
    real_T u1, u2;
    // dohvati ulaze
    const real_T *enables = (const real_T*) ssGetInputPortSignal(S,ENABLE);
    const real_T *setpoints = (const real_T*) ssGetInputPortSignal(S,SETPOINT);
    const real_T *p_gains = (const real_T*) ssGetInputPortSignal(S,P_GAIN);
    const real_T *i_gains = (const real_T*) ssGetInputPortSignal(S,I_GAIN);
    
    num_of_controllers= (NUM_OF_MOTORS + 2 - 1) / 2; // ovo je ekvivalentno ceil(x/y)
    
    num_of_controllers=1;//  override jer shema nije modificirana da radi s varijabilnim brojem motora
    
    for (target=0;target<num_of_controllers;target++)
    {
        //zasicenje 1
        if(setpoints[target*2+SETPOINT_1]>1000)
            u1=1000;
        else if(setpoints[target*2+SETPOINT_1]<-1000)
            u1=-1000;
        else
            u1=setpoints[target*2+SETPOINT_1];
        
        //zasicenje 2
        if(setpoints[target*2+SETPOINT_2]>1000)
            u2=1000;
        else if(setpoints[target*2+SETPOINT_2]<-1000)
            u2=-1000;
        else
            u2=setpoints[target*2+SETPOINT_2];
        
        //posalji ulaze na kontroler
        packMessage(&msg_s, target, SETPOINT, (int)( setpoints[target*2+SETPOINT_1] ), (int)( setpoints[target*2+SETPOINT_2]));
        BCI_TransmitCanMsg (CANBoard, Controller, &msg_s);
        
        //posalji proporcionalna pojacanja na kontroler
        packMessage(&msg_s, target, P_GAIN, (int)( p_gains[target*2+P_GAIN_1] ), (int)( p_gains[target*2+P_GAIN_2]));
        BCI_TransmitCanMsg (CANBoard, Controller, &msg_s);
        
        //posalji integralna pojacanja na kontroler
        packMessage(&msg_s, target, I_GAIN, (int)( i_gains[target*2+I_GAIN_1] ), (int)( i_gains[target*2+I_GAIN_2]));
        BCI_TransmitCanMsg (CANBoard, Controller, &msg_s);
        
        //posalji enable-e na kontroler
        packMessage(&msg_s, target, ENABLE, (int)( enables[target*2+ENABLE_1] ), (int)( enables[target*2+ENABLE_2]));
        BCI_TransmitCanMsg (CANBoard, Controller, &msg_s);
    }
}
#endif /* MDL_UPDATE */



#define MDL_DERIVATIVES  /* Change to #undef to remove function */
#if defined(MDL_DERIVATIVES)
/* Function: mdlDerivatives =================================================
 * Abstract:
 *    In this function, you compute the S-function block's derivatives.
 *    The derivatives are placed in the derivative vector, ssGetdX(S).
 */
static void mdlDerivatives(SimStruct *S)
{
}
#endif /* MDL_DERIVATIVES */



/* Function: mdlTerminate =====================================================
 * Abstract:
 *    In this function, you should perform any actions that are necessary
 *    at the termination of a simulation.  For example, if memory was
 *    allocated in mdlStart, this is the place to free it.
 */
static void mdlTerminate(SimStruct *S)
{
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif

