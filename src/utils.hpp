#ifndef UTILS_HPP
#define UTILS_HPP

#define SINGLETON(_ClassName)                   /
private:                                        /
    _ClassName ();                              /
    template<typename...Args>                   /
    _ClassName (Args&&...);                     /
    _ClassName (const _ClassName&);             /
    _ClassName& operator=(const _ClassName&);   /
                                                /
public:                                         /
    template<typename...Args>                   /
    static _ClassName& instance(Args&&...args)  /
    {                                           /
        static _ClassName instance {args};      /
        return instance;                        /
    }                                           /
private:                                        /

#endif  // UTILS_HPP