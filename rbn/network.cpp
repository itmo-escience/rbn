#include "network.hpp"
#include <iostream>

network::network(int code_, int N_ = 10) : code(code_), N(N_){
	rand = my_random::get_instance();

	alpha = doubles();
	alpha_prop = doubles();
}

void network::generate_network(int Kin_){ //generuje wezly i polaczenia poczatkowe
	//boost::random::uniform_int_distribution<> dist(0, 1);
	generate_nodes();
	generate_structure(Kin_);
}

void network::generate_nodes(){ //generates nodes
	unsigned int i, n;
	unsigned int N = (unsigned int)this->N;
	
	//ustalanie alph
	
	doubles alphas, alpha_count;
	ints a_c;
	for(i = 0; i < alpha.size(); ++i){
		alpha_count.push_back(floor(N*alpha_prop[i] + 0.5));
		a_c.push_back(0);
	}
	
	i = 0;
	n = 0;
	while((i < N) && (n < alpha.size())){
		if(alpha_count[n] == 0){
			++n;
			continue;
		}
		
		alphas.push_back(alpha[n]);
		++a_c[n];
		
		++i;
		--alpha_count[n];
	}
	//cout << alphas.size();/*
	if(alphas.size() < N){
		for(i = alphas.size(); i < N; ++i){
			alphas.push_back(alpha[n - 1]);
		}
	}//*/
	
	for(i = 0; i < N; ++i){
		int k = rand->next_int(1);
		node_ptr ptr = node_ptr(new node(n_all, k, alphas[i]));
		ptr->push_code(code);
		ptr->set_shared_from_this(ptr);

		n_all.insert(ptr);
	}
#ifndef FOUT	
	cout << "Generated nodes." << std::endl;
	cout << "Following alpha values were applied: ";
	for(i = 0; i < alpha.size(); ++i){
		cout << a_c[i] << " nodes with alpha = " << alpha[i] << ", ";
	}
	cout << "." << endl;
#endif
}

void network::generate_nodes(int N_){ //generates nodes
	N = N_;
	generate_nodes();
}

void network::generate_structure(int Kin_){ //generate initial connections
	nodes_it it = n_all.begin();
	for(int i = 0; i < N; ++i, ++it){
		(*it)->set_in_connections(Kin_);
		(*it)->create_boolean_functions();
	}
#ifndef FOUT
	cout << "Generated in-connections." << std::endl;
#endif
}

void network::generate_states(){
	nodes_it it = n_all.begin();
	state.clear();
	state.reserve(N);
	
	int k;
	for(int i = 0; i < N; ++i, ++it){
		k = rand->next_int(1);
		(*it)->set_state(k);
		(*it)->clear_sum();
		state.push_back(k);
	}
}

void network::add_node(const node_ptr& node){ //adds node to network
	n_all.insert(node);

	node->push_code(code);
}

void network::set_states(ints a){
	nodes_it it = n_all.begin();
	for(int i = 0; i < N; ++i, ++it){
		(*it)->set_state(a[i]);
	}
	state = a;
}

void network::set_alpha(double alpha_){ //ustawia parametr alpha
	alpha.clear();
	alpha.push_back(alpha_);
	alpha_prop.clear();
	alpha_prop.push_back(1.0);
}

void network::set_alpha(doubles alpha_){ //ustawia parametry alpha
	alpha = alpha_;
	for(unsigned int i = 0; i < alpha.size(); ++i)
		alpha_prop.push_back(1.0/alpha.size());
}

void network::set_connection(int node_code, node_ptr n){ //nowym wejsciem wezla o kodzie note_code staje sie wezel node
	nodes_it it;

	node_ptr hlp(new node(node_code, n_all));
	it = n_all.find(hlp);

	(*it)->add_in_connection(n);
	(*it)->update_boolean_functions();
}

void network::set_connection(node_ptr n){ //nowym wejsciem jednego z wezlow sieci staje sie wezel node
	int nr = rand->next_int(0, n_all.size()-1);
	nodes_it it = n_all.begin();

	advance(it, nr);

	(*it)->add_in_connection(n);
	(*it)->update_boolean_functions();
}

node_ptr network::get_random_node(){ //losuje wezel
	int nr = rand->next_int(0, n_all.size()-1);
	nodes_it it = n_all.begin();

	advance(it, nr);

	return (*it)->get_me();
}

void network::update_state(void){
	nodes_it it;
	it = n_all.begin();
	for(int j = 0; j < N; ++j, ++it){
		(*it)->update_state(); //aktualizacja stanow wezlow
	}
}

void network::update_state_old(void){
	nodes_it it;
	it = n_all.begin();
	for(int j = 0; j < N; ++j, ++it){
		(*it)->update_state_old();
	}
}

void network::clear_sum(void){
	nodes_it it;
	int j;
	for(j = 0, it = n_all.begin(); j < N; ++j, ++it){
		(*it)->clear_sum(); //czyszczenie sumy stanow wezlow (potrzebne do wzoru 3)
	}
}

long network::find_attractor(void){ //znajduje atraktor danej sieci i zwraca jego okres 
	long T[4] = {100, 1000, 10000, 100000};
	int max = 3;
	long i, k;
	//nodes_it it;
	
	ints state0 = state, state1;
	//cout << "fas";
	for(i = 1, k = 0; i < T[max]; ++i){
		//cout << i;
		update_state();
		//cout << i;
		state1 = get_network_state();
		if(state0 == state1)
			break;
		
		if(i == T[k]){
			++k;
			//cout << "ts";
			state0 = get_network_state();
			clear_sum();
			//cout << "te";
			//cout << (*this) << " ";
		}
		//cout << (*this) << " ";
	}
	//cout << " " << i << " ";
	this->T = i;
	//cout << "fae";
	if(i == T[max]){
#ifndef FOUT
		cout << "koniec";
#endif
		this->T = i - T[max - 1];
		return i - T[max - 1];
	}
	else if(k > 0) {
		this->T = i - T[k-1];
		return i - T[k-1];
	}
	else return i;
}

//#define UPDATE_CC
//#define UPDATE_P
//#define REMOVE_MORE //with some prob. remove link instead of adding RESULT: gives exponential in-degree, but out-degree becomes exponential as well
//#define ADD_LESS //with some prob. skip adding a new link  RESULT: TOTALLU NOT WORKING
//#define CHOOSE_ANOTHER //with some prob. dont add new link, but draw a new node
void network::update_connection(void){

	nodes_it it, beg = n_all.begin(), end = n_all.end();
	int j = 0;

	//updating nodes' probabilities
	for(it=beg; it != end; ++it){
		(*it)->update_e_in();
	}

//	for(it=beg; it != end; ++it){
//		(*it)->update_distr_vision();
//	}

	//3. losowanie i-tego wezla do zmiany
	//this loop is because there is a chance to draw a node with no edges to remove or add, 
	//then other node is drawn. There will be 10 such tries by maximum.
	while(j < 1){//10){ 
		++j;
		it = beg;

		int nr = rand->next_int(0, n_all.size()-1);
		//cout << "wylosowano: " << nr << std::endl;
		advance(it, nr);

		double av_act;
		if (T==0)
			av_act = 1;
		else av_act = (*it)->get_sum() / double(T); //srednia aktywanosc w czasie trwania atraktora

		//cout << "4.";
		//4. zmiana polaczen wybranego wezla
		double alpha = (*it)->get_alpha();
#ifndef REMOVE_MORE
		if((av_act >= 1 - alpha) || (av_act <= 0 + alpha)){ //wezel jest zamrozony, trzeba dodac polaczenie
#else
		double r = rand->next_double(),
				cc = (*it)->calc_CC()*10;
		if(cc < 0)
			cc = 0.5;
		//if((*it)->get_Kin() <= 1)
		//	r = 1;
		if(r < cc && ((av_act >= 1 - alpha) || (av_act <= 0 + alpha) )){ //wezel jest zamrozony, trzeba dodac polaczenie
#endif
#ifdef ADD_LESS
			double r = rand->next_double();
			if(r > 0.5)
				continue;
#endif
#ifdef CHOOSE_ANOTHER
			double r = rand->next_double();
			if(r > 0.5){
				--j;
				continue;
			}
#endif

			(*it)->update_distr_vision(true);

			if ((*it)->add_in_connections(1) == -1)
				continue; //not succedeed. This node can't have anything added
		}
		else{
			//cout << "usu";
			(*it)->update_distr_vision(false);
			//cout << "2";
			if ((*it)->remove_in_connections(1) == -1)
				continue; //not succeeded. This node can't have any edge removed.
		}
		break;
	}

#ifdef UPDATE_P
#ifndef CALC_CC_HERE //in node.hpp
	//updating clustering coefficients of chosen node
	double CC = (*it)->calc_CC();
#else
	double CC = (*it)->get_CC();
#endif
	//changing parameter p according to CC
	int A = 3;
	double p_new;
	if(CC != -1)
		p_new = (1.0 + sqrt(1-2*CC/A)) / 2;
	else p_new = 0.5;
	(*it)->set_p(p_new);
//cout <<p_new << "|";
#else
#ifndef CALC_CC_HERE
#ifdef UPDATE_CC
		(*it)->calc_CC();
#endif
#endif
#endif
	//cout << "5s";
	//cout << "5." << endl;
	//5. regeneracja funkcji boolowskich, quenched (hartowany) model
	//(*it)->update_boolean_functions();

	//update p
#ifdef FLUCTUATION
	//zmieniamy p o 0.01. Gdy wylosujemy 1, to +0.01, w p.p. -0.01.
	int p_val = rand->next_int(1);
	double p_ch = p_val ? 0.01: -0.01;

	(*it)->update_p(p_ch);
#endif

	//annealed way (wyzarzany model)
	it = beg;
	//nodes_it end = n_all.end();
	for(; it != end; ++it){
		(*it)->update_boolean_functions();
	}
	//cout << "5e";

	set_states(state);
	//cout <<"ie";
}

void network::iterate(void){ //iteracja sieci
	//cout << "2. znajdywanie atraktora" << endl;
	//2. losowanie sieci i znajdywanie atraktora
	//cout << "is";
	//cout << "gen_st" << endl;
	generate_states();
	//cout << "atr" << endl;
	T = find_attractor();
	//cout << "Attractor period: " << T << std::endl;

	update_connection();
}

ints network::get_network_state(void){
	int i;
	ints state;
	//state.clear();
	state.reserve(N);
	nodes_it it = n_all.begin();

	for(i = 0; i < N; ++i, ++it){
		state.push_back((*it)->get_state());
	}

	return state;
}

double network::get_Kin() const{ //zraca srednie Kin
	nodes_it it = n_all.begin();

	int j;
	double Kin_mean = 0;
	for(j = 0; j < N; ++j, ++it){
		Kin_mean += (*it)->get_Kin(); 
	}
	Kin_mean /= N;

	return Kin_mean;
}

double network::get_Kout() const{ //zraca srednie Kout
	nodes_it it = n_all.begin();

	int j;
	double Kout_mean = 0;
	for(j = 0; j < N; ++j, ++it){
		Kout_mean += (*it)->get_Kout(); 
	}
	Kout_mean /= N;

	return Kout_mean;
}

double network::get_CC() const{ //returns average clustering coefficient
	nodes_it it = n_all.begin();

	int j, N2 = N;
	double CC_mean = 0, CC;
	for(j = 0; j < N; ++j, ++it){
		CC = (*it)->get_CC();
		if(CC != -1)
			CC_mean += CC;
		else --N2;
	}
	if(N2 == 0)
		return CC_mean = -1;
	else CC_mean /= N2;

	return CC_mean;
}

/*
node_ptr& network::get_node(int n){
	if(n >= N)
		throw ("Error! get_node. Dont have so many nodes.");
	return n_all.find(n);
}*/

/*to juz chyba jest w stl
bool operator== (ints left, ints right){
	vector<int>::iterator itl = left.begin(),
		itr = right.begin(), lend = left.end();

	while(itl != lend){
		if((*itl) != (*itr))
			return false;
		++itl;
		++itr;
	}

	if(itr != right.end())
		return false;
	return true;
}
*/

std::ostream& operator<<(std::ostream& cout, const network& c){
	cout << "The network has " << c.N << " nodes." << std::endl;
	cout << "Average input connectivity: " << c.get_Kin() << std::endl;
	cout << "Average output connectivity: " << c.get_Kout() << std::endl;

	nodes_it it = c.n_all.begin(), end = c.n_all.end();
	for(; it != end; ++it){
		cout << (**it) << std::endl;
	}

	return cout;
}
