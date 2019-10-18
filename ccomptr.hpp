#include <cassert>

template <class T>
class CComPtr
{
public:
    T *p;
public:
    CComPtr()
    {
        p = NULL;
    }

    CComPtr(T *lp)
    {
        p = lp;
        if (p != NULL)
            p->AddRef();
    }

    CComPtr(const CComPtr<T> &lp)
    {
        p = lp.p;
        if (p != NULL)
            p->AddRef();
    }

    ~CComPtr()
    {
        if (p != NULL)
            p->Release();
    }

    T *operator = (T *lp)
    {
        T* pOld = p;

        p = lp;
        if (p != NULL)
            p->AddRef();

        if (pOld != NULL)
            pOld->Release();

        return *this;
    }

    T *operator = (const CComPtr<T> &lp)
    {
        T* pOld = p;

        p = lp.p;
        if (p != NULL)
            p->AddRef();

        if (pOld != NULL)
            pOld->Release();

        return *this;
    }

    template <typename Q>
    T* operator=(const CComPtr<Q>& lp)
    {
        T* pOld = p;

        if (!lp.p || FAILED(lp.p->QueryInterface(__uuidof(T), (void**)(IUnknown**)&p)))
            p = NULL;

        if (pOld != NULL)
            pOld->Release();

        return *this;
    }

    void Release()
    {
        if (p != NULL)
        {
            p->Release();
            p = NULL;
        }
    }

    void Attach(T *lp)
    {
        if (p != NULL)
            p->Release();
        p = lp;
    }

    T *Detach()
    {
        T *saveP;

        saveP = p;
        p = NULL;
        return saveP;
    }

    T **operator & ()
    {
        assert(p == NULL);
        return &p;
    }

    operator T * ()
    {
        return p;
    }

    T *operator -> ()
    {
        assert(p != NULL);
        return p;
    }
};
