Управление памятью в контейнерах STL. Ускоряем код с помощью кастомных аллокаторов

В предыдущей статье мы подробно разобрали, как устроена архитектура работы с памятью в STL. Мы увидели, как стандартная библиотека разделяет выделение памяти и конструирование объектов. Однако, стандартный std::allocator универсален, а за универсальность приходится платить скоростью. Каждый раз, когда узловой контейнер (например, std::list или std::map) добавляет элемент, он обращается к operator new и malloc. В высоко нагруженных системах или игровых движках это может стать узким местом. В этой статье мы рассмотрим проектирование кастомных аллокаторов, которые могут дать серьезный прирост производительности для таких контейнеров. Обсудим проблемы, возникающие при использовании пользовательских аллокаторов в STL и замерим прирост скорости с помощью бенчмарков.

Продолжаем работать в рамках С++11. Используемый для контейнеров по умолчанию std::allocator относится к аллокаторам без состояния (stateless). У таких аллокакторов нет внутренних нестатических полей данных. Любые два экземпляра такого распределителя памяти равны, так как память, выделенная одним экземпляром, может быть освобождена другим экземпляром. Экземпляры stateless аллокаторов всегда считаются равными при сравнении.

Перед погружением в глубины оптимизации, нужно понять, как узловые контейнеры запрашивают память. При объявлении переменной типа std::list<int, MyAllocator<int>>, вы передаете аллокатор, умеющий выделять память под тип int. Но список должен хранить не только int. Он состоит из внутренних узлов, которые помимо самого значения содержат указатели на следующий и предыдущий элементы, например так:

template <typename T>
struct ListNode {
    ListNode* next;
    T value;
};
Как списку выделить память под ListNode<int>, если ему передали MyAllocator<int>? Напрямую вызвать MyAllocator<int>::allocate() нельзя — не совпадает размер. Здесь на сцену выходит механизм rebind. Для того, чтобы разобраться как он работает, напишем простейший аллокатор:

template <typename T, typename Pool>
class CustomAllocator {
public:
    using value_type = T;

    // Позволяет allocator_traits связать аллокатор с другим типом данных
    template <typename U>
    struct rebind { using other = CustomAllocator<U, Pool>; };
        
    CustomAllocator() noexcept = default;
    explicit CustomAllocator(Pool* pool) noexcept : pool_(pool) {}

    // Конвертирующий конструктор, используемый для rebind
    template <typename U>
    CustomAllocator(const CustomAllocator<U, Pool>& other) noexcept
        : pool_(other.get_pool()) {}
    
    //выделение памяти переадресуем пулу
    T* allocate(std::size_t n) {
        if (!pool_) throw std::bad_alloc();
        void* p = pool_->allocate(n * sizeof(T), alignof(T));
        return static_cast<T*>(p);
    }
    
    //за освобождение памяти также отвечает пул
    void deallocate(T* p, std::size_t n) noexcept {
        if (!pool_) return;
        pool_->deallocate(p, n * sizeof(T));
    }
...
private:
    Pool* pool_ = nullptr;
}
Внутри StackAllocator создаем структуру rebind с помощью которой можно перенастроить аллокатор на другой тип данных. Из CustomAllocator<T, Pool> получается другой тип CustomAllocator<U, Pool> у которого меняется тип хранимого значения с T на U, но пул, из которого выделяется память, остается неизменным.

Аllocator_traits использует rebind при помощи rebind_alloc, как будет показано позже. Для поддержки rebind также нужен конструктор template<typename U>  CustomAllocator(const CustomAllocator<U, Pool>& other). Так как пользователь классов контейнеров STL задает тип значения T, а контейнеру нужно выделять память под свои внутренние структуры данных, например, Node.

Тип Pool представляет пул памяти, хранящий данные контейнера. Аллокатор является частью контейнера, поэтому он должен занимать как можно меньше места. В тоже время пул может строиться на основе различных стратегий распределения памяти, например, на основе массива, размещаемого на стеке. Чтобы ограничить размер аллокатора при использовании такого пула, распределитель должен хранить его не по значению, а по указателю или ссылке. Для упрощения логики аллокатора выделение и освобождение памяти инкапсулируются в пуле.

Теперь посмотрим простейший STL-подобный узловой контейнер:

template <typename T, typename Allocator = std::allocator<T>>
class SimpleList {
    struct Node {
        Node* next;
        T value;
        Node(const T& v, Node* n) : next(n), value(v) {}
    };
    //получаем тип аллокатора, настроенного не на тип значения Т, передаваемый
    //пользователем SimpleList, а на внутренний тип данных Node
    using NodeAlloc       = typename std::allocator_traits<Allocator>
                            ::template rebind_alloc<Node>;
    //специализируем allocator_traits полученным типом для работы с памятью
    //работаем с аллокатором не напрямую, а с помощью этого типа
    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

    Node*      head_  = nullptr;
    NodeAlloc  alloc_;

public:
    SimpleList() = default;
    explicit SimpleList(const Allocator& a) : alloc_(a) {}

    SimpleList(const SimpleList&)            = delete;
    SimpleList& operator=(const SimpleList&) = delete;

    ~SimpleList() {
        while (head_) {
            Node* next = head_->next;
            //вначале вызываем деструктор Node
            NodeAllocTraits::destroy(alloc_, head_);
            //затем освобождаем выделенную память
            NodeAllocTraits::deallocate(alloc_, head_, 1);
            head_ = next;
        }
    }

    void push_front(const T& value) {
        //выделяем память под Node
        Node* n = NodeAllocTraits::allocate(alloc_, 1);
        try {
            //конструируем объект
            NodeAllocTraits::construct(alloc_, n, value, head_);
        } catch (...) {
            //освобождаем память в случае возникновения исключения
            NodeAllocTraits::deallocate(alloc_, n, 1);
            throw;
        }
        head_ = n;
    }

    void pop_front() {
        Node* n = head_;
        head_ = n->next;
        //уничтожаем Node
        NodeAllocTraits::destroy(alloc_, n);
        NodeAllocTraits::deallocate(alloc_, n, 1);
    }

    T&       front()       { return head_->value; }
    const T& front() const { return head_->value; }
    bool     empty() const noexcept { return head_ == nullptr; }
};
С помощью rebind_alloc, использующего rebind аллокатора получаем распределитель NodeAlloc для работы с узлами (Node) списка. Работа с памятью в контейнерах STL происходит не напрямую, а при помощи std::allocator_traits. Специализируем этот класс с целью настройки на работу с Node. Далее работа с памятью происходит при помощи вызова статических методов allocate, construct, destroy и deallocate структуры NodeAllocTraits.

Наш STL подобный контейнер SimpleList принимает объект распределителя в качестве параметра конструктора. Этим достигается гибкость работы с памятью. Можно, при необходимости, как выделить собственный распределитель каждому контейнеру, так и разделить один аллокатор между несколькими. Как всегда, для избежания неопределенного поведения, нужно следить чтобы время жизни аллокатора было не короче чем у соответствующего контейнера.

Теперь посмотрим как можно ускорить работу контейнера за счет создания пользовательских аллокаторов.

Как мы выяснили в прошлой статье, std::allocator является универсальным, но за это приходится платить скоростью выделения памяти. Также он является потокобезопасным, что замедляет работу. Ускорение работы аллокаторов может быть достигнуто как отказом от потокобезопасности, так и использованием предварительного выделения памяти, что особенно важно для узловых контейнеров (std::list, std::map и т.д.), которые обращаются к аллокатору при создании каждого узла.

Рассмотрим два вида распределителей. Первый использует память в виде массива, выделенного на стеке.

template <std::size_t CapacityBytes> //размер пула в байтах
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

	// Все экземпляры StackPool взаимозаменяемы
    // любой экземпляр безопасно «освободит» чужую память
    void deallocate(void*, std::size_t bytes) noexcept {
        // память освобождается вызовом reset() и при выходе из области видимости
    }
    
    bool contains(const void* p) const noexcept {
        const auto* b = static_cast<const unsigned char*>(p);
        return b >= buffer_ && b < buffer_ + CapacityBytes;
    }

    void reset() noexcept { current_ = buffer_; }

private:
    alignas(std::max_align_t) unsigned char buffer_[CapacityBytes];
    unsigned char* current_ = buffer_;
};
Alignas(std::max_align_t) заставляет начало буфера buffer_ быть выровненным по границе alignof(std::max_align_t) — обычно 16 байт на x86-64. Без alignas выравнивание buffer_ определялось бы только типом unsigned char (1 байт) и соседними членами класса, и гарантии max_align_t не было бы.

Стековый пул подходит в ситуациях, когда не требуется длительное хранение данных, например:

Парсинг результатов HTTP запросов. Пул живёт один запрос или переиспользуется через reset() между запросами. Не нужно выделять динамическую память.

Построение игрового кадра. Всё, что нужно на один кадр, выделяется из пула и отбрасывается в конце.

Разработка для встраиваемых систем, где динамическое выделение памяти запрещено.

Объекты, живущие в пределах одной функции или итерации алгоритма.

Главной проблемой StackPool является невозможность освобождения памяти, особенно в случае, когда в ней хранятся объекты разного размера. Поэтому при необходимости длительного хранения данных нужно использовать пул на основе динамического выделения с кешированием.

Посмотрим как построить такой пул, ориентированный на использование в узловых контейнерах STL. Вначале зададим список требований.

В пуле хранятся объекты одинакового размера. Размер узла не известен заранее и может варьироваться в зависимости от реализации коньейнера. Объем памяти, выделяемой под узел, становится известным только при первом вызове allocate.

Чтобы свести к минимуму количество вызовов операторов new/delete нужно кэшировать память в списке свободных блоков. При вызове allocate берем блок из головы списка. Если список пуст, то вызываем operator new. При вызове deallocate возвращаем блок памяти в список.

Следуя устоявшейся терминологии, будем называть слэбом (slab) непрерывный участок памяти, разбитый на одинаковые блоки для хранения узлов. Простейшая реализация пула выглядит следующим образом:

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
        assert(bytes == request_size_ && "FreelistPool deallocate or wrong block size");
        if (bytes != request_size_) return;

        Node* node = static_cast<Node*>(p);
        node->next = head_;
        head_ = node;
    }
};
Так как объем памяти, выделяемой под узел, заранее не известен, то мы можем его определить и выполнить ленивую инициализацию памяти пула только при первом вызове "allocate". При возврате памяти в пул (deallocate) нужно предусмотреть защиту от случая, когда нам пытаются передать память другого размера. Почему это важно? Казалось бы, память выделяемая одним экземпляром FreelistPool может быть освобождена другим его экземпляром. Всё, в конечном счете, сводится к вызовам операторов new/delete. Однако ситуация не так проста, как может показаться на первый взгляд. Если возвращается участок памяти, размер которого меньше, чем block_size_, то при вызове конструктора в этой области памяти будет запись в смежную область, что повлечёт порчу данных и, как следствие, неопределенное поведение. Поэтому будем считать что два экземпляра FreelistPool не равны друг другу.

FreelistPool можно использовать в следующих ситуациях:

Использование в STL контейнерах, где узлы часто создаются и удаляются.

В играх для хранения частиц, летящих снарядов, противников, которые существуют на протяжении нескольких кадров, затем уничтожаются и возрождаются вновь.

Часть узлов имеет длительное время жизни, а часть — короткое. StackPool тут использовать нельзя, так как нельзя вызывать reset().

Проблемы при использовании FreelistPool:

Нельзя держать в одном пуле узлы list и map<int, double>, нужен отдельный пул на каждый размер.

Если все объекты нужно удалить одновременно, то FreelistPool делает лишнюю работу, возвращая память в список свободных блоков при deallocate, а StackPool не делает ничего.

Если необходимо выделить больше чем CapacityBytes, происходит исчерпание слэба, вызывается ::operator new для каждого нового узла.

Теперь проведем тесты производительности описанных аллокаторов и сравним их с std::allocator.
