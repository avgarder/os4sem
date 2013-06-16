template<class T>
struct vector {
    T* data;
    bool true_data;

    vector(size_t len) : true_data(true) {
        data = new T[len];
    }

    vector(vector<T>&& a) : data(a.data), true_data(true) {
        a.true_data = false;
    }

    vector<T>& operator = (vector<T>&& a) {
        vector<T> n(a);
        std::swap(n, *this);
    }

    T* get_data() {
        return data;
    }

    ~vector() {
        if (true_data) {
            delete[] data;
        }
    }

    T& operator[](const int id) {
        return data[id];
    }
};
