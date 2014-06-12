#pragma once
#define  SAFE_DELETE(x)  do{delete x; x=NULL;}while(0)
class Meter
{
public:
    int _ref;
    Meter (int r) :_ref (r)
    {
    }
};

template<typename T >
class sp
{
    void assign (const sp  &s)
    {
        _strong_meter = s._strong_meter;
        ++ (s._strong_meter->_ref);
        _weak_meter = s._weak_meter;
    }
public:
    Meter*  _strong_meter;
    T*		_raw;
    Meter*  _weak_meter;
    sp() : _strong_meter (NULL), _raw (NULL), _weak_meter (NULL) {}
    sp (T* t)
    {
        _strong_meter = new Meter (1);
        this->_raw = t;
    }

    sp (const sp & s)
    {
        assign (s);
        _raw = s._raw;
    }
    template<typename S>
    sp (const sp<S>& s)
    {
        assign (s);
        _raw = dynamic_cast<T*> (s._raw);
    }



    ~sp()
    {
        if (_strong_meter)
            -- (_strong_meter->_ref);
        if (_strong_meter &&  _strong_meter->_ref == 0)
        {
            SAFE_DELETE (_raw);
            if (_weak_meter)
                _weak_meter->_ref = -_weak_meter->_ref;
        }
    }

    sp & operator= (sp & sp_)
    {
        if (_raw == sp_._raw) return *this;
        _raw = sp_._raw;
        assign (sp_);
        return *this;
    }


    template< typename S >
    sp<T>& operator= (sp<S>& sp_)
    {
        if (_raw == sp_._raw) return *this;
        _raw = dynamic_cast<T*> (sp_._raw);
        assign (sp_);
        return *this;
    }



    T* operator*()
    {
        return this;
    }
    T* operator->()
    {
        return _raw;
    }

    operator bool()
    {
        return _raw != NULL;
    }
    bool operator== (sp& sp_)
    {
        return _raw == sp_._raw;
    }
    int use_count() const
    {
        if (_strong_meter) return _strong_meter->_ref;
        return 0;
    }

};
template<typename T >
class wp
{
public:
    T*  _raw;
    Meter * _weak_ref;
    wp() : _raw (NULL), _weak_ref (NULL) {}
    ~wp()
    {
        if (_weak_ref)
        {
            if (_weak_ref->_ref > 0)
                --_weak_ref->_ref;
            //_raw is dead
            if (_weak_ref->_ref < 0)
            {
                ++_weak_ref->_ref;
                if (_weak_ref->_ref == 0)  //I am the last weak_ptr holding the ref
                    SAFE_DELETE (_weak_ref);
            }
        }
    }
    wp (const wp& s)
    {
        assign (s);
    }
    template<typename T >
    wp (const sp<T>& s)
    {
        return wp;
    }
    void assign (const wp &s)
    {
        _raw = s._raw;
        _weak_ref = s._weak_ref;
        if (_weak_ref)
            ++_weak_ref->_ref;
    }


    wp& operator= (wp<T>& s)
    {
        if (_raw == (s._raw)) return *this;
        assign (s);
        return *this;
    }
    wp& operator= (sp<T>& s)
    {
        if (_raw == (s._raw)) return *this;
        _raw = (s._raw);
        if (!s._weak_meter)
        {
            s._weak_meter = new Meter (0);
        }
        _weak_ref = s._weak_meter;
        ++_weak_ref->_ref;
        return *this;
    }

    T* operator->()
    {
        return _raw;
    }
    operator bool()
    {
        if (_weak_ref && _weak_ref->_ref <= 0)return NULL;
        return  _raw != NULL;
    }

};