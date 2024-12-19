#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>
using namespace std;

template<typename T = int>
class Node
{
public:
    T key;
    Node** forward;
    int level;
    Node(T key, int level) {
        this->key = key;
        this->level = level;
        forward = new Node * [level + 1];
        memset(forward, 0, sizeof(Node*) * (level + 1));
    }

    ~Node() {
        delete[] forward;
    }
};

template<typename T = int>
class SkipList
{
private:
    int MAXLVL=1;
    float P;
    int level;
    Node<T>* header;
    int nodecounter;

    int randomLevel() {
        float r = (float)rand() / RAND_MAX;
        int lvl = 0;
        while (r < P && lvl < MAXLVL)
        {
            lvl++;
            r = (float)rand() / RAND_MAX;
        }
        return lvl;
    }

public:

    // КОНСТРУКТОРЫ
    SkipList(int MAXLVL, float P) {
        this->MAXLVL = MAXLVL;
        this->P = P;
        level = 0;
        header = new Node<T>(-1, MAXLVL);
        nodecounter = 0;
    }

    SkipList(const SkipList& other) : SkipList(other.MAXLVL, other.P) {
        Node<T>* current = other.header->forward[0];
        while (current != nullptr) {
            insertElement(current->key, current->level);
            current = current->forward[0];
        }
    }

    SkipList(SkipList&& other)
        : header(other.header), level(other.level), nodecounter(other.nodecounter) {
        other.header = nullptr;
        other.level = 0;
    }


    // ОПЕРАТОРЫ
    SkipList& operator=(const SkipList& other) {
        if (this == &other) return *this;
        clear();
        Node<T>* current = other.header->forward[0];
        while (current != nullptr) {
            insertElement(current->value, current->level);
            current = current->forward[0];
        }
        return *this;
    }

    SkipList& operator=(SkipList&& other) {
        if (this != &other) {
            clear();
            header = other.header;
            level = other.level;
            nodecounter = other.nodecouter;
            other.header = nullptr;
            other.level = 0;
        }
        return *this;
    }


    // УДАЛЕНИЕ
    void clear() {
        Node<T>* current = header->forward[0];
        while (current != nullptr) {
            Node<T>* temp = current;
            current = current->forward[0];
            delete temp;
        }
        for (int i = 0; i < level; ++i) {
            header->forward[i] = nullptr;
        }

        nodecounter = 0;
    }
    ~SkipList() {
        clear();
        delete header;
    }


    // ВСТАВКА ЭЛЕМЕНТОВ
    Node<T>* createNode(T key, int level) {
        Node<T>* n = new Node<T>(key, level);
        return n;
    }

    void insertElement(T key, int rlevel) {
        Node<T>* current = header;
        Node<T>** update = new Node<T>*[this->MAXLVL + 1];
        memset(update, 0, sizeof(Node<T>*) * (this->MAXLVL + 1));

        for (int i = this->level; i >= 0; --i) {
            while (current->forward[i] != nullptr && current->forward[i]->key < key)
                current = current->forward[i];
            update[i] = current;
        }
        current = current->forward[0];
        if (current == nullptr || current->key != key) {
            if (rlevel > this->level) {
                for (int i = this->level + 1; i <= rlevel; ++i) {
                    update[i] = header;
                }
                this->level = rlevel;
            }
            Node<T>* n = createNode(key, rlevel);
            for (int i = 0; i <= rlevel; ++i) {
                n->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = n;
            }
            ++this->nodecounter;
        }
        delete[] update;
    }

    void insertElement(T key) {
        insertElement(key, randomLevel());
    }

    template <typename InputIterator>
    void insertElement(InputIterator first, InputIterator last) {
        for (auto it = first; it != last; ++it) {
            insertElement(*it);
        }
    }


    // РАЗМЕРЫ
    int size() {
        return nodecounter;
    }

    bool empty() {
        int size = this->size();

        if (size > 0) {
            return false;
        }
        else {
            return true;
        }
    }


    // ИТЕРАТОР
    class Iterator {
    public:
        Node<T>* current;
        Iterator(Node<T>* node) : current(node) {}

        T& operator*() {
            return current->key;
        }

        Iterator& operator++() {
            current = current->forward[0];
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };

    Iterator begin() {
        return Iterator(header->forward[0]);
    }

    Iterator end() {
        return Iterator(nullptr);
    }

    // ПОИСК
    Iterator find(const T& value) {
        Node<T>* current = header;
        for (int i = level - 1; i >= 0; --i) {
            while (current->forward[i] != nullptr && current->forward[i]->key < value) {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        if (current != nullptr && current->key == value) {
            return Iterator(current);
        }
        return end();
    }

    int count(const T& value) const {
        int cnt = 0;
        Node<T>* current = header->forward[0];
        while (current != nullptr) {
            if (current->key == value) {
                ++cnt;
            }
            current = current->forward[0];
        }
        return cnt;
    }

    Iterator lower_bound(const T& value) {
        Node<T>* current = header;
        for (int i = level - 1; i >= 0; --i) {
            while (current->forward[i]->forward[i] != nullptr && current->forward[i]->forward[i]->key <= value) {
                current = current->forward[i];
            }
        }
        return Iterator(current->forward[0]);
    }

    Iterator upper_bound(const T& value) {
        Node<T>* current = header;
        for (int i = level - 1; i >= 0; --i) {
            while (current->forward[i] != nullptr && current->forward[i]->key <= value) {
                current = current->forward[i];
            }
        }
        return Iterator(current->forward[0]);
    }


    //ERASE
    void erase(Iterator it) {
        Node<T>* node = it.current;
        Node<T>** update = new Node<T>*[this->MAXLVL + 1];
        Node<T>* current = header;

        for (int i = 0; i <= this->MAXLVL; ++i) {
            update[i] = nullptr;
        }
        for (int i = this->level; i >= 0; --i) {
            while (current->forward[i] != nullptr && current->forward[i]->key < node->key) {
                current = current->forward[i];
            }
            update[i] = current;
        }
        if (current->forward[0] == node) {
            for (int i = 0; i <= this->level; ++i) {
                if (update[i]->forward[i] != node) break;
                update[i]->forward[i] = node->forward[i];
            }
            delete node;
            --nodecounter;
        }
        delete[] update;
    }


    void erase(Iterator first, Iterator last) {
        Iterator current = first;
        while (current != last) {
            Iterator next = current;
            ++next;
            erase(current);

            current = next;
        }
    }


    // ВИЗУАЛИЗАЦИЯ
    void displayList() {
        cout << "\n**********" << "\n";
        for (int i = 0; i <= level; i++) {
            Node<T>* node = header->forward[i];
            cout << "Level " << i << ": ";
            while (node != nullptr)
            {
                cout << node->key << " ";
                node = node->forward[i];
            }
            cout << "\n";
        }
    }
};

int main(){

    srand((unsigned)3);

    SkipList<float> lst(5, 0.7f);

    lst.insertElement(3.5f);
    lst.insertElement(6.0f);
    lst.insertElement(5.0f);
    lst.insertElement(8.0f);
    lst.insertElement(12.0f);
    lst.insertElement(159.0f);
    lst.insertElement(17456.0f);
    lst.insertElement(233.17f);
    lst.insertElement(21.0f);
    lst.insertElement(5.3f);
    lst.displayList();

    float a[5]{ 1.0f, 1.1f, 1.2f, 1.3f, 1.4f };
    lst.insertElement(&a[0], &a[5]);
    lst.displayList();

    SkipList<float> lst2(lst);
    lst2.displayList();

    SkipList<float> lst3 = lst;
    lst3.displayList();

    std::cout << lst.size() << std::endl;
    std::cout << lst.empty() << std::endl;

    lst2.clear();
    std::cout << lst2.empty() << std::endl;

    std::cout << *(lst.lower_bound(21.0f)) << std::endl;
    std::cout << *(lst.upper_bound(22.0f)) << std::endl;

    lst.displayList();

    auto frm = lst.find(5.3f), t = lst.find(159.0f);
    lst.erase(frm, t);
    lst.displayList();
}
