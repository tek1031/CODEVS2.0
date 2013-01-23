#include"game.h"
#include<string>
#include<iostream>
#include<cassert>
using namespace std;


bool Game::is_fatal_attacked(){//やばい攻撃
	int num=0;
	for(int y=T-1;y>=0;y--){
		for(int x=0;x<T;x++){
			if(packs[turn][0][y][x]>S){
				num++;
			}
		}
	}
	if(num>=10){
		return true;
	}
	return false;
}


void Game::input_stock_garbage(){
	cin>>my_stock_garbage>>enemy_stock_garbage;
	return;
}

int Game::update_enemy_state(){
	int x,r;
	cin>>x>>r;
	return update(state_enemy,x,r,H).second;
}

bool Game::input_pack_changed(){
	bool is_changed=false;
	for(int y=T-1;y>=0;y--){
		for(int x=0;x<T;x++){
			int num;
			cin>>num;
			if(packs[turn][0][y][x]!=num){
				packs[turn][0][y][x]=num;
				is_changed=true;
			}
		}
	}
	string temp;
	cin>>temp;//END読み飛ばし

	for(int y=T-1;y>=0;y--){
		for(int x=0;x<T;x++){
			int num;
			cin>>num;
			packs_enemy[turn][0][y][x]=num;
		}
	}
	cin>>temp;//END読み飛ばし

	for(int r=1;r<=3;r++){//回転するよ
		for(int y=0;y<T;y++){
			for(int x=0;x<T;x++){
				int nx,ny;
				if(r==1){
					nx=y;
					ny=(T-1)-x;
				}
				else if(r==2){
					nx=(T-1)-x;
					ny=(T-1)-y;
				}
				else{
					nx=(T-1)-y;
					ny=x;

				}
				packs[turn][r][ny][nx]=packs[turn][0][y][x];
				packs_enemy[turn][r][ny][nx]=packs_enemy[turn][0][y][x];
			}
		}
	}
	return is_changed;
}


//stateをもらって，落とす座標と回転を指定されたら，状態を更新して，得られた得点を返す
pair<int,long long int> Game::update(State &state,const int x,const int r,const int h)const{
	pair<int,long long > res(0,0);
	if(!check_inside(state,x,r)){//はみ出した
		return pair<int,long long >(-1,-1);
	}
	push_pack(state,x,r);
	state.order[state.step_idx]=pair<int,int>(x,r);
	state.step_idx++;//ここでstateの次の状態を更新する
	int change_info[4][2][MAX_W+MAX_H+MAX_T];//前と状態が変化した座標の最大値と最小値をメモっておく（変化しなければ-1）
	bool is_drop[MAX_W];
	memset(is_drop,false,sizeof(is_drop));
	for(int drop_x=max(0,x);drop_x<min(W,x+T);drop_x++){
		is_drop[drop_x]=true;
	}

	int chain=0;
	int total=0;
	while(true){
		drop_block(state,change_info,is_drop);
		int num_erased_block=erase(state,change_info,is_drop);
		if(num_erased_block==0) break;
		chain++;
		total+=calc_garbage_block_point(chain,num_erased_block);
//		res.first=chain;
	}
	res.first=chain;
	res.second=total;
	if(!check_over(state,h)){
		return pair<int,long long>(-1,-1);
	}
	return res;
}


void Game::output(const State &state){
	cout<<state.order[turn].first<<" "<<state.order[turn].second<<endl;
	update(now_state,state.order[turn].first,state.order[turn].second,H);
	turn++;
}

void Game::output(int x,int r){
	cout<<x<<" "<<r<<endl;
	update(now_state,x,r,H);
	turn++;
}

bool Game::check_inside(const State &state,const int &left_x,const int &r)const{
	for(int k=0;k<T;k++){
		int x=left_x+k;
		if(x<0 || x>=W){
			for(int y=0;y<T;y++){
				if(packs[state.step_idx][r][y][k]>=1) return false;
			}
		}
	}
	return true;
}

bool Game::check_over(const State &state,const int h)const{
	for(int x=0;x<W;x++){
		if(state.field[min(h,H)][x]!=0) return false;
	}
	return true;
}

//予選で使ったやつ　もう使っていない
inline long long int Game::calc_point(int C,int turn,int E)const{
	int n=turn/100;
	return ((long long)1<<(min(C,P+n)-1))*max(1,(C-(P+n)+1))*E;
}

//パックをそっと上の方に配置する
void Game::push_pack(State &state,const int &left_x,const int &r)const{
	for(int x_pack=0;x_pack<T;x_pack++){
		int x=left_x+x_pack;
		if(x<0 || x>=(int)W) continue;
		for(int y=0;y<T;y++){
			state.field[H+1+y][x]=packs[state.step_idx][r][y][x_pack];//フィールドの上空に配置
		}
	}
}

void Game::drop_block(State &state,int change_info[][2][MAX_W+MAX_H+MAX_T],bool is_drop[MAX_W])const{//ブロックを全て下まで落とす
	for(int i=0;i<4;i++)//初期化
		memset(change_info[i],-1,sizeof(change_info[i]));

	for(int x=0;x<W;x++){
		if(!is_drop[x]){//ここは落とさなくても良い
			continue;
		}
		bool is_continue=true;
		int y=0;
		int y_first_block=0;
		while(is_continue){
			is_continue=false;
			int y_first_empty=H+1+MAX_T;
			//空のブロックを見る
			for(;y<H+MAX_T+1;y++){
				if(state.field[y][x]==0){//空だぞ！！
					y_first_empty=y;
					break;
				}
			}
			if(y_first_empty==H+1+MAX_T) break;//空がなかった
			y_first_block=max(y,y_first_block+1);
			//空より上にある最初のブロックを見つける
			for(y_first_block=y_first_empty+1;y_first_block<H+1+MAX_T;y_first_block++){
				if(state.field[y_first_block][x]>0){
					is_continue=true;//見つかった
					break;
				}
			}
			if(!is_continue){
				break;
			}
			swap(state.field[y_first_empty][x],state.field[y_first_block][x]);//交換する
			y=y_first_empty+1;

			//こっから高速化のためのZONE　めんどいよ！覚悟してね　やってることは空のところに落ちてきたところ場所をメモってる

			if(change_info[0][0][x]<0){//縦の最小値について
				change_info[0][0][x]=y_first_empty;
			}else{
				change_info[0][0][x]=min((int)y_first_empty,change_info[0][0][x]);
			}
			if(change_info[0][1][x]<0){//縦の最大値について
				change_info[0][1][x]=y_first_empty;
			}else{
				change_info[0][1][x]=max((int)y_first_empty,change_info[0][1][x]);
			}

			//縦終了　次は横
			if(change_info[1][0][y_first_empty]<0){//横の最小値について
				change_info[1][0][y_first_empty]=x;
			}else{
				change_info[1][0][y_first_empty]=min((int)x,change_info[1][0][y_first_empty]);
			}

			if(change_info[1][1][y_first_empty]<0){//横の最大値について
				change_info[1][1][y_first_empty]=x;
			}else{
				change_info[1][1][y_first_empty]=max((int)x,change_info[1][1][y_first_empty]);
			}

			//次は右下方向について
			int line_idx=y_first_empty+x;//斜めのラインの番号　文章だと説明きついので図に書いておくから見ておいて
			int pos_idx=x;//ライン上の座標を表す
			if(y_first_empty+x>H+T-1){//ラインが上の辺から出てる場合
				pos_idx=(H+T-1)-y_first_empty;
			}
			assert(pos_idx>=0);

			if(change_info[2][0][line_idx]<0){//横の最小値について
				change_info[2][0][line_idx]=pos_idx;
			}else{
				change_info[2][0][line_idx]=min(pos_idx,change_info[2][0][line_idx]);
			}

			if(change_info[2][1][line_idx]<0){//横の最大値について
				change_info[2][1][line_idx]=pos_idx;
			}else{
				change_info[2][1][line_idx]=max(pos_idx,change_info[2][1][line_idx]);
			}

			//最後は左下方向について
			line_idx=y_first_empty+((W-1)-x);//斜めのラインの番号　文章だと説明きついので図に書いておくから見ておいて
			pos_idx=(W-1)-x;//ライン上の座標を表す
			if(y_first_empty+(W-1)-x>H+T-1){//ラインが上の辺から出てる場合
				pos_idx=(H+T-1)-y_first_empty;
			}
			assert(pos_idx>=0);

			if(change_info[3][0][line_idx]<0){//横の最小値について
				change_info[3][0][line_idx]=pos_idx;
			}else{
				change_info[3][0][line_idx]=min(pos_idx,change_info[3][0][line_idx]);
			}

			if(change_info[3][1][line_idx]<0){//横の最大値について
				change_info[3][1][line_idx]=pos_idx;
			}else{
				change_info[3][1][line_idx]=max(pos_idx,change_info[3][1][line_idx]);
			}
		}
	}
}

int Game::erase(State &state,int change_info[][2][MAX_W+MAX_H+MAX_T],bool is_drop[MAX_W])const{
	bool is_erased[MAX_H][MAX_W];//消すところを覚えておく
	memset(is_erased,false,sizeof(is_erased));
	memset(is_drop,false,sizeof(is_drop));

	//[縦，横，右下，左下][min or max][座標] 左上
	unsigned res=0;
	int change_pos[2][MAX_H];
	memset(change_pos,-1,sizeof(change_pos));
	res+=erase_vertically(state,is_erased,change_info[0],change_pos);
	res+=erase_horizontally(state,is_erased,change_info[1],change_pos);
	res+=erase_right_down(state,is_erased,change_info[2],change_pos);
	res+=erase_left_down(state,is_erased,change_info[3],change_pos);

	for(int y=0;y<H+MAX_T;y++){
		if(change_pos[0]<0) continue;
		for(int x=max(change_pos[0][y],0);x<=change_pos[1][y];x++){
			if(is_erased[y][x]){
				if(state.field[y][x]==S){//Sのブロックが落ちてきた場合　消しすぎる
					res-=3;
				}
				state.field[y][x]=0;
				is_drop[x]=true;
			}
		}
	}
	return res;
}

int Game::erase_vertically(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const{
	int res=0;//消えるブロックの個数を数える
	for(int x=0;x<W;x++){//全てのx座標に対して　縦をチェックする
		int front,back=change_info[0][x];
		if(back<0) continue;//そこの列に変化はなかった
		int sum=state.field[back][x];//着目してるとこの合計値
		front=back;
		while(sum<S){
			if(front==0) break; //これ以上戻れないよ
			front--;
			sum+=state.field[front][x];
		}

		while(front<H+MAX_T && back<H+MAX_T && front<change_info[1][x]){//front,backが範囲内にある限り処理をする
			if(state.field[back][x]<=0 || state.field[back][x]>S){//ブロックがないorお邪魔ブロックだと飛ばす
				sum=0;//合計値はリセット
				front=H+MAX_T;
				for(int y=back+1;y<H+MAX_T;y++){
					if(state.field[y][x]>0 && state.field[y][x]<=S){//ブロックがあったらfrontとback,sumをもう一回設定
						front=y;
						back=y;
						sum=state.field[y][x];
						break;
					}
				}
			}else{//ブロックがある場合
				if(sum>S){//目標値超えてるよ!
					sum-=state.field[front][x];//戦闘を一個繰り上げて，sumをその分引く
					front++;
				}
				else if(sum==S){//目標値ジャスト！　消えるね
					for(int y=front;y<=back;y++){
						is_erased[y][x]=true;//消えるというメモをつけておく

						if(change_pos[0][y]<0){
							change_pos[0][y]=x;
						}
						else{
							change_pos[0][y]=min(change_pos[0][y],x);
						}
						if(change_pos[1][y]<0){
							change_pos[1][y]=x;
						}
						else{
							change_pos[1][y]=max(change_pos[1][y],x);
						}

					}
					res+=back-front+1;//消えるブロックの個数をメモ
					sum-=state.field[front][x];//frontを一個進めて，sumを減らす
					front++;
				}
				else{//目標値以下
					back++;//もう一個後ろを見る
					if(back<H+MAX_T) sum+=state.field[back][x];//sumに一個後ろの分も追加
				}
			}
		}
	}
	return res;
}

int Game::erase_horizontally(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const{
	int res=0;//消えるブロックの個数を数える

	for(int y=0;y<H+MAX_T;y++){//全てのy座標に対して　横をチェックする		
		int front,back=change_info[0][y];
		if(back<0) continue;//そこの列に変化はなかった
		else{
			;
		}
		int sum=state.field[y][back];//着目してるとこの合計値
		front=back;
		while(sum<S){
			if(front==0) break; //これ以上戻れないよ
			if(state.field[y][front-1]<=0 || state.field[y][front-1]>=S){//これ以上下がると空白 or 邪魔ブロック
				break;
			}
			front--;
			sum+=state.field[y][front];
		}

		while(front<W && back<W && front<=change_info[1][y]){//front,backが範囲内にある限り処理をする
			if(state.field[y][back]<=0 || state.field[y][back]>S){//ブロックがないorお邪魔ブロックだと飛ばす
				sum=0;//合計値はリセット
				front=W;
				for(int x=back+1;x<W;x++){
					if(state.field[y][x]>0 && state.field[y][x]<=S){//ブロックがあったらfrontとback,sumをもう一回設定
						front=x;
						back=x;
						sum=state.field[y][x];
						break;
					}
				}
			}else{//ブロックがある場合
				if(sum>S){//目標値超えてるよ!
					sum-=state.field[y][front];//戦闘を一個繰り上げて，sumをその分引く
					front++;
				}
				else if(sum==S){//目標値ジャスト！　消えるね
					for(int x=front;x<=back;x++){
						is_erased[y][x]=true;//消えるというメモをつけておく
						if(change_pos[0][y]<0){
							change_pos[0][y]=x;
						}
						else{
							change_pos[0][y]=min(change_pos[0][y],x);
						}
						if(change_pos[1][y]<0){
							change_pos[1][y]=x;
						}
						else{
							change_pos[1][y]=max(change_pos[1][y],x);
						}

					}
					res+=back-front+1;//消えるブロックの個数をメモ
					sum-=state.field[y][front];//frontを一個進めて，sumを減らす
					front++;

				}
				else{//目標値以下
					back++;//もう一個後ろを見る
					if(back<W) sum+=state.field[y][back];//sumに一個後ろの分も追加
				}
			}
		}
	}
	return res;
}

int Game::erase_right_down(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const{
	int res=0;
	const int dx=1,dy=-1;//進む方向　右下に
	//line_idxについてはどこかに図を書いておくので　見ておいて

	for(int line_idx=0;line_idx<H+W+T-2;line_idx++){
		int start_x,start_y;
		if(line_idx<=H+T-1){//左側の辺から伸びるライン
			start_x=0;
			start_y=line_idx;
		}
		else{//上の辺から伸びるライン
			start_x=line_idx-(H+T-1);
			start_y=H+T-1;
		}

		int front,back=change_info[0][line_idx];
		if(back<0) continue;//そこの列に変化はなかった
		int front_x,front_y,back_x,back_y;
		back_x=start_x+back*dx;
		back_y=start_y+back*dy;
		int sum=state.field[back_y][back_x];//着目してるとこの合計値
		front=back;
		front_x=back_x;
		front_y=back_y;
		while(sum<S){
			if(front_x == 0 || front_y>=H+T-1) break; //これ以上左上に行けない
			if(state.field[front_y-dy][front_x-dx]<=0 || state.field[front_y-dy][front_x-dx]>=S){//これ以上下がると空白 or 邪魔ブロック
				break;
			}
			front--;
			front_x-=dx;
			front_y-=dy;
			sum+=state.field[front_y][front_x];
		}

		while( (front_x<W && front_y>=0) && (back_x<W && back_y>=0) && front<=change_info[1][line_idx] ){//front,backが範囲内にある限り処理をする
			if(state.field[back_y][back_x]<=0 || state.field[back_y][back_x]>S){//ブロックがないorお邪魔ブロックだと飛ばす
				sum=0;//合計値はリセット
				front_x=W;//番兵的な
				for(int k=back+1; start_x+dx*k<W && start_y+dy*k>=0; k++){//左上から見てはみ出す直前までブロックがあるところ見ていく
					int now_x=start_x+dx*k,now_y=start_y+dy*k;
					if(state.field[now_y][now_x]>0 && state.field[now_y][now_x]<=S){
						front=k;
						back=k;
						front_x=start_x+dx*front;
						front_y=start_y+dy*front;
						back_x=front_x;
						back_y=front_y;
						sum=state.field[front_y][front_x];
						break;
					}
				}
				if(front_x==W) break;
			}else{//ブロックがある場合
				if(sum>S){//目標値超えてるよ!
					sum-=state.field[front_y][front_x];//戦闘を一個繰り上げて，sumをその分引く
					front++;
					front_x+=dx;
					front_y+=dy;
				}
				else if(sum==S){//目標値ジャスト！　消えるね
					for(int k=front;k<=back;k++){
						int now_x=start_x+dx*k,now_y=start_y+dy*k;
						is_erased[now_y][now_x]=true;//消えるというメモをつけておく

						if(change_pos[0]<0){//消える場所覚える
							change_pos[0][now_y]=now_x;
						}
						else{
							change_pos[0][now_y]=min(change_pos[0][now_y],now_x);
						}
						if(change_pos[1]<0){
							change_pos[1][now_y]=now_x;
						}
						else{
							change_pos[1][now_y]=max(change_pos[1][now_y],now_x);
						}
					}
					res+=back-front+1;//消えるブロックの個数をメモ
					sum-=state.field[front_y][front_x];//frontを一個進めて，sumを減らす
					front++;
					front_x+=dx;
					front_y+=dy;
				}
				else{//目標値以下
					back++;//もう一個後ろを見る
					back_x+=dx;
					back_y+=dy;
					if(back_x<W && back_y>=0) sum+=state.field[back_y][back_x];//sumに一個後ろの分も追加
				}
			}
		}
	}
	return res;
}

int Game::erase_left_down(State &state,bool is_erased[MAX_H][MAX_W],int change_info[2][MAX_W+MAX_H+MAX_T],int change_pos[2][MAX_H])const{
	int res=0;
	const int dx=-1,dy=-1;//進む方向　左下に
	//line_idxについてはどこかに図を書いておくので　見ておいて

	/*	for(int i=0;i<H+W-2;i++){
	cout<<i<<": "<<change_info[0][i]<<endl;
	}*/

	for(int line_idx=0;line_idx<H+W+T-2;line_idx++){
		int start_x,start_y;
		if(line_idx<=H+T-1){//右側の辺から伸びるライン
			start_x=W-1;
			start_y=line_idx;
		}
		else{//上の辺から伸びるライン
			start_x=(W-1)-(line_idx-(H+T-1));//右上から左にいく
			start_y=H+T-1;
		}

		int front,back=change_info[0][line_idx];
		if(back<0) continue;//そこの列に変化はなかった
		int front_x,front_y,back_x,back_y;
		back_x=start_x+back*dx;
		back_y=start_y+back*dy;
		int sum=state.field[back_y][back_x];//着目してるとこの合計値
		front=back;
		front_x=back_x;
		front_y=back_y;
		while(sum<S){
			if(front_x == W-1 || front_y>=H+T-1) break; //これ以上右上に行けない
			if(state.field[front_y-dy][front_x-dx]<=0 || state.field[front_y-dy][front_x-dx]>=S){//これ以上下がると空白 or 邪魔ブロック
				break;
			}
			front--;
			front_x-=dx;
			front_y-=dy;
			sum+=state.field[front_y][front_x];
		}


		while( (front_x>=0 && front_y>=0) && (back_x>=0 && back_y>=0) && front<=change_info[1][line_idx] ){//front,backが範囲内にある限り処理をする
			if(state.field[back_y][back_x]<=0 || state.field[back_y][back_x]>S){//ブロックがないorお邪魔ブロックだと飛ばす
				sum=0;//合計値はリセット
				front_x=W;//番兵的な
				for(int k=back+1; start_x+dx*k>=0 && (int)(start_y+dy*k)>=0; k++){//左上から見てはみ出す直前までブロックがあるところ見ていく
					int now_x=start_x+dx*k,now_y=start_y+dy*k;
					if(state.field[now_y][now_x]>0 && state.field[now_y][now_x]<=S){
						front=k;
						back=k;
						front_x=start_x+dx*front;
						front_y=start_y+dy*front;
						back_x=front_x;
						back_y=front_y;
						sum=state.field[front_y][front_x];
						break;
					}
				}
				if(front_x==W) break;
			}else{//ブロックがある場合
				if(sum>S){//目標値超えてるよ!
					sum-=state.field[front_y][front_x];//戦闘を一個繰り上げて，sumをその分引く
					front++;
					front_x+=dx;
					front_y+=dy;
				}
				else if(sum==S){//目標値ジャスト！　消えるね
					for(int k=front;k<=back;k++){
						int now_x=start_x+dx*k,now_y=start_y+dy*k;
						is_erased[now_y][now_x]=true;//消えるというメモをつけておく
						if(change_pos[0]<0){//消える場所覚える
							change_pos[0][now_y]=now_x;
						}
						else{
							change_pos[0][now_y]=min(change_pos[0][now_y],now_x);
						}
						if(change_pos[1]<0){
							change_pos[1][now_y]=now_x;
						}
						else{
							change_pos[1][now_y]=max(change_pos[1][now_y],now_x);
						}

					}
					res+=back-front+1;//消えるブロックの個数をメモ
					sum-=state.field[front_y][front_x];//frontを一個進めて，sumを減らす
					front++;
					front_x+=dx;
					front_y+=dy;
				}
				else{//目標値以下
					back++;//もう一個後ろを見る
					back_x+=dx;
					back_y+=dy;
					if(back_x>=0 && back_y>=0) sum+=state.field[back_y][back_x];//sumに一個後ろの分も追加
				}
			}
		}

	}
	return res;
}

void Game::input_pack(){
	for(int i=0;i<N;i++){
		for(int y=T-1;y>=0;y--){
			for(int x=0;x<T;x++){
				int num;
				cin>>num;
				packs[i][0][y][x]=num;
				packs_enemy[i][0][y][x]=num;
			}
		}
		string temp;
		cin>>temp;//END読み飛ばし

		for(int r=1;r<=3;r++){//回転するよ
			for(int y=0;y<T;y++){
				for(int x=0;x<T;x++){
					int nx,ny;
					if(r==1){
						nx=y;
						ny=(T-1)-x;
					}
					else if(r==2){
						nx=(T-1)-x;
						ny=(T-1)-y;
					}
					else{
						nx=(T-1)-y;
						ny=x;

					}
					packs[i][r][ny][nx]=packs[i][0][y][x];
					packs_enemy[i][r][ny][nx]=packs_enemy[i][0][y][x];
				}
			}
		}
	}
}