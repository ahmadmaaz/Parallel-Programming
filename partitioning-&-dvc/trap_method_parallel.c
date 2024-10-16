#include <stdio.h>
#include <math.h>
#include <time.h>
// Function to evaluate the curve (y = f(x))
float f(float x)
{
    return x * x; // Example: y = x^2
}

// Function to compute the area of a trapezoid
float trapezoid_area(float a, float b, float d)
{
    float area = 0;
    for (float x = a; x < b; x += d)
    {
        area += f(x) + f(x + d);
    }

    return area * d / 2.0f;
}



int main(int argc, char **argv)
{
    float a = 0.0f, b = 1.0f; // Limits of integration
    int n;
    float  total_area;

    printf("Enter the number of intervals: ");
    scanf("%d", &n);

    float d = (b - a) / n; // delta
    clock_t start_time = clock();

    // Each process calculates the area of its subinterval
    total_area = trapezoid_area(a, b, d);

    clock_t end_time = clock();

    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Execution time: %f seconds\n", time_taken);
    printf("The total area under the curve is: %f\n", total_area);

    return 0;
}