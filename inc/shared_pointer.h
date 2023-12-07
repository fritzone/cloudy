#ifndef SHARED_POINTER_H
#define SHARED_POINTER_H



template <typename T, typename Freer>
class Sp
{
    Freer freer;
    struct deleter
    {
        deleter(T* o, Sp<T,Freer>* par) : obj(o), parent(par) {}
        ~deleter() {
            parent->freer(obj);
        }
        T* obj;
        Sp<T,Freer>* parent;

        T* release()
        {
            T* emp = obj;
            obj = nullptr;
            return emp;
        }
    };


public:
    Sp(Freer f) : freer(f), obj(nullptr), count(0)
    {}

    Sp(T* t, Freer f) : freer(f)
    {
        deleter d(t, this);
        count = new size_t(1);
        obj = d.release();
    }

    ~Sp()
    {
        destroy();
    }

    Sp(const Sp& other) : obj(other.obj), count(other.count)
    {
        if(count)
        {
            (*count)++;
        }
    }

    void swap(Sp& other)
    {
        std::swap(count, other.count);
        std::swap(obj, other.obj);
    }

    Sp& operator =(T* newValue)
    {
        destroy();
        count = new size_t(1);
        obj = newValue;
        return *this;
    }

    Sp& operator = (const Sp& rhs)
    {
        Sp temp(rhs);
        swap(temp);
        return *this;
    }

    T& operator *() const
    {
        if(obj)
        {
            return *obj;
        }
        throw std::exception();
    }

    T* operator ->() const
    {
        return obj;
    }

private:

    void destroy()
    {
        if(count)
        {
            if(!(--(*count)))
            {
                try
                {
                    freer(obj);
                }
                catch(...)
                {}

                delete count;
            }
        }
    }

    T* obj;
    size_t* count;
};

#endif // SHARED_POINTER_H
