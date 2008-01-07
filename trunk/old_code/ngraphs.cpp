#include "ngraphs.h"
/*
int Bigraphs[BI_SIZE], Trigraphs[TRI_SIZE];
int Tetragraphs[TETRA_SIZE], Pentagraphs[PENTA_SIZE];

int ReadNGraphs(char *filename, int n) 
{
	FILE *tgfile;
	char ngraph[8];
	int *ngraphs;
	int nsize, freq, index;
	float percent;

	if(!(tgfile=fopen(filename,"r"))) return 0;

	if(n==2) {ngraphs=Bigraphs; nsize=BI_SIZE;}
	if(n==3) {ngraphs=Trigraphs; nsize=TRI_SIZE;}
	if(n==4) {ngraphs=Tetragraphs; nsize=TETRA_SIZE;}
	if(n==5) {ngraphs=Pentagraphs; nsize=PENTA_SIZE;}

	//init to zero
	memset(ngraphs,0,nsize*sizeof(long));

	//read file
	while(fscanf(tgfile,"%s : %i %f",ngraph,&freq,&percent)!=EOF) 
	{
		index=(ngraph[0]-'A')+(ngraph[1]-'A')*UNI_SIZE;
		if(n>2) index+=(ngraph[2]-'A')*BI_SIZE;
		if(n>3) index+=(ngraph[3]-'A')*TRI_SIZE;
		if(n>4) index+=(ngraph[4]-'A')*TETRA_SIZE; 

		if(index<0 || index>nsize) continue;
		ngraphs[index]=int(10*log(freq));
	}

	fclose(tgfile); 

	return 1;
}

int CalcScore(const char *plain_text, int use_graphs)
{
	int *ngraphs, nsize, text_len, index=0;
	long bi_score=0, tri_score=0, tetra_score=0, penta_score=0, *n_score;
	const char *ngraph;
			
	text_len=strlen(plain_text);

	for(int n=2; n<=5; n++)
	{
		if(text_len<n) continue;
		if(!(use_graphs & 0x01<<(n-2))) continue;
		
		if(n==2) {ngraphs=Bigraphs; nsize=BI_SIZE; n_score=&bi_score;}
		if(n==3) {ngraphs=Trigraphs; nsize=TRI_SIZE; n_score=&tri_score;}
		if(n==4) {ngraphs=Tetragraphs; nsize=TETRA_SIZE; n_score=&tetra_score;}
		if(n==5) {ngraphs=Pentagraphs; nsize=PENTA_SIZE; n_score=&penta_score;}
		
		//this is the time consuming loop
		for(int position=0; position<=(text_len-n); position++)
		{
			ngraph=plain_text+position;

			index=(ngraph[0]-'A')+(ngraph[1]-'A')*UNI_SIZE;
			if(n>2) index+=(ngraph[2]-'A')*BI_SIZE;
			if(n>3) index+=(ngraph[3]-'A')*TRI_SIZE;
			if(n>4) index+=(ngraph[4]-'A')*TETRA_SIZE; 

			if(index<0 || index>nsize) continue;
			*n_score+=ngraphs[index];
		}
	}

	return (bi_score>>5) + (tri_score>>4) + (tetra_score>>3) + (penta_score);
}

int Solve(Message &msg, SOLVEINFO &info, int &use_graphs)
{
 	char plain[MAX_MSG];
	int cur_score=0, last_score=0, start_time=0, end_time=0;
	int num_symbols, swap1=0, swap2=0;
	Map org_map, best_map;

	num_symbols=msg.cur_map.GetNumSymbols();

	//initial score
	msg.GetPlain(plain);
	cur_score=0;
	info.best_score=last_score=CalcScore(plain,use_graphs);
	org_map=msg.cur_map;
	best_map=msg.cur_map;
	info.disp_all();

	for(info.cur_try=0; info.cur_fail<info.max_fail; info.cur_try++)
	//while(info.running)
	{
		//time for last iteration
		info.last_time=float(end_time-start_time)/1000;
		
		start_time=info.time_func();
		
		//restart to best map after 600 iterations
		if(info.cur_try>info.revert) msg.cur_map=best_map;

		//display info for this iteration
		info.disp_info();

		//breed and test generations
		for(swap1=0; swap1<num_symbols && info.running; swap1++)
			for(swap2=0; swap2<num_symbols && info.running; swap2++)
			{
				//decode & score
				msg.GetPlain(plain);
				cur_score=CalcScore(plain,use_graphs);
	
				//current score better than best score
				if(cur_score>info.best_score) 
				{
					info.best_score=cur_score;
					best_map=msg.cur_map;
					info.cur_fail=0;
					info.disp_all();
				}
				
				msg.cur_map.SwapSymbols(swap1,swap2);
				
				//current score worse than last score
				last_score=cur_score;
				msg.GetPlain(plain);
				cur_score=CalcScore(plain,use_graphs);
				
				if(cur_score<last_score) msg.cur_map.SwapSymbols(swap1,swap2);
			}
		
		//after each iteration, shuffle 5 times
		for(int shuffle=0; shuffle<info.swaps; shuffle++)
		{
			do
			{
				swap1=rand()%num_symbols;
				swap2=rand()%num_symbols;
			}
			while(swap1==swap2);
			
			msg.cur_map.SwapSymbols(swap1,swap2);
		}

		info.cur_fail++;
		end_time=info.time_func();
		
		if(!info.running) break;
	}

	msg.cur_map=best_map;
	
	return info.best_score;
}
*/
