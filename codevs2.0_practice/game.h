#include"state.h"
#pragma once

/*
ゲーム全体の情報を持っておく
パックの情報，今の状態
連鎖などの計算をここで行う
*/

class Game{
	int packs[1000][4][MAX_T][MAX_T];//パックの中身　もらったら　事前に全部回転させたのも計算しちゃう
	int packs_enemy[1000][4][MAX_T][MAX_T];
	int turn;
	State now_state;//現在のState
	State state_enemy;
	int my_stock_garbage,enemy_stock_garbage;

	void input_pack();
	bool check_inside(const State &state,const int &x,const int &r)const;
	bool check_over(const State &state,const int h)const;
	void push_pack(State &state,const int &x,const int &r)const;
	void drop_block(State &state,int change_info[][2][MAX_W+MAX_H+MAX_T],bool is_drop[MAX_W])const;//ブロックを全て下まで落とす
	int erase(State &state,int change_info[][2][MAX_W+MAX_H+MAX_T],bool is_drop[MAX_W])const;
	int erase_vertically(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const;
	int erase_horizontally(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const;
	int erase_right_down(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const;
	int erase_left_down(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const;
	long long int calc_point(int C,int turn,int E)const;
	int calc_garbage_block_point(const int &C,const int &E)const{
		return C*(int)(log((double)E)/(log((double)2)));
	}
	bool is_attacked();//攻撃されたらtrueが返る
public:
	const int H,W,N,T,S,P;
	void output(const State &state);
	void output(int x,int r);
	bool input_pack_changed();
	int update_enemy_state();
	void input_stock_garbage();
	bool is_fatal_attacked();//やばい攻撃

	State get_state()const{return now_state;}
	Game(int W,int H,int T,int S,int N,int P):N(N),H(H),W(W),T(T),S(S),P(P),turn(0){
		input_pack();
		now_state=State();
	}

	void get_pack(const int &idx,const int &rotation,int res[][MAX_T])const{
		memcpy(res,packs[idx][rotation],sizeof(packs[idx][rotation]));
	}
	pair<int,long long int> update(State &state,const int x,const int r,const int h)const;
	int get_turn()const{return turn;}
};