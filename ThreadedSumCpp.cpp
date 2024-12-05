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
        vec[i] = size - i - 1;
    }
    vec[size * 1 / 4] = -12;
    return vec;
}

std::string pick_method()
{
    std::cout << "Choose function:" << std::endl;
    std::cout << "1. 2 sums" << std::endl;
    std::cout << "2. Find min value and index" << std::endl;
    std::string input;
    
    do
    {
        std::cin >> input;
    } while (input != "1" && input != "2");

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
            //std::this_thread::sleep_for(std::chrono::milliseconds(1));

#pragma omp critical
            if (vec[i] < min)
            {
                min = vec[i];
                index = i;
            }            
        }
    }
    return std::pair<int, int>(min, index);
}

int main()
{
    omp_set_nested(true);

    int size = 1e8;

    std::string input = pick_method();

    if (input == "1") {
        vecint vec1 = create_sum_test_vector(size);
        vecint vec2 = create_sum_test_vector(size);

        int sum1, sum2;
        std::chrono::milliseconds duration1, duration2;
        
#pragma omp sections
        {
#pragma omp section
            {
                auto start = std::chrono::high_resolution_clock::now();
                sum1 = vector_sum_simple(vec1);
                auto end = std::chrono::high_resolution_clock::now();
                
                duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            }
#pragma omp section
            {
                auto start = std::chrono::high_resolution_clock::now();
                sum2 = vector_sum_simple(vec2);
                auto end = std::chrono::high_resolution_clock::now();

                duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            }
        }
        
        std::cout << "Simple sum" << std::endl;
        std::cout << "The sum is: " << sum1 << std::endl;
        std::cout << "Calculation took " << duration1.count() << " milliseconds" << std::endl;
        std::cout << "--------------" << std::endl;
        std::cout << "Halving sum" << std::endl;
        std::cout << "The sum is: " << sum2 << std::endl;
        std::cout << "Calculation took " << duration2.count() << " milliseconds" << std::endl;
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
