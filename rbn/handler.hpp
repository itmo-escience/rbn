/*
TODO: powinien do kazdego pliku byc dodatkowo wygenerowany plik informaujacy o parametrach tej symulacji
*/

#ifndef HSYSTEM_HANDLER_HPP
#define HSYSTEM_HANDLER_HPP

#include <iostream>
#include <cstdio>
#include <fstream>
#include <ctime>
#include <sstream>
#include <string.h>
#include <vector>
//#include <map>

#include "hsystem.hpp"
#include "hparams.hpp"

typedef vector<double> doubles;
typedef vector<int> ints;
typedef vector<doubles > doubles2;
typedef vector<ints > ints2;
//typedef map<int, int> map_int;
//typedef map<int, ints> map_ints;
//typedef vector<map_ints> vec_map_ints;
typedef vector<std::string > strings;

using namespace std;

class handler {
public:
	enum OPTIONS{
		EACH_KIN, //each min Kin
		//EACH_INFO,
		HIST_KIN, //each node Kin
		HIST_INFO, //each node activity
		HIST_LAST, //
		EACH_PERIOD,//saving period lengths
		EACH_BASIN, //maybe not working
		PAJEK_STRUCTURE, //saving pajek structure
		HIST_KOUT, //each node Kout
		HIST_CC //each node's clustering coefficient
	};

	handler(){}
	handler(string filename);
	handler(ints levels_, doubles p, int repetitions, int iterations);
	handler(int nets_, ints sizes_, ints Kins_, double q0_, double alpha, int repetitions, int iterations);
	handler(int nets_, ints sizes_, ints Kins_, double q0_, doubles alpha, int repetitions, int iterations);

	//void initialize();
	void reset_vectors();
	void work();

	void set_inform_after(int i) {params.inform_after = i;}
	void set_save_hist_from(int i) {params.save_hist_from = i;}
	
	void save(OPTIONS);

	void set_delimiter (char c) {params.delimiter = c;}
	void set_save_name(string, OPTIONS);
	void add_n(string n) {params.add_on = n + params.add_on; params.pajek_add_on = n + params.pajek_add_on;}

private:
	//void prepare_network_structure(hsystem* hs, int nr);

	//work information
	hparams params;

	doubles2 d_each_Kin;
	ints2 d_hist_Kin;
	doubles2 d_hist_info;
	doubles2 d_hist_last;
	ints d_each_period;
	ints d_each_basin;
	strings d_pajek_structure;
	ints2 d_hist_Kout;
	doubles2 d_hist_cc;
};

#endif
