#ifndef BUFFER_2D_HPP
#define BUFFER_2D_HPP

template<class T>
struct buffer_2d {
	typedef T value_type;
	typedef vector< vector<T> > storage_type;

	//Constructors
	explicit buffer_2d(bool enabled = true)
		: m_enabled(enabled)
		, m_epoch_delimiter('\n')
		, m_value_delimiter(' ')
		, m_reserve_size()
		, m_values() {};

	// Getters
	bool enabled() const { return m_enabled; }
	storage_type& values() { return m_values; }
	storage_type values() const { return m_values; }

	// Setters
	void set_enabled(bool enable_ = true) { m_enabled = enable_; }
	void enable() { m_enabled = true; }
	void disable() { m_enabled = false; }
	void set_reserve_size(size_t size) { m_reserve_size = size; }

	// Member functions
	void clear();
	void push_back(const T& value);
	void next_epoch();
	void flush(std::ostream& os);
	void flush(const char* filename);

private:
	bool m_enabled;
	char m_epoch_delimiter;
	char m_value_delimiter;
	size_t m_reserve_size;
	vector< vector<T> > m_values;
};

template<class T>
void buffer_2d<T>::clear() {
	if(!m_enabled) return;
	m_values.clear();
	next_epoch();
}

template<class T>
void buffer_2d<T>::push_back(const T& value) {
	if(!m_enabled) return;
	if(m_values.empty()) {
		next_epoch();
	}
	m_values.back().push_back(value);
}

template<class T>
void buffer_2d<T>::next_epoch() {
	if(!m_enabled) return;
	m_values.push_back(vector<T>());
	if(m_reserve_size) {
		m_values.back().reserve(m_reserve_size);
	}
}

template<class T>
void buffer_2d<T>::flush(std::ostream& os) {
	if(!m_enabled) return;
	for(size_t i = 0; i < m_values.size(); ++i) {
		if(m_values[i].size() > 0) {
			std::copy(m_values[i].begin(), m_values[i].end(), std::ostream_iterator<T>(os, " "));
			os << m_epoch_delimiter;
		}
	}
	os.flush();
	clear();
}

template<class T>
void buffer_2d<T>::flush(const char* filename) {
	if(!m_enabled) return;
	std::ofstream os(filename, std::ios::app);
	flush(os);
}

#endif // BUFFER_2D_HPP
