#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <vector>

#include "node.hpp"
#include "boolean_functions.hpp"
#include "my_random.hpp"

#include "fluctuation.hpp" //there you need to comment in/out the line if you want to add p fluctuation. 

#include <cmath>

using namespace std;

typedef vector<int> ints;
typedef vector<double> doubles;

//class representing network
class network{
public:
	//TODO: other constructors
	network(int code, int N_);
	~network(){}

	void generate_network(int Kin_); //generuje wezly i polaczenia poczatkowe
	void generate_nodes(int N_); //generates nodes
	void generate_nodes(); //generates nodes
	void generate_structure(int Kin_); //generate initial connections
	void generate_states(); //generuje losowy stan sieci

	void add_node(const node_ptr& node); //adds node to network

	void set_states(ints); //ustawia stan sieci
	void set_alpha(double alpha_); //ustawia parametr alpha
	void set_alpha(doubles alpha_); //ustawia parametry alpha
	void set_alpha(doubles alpha_, doubles prop); //ustawia parametry alpha i alpha_prop
	void set_connection(int node_code, node_ptr node); //nowym wejsciem wezla o kodzie node_code staje sie wezel node
	void set_connection(node_ptr node); //nowym wejsciem jednego z wezlow sieci staje sie wezel node

	node_ptr get_random_node(); //losuje wezel

	void update_state(void); //aktualizuje stan sieci
	void update_state_old(void);
	void clear_sum(void); //czysci sumy/zmian wszystkich wezlow
	long find_attractor(void); //znajduje atraktor danej sieci i zwraca jego okres 
	void update_connection(void); //updateituje na podstawie wyniku szukania atraktora
	void iterate(void); //iteracja sieci

	ints get_network_state(void); //zwraca stan sieci (wszystkich wezlow) jako liczba w kodzie dwojkowym
	double get_Kin() const; //zraca srednie Kin
	double get_Kout() const; //zraca srednie Kout
	double get_CC() const; //returns average clustering coefficient

	int get_code() const{return code;}
	nodes& get_n_all() {return n_all;}
	long get_period() const {return T;}

	void set_period(long T_) {T = T_;}

	friend std::ostream& operator<<(std::ostream&, const network&);

protected:
	const int code; //network code
	int N; //liczba wezlow
	doubles alpha; //parameter of Changing Connections Rule. Zawiera tablice alf. Wezly moga przyjmowac rozne alfa. Poki co liczba wezlow przyjmujacych dane alfa jest z gory okreslona (nie jest zmienna losowa). To jaki procent wszystkich wezlow przyjmie dana alfa jest zapisane w zmiennej alpha_prop
	doubles alpha_prop; //patrz wyzej
	nodes n_all; //wezly
	my_random *rand;

	long T; //okres atraktora
	ints state;
};

//bool operator== (ints left, ints right);
std::ostream& operator<<(std::ostream&, const  network&);

#endif
