#define TEST_OPTIONAL 1
#define TEST_VECTOR 0

#if TEST_OPTIONAL
#include "optional.h"

 #include <cassert>
 #include <memory>
#include <vector>
 struct C {
     C() noexcept {
         //std::vector<std::string> ar;
         //ar.emplace_back()
         ++def_ctor;
     }
     C(const C& /*other*/) noexcept {
         ++copy_ctor;
     }
     C(C&& /*other*/) noexcept {
         ++move_ctor;
     }
     C& operator=(const C& other) noexcept {
         if (this != &other) {
             ++copy_assign;
         }
         return *this;
     }
     C& operator=(C&& /*other*/) noexcept {
         ++move_assign;
         return *this;
     }
     ~C() {
         ++dtor;
     }

     static size_t InstanceCount() {
         return def_ctor + copy_ctor + move_ctor - dtor;
     }

     static void Reset() {
         def_ctor = 0;
         copy_ctor = 0;
         move_ctor = 0;
         copy_assign = 0;
         move_assign = 0;
         dtor = 0;
     }

     inline static size_t def_ctor = 0;
     inline static size_t copy_ctor = 0;
     inline static size_t move_ctor = 0;
     inline static size_t copy_assign = 0;
     inline static size_t move_assign = 0;
     inline static size_t dtor = 0;
 };

 void TestInitialization() {
     C::Reset();
     {
         Optional<C> o;
         assert(!o.HasValue());
         assert(C::InstanceCount() == 0);
     }
     assert(C::InstanceCount() == 0);

     C::Reset();
     {
         C c;
         Optional<C> o(c);
         assert(o.HasValue());
         assert(C::def_ctor == 1 && C::copy_ctor == 1);
         assert(C::InstanceCount() == 2);
     }
     assert(C::InstanceCount() == 0);

     C::Reset();
     {
         C c;
         Optional<C> o(std::move(c));
         assert(o.HasValue());
         assert(C::def_ctor == 1 && C::move_ctor == 1 && C::copy_ctor == 0 && C::copy_assign == 0
                && C::move_assign == 0);
         assert(C::InstanceCount() == 2);
     }
     assert(C::InstanceCount() == 0);

     C::Reset();
     {
         C c;
         Optional<C> o1(c);
         const Optional<C> o2(o1);
         assert(o1.HasValue());
         assert(o2.HasValue());
         assert(C::def_ctor == 1 && C::move_ctor == 0 && C::copy_ctor == 2 && C::copy_assign == 0
                && C::move_assign == 0);
         assert(C::InstanceCount() == 3);
     }
     assert(C::InstanceCount() == 0);

     C::Reset();
     {
         C c;
         Optional<C> o1(c);
         const Optional<C> o2(std::move(o1));
         assert(C::def_ctor == 1 && C::copy_ctor == 1 && C::move_ctor == 1 && C::copy_assign == 0
                && C::move_assign == 0);
         assert(C::InstanceCount() == 3);
     }
     assert(C::InstanceCount() == 0);
 }

 void TestAssignment() {
     Optional<C> o1;
     Optional<C> o2;
     {  // Assign a value to empty
         C::Reset();
         C c;
         o1 = c;
         assert(C::def_ctor == 1 && C::copy_ctor == 1 && C::dtor == 0);
     }
     {  // Assign a non-empty to empty
         C::Reset();
         o2 = o1;
         assert(C::copy_ctor == 1 && C::copy_assign == 0 && C::dtor == 0);
     }
     {  // Assign non-empty to non-empty
         C::Reset();
         o2 = o1;
         assert(C::copy_ctor == 0 && C::copy_assign == 1 && C::dtor == 0);
     }
     {  // Assign empty to non-empty
         C::Reset();
         Optional<C> empty;
         o1 = empty;
         assert(C::copy_ctor == 0 && C::dtor == 1);
         assert(!o1.HasValue());
     }
 }

 void TestMoveAssignment() {
     {  // Assign a value to empty
         Optional<C> o1;
         C::Reset();
         C c;
         o1 = std::move(c);
         assert(C::def_ctor == 1 && C::move_ctor == 1 && C::dtor == 0);
     }
     {  // Assign a non-empty to empty
         Optional<C> o1;
         Optional<C> o2{C{}};
         C::Reset();
         o1 = std::move(o2);
         assert(C::move_ctor == 1 && C::move_assign == 0 && C::dtor == 0);
     }
     {  // Assign non-empty to non-empty
         Optional<C> o1{C{}};
         Optional<C> o2{C{}};
         C::Reset();
         o2 = std::move(o1);
         assert(C::copy_ctor == 0 && C::move_assign == 1 && C::dtor == 0);
     }
     {  // Assign empty to non-empty
         Optional<C> o1{C{}};
         C::Reset();
         Optional<C> empty;
         o1 = std::move(empty);
         assert(C::copy_ctor == 0 && C::move_ctor == 0 && C::move_assign == 0 && C::dtor == 1);
         assert(!o1.HasValue());
     }
 }

 void TestValueAccess() {
     using namespace std::literals;
     {
         Optional<std::string> o;
         o = "hello"s;
         assert(o.HasValue());
         assert(o.Value() == "hello"s);
         assert(&*o == &o.Value());
         assert(o->length() == 5);
     }
     {
         try {
             Optional<int> o;
             [[maybe_unused]] int v = o.Value();
             assert(false);
         } catch (const BadOptionalAccess& /*e*/) {
         } catch (...) {
             assert(false);
         }
     }
 }

 void TestReset() {
     C::Reset();
     {
         Optional<C> o{C()};
         assert(o.HasValue());
         o.Reset();
         assert(!o.HasValue());
     }
 }

 void TestEmplace() {
     struct S {
         S(int i, std::unique_ptr<int>&& p)
             : i(i)
             , p(std::move(p))  //
         {
         }
         int i;
         std::unique_ptr<int> p;
     };

     Optional<S> o;
     o.Emplace(1, std::make_unique<int>(2));
     assert(o.HasValue());
     assert(o->i == 1);
     assert(*(o->p) == 2);

     o.Emplace(3, std::make_unique<int>(4));
     assert(o.HasValue());
     assert(o->i == 3);
     assert(*(o->p) == 4);
 }

 int main() {
     try {
         TestInitialization();
         TestAssignment();
         TestMoveAssignment();
         TestValueAccess();
         TestReset();
         TestEmplace();
     } catch (...) {
         assert(false);
     }
 }
#endif

#if TEST_VECTOR
#include "vector.h"

#include <stdexcept>
#include <iostream>

namespace {

// Магическое число, используемое для отслеживания живости объекта
inline const uint32_t DEFAULT_COOKIE = 0xdeadbeef;

struct TestObj {
    TestObj() = default;
    TestObj(const TestObj& other) = default;
    TestObj& operator=(const TestObj& other) = default;
    TestObj(TestObj&& other) = default;
    TestObj& operator=(TestObj&& other) = default;
    ~TestObj() {
        cookie = 0;
    }
    [[nodiscard]] bool IsAlive() const noexcept {
        return cookie == DEFAULT_COOKIE;
    }
    uint32_t cookie = DEFAULT_COOKIE;
};

struct Obj {
    Obj() {
        if (default_construction_throw_countdown > 0) {
            if (--default_construction_throw_countdown == 0) {
                throw std::runtime_error("Oops");
            }
        }
        ++num_default_constructed;
    }

    explicit Obj(int id)
        : id(id)  //
    {
        ++num_constructed_with_id;
    }

    Obj(const Obj& other)
        : id(other.id)  //
    {
        if (other.throw_on_copy) {
            throw std::runtime_error("Oops");
        }
        ++num_copied;
    }

    Obj(Obj&& other) noexcept
        : id(other.id)  //
    {
        ++num_moved;
    }

    Obj& operator=(const Obj& other) = default;
    Obj& operator=(Obj&& other) = default;

    ~Obj() {
        ++num_destroyed;
        id = 0;
    }

    static int GetAliveObjectCount() {
        return num_default_constructed + num_copied + num_moved + num_constructed_with_id
            - num_destroyed;
    }

    static void ResetCounters() {
        default_construction_throw_countdown = 0;
        num_default_constructed = 0;
        num_copied = 0;
        num_moved = 0;
        num_destroyed = 0;
        num_constructed_with_id = 0;
    }

    bool throw_on_copy = false;
    int id = 0;

    static inline int default_construction_throw_countdown = 0;
    static inline int num_default_constructed = 0;
    static inline int num_constructed_with_id = 0;
    static inline int num_copied = 0;
    static inline int num_moved = 0;
    static inline int num_destroyed = 0;
};

}  // namespace

void Test1() {
    Obj::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj> v;
        v.Reserve(SIZE);
        assert(Obj::GetAliveObjectCount() == 0);
    }
    {
        Vector<Obj> v(SIZE);
        assert(Obj::GetAliveObjectCount() == SIZE);
        const int old_copy_count = Obj::num_copied;
        const int old_move_count = Obj::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj::GetAliveObjectCount() == SIZE);
        assert(Obj::num_copied == old_copy_count);
        assert(Obj::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj::GetAliveObjectCount() == 0);
}

void Test2() {
    const size_t SIZE = 100;
    Obj::ResetCounters();
    {
        Obj::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj> v(SIZE);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj::num_default_constructed == SIZE / 2 - 1);
        assert(Obj::GetAliveObjectCount() == 0);
    }
    Obj::ResetCounters();
    {
        Vector<Obj> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj> v_copy(v);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
            assert(Obj::num_copied == SIZE / 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj::GetAliveObjectCount() == SIZE);
    }
    Obj::ResetCounters();
    {
        Vector<Obj> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj::GetAliveObjectCount() == SIZE);
    }
}

void Test3() {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));

            assert(v_copy.Size() == MEDIUM_SIZE);
            assert(v_copy.Capacity() == MEDIUM_SIZE);
        }
        assert(Obj::GetAliveObjectCount() == 0);
    }
    {
        Obj::ResetCounters();
        {
            Vector<Obj> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            assert(Obj::num_default_constructed == MEDIUM_SIZE);
            Vector<Obj> moved_from_v(std::move(v));
            assert(moved_from_v.Size() == MEDIUM_SIZE);
            assert(moved_from_v[MEDIUM_SIZE / 2].id == ID);
        }
        assert(Obj::GetAliveObjectCount() == 0);

        assert(Obj::num_moved == 0);
        assert(Obj::num_copied == 0);
        assert(Obj::num_default_constructed == MEDIUM_SIZE);
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj> v_large(LARGE_SIZE);
        v_large = v_medium;
        assert(v_large.Size() == MEDIUM_SIZE);
        assert(v_large.Capacity() == LARGE_SIZE);
        assert(v_large[MEDIUM_SIZE / 2].id == ID);
        assert(Obj::GetAliveObjectCount() == MEDIUM_SIZE + MEDIUM_SIZE);
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(MEDIUM_SIZE);
        {
            Vector<Obj> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            assert(v.Size() == LARGE_SIZE);
            assert(v_large.Capacity() == LARGE_SIZE);
            assert(v_large[LARGE_SIZE - 1].id == ID);
            assert(Obj::GetAliveObjectCount() == LARGE_SIZE + LARGE_SIZE);
        }
        assert(Obj::GetAliveObjectCount() == LARGE_SIZE);
    }
    assert(Obj::GetAliveObjectCount() == 0);
    {
        Obj::ResetCounters();
        Vector<Obj> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj::num_copied;
        v_small = v;
        assert(v_small.Size() == v.Size());
        assert(v_small.Capacity() == MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        assert(Obj::num_copied - num_copies == MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}

void Test4() {
    const size_t ID = 42;
    const size_t SIZE = 100'500;
    {
        Obj::ResetCounters();
        Vector<Obj> v;
        v.Resize(SIZE);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj::num_default_constructed == SIZE);
    }
    assert(Obj::GetAliveObjectCount() == 0);

    {
        const size_t NEW_SIZE = 10'000;
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        v.Resize(NEW_SIZE);
        assert(v.Size() == NEW_SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj::num_destroyed == SIZE - NEW_SIZE);
    }
    assert(Obj::GetAliveObjectCount() == 0);
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        Obj o{ID};
        v.PushBack(o);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj::num_default_constructed == SIZE);
        std::cout << Obj::num_copied << " " << Obj::num_constructed_with_id << " " << Obj::num_moved << std::endl;
        assert(Obj::num_copied == 1);
        assert(Obj::num_constructed_with_id == 1);
        assert(Obj::num_moved == SIZE);
    }
    assert(Obj::GetAliveObjectCount() == 0);
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        v.PushBack(Obj{ID});
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj::num_default_constructed == SIZE);
        assert(Obj::num_copied == 0);
        assert(Obj::num_constructed_with_id == 1);
        assert(Obj::num_moved == SIZE + 1);
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v;
        v.PushBack(Obj{ID});
        v.PopBack();
        assert(v.Size() == 0);
        assert(v.Capacity() == 1);
        assert(Obj::GetAliveObjectCount() == 0);
    }

    {
        Vector<TestObj> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(v[0]);
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
    {
        Vector<TestObj> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack для перемещения существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(std::move(v[0]));
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
}

int main() {
    Test1();
    Test2();
    Test3();
    Test4();
}
#endif
