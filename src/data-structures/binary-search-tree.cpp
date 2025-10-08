#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

template <typename T>
class BinarySearchTree
{
public:
    BinarySearchTree() = default;
    ~BinarySearchTree() = default;

    BinarySearchTree(const BinarySearchTree& other) : root{CloneTree(other.root.get())}
    {
    }

    BinarySearchTree& operator=(const BinarySearchTree& other)
    {
        if (this == &other)
            return *this;

        BinarySearchTree temp{other};
        swap(*this, temp);
        return *this;
    }

    BinarySearchTree(BinarySearchTree&& other) noexcept : root{std::exchange(other.root, nullptr)}
    {
    }

    BinarySearchTree& operator=(BinarySearchTree&& other) noexcept
    {
        if (this != &other)
            swap(*this, other);
        return *this;
    }

    [[nodiscard]] std::string InOrderPrint() const
    {
        std::stringstream ss;

        auto node = MinNode(root.get());
        while (node)
        {
            ss << node->key << ", ";
            node = SuccNode(node);
        }

        return ss.str();
    }

    [[nodiscard]] std::optional<T> Min() const
    {
        auto min = MinNode(root.get());
        return min ? std::optional{min->key} : std::nullopt;
    }

    [[nodiscard]] std::optional<T> Max() const
    {
        auto max = MaxNode(root.get());
        return max ? std::optional{max->key} : std::nullopt;
    }

    [[nodiscard]] bool Search(const T& key) const
    {
        auto node = root.get();
        while (node)
        {
            if (node->key == key)
                return true;

            if (key < node->key)
                node = node->left.get();
            else
                node = node->right.get();
        }

        return false;
    }

    void Insert(T key)
    {
        auto current = root.get();
        Node* parent = nullptr;

        // Descend to a leaf
        while (current != nullptr)
        {
            parent = current;
            if (key < current->key)
                current = current->left.get();
            else
                current = current->right.get();
        }

        // Set parent
        auto new_node = std::make_unique<Node>(std::move(key)); // key MOVED-FROM!
        new_node->parent = parent;

        // Attach node to the appropriate child
        if (parent == nullptr)
        {
            root = std::move(new_node);
        }
        else
        {
            if (new_node->key < parent->key)
                parent->left = std::move(new_node);
            else
                parent->right = std::move(new_node);
        }
    }

private:
    struct Node
    {
        Node(T key) : key{std::move(key)}
        {
        }

        T key;
        Node* parent = nullptr;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    };

    static std::unique_ptr<Node> CloneTree(const Node* srcNode)
    {
        if (srcNode == nullptr)
            return {};

        auto node = std::make_unique<Node>(srcNode->key);
        node->left = CloneTree(srcNode->left.get());
        if (node->left)
            node->left->parent = node.get();

        node->right = CloneTree(srcNode->right.get());
        if (node->right)
            node->right->parent = node.get();

        return node;
    }

    Node* MinNode(Node* node) const
    {
        if (node == nullptr)
            return node;

        while (node->left)
            node = node->left.get();
        return node;
    }

    Node* MaxNode(Node* node) const
    {
        if (node == nullptr)
            return node;

        while (node->right)
            node = node->right.get();
        return node;
    }

    Node* SuccNode(Node* node) const
    {
        if (node == nullptr)
            return node;

        // The successor is the next left-most node from the right child
        if (node->right)
            return MinNode(node->right.get());

        // Find the lowest ancestor of 'node' whose left child is an ancestor of 'node'
        Node* succ = node->parent;
        while (succ && node == succ->right.get())
        {
            node = succ;
            succ = succ->parent;
        }
        return succ;
    }

    friend void swap(BinarySearchTree& left, BinarySearchTree& right) noexcept
    {
        using std::swap;
        swap(left.root, right.root);
    }

    std::unique_ptr<Node> root;
};

int main()
{
    BinarySearchTree<int> B1;
    std::cout << "B1: " << B1.InOrderPrint() << '\n';

    B1.Insert(5);
    B1.Insert(1);
    B1.Insert(2);
    B1.Insert(8);
    B1.Insert(3);
    B1.Insert(-1);
    B1.Insert(7);
    B1.Insert(5);
    B1.Insert(3);
    B1.Insert(2);

    auto B2 = std::move(B1);

    std::cout << "B2: " << B2.InOrderPrint() << '\n';
    std::cout << "Min: " << *B2.Min() << '\n';
    std::cout << "Max: " << *B2.Max() << '\n';
    std::cout << "Search(-1): " << std::boolalpha << B2.Search(-1) << '\n';
    std::cout << "Search(-2): " << std::boolalpha << B2.Search(-2) << '\n';

    BinarySearchTree<int> B3;
    B3 = std::move(B2);

    BinarySearchTree<int> B4{B3};
    std::cout << "B4: " << B4.InOrderPrint() << '\n';
    std::cout << "B3: " << B3.InOrderPrint() << '\n';

    BinarySearchTree<int> B5;
    B5 = B4;
    std::cout << "B5: " << B5.InOrderPrint() << '\n';

    return 0;
}