#ifndef RADIO_PROTOCOL
#define RADIO_PROTOCOL

#define GATEWAY_ID 0x00

#define PKG_INTENSITY_LEN 0x08
#define PKG_MOVEMENT_LEN 0x05
#define PKG_HEATER_LEN 0x0c
#define PKG_ACK_LEN 0x06
#define PKG_HEATER_STATUS_LEN 0x0f
#define PKG_HEATER_REQUEST_LEN 0x06


#define PKG_TYPE_INVALID 0x00
#define PKG_TYPE_ACK 0x05
#define PKG_TYPE_INTENSITY 0x02
#define PKG_TYPE_MOVEMENT 0x03
#define PKG_TYPE_HEATER 0x04
#define PKG_TYPE_HEATER_STATUS 0x05
#define PKG_TYPE_HEATER_REQUEST 0x06

#define HEATER_FAULT_NONE 0x1
#define HEATER_FAULT_GAS_HIGH 0x2
#define HEATER_FAULT_HW_MALFUNCTION 0x4

#define HEATER_STATUS_ON 0x1
#define HEATER_STATUS_OFF 0x2
#define HEATER_STATUS_FAULT 0x4

#define HEATER_REQ_ON HEATER_STATUS_ON 
#define HEATER_REQ_OFF HEATER_STATUS_OFF 


/*
PKG HEATER STATUS
0 [addressDest 1B]
1 [addressSrc 1B]
2 [pgk_type==PKG_TYPE_HEATER_STATUS 1B]
3 [heater status 1=On 0=Off 2=Fault 1B]
4 [fault code 1B]
5 [last gas reading LO 1B]
6 [last gas reading HI 1B]
7 [lowThresh LO 1B]
8 [lowThresh HI 1B]
9 [medThresh LO 1B]
A [medThresh HI 1B]
B [highThresh LO 1B]
C [highThresh HI 1B]
D [sequence 1B]
E [checksum 1B]
*/

/*
PKG ACK
0 [addressDest 1B]
1 [addressSrc 1B]
2 [pgk_type==PKG_TYPE_ACK 1B]
3 [sequence 1B]
4 [checksum 1B]
*/

/*
PKG heater
0 [addressDest 1B]
1 [addressSrc 1B]
2 [pgk_type==PKG_TYPE_HEATER 1B]
3 [heaterEnable]
4 [lowThresh LO 1B]
5 [lowThresh HI 1B]
6 [medThresh LO 1B]
7 [medThresh HI 1B]
8 [highThresh LO 1B]
9 [highThresh HI 1B]
A [sequence 1B]
B [checksum 1B]
*/

/*
PKG_TYPE_HEATER_REQUEST
0 [addressDest 1B]
1 [addressSrc 1B]
2 [pgk_type==PKG_TYPE_HEATER_REQUEST 1B]
3 [reserved]
4 [sequence 1B]
5 [checksum 1B]
*/

/*PKG intensity
0 [addressDest 1B]
<--- to add: address of sender
1 [pgk_type==PKG_TYPE_INTENSITY 1B]
2 [intensity 1B]
3 [on duration(s)= min 4b + sec*4 4b 1B]
4 [flags 4b fadeSpeed 4b]
5 [minValue 1B]
6 [sequence 1B]
7 [checksum 1B]
*/


#define PKG_MANUAL_FLAG 0x80

#define LIGHT_STATE_ON 0
#define LIGHT_STATE_OFF 1
#define LIGHT_STATE_MANUAL 2

#define MOVEMENT_ON 0x01
#define MOVEMENT_OFF 0x02

#define CHECKSUM_MOVEMENT_ON(id) (PKG_TYPE_MOVEMENT + MOVEMENT_ON + (id))
#define CHECKSUM_MOVEMENT_OFF(id) (PKG_TYPE_MOVEMENT  + MOVEMENT_OFF + (id))

#endif