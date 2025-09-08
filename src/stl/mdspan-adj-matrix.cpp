#include <limits>
#include <mdspan>
#include <print>
#include <type_traits>
#include <vector>

// Source: https://www.cppstories.com/2025/cpp23_mdspan_adj/

// Adjacency matrix definition:
// Square matrix used to represent a finite (usually dense) graph.
// Elements of the matrix indicate if a pair of vertices are adjacent or not. In weighted graphs
// elements store the edge weight.

using namespace std::literals;

template <typename T>
    requires std::is_arithmetic_v<T>
class Graph
{
public:
    static constexpr T INF = std::numeric_limits<T>::max();

    explicit Graph(size_t numVerts = 0)
        : numVerts{numVerts}, adjacencyMatrix(numVerts * numVerts, INF),
          matrixView(adjacencyMatrix.data(), numVerts, numVerts)
    {
        // All verts are connected to themselves
        for (size_t i = 0; i < numVerts; ++i)
            matrixView[i, i] = 0;
    }

    Graph(const Graph& Other)
        : numVerts{Other.numVerts}, adjacencyMatrix(Other.adjacencyMatrix),
          matrixView(adjacencyMatrix.data(), numVerts, numVerts)
    {
    }

    Graph& operator=(Graph Other)
    {
        swap(*this, Other);
        return *this;
    }

    Graph& operator=(Graph&& Other) = default;

    void AddEdge(size_t u, size_t v, T weight)
    {
        if (u < numVerts && v < numVerts && u != v)
        {
            matrixView[u, v] = weight;
            matrixView[v, u] = weight;
        }
    }

    [[nodiscard]] bool IsConnected(size_t u, size_t v) const
    {
        return u < numVerts && v < numVerts && matrixView[u, v] != INF;
    }

    [[nodiscard]] size_t GetNumVerts() const
    {
        return numVerts;
    }

    // Returning immutable view over the matrix
    [[nodiscard]] std::mdspan<const T, std::dextents<size_t, 2>> GetAdjacencyMatrix() const
    {
        return matrixView;
    }

private:
    friend void swap(Graph& first, Graph& second) noexcept
    {
        using namespace std;
        swap(first.numVerts, second.numVerts);
        swap(first.adjacencyMatrix, second.adjacencyMatrix);
        swap(first.matrixView, second.matrixView);
    }

    size_t numVerts;
    std::vector<T> adjacencyMatrix;
    std::mdspan<T, std::dextents<size_t, 2>> matrixView;
};

template <typename T>
void printGraph(const Graph<T>& g, const std::string_view label)
{
    std::println("Adjacency Matrix {}=", label);
    const auto& matrix = g.GetAdjacencyMatrix();
    for (size_t row = 0; row < matrix.extent(0); ++row)
    {
        for (size_t col = 0; col < matrix.extent(1); ++col)
        {
            const auto weight = matrix[row, col];
            if (weight == Graph<T>::INF)
                std::print(" ∞ ");
            else
                std::print(" {} ", weight);
        }
        std::println();
    }
}

int main()
{
    Graph<int> g1{5};
    g1.AddEdge(0, 1, 4);
    g1.AddEdge(0, 2, 8);
    g1.AddEdge(1, 2, 2);
    g1.AddEdge(1, 3, 6);
    g1.AddEdge(2, 3, 3);
    g1.AddEdge(3, 4, 5);
    g1.AddEdge(4, 0, 7);

    printGraph(g1, "g1"sv);
    std::println();
    std::println("Is node 1 connected to node 3? {}", g1.IsConnected(1, 3));
    std::println("Is node 0 connected to node 4? {}", g1.IsConnected(0, 4));

    auto g2 = g1;
    g2.AddEdge(0, 1, 7);
    printGraph(g2, "g2"sv);

    Graph<int> g3;
    g3 = g2;
    g3.AddEdge(0, 1, 66);

    std::println();
    printGraph(g3, "g3"sv);

    std::println();
    printGraph(g2, "g2"sv);

    std::println();
    printGraph(g1, "g1"sv);

    return 0;
}