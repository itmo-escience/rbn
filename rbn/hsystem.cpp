#include "hsystem.hpp"

#include <stdexcept>
#include <ctime>

#ifdef ENABLE_PARALLEL
#include "gpu/converter.hpp"
#endif

hsystem::hsystem(){
	rand = my_random::get_instance();
}

hsystem::hsystem(hparams p){
	params = p;
	rand = my_random::get_instance();

	generate_nodes();
}

void hsystem::generate_nodes(int N){//generates nodes
	all.clear();
	//creating N nodes
	for(int i = 0; i < N; ++i){
		int k = rand->next_int(1);
		node_ptr ptr = node_ptr(new node(all, k, 0));
		ptr->set_shared_from_this(ptr);
		if(params.rbn_version != 1) {
			ptr->set_preferential(params.link_update);
		} else {
			ptr->set_preferential(0);
		}
		double p_ch = params.ini_p - 0.5;
		ptr->update_p(p_ch);

		all.insert(ptr);
	}

	cout << "Generated " << all.size() << " nodes." << endl;
}

void hsystem::generate_nodes(){
	generate_nodes(params.nodes);
} 

void hsystem::generate_hnetwork(int nets_nr, int Kin_){
	ints Kins;
	for(int i = 0; i < nets_nr; ++i)
		Kins.push_back(Kin_);
	generate_hnetwork(nets_nr, Kins);
}

void hsystem::generate_hnetwork(int nets_nr, ints Kins_){
	nets.clear();

	int N = all.size();
	int nodes_nr = N / nets_nr;

	for(int i = 0; i < nets_nr; ++i){

		nets.push_back(network_ptr(new network(i+1, nodes_nr)));
		nets[i]->set_alpha(0);

		for(int j = 0; j < nodes_nr; ++j){
			nodes_it it;
			node_ptr hlp(new node(nodes_nr*i + j, all));
			it = all.find(hlp);

			nets[i]->add_node(*it);
			--N;
		}
	}

	//in case the the nodes do not equally divide into subnetworks.
	int k = nets_nr * nodes_nr, l = 0;
	while (N > 0){
		nodes_it it;
		node_ptr hlp(new node(k, all));
		it = all.find(hlp);

		nets[l]->add_node(*it);
		--N;
		++k;
		++l;
	}
	
	cout << "Divided " << all.size() << " nodes into " << nets.size() << " subnetworks." << endl;

	if( get_Kin() == 0){

		//generating initial subnetworks structure and states
		for(int i = 0; i < nets_nr; ++i){
			nets[i]->generate_structure(Kins_[i]);
			nets[i]->generate_states();
		}
 
		cout << "Generated initial structure of networks: added ";
		for(int i = 0; i < nets_nr; ++i){
			cout << Kins_[i] << " ";
		}
		cout << " incoming connections to each node." << endl;
		cout << "Generated nodes' states." << endl;
	}

	if(nets_nr > 1){
		if(params.rbn_version == 1 || params.rbn_version == 4){
			generate_interconnections(hsystem::CONST_NUM_UNIFORM);
		}
	}

	nodes_it it = all.begin(), end = all.end();
	for(; it != end; ++it){
		(*it)->update_vision();
		(*it)->calc_CC();
		(*it)->clear_vision();
		(*it)->update_boolean_functions();
		//(*it)->clear_e_in_all();
	}
/*
	//updating nodes' field vision
	if(params.rbn_version == 1){
		nodes_it it = all.begin(), end = all.end();
		for(; it != end; ++it){
			(*it)->update_vision();
		}
	}
	else if(params.rbn_version == 2){
		nodes_it it = all.begin(), end = all.end();
		for(; it != end; ++it){
			(*it)->update_vision();
		}
	}
	else if(params.rbn_version == 3){
		nodes_it it = all.begin(), end = all.end();
		for(; it != end; ++it){
			(*it)->update_vision();
		}
	}
	else if(params.rbn_version == 4){
		nodes_it it = all.begin(), end = all.end();
		for(; it != end; ++it){
			(*it)->update_vision();
		}
	}
*/
}

void hsystem::generate_hnetwork(int l){
	if(params.rbn_version == 1){
		int nets_nr = params.network_count;
		generate_hnetwork(nets_nr, params.Kins);
	}
	else if(params.rbn_version == 2){
		int nets_nr = params.levels[l];
		generate_hnetwork(nets_nr, params.ini_Kin);

		//updating nodes' parameter p and boolean functions
		nodes_it it = all.begin(), end = all.end();
		for(; it != end; ++it){
			(*it)->update_p(params.p_changes[l]);
			(*it)->update_boolean_functions();
		}
	}
	else if(params.rbn_version == 3){
		int nets_nr = 1;
		generate_hnetwork(nets_nr, params.ini_Kin);
	}
	else if(params.rbn_version == 4){
		/** Now it will only be working with 2 hierarchy levels.
		 * However it can be extended to more levels,
		 * like in rbn_version 3.
		 */
		int nets_nr;
		if(l == 1)
			nets_nr = params.network_count;
		else if(l == 2)
			nets_nr = 1;

		//switching the code
		nodes_it it = all.begin(), end = all.end();
		for(; it != end; ++it){
			(*it)->switch_code(l - 1);
			(*it)->update_vision();
		}

		generate_hnetwork(nets_nr, params.Kins);
	}
}


void hsystem::generate_interconnections(int c){
	int w = 0;
	if(c == CONST_PROB){
		//model sta�y generowania: okresla sie stala liczbe wezlow, ktore zostana polaczone
		double q0s = params.prob / 3; //pstwo polaczenia jakiegos wezla z danej sieci z ktoryms wezlem z dowolnej innej

		//double q0i; //pstwo polaczenia konkretnego wezla danej sieci z pewnym wezlem dowolnej innej
		int n; //liczba polaczen wchodzacych z innych sieci

		int N = 0; //liczba wszystkich wezlow
		for(int i = 0; i < params.network_count; ++i){
			N += params.get_network_size(i);
		}
	//s/td::cout << N << " ";
		for(int i = 0; i < params.network_count; ++i){
			//q0i = q0s / params.get_network_size(i);
			n = int((N - params.get_network_size(i)) * q0s);
			if(n == 0) ++n;
	//std::cout << n << " ";
			for(int j = 0; j < params.network_count; ++j){
				if(i == j)
					continue;
				for(int k = 0; k < n; ++k){
					//cout << "tu";
					node_ptr n = nets[j]->get_random_node();
					//cout << "tu";
					int nr = rand->next_int(0, params.get_network_size(i) - 1);

					try{
						nets[i]->set_connection(nr, n);
						++w;
					}catch(const char*){ //dodano juz to polaczenie
					}
				}
			}
		}
	}
	//stala liczba polaczen wejsciowych rozlozona rownomiernie
	else if(c == CONST_NUM_UNIFORM){
		int S = params.network_count;
		if(S == 1)
			return;
		int n = params.tot_incon / (S * (S-1)); //tyle conajmniej polaczen we bedzie miedzy dowolnymi dwoma sieciami (relacji jednostronnej)

		//cout << "netssize: " << nets.size() << endl << (*nets[0]) << (*nets[1]) << endl;
		for(int i = 0; i < params.network_count; ++i){

			for(int j = 0; j < params.network_count; ++j){
				if(i == j)
					continue;
				int error = 0;
				for(int k = 0; k < n; ++k){
					//cout << "tu ";
					node_ptr n = nets[j]->get_random_node();
					//cout << "ta ";
					//int nr = rand->next_int(0, params.get_network_size(i) - 1);
					//cout << "te ";
					try{
						//nets[i]->set_connection(nr, n);
						nets[i]->set_connection(n);
						++w;
						error = 0;
					}catch(const char*){
						++error;
						if(error == 100)
							break;
						--k;
					}
				}
			}
		}

		int miss = params.tot_incon - w; //brakujace polaczenia
		int error = 0;
		for(int k = 0; k < miss; ++k){
			//losuj siec wyjsciowa
			int f1 = rand->next_int(0, params.network_count - 1);
			//losuj wezel
			node_ptr n1 = nets[f1]->get_random_node();

			//losuj siec wejsciowa
			int f2 = rand->next_int(0, params.network_count - 2);
			if(f2 == f1)
				++f2;
			//losuje wezel z tej sieci
			//int nr = rand->next_int(0, params.get_network_size(f2) - 1);

			//std::cout << f1 << "(" << n1->get_code() << ") >> " << f2 << "(" << nr << ")" << std::endl;

			try{
				nets[f2]->set_connection(/*nr, */n1);
				++w;
				error = 0;
			}catch(const char*){
				++error;
				if(error == 100)
					break;
				--k;
			}
		}
	}
#ifndef FOUT
	cout << "Generated " << w << " interconnections." << endl;
#else
	fout << "Generated " << w << " interconnections." << endl;
#endif
	//cout << (*nets[0]) << endl << (*nets[1]) << endl;
}

int hsystem::find_attractor(void) {
#ifdef ENABLE_PARALLEL
	gpu::rbn rbn = gpu::converter::make_rbn(*this);
	gpu::state gpu_state = gpu::converter::get_state(*this);
	gpu::attractor_info ai = rbn.find_attractor(gpu_state, params.max_attractor_length, params.use_knuth);
	gpu::converter::update_original_network(ai, *this);
	T = ai.length;
	it = ai.length + ai.transient;
#else
	unsigned int T[] = {100, 1000, 10000, 100000};
	const int max = sizeof(T) / sizeof(unsigned int) - 1;
	unsigned int i, j, k;

	ints2 state0 = state, state1;
	for(j = 0; j < nets.size(); ++j){
		state1.push_back(ints() );
	}

	for(i = 1, k = 0; i < T[max]; ++i){
		for(j = 0; j < nets.size(); ++j){
			//cout << (*nets[j]);
			nets[j]->update_state();
			//cout << i;
			state1[j] = nets[j]->get_network_state();
		}

		for(j = 0; j < nets.size(); ++j) {
			nets[j]->update_state_old();
		}

		if(state0 == state1)
			break;

		if(i == T[k]){
			++k;
			//cout << "ts";
			state0 = get_network_state();
			for(j = 0; j < nets.size(); ++j){
				nets[j]->clear_sum(); //czyszczenie sumy stanow wezlow (potrzebne do wzoru 3)
			}
		}
	}

	it = i;
	this->T = i;

	if(i == T[max]){
		cout << "koniec";// << std::endl;
		this->T = i - T[max - 1];
	}
	else if(k > 0) {
		this->T = i - T[k-1];
	}
#endif
	return this->T;
}

void hsystem::iterate(void){
    clock_t t = clock();
	if(params.rbn_version == 1){
		for(int i = 0; i < params.network_count; ++i){
			//nets[i]->iterate();
			nets[i]->generate_states();
		}
		find_attractor();
		//if (T < 5)
			//cout << T ;//<< endl;
		for(int i = 0; i < params.network_count; ++i){
			nets[i]->set_period(T);
            size_t nodes_to_rewire = 1;
			if(params.proportional) {
				nodes_to_rewire = nets[i]->get_n_all().size() / 80u;
			}
            for(size_t u = 0; u < nodes_to_rewire; ++u) {
                nets[i]->update_connection();
            }
		}
	}
	else if(params.rbn_version == 2){
		for(unsigned int i = 0; i < nets.size(); ++i){
			nets[i]->iterate();
		}
	}
	else if(params.rbn_version == 3){
		//cout << nets.size();
		nets[0]->iterate();
	}
	else if(params.rbn_version == 4){
		for(unsigned int i = 0; i < nets.size(); ++i){
			//nets[i]->iterate();
			nets[i]->generate_states();
		}
		//cout << "fdfd ";
		find_attractor();
		//if (T < 5)
		//cout << T ;//<< endl;
		for(unsigned int i = 0; i < nets.size(); ++i){
			nets[i]->set_period(T);
			nets[i]->update_connection();
		}
		//cout << " k ";
	}
    static std::ofstream file("time_per_epoch", std::ios::app);
    t = clock() - t;
    file << ((float)t)/CLOCKS_PER_SEC << std::endl;
    file.flush();
}

ints2 hsystem::get_network_state(void){
	int i, N = nets.size();
	ints2 state;
	//state.clear();
	state.reserve(N);
	//nodes_it it = all.begin();

	for(i = 0; i < N; ++i){
		state.push_back(nets[i]->get_network_state());
	}

	return state;
}

ints hsystem::get_network_state(unsigned int n){
	if(n >= nets.size())
		return ints();
	return nets[n]->get_network_state();
}

double hsystem::get_Kin(unsigned int n) const{
	if(n >= nets.size())
		return -1;
	return nets[n]->get_Kin();
}

double hsystem::get_Kout(unsigned int n) const{
	if(n >= nets.size())
		return -1;
	return nets[n]->get_Kout();
}

double hsystem::get_Kin() const{
	double mean = 0;
	unsigned int N = nets.size();
	for(unsigned int i = 0; i < N; ++i){
		mean += nets[i]->get_Kin();
	}
	return mean / N;
}

double hsystem::get_CC() const{ //returns average CC
	double mean = 0;
	unsigned int N = nets.size(), N2;
	N2 = N;
	double CC;
	for(unsigned int i = 0; i < N; ++i){
		CC = nets[i]->get_CC();
		if(CC!=-1)
			mean += nets[i]->get_CC();
		else --N2;
	}

	if(N2 == 0)
		return -1;
	else return mean / N2;
}

nodes& hsystem::get_all(unsigned int n){
	if(n >= nets.size())
		throw "Error! Too large n!";
	return nets[n]->get_n_all();
}

long hsystem::get_period(unsigned int n) const{
	if(n >= nets.size())
		return -1;
	return nets[n]->get_period();
}

/**
* Prints network in pajek format
*/
string hsystem::print_net(){
	string str;
	stringstream cout;
	//count nodes
	int nodes_nr = all.size();
	
	cout << "*Vertices " << nodes_nr << std::endl;

	//writing edgeslist
	cout << "*Arcslist" << std::endl;

	nodes_it it = all.begin(), end = all.end(), itb, ite;
	int code;
	nodes output;

	for(; it != end; ++it){
		code = (*it)->get_codes(0) + 1;

		output = (*it)->get_output();
		if(output.size() == 0)
			continue;

		//writing this node code
		cout << code << " ";

		itb = output.begin();
		ite = output.end();
		for(; itb != ite; ++itb){
			//writing connections
			cout << (*itb)->get_codes(0) + 1 << " ";
		}
		cout << std::endl;
	}

	str = cout.str();

	return str;
}

/**
* Some format
*/
std::ostream& operator<<(std::ostream& cout, const hsystem& c){
	int n = c.nets.size() ;
	cout << "The hierarchical system cointains of " << n << " networks." << std::endl;
	for(int i = 0; i < n; ++i){
		cout << "Network " << i <<". " << c.nets[i] << std::endl;
	}
	return cout;
}
