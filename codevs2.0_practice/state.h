#include<utility>
#include<iostream>
#include"util.h"
using namespace std;
#pragma once

class State{
	friend class Game;
	long long int value;//今まで得られることが確定した点数
	int depth;//次に探索する深さ　これはチェインの総数
	pair<int,int> order[1000];//今までおこなってきた命令列 (x,rotation)
	int step_idx;//今何個目のパックを落とすのか　0origin
public:
	int field[MAX_H][MAX_W];//フィールドを表す　下が0となるように変更
	State():step_idx(0),value(0),depth(2){//ホントに最初の状態を作るとき
		memset(field,0,sizeof(field));
	}
	//状態を全て指定して初期化する
	long long int get_value()const{return value;}
	void set_value(const long long int max_value){
		value=max_value;
	}
	int get_step_idx()const{return step_idx;}
	int get_depth()const{return depth;}
	void increment_depth(){depth++;}
	void reset_depth(){depth=2;}
	const pair<int,int>* get_order()const{return order;}
	int get_height(){
		int res=0;
		for(int x=0;x<20;x++){
			for(int y=res;y<36;y++){
				if(field[y][x]==0){
					res=y;
					break;
				}
			}
		}
		return res;
	}
};
