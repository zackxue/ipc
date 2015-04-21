

#ifndef __MPEG2TS_H__
#define __MPEG2TS_H__

#include <stdint.h>
#include <stdbool.h>


#define TRANSPORT_PACKET_SIZE (1)

#pragma pack(1)

typedef struct
{
	//byte 0
	unsigned char sync_byte; //同步字节，固定为0x47 ，表示后面的是一个TS分组，当然，后面包中的数据是不会出现0x47的

	//byte 1
	unsigned char PID_high5 :5; //PID的头5位
	unsigned char transport_priority :1; //传输优先级位，1表示高优先级，传输机制可能用到，解码好像用不着。
	unsigned char payload_unit_start_indicator :1; //这个位功能有点复杂，字面意思是有效负载的开始标志，根据后面有效负载的内容不同功能也不同，
	unsigned char transport_error_indicator :1; //传输错误标志位，一般传输错误的话就不会处理这个包了

	//byte 2
	unsigned char PID_low8; //PID的后8位。这个比较重要，指出了这个包的有效负载数据的类型，告诉我们这个包传输的是什么内容。

	//byte 3
	unsigned char continuity_counter :4; //一个4bit的计数器，范围0-15，具有相同的PID的TS分组传输时每次加1，到15后清0。不过，有些情况下是不计数的。
	unsigned char adaptation_field_control :2; //表示TS分组首部后面是否跟随有调整字段和有效负载。01仅含有效负载，10仅含调整字段，11含有调整字段和有效负载。为00的话解码器不进行处理。空分组没有调整字段
	unsigned char transport_scrambling_control :2; //加密标志位，表示TS分组有效负载的加密模式。TS分组首部(也就是前面这32bit)是不应被加密的，00表示未加密。

	//byte 4-187
	unsigned char data[188 - 4];
} _MPEGTS_PACKET; //32bit


typedef struct TRANSPORT_PACKET
{
	uint8_t syn_byte;
	//!< 这是1B长度字段，其值为0x47，该字段是MPEG-2TS的传送包标识符。

	uint8_t PID_bit12_8 : 5;
	//!< 这个13b长度的字段，表示存储于传送包的有效净荷中数据的类型。
	//!< PID=0x0000表示净荷的数据为节目关联表；
	//!< PID=0x0001表示净荷的数据为条件访问表；
	//!< PID=0x0003~0x000F为保留；
	//!< PID=0x1FFF表示净荷的数据空包；
	//!< 其他PID值表示净荷的数据为节目映射表，网络信息，以及由用户定义打包的音频/视频数据PES包等。

	uint16_t transport_priority : 1;
		//!< 这是一个1b长度的字段。当该字段置为1，表示相关的包比其他具有相同PID但此字段为“0” 的包有更高的优先权。可以根据此字段确定在一个原始流中数据的传送优先级。
	uint16_t payload_unit_start_indicator : 1;
	//!< 这时1b长度的字段。该字段用来表示TS包的有效净荷有PES包或者PSI数据的情况。
	uint16_t transport_error_indicator : 1;
	//!< 这是一个1b长度的字段。值为1时，表示在相关的传送包中至少有一个不可纠正的错误，只有在错误纠正之后，该胃才能被重新置0。

	uint8_t PID_bit7_0; //!< Pid的低8位
	
	uint8_t continuity_counter : 4;
		//!< 这是一个4b长度的字段。随着具有相同的PID TS包的增加而增加，当它达到最大时，有恢复为“0”。如果调整字段控制值adaptation_field_control为“00”或“10“，该连续计数器不增加。
	uint8_t adaptation_field_control : 2;
	//!< 这是一个2b长度字段，表示传送流包首部是否跟随有调整字段和/或有效净荷。
	uint8_t transport_scrambling_control : 2;
	//!< 这是一个2b长度字段。该字段用来指示传送流包有效净荷的加扰方式。如果传送流包首部包含调整字段，则不应该被加扰。对于空包，transport_scrambling_control的值为“00”。
	
	uint8_t adaptation_field_or_payload[188 - 4];
	
}TRANSPORT_PACKET_t;
#pragma pack()


extern uint32_t mp2ts_pcr_base_us(uint64_t t_us);
extern uint32_t mp2ts_pcr_ext_us(uint64_t t_us);

extern uint32_t mp2ts_pcr_base_s(uint32_t t_s);
extern uint32_t mp2ts_pcr_ext_s(uint32_t t_s);


#endif //__MPEG2TS_H__

