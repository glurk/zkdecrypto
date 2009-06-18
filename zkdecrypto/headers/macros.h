#ifndef _MACROS_H_
#define _MACROS_H_

#define ROUNDTOINT(F) (DECIMAL(F)>=.5? int(F)+1:int(F))
#define ROUNDUP(F) (DECIMAL(F)>0? int(F)+1:int(F))
#define IS_ASCII(C) (((unsigned char)C)>=0x20 && ((unsigned char)C)<=0xFE)
#define DECIMAL(N) (N-int(N))
#define ABS(X) ((X)<0? (-1*(X)):(X))
#define MAX(X,Y) (X>Y? X:Y)
#define MIN(X,Y) (X>Y? Y:X)
#define CLOSER(A,B,C) (ABS((A)-(C))<ABS((B)-(C))) //TRUE if A is closer to C than B is
#define CLOSE_TO(A,B,T) (ABS((A)-(B))<=ROUNDTOINT((B)*(T))? true:false)
#define IS_BETWEEN_A(X,Y,Z) ((X)>=(Y) && (X)<=(Z))
#define IS_BETWEEN(X,Y,Z) (Y<Z? IS_BETWEEN_A(X,Y,Z):IS_BETWEEN_A(X,Z,Y))

#define IS_LOWER_LTR(c) ((c>='a' && c<='z')? 1:0)
#define IS_UPPER_LTR(c) ((c>='A' && c<='Z')? 1:0)

#endif
