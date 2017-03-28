#include "handler.hpp"

#include "execution_policy.hpp"

//#include <boost/filesystem.hpp>

handler::handler(string filename) {
	params = hparams(filename);

	//initialize();
}

handler::handler(ints levels_, doubles p, int repetitions_, int iterations_){
	double ini_p = p[0];
	doubles p_ch;

	p_ch.push_back(0.0);
	for(unsigned int i = 1; i < p.size(); ++i){
		p_ch.push_back(p[i] - p[i-1]);
	}

	params = hparams(levels_, p_ch);
	params.ini_p = ini_p;

	params.repetitions = repetitions_;
	params.iterations = iterations_;

	//initialize();
}

handler::handler(int nets_, ints sizes_, ints Kins_, double q0_, double alpha = 0, int repetitions_ = 5, int iterations_ = 1500) {
	params = hparams(nets_, sizes_);
	params.prob = q0_;
	params.tot_incon = q0_;
	params.set_alpha(alpha);
	params.repetitions = repetitions_;
	params.iterations = iterations_;
	params.Kins = Kins_;
}

handler::handler(int nets_, ints sizes_, ints Kins_, double q0_, doubles alpha, int repetitions_ = 5, int iterations_ = 1500) {
	params = hparams(nets_, sizes_);
	params.prob = q0_;
	params.tot_incon = q0_;
	params.set_alpha(alpha);
	params.Kins = Kins_;
	params.repetitions = repetitions_;
	params.iterations = iterations_;
}

#include <fstream>
#include <sstream>
#include <ctime>
#include <string>

#ifdef FOUT
#define OUTSTREAM fout
#else
#define OUTSTREAM cout
#endif

void handler::work() {
	clock_t t_all = clock(), t = clock();
	enable_output();
	
	for(int j = 0; j < params.repetitions; ++j){
		hsystem * hs = new hsystem(params); //creating ensemble, generating nodes (l == 0)
		
		unsigned int l = 1, max = 2;
		//if(params.rbn_version == 1 || params.rbn_version == 4){
		//}
		if(params.rbn_version == 2 ) {
			max = params.levels.size();
		}

		if(params.rbn_version == 4){
			if(params.scheme == 1) {//HARBN where interlinks are updated.
				max = 3;
			}
		}

		if((params.rbn_version != 4) || (params.scheme != 2)){
			for(l = 1; l < max; ++l){

				hs->generate_hnetwork(l); //1st hierarchy level
				d_pajek_structure.push_back(hs->print_net());

				if(params.rbn_version == 4)
					if(params.scheme == 1)//HARBN where interlinks are updated.
						continue;
				
					
				for(int i = 0; i < params.iterations; ++i){
					set_hist_size(params.nodes);
					set_networks_size(hs->get_count());
					hs->iterate(execution_policy::parallel());
					save_networks_data(hs);
					if((i + 1) >= params.save_hist_from){
						save_hist(hs);
					}

					if((i + 1) % params.inform_after == 0 || (i + 1) == params.iterations) {
						int k_in = hs->get_Kin(0);
						t = clock() - t;
						log_status(i + 1, k_in, t);
						flush_all();
					}
				}

			}
		}
		else{ //HARBN changing inter- and intralinks by turns
			int iter = 0, turn;
			for(turn = 0; iter < params.iterations; ++turn){
				l = turn%2 + 1; //once intralinks and then interlinks are updated
				hs->generate_hnetwork(l); //1st hierarchy level
				d_pajek_structure.push_back(hs->print_net());
				set_hist_size(params.nodes);
				set_networks_size(hs->get_count());

				int stage_iterations;

				stage_iterations = (turn < 2)
						? params.initial_iterations
								: params.later_iterations;

				for(int i = 0; i < stage_iterations; ++i, ++iter){
					hs->iterate();

					save_networks_data(hs);
					if(iter >= params.save_hist_from) {
						save_hist(hs);
					}

					if((iter+1) % params.inform_after == 0){
						int k_in = hs->get_Kin(0);
						t = clock() - t;
						log_status(i + 1, k_in, t);
					}
				}
			}
		 }
		//saving last structure
		d_pajek_structure.push_back(hs->print_net());
		if(params.pajek_structure)
			save(PAJEK_STRUCTURE);

		delete hs;
		t_all = clock() - t_all;
#ifndef FOUT
		cout << "elapsed s: " << ((float)t_all)/CLOCKS_PER_SEC << endl;
		t_all = clock();
		cout << j+1 << " ";
#else
		fout << "elapsed s: " << ((float)t_all)/CLOCKS_PER_SEC << endl;
		t_all = clock();
		fout << j+1 << " ";
#endif
	}
}

void handler::enable_output() {
	d_hist_Kin.set_enabled(params.hist_Kin);
	d_hist_Kout.set_enabled(params.hist_Kout);
	d_hist_info.set_enabled(params.hist_info);
	d_hist_cc.set_enabled(params.hist_cc);
	d_each_period.set_enabled(params.each_period);
	d_each_basin.set_enabled(params.each_basin);
	d_each_Kin.set_enabled(params.each_Kin);
}

void handler::flush_all() {
	d_hist_Kin.flush((params.f_hist_Kin + params.add_on).c_str());
	d_hist_Kout.flush((params.f_hist_info + params.add_on).c_str());
	d_hist_info.flush((params.f_hist_Kout + params.add_on).c_str());
	d_hist_cc.flush((params.f_hist_cc + params.add_on).c_str());
	d_each_Kin.flush((params.f_each_Kin + params.add_on).c_str());
	d_each_period.flush((params.f_each_period + params.add_on).c_str());
	d_each_basin.flush((params.f_each_basin + params.add_on).c_str());
}

void handler::set_hist_size(size_t size) {
	d_hist_Kin.set_reserve_size(size);
	d_hist_Kout.set_reserve_size(size);
	d_hist_info.set_reserve_size(size);
}

void handler::set_networks_size(size_t size) {
	d_each_period.set_reserve_size(size);
	d_each_basin.set_reserve_size(size);
	d_each_Kin.set_reserve_size(size);
}

void handler::save_hist(hsystem* hs) {
	int n = hs->get_count();
	for(int hh = 0; hh < n; ++hh){
		save_hist(hs, hh);
	}
	d_hist_info.next_epoch();
	d_hist_Kin.next_epoch();
	d_hist_Kout.next_epoch();
	d_hist_cc.next_epoch();
}

void handler::save_hist(hsystem* hs, int network_index) {
	nodes_it it = hs->get_all(network_index).begin();
	nodes_it end = hs->get_all(network_index).end();
	long T = hs->get_period(network_index);
	for(; it != end; ++it){
		d_hist_info.push_back(double ((*it)->get_changes()) / T);
		d_hist_Kin.push_back((*it)->get_Kin() ); 
		d_hist_Kout.push_back((*it)->get_Kout() );
		d_hist_cc.push_back((*it)->get_CC());
	}
}

void handler::save_networks_data(hsystem* hs) {
	int n = hs->get_count();
	for(int hh = 0; hh < n; ++hh){
		d_each_period.push_back(hs->get_period(hh));
		d_each_basin.push_back(hs->get_basin(hh));
		d_each_Kin.push_back(hs->get_Kin(hh));
	}
	d_each_period.next_epoch();
	d_each_basin.next_epoch();
	d_each_Kin.next_epoch();
}

void handler::log_status(int epoch, int k_in, clock_t elapsed_clocks) const {
	OUTSTREAM << epoch << ": Connectivity of 1st network: " << k_in << endl;
	OUTSTREAM << "elapsed s: " << ((float) elapsed_clocks / CLOCKS_PER_SEC) << endl;

}
	
void handler::save(OPTIONS opt){
	//int n = params.get_network_count();
	//int nodes_nr = params.levels[0];
	switch(opt) {
	case HIST_KIN:
		params.hist_Kin = true;
		d_hist_info.flush((params.f_hist_Kin + params.add_on).c_str());
		break;
	case HIST_INFO:
		params.hist_info = true;
		d_hist_info.flush((params.f_hist_info + params.add_on).c_str());
		break;
	case HIST_KOUT:
		params.hist_Kout = true;
		d_hist_info.flush((params.f_hist_Kout + params.add_on).c_str());
		break;
	case HIST_CC:
		params.hist_cc = true;
		d_hist_info.flush((params.f_hist_cc + params.add_on).c_str());
		break;
	case EACH_KIN:
		params.each_Kin = true;
		d_each_Kin.flush((params.f_each_Kin + params.add_on).c_str());
		break;
	case EACH_PERIOD:
		params.each_period = true;
		d_each_period.flush((params.f_each_period + params.add_on).c_str());
		break;
	case EACH_BASIN:
		params.each_basin = true;
		d_each_basin.flush((params.f_each_basin + params.add_on).c_str());
		break;
	case PAJEK_STRUCTURE:
		{
			params.pajek_structure = true;
			if(d_pajek_structure.size() != 0){
				for(unsigned int i = 0; i < d_pajek_structure.size(); ++i) {
					string nr = "i" + std::string(size_t(1), '0' + i);
					ofstream data_file((params.f_pajek_structure + nr + params.pajek_add_on).c_str(), ofstream::app);
					data_file << d_pajek_structure[i];
					data_file.close();
				}
			}
			break;
		}
	}
	//reset_vectors();
}

void handler::set_save_name(string f, OPTIONS opt){
	if(opt == EACH_KIN)
		params.f_each_Kin = f;
//	else if(opt == EACH_INFO)
//		f_each_info = f;
	else if(opt == HIST_KIN)
		params.f_hist_Kin = f;
	else if(opt == HIST_INFO)
		params.f_hist_info = f;
	else if(opt == HIST_LAST)
		params.f_hist_last = f;
	else if(opt == EACH_PERIOD)
		params.f_each_period = f;
	else if(opt == EACH_BASIN)
		params.f_each_basin = f;
	else if(opt == PAJEK_STRUCTURE)
		params.f_pajek_structure = f;
	else if(opt == HIST_KOUT)
		params.f_hist_Kout = f;
	else if(opt == HIST_CC)
		params.f_hist_cc = f;
}
