#include <stdio.h>
//#include "headers/message.h"
#include "headers/z340.h"

int main(int argc, char *argv[])
{
	Message msg;
	char cipher_file[1024], key_file[1024], graph_file[1024];
	char graph_base[64], key_param[256], language[8]="", key[256];
	SOLVEINFO si;
	int ug=USE_ALL, cipher_ok=false, key_ok=false, key_file_ok=false;
	
	//read parameters
	for(int param=1; param<argc; param++)
	{
		//cipher file
		if(!strcmp(argv[param],"-cf"))
		{
			strcpy(cipher_file,argv[param+1]);
			
			if(!msg.Read(cipher_file)) 
				{printf("Error loading %s",cipher_file); exit(0);}
				
			else cipher_ok=true;
		}
				
		//key file
		if(!strcmp(argv[param],"-kf"))
		{
			strcpy(key_file,argv[param+1]);
			
			if(!msg.cur_map.Read(key_file)) 
				{printf("Error loading %s",key_file); exit(0);}
				
			else key_ok=key_file_ok=true;
		}
		
		//extra, or all of key
		if(!strcmp(argv[param],"-k"))
		{
			strcpy(key_param,argv[param+1]);
			key_ok=true;
		}

		//extra, or all of key
		if(!strcmp(argv[param],"-l"))
		{
			strcpy(language,argv[param+1]);
		}
	}	
	
	//print usage
	if(!cipher_ok || !key_ok)
	{
		printf("Zodiac Decrypto\nUsage: -cf <cipher_file> [-kf <key_file> and/or -k <key>] [-l <language>]");
		exit(0);
	}

	//load ngraph files
	if(language[0]=='\0') strcpy(language,"eng");

	for(int n=1; n<=5; n++)
	{
		if(n==1)  strcpy(graph_base,"unigraphs.txt");
		if(n==2)  strcpy(graph_base,"bigraphs.txt");
		if(n==3)  strcpy(graph_base,"trigraphs.txt");
		if(n==4)  strcpy(graph_base,"tetragraphs.txt");
		if(n==5)  strcpy(graph_base,"pentagraphs.txt");
		
		sprintf(graph_file,"%s\\%s\\%s","language",language,graph_base);

		if(!ReadNGraphs(graph_file,n))
		{
			printf("Could not open %s",graph_file);
			exit(0);
		}
	}
	
	//setup key
	if(key_file_ok) msg.cur_map.ToKey(key,key_param);
	else strcpy(key,key_param);

	//solve info
	memset(&si,0,sizeof(SOLVEINFO));
	si.max_fail=500;
	si.revert=600;
	si.swaps=5;
	si.running=true;

	hillclimb(msg.GetCipher(),msg.GetLength(),key,msg.cur_map.GetLocked(),si,ug,true);
	
	return 0;
}
