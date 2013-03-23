#define START		0
#define MOV			1
#define ARITH		2
#define LOGIC		3
#define BRANCH		4
#define PUSH		5
#define POP			6
#define CALL		7
#define RET			8
#define IN			9
#define OUT			10
#define HALT		11
#define REG			12
#define MEM_REG		13
#define SP			14
#define BP			15
#define NUM			16
#define ADD			17
#define SUB			18
#define MUL			19
#define DIV			20
#define MOD			21
#define LT			22
#define GT			23
#define EQ			24
#define NE			25
#define GE			26
#define LE			27
#define JZ			28
#define JNZ			29
#define JMP			30
#define LABEL		31
#define MEM_SP		32
#define MEM_BP		33
#define MEM_IP		34
#define MEM_DIR		35

#define IP			37
#define INR			38
#define DCR			39

//Added
#define STRING			40
#define ILLTOKEN		41

#define LOAD		44
#define STORE		45
#define INT			46
#define IRET		47
#define END 		48
#define BRKP		49

#define PTBR		50
#define PTLR		51
#define EFR			52
#define MEM_PTBR	53
#define MEM_PTLR	54
#define MEM_EFR		55

#define MEM_DIR_REG		56
#define MEM_DIR_SP		57
#define MEM_DIR_BP		58
#define MEM_DIR_IP		59
#define MEM_DIR_PTBR	60
#define MEM_DIR_PTLR	61
#define MEM_DIR_EFR		62
#define MEM_DIR_IN		63

#define ILLREG			64
