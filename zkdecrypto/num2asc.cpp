#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *in_file, *out_file;
    char in_name[1024], out_name[1024], number[8];
    int ascii;
    
    if(argc<3)
    {
		printf("usage: num2asc in out\n");
		return 0;
	}
	
	strcpy(in_name,argv[1]);
	strcpy(out_name,argv[2]);
    
    in_file=fopen(in_name,"r");
    out_file=fopen(out_name,"w");
    
    if(!in_file || !out_file) 
	{
		printf("error opening file\n");
		return 0;
	}
    
    while(fscanf(in_file,"%s",number)!=EOF)
    {
		ascii=atoi(number);
		
		if(ascii>0 && ascii<128)
			putc(char(ascii+0x20),out_file);
	}
	
	fclose(in_file);
	fclose(out_file);
    
    return 0;
}
