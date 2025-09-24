#include <exception>
#include <ios>
#include <iostream>
#include <memory>
#include <vector>

struct Widget : std::enable_shared_from_this<Widget>
{
    int val = 0;

    // - shared_from_this allows shared_ptr creation for the current object
    // without actually duplicating control blocks.
    // - Associates a control to the current object and creates a shared_ptr from it.
    // - There must be a preexisting shared_ptr for Widget otherwise
    // shared_from_this typically throws.
    void process(std::vector<std::shared_ptr<Widget>>& processedWidgets)
    {
        processedWidgets.emplace_back(shared_from_this());
        std::cout << "Widget " << val << " processed!\n";
    }

    // To prevent using Widget without a shared_ptr constructors can be
    // made private and factory methods added that return only shared_ptrs.
    Widget(int val = 0) : val{val}
    {
        std::cout << "Constructing Widget " << val << '\n';
    }

    ~Widget()
    {
        std::cout << "Destroying Widget " << val << '\n';
    }

    Widget(const Widget&) = default;
    Widget& operator=(const Widget&) = default;
    Widget(Widget&&) = default;
    Widget& operator=(Widget&&) = default;
};

int main()
{
    // Shared from unique ptr
    {
        std::cout << "=========== Shared from unique ptr ===========\n";
        struct UniquePtrDeleter
        {
            void operator()(Widget* w)
            {
                std::cout << "Unique Ptr Deleter\n";
                delete w;
            }
        };

        std::unique_ptr<Widget, UniquePtrDeleter> unique_ptr{new Widget};

        // Note that shared_ptr stores and uses the deleter of unique_ptr
        std::shared_ptr<Widget> shared_ptr = std::move(unique_ptr);
    }

    // Custom Deleter
    {
        std::cout << "=========== Custom Deleter ===========\n";
        // Custom deleters are not part of the type system, so
        // two shared pointers with a different deleter are the same type.
        auto customDeleter1 = [](Widget* w) {
            std::cout << "Custom Deleter 1\n";
            delete w;
        };

        auto customDeleter2 = [](Widget* w) {
            std::cout << "Custom Deleter 2\n";
            delete w;
        };

        std::shared_ptr<Widget> pw1{new Widget, customDeleter1};
        std::shared_ptr<Widget> pw2{new Widget, customDeleter2};

        std::vector<std::shared_ptr<Widget>> widget_ptrs;
        widget_ptrs.push_back(pw1);
        widget_ptrs.push_back(pw2);
    }

    // std::enable_shared_from_this
    {
        std::cout << "=========== std::enable_shared_from_this ===========\n";
        std::vector<std::shared_ptr<Widget>> processed;

        try
        {
            Widget w{1};
            w.process(processed);
        }
        catch (const std::exception& ex)
        {
            std::cout << "Failed to use enable_shared_from_this without prior shared_ptr! ";
            std::cout << "(exception: " << ex.what() << ")\n";
        }

        auto widget_ptr = std::make_shared<Widget>(2);
        widget_ptr->process(processed);
    }

    // shared pointer to array (C++17)
    {
        std::cout << "=========== shared pointer to array ===========\n";
        std::shared_ptr<Widget[]> arr_shared_ptr(new Widget[3]);
        arr_shared_ptr[0].val = 1;
        arr_shared_ptr[1].val = 2;
        arr_shared_ptr[2].val = 3;

        // std::make_shared does not support arrays
    }

    // weak pointer
    {
        std::cout << "=========== shared pointer to array ===========\n";

        auto widget_shared_ptr = std::make_shared<Widget>(1);
        std::weak_ptr<Widget> widget_weak_ptr = widget_shared_ptr;

        if (auto shared_from_weak = widget_weak_ptr.lock())
        {
            std::cout << "Created shared from weak! (Widget " << shared_from_weak->val << ")\n";
        }

        widget_shared_ptr.reset();

        std::cout << "Weak ptr expired: " << std::boolalpha << widget_weak_ptr.expired() << '\n';

        try
        {
            std::shared_ptr<Widget> sh{widget_weak_ptr};
        }
        catch (const std::exception& ex)
        {
            std::cout << "Cannot create shared from expired weak (exception:" << ex.what() << ")\n";
        }
    }
    return 0;
}