#pragma once

#define EMPTY_COMMAND = -1

#define IN_3A			0x03 //mm mr mv
#define IN_2A			0x02 //m r v
#define IN_1A			0x01 //no arguments

#define TP_RR			(TP_R << 8) | TP_R
#define TP_RM			(TP_R << 8) | TP_M
#define TP_RV			(TP_R << 8) | TP_V

#define TP_MR			(TP_M << 8) | TP_R
#define TP_MM			(TP_M << 8) | TP_M
#define TP_MV			(TP_M << 8) | TP_V

#define TP_M			0x00
#define TP_R			0x01
#define TP_V			0x02

#define IN_RESERVED		0x00
#define IN_MOV			(IN_RESERVED+1		<< 16 | IN_3A)
#define IN_DEC			((IN_MOV>>16)+1		<< 16 |	IN_2A)
#define IN_INC			((IN_DEC>>16)+1		<< 16 |	IN_2A)
#define IN_ADD			((IN_INC>>16)+1		<< 16 |	IN_3A)
#define IN_SUB			((IN_ADD>>16)+1		<< 16 |	IN_3A)
#define IN_MUL			((IN_SUB>>16)+1		<< 16 |	IN_3A)
#define IN_DIV			((IN_MUL>>16)+1		<< 16 |	IN_3A)
#define IN_JMP			((IN_INC>>16)+1		<< 16 |	IN_2A)
#define IN_JE			((IN_JMP>>16)+1		<< 16 |	IN_2A)
#define IN_JGE			((IN_JE>>16)+1		<< 16 |	IN_2A)
#define IN_JLE			((IN_JGE>>16)+1		<< 16 |	IN_2A)
#define IN_JNE			((IN_JLE>>16)+1		<< 16 |	IN_2A)
#define IN_CMP			((IN_JNE>>16)+1		<< 16 |	IN_3A)
#define IN_OUT			((IN_CMP>>16)+1		<< 16 |	IN_3A)
#define IN_PUSH			((IN_OUT>>16)+1		<< 16 |	IN_2A)
#define IN_POP			((IN_PUSH>>16)+1	<< 16 |	IN_2A)
#define IN_POPAD		((IN_POP>>16)+1		<< 16 |	IN_1A)
#define IN_PUSHAD		((IN_POPAD>>16)+1	<< 16 | IN_1A)
#define IN_CALL			((IN_PUSHAD>>16)+1	<< 16 | IN_2A)
#define IN_RET			((IN_CALL>>16)+1	<< 16 |	IN_1A)
#define IN_IN			((IN_RET>>16)+1		<< 16 |	IN_2A)

//registers
#define IR_RESERVED		0x00
//comparing registers
#define IR_REQ			0x01
#define IR_RNEQ			0x02
#define IR_REG			0x03
#define IR_REGEQ		0x04
#define IR_REL			0x05
#define IR_RELEQ		0x06

#define ALLOC_CMD		"alloc"

#define RESERVE_DWORD(bytes)\
	bytes.push_back(0);\
	bytes.push_back(0);\
	bytes.push_back(0);\
	bytes.push_back(0);\

#define PUSH_DWORD(bytes, str)\
	{\
	int n = atoi(str);\
	char b[4];\
	memcpy(b, &n, sizeof n);\
	bytes.push_back(b[0]);\
	bytes.push_back(b[1]);\
	bytes.push_back(b[2]);\
	bytes.push_back(b[3]);\
	}