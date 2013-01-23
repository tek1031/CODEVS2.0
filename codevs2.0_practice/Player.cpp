#include"Player.h"
#include<string>
#include<time.h>
#include<omp.h>
#include<vector>
#include<cassert>

//ランダムにゲームオーバーにならない行動を行う
void Player::random_action(){
	for(int x=-game.T;x<game.W+game.T;x++){
		for(int r=0;r<4;r++){
			State state=game.get_state();
			if(game.update(state,x,r,game.H).second>=0){
				game.output(x,r);
				search_target.clear();
				return;
			}
		}
	}
	search_target.clear();
	cout<<-100<<" "<<-100<<endl;//GAME OVER
}

//右にゴミを寄せる行動を行う
void Player::push_garbage(){
	for(int x=game.W+game.T;x>=-game.T;x--){
		for(int r=0;r<4;r++){
			State state= game.get_state();
			int got_point=game.update(state,x,r,game.H).second;
			if(got_point>=0){
				game.output(x,r);
				search_target.clear();
				return;
			}
		}
	}
	cout<<-100<<" "<<-100<<endl;//GAME OVER
}

//出力を吐き出す
void Player::output(){
	bool is_attacked=false;
	if(game.get_turn()>0){
		time_t bef=clock();
		is_attacked=game.input_pack_changed();
		game.update_enemy_state();
		game.input_stock_garbage();
		start_time+=clock()-bef;
	}

	//残り時間を見て，保管しておく状態の個数を調整する
	if((start_time-clock())/CLOCKS_PER_SEC>=7000 && (start_time-clock())/CLOCKS_PER_SEC>game.get_turn()*10){ //間に合いそうにないです
		num_good_state=1;
	}
	else if(num_good_state==1){//間に合いそう
		num_good_state=3;
	}
	bool is_fatal_attacked=false;

	//攻撃を受けた！
	if(is_attacked){
		best_score=0;
		is_fatal_attacked=game.is_fatal_attacked();
		//ブロックが10個以上落ちてくる　激しい攻撃だった？
		if(is_fatal_attacked){
			save_point=State();//セーブポイントを破棄する
			push_garbage();//右にブロックを寄せる
			return;
		}
	}

	if((!is_rensa_now && save_point.get_step_idx()<=game.get_turn()) || game.get_turn()==0 || is_attacked){//連鎖中でない or セーブポイントがすでに前のターン
		is_rensa_now=false;
		int before_best_score=best_score;
		best_score=0;//リセット
		save_point=calc();//もう1回まっさらな状態から探している　一番よい行動をセーブポイントとして保存
		search_target.clear();
		if(before_best_score>=best_score){//探索しても連鎖の点数が上がらなかったら　連鎖開始フラグON
			is_rensa_now=true;
		}
		else if(best_score<=0){//解なし　死亡？　一応ランダムな行動をしてみる　
			random_action();
			return;
		}
	}

	/*
	セーブポイントは連鎖の途中の行動列が，
	best_stateは連鎖を完了させる行動列が保存されている
	*/

	//search END
	//保存されている行動を行う
	if(is_rensa_now){//連鎖発動　best_state行動を行う
		save_point=State();
		best_score=0;
		game.output(best_state);//一番よさそうな奴をoutput
		if(game.get_turn()>=best_state.get_step_idx()){
			is_rensa_now=false;//連鎖完了！
			start_turn=game.get_turn();
		}
	}else{//途中です とりあえずセーブポイントの行動を行う
		game.output(save_point);
	}
}


//前のプログラムをごりごり改悪した凶悪コード
State Player::calc(){
	limit_height=game.H;
	int iteration_count=0;
	turn_updated=0;
	best_score=0;
	State res=State();
	while(game.get_turn()<game.N){
		State now_state;
		if(!search_target.empty()){
			now_state=*search_target.begin();
		}else{
			now_state=game.get_state();
		}
		search_tree(now_state,search_target,0,0,clock());//最初に浅く探索をする
		while(search_target.size()>0){
			time_t time_iteration_start=clock();
			iteration_count++;
			list<State> next_target;//次に探索する状態を入れる
			for(list<State>::iterator target=search_target.begin();target!=search_target.end();target++){
				list<State> list_good_state;
				limit_sum_chain_num=target->get_depth();
				limit_depth=target->get_depth();
				long long value=search_tree(*target,list_good_state,0,0,clock());//状態targetから探索を開始
				if(!list_good_state.empty()){//答えが見つかったね
					int value_got=list_good_state.begin()->get_value();
					if(res.get_value()<value_got){
						res=*list_good_state.begin();
					}
				}else{//答えがないだと・・・？　深さを増やしてみるか　反復進化法
					if(target->get_depth()<=2){//3くらいまではやってみるか
						target->increment_depth();
						insert_state(next_target,*target);
					}
				}
				int count=0;//極所解を回避したいので　同じ状態から派生したものを何個も入れない　2個くらいにしておく
				for(list<State>::iterator next_state=list_good_state.begin();count<2 && next_state!=list_good_state.end();next_state++){
					if(next_state->get_value()==target->get_value()){
						target->increment_depth();
						if(target->get_depth()<=3){//3くらいまではやってみるか
							insert_state(next_target,*target);
							target->reset_depth();
						}
					}
					else{
						next_state->reset_depth();
						insert_state(next_target,*next_state);
					}
					count++;
				}
			}
			if(next_target.empty() || next_target.begin()->get_step_idx()>game.get_turn()+5){//5ターン以上先の行動は邪魔が入る確率が高いので探索終わり！
				return res;//この行動がなかなかいいっぽい？
			}
			search_target=next_target;
		}
		return State();//見つからなかった場合だけ
	}
	return State();
}

//状態が与えられるので，よさそうならこれをリストに入れる，そうでないなら何もしない　挿入ソートしてる
void Player::insert_state(list<State> &list_state,State state){
	if(state.get_value()<=0) return;

	if(list_state.size()==0){//リストになにも入っていなければ無条件に入れる
		list_state.push_back(state);
		return;
	}

	if(state.get_value()<list_state.back().get_value()){//リストの一番後ろよりも小さい・・・
		if(list_state.size()<num_good_state){//まだ個数が制限未満　後ろにいれる
			list_state.push_back(state);
		}
		else{//制限以上　つまりいらない子であったことが判明
			return;//何もせずさようなら
		}
	}

	//必要な子であることはわかったので，後ろからどこに入れるべきかを調べ，挿入（挿入ソート）

	for (	list<State>::iterator it=list_state.begin(); it!=list_state.end();it++){
		if(it->get_value()==state.get_value()){//点数が同じだったら同じかどうかの判定を行う
			if(it->get_value()==state.get_value()){//同じ状態っぽいのでスルー
				break;
			}

		}
		else if(it->get_value()<state.get_value()){//価値が自分未満のものを発見したら挿入
			list_state.insert(it,state);//そいつの直前に挿入するとソートされたまま
			break;
		}
	}
	if(list_state.size()>num_good_state)//制限以上の個数がリストに入っている
		list_state.pop_back();//一番後ろはもうさようなら
	return;
}


/*ゲーム木をもぐる　けっこう複雑
sum_num_chainは今まで起こったチェインの回数をカウントする
戻り値は次の一手で得られる点数の最大値　これが大きいほど，よい状態といえる
*/
long long int Player::search_tree(State state,list<State> &list_good_state,int sum_num_chain,int depth,const time_t time){
	if((double)(clock()-time)/CLOCKS_PER_SEC>10.0) return 0;//時間かけすぎ乙
	if(state.get_step_idx()+game.get_turn()==game.N) return 1;//ゲームが終了したので終わる 停止性が心配なので1を返そう	
	if(depth>=3) return 0;//深さ4以上は時間かかりすぎるので諦める

	if(limit_depth<depth) return 0;
	long long int res=0;//この1回で得られた点数の最大点数
	long long int max_value=0;//down_treeの戻り値の最大値（これが大きければ，このstateは優秀である）

	//全てのx座標,方向について試す もしかしたら回らないかもしれない
	for(int x=-game.T+1;x<game.W-5;x++){
		for(int r=0;r<4;r++){
			State copy_state=state;
			//状態を更新してみる
			pair<int,long long>got=game.update(copy_state,x,r,limit_height);
			long long int got_point=got.second;
			int got_chain=got.first;
			if(got_point<0){//GAMEOVERになったのでもどる
				continue;
			}
			//チェインの合計数やらスコアやらを更新
			sum_num_chain+=got_chain;
			res=max(res,got_point);

			if(got_point/(state.get_step_idx()-start_turn+1)>best_score){//得られた点数が現在の最高点を超えたら
				best_score=got_point/(state.get_step_idx()-start_turn+1);//最高点を更新
				best_state=copy_state;//一番よい状態を更新
				turn_updated=state.get_step_idx();
			}
			//枝刈りの条件に当てはまらなければもぐる
			if(sum_num_chain<limit_sum_chain_num || depth<=2){//チェインが一定数をきったら次ももぐる　大連鎖が起こる可能性が高い
				max_value=max(max_value,search_tree(copy_state,list_good_state,sum_num_chain,depth+1,time));
			}
			sum_num_chain-=got_chain;
		}
	}
	state.set_value(max_value);//このstateはこの点数を得ることが保証されている
	if(depth>=1){
		insert_state(list_good_state,state);//さて深く探索するに値するstateなの？
	}
	return res;
}

