// Copyright (C) 2011 - 2012 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// The idea and interface is based on Boost.Optional library
// authored by Fernando Luis Cacciola Carballal

# include "optional.hpp"
# include <vector>
# include <iostream>
# include <functional>
# include <complex>



struct caller {
    template <class T> caller(T fun) { fun(); }
};
# define CAT2(X, Y) X ## Y
# define CAT(X, Y) CAT2(X, Y)
# define TEST(NAME) caller CAT(__VAR, __LINE__) = []

enum  State 
{
    sDefaultConstructed,
    sValueCopyConstructed,
    sValueMoveConstructed,
    sCopyConstructed,
    sMoveConstructed,
    sMoveAssigned,
    sCopyAssigned,
    sValueCopyAssigned,
    sValueMoveAssigned,
    sMovedFrom,
    sValueConstructed
};

struct OracleVal
{
    State s;
    int i;
    OracleVal(int i = 0) : s(sValueConstructed), i(i) {}
};

struct Oracle
{
    State s;
    OracleVal val;

    Oracle() : s(sDefaultConstructed) {}
    Oracle(const OracleVal& v) : s(sValueCopyConstructed), val(v) {}
    Oracle(OracleVal&& v) : s(sValueMoveConstructed), val(std::move(v)) {v.s = sMovedFrom;}
    Oracle(const Oracle& o) : s(sCopyConstructed), val(o.val) {}
    Oracle(Oracle&& o) : s(sMoveConstructed), val(std::move(o.val)) {o.s = sMovedFrom;}

    Oracle& operator=(const OracleVal& v) { s = sValueCopyConstructed; val = v; return *this; }
    Oracle& operator=(OracleVal&& v) { s = sValueMoveConstructed; val = std::move(v); v.s = sMovedFrom; return *this; }
    Oracle& operator=(const Oracle& o) { s = sCopyConstructed; val = o.val; return *this; }
    Oracle& operator=(Oracle&& o) { s = sMoveConstructed; val = std::move(o.val); o.s = sMovedFrom; return *this; }
};

struct Guard
{
    std::string val;
    Guard() : val{} {}
    explicit Guard(std::string s, int = 0) : val(s) {}
    Guard(const Guard&) = delete;
    Guard(Guard&&) = delete;
    void operator=(const Guard&) = delete;
    void operator=(Guard&&) = delete;
};

struct ExplicitStr
{
    std::string s;
    explicit ExplicitStr(const char* chp) : s(chp) {};
};

struct Date
{
    int i;
    Date() = delete;
    Date(int i) : i{i} {};
    Date(Date&& d) : i(d.i) { d.i = 0; }
    Date(const Date&) = delete;
    Date& operator=(const Date&) = delete;
    Date& operator=(Date&& d) { i = d.i; d.i = 0; return *this;};
};

bool operator==( Oracle const& a, Oracle const& b ) { return a.val.i == b.val.i; }
bool operator!=( Oracle const& a, Oracle const& b ) { return a.val.i != b.val.i; }


namespace tr2 = std::experimental;


TEST(disengaged_ctor)
{
    tr2::optional<int> o1;
    assert (!o1);

    tr2::optional<int> o2 = tr2::nullopt;
    assert (!o2);

    tr2::optional<int> o3 = o2;
    assert (!o3);

    assert (o1 == tr2::nullopt);
    assert (o1 == tr2::optional<int>{});
    assert (!o1);
    assert (bool(o1) == false);

    assert (o2 == tr2::nullopt);
    assert (o2 == tr2::optional<int>{});
    assert (!o2);
    assert (bool(o2) == false);

    assert (o3 == tr2::nullopt);
    assert (o3 == tr2::optional<int>{});
    assert (!o3);
    assert (bool(o3) == false);

    assert (o1 == o2);
    assert (o2 == o1);
    assert (o1 == o3);
    assert (o3 == o1);
    assert (o2 == o3);
    assert (o3 == o2);
};


TEST(value_ctor)
{
  OracleVal v;
  tr2::optional<Oracle> oo1(v);
  assert (oo1 != tr2::nullopt);
  assert (oo1 != tr2::optional<Oracle>{});
  assert (oo1 == tr2::optional<Oracle>{v});
  assert (!!oo1);
  assert (bool(oo1));
  // NA: assert (oo1->s == sValueCopyConstructed);
  assert (oo1->s == sMoveConstructed);
  assert (v.s == sValueConstructed);
  
  tr2::optional<Oracle> oo2(std::move(v));
  assert (oo2 != tr2::nullopt);
  assert (oo2 != tr2::optional<Oracle>{});
  assert (oo2 == oo1);
  assert (!!oo2);
  assert (bool(oo2));
  // NA: assert (oo2->s == sValueMoveConstructed);
  assert (oo2->s == sMoveConstructed);
  assert (v.s == sMovedFrom);

  {
      OracleVal v;
      tr2::optional<Oracle> oo1{tr2::in_place, v};
      assert (oo1 != tr2::nullopt);
      assert (oo1 != tr2::optional<Oracle>{});
      assert (oo1 == tr2::optional<Oracle>{v});
      assert (!!oo1);
      assert (bool(oo1));
      assert (oo1->s == sValueCopyConstructed);
      assert (v.s == sValueConstructed);

      tr2::optional<Oracle> oo2{tr2::in_place, std::move(v)};
      assert (oo2 != tr2::nullopt);
      assert (oo2 != tr2::optional<Oracle>{});
      assert (oo2 == oo1);
      assert (!!oo2);
      assert (bool(oo2));
      assert (oo2->s == sValueMoveConstructed);
      assert (v.s == sMovedFrom);
  }
};


TEST(assignment)
{
    tr2::optional<int> oi;
    oi = tr2::optional<int>{1};
    assert (*oi == 1);

    oi = tr2::nullopt;
    assert (!oi);

    oi = 2;
    assert (*oi == 2);

    oi = {};
    assert (!oi);
};


template <class T>
struct MoveAware
{
  T val;
  bool moved;
  MoveAware(T val) : val(val), moved(false) {}
  MoveAware(MoveAware const&) = delete;
  MoveAware(MoveAware&& rhs) : val(rhs.val), moved(rhs.moved) {
    rhs.moved = true;
  }
  MoveAware& operator=(MoveAware const&) = delete;
  MoveAware& operator=(MoveAware&& rhs) {
    val = (rhs.val);
    moved = (rhs.moved);
    rhs.moved = true;
    return *this;
  }
};

TEST(moved_from_state)
{
  // first, test mock:
  MoveAware<int> i{1}, j{2};
  assert (i.val == 1);
  assert (!i.moved);
  assert (j.val == 2);
  assert (!j.moved);
  
  MoveAware<int> k = std::move(i);
  assert (k.val == 1);
  assert (!k.moved);
  assert (i.val == 1);
  assert (i.moved);
  
  k = std::move(j);
  assert (k.val == 2);
  assert (!k.moved);
  assert (j.val == 2);
  assert (j.moved);
  
  // now, test optional
  tr2::optional<MoveAware<int>> oi{1}, oj{2};
  assert (oi);
  assert (!oi->moved);
  assert (oj);
  assert (!oj->moved);
  
  tr2::optional<MoveAware<int>> ok = std::move(oi);
  assert (ok);
  assert (!ok->moved);
  assert (oi);
  assert (oi->moved);
  
  ok = std::move(oj);
  assert (ok);
  assert (!ok->moved);
  assert (oj);
  assert (oj->moved);
};


TEST(copy_move_ctor_optional_int)
{
  tr2::optional<int> oi;
  tr2::optional<int> oj = oi;
  
  assert (!oj);
  assert (oj == oi);
  assert (oj == tr2::nullopt);
  assert (!bool(oj));
  
  oi = 1;
  tr2::optional<int> ok = oi;
  assert (!!ok);
  assert (bool(ok));
  assert (ok == oi);
  assert (ok != oj);
  assert (*ok == 1);
  
  tr2::optional<int> ol = std::move(oi);
  assert (!!ol);
  assert (bool(ol));
  assert (ol == oi);
  assert (ol != oj);
  assert (*ol == 1);
};


TEST(optional_optional)
{
  tr2::optional<tr2::optional<int>> oi1 = tr2::nullopt;
  assert (oi1 == tr2::nullopt);
  assert (!oi1);
  
  {
  tr2::optional<tr2::optional<int>> oi2 {tr2::in_place};
  assert (oi2 != tr2::nullopt);
  assert (bool(oi2));
  assert (*oi2 == tr2::nullopt);
  //assert (!(*oi2));
  //std::cout << typeid(**oi2).name() << std::endl;
  }
  
  {
  tr2::optional<tr2::optional<int>> oi2 {tr2::in_place, tr2::nullopt};
  assert (oi2 != tr2::nullopt);
  assert (bool(oi2));
  assert (*oi2 == tr2::nullopt);
  assert (!*oi2);
  }
  
  {
  tr2::optional<tr2::optional<int>> oi2 {tr2::optional<int>{}};
  assert (oi2 != tr2::nullopt);
  assert (bool(oi2));
  assert (*oi2 == tr2::nullopt);
  assert (!*oi2);
  }
  
  tr2::optional<int> oi;
  auto ooi = tr2::make_optional(oi);
  static_assert( std::is_same<tr2::optional<tr2::optional<int>>, decltype(ooi)>::value, "");

};

TEST(example_guard)
{
  using namespace tr2;
  //FAILS: optional<Guard> ogx(Guard("res1")); 
  //FAILS: optional<Guard> ogx = "res1"; 
  //FAILS: optional<Guard> ogx("res1"); 
  optional<Guard> oga;                     // Guard is non-copyable (and non-moveable)     
  optional<Guard> ogb(in_place, "res1");   // initialzes the contained value with "res1"  
  assert (bool(ogb));
  assert (ogb->val == "res1");
            
  optional<Guard> ogc(in_place);           // default-constructs the contained value
  assert (bool(ogc));
  assert (ogc->val == "");

  oga.emplace("res1");                     // initialzes the contained value with "res1"  
  assert (bool(oga));
  assert (oga->val == "res1");
  
  oga.emplace();                           // destroys the contained value and 
                                           // default-constructs the new one
  assert (bool(oga));
  assert (oga->val == "");
  
  oga = nullopt;                        // OK: make disengaged the optional Guard
  assert (!(oga));
  //FAILS: ogb = {};                          // ERROR: Guard is not Moveable
};


void process(){}
void process(int ){}
void processNil(){}


TEST(example1)
{
  using namespace tr2;
  optional<int> oi;                 // create disengaged object
  optional<int> oj = nullopt;          // alternative syntax
  oi = oj;                          // assign disengaged object
  optional<int> ok = oj;            // ok is disengaged

  if (oi)  assert(false);           // 'if oi is engaged...'
  if (!oi) assert(true);            // 'if oi is disengaged...'

  if (oi != nullopt) assert(false);    // 'if oi is engaged...'
  if (oi == nullopt) assert(true);     // 'if oi is disengaged...'

  assert(oi == ok);                 // two disengaged optionals compare equal
  
  ///////////////////////////////////////////////////////////////////////////
  optional<int> ol{1};              // ol is engaged; its contained value is 1
  ok = 2;                           // ok becomes engaged; its contained value is 2
  oj = ol;                          // oj becomes engaged; its contained value is 1

  assert(oi != ol);                 // disengaged != engaged
  assert(ok != ol);                 // different contained values
  assert(oj == ol);                 // same contained value
  assert(oi < ol);                  // disengaged < engaged
  assert(ol < ok);                  // less by contained value
  
  /////////////////////////////////////////////////////////////////////////////
  optional<int> om{1};              // om is engaged; its contained value is 1
  optional<int> on = om;            // on is engaged; its contained value is 1
  om = 2;                           // om is engaged; its contained value is 2
  assert (on != om);                // on still contains 3. They are not pointers

  /////////////////////////////////////////////////////////////////////////////
  int i = *ol;                      // i obtains the value contained in ol
  assert (i == 1);
  *ol = 9;                          // the object contained in ol becomes 9
  assert(*ol == 9);
  assert(ol == make_optional(9));  
  
  ///////////////////////////////////
  int p = 1;
  optional<int> op = p;
  assert(*op == 1);
  p = 2;                         
  assert(*op == 1);                 // value contained in op is separated from p

  ////////////////////////////////
  if (ol)                      
    process(*ol);                   // use contained value if present
  else
    process();                      // proceed without contained value
    
  if (!om)   
    processNil();
  else  
    process(*om);   
  
  /////////////////////////////////////////
  process(ol.value_or(0));     // use 0 if ol is disengaged
  
  ////////////////////////////////////////////
  ok = nullopt;                         // if ok was engaged calls T's dtor
  oj = {};                           // assigns a temporary disengaged optional
};


TEST(example_guard) 
{
  using std::experimental::optional;
  const optional<int> c = 4; 
  int i = *c;                        // i becomes 4
  assert (i == 4);
  // FAILS: *c = i;                            // ERROR: cannot assign to const int&
};


TEST(example_ref)
{
  using namespace std::experimental;
  int i = 1;
  int j = 2;
  optional<int&> ora;                 // disengaged optional reference to int
  optional<int&> orb = i;             // contained reference refers to object i

  *orb = 3;                          // i becomes 3
  // FAILS: ora = j;                           // ERROR: optional refs do not have assignment from T
  // FAILS: ora = {j};                         // ERROR: optional refs do not have copy/move assignment
  // FAILS: ora = orb;                         // ERROR: no copy/move assignment
  ora.emplace(j);                    // OK: contained reference refers to object j
  ora.emplace(i);                    // OK: contained reference now refers to object i

  ora = nullopt;                        // OK: ora becomes disengaged
};


template <typename T>
T getValue( tr2::optional<T> newVal = tr2::nullopt, tr2::optional<T&> storeHere = tr2::nullopt )
{
  T cached{};
  
  if (newVal) {
    cached = *newVal;
    
    if (storeHere) {
      *storeHere = *newVal; // LEGAL: assigning T to T
    }      
  }
  return cached;      
}

TEST(example_optional_arg)
{
  int iii = 0;
  iii = getValue<int>(iii, iii);
  iii = getValue<int>(iii);
  iii = getValue<int>();
  
  {
    using namespace std::experimental;
    optional<Guard> grd1{in_place, "res1", 1};   // guard 1 initialized
    optional<Guard> grd2;

    grd2.emplace("res2", 2);                     // guard 2 initialized
    grd1 = nullopt;                                 // guard 1 released

  }                                              // guard 2 released (in dtor)
};


std::tuple<Date, Date, Date> getStartMidEnd() { return std::tuple<Date, Date, Date>{Date{1}, Date{2}, Date{3}}; }
void run(Date const&, Date const&, Date const&) {}

TEST(example_date)
{
  using namespace std::experimental;
  optional<Date> start, mid, end;           // Date doesn't have default ctor (no good default date)

  std::tie(start, mid, end) = getStartMidEnd();
  run(*start, *mid, *end); 
};


std::experimental::optional<char> readNextChar(){ return{}; }

void run(std::experimental::optional<std::string>) {}
void run(std::complex<double>) {}


template <class T>
void assign_norebind(tr2::optional<T&>& optref, T& obj)
{
  if (optref) *optref = obj;
  else        optref.emplace(obj);
}

template <typename T> void unused(T&&) {}

TEST(example_conceptual_model)
{
  using namespace std::experimental;
  
  optional<int> oi = 0;
  optional<int> oj = 1;
  optional<int> ok = nullopt;

  oi = 1;
  oj = nullopt;
  ok = 0;

  unused(oi == nullopt);
  unused(oj == 0);
  unused(ok == 1);
};

TEST(example_rationale)
{
  using namespace std::experimental;
  if (optional<char> ch = readNextChar()) {
    unused(ch);
    // ...
  }
  
  //////////////////////////////////
  optional<int> opt1 = nullopt; 
  optional<int> opt2 = {}; 

  opt1 = nullopt;
  opt2 = {};

  if (opt1 == nullopt) {}
  if (!opt2) {}
  if (opt2 == optional<int>{}) {}
  
  
  
  ////////////////////////////////

  run(nullopt);            // pick the second overload
  // FAILS: run({});              // ambiguous

  if (opt1 == nullopt) {} // fine
  // FAILS: if (opt2 == {}) {}   // ilegal
  
  ////////////////////////////////
  assert (optional<unsigned>{}  < optional<unsigned>{0});
  assert (optional<unsigned>{0} < optional<unsigned>{1});
  assert (!(optional<unsigned>{}  < optional<unsigned>{}) );
  assert (!(optional<unsigned>{1} < optional<unsigned>{1}));

  assert (optional<unsigned>{}  != optional<unsigned>{0});
  assert (optional<unsigned>{0} != optional<unsigned>{1});
  assert (optional<unsigned>{}  == optional<unsigned>{} );
  assert (optional<unsigned>{0} == optional<unsigned>{0});
  
  /////////////////////////////////
  optional<int> o;
  o = make_optional(1);         // copy/move assignment
  o = 1;           // assignment from T
  o.emplace(1);    // emplacement 
  
  ////////////////////////////////////
  int isas = 0, i = 9;
  optional<int&> asas = i;
  assign_norebind(asas, isas);
  
  /////////////////////////////////////
  ////tr2::optional<std::vector<int>> ov2 = {2, 3};
  ////assert (bool(ov2));
  ////assert ((*ov2)[1] == 3);
  ////
  ////////////////////////////////
  ////std::vector<int> v = {1, 2, 4, 8};
  ////optional<std::vector<int>> ov = {1, 2, 4, 8};

  ////assert (v == *ov);
  ////
  ////ov = {1, 2, 4, 8};

  ////std::allocator<int> a;
  ////optional<std::vector<int>> ou { in_place, {1, 2, 4, 8}, a };

  ////assert (ou == ov);

  //////////////////////////////
  // inconvenient syntax:
  {
    
      tr2::optional<std::vector<int>> ov2{tr2::in_place, {2, 3}};
    
      assert (bool(ov2));
      assert ((*ov2)[1] == 3);
  
      ////////////////////////////

      std::vector<int> v = {1, 2, 4, 8};
      optional<std::vector<int>> ov{tr2::in_place, {1, 2, 4, 8}};

      assert (v == *ov);
  
      ov.emplace({1, 2, 4, 8});
/*
      std::allocator<int> a;
      optional<std::vector<int>> ou { in_place, {1, 2, 4, 8}, a };

      assert (ou == ov);
*/
  }

  /////////////////////////////////
  {
  typedef int T;
  optional<optional<T>> ot {in_place};
  optional<optional<T>> ou {in_place, nullopt};
  optional<optional<T>> ov {optional<T>{}};
  
  optional<int> oi;
  auto ooi = make_optional(oi);
  static_assert( std::is_same<optional<optional<int>>, decltype(ooi)>::value, "");
  }
};


bool fun(std::string , std::experimental::optional<int> oi = std::experimental::nullopt) 
{
  return bool(oi);
}

TEST(example_converting_ctor)
{
  using namespace std::experimental;
  
  assert (true == fun("dog", 2));
  assert (false == fun("dog"));
  assert (false == fun("dog", nullopt)); // just to be explicit
};


TEST(bad_comparison)
{
  tr2::optional<int> oi, oj;
  int i;
  bool b = (oi == oj);
  b = (oi >= i);
  b = (oi == i);
  unused(b);
};


//// NOT APPLICABLE ANYMORE
////TEST(perfect_ctor)
////{
////  //tr2::optional<std::string> ois = "OS";
////  assert (*ois == "OS");
////  
////  // FAILS: tr2::optional<ExplicitStr> oes = "OS"; 
////  tr2::optional<ExplicitStr> oes{"OS"};
////  assert (oes->s == "OS");
////};

TEST(value_or)
{
  tr2::optional<int> oi = 1;
  int i = oi.value_or(0);
  assert (i == 1);
  
  oi = tr2::nullopt;
  assert (oi.value_or(3) == 3);
  
  tr2::optional<std::string> os{"AAA"};
  assert (os.value_or("BBB") == "AAA");
  os = {};
  assert (os.value_or("BBB") == "BBB");
};

TEST(mixed_order)
{
  using namespace std::experimental;
  
  optional<int> oN {nullopt};
  optional<int> o0 {0};
  optional<int> o1 {1};
  
  assert ( (oN <   0));
  assert ( (oN <   1));
  assert (!(o0 <   0));
  assert ( (o0 <   1));
  assert (!(o1 <   0));
  assert (!(o1 <   1));
  
  assert (!(oN >=  0));
  assert (!(oN >=  1));
  assert ( (o0 >=  0));
  assert (!(o0 >=  1));
  assert ( (o1 >=  0));
  assert ( (o1 >=  1));
  
  assert (!(oN >   0));
  assert (!(oN >   1));
  assert (!(o0 >   0));
  assert (!(o0 >   1));
  assert ( (o1 >   0));
  assert (!(o1 >   1));
  
  assert ( (oN <=  0));
  assert ( (oN <=  1));
  assert ( (o0 <=  0));
  assert ( (o0 <=  1));
  assert (!(o1 <=  0));
  assert ( (o1 <=  1));
  
  assert ( (0 >  oN));
  assert ( (1 >  oN));
  assert (!(0 >  o0));
  assert ( (1 >  o0));
  assert (!(0 >  o1));
  assert (!(1 >  o1));
  
  assert (!(0 <= oN));
  assert (!(1 <= oN));
  assert ( (0 <= o0));
  assert (!(1 <= o0));
  assert ( (0 <= o1));
  assert ( (1 <= o1));
  
  assert (!(0 <  oN));
  assert (!(1 <  oN));
  assert (!(0 <  o0));
  assert (!(1 <  o0));
  assert ( (0 <  o1));
  assert (!(1 <  o1));
  
  assert ( (0 >= oN));
  assert ( (1 >= oN));
  assert ( (0 >= o0));
  assert ( (1 >= o0));
  assert (!(0 >= o1));
  assert ( (1 >= o1));
};

struct BadRelops
{
  int i;
};

constexpr bool operator<(BadRelops a, BadRelops b) { return a.i < b.i; }
constexpr bool operator>(BadRelops a, BadRelops b) { return a.i < b.i; } // intentional error!

TEST(bad_relops)
{
  using namespace std::experimental;
  BadRelops a{1}, b{2};
  assert (a < b);
  assert (a > b);
  
  optional<BadRelops> oa = a, ob = b;
  assert (oa < ob);
  assert (!(oa > ob));
  
  assert (oa < b);
  assert (oa > b);
  
  optional<BadRelops&> ra = a, rb = b;
  assert (ra < rb);
  assert (!(ra > rb));
  
  assert (ra < b);
  assert (ra > b);
};


TEST(mixed_equality)
{
  using namespace std::experimental;
  
  assert (make_optional(0) == 0);
  assert (make_optional(1) == 1);
  assert (make_optional(0) != 1);
  assert (make_optional(1) != 0);
  
  optional<int> oN {nullopt};
  optional<int> o0 {0};
  optional<int> o1 {1};
  
  assert (o0 ==  0);
  assert ( 0 == o0);
  assert (o1 ==  1);
  assert ( 1 == o1);
  assert (o1 !=  0);
  assert ( 0 != o1);
  assert (o0 !=  1);
  assert ( 1 != o0);
  
  assert ( 1 != oN);
  assert ( 0 != oN);
  assert (oN !=  1);
  assert (oN !=  0);
  assert (!( 1 == oN));
  assert (!( 0 == oN));
  assert (!(oN ==  1));
  assert (!(oN ==  0));
  
  std::string cat{"cat"}, dog{"dog"};
  optional<std::string> oNil{}, oDog{"dog"}, oCat{"cat"};
  
  assert (oCat ==  cat);
  assert ( cat == oCat);
  assert (oDog ==  dog);
  assert ( dog == oDog);
  assert (oDog !=  cat);
  assert ( cat != oDog);
  assert (oCat !=  dog);
  assert ( dog != oCat);
  
  assert ( dog != oNil);
  assert ( cat != oNil);
  assert (oNil !=  dog);
  assert (oNil !=  cat);
  assert (!( dog == oNil));
  assert (!( cat == oNil));
  assert (!(oNil ==  dog));
  assert (!(oNil ==  cat));
};

TEST(const_propagation)
{
  using namespace std::experimental;
  
  optional<int> mmi{0};
  static_assert(std::is_same<decltype(*mmi), int&>::value, "WTF");
  
  const optional<int> cmi{0};
  static_assert(std::is_same<decltype(*cmi), const int&>::value, "WTF");
  
  optional<const int> mci{0};
  static_assert(std::is_same<decltype(*mci), const int&>::value, "WTF");
  
  optional<const int> cci{0};
  static_assert(std::is_same<decltype(*cci), const int&>::value, "WTF");
};


static_assert(std::is_base_of<std::logic_error, std::experimental::bad_optional_access>::value, "");

TEST(safe_value)
{
  using namespace std::experimental;
  
  try {
    optional<int> ovN{}, ov1{1};
    
    int& r1 = ov1.value();
    assert (r1 == 1);
    
    try { 
      ovN.value();
      assert (false);
    }
    catch (bad_optional_access const&) {
    }
    
    { // ref variant
      int i1 = 1;
      optional<int&> orN{}, or1{i1};
      
      int& r2 = or1.value();
      assert (r2 == 1);
      
      try { 
        orN.value();
        assert (false);
      }
      catch (bad_optional_access const&) {
      }
    }
  }  
  catch(...) {
    assert (false);
  }
};

TEST(optional_ref)
{
  using namespace tr2;
  // FAILS: optional<int&&> orr;
  // FAILS: optional<nullopt_t&> on;
  int i = 8;
  optional<int&> ori;
  assert (!ori);
  ori.emplace(i);
  assert (bool(ori));
  assert (*ori == 8);
  assert (&*ori == &i);
  *ori = 9;
  assert (i == 9);
  
  // FAILS: int& ir = ori.value_or(i);
  int ii = ori.value_or(i);
  assert (ii == 9);
  ii = 7;
  assert (*ori == 9);
  
  int j = 22;
  auto&& oj = make_optional(std::ref(j));
  *oj = 23;
  assert (&*oj == &j);
  assert (j == 23);
};

TEST(optional_ref_const_propagation)
{
  using namespace std::experimental;
  
  int i = 9;
  const optional<int&> mi = i;
  int& r = *mi; 
  optional<const int&> ci = i;
  static_assert(std::is_same<decltype(*mi), int&>::value, "WTF");
  static_assert(std::is_same<decltype(*ci), const int&>::value, "WTF");
  
  unused(r);
};

TEST(optional_ref_assign)
{
  using namespace std::experimental;
  
  int i = 9;
  optional<int&> ori = i;
  
  int j = 1;
  ori = optional<int&>{j};
  ori = {j};
  // FAILS: ori = j;
  
  optional<int&> orx = ori;
  ori = orx;
  
  optional<int&> orj = j;
  
  assert (ori);
  assert (*ori == 1);
  assert (ori == orj);
  assert (i == 9);
  
  *ori = 2;
  assert (*ori == 2);
  assert (ori == 2);
  assert (2 == ori);
  assert (ori != 3);
  
  assert (ori == orj);
  assert (j == 2);
  assert (i == 9);
  
  ori = {};
  assert (!ori);
  assert (ori != orj);
  assert (j == 2);
  assert (i == 9);
};


TEST(optional_ref_swap)
{
  using namespace std::experimental;
  int i = 0;
  int j = 1;
  optional<int&> oi = i;
  optional<int&> oj = j;
  
  assert (&*oi == &i);
  assert (&*oj == &j);
  
  swap(oi, oj);
  assert (&*oi == &j);
  assert (&*oj == &i);
};

TEST(optional_initialization)
{
    using namespace tr2;
    using std::string;
    string s = "STR";

    optional<string> os{s};
    optional<string> ot = s;
    optional<string> ou{"STR"};
    optional<string> ov = string{"STR"};
    
};

#include <unordered_set>

TEST(optional_hashing)
{
    using namespace tr2;
    using std::string;
    
    std::hash<int> hi;
    std::hash<optional<int>> hoi;
    std::hash<string> hs;
    std::hash<optional<string>> hos;
    
    assert (hi(0) == hoi(optional<int>{0}));
    assert (hi(1) == hoi(optional<int>{1}));
    assert (hi(3198) == hoi(optional<int>{3198}));
    
    assert (hs("") == hos(optional<string>{""}));
    assert (hs("0") == hos(optional<string>{"0"}));
    assert (hs("Qa1#") == hos(optional<string>{"Qa1#"}));
    
    std::unordered_set<optional<string>> set;
    assert(set.find({"Qa1#"}) == set.end());
    
    set.insert({"0"});
    assert(set.find({"Qa1#"}) == set.end());
    
    set.insert({"Qa1#"});
    assert(set.find({"Qa1#"}) != set.end());
};


// optional_ref_emulation
template <class T>
struct generic
{
  typedef T type;
};

template <class U>
struct generic<U&>
{
  typedef std::reference_wrapper<U> type;
};

template <class T>
using Generic = typename generic<T>::type;

template <class X>
bool generic_fun()
{
  std::experimental::optional<Generic<X>> op;
  return bool(op);
}

TEST(optional_ref_emulation)
{
  using namespace std::experimental;
  optional<Generic<int>> oi = 1;
  assert (*oi == 1);
  
  int i = 8;
  int j = 4;
  optional<Generic<int&>> ori {i};
  assert (*ori == 8);
  assert ((void*)&*ori != (void*)&i); // !DIFFERENT THAN optional<T&>

  *ori = j;
  assert (*ori == 4);
};


# if OPTIONAL_HAS_THIS_RVALUE_REFS == 1
TEST(moved_on_value_or)
{
  using namespace tr2;
  optional<Oracle> oo{in_place};
  
  assert (oo);
  assert (oo->s == sDefaultConstructed);
  
  Oracle o = std::move(oo).value_or( Oracle{OracleVal{}} );
  assert (oo);
  assert (oo->s == sMovedFrom);
  assert (o.s == sMoveConstructed);
  
  optional<MoveAware<int>> om {in_place, 1};
  assert (om);
  assert (om->moved == false);
  
  /*MoveAware<int> m =*/ std::move(om).value_or( MoveAware<int>{1} );
  assert (om);
  assert (om->moved == true);

# if OPTIONAL_HAS_MOVE_ACCESSORS == 1  
  {
    Date d = optional<Date>{in_place, 1}.value();
    assert (d.i); // to silence compiler warning
	
	Date d2 = *optional<Date>{in_place, 1};
    assert (d2.i); // to silence compiler warning
  }
# endif
};
# endif


TEST(optional_ref_hashing)
{
    using namespace tr2;
    using std::string;
    
    std::hash<int> hi;
    std::hash<optional<int&>> hoi;
    std::hash<string> hs;
    std::hash<optional<string&>> hos;
    
    int i0 = 0;
    int i1 = 1;
    assert (hi(0) == hoi(optional<int&>{i0}));
    assert (hi(1) == hoi(optional<int&>{i1}));
    
    string s{""};
    string s0{"0"};
    string sCAT{"CAT"};
    assert (hs("") == hos(optional<string&>{s}));
    assert (hs("0") == hos(optional<string&>{s0}));
    assert (hs("CAT") == hos(optional<string&>{sCAT}));
    
    std::unordered_set<optional<string&>> set;
    assert(set.find({sCAT}) == set.end());
    
    set.insert({s0});
    assert(set.find({sCAT}) == set.end());
    
    set.insert({sCAT});
    assert(set.find({sCAT}) != set.end());
};

struct Combined
{
  int m = 0;
  int n = 1;
  
  constexpr Combined() : m{5}, n{6} {}
  constexpr Combined(int m, int n) : m{m}, n{n} {}
};

struct Nasty
{
  int m = 0;
  int n = 1;
  
  constexpr Nasty() : m{5}, n{6} {}
  constexpr Nasty(int m, int n) : m{m}, n{n} {}
  
  int operator&() { return n; }
  int operator&() const { return n; }
};

TEST(arrow_operator)
{
  using namespace std::experimental;
  
  optional<Combined> oc1{in_place, 1, 2};
  assert (oc1);
  assert (oc1->m == 1);
  assert (oc1->n == 2);
  
  optional<Nasty> on{in_place, 1, 2};
  assert (on);
  assert (on->m == 1);
  assert (on->n == 2);
};

TEST(arrow_wit_optional_ref)
{
  using namespace std::experimental;
  
  Combined c{1, 2};
  optional<Combined&> oc = c;
  assert (oc);
  assert (oc->m == 1);
  assert (oc->n == 2);
  
  Nasty n{1, 2};
  Nasty m{3, 4};
  Nasty p{5, 6};
  
  optional<Nasty&> on{n};
  assert (on);
  assert (on->m == 1);
  assert (on->n == 2);
  
  on = {m};
  assert (on);
  assert (on->m == 3);
  assert (on->n == 4);
  
  on.emplace(p);
  assert (on);
  assert (on->m == 5);
  assert (on->n == 6);
  
  optional<Nasty&> om{in_place, n};
  assert (om);
  assert (om->m == 1);
  assert (om->n == 2);
};

TEST(no_dangling_reference_in_value)
{
  // this mostly tests compiler warnings
  using namespace std::experimental;
  optional<int> oi {2};
  unused (oi.value());
  const optional<int> coi {3};
  unused (coi.value());
};

struct CountedObject
{
  static int _counter;
  bool _throw;
  CountedObject(bool b) : _throw(b) { ++_counter; }
  CountedObject(CountedObject const& rhs) : _throw(rhs._throw) { if (_throw) throw int(); }
  ~CountedObject() { --_counter; }
};

int CountedObject::_counter = 0;

TEST(exception_safety)
{
  using namespace std::experimental;
  try {
    optional<CountedObject> oo(in_place, true); // throw
    optional<CountedObject> o1(oo);
  }
  catch(...)
  {
    //
  }
  assert(CountedObject::_counter == 0);
  
  try {
    optional<CountedObject> oo(in_place, true); // throw
    optional<CountedObject> o1(std::move(oo));  // now move
  }
  catch(...)
  {
    //
  }
  assert(CountedObject::_counter == 0);
};

//// constexpr tests

// these 4 classes have different noexcept signatures in move operations
struct NothrowBoth {
  NothrowBoth(NothrowBoth&&) noexcept(true) {};
  void operator=(NothrowBoth&&) noexcept(true) {};
};
struct NothrowCtor {
  NothrowCtor(NothrowCtor&&) noexcept(true) {};
  void operator=(NothrowCtor&&) noexcept(false) {};
};
struct NothrowAssign {
  NothrowAssign(NothrowAssign&&) noexcept(false) {};
  void operator=(NothrowAssign&&) noexcept(true) {};
};
struct NothrowNone {
  NothrowNone(NothrowNone&&) noexcept(false) {};
  void operator=(NothrowNone&&) noexcept(false) {};
};

void test_noexcept()
{
  {
    tr2::optional<NothrowBoth> b1, b2;
    static_assert(noexcept(tr2::optional<NothrowBoth>{tr2::constexpr_move(b1)}), "bad noexcept!");
    static_assert(noexcept(b1 = tr2::constexpr_move(b2)), "bad noexcept!");
  }
  {
    tr2::optional<NothrowCtor> c1, c2;
    static_assert(noexcept(tr2::optional<NothrowCtor>{tr2::constexpr_move(c1)}), "bad noexcept!");
    static_assert(!noexcept(c1 = tr2::constexpr_move(c2)), "bad noexcept!");
  }
  {
    tr2::optional<NothrowAssign> a1, a2;
    static_assert(!noexcept(tr2::optional<NothrowAssign>{tr2::constexpr_move(a1)}), "bad noexcept!");
    static_assert(!noexcept(a1 = tr2::constexpr_move(a2)), "bad noexcept!");
  }
  {
    tr2::optional<NothrowNone> n1, n2;
    static_assert(!noexcept(tr2::optional<NothrowNone>{tr2::constexpr_move(n1)}), "bad noexcept!");
    static_assert(!noexcept(n1 = tr2::constexpr_move(n2)), "bad noexcept!");
  }
}


void constexpr_test_disengaged()
{
  constexpr tr2::optional<int> g0{};
  constexpr tr2::optional<int> g1{tr2::nullopt};
  static_assert( !g0, "initialized!" );
  static_assert( !g1, "initialized!" );
  
  static_assert( bool(g1) == bool(g0), "ne!" );
  
  static_assert( g1 == g0, "ne!" );
  static_assert( !(g1 != g0), "ne!" );
  static_assert( g1 >= g0, "ne!" );
  static_assert( !(g1 > g0), "ne!" );
  static_assert( g1 <= g0, "ne!" );
  static_assert( !(g1 <g0), "ne!" );
  
  static_assert( g1 == tr2::nullopt, "!" );
  static_assert( !(g1 != tr2::nullopt), "!" );
  static_assert( g1 <= tr2::nullopt, "!" );
  static_assert( !(g1 < tr2::nullopt), "!" );
  static_assert( g1 >= tr2::nullopt, "!" );
  static_assert( !(g1 > tr2::nullopt), "!" );
  
  static_assert(  (tr2::nullopt == g0), "!" );
  static_assert( !(tr2::nullopt != g0), "!" );
  static_assert(  (tr2::nullopt >= g0), "!" );
  static_assert( !(tr2::nullopt >  g0), "!" );
  static_assert(  (tr2::nullopt <= g0), "!" );
  static_assert( !(tr2::nullopt <  g0), "!" );
  
  static_assert(  (g1 != tr2::optional<int>(1)), "!" );
  static_assert( !(g1 == tr2::optional<int>(1)), "!" );
  static_assert(  (g1 <  tr2::optional<int>(1)), "!" );
  static_assert(  (g1 <= tr2::optional<int>(1)), "!" );
  static_assert( !(g1 >  tr2::optional<int>(1)), "!" );
  static_assert( !(g1 >  tr2::optional<int>(1)), "!" );
}


constexpr tr2::optional<int> g0{};
constexpr tr2::optional<int> g2{2};
static_assert( g2, "not initialized!" );
static_assert( *g2 == 2, "not 2!" );
static_assert( g2 == tr2::optional<int>(2), "not 2!" );
static_assert( g2 != g0, "eq!" );

# if OPTIONAL_HAS_MOVE_ACCESSORS == 1
static_assert( *tr2::optional<int>{3} == 3, "WTF!" );
static_assert( tr2::optional<int>{3}.value() == 3, "WTF!" );
static_assert( tr2::optional<int>{3}.value_or(1) == 3, "WTF!" );
static_assert( tr2::optional<int>{}.value_or(4) == 4, "WTF!" );
# endif

constexpr tr2::optional<Combined> gc0{tr2::in_place};
static_assert(gc0->n == 6, "WTF!");

// optional refs
int gi = 0;
constexpr tr2::optional<int&> gori = gi;
constexpr tr2::optional<int&> gorn{};
constexpr int& gri = *gori;
static_assert(gori, "WTF");
static_assert(!gorn, "WTF");
static_assert(gori != tr2::nullopt, "WTF");
static_assert(gorn == tr2::nullopt, "WTF");
static_assert(&gri == &*gori, "WTF");

constexpr int gci = 1;
constexpr tr2::optional<int const&> gorci = gci;
constexpr tr2::optional<int const&> gorcn{};

static_assert(gorcn <  gorci, "WTF");
static_assert(gorcn <= gorci, "WTF");
static_assert(gorci == gorci, "WTF");
static_assert(*gorci == 1, "WTF");
static_assert(gorci == gci, "WTF");

namespace constexpr_optional_ref_and_arrow 
{
  using namespace std::experimental;
  constexpr Combined c{1, 2};
  constexpr optional<Combined const&> oc = c;
  static_assert(oc, "WTF!");
  static_assert(oc->m == 1, "WTF!");
  static_assert(oc->n == 2, "WTF!");
}

#if OPTIONAL_HAS_CONSTEXPR_INIT_LIST

namespace InitList
{
  using namespace std::experimental;
  
  struct ConstInitLister
  {
    template <typename T>
	constexpr ConstInitLister(std::initializer_list<T> il) : len (il.size()) {}
    size_t len;
  };
  
  constexpr ConstInitLister CIL {2, 3, 4};
  static_assert(CIL.len == 3, "WTF!");
  
  constexpr optional<ConstInitLister> oil {in_place, {4, 5, 6, 7}};
  static_assert(oil, "WTF!");
  static_assert(oil->len == 4, "WTF!");
}

#endif // OPTIONAL_HAS_CONSTEXPR_INIT_LIST

// end constexpr tests


#include <string>


struct VEC
{
    std::vector<int> v;
    template <typename... X>
    VEC( X&&...x) : v(std::forward<X>(x)...) {}

    template <typename U, typename... X>
    VEC(std::initializer_list<U> il, X&&...x) : v(il, std::forward<X>(x)...) {}
};



int main() {
  tr2::optional<int> oi = 1;
  assert (bool(oi));
  oi.operator=({});
  assert (!oi);

  VEC v = {5, 6};

  if (OPTIONAL_HAS_THIS_RVALUE_REFS)
    std::cout << "Optional has rvalue references for *this" << std::endl;
  else
    std::cout << "Optional doesn't have rvalue references for *this" << std::endl;
	
  if (OPTIONAL_HAS_CONSTEXPR_INIT_LIST)
    std::cout << "Optional has constexpr initializer_list" << std::endl;
  else
    std::cout << "Optional doesn't have constexpr initializer_list" << std::endl;

  if (OPTIONAL_HAS_MOVE_ACCESSORS)
    std::cout << "Optional has constexpr move accessors" << std::endl;
  else
    std::cout << "Optional doesn't have constexpr move accessors" << std::endl;	
}

