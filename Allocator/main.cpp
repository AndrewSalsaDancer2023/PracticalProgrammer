#include <benchmark/benchmark.h>

#include <map>
#include <numeric>
#include <algorithm>
#include <random>

#include <iostream>
#include <list> 
#include <cstdio>
#include <cstddef>
#include <memory>

#include <vector>
#include <cassert>
#include <new>
#include <stdexcept>
#include <type_traits>

template <std::size_t CapacityBytes>
class StackPool {
public:
    StackPool() = default;
    StackPool(const StackPool&) = delete;
    StackPool& operator=(const StackPool&) = delete;

    void* allocate(std::size_t bytes, std::size_t alignment) {
        void* ptr = current_;
        std::size_t space = (buffer_ + CapacityBytes) - current_;
        if (!std::align(alignment, bytes, ptr, space))
            throw std::bad_alloc();
        current_ = static_cast<unsigned char*>(ptr) + bytes;
        return ptr;
    }

    void deallocate(void*, std::size_t bytes) noexcept {
        //память освобождается только reset()
    }

    void reset() noexcept { current_ = buffer_; }

private:
    alignas(std::max_align_t) unsigned char buffer_[CapacityBytes];
    unsigned char* current_ = buffer_;
};

// Выделяем 1 МБ на стеке (для 10 000 нод списка этого хватит с огромным запасом)
constexpr std::size_t POOL_SIZE_BYTES = 1024 * 1024;
static StackPool<POOL_SIZE_BYTES> global_pool;


template <std::size_t CapacityBytes>
class FreelistPool {
    struct Node { Node* next; };

    Node*           head_        = nullptr;// указатель на голову списка свободных блоков
    unsigned char*  slab_        = nullptr;// указатель на непрерывный блок памяти
    unsigned char*  slab_next_   = nullptr;// указатель на следующий свободный блок из слэба
    unsigned char*  slab_end_    = nullptr;// slab_ + CapacityBytes
    std::size_t     request_size_= 0;	   // размер запрашиваемого блока при аллокации 	
    std::size_t     block_size_  = 0;	   // размер выровненного блока

    //проверка на принадлежность адреса слэбу. Нужна для оптимизации возврата
    //выделенной памяти. Если память не в слэбе, то для её освобождения нужно
    //вызывать operator delete.
    bool in_slab(const void* p) const noexcept {
        const auto* b = static_cast<const unsigned char*>(p);
        return slab_ && b >= slab_ && b < slab_end_;
    }

	//ленивое создание слэба и определение его границ
    void init_layout() {
        slab_ = static_cast<unsigned char*>(::operator new(CapacityBytes));
        slab_next_ = slab_;
        slab_end_  = slab_ + CapacityBytes;
    }

public:
    FreelistPool() = default;
    FreelistPool(const FreelistPool&) = delete;
    FreelistPool& operator=(const FreelistPool&) = delete;

    ~FreelistPool() {
        //вначале нужно пройти по списку свободных блоков
        //и найти не принадлежащие слэбу.
        Node* node = head_;
        while (node) {
            Node* next = node->next;
            //каждый такой блок освобождаем отдельно
            if (!in_slab(node)) ::operator delete(node);
            node = next;
        }
        //в конце освобождаем слэб
        ::operator delete(slab_);
    }
    
    void calc_block_size(std::size_t bytes, std::size_t alignment) {
        if (alignment < alignof(Node))
          alignment = alignof(Node);
        //определяем выравнивание данных в памяти и размер блока
        //на основании размера узла

        request_size_ = bytes;
        std::size_t sz = bytes < sizeof(Node) ? sizeof(Node) : bytes;
        const std::size_t mask = alignment - 1;
        block_size_ = (sz + mask) & ~mask;
        if (block_size_ > CapacityBytes) throw std::bad_alloc();
    }

    void* allocate(std::size_t bytes, std::size_t alignment) {
        if (request_size_ == 0) {            
            if (alignment < alignof(Node))
                alignment = alignof(Node);
            calc_block_size(bytes, alignment);
            init_layout();
        } else if (bytes != request_size_) {
            throw std::invalid_argument("FreelistPool: mixed block sizes");
        }

        // переиспользуем ранее освобождённые блоки
        if (head_) {
            Node* n = head_;
            head_ = n->next;
            return n;
        }

        // если освобожденных блоков нет, то пытаемся взять из слэба
        if (slab_next_ + block_size_ <= slab_end_) {
            void* p = slab_next_;
            slab_next_ += block_size_;
            return p;
        }

        // если слэб исчерпан, то выделяем память, обращаясь к системе
        return ::operator new(block_size_);
    }

    void deallocate(void* p, std::size_t bytes) noexcept {
	    //защита от случая, когда в список пытаются вернуть блок другого размера
        assert(bytes == request_size_ && "deallocate or wrong block size");
        if (bytes != request_size_) return;
	    
        Node* node = static_cast<Node*>(p);
        node->next = head_;
        head_ = node;
    }
};

template <typename P1, typename P2>
struct pools_interchangeable : std::is_same<P1, P2> {};

// Все StackPool<N> взаимозаменяемы между собой, независимо от N.
template <std::size_t N1, std::size_t N2>
struct pools_interchangeable<StackPool<N1>, StackPool<N2>> : std::true_type {};


template <typename T, typename Pool>
class CustomAllocator {
public:
    using value_type = T;

    // Позволяет allocator_traits перепривязать аллокатор под другой тип
    template <typename U>
    struct rebind { using other = CustomAllocator<U, Pool>; };

    CustomAllocator() noexcept = default;
    explicit CustomAllocator(Pool* pool) noexcept : pool_(pool) {}

    // Конструктор обязателен для rebind
    template <typename U>
    CustomAllocator(const CustomAllocator<U, Pool>& other) noexcept
        : pool_(other.get_pool()) {}

    T* allocate(std::size_t n) {
        if (!pool_ || (n > std::numeric_limits<std::size_t>::max() / sizeof(T)))
		    throw std::bad_alloc();
        void* p = pool_->allocate(n * sizeof(T), alignof(T));
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        if (!pool_) return;
        pool_->deallocate(p, n * sizeof(T));
    }

    Pool* get_pool() const noexcept { return pool_; }

template <typename U, typename OtherPool>
bool operator==(const CustomAllocator<U, OtherPool>& other) const noexcept {
    return pools_interchangeable<Pool, OtherPool>::value;
}
template <typename U, typename OtherPool>
bool operator!=(const CustomAllocator<U, OtherPool>& other) const noexcept {
    return !(*this == other);
}
private:
    Pool* pool_ = nullptr;
};

// Тест 1: Стандартный std::list (10 000 аллокаций в куче)
static void Std_List_PushBack(benchmark::State& state) {
    const int elements = state.range(0); 
    
    for (auto _ : state) {
        std::list<int> l;
        for (int i = 0; i < elements; ++i) {
            l.push_back(i);
        }
        benchmark::DoNotOptimize(l.size());
    }
}
BENCHMARK(Std_List_PushBack)->Arg(10000);

// Тест 2: Кастомный CustomAllocator + StackPool + std::list
static void Stack_List_PushBack(benchmark::State& state) {
    const int elements = state.range(0);
    
    for (auto _ : state) {
        global_pool.reset(); 
        
        CustomAllocator<int, StackPool<POOL_SIZE_BYTES>> a1(&global_pool);
        std::list<int, CustomAllocator<int, StackPool<POOL_SIZE_BYTES>>> l{ a1 };
        for (int i = 0; i < elements; ++i) {
            l.push_back(i);
        }
        benchmark::DoNotOptimize(l.size());
    }
}

BENCHMARK(Stack_List_PushBack)->Arg(10000);

// Тест 3: Кастомный CustomAllocator + FreelistPool + std::list
static void FreeList_List_PushBack(benchmark::State& state) {
    const int elements = state.range(0);
    
    for (auto _ : state) {
        FreelistPool<POOL_SIZE_BYTES> fp;
        CustomAllocator<int, FreelistPool<POOL_SIZE_BYTES>> al(&fp); 
        std::list<int, CustomAllocator<int, FreelistPool<POOL_SIZE_BYTES>>> l{ al };
        for (int i = 0; i < elements; ++i) {
            l.push_back(i);
        }
        benchmark::DoNotOptimize(l.size());
    }
}

BENCHMARK(FreeList_List_PushBack)->Arg(10000);
// --- БЕНЧМАРКИ ОБХОДА STD::LIST ---

static void ListIteration_Setup(benchmark::State& state, bool use_stack) {
    const int elements = state.range(0);
    if (use_stack) {
        for (auto _ : state) {
            state.PauseTiming();
            global_pool.reset();
            
             CustomAllocator<int, StackPool<POOL_SIZE_BYTES>> a1(&global_pool);
	     std::list<int, CustomAllocator<int, StackPool<POOL_SIZE_BYTES>>> l{ a1 };
            
            for (int i = 0; i < elements; ++i) l.push_back(i);
            state.ResumeTiming();

            // ЗАМЕР: Полный последовательный обход списка
            long long sum = 0;
            for (int val : l) {
                sum += val;
            }
            benchmark::DoNotOptimize(sum);
        }
    } else {
        for (auto _ : state) {
            state.PauseTiming();
            std::list<int> l;
            for (int i = 0; i < elements; ++i) l.push_back(i);
            state.ResumeTiming();

            long long sum = 0;
            for (int val : l) {
                sum += val;
            }
            benchmark::DoNotOptimize(sum);
        }
    }
}

static void FreeList_List_Iter(benchmark::State& state) {
    const int elements = state.range(0);
 
	for (auto _ : state) {
	    state.PauseTiming();
        global_pool.reset();
               
        FreelistPool<POOL_SIZE_BYTES> fp;
        CustomAllocator<int, FreelistPool<POOL_SIZE_BYTES>> al(&fp); 
	    std::list<int, CustomAllocator<int, FreelistPool<POOL_SIZE_BYTES>>> l{ al};
            
        for (int i = 0; i < elements; ++i) l.push_back(i);
 	       state.ResumeTiming();

        long long sum = 0;
        for (int val : l) {
   		    sum += val;
        }
        benchmark::DoNotOptimize(sum);
	}
}

static void Std_List_Iter(benchmark::State& state) { ListIteration_Setup(state, false); }
static void Stack_List_Iter(benchmark::State& state) { ListIteration_Setup(state, true); }
BENCHMARK(Std_List_Iter)->Arg(10000);
BENCHMARK(Stack_List_Iter)->Arg(10000);
BENCHMARK(FreeList_List_Iter)->Arg(10000);

// --- БЕНЧМАРКИ ДЛЯ STD::MAP ---

// Тест 4: Стандартный std::map (10 000 аллокаций узлов дерева в куче)
static void StdMap_Insert(benchmark::State& state) {
    const int elements = state.range(0); 
    
    for (auto _ : state) {
        std::map<int, int> m;
        for (int i = 0; i < elements; ++i) {
            m[i] = i; 
        }
        benchmark::DoNotOptimize(m.size());
    }
}
BENCHMARK(StdMap_Insert)->Arg(10000);

// Тест 5: Наш CustomAllocator + std::map
static void StackMap_Insert(benchmark::State& state) {
    const int elements = state.range(0);
    
    for (auto _ : state) {
        global_pool.reset(); 
        
        CustomAllocator<std::pair<const int, int>, StackPool<POOL_SIZE_BYTES>> a1(&global_pool);
        std::map<int, int, std::less<int>, CustomAllocator<std::pair<const int, int>, StackPool<POOL_SIZE_BYTES>>> m{
            std::less<int>(), 
            CustomAllocator<std::pair<const int, int>, StackPool<POOL_SIZE_BYTES>>(a1)};
        
        for (int i = 0; i < elements; ++i) {
            m[i] = i; 
        }
        benchmark::DoNotOptimize(m.size());
    }
}
BENCHMARK(StackMap_Insert)->Arg(10000);

// Тест 6: Наш CustomAllocator + std::map
static void FreeListMap_Insert(benchmark::State& state) {
    const int elements = state.range(0);
    
    for (auto _ : state) {
        FreelistPool<POOL_SIZE_BYTES> fp;
        CustomAllocator<std::pair<const int, int>, FreelistPool<POOL_SIZE_BYTES>> al(&fp); 
    
        std::map<int, int, std::less<int>, CustomAllocator<std::pair<const int, int>, FreelistPool<POOL_SIZE_BYTES>>> m{
            std::less<int>(), 
            CustomAllocator<std::pair<const int, int>, FreelistPool<POOL_SIZE_BYTES>>(al)};
        
        for (int i = 0; i < elements; ++i) {
            m[i] = i;
        }
        benchmark::DoNotOptimize(m.size());
    }
}
BENCHMARK(FreeListMap_Insert)->Arg(10000);

static std::vector<int> MakeSearchKeys(int elements) {
    std::vector<int> keys(elements);

    std::iota(keys.begin(), keys.end(), 0);

    std::mt19937 g(42);
    std::shuffle(keys.begin(), keys.end(), g);

    return keys;
}

template <typename Map>
static void RunMapFindScenario(
    benchmark::State& state,
    Map& m,
    const std::vector<int>& keys)
{
    // Заполняем карту вне измеряемого интервала
    for (int k : keys)
        m[k] = k;

    state.ResumeTiming();

    // Измеряем только поиск
    for (int k : keys) {
        auto it = m.find(k);
        benchmark::DoNotOptimize(it);
    }
}

static void MapFind_Std(benchmark::State& state) {
    const auto keys = MakeSearchKeys(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();

        std::map<int, int> m;

        RunMapFindScenario(state, m, keys);
    }
}

static void MapFind_Stack(benchmark::State& state) {
    const auto keys = MakeSearchKeys(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();

        global_pool.reset();

        using Allocator = CustomAllocator<
            std::pair<const int, int>,
            StackPool<POOL_SIZE_BYTES>>;

        std::map<int, int, std::less<int>, Allocator> m{
            std::less<int>{},
            Allocator(&global_pool)
        };

        RunMapFindScenario(state, m, keys);
    }
}

static void MapFind_FreeList(benchmark::State& state) {
    const auto keys = MakeSearchKeys(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();

        FreelistPool<POOL_SIZE_BYTES> fp;

        using Allocator = CustomAllocator<
            std::pair<const int, int>,
            FreelistPool<POOL_SIZE_BYTES>>;

        std::map<int, int, std::less<int>, Allocator> m{
            std::less<int>{},
            Allocator(&fp)
        };

        RunMapFindScenario(state, m, keys);
    }
}

BENCHMARK(MapFind_Std)->Arg(10000);
BENCHMARK(MapFind_Stack)->Arg(10000);
BENCHMARK(MapFind_FreeList)->Arg(10000);

template <typename Map>
static void RunMapDegradationScenario(Map& m) {
    // 1. Первичное заполнение
    for (int i = 0; i < 4000; ++i)
        m[i] = i;

    // 2. Удаляем четные элементы
    for (int i = 0; i < 4000; i += 2)
        m.erase(i);

    // 3. Добавляем новые элементы
    for (int i = 10000; i < 12000; ++i)
        m[i] = i;

    // 4. Поиск по всему диапазону
    int found = 0;

    for (int i = 0; i < 12000; ++i) {
        if (m.find(i) != m.end())
            ++found;
    }

    benchmark::DoNotOptimize(found);
}

using MapValue = std::pair<const int, int>;

using StdMap = std::map<int, int>;

using StackMapAllocator =
    CustomAllocator<MapValue, StackPool<POOL_SIZE_BYTES>>;

using StackMap =
    std::map<int, int, std::less<int>, StackMapAllocator>;

using FreeListMapAllocator =
    CustomAllocator<MapValue, FreelistPool<POOL_SIZE_BYTES>>;

using FreeListMap =
    std::map<int, int, std::less<int>, FreeListMapAllocator>;

static void Benchmark_MapDegradation_Std(benchmark::State& state) {
    for (auto _ : state) {
        StdMap m;
        RunMapDegradationScenario(m);
    }
}

static void Benchmark_MapDegradation_Stack(benchmark::State& state) {
    for (auto _ : state) {
        global_pool.reset();

        StackMap m{
            std::less<int>{},
            StackMapAllocator(&global_pool)
        };

        RunMapDegradationScenario(m);
    }
}

static void Benchmark_MapDegradation_FreeList(benchmark::State& state) {
    for (auto _ : state) {
        FreelistPool<POOL_SIZE_BYTES> fp;

        FreeListMap m{
            std::less<int>{},
            FreeListMapAllocator(&fp)
        };

        RunMapDegradationScenario(m);
    }
}

BENCHMARK(Benchmark_MapDegradation_Std);
BENCHMARK(Benchmark_MapDegradation_Stack);
BENCHMARK(Benchmark_MapDegradation_FreeList);

BENCHMARK_MAIN();

