#pragma once
#include"game.h"
#include"util.h"
#include<list>
#include<fstream>
#include<iostream>
#include<string>
#include<time.h>
using namespace std;

class Player{
	Game game;//ゲームエンジンは常に使うので持っておく
	State best_state;//一番よい結果の状態を保存しておく
	long long best_score;//一番よいスコアを保存しておく
	int limit_sum_chain_num;
	int num_good_state;
	int limit_depth;//本当の意味での木の深さ制限　これないと　一生計算してしまう
	void insert_state(list<State> &list_state,State state);
	long long int search_tree(State state,list<State> &list_good_state,int sum_num_chain,const int depth,time_t start_time);
	ifstream halt_message;
	time_t start_time;
	int limit_height;
	int turn_updated;
	int start_turn;
	int debug;
	bool is_rensa_now;
	list<State> search_target;
	State save_point;
	void push_garbage();
	void random_action();

public:
	Player(int W,int H,int T,int S,int N,int P):num_good_state(3),limit_sum_chain_num(2),best_state(State()),best_score(0),game(Game(W,H,T,S,N,P)),limit_depth(2),start_time(clock()){
		debug=0;
		is_rensa_now=false;
		save_point=State();
		start_turn=0;
		limit_depth=2;
	};
	void output();
	State calc();
};