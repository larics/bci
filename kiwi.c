#include "kiwi.h"
#include "bci.h"




int BCI_CreateCANMsg (BCI_ts_CanMsg * CANMsg, UINT32 ID, UINT8 * Data,  // Databytes of the message
                      UINT8 DLC,       
                      UINT8 MFF);       




int unpackMessage(BCI_ts_CanMsg msg, int *valueID1, int *valueID2, int *value1, int *value2) 
{
	int ret = BAD_MESSAGE;
	int nodeID = msg.id % 16;
	int VAR_ID1 = (msg.id / 256) * 2 - 1;
	int VAR_ID2 = (msg.id / 256) * 2;
	*value1 = ((msg.a_data[3] * 256 + msg.a_data[2]) * 256 + msg.a_data[1]) * 256 + msg.a_data[0]; 
	*value2 = ((msg.a_data[7] * 256 + msg.a_data[6]) * 256 + msg.a_data[5]) * 256 + msg.a_data[4];
    //Vrsta mjerenja(NUM_OF_MOTORS*(0 do OUTVAR_NUM))) + (0 do broj kontrolera)*2 motora po kontroleru + broj motora(0 ili 1)
	*valueID1 = ((VAR_ID1 - 1)/2)*NUM_OF_MOTORS + (nodeID - 1)*2+(VAR_ID1-1)%2 ; 
	*valueID2 = ((VAR_ID2 - 1)/2)*NUM_OF_MOTORS + (nodeID - 1)*2+(VAR_ID2-1)%2 ;
    ret = UNPACK_OK;
	return ret;

}



int packMessage(BCI_ts_CanMsg *msg, int controller, int valueID1, int value1, int value2)
{
	UINT8 PacketData[8];
	int nodeID = controller+1;
	int VAR_ID = valueID1 + 2;
	int msg_id = VAR_ID * 0x100 + nodeID;


	PacketData[0] = (value1 & 0x000000FF) >>  0;
    PacketData[1] = (value1 & 0x0000FF00) >>  8;
    PacketData[2] = (value1 & 0x00FF0000) >> 16;
    PacketData[3] = (value1 & 0xFF000000) >> 24;

	PacketData[4] = (value2 & 0x000000FF) >>  0;
    PacketData[5] = (value2 & 0x0000FF00) >>  8;
    PacketData[6] = (value2 & 0x00FF0000) >> 16;
    PacketData[7] = (value2 & 0xFF000000) >> 24;

    BCI_CreateCANMsg(msg, msg_id, PacketData, 8, BCI_MFF_11_DAT);


}