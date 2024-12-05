// ThreadedSumCpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <omp.h>

#define vecint std::vector<int>

vecint create_sum_test_vector(int size)
{
    vecint vec(size);
    std::fill(vec.begin(), vec.end(), 1);
    return vec;
}

vecint create_min_test_vector(int size)
{
    vecint vec(size);
    std::fill(vec.begin(), vec.end(), 1);
    for (int i = 0; i < vec.size(); i += 3) {
        vec[i] = -i;
    }
    return vec;
}

std::string pick_method()
{
    std::cout << "Choose function:" << std::endl;
    std::cout << "1. Simple sum" << std::endl;
    std::cout << "2. Sum with array halving" << std::endl;
    std::cout << "3. Find min value and index" << std::endl;
    std::string input;
    
    do
    {
        std::cin >> input;
    } while (input != "1" && input != "2" && input != "3");

    return input;
}

int vector_sum_simple(vecint vec)
{
    int sum = 0;
#pragma omp parallel for reduction (+:sum)
    for (int i = 0; i < vec.size(); i++)
    {
        sum += vec[i];
    }
    return sum;
}

int vector_sum_halving(vecint vec)
{
    int n = vec.size();
    while (n > 1)
    {
#pragma omp parallel for
        for (int i = 0; i < n / 2; i++)
        {
            vec[i] += vec[n - i - 1];
        }
        n = (n + 1) / 2;
    }

    return vec[0];
}

std::pair<int, int> vector_find_min(vecint vec)
{
    int min = INT_MAX,
        index = -1;

#pragma omp parallel for shared(min, index)
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec[i] < min)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
#pragma omp critical
            {
                if (vec[i] < min)
                {
                    min = vec[i];
                    index = i;
                }
            }
        }
    }
    return std::pair<int, int>(min, index);
}

int main()
{
    int size = 1e4;

    std::string input = pick_method();
    int sum;

    if (input == "1" || input == "2") {
        vecint vec = create_sum_test_vector(size);
        
        auto start = std::chrono::high_resolution_clock::now();
        sum = input == "1" ? vector_sum_simple(vec) : vector_sum_halving(vec);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "The sum is: " << sum << std::endl;
        std::cout << "Calculation took " << duration.count() << " milliseconds";
    }
    else {
        vecint vec = create_min_test_vector(size);

        auto start = std::chrono::high_resolution_clock::now();
        auto pair = vector_find_min(vec);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Min element: " << pair.first << std::endl;
        std::cout << "Min index: " << pair.second << std::endl;
        std::cout << "Calculation took " << duration.count() << " milliseconds";
    }
}
