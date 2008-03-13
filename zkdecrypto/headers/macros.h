#ifndef _MACROS_H_
#define _MACROS_H_

#define ROUNDTOINT(F) (DECIMAL(F)>=.5? int(F)+1:int(F))
#define ROUNDUP(F) (DECIMAL(F)>0? int(F)+1:int(F))
#define IS_ASCII(C) (C>0x1F && C<0x7F)
#define DECIMAL(N) (N-int(N))
#define ABS(X) ((X)<0? (-1*(X)):(X))
#define CLOSER(A,B,C) (ABS((A)-(C))<ABS((B)-(C))) //TRUE if A is closer to C than B is
#define CLOSE_TO(A,B,T) (ABS((A)-(B))<=ROUNDTOINT((B)*(T))? true:false)
#define IS_BETWEEN(X,Y,Z) ((X)>=(Y) && (X)<=(Z))

#endif
