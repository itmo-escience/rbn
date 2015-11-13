#ifndef NODE_HPP
#define NODE_HPP

//#define CALC_CC_HERE

#define _USE_MATH_DEFINES

#include <cmath>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <ctime>
//#include <iterator>
#include <iomanip>      // std::setprecision
#include "boolean_functions.hpp"
#include "my_random.hpp"
#include "node_fwd.hpp"

#ifdef ENABLE_GPU_ACCELERATION
	#include "gpu_acc/dev_network_fwd.hpp"
#endif

typedef std::set<node_ptr> nodes;
typedef std::set<node_ptr>::iterator nodes_it;
typedef boost::shared_ptr<boolean_functions> funs_ptr;
typedef std::vector<int > ints;
typedef std::vector<double> doubles;

//class representing node
class node {
public:
	#ifdef ENABLE_GPU_ACCELERATION
		friend struct gpu_acc::network;
	#endif
	node(int code_, nodes& all_, double alpha_ = 0.0);
	node(nodes& all_, int state = 0, double alpha_ = 0.0);
	~node();// {if(p) std::cout << "destroying node nr: " << code << std::endl;}

	void set_nodes(nodes& all_) {all = all_;}
	void push_code(int c); //adds code of next hierarchy level
	void switch_code(int l); //switches to the level that already was created
	void update_vision(); //updates field vision
	void update_e_in(); //updates field e_in_all
	double decrease_e_in_fun(double) const; //function used to decrease e_in value in next epoch. NOT WORKING!!!
	void update_distr_vision(bool addlink = true);//updates field distr_vision

	//void set_out_connections(int Kout_); //ustawia polaczenia wyjsciowe
	int set_in_connections(int Kin_); //sets in connections, returns 0 on success, -1 on failure
	void set_preferential(bool PA_, bool PD_) {PA = PA_; PD = PD_;}
	void set_preferential(int PAD) {PA = bool(PAD%2); PD = bool(PAD/2);} // 0 - normal, 1 - PA, 2 - PD, 3 - PAD
	void create_boolean_functions(void); //tworzy funkcje boolowskie
	void update_boolean_functions(void); //odnawia funkcje boolowskie

	void update_p(double p_ch);
	void set_p(double p_new){p = p_new;}

	void set_shared_from_this(node_ptr ptr) {me = ptr;}

//changing connections functions
	int add_remove_in_connections(int con); //dodaje/usuwa con polaczen wejsciowych
	int add_in_connections(int con); //dodaje con polaczen wejsciowych
	int remove_in_connections(int con); //usuwa con polaczen wejsciowych
	int add_in_connection(const node_ptr&); //dodaje polaczenie z wezla z poza sieci
	int remove_in_connection(const node_ptr&); // usuwa polaczenie z wezla z poza sieci

//changing/updating states functions
	void update_state(); //aktualizacja stanow na podstawie wejsc
	void set_state(int k) {state = k; state_old = k;}
	void clear_sum(void) { sum = 0; changes = 0;}

	double calc_CC(); //calculate clustering coefficient
	double update_CC(node_ptr node); //calculate CC of this node, node pointed by it and their neighbours
	void update_neigh_CC(); //calculate clustering coefficients of all neighbours

//getters
	int get_level() const {return level;}
	int get_codes(int l) const {return codes[l];}
	int get_code() const {return code;}
	double get_alpha() const {return alpha;}
	node_ptr get_me() {return me;}
	int get_state() const {return state;}
	int get_state_old() const {return state_old;}
	long get_sum() const {return sum;}
	long get_changes() const {return changes;}
	int get_Kin() const {return Kin;}
	int get_Kout() const {return Kout;}
	double get_CC() const {return CC;}
	nodes get_input() ;
	nodes get_output() {return output;}

	double get_e_in(const node_ptr&) const; //returns e_in to connecting to this node
	double get_e_in(const int) const; //returns e_in to connecting to the node of givent int code
	doubles& get_e_ins() {return e_in_all;} //returns all e_in

	bool operator<(const node& right) {return this->get_code() < right.get_code(); }
	bool operator<(const int right);// {return this->get_code() < right; }
	
	void print_distribution();
	friend std::ostream& operator<<(std::ostream&, const node&);

protected:
	nodes& all; //wszystkie wezly
	nodes vision; //contains nodes which can make a connection to this node, but have not done it yet.
	nodes remove_vision; //contains nodes which have a connection to this node and this connection can be removed. remove_vision.size() == Kins[level]
	nodes input; //wezly, ktore maja polaczenie wejsciowe
	nodes output; //wezly, do ktorych ten wezel ma polaczenie wejsciowe

	int level; //current level, usually codes.size() - 1
	ints codes; //code of this node. It is vector of integers. 1st value is node number, 2nd - group of level 2 nr, etc.
	const int code; //kod
	//const int net_code; //kod sieci //to delete

	int state; //aktualny stan
	int state_old; //stan sprzed poprzedniej aktualizacji (wywolania update_state())
	long sum; //suma zmieniana w trakcie iterowania. Suma stanow wezla.
	long changes; //liczba zmian stanu wezla

	my_random* random;
	funs_ptr funs; //losowe funkcje boolowskie
	double p; //p-stwo generowania losowych funkji boolowskich
	const double alpha; //parametr okreslajacy zmiennosc polaczen tego wezla

	node_ptr me; //wskazuje na siebie

	doubles e_in_all; // contains internal pseudoprobabilities of connecting from this node to any other node
	doubles distr_vision;// contains distribution of connecting from this node to other nodes that are in field vision
	bool PA; //preferential attachment
	bool PD; //preferential detachment

	ints Kins; //in-degree connectivity with different levels: 1, 2, ...
	int Kin; //in-degree connectivity 
	int Kout; //out-degree connectivity
	//int ext_Kin; //external in-degree connectivity

	double CC; //local clustering coefficient
};

//operators
std::ostream& operator<<(std::ostream&, const  node&);
bool operator<(const node_ptr&, const node_ptr&);
bool operator<(const node_ptr&, const int);
bool operator<(const int, const node_ptr&);

//combines two node sets
nodes combine(nodes& a, nodes& b);

//gets node number in a set
int get_node_number(const nodes &set, const node_ptr ptr);

template<typename T>
void print_vector(std::vector<T>);

#endif
