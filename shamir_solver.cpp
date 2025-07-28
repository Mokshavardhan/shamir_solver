#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include "json.hpp"

#include <boost/multiprecision/cpp_int.hpp>
using json = nlohmann::json;
using BigInt = boost::multiprecision::cpp_int;
BigInt decodeValue(const std::string& valueStr, int base) {
    BigInt result = 0;
    BigInt power = 1;

    for (int i = valueStr.length() - 1; i >= 0; i--) {
        int digit;
        if (valueStr[i] >= '0' && valueStr[i] <= '9') {
            digit = valueStr[i] - '0';
        } else if (valueStr[i] >= 'a' && valueStr[i] <= 'f') {
            digit = valueStr[i] - 'a' + 10;
        } else {
            throw std::invalid_argument("Invalid character in value string");
        }
        
        if (digit >= base) {
            throw std::invalid_argument("Digit value exceeds base.");
        }

        result += digit * power;
        power *= base;
    }
    return result;
}

BigInt findSecret(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    json data = json::parse(file);

    int k = data["keys"]["k"];
    std::vector<std::pair<BigInt, BigInt>> points;
    int count = 0;
    for (auto& el : data.items()) {
        if (el.key() == "keys" || count >= k) continue;
        BigInt x = std::stoi(el.key());
        int base = std::stoi(el.value()["base"].get<std::string>());
        std::string valueStr = el.value()["value"].get<std::string>();
        BigInt y = decodeValue(valueStr, base);

        points.push_back({x, y});
        count++;
    }
    
    if (points.size() < k) {
        throw std::runtime_error("Not enough points to solve the polynomial.");
    }
    BigInt secret = 0;

    for (int j = 0; j < k; ++j) {
        BigInt xj = points[j].first;
        BigInt yj = points[j].second;

        BigInt numerator = 1;
        BigInt denominator = 1;

        for (int i = 0; i < k; ++i) {
            if (i == j) continue;
            
            BigInt xi = points[i].first;
            numerator *= xi;
            denominator *= (xi - xj);
        }
        BigInt term = yj * numerator / denominator;
        secret += term;
    }

    return secret;
}

int main() {
    try {
        BigInt secret1 = findSecret("testcase1.json");
        std::cout << "Secret for Test Case 1: " << secret1 << std::endl;

        BigInt secret2 = findSecret("testcase2.json");
        std::cout << "Secret for Test Case 2: " << secret2 << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}