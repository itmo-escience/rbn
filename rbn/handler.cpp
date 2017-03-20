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

void handler::reset_vectors(){

	d_each_Kin.clear();
	d_hist_Kin.clear();
	d_hist_info.clear();
	d_hist_last.clear();
	d_each_period.clear();
	d_each_basin.clear();
	d_pajek_structure.clear();
	d_hist_Kout.clear();
	d_hist_info.clear();

	d_each_Kin.push_back(vector<double>() ); //save Kin after each iteration
	d_each_Kin[0].reserve(params.iterations);

	d_hist_last.push_back(doubles() ); 
	d_hist_last[0].reserve(params.levels.size());

	d_each_period.reserve(params.iterations);
	d_each_basin.reserve(params.iterations);

	//d_pajek_structure.push_back("");
}

#include <fstream>
#include <sstream>
#include <ctime>
#include <string>

void handler::work(){
	ints hk, hkout;
	doubles hi, hc;

	reset_vectors();

	hk.reserve(params.nodes);
	hkout.reserve(params.nodes);
	hi.reserve(params.nodes);
	hc.reserve(params.nodes);

/*	if(params.rbn_version == 1 || params.rbn_version == 4){
		hk.reserve()
	}
	else if(params.rbn_version == 2 || params.rbn_version == 3){
		hk.reserve(params.nodes);
		hkout.reserve(params.nodes);
		hi.reserve(params.nodes);
	}*/

	clock_t t_all = clock(), t = clock();
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
				//std::stringstream ss;
				//ss << "level_" << l;
				//boost::filesystem::path path(ss.str());

				hs->generate_hnetwork(l); //1st hierarchy level
				d_pajek_structure.push_back(hs->print_net());

				if(params.rbn_version == 4)
					if(params.scheme == 1)//HARBN where interlinks are updated.
						continue;

				for(int i = 0; i < params.iterations; ++i){
					hs->iterate(execution_policy::parallel());

					if((i+1) % params.inform_after == 0){

#ifndef FOUT
						cout << i+1 << ": Connectivity of 1st network: " << hs->get_Kin(0) << endl;
						t = clock() - t;
						cout << "elapsed s: " << ((float)t)/CLOCKS_PER_SEC << endl;
						t = clock();
						//log_epoch_finished();
#else
						fout << i+1 << ": Connectivity of 1st network: " << hs->get_Kin(0) << endl;
						t = clock() - t;
						fout << "elapsed s: " << ((float)t)/CLOCKS_PER_SEC << endl;
						t = clock();
#endif
					}

					d_each_period.push_back(hs->get_period(0)); //saving periods need to be done better
					d_each_basin.push_back(hs->get_basin());

					d_each_Kin[0].push_back(hs->get_Kin());

					if(i >= params.save_hist_from){
						int n = hs->get_count();

						for(int hh = 0; hh < n; ++hh){

							nodes_it it = hs->get_all(hh).begin(), end = hs->get_all(hh).end();
							long T = hs->get_period(hh);

							for(; it != end; ++it){
								hi.push_back(double ((*it)->get_changes()) / T);
								hk.push_back((*it)->get_Kin() ); 
								hkout.push_back((*it)->get_Kout() );
								hc.push_back((*it)->get_CC());
							}
						}
					}

					//przepisywanie wynikow do wlasciwych wektorow
					if(i >= params.save_hist_from){
						d_hist_Kin.push_back(hk);
						hk.clear();
						d_hist_info.push_back(hi);
						hi.clear();
						d_hist_Kout.push_back(hkout);
						hkout.clear();
						d_hist_cc.push_back(hc);
						hc.clear();
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

				int stage_iterations;

				stage_iterations = (turn < 2)
						? params.initial_iterations
								: params.later_iterations;

				for(int i = 0; i < stage_iterations; ++i, ++iter){
					hs->iterate();

					if((iter+1) % params.inform_after == 0){

#ifndef FOUT
						cout << iter+1 << ": Connectivity of 1st network: " << hs->get_Kin(0) << endl;
						t = clock() - t;
						cout << "elapsed s: " << ((float)t)/CLOCKS_PER_SEC << endl;
						t = clock();
#else
						fout << iter+1 << ": Connectivity of 1st network: " << hs->get_Kin(0) << endl;
						t = clock() - t;
						fout << "elapsed s: " << ((float)t)/CLOCKS_PER_SEC << endl;
						t = clock();
#endif
					}

					d_each_period.push_back(hs->get_period(0)); //saving periods need to be done better
					d_each_basin.push_back(hs->get_basin());

					d_each_Kin[0].push_back(hs->get_Kin());

					if(iter >= params.save_hist_from){
						int n = hs->get_count();

						for(int hh = 0; hh < n; ++hh){

							nodes_it it = hs->get_all(hh).begin(), end = hs->get_all(hh).end();
							long T = hs->get_period(hh);

							for(; it != end; ++it){
								hi.push_back(double ((*it)->get_changes()) / T);
								hk.push_back((*it)->get_Kin() );
								hkout.push_back((*it)->get_Kout() );
								hc.push_back((*it)->get_CC());
							}
						}
					}

					//przepisywanie wynikow do wlasciwych wektorow
					if(iter >= params.save_hist_from){
						d_hist_Kin.push_back(hk);
						hk.clear();
						d_hist_info.push_back(hi);
						hi.clear();
						d_hist_Kout.push_back(hkout);
						hkout.clear();
						d_hist_cc.push_back(hc);
						hc.clear();
					}
				}
			}
		 }
		//saving last structure
		d_pajek_structure.push_back(hs->print_net());

		//przepisywanie wynikow do pliku
		if(params.each_Kin)
			save(EACH_KIN);
		if(params.hist_Kin)
			save(HIST_KIN);
		if(params.hist_info)
			save(HIST_INFO);
		if(params.hist_last)
			save(HIST_LAST);
		if(params.each_period)
			save(EACH_PERIOD);
		if(params.each_basin)
			save(EACH_BASIN);
		if(params.pajek_structure)
			save(PAJEK_STRUCTURE);
		if(params.hist_Kout)
			save(HIST_KOUT);
		if(params.hist_cc)
			save(HIST_CC);
		reset_vectors();

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

void handler::save(OPTIONS opt){
	//int n = params.get_network_count();
	//int nodes_nr = params.levels[0];
	if(opt == EACH_KIN){
		params.each_Kin = true;
		if(d_each_Kin.size() != 0){
			//for(int i = 0; i < nodes_nr; ++i){
				//string nr = "n" + std::string(size_t(1), '0' + i);
				ofstream data_file((params.f_each_Kin /* + nr*/ + params.add_on).c_str(), ofstream::app);
				for(unsigned int j = 0; j < d_each_Kin[0].size(); ++j)
					data_file << d_each_Kin[0][j] << params.delimiter;
				data_file << endl;
				data_file.close();
				
				d_each_Kin[0].clear();
			//}
			//d_each_Kin.clear();
		}
	}
	
	else if(opt == HIST_KIN){
		params.hist_Kin = true;
		if(d_hist_Kin.size() != 0){
			int n = d_hist_Kin.size();
			ofstream file((params.f_hist_Kin + params.add_on).c_str(), ofstream::app);

			for(int i = 0; i < n; ++i){
				for(unsigned int k = 0; k < d_hist_Kin[i].size(); ++k){
					file << d_hist_Kin[i][k] << params.delimiter;
				}
				file << endl;
				d_hist_Kin[i].clear();
			}

			file.close();
			//d_hist_Kin.clear();
		}
	}
	else if(opt == HIST_INFO){
		params.hist_info = true;
		if(d_hist_info.size() != 0){
			int n = d_hist_info.size();
			ofstream info_hist_file((params.f_hist_info + params.add_on).c_str(), ofstream::app);

			for(int i = 0; i < n; ++i){
				for(unsigned int k = 0; k < d_hist_info[i].size(); ++k){
					info_hist_file << d_hist_info[i][k] << params.delimiter;
				}
				info_hist_file << endl;
				d_hist_info[i].clear();
			}
			info_hist_file.close();
			//d_hist_Kin.clear();
		}
	}
	else if(opt == EACH_PERIOD){
		params.each_period = true;
		if(d_each_period.size() != 0){
			ofstream data_file((params.f_each_period + params.add_on).c_str(), ofstream::app);
			for(int i = 0; i < params.iterations; ++i){
				data_file << d_each_period[i] << params.delimiter;
			}
			data_file << endl;
			data_file.close();
		}
	}
	else if(opt == EACH_BASIN){
		params.each_basin = true;
		if(d_each_basin.size() != 0){
			ofstream data_file((params.f_each_basin + params.add_on).c_str(), ofstream::app);
			for(int i = 0; i < params.iterations; ++i){
				data_file << d_each_basin[i] << params.delimiter;
			}
			data_file << endl;
			data_file.close();
		}
	}
	else if(opt == PAJEK_STRUCTURE){
		params.pajek_structure = true;
		if(d_pajek_structure.size() != 0){
			for(unsigned int i = 0; i < d_pajek_structure.size(); ++i){
				string nr = "i" + std::string(size_t(1), '0' + i);
				ofstream data_file((params.f_pajek_structure + nr + params.pajek_add_on).c_str(), ofstream::app);
				data_file << d_pajek_structure[i];
				data_file.close();
			}
		}
	}
	else if(opt == HIST_KOUT){
		params.hist_Kout = true;
		if(d_hist_Kout.size() != 0){
			int n = d_hist_Kout.size();
			ofstream file((params.f_hist_Kout + params.add_on).c_str(), ofstream::app);

			for(int i = 0; i < n; ++i){
				for(unsigned int k = 0; k < d_hist_Kout[i].size(); ++k){
					file << d_hist_Kout[i][k] << params.delimiter;
				}
				file << endl;
				d_hist_Kout[i].clear();
			}
			file.close();
			//d_hist_Kin.clear();
		}
	} else if(opt == HIST_CC){
		params.hist_cc = true;
		if(d_hist_cc.size() != 0){
			int n = d_hist_cc.size();
			ofstream cc_hist_file((params.f_hist_cc + params.add_on).c_str(), ofstream::app);

			for(int i = 0; i < n; ++i){
				for(unsigned int k = 0; k < d_hist_cc[i].size(); ++k){
					cc_hist_file << d_hist_cc[i][k] << params.delimiter;
				}
				cc_hist_file << endl;
				d_hist_cc[i].clear();
			}
			cc_hist_file.close();
			//d_hist_Kin.clear();
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

/*
void handler::prepare_network_structure(hsystem* hs, int nr){
	int node_code;
	ints out_codes;

	nodes_it it = hs->get_all().begin(), end = hs->get_all().end(),
		itw, endw;
	for(; it != end; ++it){
		node_code = (*it)->get_code() + 1; //1 is added not to have node number 0
		
		nodes output = (*it)->get_input();

				itw = output.begin(); 
				endw = output.end();
				for(; itw != endw; ++itw){
					out_codes.push_back(get_node_code(itw) + 1);//1 is added not to have node number 0
				}

				data[save_moment][node_code] = out_codes;
				out_codes.clear();
			}
		}
}
*/

/*BOOST_FUSION_ADAPT_STRUCT(
	handler::node_info,
	(int, in_degree)
	(int, out_degree)
	(int, clustering_corfficient)
	(double, activity)
);

BOOST_FUSION_DEFINE_STRUCT(,
	save_flags,
	(bool, hist_Kin)
	(bool, hist_Kout)
	(bool, hist_cc)
	(bool, hist_info)
)

template<class T, class U>
void save_node_metric(std::ostream& os, T metric, U delimiter) {
	if(os.is_open()) {
		os << metric << delimiter;
	}
}

void handler::save_nodes_info(const std::vector<node_info>& nodes) const {
	std::ofstream s_in_degree;
	std::ofstream s_out_degree;
	std::ofstream s_clustering_corfficient;
	std::ofstream s_activity;

	if(params.hist_Kin) {
		s_in_degree.open(params.f_hist_Kin, std::ios::app);
	}
	if(params.hist_Kout) {
		s_in_degree.open(params.f_hist_Kout, std::ios::app);
	}
	if(params.hist_info) {
		s_in_degree.open(params.f_hist_info, std::ios::app);
	}
	if(params.hist_cc) {
		s_in_degree.open(params.f_hist_cc, std::ios::app);
	}

	for(size_t  = 0; i < nodes.size(); ++i) {
		save_node_metric(s_in_degree, node.in_degree, params.delimiter);
		save_node_metric(s_out_degree, node.out_degree, params.delimiter);
		save_node_metric(s_clustering_corfficient, node.clustering_corfficient, params.delimiter);
		save_node_metric(s_activity, node.activity, params.delimiter);
	}
}

void handler::epoch_finished(const std::vector<node_info>& nodes, const network_info& nets) {
	save_nodes_info(nodes)
}*/