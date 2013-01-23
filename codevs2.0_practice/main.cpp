#include<iostream>
#include<vector>
#include<omp.h>
#include"state.h"
#include<string>
#include"game.h"
#include<fstream>
#include<cassert>
#include<time.h>
#include"Player.h"
using namespace std;

int main()
{
	int H,W,T,N,S,P;
	cin>>W>>H>>T>>S>>N;
	if(W==10) P=25;
	else if(W==15) P=30;
	else P=35;
	Player player(W,H,T,S,N,P);
	while(true){
		player.output();
	}
	return 0;
}