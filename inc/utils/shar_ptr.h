#ifndef SHARED_POINTER_H
#define SHARED_POINTER_H

template <typename T, typename Freer>
class shared_pointer
{
    Freer freer;
    struct deleter
    {
        deleter(T* o, shared_pointer<T,Freer>* par) : obj(o), parent(par) {}
        ~deleter() {
            parent->freer(obj);
        }
        T* obj;
        shared_pointer<T,Freer>* parent;

        T* release()
        {
            T* emp = obj;
            obj = NULL;
            return emp;
        }
    };


public:
    shared_pointer(Freer f) : freer(f), obj(NULL), count(0)
    {}

    shared_pointer(T* t, Freer f) : freer(f)
    {
        deleter d(t, this);
        count = new size_t(1);
        obj = d.release();
    }

    ~shared_pointer()
    {
        destroy();
    }

    shared_pointer(const shared_pointer& other) : obj(other.obj), count(other.count)
    {
        if(count)
        {
            (*count)++;
        }
    }

    void swap(shared_pointer& other)
    {
        std::swap(count, other.count);
        std::swap(obj, other.obj);
    }

    shared_pointer& operator =(T* newValue)
    {
        destroy();
        count = new size_t(1);
        obj = newValue;
        return *this;
    }

    shared_pointer& operator = (const shared_pointer& rhs)
    {
        shared_pointer temp(rhs);
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
