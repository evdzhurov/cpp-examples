#include <cassert>
#include <memory>
#include <utility>

template <typename T>
class DoublyLinkedList
{
public:
    void Prepend(T data)
    {
        auto new_head = std::make_unique<Node>(std::move(data));
        new_head->next = std::move(head);

        if (new_head->next)
            new_head->next->prev = new_head.get();

        head = std::move(new_head);
        if (tail == nullptr)
            tail = head.get();
    }

    void Append(T data)
    {
        if (tail == nullptr)
            Prepend(std::move(data));
        else
            InsertAfter(tail, std::move(data));
    }

    void PopBack()
    {
        if (tail)
            Remove(tail);
    }

    void PopFront()
    {
        if (head)
            Remove(head.get());
    }

    [[nodiscard]] bool Search(const T& key) const
    {
        auto node = head.get();
        while (node && node->data != key)
            node = node->next.get();
        return node != nullptr;
    }

    [[nodiscard]] T* Head() const
    {
        return head ? &head->data : nullptr;
    }

    [[nodiscard]] T* Tail() const
    {
        return tail ? &tail->data : nullptr;
    }

private:
    struct Node
    {
        Node(T data) : data{std::move(data)}
        {
        }

        T data;
        std::unique_ptr<Node> next;
        Node* prev = nullptr;
    };

    void InsertAfter(Node* x, T data)
    {
        auto new_node = std::make_unique<Node>(std::move(data));

        // x -> new
        new_node->prev = x;
        // new -> next
        new_node->next = std::move(x->next);

        // new <- next
        if (new_node->next)
            new_node->next->prev = new_node.get();
        else
            tail = new_node.get();

        // x -> new
        x->next = std::move(new_node);
    }

    void Remove(Node* x)
    {
        auto left = x->prev;
        if (left)
        {
            left->next = std::move(x->next); // x gets detached

            if (left->next)
                left->next->prev = left;
            else
                tail = left; // x was the previous tail
        }
        else
        {
            head = std::move(x->next);
            if (!head)
                tail = nullptr;
            else
                head->prev = nullptr;
        }
    }

    std::unique_ptr<Node> head;
    Node* tail = nullptr;
};

int main()
{
    DoublyLinkedList<int> L1;

    L1.Prepend(3);
    L1.Prepend(2);
    L1.Prepend(1);
    assert(L1.Head() && *L1.Head() == 1);
    assert(L1.Tail() && *L1.Tail() == 3);

    L1.Append(4);
    assert(L1.Head() && *L1.Head() == 1);
    assert(L1.Tail() && *L1.Tail() == 4);

    L1.PopBack();
    assert(L1.Tail() && *L1.Tail() == 3);

    L1.PopBack();
    assert(L1.Tail() && *L1.Tail() == 2);
    L1.PopBack();
    assert(L1.Tail() && *L1.Tail() == 1);
    L1.PopBack();
    assert(!L1.Head() && !L1.Tail());

    L1.Append(1);
    assert(L1.Head() && *L1.Head() == 1);
    assert(L1.Tail() && *L1.Tail() == 1);

    L1.Append(2);
    assert(L1.Head() && *L1.Head() == 1);
    assert(L1.Tail() && *L1.Tail() == 2);

    L1.Append(3);
    assert(L1.Head() && *L1.Head() == 1);
    assert(L1.Tail() && *L1.Tail() == 3);

    assert(L1.Search(1));
    assert(L1.Search(2));
    assert(L1.Search(3));
    assert(!L1.Search(4));

    L1.PopFront();
    assert(L1.Head() && *L1.Head() == 2);
    L1.PopFront();
    assert(L1.Head() && *L1.Head() == 3);
    L1.PopFront();
    assert(!L1.Head() && !L1.Tail());

    return 0;
}