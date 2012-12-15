%{
	#include "instr.h"
	#include "data.h"
	#include "interrupt.h"
	#define YY_INPUT(buf,result,max_size)		\
	{						\
		int len;				\
		strcpy(buf, instruction);	\
		len = strlen(buf);			\
		if(!strcmp(buf,"OVER\n"))		\
		{					\
 			result = YY_NULL;		\
		}					\
		else					\
		{					\
			result = len;			\
		}					\
	}
%}

%option noyywrap

%%

START		{ /*printf("START instruction found!! \n");reg[IP_REG]++;line_count++;printf("IP=%d\n",reg[IP_REG]);*/return(START);}
MOV		{ yylval.flag=0; return(MOV); }
SP 		{ yylval.flag=SP; return(0); }
BP		{ yylval.flag=BP; return(0); }
IP		{ /*printf("<ERROR:%d:> IP cannot be an arguement\n",getInteger(reg[IP_REG]));exit(0);*/
		  yylval.flag=IP; return(0); }
PID		{yylval.flag=REG; return(PID_REG);}
ADD		{ yylval.flag=ADD; return(ARITH); }
SUB		{ yylval.flag=SUB; return(ARITH); }
MUL		{ yylval.flag=MUL; return(ARITH); }
DIV		{ yylval.flag=DIV; return(ARITH); }
MOD		{ yylval.flag=MOD; return(ARITH); }
INR		{ yylval.flag=INR; return(ARITH); }
DCR		{ yylval.flag=DCR; return(ARITH); }
LT    		{ yylval.flag=LT; return(LOGIC); }
GT    		{ yylval.flag=GT; return(LOGIC); }
EQ    		{ yylval.flag=EQ; return(LOGIC); }
NE    		{ yylval.flag=NE; return(LOGIC); }
GE    		{ yylval.flag=GE; return(LOGIC); }
LE    		{ yylval.flag=LE; return(LOGIC); }
JZ 		{ yylval.flag=JZ; return(BRANCH); }
JNZ 		{ yylval.flag=JNZ; return(BRANCH); }
JMP 		{ yylval.flag=JMP; return(BRANCH); }
PUSH		{ yylval.flag=0; return(PUSH); }
POP		{ yylval.flag=0; return(POP); }
CALL		{ yylval.flag=0; return(CALL); }
RET		{ yylval.flag=0; return(RET); }
IN    		{ yylval.flag=0; return(IN); }
OUT		{ yylval.flag=0; return(OUT); }
SIN		{ yylval.flag=0; return(SIN); }
SOUT		{ yylval.flag=0; return(SOUT); }
STRCMP		{ yylval.flag=0; return(STRCMP); }
STRCPY		{ yylval.flag=0; return(STRCPY); }
LOAD		{ yylval.flag=0; return(LOAD); }
STORE		{ yylval.flag=0; return(STORE); }
HALT		{ printf("OVER!!!!!!!!\n");yylval.flag=0; return(HALT); }
INT		{ yylval.flag=0; return(INT); }
END		{ yylval.flag=0; return(END); }
BRKP		{ yylval.flag=0; return(BRKP); }
IRET 		{yylval.flag=0; return(IRET);}
R[0-9]+         { yylval.flag=REG; yytext++; return(atoi(yytext));}
S[0-9]+		{ 
			if(mode == USER_MODE){
			      printf("Error: Kernel Registers cannot be used in user mode\n");
			      exit(0);
			  }
			yylval.flag=REG; yytext++; return((atoi(yytext) + NO_USER_REG + NO_SPECIAL_REG));
		}
T[0-9]+		{ 
			if(mode == USER_MODE){
			      printf("Error: Temporary Registers cannot be used in user mode\n");
			      exit(0);
			  }
			yylval.flag=REG; yytext++; return((atoi(yytext) + NO_USER_REG + NO_SPECIAL_REG + NO_SYS_REG));
		}
\[R[0-9]+\]     {	
			yylval.flag=MEM_REG; 
			yytext[yyleng-1]='\0';
			yytext=yytext+2;
			return(atoi(yytext)); 
		}	
\[S[0-9]+\]     {	
			if(mode == USER_MODE){
			      printf("Error: Kernel Registers cannot be used in user mode\n");
			      exit(0);
			  }
			yylval.flag=MEM_REG; 
			yytext[yyleng-1]='\0';
			yytext=yytext+2;
			return((atoi(yytext) + NO_USER_REG + NO_SPECIAL_REG)); 
		}
\[T[0-9]+\]     {	
			if(mode == USER_MODE){
			      printf("Error: Temporary Registers cannot be used in user mode\n");
			      exit(0);
			  }
			yylval.flag=MEM_REG; 
			yytext[yyleng-1]='\0';
			yytext=yytext+2;
			return((atoi(yytext) + NO_USER_REG + NO_SPECIAL_REG + NO_SYS_REG)); 
		}
\[[0-9]+\]	{
			yylval.flag=MEM_DIR;
			yytext[yyleng-1]='\0';
			yytext++;
			return(atoi(yytext));
		}
\[SP\]		{ yylval.flag=MEM_SP; return(0); }
\[BP\]		{ yylval.flag=MEM_BP; return(0); }
\[IP\]		{ printf("<ERROR:%d:>IP cannot be an arguement\n",getInteger(reg[IP_REG]));exit(0);
		  yylval.flag=MEM_IP; return(0); }		//error: Is this needed.
-?[0-9]+  	{ yylval.flag=NUM; return(atoi(yytext)); }
[\t ]*		; 
OVER		{ //printf("<ERROR> HALT instruction missing\n"); 
			printf("OVER!!!!!!!!\n"); 
			yylval.flag=0; return(HALT); 
		  exit(0);
		}
\n	        ;
[A-Za-z0-9]+[\t ]*:	;
\/\/.*		;
[,:]		;
[A-Za-z0-9]+	{ strcpy(yylval.data,yytext); return(LABEL);}
.		{ printf("<ERROR:%d: Unexpected symbol %s\n",getInteger(reg[IP_REG]),yytext);
		  exit(0);
		}
%%
