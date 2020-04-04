#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <experimental/mdspan>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

namespace stdex = std::experimental;
using namespace ranges;

auto iota2(int rows, int columns)
{
    return views::cartesian_product(
        views::iota(0, rows),
        views::iota(0, columns)
    );
}

template<typename subfield_t>
bool apply(const subfield_t& window)
{
    size_t n = 0;
    for (auto [i, j] : iota2(3, 3))
        if (window(i, j))
            n += 1;
    if (window(1, 1))
        return n == 3 || n == 4;
    else
        return n == 3;
}

static const int N = 30;

template<typename grid_t>
bool print(const grid_t& grid)
{
    for (size_t i : views::iota(1, N-1))
    {
        for (size_t j : views::iota(1, N-1))
            std::cout << (grid(i, j)?"#":"-") << " ";
        std::cout << std::endl;
    }
}

int main()
{
    auto grid = views::cartesian_product(
        views::iota(1,N - 1),
        views::iota(1,N - 1)
    );
    using cell_t = uint8_t;
    std::array<cell_t, N * N> buffer1;
    auto field1 = stdex::mdspan<cell_t, N, N>{buffer1.data()};
    std::array<cell_t, N * N> buffer2;
    auto field2 = stdex::mdspan<cell_t, N, N>{buffer2.data()};
    for (;;)
    {
        for (auto [i, j, out] :
            grid |
            views::transform([&field1](auto ij){
                auto [i, j] = ij;
                return std::tuple{
                    i,
                    j,
                    apply(
                        stdex::subspan(
                            field1,
                            std::pair{i - 1, i + 2},
                            std::pair{j - 1, j + 2}
                        )
                    )
                };
            })
        )
        {
            field2(i, j) = out;
        }
        std::cout << "\033[2J";
        print(field2);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(0.01s);
        std::swap(field1, field2);
    }
    return 0;
}
