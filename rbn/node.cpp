#include "node.hpp"
#include <iostream>

node::node(nodes& all_, int state_, double alpha_) : all(all_), code(all_.size()), state(state_), sum(0), changes(0), p(0.5),  alpha(alpha_){
	state_old = state;

	Kin = 0;
	Kout = 0;
	//ext_Kin = 0;

	random = my_random::get_instance();

	codes.push_back(code);
	level = codes.size() - 1;
	Kins.push_back(0);
}

node::node(int code_, nodes& all_, double alpha_) : all(all_), code(code_), p(0.0), alpha(alpha_) {}

node::~node() {}

void node::push_code(int c){
	codes.push_back(c); 
	Kins.push_back(0);
	//input = combine(input, remove_vision);
	remove_vision.clear(); //no connections to destroy
	++level;
}

void node::switch_code(int l){ //switches to the level that already was created
	if(l > level)
		cout << "ERROR! There is not such level yet. Use push_code instead." << endl;

	level = l;
	remove_vision.clear();
	vision.clear();

}

void node::update_vision(){
	vision.clear();

	//adding nodes to vision that have same number of network in current level and different in the previous
	//and these nodes should not already have a connection to this node
	
	if(level == 0) return; //there are no nodes that can make a connection to this node!
	
	nodes_it it = all.begin(), end = all.end();
	for(; it != end; ++it){
		if (me == *it)
			continue;
		int cur_code = (*it)->get_codes(level),
			prev_code = (*it)->get_codes(level - 1);

		if(cur_code != codes[level])
			continue;
		if(prev_code == codes[level - 1])
			continue;

		nodes_it th_it = input.find(*it);
		if(th_it != input.end()){
			remove_vision.insert(*it);
			continue;
		}
		vision.insert(*it);
	}
	//cout << vision.size() << " " << remove_vision.size() << "|";

	//initializing
	if(e_in_all.size() == 0){
		e_in_all = doubles(all.size(), 0.01);
		int nr = get_node_number(all, me);
		e_in_all[nr] = 0;
		distr_vision = doubles(vision.size(), 1.0/vision.size());
		for(unsigned int i = 1; i < vision.size(); ++i){
			distr_vision[i] += distr_vision[i-1];
		}
	}
}

/**
 * This function updates e_in fields, that is: decreases values of field e_in_all according to the equation,
 * sets smaller than 0.01 to 0.01, sets those, that this node has connection to, to 1.0.
 *
 * (1) val(t) = exp(-par*(-ln(val(t-1))/par + 1)
 *
 * The equation of decreasing is expotential: val(t) = exp(-par*t).
 * If par is set to 0.023, a node that had 1.0 after 100 epochs has 0.1. Time t is unknown
 * If par is set to 0.23, a node that had 1.0 after 10 epochs has 0.1
 * (it could be known, but more data should be kept). Knowing the value in the last epoch the value in next can be derived as it was written.
 *
 * (2) val(t) = par*(pow(pow(val(t-1)/par, -1.0/gamma) + 1), -gamma)
 *
 * The equation of decreasing is power-law: val(t) = par*pow(t, -gamma).
 * If par is set to 1 and gamma is 2, a node that had 1.0 after 10 epochs has 0.01.
 * If par is set to 10 and gamma is 3, a node that had 1.0 after 10 epochs has 0.01.
 * Time t is unknown
 * (it could be known, but more data should be kept). Knowing the value in the last epoch the value in next can be derived as it was written.
 *
 * (3) val(t) = par*(atan(-alfa + tan(val(t-1)/par - pi/2)) + pi/2)
 *
 * The equation of decreasing is given by modified arcus tangens: val(t) = par*(atan(-alfa*t + T) + pi/2).
 * T shifts the atan. T is related with par and alfa with following equations:
 * 		par = 1.0/(atan(T) + pi/2) (calculated from requirement that val(0) = 1
 * 		alfa = 1.0/tau(T-tan(eps/par - pi/2)), where tau is the time after val decreases to eps.
 * If T is set to 2, tau = 10, eps = 0.1, then par = 0.37 and alfa = 0.56
 *
 * (4) There is no equation of decreasing. If the node is not connected then the val(t) = 0.01. If it is connected the val(t) = 1.
 *
 * These equations might be implemented in decrease_e_in_fun function.
 */
#define EQ4
void node::update_e_in(){ //updates fields e_in
	// decreasing all values
#ifdef EQ1
	double par = 0.23;//0.023;
#elif defined EQ2
//#ifdef EQ2
	double gamma = 2, eps = 0.1, tau = 10;
	double par = 100;//pow((pow(eps,-1.0/gamma)-1)/tau,-gamma);
//#endif
#elif defined EQ3
	double T = 2, eps = 0.1, tau = 10;
	double par = 1.0/(atan(T) + M_PI_2);
	double alfa = 1.0/tau*(T - tan(eps/par - M_PI_2));
#endif

	unsigned int i = 0;
	for(; i < e_in_all.size(); ++i){
#ifdef EQ1
		e_in_all[i] = exp(-par*(-log(e_in_all[i])/par + 1));
#elif defined EQ2
		e_in_all[i] = par*pow(pow(e_in_all[i]/par, -1.0/gamma) + 1, -gamma);
#elif defined EQ3
		e_in_all[i] = par*(atan(-alfa+tan(e_in_all[i]/par - M_PI_2)) + M_PI_2);
#elif defined EQ4
		e_in_all[i] = 0.01;
#endif

		//TODO: drobny blad ponizej
		//setting smaller to 0.1
		if (e_in_all[i] < 0.01) //tu jest drobny blad, jest 0.01 zamiast eps
			e_in_all[i] = 0.01;
	}

	//setting to 1
	int nr;
	nodes_it it = output.begin(), end = output.end();
/*
	for(; it != end; ++it){
		nr = get_node_number(all, *it);

		e_in_all[nr] += 0.2;
	}
//*/
	//setting input nodes to 1 as well
	it = input.begin();
	end = input.end();
	for(; it != end; ++it){
		nr = get_node_number(all, *it);
		e_in_all[nr] = 2.0;//1.0;
	}

	//setting e_in to this node to 0.0
	nr = get_node_number(all, me);
	e_in_all[nr] = 0;

	//print_vector(e_in_all);
}

/**
 * This function is used to decrease e_in value in next epoch
 */
//#define //defined before update_e_in
double node::decrease_e_in_fun(double old_val) const{
	double new_value = 0;
//not working

	return new_value;
}


/**
 * This function creates distribution values for choosing a node. Only nodes from vision set are considered.
 * Apart from taking internal pseudoprob. values, pseudoprob. values from nodes, that this node has a connection to, has to be taken into account.
 * That is why output set has to be iterated.
 * 23.04.2015 In recent changes, only values of input nodes are taken into consideration
 * 15.05.2015 Implemented different number of neigbours considered. If neighbours is 2 then the in-neighbours of nodes' in-neighbours are considered,
 * i.e. nodes that have a in-connection to the nodes that have a in-connection to the considered node.
 * Also input parameter has been added. addlink should be true if link is to be added. It should be false if link is to be deleted.
 * If addlink is true, then after this function field distr_vision contains info about distribution important for new link adding.
 * Otherwise, field distr_vision contains info about distribution important for link deleting.
 */
#define neighbours 1 //defines level of neighbours that are considered
void node::update_distr_vision(bool addlink){//updates field prob_vision
	distr_vision.clear();

	doubles ptr;
	doubles prob_all;// = e_in_all;
	if(addlink)
		prob_all = e_in_all;
	else
		prob_all = doubles(e_in_all.size(), 0.01);

//print_vector(prob_all);
	nodes_it it = input.begin(), end = input.end();
	int nr;
	unsigned int i;
	for(; it != end; ++it){
		ptr = (*it)->e_in_all;
		//for(i = 0; i < all.size(); ++i){
		//	prob_all[i] += ptr[i];
		//}
//print_vector()
		//prob_all.resize(ptr.size());
		std::transform(ptr.begin(), ptr.end(), prob_all.begin(), prob_all.begin(), plus<double>());

#if neighbours == 2
		nodes it_input = (*it)->get_input();
		nodes_it it2 = it_input.begin(), end2 = it_input.end();
		//cout << it_input.size() << endl;
		for(; it2 != end2; ++it2){
			ptr = (*it2)->e_in_all;
			std::transform(ptr.begin(), ptr.end(), prob_all.begin(), prob_all.begin(), plus<double>());
		}
#endif
	}

	//print_vector(prob_all);

	if(addlink){
		it = vision.begin();
		end = vision.end();
	}
	else{
		it = remove_vision.begin();
		end = remove_vision.end();

		//cout << remove_vision.size() << "_";
	}

	double sum = 0;
	if(addlink){
		for(; it != end; ++it){
			nr = get_node_number(all, *it);

			distr_vision.push_back(prob_all[nr]);
			sum += prob_all[nr];
		}
	}
	else{//reverting  prob. to have it more likely to disconnect a node that has fewer connections
		double val;
		for(; it != end; ++it){
			nr = get_node_number(all, *it);
			val = 1.0 / prob_all[nr];
			distr_vision.push_back(val);
			sum += val;
		}
	}
	//cout << sum << "|";
	//cout << endl;

	unsigned int n = distr_vision.size();


	//print_distribution();
	//calculating probabilities
	for(i = 0; i < n;  ++i){
		distr_vision[i] /= sum;
//		if(distr_vision[i] == 0){
//			cout << "ERROR!!! 0 probability";
//			cout << *me;
//		}
	}
	//print_distribution();



	//print_distribution();
	//calculating distribution
	for(i = 1; i < distr_vision.size();  ++i){
		distr_vision[i] += distr_vision[i - 1];
	}

//	if(abs(distr_vision[n-1] - 1) > 1e-14){
//		//cout << distr_vision[n-1] << " ERROR! CDF not grows to 1! |" << addlink;
//		//if(addlink)
//		//	cout << "error";
//		//cout << "ERROR";
//		//cout << *me;
//		//print_vector(vector<node_ptr>(input));
//		print_distribution();
//		//print_vector(prob_all);
//		cout << sum;
//
//		//cout << endl;
//	}

	//print distr_vision
	//print_distribution();
}

int node::set_in_connections(int Kin_){
	update_vision();
	if(vision.size() == 0)
		return -1;

	//if there are less possible nodes then Kin_, then only Kin edges are created.
	if(vision.size() < (unsigned int) Kin_)
		Kin = vision.size();
	else Kin = Kin_;
	
	int j = 0;
	while(j < Kin){

		int k = random->next_int(0, vision.size()-1);

/*
		double r = random->next_double();
		unsigned int k;
		for(k = 0; k < distr_vision.size(); ++k){
			if(r <= distr_vision[k])
				break;
		}
*/
		nodes_it it = vision.begin();
		advance(it, k);
		
		input.insert(*it);
		(*it)->output.insert(me);
		(*it)->Kout++;

		remove_vision.insert(*it);
		vision.erase(it);

		++j;
	}

	Kins[level] = Kin;

	return 0;
}

void node::create_boolean_functions(void){
	funs.reset();
	funs = funs_ptr(new boolean_functions(p, Kin));// + ext_Kin));
}

void node::update_boolean_functions(void){
	funs->update_connections(Kin);// + ext_Kin);
}

void node::update_p(double p_ch){
	p += p_ch;

	create_boolean_functions();
}

int node::add_remove_in_connections(int con){
	if(con > 0)
		return add_in_connections(con);
	else if(con < 0)
		return remove_in_connections(-con);
	return 0;
}

int node::add_in_connections(int con){ //dodaje con polaczen wejsciowych

	if(vision.size() == 0)
		return -1;

	//if there are less possible nodes then Kin_, then only Kin edges are created.
	if(vision.size() < (unsigned int)  con)
		con = vision.size();
	
	Kin += con;
	Kins[level] += con;
	
	int j = 0;
	while(j < con){	
		unsigned int k;
//#ifndef PA
		if(!PA)
			k = random->next_int(0, vision.size()-1);
//#else
		else{
			double r = random->next_double();
			//unsigned int k;
			for(k = 0; k < distr_vision.size(); ++k){
				if(r <= distr_vision[k])
					break;
			}
		}
//#endif
		nodes_it it = vision.begin();
		advance(it, k);

		input.insert(*it);
		(*it)->output.insert(me);
		(*it)->Kout++;

#ifdef CALC_CC_HERE
		update_CC(*it);
#endif

		remove_vision.insert(*it);
		vision.erase(it);

		++j;

		if(con > 1){
			update_distr_vision();
		}
	}

	//cout << "|ab";
	//updating CC

	//cout << "aa|";

	return 0;
}

int node::remove_in_connections(int con){ //usuwa con polaczen wejsciowych
	if(Kins[level] == 0)
		return -1;

	if(remove_vision.size() == 0){
		cout << "ERROR!!!";
	}

	//if there are less possible nodes then con, then only con edges are deleted.
	if(Kins[level] < con)
		con = Kins[level];
	
	Kin -= con;
	Kins[level] -= con;
	
	int j = 0;
	while(j < con){
		unsigned int k;
//#ifndef PD
		if(!PD)
			//equal disconnection prob
			k = random->next_int(0, remove_vision.size()-1);
//#else
		else{
			//modified disconnection prob.
			double r = random->next_double();
			//unsigned int k;
			for(k = 0; k < distr_vision.size(); ++k){
				if(r <= distr_vision[k])
					break;
			}
		}
//#endif
		
		nodes_it it = remove_vision.begin();
		advance(it, k);

		nodes_it it_me = (*it)->output.find(me);
		if(it_me == (*it)->output.end())
			cout << (*(*it));

		//cout << "|" << (*it)->get_CC() << " " << (*it)->get_Kout() + (*it)->get_Kin() << " ";

		(*it)->output.erase(it_me);
		(*it)->Kout--;
		

		//updating CC in this node, because in a moment we will not know which node was it
		//cout << (*it)->calc_CC();
		node_ptr n = (*it);

		vision.insert(*it);

		nodes_it it2 = input.find(*it);
		input.erase(it2);
		remove_vision.erase(it);
		++j;

#ifdef CALC_CC_HERE
		update_CC(n);
#endif

	}

//	cout << "rm";
	//updating CC
//	update_CC();
//	cout << "ra|";

	if(Kin < 0)
		cout << "he";

	return 0;
}

int node::add_in_connection(const node_ptr& a){ //dodaje polaczenie wejsciowe z zewnetrznego wezla, tylko wtedy powinno byc stosowane by dobrze dzialaly pola level, Kins itp.
	unsigned int s = input.size();

	input.insert(a);
	a->output.insert(me);
	a->Kout++;

	if( s == input.size())
		return -1;
	else ++Kin;//ext_Kin++;

	if((unsigned int)(level + 1)== Kins.size())
		Kins.push_back(0);
	Kins[level + 1] = Kins[level + 1] + 1;

	//cout << Kins[2] << " " << Kins.size() << " " << level << "|";
	//cout << codes[0] << " " << codes[1] << " " << codes.size() << "|";
	//if(Kins[2] == 0)
	//	cout << "j";
	return 0;
}

int node::remove_in_connection(const node_ptr& a){ //usuwa polaczenie wejsciowe z zewnetrznego wezla
	nodes_it it_a = input.find(a);
	unsigned int s = input.size();

	nodes_it it_me = a->output.find(me);
	if(it_me == a->output.end())
		cout << (*a);

	a->output.erase(it_me);
	a->Kout--;

	input.erase(it_a);

	if( s == input.size())
		return -1;
	else --Kin;//--ext_Kin;

	Kins[level + 1] = Kins[level + 1] - 1;

	return 0;
}

void node::update_state(){
		state_old = state;

		state = funs->get_value(input);

		sum += state;
		if(state != state_old)
			changes += 1;
}

void node::update_state_old(){
		state_old = state;
}

double node::calc_CC(){ //calculate clustering coefficient
	if(Kin+Kout < 2) {//CC is undefined
		CC = -1.0;
		return CC;
	}

	nodes neigh = combine(input, output);
	nodes_it beg = neigh.begin(), end = neigh.end();
	//nodes_it it2 = it;

	CC = 0;
	int N = neigh.size();

	if(N < 2){
		CC = -1.0;
		return CC;
	}

	int cons = 0;
	for(nodes_it it = beg; it != end; ++it){
		for(nodes_it it2 = beg; it2!=end; ++it2){
			if(it == it2)
				continue;

			if((*it)->input.find(*it2) != (*it)->input.end())
				++cons;
		}
	}

	CC = double(cons) / N / (N-1);

	return CC;
}

double node::update_CC(node_ptr node){ //calculate CC of this node, node pointed by it and their neighbours
	//finding mutual neighbours
	nodes this_neigh = combine(input, output), it_neigh = combine(node->input, node->output);
	nodes all_neigh = combine(this_neigh, it_neigh);
	//nodes& neigh = this_neigh;
	nodes_it beg = this_neigh.begin(), end = this_neigh.end();

	int na = all_neigh.size(), nt = this_neigh.size(), ni = it_neigh.size();

	//if(na == nt + ni){ //there are no mutual neighbours	}

	if(na != nt + ni){
		int mut = 0;
		for(nodes_it it = beg; it != end; ++it){
			if(it_neigh.find(*it) != it_neigh.end()){
				(*it)->calc_CC();
				++mut;
			}
		}
		if(nt + ni != na + mut)
			cout << "ERROR. Sth went wrong!" << endl;
	}

	//updating it CC:
	node->calc_CC();

	if(nt < 2){
		CC = -1.0;
	}
	else{
		int cons = 0;
		for(nodes_it it = beg; it != end; ++it){
			(*it)->calc_CC();
			for(nodes_it it2 = beg; it2!=end; ++it2){
				if(it == it2)
					continue;

				if((*it)->input.find(*it2) != (*it)->input.end())
					++cons;
			}
		}
		CC = double(cons) / nt / (nt-1);
	}
//cout << "c";
	return CC;
}

void node::update_neigh_CC(){ //calculate clustering coefficients of all neighbours
	nodes neigh = combine(input, output);
	nodes_it beg = neigh.begin(), end = neigh.end();
	//nodes_it it2 = it;

	for(nodes_it it = beg; it != end; ++it){
		(*it)->calc_CC();
	}
}

nodes node::get_input() 
{
	return combine(input, remove_vision);
}

double node::get_e_in(const node_ptr& ptr) const{ //returns e_in to connecting to this node
	nodes_it it = all.find(ptr);
	int dist = distance(all.begin(), it);

	return e_in_all[dist];
}

double node::get_e_in(const int k) const{ //returns e_in to connecting to the node of given int code
	node_ptr ptr = node_ptr(new node(k, all, 0.0));
	//advance(it, k);

	return get_e_in(ptr);
}

/*
//TODO: consider adding different level codes
bool node::operator<(const node right) { 
	
	return code < right.get_code(); 
}
*/

bool node::operator<(const int right) {
	return this->get_code() < right; 
}

void node::print_distribution(){
	if(code > -1){
	for(unsigned int i = 0; i < distr_vision.size(); ++i){
		cout << /*std::setprecision(1) << */ floor(distr_vision[i]*100) << " "; // @kkirka
	}
	cout << endl;}

}

//TODO: add different level codes
std::ostream& operator<<(std::ostream& cout, const node& c){
	cout << "C: " << c.code << ". S: " << c.state << ". Output: ";

	nodes_it it = c.output.begin(), end = c.output.end();
	for(; it != end; ++it){
		cout << (*it)->get_code() << " ";
	}

	cout << "; Input: ";

	it = c.input.begin(); 
	end = c.input.end();
	for(; it != end; ++it){
		cout << /*(*it)->get_net_code() << "." <<*/ (*it)->get_code() << " ";
	}
	/*
	if(c.ext_input.size() != 0){
		cout << " |O| ";
		it = c.ext_input.begin(); 
		end = c.ext_input.end();
		for(; it != end; ++it){
			cout << (*it)->get_code() << " ";
		}
	}*/

	return cout;
}

bool operator<(const node_ptr& l, const node_ptr& r){
	//if(l->get_code() < r->get_code())
	if((*l) < (*r) )
		return true;
	else return false;
}

bool operator<(const node_ptr& l, const int r){
	if((*l) < r)
		return true;
	else return false;
}

bool operator<(const int l, const node_ptr& r){
	return r < l;
}

nodes combine(nodes& a, nodes& b){
	if(b.size() == 0)
		return a;
	else if(a.size() == 0)
		return b;
	nodes c = a;

	nodes_it beg = b.begin(), end = b.end();
	//for(; it != end; ++it)
	for(; beg != end; ++beg)
		c.insert(*beg);

	return c;
}

//gets node number in a set
int get_node_number(const nodes &set, const node_ptr ptr){
	nodes_it it = set.find(ptr);
	int dist = distance(set.begin(), it);

	return dist;
}

template<typename T>
void print_vector(std::vector<T> vec){
	for(unsigned int i = 0; i < vec.size(); ++i){
		cout << /*std::setprecision(1) << */ round(vec[i]*100) << " ";
	}
	cout << endl;
}
