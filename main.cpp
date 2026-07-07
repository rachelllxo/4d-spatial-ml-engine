#include<iostream>
#include<cmath>
#include<algorithm>
#include<vector>


const int DIM = 4; // 4-Dimensional Feature Vectors (ML Space)

// 1. Math Element: Define our Multi-Dimensional Point
struct Point {
    double coords[DIM];

    // Square Euclidean Distance Formula
    double squared_distance(const Point& other) const {
        double sum = 0;
        for (int i = 0; i < DIM; ++i) {
            double diff = coords[i] - other.coords[i];
            sum += diff * diff;
        }
        return sum;
    }
};

// 2. CSE Element: Custom K-d Tree Node Layout
struct Node {
    Point point;
    Node* left = nullptr;
    Node* right = nullptr;

    Node(Point p) : point(p) {}
};

// Recursive function to build the tree across alternating dimensions
Node* build_tree(std::vector<Point>& points, int start, int end, int depth) {
    if (start >= end) return nullptr;

    int axis = depth % DIM; // Cycle through X, Y, Z, W dimensions

    // Sort points along current axis to find the median element
    std::sort(points.begin() + start, points.begin() + end, [axis](const Point& a, const Point& b) {
        return a.coords[axis] < b.coords[axis];
    });

    int median = start + (end - start) / 2;
    Node* node = new Node(points[median]);

    // Recursively partition remaining subsets
    node->left = build_tree(points, start, median, depth + 1);
    node->right = build_tree(points, median + 1, end, depth + 1);

    return node;
}

// 3. CSE & Physics Element: Pruned Radius Search Algorithm
void radius_search(Node* root, const Point& query, double radius, int depth, std::vector<Point>& results) {
    if (!root) return;

    double sq_dist = root->point.squared_distance(query);
    if (sq_dist <= radius * radius) {
        results.push_back(root->point);
    }

    int axis = depth % DIM;
    double axis_diff = query.coords[axis] - root->point.coords[axis];

    // Smart Tree Pruning: Check subtrees based on geometric orientation
    if (axis_diff < 0) {
        radius_search(root->left, query, radius, depth + 1, results);
        // If the bounding box cross-boundary is close enough, check the other side
        if (std::abs(axis_diff) < radius) {
            radius_search(root->right, query, radius, depth + 1, results);
        }
    } else {
        radius_search(root->right, query, radius, depth + 1, results);
        if (std::abs(axis_diff) < radius) {
            radius_search(root->left, query, radius, depth + 1, results);
        }
    }
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "   Custom 4D Spatial Search Engine    \n";
    std::cout << "========================================================\n\n";

    const int NUM_ENTITIES = 20000; // Adjusted slightly for online compiler memory allocations
    std::vector<Point> dataset(NUM_ENTITIES);

    std::mt19937 rng(42); 
    std::uniform_real_distribution<double> dist(-100.0, 100.0);

    for (int i = 0; i < NUM_ENTITIES; ++i) {
        for (int d = 0; d < DIM; ++d) {
            dataset[i].coords[d] = dist(rng);
        }
    }

    std::cout << "[Step 1/2] Bulk-loading points into K-d Tree...\n";
    auto start_build = std::chrono::high_resolution_clock::now();
    
    Node* root = build_tree(dataset, 0, NUM_ENTITIES, 0);
    
    auto end_build = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> build_time = end_build - start_build;
    std::cout << "-> Space partitioning complete in: " << build_time.count() << " ms\n\n";

    // Define Radius Search Parameters
    Point query_center = {0.0, 0.0, 0.0, 0.0};
    double search_radius = 20.0;

    std::cout << "[Step 2/2] Running optimized radius search...\n";
    auto start_query = std::chrono::high_resolution_clock::now();

    std::vector<Point> matched_neighbors;
    radius_search(root, query_center, search_radius, 0, matched_neighbors);

    auto end_query = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> query_time = end_query - start_query;

    std::cout << "-> Done. Execution time: " << query_time.count() << " ms\n";
    std::cout << "-> Vectors found within radius: " << matched_neighbors.size() << "\n";
    std::cout << "========================================================\n";

    return 0;
}