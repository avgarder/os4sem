template<class T>
class optional {
    bool data_ex;
    T data;
public:
    optional() : data_ex(false) {
    }

    optional(T&& d) : data(d), data_ex(true) {
    }

    bool data_exists() const {
        return data_ex;
    }

    T& operator *() const {
        return data;
    }
};
