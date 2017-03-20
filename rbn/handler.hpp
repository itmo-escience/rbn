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
#include "buffer_2d.hpp"

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

	/*struct node_info {
		int in_degree;
		int out_degree;
		int clustering_corfficient;
		double activity;
	};

	struct network_info {
		int average_in_degree;
		int attractor_length; // period
		int transient_length; // basin
	};*/

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
	/*void epoch_finished(const std::vector<node_info>& nodes, const std::vector<network_info>& nets) const;

	void save_nodes_info(const std::vector<node_info>& nodes) const;
	void save_networks_info(const std::vector<network_info>& nodes) const;
	void simulation_started();
	void simulation_finished();*/
	//void prepare_network_structure(hsystem* hs, int nr);
	
	void enable_output();
	void set_hist_size(size_t);
	void set_networks_size(size_t);
	void flush_all();
	void save_hist(hsystem* hs);
	void save_hist(hsystem* hs, int network_index);
	void save_networks_data(hsystem* hs);
	void log_status(int epoch, int k_in, clock_t elapsed_clocks) const;

	//work information
	hparams params;

	/*doubles2 d_each_Kin;
	ints2 d_hist_Kin;
	doubles2 d_hist_info;
	doubles2 d_hist_last;
	ints d_each_period;
	ints d_each_basin;
	strings d_pajek_structure;
	ints2 d_hist_Kout;
	doubles2 d_hist_cc;*/

	// Per-node metrics
	buffer_2d<int> d_hist_Kin;
	buffer_2d<double> d_hist_info;
	buffer_2d<int> d_hist_Kout;
	buffer_2d<double> d_hist_last;
	buffer_2d<double> d_hist_cc;

	// Per-network metrics
	buffer_2d<double> d_each_Kin;
	buffer_2d<int> d_each_period;
	buffer_2d<int> d_each_basin;
	strings d_pajek_structure;
};

#endif
