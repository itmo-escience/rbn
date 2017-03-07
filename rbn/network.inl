template<class ExecutionPolicy>
void network::update_state(ExecutionPolicy) {
	nodes_it it = n_all.begin();
	for(int j = 0; j < N; ++j, ++it){
		(*it)->update_state(); //aktualizacja stanow wezlow
	}
}

template<class ExecutionPolicy>
long network::find_attractor(ExecutionPolicy ep) { //znajduje atraktor danej sieci i zwraca jego okres 
	long T[4] = {100, 1000, 10000, 100000};
	int max = 3;
	long i, k;
	//nodes_it it;
	
	ints state0 = state, state1;
	//cout << "fas";
	for(i = 1, k = 0; i < T[max]; ++i){
		update_state(ep);
		update_state_old();

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

template<class ExecutionPolicy>
void network::iterate(ExecutionPolicy ep) {
	//cout << "2. znajdywanie atraktora" << endl;
	//2. losowanie sieci i znajdywanie atraktora
	//cout << "is";
	//cout << "gen_st" << endl;
	generate_states();
	//cout << "atr" << endl;
	T = find_attractor(ep);
	//cout << "Attractor period: " << T << std::endl;
	update_connection();
}