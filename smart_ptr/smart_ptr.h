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

public:
    Meter*  _meter;
    T*		_raw;
    Meter*  _weak_ref_on_me;
    sp() : _meter (NULL), _raw (NULL), _weak_ref_on_me (NULL)
    {
    }
    sp (T* t)
    {
        _meter = new Meter (1);
        this->_raw = t;
        //_weak_ref_on_me = new Meter (0);
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

    void assign (const sp  &s)
    {
        _meter = s._meter;
        ++ (s._meter->_ref);
        _weak_ref_on_me = s._weak_ref_on_me;
    }

    ~sp()
    {
        if (_meter)
            -- (_meter->_ref);
        if (_meter &&  _meter->_ref == 0)
        {
            SAFE_DELETE (_raw);
            if (_weak_ref_on_me)
                _weak_ref_on_me->_ref = -_weak_ref_on_me->_ref;
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
        if (_meter) return _meter->_ref;
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
                if (_weak_ref->_ref == 0)  //I am the last raw hold the ref
                    SAFE_DELETE (_weak_ref);
            }
        }
    }
    wp (const wp& s)
    {
        _raw = s._raw;
    }
    wp& operator= (sp<T>& s)
    {
        if (_raw == (s._raw)) return *this;
        _raw = (s._raw);
        if (!s._weak_ref_on_me)
        {
            s._weak_ref_on_me = new Meter (0);
        }
        _weak_ref = s._weak_ref_on_me;
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