#include <omp.h>

template<class ExecutionPolicy>
int hsystem::find_attractor(ExecutionPolicy) {
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
	return this->T;
}

template<class ExecutionPolicy>
void hsystem::iterate(ExecutionPolicy ep) {
	clock_t t = clock();
	if(params.rbn_version == 1){
		for(int i = 0; i < params.network_count; ++i){
			//nets[i]->iterate();
			nets[i]->generate_states();
		}
		find_attractor(ep);
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
		iterate_v2(ep);
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

template<class ExecutionPolicy>
void hsystem::iterate_v2(ExecutionPolicy ep) {
	for(unsigned int i = 0; i < nets.size(); ++i){
		nets[i]->iterate(ep);
	}
}
