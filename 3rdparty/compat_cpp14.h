// Ville Voutilainen code from include/bits/range_access.h from libstdc++

#if defined(__GNUC__) && ((__GNUC__ == 5 && __GNUC_MINOR__ < 2) || (__GNUC__ == 4 && __GNUC_MINOR__ > 8))

template<class _Container>
  inline constexpr auto
  cbegin(const _Container& __cont) noexcept(noexcept(std::begin(__cont)))-> decltype(std::begin(__cont))
  { return std::begin(__cont); }

template<class _Container>
  inline constexpr auto
  cend(const _Container& __cont) noexcept(noexcept(std::end(__cont)))-> decltype(std::end(__cont))
  { return std::end(__cont); }

template<class _Container>
  inline auto
  rbegin(_Container& __cont) -> decltype(__cont.rbegin())
  { return __cont.rbegin(); }

template<class _Container>
  inline auto
  rbegin(const _Container& __cont) -> decltype(__cont.rbegin())
  { return __cont.rbegin(); }

template<class _Container>
  inline auto
  rend(_Container& __cont) -> decltype(__cont.rend())
  { return __cont.rend(); }

template<class _Container>
  inline auto
  rend(const _Container& __cont) -> decltype(__cont.rend())
  { return __cont.rend(); }

template<class _Tp, size_t _Nm>
  inline reverse_iterator<_Tp*>
  rbegin(_Tp (&__arr)[_Nm])
  { return reverse_iterator<_Tp*>(__arr + _Nm); }

template<class _Tp, size_t _Nm>
  inline reverse_iterator<_Tp*>
  rend(_Tp (&__arr)[_Nm])
  { return reverse_iterator<_Tp*>(__arr); }

template<class _Tp>
  inline reverse_iterator<const _Tp*>
  rbegin(initializer_list<_Tp> __il)
  { return reverse_iterator<const _Tp*>(__il.end()); }

template<class _Tp>
  inline reverse_iterator<const _Tp*>
  rend(initializer_list<_Tp> __il)
  { return reverse_iterator<const _Tp*>(__il.begin()); }

template<class _Container>
  inline auto
  crbegin(const _Container& __cont) -> decltype(std::rbegin(__cont))
  { return std::rbegin(__cont); }

template<class _Container>
  inline auto
  crend(const _Container& __cont) -> decltype(std::rend(__cont))
  { return std::rend(__cont); }
  
#endif