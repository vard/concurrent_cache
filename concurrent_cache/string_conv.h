#ifndef STRING_CONV_H
#define STRING_CONV_H

#include <string>

namespace concurrent_cache{


// To string
std::string toString(const std::string& val) {
    return val;
}

std::string toString(int val) {
    return std::to_string(val);
}

std::string toString(long val) {
    return std::to_string (val);
}

std::string toString(long long val) {
    return std::to_string (val);
}

std::string toString(unsigned val) {
    return std::to_string (val);
}

std::string toString(unsigned long val) {
    return std::to_string(val);
}

std::string toString(unsigned long long val) {
    return std::to_string (val);
}

std::string toString(float val) {
    return std::to_string (val);
}

std::string toString(double val) {
    return std::to_string (val);
}

std::string toString(long double val) {
    return std::to_string (val);
}


// From string
void fromString(const std::string& str, std::string& val){
    val = str;
}

void fromString(const std::string& str, int& val){
    val = std::stoi(str);
}

void fromString(const std::string& str, long & val){
    val = std::stol(str);
}

void fromString(const std::string& str, long long& val){
    val = std::stoll(str);
}

void fromString(const std::string& str, unsigned long& val){
    val = std::stoul(str);
}

void fromString(const std::string& str, unsigned long long& val){
    val = std::stoull(str);
}

void fromString(const std::string& str, float& val){
    val = std::stof(str);
}

void fromString(const std::string& str, double& val){
    val = std::stod(str);
}

void fromString(const std::string& str, long double& val){
    val = std::stold(str);
}

} // namespace
#endif // STRING_CONV_H
